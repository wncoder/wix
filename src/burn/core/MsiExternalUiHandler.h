//-------------------------------------------------------------------------------------------------
// <copyright file="MsiExternalUiHandler.h" company="Microsoft">
//    Copyright (c) Microsoft Corporation.  All rights reserved.
//    
//    The use and distribution terms for this software are covered by the
//    Common Public License 1.0 (http://opensource.org/licenses/cpl.php)
//    which can be found in the file CPL.TXT at the root of this distribution.
//    By using this software in any fashion, you are agreeing to be bound by
//    the terms of this license.
//    
//    You must not remove this notice, or any other, from this software.
// </copyright>
// 
// <summary>
//
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once

#include "interfaces\IPerformer.h"
#include "interfaces\ILogger.h"
#include "LogSignatureDecorator.h"
#include "PhasedProgressObserver.h"
#include "CmdLineParser.h"

namespace IronMan
{

class ParseRecord
{
    CString m_record; // the format is (at most) 1: ## 2: ## 3: ## 4: ##
public:
    ParseRecord(const CString& record) : m_record(record) {}
    virtual ~ParseRecord() {}

    int GetInt(int index) // 1-based!
    {
        CString cs = GetString(index);
        if (cs.IsEmpty())
            return -1;

        int i = _ttoi(cs);
        if (i == 0)
        {
            if (cs[0] != L'0')
                return -1;
        }
        return i;
    }

    CString GetString(int index)
    {
        if (IsNull(index))
            return L"";

        int s = Find(index);
        int e = Find(index + 1);
        if (e == -1)
            return m_record.Mid(s + 3);  // skip up past 1:_ (where _ is a space)
        else
            return m_record.Mid(s + 3, e - 1 - (s + 3) );
    }

    bool IsNull(int index) const
    {
        return Find(index) == -1;
    }
private:
    int Find(int index) const
    {
        TCHAR fieldLead[] = _T("n: ");
        fieldLead[0] = _T('1') + index - 1;
        return m_record.Find(fieldLead);
    }
};

struct FilesInUseMessageHandler
{
    static INT ProcessMessage(MSIHANDLE hRecord,  ILogger& logger)
    {
        CString messageToLog;
        CSimpleArray<CString> files;
        CSimpleArray<LPCWSTR> files2;

        // Extract files
        UINT uiLen = ::MsiRecordGetFieldCount(hRecord);
        for (UINT ui = 0; ui < uiLen; ui++)
        {
            TCHAR buffer[2] = {0};
            DWORD cchValueBuf = 0;
            DWORD err = ::MsiRecordGetString(hRecord, ui, buffer, &cchValueBuf);
            HIASSERT(err == ERROR_MORE_DATA, L"the data fits in a 0-character buffer!");

            ++cchValueBuf; // include the null

            CString cs;
            err = ::MsiRecordGetString(hRecord, ui, cs.GetBuffer(cchValueBuf), &cchValueBuf);
            HIASSERT(err == ERROR_SUCCESS, L"da heck?  They just told me the size of the string...");

            cs._ReleaseBuffer();
            if (cs.GetLength() > 0)
            {
                messageToLog += L", " + cs;
                if (ui == 0)
                {
                    messageToLog += L": ";
                }
                else
                {
                    // May need to ensure we have process names or exe names and not just process ID, and possibly remove duplicates but this has not been a problem
                    files.Add(cs);
                    files2.Add(files[ui-1]);
                }
            }
        }

        INT res = IDIGNORE; // SilentUI ignores files in use message.
        LOG(logger, ILogger::Error, messageToLog);
        return res;
    }
};

class MsiExternalUiHandler
{
    DWORD m_dwError;  //Should be set only by INSTALLMESSAGE_ERROR message.
    const bool& m_bAborted;
    bool m_installerDisabledCancel;
    Ux& m_uxLogger;
    IBurnView *m_pBurnView;
    LPCWSTR m_wzPackageId;

    // Upgrade detection variables
    bool m_toProcessRelatedProductsAction;
    bool m_relatedProductsActionStarted;
    bool m_relatedProductsActionProcessed;
    bool m_performingMajorUpgrade;
    bool m_bRollback;

    CString m_strCurrentStep;
    CString m_productCode;
    MsiProgressObserver m_observer;

    ILogger& m_logger;

    // stolen from MSI sources
    bool     m_fProgressByData;
    int      m_iPerTick;
    int      m_iProgress;
    int      m_iProgressTotal;

    // stolen from MSI sources
    enum ipdEnum // Master reset progress direction
    {
        ipdForward,  // Advance progress bar forward
        ipdBackward, // "       "        "   backward
        ipdNextEnum
    };
    ipdEnum m_ipdDirection;

    // stolen from MSI sources
    enum ietEnum // Master reset event types
    {
        ietUninitialized = -1,
        ietTimeRemaining = 0,
        ietScriptInProgress,
        ietTimeRemainingOverflow,
    };
    ietEnum m_ietEventType;
    
    struct RestorePreviousUIHandler
    {
        INSTALLUI_HANDLER m_handler;
        RestorePreviousUIHandler() : m_handler(0) {}
        ~RestorePreviousUIHandler() { ::MsiSetExternalUI(m_handler, 0, NULL); }
    } m_restorePreviousUIHandler;

    /*
        This method allocates weights for different MSI phases.
        Specifically we are allocating: 
            Phase           Ticks       Percent
            Scripting        51          20%
            Execution       194          76%
            Additions        10           4%
            -------------------------------------
            Total           255         100
            -------------------------------------
    */
    static CSimpleArray<UCHAR> GetPhaseWeights()
    {
        CSimpleArray<UCHAR> phaseWeights;
        phaseWeights.Add(51);
        phaseWeights.Add(194);
        phaseWeights.Add(10);
        return phaseWeights;
    }

    void ApplyWorkaroundToEnsurePatchesAreCosted()
    {
        // File costing is skipped when applying Patch(es) and INSTALLUILEVEL is NONE.
        //              Workaround: Set INSTALLUILEVEL to anything but NONE only once.
        static bool bFirstTime = true;
        if (bFirstTime == true)
        {
            UINT r1 = MsiSetInternalUI(INSTALLUILEVEL_BASIC, NULL);
            bFirstTime = false;
        }
    }

public:
    MsiExternalUiHandler(const bool& bAborted
                        , IProgressObserver& observer
                        
                        , ILogger& logger
                        , Ux& uxLogger
                        , __in IBurnView* pBurnView = NULL
                        , __in_z LPCWSTR wzPackageId = NULL)
        : m_bAborted(bAborted)
        , m_installerDisabledCancel(false)
        , m_observer(observer, GetPhaseWeights()) 
        , m_logger(logger)
        , m_uxLogger(uxLogger)
        , m_pBurnView(pBurnView)
        , m_wzPackageId(wzPackageId)
        , m_dwError(ERROR_SUCCESS)
        // state machine variables
        , m_iPerTick(0)                     // number of ticks per action data message
        , m_iProgress(0)                    // progress so far:  this happens twice per product that this patch applies to.
        , m_iProgressTotal(0)               // total progress (i.e., length of the progress bar):  total for creating the script, then total for applying the patch, etc.
        , m_fProgressByData(false)          // is data action or explicit progress message being used
        , m_ipdDirection(ipdForward)        // forward or backwards
        , m_ietEventType(ietUninitialized)  // script event or time remaining event
        , m_toProcessRelatedProductsAction(false)   // controls if we need to listen to upgrade detection messags
        , m_relatedProductsActionStarted(false)     // true when first message we are expecting was received
        , m_relatedProductsActionProcessed(false)   // true when last message we are expecting is received
        , m_performingMajorUpgrade(false)           // true only when major upgrade is being performed in this session
        , m_productCode(L"")                        // product of the msi being installed.
        , m_strCurrentStep(L"")                      // the name of the current action
        , m_bRollback(false)                         // true when MSI is rollback all the changes.
    {
        ApplyWorkaroundToEnsurePatchesAreCosted();

        // Turn off MSI's UI except for the "browse for sources", so MSI handles the missing sources UI itself
        CCmdLineSwitches switches;
        INSTALLUILEVEL externalUIFlags = INSTALLUILEVEL(INSTALLUILEVEL_NONE|INSTALLUILEVEL_SOURCERESONLY);
        
        if (!switches.InteractiveMode())
            externalUIFlags = INSTALLUILEVEL_NONE;  // No dialogs should be displayed for quiet & passive mode.

        ::MsiSetInternalUI(externalUIFlags, NULL);

        // The external UI handler enabled by calling MsiSetExternalUIRecord receives messages in the format of a Record Object. 
        // The external UI handler enabled by calling MsiSetExternalUI receives messages in the format of a string. An external 
        // UI is always called before the Windows Installer internal UI. An enabled record-based external UI is called before any 
        // string-based external UI. If the record-based external UI handler returns 0 (zero), the message is sent to any enabled 
        // string-based external UI handler. If the external UI handler returns a non-zero value, the internal Windows Installer 
        // UI handler is suppressed and the messages are considered handled. 
        // set us as UI handler, for everything
        ::MsiSetExternalUIRecord(UiHandlerRecord, 
                                 INSTALLLOGMODE_FILESINUSE | INSTALLLOGMODE_ERROR | INSTALLLOGMODE_COMMONDATA | INSTALLLOGMODE_ACTIONSTART, 
                                 this, 
                                 NULL);
        
        m_restorePreviousUIHandler.m_handler = ::MsiSetExternalUI(UiHandler,
                                                                    INSTALLLOGMODE_FATALEXIT |
                                                                    INSTALLLOGMODE_ERROR | // Q. what does both do?  A. it goes through whichever is set first (i.e., so the UIRecord version will be called preferentially if we're on a "MSI 3.1 or later" box)
                                                                    INSTALLLOGMODE_WARNING |
                                                                    INSTALLLOGMODE_USER |
                                                                    INSTALLLOGMODE_INFO |
                                                                    INSTALLLOGMODE_RESOLVESOURCE |
                                                                    INSTALLLOGMODE_OUTOFDISKSPACE |
                                                                    INSTALLLOGMODE_ACTIONSTART |
                                                                    INSTALLLOGMODE_ACTIONDATA |
                                                                    INSTALLLOGMODE_COMMONDATA |
                                                                    INSTALLLOGMODE_PROGRESS |
                                                                    INSTALLLOGMODE_INITIALIZE |
                                                                    INSTALLLOGMODE_TERMINATE |
                                                                    INSTALLLOGMODE_SHOWDIALOG,
                                                                    this);
    }
    virtual ~MsiExternalUiHandler() 
    {
        ::MsiSetExternalUIRecord(NULL,0,0,0);
    }
   
    virtual bool IsMajorUpgrade() const
    {
        return m_performingMajorUpgrade;
    }

    virtual void SetProductCode(const CString& productCode)
    {
        m_productCode = productCode;
        m_toProcessRelatedProductsAction = true;
    }

    static INT WINAPI UiHandler(LPVOID pvContext, UINT iMessageType, LPCTSTR szMessage)
    {
        return reinterpret_cast<MsiExternalUiHandler*>(pvContext)->UiHandler(iMessageType, szMessage);
    }
    static INT CALLBACK UiHandlerRecord(LPVOID pvContext, UINT iMessageType, MSIHANDLE hRecord)
    {
        return reinterpret_cast<MsiExternalUiHandler*>(pvContext)->UiHandlerRecord(iMessageType, hRecord);
    }
    DWORD GetInternalError() const { return m_dwError; }

    //------------------------------------------------------------------------------
    // GetCurrentStepName
    //
    // Get the name of the current MSI Action Name that was last executed 
    // before the rollback.  E.g InstallValidate, Custom Action name.
    //------------------------------------------------------------------------------
    const CString& GetCurrentStepName() const 
    {
        return m_strCurrentStep; 
    }

    void Reset(bool bExplicitRollback=false)
    { // use with caution!  should be called before next product install (i.e., at the same time as SetNextPhase on PhasedProgressObserver)
        m_iPerTick = 0;
        m_iProgress = 0;
        m_iProgressTotal = 0;
        m_fProgressByData = false;
        m_ipdDirection = ipdForward;
        m_ietEventType = ietUninitialized;

    /* commented out for now.  See comment in PhasedProgressObserver.h
        m_observer.Reset(bExplicitRollback);
    */
    }

private:
    INT UiHandler(UINT iMessageType, LPCTSTR szMessage)
    {
        if (!szMessage)
            return 0;

        INSTALLMESSAGE mt = (INSTALLMESSAGE)(0xFF000000 & iMessageType);
        UINT uiFlags = 0x00FFFFFF & iMessageType;

        switch (mt)
        {
        case INSTALLMESSAGE_FATALEXIT:      return Log(IDCANCEL, szMessage, ILogger::Error, L"INSTALLMESSAGE_FATALEXIT",   CString(__FUNCTION__));
        case INSTALLMESSAGE_ERROR:
            {
                Log(szMessage, ILogger::Error, L"INSTALLMESSAGE_ERROR_1",        CString(__FUNCTION__));
                return InstallMessageErrorHandler(uiFlags, szMessage, NULL);
            }
        case INSTALLMESSAGE_WARNING: __fallthrough;
        case INSTALLMESSAGE_OUTOFDISKSPACE: __fallthrough;
        case INSTALLMESSAGE_USER: __fallthrough;
        case INSTALLMESSAGE_INFO: __fallthrough;
        case INSTALLMESSAGE_TERMINATE: __fallthrough;
        case INSTALLMESSAGE_INITIALIZE: __fallthrough;
        case INSTALLMESSAGE_SHOWDIALOG:
            return m_pBurnView->OnExecuteMsiMessage(m_wzPackageId, mt, uiFlags, szMessage);

        case INSTALLMESSAGE_ACTIONSTART:
            {
                if (m_toProcessRelatedProductsAction && !m_relatedProductsActionProcessed)
                {
                    if (-1 != CString(szMessage).Find(L"RemoveExistingProducts"))
                    {
                        m_relatedProductsActionStarted = true;
                    }
                }

                return m_pBurnView->OnExecuteMsiMessage(m_wzPackageId, mt, uiFlags, szMessage);
            }
        case INSTALLMESSAGE_ACTIONDATA:     
            {
                if (m_toProcessRelatedProductsAction && !m_relatedProductsActionProcessed && m_relatedProductsActionStarted)
                {
                    CString actionData(szMessage);
                    if (-1 != actionData.Find(L"UPGRADINGPRODUCTCODE"))
                    {
                        m_relatedProductsActionProcessed = true;
                        if (!m_productCode.IsEmpty() && -1 != actionData.Find(m_productCode))
                        {
                            m_performingMajorUpgrade = true;
                        }
                    }
                }

                INT idRet = Log(szMessage, ILogger::Debug, L"INSTALLMESSAGE_ACTIONDATA", CString(__FUNCTION__));

                return OnProgressMessage(L"1: 2 2: 0", true);
            }

        // logging is different for this guy (done inside the OnProgressMessage)
        case INSTALLMESSAGE_PROGRESS:
            return OnProgressMessage(szMessage);

        case INSTALLMESSAGE_COMMONDATA:     return Log(szMessage, ILogger::Debug, L"INSTALLMESSAGE_COMMONDATA", CString(__FUNCTION__));

        // must always return 0 for this one, for some reason (see MSDN sample code for that comment)
        case INSTALLMESSAGE_RESOLVESOURCE:  return Log(0, szMessage, ILogger::Verbose, L"INSTALLMESSAGE_RESOLVESOURCE", CString(__FUNCTION__));

        // MSDN sample also returns 0 for unknown message types
        default:                            return Log(0, szMessage, ILogger::Debug, L"unknown message type", CString(__FUNCTION__));

        }
    }
    void ParseErrorString(LPCTSTR szMessage)
    {
        // parse up string looking for an error between 1000-1999 and greater than 2000, per ms-help://MS.MSDN.vAug06.en/msi/setup/windows_installer_error_messages.htm

        // idea:  it looks like there's a period after the error number.  So, keep trying until the first period, then bail.

        for(int i=0; true; ++i)
        {
            if (szMessage[i] == L'\0')
                return;
            if (szMessage[i] == L'.')
                return;

            int err = _ttoi(&szMessage[i]);
            if (err == 0)
                continue; // probably not a number
            if (err == INT_MAX)
                return;   // overflow
            if (err == INT_MIN)
                return;   // underflow
            if (err < 1000)
                return; // not the right range

            // just right
            m_dwError = err;
            return;
        }
    }

    INT UiHandlerRecord(
        __in UINT iMessageType, 
        __in MSIHANDLE hRecord
        )
    {
        INSTALLMESSAGE mt = (INSTALLMESSAGE)(0xFF000000 & iMessageType);
        UINT uiFlags = 0x00FFFFFF & iMessageType;

        INT nReturn = 0;

        switch (mt)
        {
        case INSTALLMESSAGE_ERROR:
            nReturn = InstallMessageErrorHandler(uiFlags, NULL, hRecord);
            break;

        case INSTALLLOGMODE_FILESINUSE:
            nReturn = OnFilesInUseRecord(hRecord);
            break;

        case INSTALLMESSAGE_COMMONDATA:
            nReturn = InstallMessageCommonDataHandler(hRecord);
            break;        
        case INSTALLMESSAGE_ACTIONSTART:
            {
                if (!m_bRollback)
                {
                    CString strCurrentStep;
                    DWORD dwLength = MAX_PATH;
                    UINT err = ::MsiRecordGetString(hRecord, 1, strCurrentStep.GetBuffer(MAX_PATH), &dwLength);
                    strCurrentStep._ReleaseBuffer();

                    if (ERROR_SUCCESS == err)
                    {
                        if (L"Rollback" == strCurrentStep)
                        {
                            m_bRollback = true;
                        }
                        else if (L"" != strCurrentStep)
                        {
                            m_strCurrentStep = strCurrentStep;
                        }
                    }
                }
                break;
            }
        default:
            HIASSERT(false, L"Unexpected external ui message in UiHandlerRecord!");
            nReturn = -1;
            break;
        }

        return nReturn;
    }

    INT InstallMessageErrorHandler(
        __in UINT uiFlags,
        __in LPCTSTR szMessage, 
        __in MSIHANDLE hRecord
        )
    {
        DWORD dwErrorCode = 0;

        if (szMessage)
        {
            for(int i=0; true; ++i)
            {
                if (szMessage[i] == L'\0')
                    break;
                if (szMessage[i] == L'.')
                    break;

                int err = _ttoi(&szMessage[i]);
                if (err == 0)
                    continue; // probably not a number
                if (err == INT_MAX)
                    break;   // overflow
                if (err == INT_MIN)
                    break;   // underflow
                if (err < 1000)
                    break; // not the right range

                // just right
                dwErrorCode = m_dwError = err;
            }
            Log(szMessage, ILogger::Error, L"INSTALLMESSAGE_ERROR_2", CString(__FUNCTION__)); 
            if (m_pBurnView)
            {
                return m_pBurnView->OnError(m_wzPackageId, dwErrorCode, szMessage, uiFlags);
            }
        }

        if (hRecord)
        {
            CString messageToLog;
            m_dwError = MSI_NULL_INTEGER;

            DWORD cchValueBuf = 0;
            TCHAR buffer[2] = {0};

            DWORD err = ::MsiFormatRecord(NULL, hRecord, buffer, &cchValueBuf);
            if (ERROR_MORE_DATA != err)
            {
                messageToLog = L"Cannot display error: No message in MSI Record";
            }
            else
            {
                ++cchValueBuf; // include the null

                err = ::MsiFormatRecord(NULL, hRecord, messageToLog.GetBuffer(cchValueBuf), &cchValueBuf);
                messageToLog._ReleaseBuffer();
                if (ERROR_SUCCESS != err)
                {
                    HIASSERT(false, L"Bad formattable message; may require a session handle.");
                    if ( messageToLog.IsEmpty() )
                        messageToLog = L"Cannot display error: Failed to get message in MSI Record";
                }

                UINT uiLen = ::MsiRecordGetFieldCount(hRecord);
                if (uiLen > 0)
                {
                    // now get and hang onto the all imporant internal error message ID
                    dwErrorCode = m_dwError = ::MsiRecordGetInteger(hRecord, 1);
                    HIASSERT(m_dwError != MSI_NULL_INTEGER, L"This field should be an error message ID.");
                }

               Log(messageToLog, ILogger::Error, L"INSTALLMESSAGE_ERROR_3", CString(__FUNCTION__)); 
               
               if (m_pBurnView)
               {
                   return m_pBurnView->OnError(m_wzPackageId, dwErrorCode, messageToLog, uiFlags);
               }
            }
        }

        return IDOK;
    }

    INT OnFilesInUseRecord(
        __in MSIHANDLE hRecord
        )
    {
        HRESULT hr = S_OK;
        DWORD er = ERROR_SUCCESS;
        int nResult = IDOK;
        DWORD cFiles = 0;
        LPWSTR* rgwzFiles = NULL;
        DWORD cch = 0;

        cFiles = ::MsiRecordGetFieldCount(hRecord);

        rgwzFiles = (LPWSTR*)MemAlloc(sizeof(LPWSTR*) * cFiles, TRUE);
        ExitOnNull(rgwzFiles, hr, E_OUTOFMEMORY, "Failed to allocate buffer.");

        for (DWORD i = 0; i < cFiles; ++i)
        {
            // get string from record
#pragma prefast(push)
#pragma prefast(disable:6298)
            er = ::MsiRecordGetStringW(hRecord, i + 1, L"", &cch);
#pragma prefast(pop)
            if (ERROR_MORE_DATA == er)
            {
                hr = StrAlloc(&rgwzFiles[i], ++cch);
                ExitOnFailure(hr, "Failed to allocate string buffer.");

                er = ::MsiRecordGetStringW(hRecord, i + 1, rgwzFiles[i], &cch);
            }
            ExitOnWin32Error1(er, hr, "Failed to get record field as string: %u", i);
        }

        nResult = m_pBurnView->OnExecuteMsiFilesInUse(m_wzPackageId, cFiles, (LPCWSTR*)rgwzFiles);

    LExit:
        if (rgwzFiles)
        {
            for (DWORD i = 0; i <= cFiles; ++i)
            {
                ReleaseStr(rgwzFiles[i]);
            }
            MemFree(rgwzFiles);
        }

        return nResult;
    }

    // Handles CommonData Messages.
    // This handler is specifically used to Enable/Disable cancel button as of now.
    INT InstallMessageCommonDataHandler(MSIHANDLE hRecord)
    {
        DWORD cchValueBuf = 0;
        TCHAR buffer[2] = {0};
        CString messageToLog;

        DWORD err = ::MsiFormatRecord(NULL, hRecord, buffer, &cchValueBuf);
        if (ERROR_MORE_DATA != err)
        {
            messageToLog = L"Cannot display error: No message in MSI Record";
        }
        else
        {
            ++cchValueBuf; // include the null

            err = ::MsiFormatRecord(NULL, hRecord, messageToLog.GetBuffer(cchValueBuf), &cchValueBuf);
            messageToLog._ReleaseBuffer();
            if (ERROR_SUCCESS != err)
            {
                HIASSERT(false, L"Bad formattable message; may require a session handle.");
                if ( messageToLog.IsEmpty() )
                    messageToLog = L"Cannot display error: Failed to get message in MSI Record";
            }

            DWORD dwP1 = MSI_NULL_INTEGER;
            DWORD dwP2 = MSI_NULL_INTEGER;

            UINT uiLen = ::MsiRecordGetFieldCount(hRecord);
            // Only if not cancelled (by user) we will go about 
            // changing cancel behavior to honour Enable/Disable message from Windows Installer
            if (!m_bAborted &&  uiLen > 0)
            {
                // Get P1 and P2 values from the record.
                dwP1 = ::MsiRecordGetInteger(hRecord, 1);
                if (dwP1 != MSI_NULL_INTEGER)
                {
                    dwP2 = ::MsiRecordGetInteger(hRecord, 2);
                    if (dwP2 != MSI_NULL_INTEGER)
                    {
                        if (2 == dwP1) // If P1 == 2, we got cancel related record.
                        {
                            if (0 == dwP2) // If P2 = 0, Cancel should be disabled.
                            {
                                // ::MessageBox(NULL, L"Disable Message", L"From Installer", MB_OK);
                                m_installerDisabledCancel = true;
                                //Disable Cancel
                                m_observer.OnStateChange(IProgressObserver::DisableCancel);
                            }
                            else if (1 == dwP2) // If P2 == 1, We can enable cancel again.
                            {
                                // ::MessageBox(NULL, L"Enable Message", L"From Installer", MB_OK);
                                m_installerDisabledCancel = false;
                                //Enable Cancel
                                m_observer.OnStateChange(IProgressObserver::EnableCancel);
                            }
                        }
                    }
                }
            }
        }
        return Log(IDOK, messageToLog, ILogger::Debug, L"INSTALLMESSAGE_COMMONDATA", CString(__FUNCTION__));
    }

    // logging helper methods
    INT Log(LPCTSTR szMessage, ILogger::LoggingLevel lvl, const CString& messageType, const CString& sig)
    {
        return Log(m_bAborted && !m_installerDisabledCancel ? IDCANCEL : IDOK, szMessage, lvl, messageType, sig);
        // return Log(m_bAborted ? IDCANCEL : IDOK, szMessage, lvl, messageType, sig);
    }

    INT Log(INT res, LPCTSTR szMessage, ILogger::LoggingLevel lvl, const CString& messageType, const CString& sig)
    {
        CString cs(L" Returning ");

        switch(res)
        {
        default:
            HIASSERT(false, L"unknown return type!");
            cs += L"unknown return type";
            break;
        case IDOK:      cs += L"IDOK";     break;
        case IDCANCEL:  cs += L"IDCANCEL"; break;
        case IDNO:      cs += L"IDNO";     break;
        case IDRETRY:   cs += L"IDRETRY";  break;
        case IDIGNORE:  cs += L"IDIGNORE"; break;
        case 0:         cs += L"0";        break;
        }

        cs += L". ";
        cs += messageType;

        if (szMessage[0])
        {
            cs += L" [";
            cs += szMessage;
            cs += L"]";
        }

        // Log only errors
        if (lvl == ILogger::Error)
        {
#ifdef IRONMAN_FUNCTIONNAMEINLOGFILE
            m_logger.Log(lvl, IronMan::LogHelper::DecorateFunctionName(sig) + cs);       
#else
            LOG(m_logger, lvl, cs);
#endif
        }

        return res;
    }

    INT OnProgressMessage(LPCTSTR szMessage, bool bActionData=false)
    {
        int nResult = IDOK;
        HRESULT hr = S_OK;

        // stolen from MSI sources
        enum imdEnum  // imt message data fields
        {
            imdSubclass      = 1,
            imdProgressTotal = 2,
            imdPerTick       = 2,
            imdIncrement     = 2,
            imdType          = 3,
            imdDirection     = 3,
            imdEventType     = 4,
            imdNextEnum
        };
        enum iscEnum  // imtProgress subclass messages
        {
            iscMasterReset      = 0,
            iscActionInfo       = 1,
            iscProgressReport   = 2,
            iscProgressAddition = 3,
            iscNextEnum
        };

        ParseRecord pr(szMessage);
        switch (pr.GetInt(imdSubclass))
        {
        case iscMasterReset: // Master reset
        {
            m_iProgressTotal = pr.GetInt(imdProgressTotal);
            m_ipdDirection = (ipdEnum) pr.GetInt(imdDirection);
            m_iProgress = m_ipdDirection == ipdForward ? 0 : m_iProgressTotal;
            m_fProgressByData = false;

            m_ietEventType = (ietEnum) pr.GetInt(imdEventType);
            if ((m_ietEventType == ietTimeRemaining) || (m_ietEventType == ietScriptInProgress))
            {
                if (m_ipdDirection == ipdForward)
                {
                    nResult = m_observer.SetNextPhase();
                }
                else if (m_bAborted)
                {
                    m_observer.OnStateChange(IProgressObserver::UserCancelled);
                    m_observer.OnStateChange(IProgressObserver::Rollback);
                }
                else
                {
                    m_observer.OnStateChange(IProgressObserver::Rollback);
                }

            /* commented out for now.  See comment in PhasedProgressObserver.h
                else
                    m_observer.ImplicitRollback();
            */
            }
            break;
        }
        case iscActionInfo: // Action init
            m_iPerTick = pr.GetInt(imdPerTick);
            m_fProgressByData = pr.GetInt(imdType) != 0;
            break;

        case iscProgressReport: // Reporting actual progress
            {
                int nIncrement = pr.GetInt(imdIncrement);
                int iSign = m_ipdDirection == ipdForward ? 1 : -1;

                int iProgressPrev = m_iProgress;

                if (bActionData == true)
                {
                    m_iProgress += (m_iPerTick * iSign);
                }
                else
                {
                    if (nIncrement > 0)
                    {
                        m_iProgress += pr.GetInt(imdIncrement) * iSign;
                    }
                }

                //
                //  The following piece of code will be executed only during rollback. During rollback MSI is sending lot of garbage data 
                //  that results in negative progress. The code below is a way to work around this problem. I selected the magic number 200
                //  based on trial and error.
                //
                //  Every time we get a negative progress we rollback by an amount equal to 200th of what is left to rollback.
                //
                static const int c_nMagicNumber = 200;
                if (m_bAborted &&   // Setup has been aborted
                    m_ipdDirection == ipdBackward &&    // MSI is sending rollback progress data
                    m_iProgress <= 0)
                {
                    int iTemp = iProgressPrev/c_nMagicNumber;
                    m_iProgress = iProgressPrev - iTemp;
                }

                nResult = SetProgressGauge(m_iProgress, m_iProgressTotal, bActionData);
            }

        case iscProgressAddition: // Progress addition
            // MSI source's basic ui handler ignores this.  Hmm.
            break;

        default:
            HIASSERT(false, L"unknown field1 in INSTALLMESSAGE_PROGRESS message");
            __fallthrough;
        case -1: // comment from MSI code:  "no progress, used to keep UI alive when running in other thread/process"
            break;
        }

        return (m_bAborted && !m_installerDisabledCancel) ? IDCANCEL : IDOK;
    }

    int SetProgressGauge(int iSoFar, int iTotal, bool bActionData)
    {
        int nResult = IDOK;

        switch(m_ietEventType)
        {
        case ietUninitialized:
        case ietTimeRemainingOverflow:
            return IDOK;
        case ietTimeRemaining:
        case ietScriptInProgress:
            break;
        }

        if (iTotal <= 0)
            return IDOK;

        UCHAR ucSoFar=0;
        if (iSoFar < iTotal)
        {
            ULONGLONG ull = iSoFar;
            ull *= 255;
            ull /= iTotal;

            ucSoFar = static_cast<UCHAR>(ull);
        }
        else
        {
            ucSoFar = 255;
        }

        nResult = m_observer.OnProgress(ucSoFar);

        return nResult;
    }
};
}
