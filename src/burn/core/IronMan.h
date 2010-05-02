//-------------------------------------------------------------------------------------------------
// <copyright file="IronMan.h" company="Microsoft">
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
//    This has the main entry point for the engine where we create the UI,
//    controllers and so forth.
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once

static BURN_ENGINE_STATE* vpEngineState;
static BURN_RESUME_TYPE vResumeType;

#include "UXFactory.h"
#include "schema\EngineData.h"
#include "CompositeController.h"
#include "NotifyController.h"
#include "MmioController.h"
#include "SmartLibrary.h"
#include "CompositePerformer.h"
#include "CopyPerformer.h"
#include "CompositeDownloader.h"
#include "UberCoordinator.h"
#include "CmdLineParser.h"
#include "ModuleUtils.h"
#include "DhtmlLogger.h"
#include "TargetPackages.h"
#include "RebootManager.h"
#include "ValidateSemantics.h"
#include "Blockers.h"
#include "BlockChecker.h"
#include "OnlyOneInstance.h"
#include "OperationData.h"
#include "PackageData.h"
#include "UnElevatedController.h"
#include "ElevatedController.h"

//Watson
#include "watson\WatsonManifest.h"
#include "watson\WatsonException.h"
#include "watson\WatsonDefinition.h"

//Ux
#include "Ux\Ux.h"
#include "Ux\FileMetrics.h"

#include "BurnView.h"
#include "BurnController.h"
#include "CacheManager.h"

namespace IronMan
{

//------------------------------------------------------------------------------
// HelpString class
//
// Creates the help string used when a /? or -? is passed to IronSetup
// 
// To add new command line switches, add them below and modify the following
// CmdLineParser.h
// HelpUsageDlgTests.cpp (s_usageString and mapSwitchStatus)
// IronManSatelite.rc
// HelpUsageDlg.h
//-------------------------------------------------------------------------------
class HelpString
{
    bool m_bAreIllegalCommandLineSwitchesUsed;
public:
    HelpString(
                 const DisabledCommandLineSwitches & disabledCommandLineSwitches,     // [in]  Disabled command line switches from Configuration section
                 const AdditionalCommandLineSwitches & additionalCommandLineSwitches, // [in]  Optional Additional command line switches from Configuration section
                 ILogger& logger,                                                     // [in]  Logger
                 CSimpleMap<CString, bool> & mapSwitchStatus                          // [out] Returns map of command line switch names to bool indicating if it is disabled
               )
    {    
        m_bAreIllegalCommandLineSwitchesUsed = false;

        mapSwitchStatus.RemoveAll();

        int iNumDisabledCommandLineSwitches = disabledCommandLineSwitches.GetSize();
        int iNumAdditionalCommandLineSwitches = additionalCommandLineSwitches.GetSize();

        IronMan::CCmdLineSwitches switches;

        CSimpleArray<CString> rgstrKnownSwitches(switches.GetKnownSwitches());
        // Add known switches to map
        for (int iSwitch = 0; iSwitch < rgstrKnownSwitches.GetSize(); ++iSwitch)
        {
            mapSwitchStatus.Add(rgstrKnownSwitches[iSwitch], false /* assume not disabled */);
        }
        mapSwitchStatus.Add(L"Setup", false);  // This is the application name and always comes last

        // Find the "q" switch index, since quiet cannot be disabled
        int indexOfQuietSwitch = mapSwitchStatus.FindKey(L"q");

        // Check if any disabled command line switches are used
        for (int i = 0; i < iNumDisabledCommandLineSwitches; ++i)
        {
            CString csNameOfNextDisabledSwitch = disabledCommandLineSwitches[i].GetSwitchName();

            // The /q switch has a synonym, /quiet
            if (csNameOfNextDisabledSwitch.CompareNoCase(L"quiet") == 0)
            {
                csNameOfNextDisabledSwitch = CString(L"q");
            }

            // See if disabled switch actually maps to an option
            int indexOfSwitch = mapSwitchStatus.FindKey(csNameOfNextDisabledSwitch);
            if ((indexOfSwitch != -1)  &&  (indexOfSwitch != indexOfQuietSwitch)) // Do not allow quiet to be disabled
            {
                mapSwitchStatus.SetAt(csNameOfNextDisabledSwitch, true);
            }
            else if (indexOfSwitch == 0)
            {
                LOG(logger, ILogger::Verbose, L"The \"" + csNameOfNextDisabledSwitch + L"\" switch cannot be disabled, but is specified in the DisabledCommandLineSwitches.");
            }

            // Should this be done in the verify class?
            if (switches.SwitchIsPresent(csNameOfNextDisabledSwitch))
            {                   
                LOG(logger, ILogger::Error, L"Command-line option error: the \"" + csNameOfNextDisabledSwitch + L"\" switch has been disallowed for this package.");                
                LOG(logger, ILogger::Result, L"The \"" + csNameOfNextDisabledSwitch + L"\" switch is disallowed for this package.");
                m_bAreIllegalCommandLineSwitchesUsed = true;
            }
        }

        // Now check that only known or authoring-declared additional switches are used
        CString strIllegalSwitches;
        // Build string array of additional switches
        CSimpleArray<CString> rgstrAdditionalSwitches;
        for (int i = 0; i < additionalCommandLineSwitches.GetSize(); ++i)
        {
            rgstrAdditionalSwitches.Add(additionalCommandLineSwitches[i].GetSwitchName());
        }

        if (!switches.AllSwitchesAreValid(rgstrAdditionalSwitches,strIllegalSwitches))
        {
            LOG(logger, ILogger::Error, L"Command-line option error: unrecognized switch(es) \"" + strIllegalSwitches + L"\".");
            LOG(logger, ILogger::Result, L"Unrecognized switch(es) \"" + strIllegalSwitches + L"\".");
            m_bAreIllegalCommandLineSwitchesUsed = true;
        }
    }

    bool IllegalCommandLineSwitchesAreUsed()
    {
        return m_bAreIllegalCommandLineSwitchesUsed;
    }
};

struct LogCreationUtils
{
    //------------------------------------------------------------------------------
    // CreateLogger
    //
    // Creates Text or DHTML logger object based on cmd line switches. The caller does 
    // not need to catch exceptions since if cannot create a logger, the NullLogger is returned.
    // when .txt or .htm(l) files have been passed. If no log file is passed in the commandline,
    // this function WILL return DHTMLLogger by default if it can and TextLogger if can't.
    //------------------------------------------------------------------------------
    static CAutoPtr<IronMan::ILogger> CreateLogger(const IronMan::CCmdLineSwitches& switches)
    {
        // We should never fail just because we can't create a log file
        // If nothing works, then we should just use the NullLogger
        CPath logFileName;
        try
        {
            // If a file name or directory is passed in, try to use them first
            logFileName = CPath(switches.GetLogFileSpecification());
            CString logFileExtension = logFileName.GetExtension().MakeLower();
            if (logFileExtension == L".txt")
            {
                return CAutoPtr<IronMan::ILogger>(new IronMan::TextLogger(logFileName));
            }
            else if (logFileExtension == L".htm" || logFileExtension == L".html") 
            {
                return CAutoPtr<IronMan::ILogger>(new IronMan::CDhtmlLogger(switches.GetLogFileSpecification()));
            }
            else if (!logFileName.m_strPath.IsEmpty())
            {
                // Directory or non-standard extension(not txt, htm or html)
                return CAutoPtr<IronMan::ILogger>(new IronMan::CDhtmlLogger(logFileName));
            }
        }
        catch(const IronMan::CDetailException)
        {
            // If unable to create a log file in a specific directory or with a specific name
            // we want to try to use the default log location if this is an IronMan Exception
            logFileName = L"";
        }
        catch (...)
        {
            // Non Ironman exception, just use the NullLogger
            return CAutoPtr<IronMan::ILogger>(new IronMan::NullLogger());
        }

        // logFileName is empty
        try
        {
            try 
            {
                return CAutoPtr<IronMan::ILogger>(new IronMan::CDhtmlLogger(logFileName));
            }
            catch(const IronMan::CDetailException)
            {
                return CAutoPtr<IronMan::ILogger>(new IronMan::TextLogger(logFileName));
            }
            catch (...)
            {
                // Non Ironman exception, just use the NullLogger
                return CAutoPtr<IronMan::ILogger>(new IronMan::NullLogger());
            }
        }
        catch (...)
        {
            // all failed, use the NullLogger
            return CAutoPtr<IronMan::ILogger>(new IronMan::NullLogger());
        }
    }

    static CString FormatLogFileErrorMessage(const IronMan::CCmdLineSwitches& switches, const CString strActualLogFile)
    {
        CString strMessage;
        strMessage = L"Could not create log file " + CString(strActualLogFile);
        return strMessage;
    }
};

struct CrossCuttingConcerns
{    
    FileMetrics m_metrics;
    Ux* m_uxLogger;
    
    CrossCuttingConcerns()
        : m_uxLogger(NULL)
    {
        //Initialize Watson/WER
        WatsonData::WatsonDataStatic() = new WatsonData();
        WatsonException<>::InitialiseExceptionHandler();

        IronMan::CCmdLineSwitches switches;
        Operation uioOperation;

        WatsonData::WatsonDataStatic()->SetQueueMode(!switches.InteractiveMode());
        WatsonData::WatsonDataStatic()->SetOperation(uioOperation.GetOperation());
        WatsonData::WatsonDataStatic()->SetUiMode(switches.PassiveMode(), switches.QuietMode());
    }
    virtual ~CrossCuttingConcerns()
    {
        delete m_uxLogger;
        delete WatsonData::WatsonDataStatic();
        WatsonData::WatsonDataStatic() = NULL;
    }
    Ux& GetUxLogger() const 
    {  
        return *m_uxLogger; 
    } // will blow up if not initialized.  This is on purpose.

    //Get the time the whole process started.
    CTime GetStartTime() 
    {
        //There is a possiblility that UxLogger has not been initialized.
        //Therefore, we need to protect against it.
        CTime time = CTime::GetCurrentTime();
        if (NULL != m_uxLogger)
        {
           time = m_uxLogger->GetStartingTime();
        }
        return time;
    }

    void InitializeUxLogger(ILogger& logger
                            , const IPackageData& packageData
                            , unsigned int LangId
                            , Operation::euiOperation eOperation
                            , bool bSilent
                            , bool bPassive)
    {
        IMASSERT(m_uxLogger == NULL);

        const UserExperienceDataCollection::Policy policy = packageData.GetPolicy();

        m_uxLogger = new Ux(logger, m_metrics);
        m_uxLogger->StartAndInitializeUxLogger(eOperation, bSilent, bPassive, policy, packageData.GetMetricsLoaderExe());

        WatsonData::WatsonDataStatic()->GetFilesToKeep().Add(logger.GetFilePath());
        //Get the operation here since it would be correct.
        WatsonData::WatsonDataStatic()->SetOperation(eOperation);
        WatsonData::WatsonDataStatic()->SetApplicationName(packageData.GetPackageName());
        WatsonData::WatsonDataStatic()->SetPackageVersion(packageData.GetVersion());
        WatsonData::WatsonDataStatic()->SetLocApplicationName(packageData.GetPackageName());
        WatsonData::WatsonDataStatic()->SetLcidUi(MAKELCID(LangId, SORT_DEFAULT));
        m_uxLogger->RecordEngineData(packageData.GetPackageName()
                                    , packageData.GetVersion()
                                    , LangId
                                    , packageData.GetServicingTrain());
    }

    static void RenameLog(const CPath& before, ILogger& newLogger)
    {
        WatsonData::WatsonDataStatic()->GetFilesToKeep().Remove(before);
        WatsonData::WatsonDataStatic()->GetFilesToKeep().Add(newLogger.GetFilePath());
        WatsonData::WatsonDataStatic()->SetLogger(newLogger);
    }

    //------------------------------------------------------------------------------
    // ShouldSendWatsonReport
    //
    // Determine if we should send Watson Report
    // We only want to report if the following conditions are true:
    //      a.  the User approved. 
    //      b.  there is a failure. 
    //      c.  it isn't an abort.  
    //           - The abort scenario is possible in the pipe scenario where a user abort a session. 
    //      d.  When it is not 1602.  1602 is not an error because user abort the install.
    //      e.  When it is not 1642.
    //------------------------------------------------------------------------------
    static bool ShouldSendWatsonReport( __in bool UserApprovedSendReport,
                                        __in HRESULT hr)
    {
        return (UserApprovedSendReport 
                    && !MSIUtils::IsSuccess(hr) 
                    && E_ABORT != hr 
                    && S_FALSE != hr
                    && HRESULT_FROM_WIN32(ERROR_INSTALL_USEREXIT) != HRESULT_FROM_WIN32(hr)
                    && HRESULT_FROM_WIN32(ERROR_PATCH_TARGET_NOT_FOUND) != HRESULT_FROM_WIN32(hr));
    }

    void CompleteCCC(HRESULT& hr
                     , const bool bIsSilentOrPassive
                     , bool bSendWatsonReport
                     , ILogger& logger)
    {
        // Standardize error if needed
        hr = StandardizeError(hr);

        m_uxLogger->RecordReturnCode(hr);

        if (ShouldSendWatsonReport(bSendWatsonReport, hr))
        {
            //Ensure that user experience data logs the same bucketing as Watson. 
            m_uxLogger->RecordCrashErrorDatapoints(hr, false);
            //Watson
            PrepareAndSendManifestWatsonReport(hr, logger);
        }

        m_uxLogger->EndAndSendUxReport();
    }

    //------------------------------------------------------------------------------
    // StandardizeError
    //
    // Standardizes Error so that user experience data will have one bucket for a particular error.
    //
    //------------------------------------------------------------------------------
    static HRESULT StandardizeError(HRESULT hr)
    {
        // HRESULT_FROM_WIN32(ERROR_INSTALL_USEREXIT) should be returned instead of E_ABORT
        if ( E_ABORT == hr )
        {
            return HRESULT_FROM_WIN32(ERROR_INSTALL_USEREXIT);
        }
        // Error not changed
        return hr;
    }

    static void LogCommandLineAndTimeZone(ILogger& logger)
    {
        CString section = L" ";
        PUSHLOGSECTIONPOP(logger, L"Environment details", L" ", section);

        CString strMessage;
        strMessage.Format(L"CommandLine = %s", ::GetCommandLine());
        LOG(logger, ILogger::Result, strMessage);

        TIME_ZONE_INFORMATION TimeZoneInformation = {0};
        DWORD result = GetTimeZoneInformation(&TimeZoneInformation);
        LPCWSTR pTimeZoneName = NULL;
        if (result == TIME_ZONE_ID_STANDARD)
            pTimeZoneName = TimeZoneInformation.StandardName;
        else if (result == TIME_ZONE_ID_DAYLIGHT)
            pTimeZoneName = TimeZoneInformation.DaylightName;
        else
            return;
        
        strMessage.Format(L"TimeZone = %s", pTimeZoneName);
        LOG(logger, ILogger::Result, strMessage);

        // Get initial thread locale
        LCID lcidInitial = IronMan::ThreadLanguageSelector::GetLocaleId();
        strMessage.Format(L"Initial LCID = %u", lcidInitial);
        LOG(logger, ILogger::Result, strMessage);
    }

    static void LogPackageNameAndVersion(const Ui& ui, ILogger& logger)
    {
        CString section = L" ";
        PUSHLOGSECTIONPOP(logger, L"Package details", ui.GetName(), section);

        CString strMessage;
        strMessage.Format(L"Package Name = %s", ui.GetName());
        LOG(logger, ILogger::Result, strMessage);

        strMessage.Format(L"Package Version = %s", ui.GetVersion());
        LOG(logger, ILogger::Result, strMessage);
    }

    static void LogOperationType(Operation::euiOperation uioOperation, ILogger& logger)
    {
        CString section = L" ";
        PUSHLOGSECTIONPOP(logger, L"Operation Type", L" ", section);
        CString strMessage;
        CString strOperation = Operation::GetOperationCanonicalString(uioOperation);
        strMessage.Format(L"Operation: %s", strOperation);
        LOG(logger, ILogger::Result, strMessage);
    }

    static void LogUserCollectionPolicy(UserExperienceDataCollection::Policy policy, ILogger& logger)
    {
        CString section = L" ";
        PUSHLOGSECTIONPOP(logger, L"User Experience Data Collection Policy", L" ", section);
        CString strMessage;
        CString strOperation("Unknown");

        switch (policy)
        {
        case UserExperienceDataCollection::Disabled :
            strOperation = "Disabled";
            break;
        case UserExperienceDataCollection::AlwaysUploaded :
            strOperation = "AlwaysUploaded";
            break;
        case UserExperienceDataCollection::UserControlled :
            strOperation = "UserControlled";
            break;
        default:
            IMASSERT2(false, L"User Experience Data Collection should be known, e.g. UserControlled");
            break;
        }

        strMessage.Format(L"User Experience Data Collection Policy: %s", strOperation);
        LOG(logger, ILogger::Result, strMessage);
    }

    static void LogOSVersion(ILogger& logger)
    {
        CString section = L" ";
        PUSHLOGSECTIONPOP(logger, L"OS Version Information", L" ", section);

        OSVERSIONINFOEX osVersion = {0};
        osVersion.dwOSVersionInfoSize = sizeof(osVersion);

        if ( GetVersionEx( reinterpret_cast<OSVERSIONINFO*>(&osVersion) ) ) 
        {
            CString strMessage;
            strMessage.Format(L"OS Version = %d.%d.%d, Platform %d",osVersion.dwMajorVersion,osVersion.dwMinorVersion,osVersion.dwBuildNumber,osVersion.dwPlatformId );
            if (wcslen(osVersion.szCSDVersion) > 0)
            {
                strMessage += L", ";
                strMessage += osVersion.szCSDVersion;
            }
            
            LOG(logger, ILogger::Result, strMessage);
            LOGEX(logger, ILogger::Result, L"OS Description = %s", CSystemUtil::GetOSDescription());
        }
        else
        {
            LOG(logger, ILogger::Result, L"Could not determine OS version");
        }
    }
private:
    //------------------------------------------------------------------------------
    // PrepareAndSendManifestWatsonReport
    //
    // To encapsulate the sending of manifest Watson report.
    //
    //------------------------------------------------------------------------------
    static void PrepareAndSendManifestWatsonReport(HRESULT hr, ILogger& logger)
    {
        LOG(logger, ILogger::Verbose, L"Sending Manifest Report");
        logger.Close();

        WatsonManifest<> watson;
        watson.SendReport(  WatsonData::WatsonDataStatic()->GetGeneralAppName()
                            , WatsonData::WatsonDataStatic()->GetPackageVersion()
                            , WatsonData::WatsonDataStatic()->GetGeneralAppName()
                            , WatsonData::WatsonDataStatic()->GetLcidUi()
                            , WatsonData::WatsonDataStatic()->GetCurrentItemName()
                            , WatsonData::WatsonDataStatic()->GetCurrentPhaseString()
                            , WatsonData::WatsonDataStatic()->GetCurrentActionString()
                            , WatsonData::WatsonDataStatic()->GetUiModeString()
                            , hr
                            , WatsonData::WatsonDataStatic()->GetFilesToKeep()
                            , !(WatsonData::WatsonDataStatic()->IsQueue())
                            , WatsonData::WatsonDataStatic()->GetOperation()
                            , WatsonData::WatsonDataStatic()->IsQueue()
                            , WatsonData::WatsonDataStatic()->GetInternalErrorString()
                            , WatsonData::WatsonDataStatic()->GetWatsonHeader()
                            , WatsonData::WatsonDataStatic()->GetCurrentStep());
        logger.OpenForAppend();
    }
};

template <typename CCC>
class MainT
{
private:
    CCC m_ccc;

    // In running in help mode or elevation mode we are using the null logger and want to detach rather than have the CAutoPtr try to destroy the static object
    // This private class handles this for us
    class DetachNullLogger
    {
        bool m_fUsingNullLogger;
        CAutoPtr<IronMan::ILogger> & m_spLogger;
    public:
        // Constructor which detaches 
        DetachNullLogger(const IronMan::CCmdLineSwitches &switches, CAutoPtr<IronMan::ILogger> &spLogger)
            : m_spLogger(spLogger)
        {
            m_fUsingNullLogger = switches.RequireHelp() || switches.IsRunningElevated() || switches.IsRunningServer();
        }

        // Destructor which detaches 
        ~DetachNullLogger()
        {
            // Don't try to destroy null logger
            if (m_fUsingNullLogger)
            {
                m_spLogger.Detach();
            }
        }

        // Are we using the null logger
        bool UsingNullLogger()
        {
            return m_fUsingNullLogger;
        }
    };

//Make protected for unit testing.
protected:
    // Log and collect the user experience data
    void LogAndReportMetrics(HRESULT& hrDetail
                                    , CString csAction
                                    , bool bShowMessageBox
                                    , ILogger& logger)
    {
        // Show the help dialog

        if (bShowMessageBox)
        {
            ValidateSemantics::OutputErrorMessage(logger);
        }

        LogAndReportMetrics(hrDetail, csAction, logger);
    }

    void LogAndReportMetrics(HRESULT& hrDetail
                            , CString csAction
                            , ILogger& logger)
    {
        // Standarize error if needed
        hrDetail = CrossCuttingConcerns::StandardizeError(hrDetail);

        IronMan::CCmdLineSwitches switches;
        m_ccc.GetUxLogger().RecordInternalFailure(csAction, hrDetail);
        HRESULT hr = CustomErrors::InternalErrorOrUserError;
        m_ccc.CompleteCCC(hr, !switches.InteractiveMode(), false, logger);
    }

    // This is only called for badly authored packages with corrupt or missing .xml data and should never happen for released products
    void DisplayBadPackageMessage(ILogger& logger)
    {
        // Put up message box to indicate bad package
        // Make sure final result text indicates bad package (this takes care of silent/passive mode itself)
        ValidateSemantics::OutputErrorMessage(logger, ILogger::Result);
        // When the LocalizedDataLoggerDecorator gets destructed the Watson logger should no longer use it 
        WatsonData::WatsonDataStatic()->SetLogger(NullLogger::GetNullLogger());
        // Do not rethrow - nothing to handle it and the logger will be destroyed by now - just return the HR
    }

public:
    // Constructor for MainT
    MainT()
    {
        // Set the DebugIronMan environment variable if you want to attach the debugger
        WCHAR szDebugIronManFlag[256] = L"";
        INT iBytes = ::GetEnvironmentVariableW(L"DebugIronMan", szDebugIronManFlag, sizeof(szDebugIronManFlag)/sizeof(WCHAR)-1);
        if (iBytes>0 && !wcscmp(szDebugIronManFlag, L"1"))
        {
            ::MessageBox(NULL, L"Attach debugger now.\r\nSet DebugIronMan= to disable this.", L"Debug", MB_OK);; // If you break here set a breakpoint at the next statement and hit F5
        }


    }

    // Destructor for MainT
    virtual ~MainT()
    {
    }

    // MainT Run method
    HRESULT Run()
    {
        HRESULT hr = S_OK;
        BOOL fHasDHTMLHeader = FALSE;

        // Create the logger and open the log file - it will be renamed below once we know the package name
        CAutoPtr<IronMan::ILogger> spLogger(&NullLogger::GetNullLogger());

        IronMan::CCmdLineSwitches switches;

        // Ignore HRESULT - if this failed, we'll just assume we don't have a header and continue
        ModuleUtils::HasDHTMLLogHeader(&fHasDHTMLHeader);
        hr = IronMan::CDhtmlLoggerT<IronMan::TextLogger>::SetupStaticLoggingSettings(fHasDHTMLHeader);
        if (FAILED(hr))
        {
            return hr;
        }

        // Extract any vital engine data files at this point - not necessarily fatal if we can't find or extract embedded payload
        hr = ModuleUtils::ExtractEngineData<CCmdLineSwitches>(ModuleUtils::eplUxPayloads);

        // Make sure CAutoPtr doesn't try to destroy the static null logger which is not superceded in help mode
        DetachNullLogger handleNullLogger(switches, spLogger);

        // Don't create logger if this is /help or running in elevated mode.
        if (!handleNullLogger.UsingNullLogger())
        {
            spLogger.Detach();
            // Create the real logger
            spLogger = LogCreationUtils::CreateLogger(switches);
            // Set temporary name if one was not specified on the command line
            if (switches.GetLogFileSpecification().IsEmpty())
            {
                CString strLogFileNameWithoutExtension = LogUtils::GenerateLogFileName(L"Setup");
                spLogger->RenameLog(CPath(strLogFileNameWithoutExtension));
            }
        }

        WatsonLoggerDecorator logger(*spLogger);

        // Log the OS Version
        m_ccc.LogOSVersion(logger);
        // Log the command line switches and timezone information
        m_ccc.LogCommandLineAndTimeZone(logger);

        // Add the log file to the WastsonData, so if a crash occurs then the log file will be uploaded
        WatsonData::WatsonDataStatic()->GetFilesToKeep().Add(spLogger->GetFilePath());

        // Set the logger to the current logger so that if a crash occurs the log can be closed before it is uploaded
        WatsonData::WatsonDataStatic()->SetLogger(logger);

        // Run the engine and fire up the UI if appropriate
        hr = E_FAIL;
        try
        {
            hr = Run<RetryingDownloader, CompositePerformer, FileAuthenticity>(logger);
        }
        catch(const CInvalidXmlException& )
        {
            DisplayBadPackageMessage(logger);
            hr = CustomErrors::InternalErrorOrUserError;
            // Do not rethrow - nothing to handle it and the logger will be destroyed by now - just return the HR
        }
        catch(const CNotFoundException& )
        {
            DisplayBadPackageMessage(logger);
            hr = CustomErrors::InternalErrorOrUserError;
            // Do not rethrow - nothing to handle it and the logger will be destroyed by now - just return the HR
        }

        LogUtils::LogFinalResult(hr, logger, m_ccc.GetStartTime());
        
        // change HRESULTs back into Win32 errors, iff they are Win32 error.
        // Why would anyone want to do such a thing?  So we don't break some (supposedly) existing test apps.
        if (HRESULT_FACILITY(hr) == FACILITY_WIN32)
            hr = HRESULT_CODE(hr);
        return hr;
    }

    HRESULT SystemHasMsi31( const CString& packageName
                            , ILogger& logger)
    {
        if (GetProcAddress(GetModuleHandle(L"msi.dll"), "MsiSetExternalUIRecord"))
        {
            return S_OK;
        }

        LOG(logger, ILogger::Error, L"Windows Installer version is less than 3.1");

        m_ccc.GetUxLogger().RecordBlocker(UxEnum::bInternal, L"MSI31");

        //Upload
        IronMan::CCmdLineSwitches switches;
        HRESULT hrBlockers = CustomErrors::StopBlockerHitOrSystemRequirementNotMet;
        m_ccc.CompleteCCC(hrBlockers, !switches.InteractiveMode(), false, logger);
        return hrBlockers;
    }

    //------------------------------------------------------------------------------------------------
    // The run does the followings:
    //  1. Pre-load validation
    //  2. Load data files
    //  3. Load UI Resources
    //  4. Post load verification
    //  5. Check for any users error
    //  6. Process Blockers
    //  7. Execution
    //------------------------------------------------------------------------------------------------
    template <typename RetryingDownloader, typename CompositePerformer, typename FileAuthenticity>
    HRESULT Run(ILogger& logger)
    {
        // Initialize source location to the executable launch path
        SourceLocation::SetPath(ModuleUtils::GetDllPath());

        HRESULT hr = S_OK;
        BYTE* pbBuffer = NULL;
        SIZE_T cbBuffer = 0;
        IronMan::CCmdLineSwitches switches;
        IBurnUserExperience* pUserExperience = NULL;
        UXFactory uxFactory;

        {
        //------------------------------------------------------------------------------------------------
        //Step 1 Pre-load Validation 
        //
        // Note
        //======
        //1. We cannot upload user experience data data in this step since we have yet to process parameterinfo.xml
        //2. If validation fails in this step, we throw
        //------------------------------------------------------------------------------------------------
        // Ensure engine data file exists and is in UTF-16 format
        ValidateSemantics::IsFileUTF16WithBOM(ModuleUtils::GetParameterFilePath(L"ParameterInfo.xml"), logger);

         //Ensure that there is not #loc in UI/@Name or and BlockIf/@Id
        ValidateSemantics::VerifyTokensAreUsedCorrectlyInParameterInfoFile(ModuleUtils::GetParameterFilePath(L"ParameterInfo.xml"), logger);

        // Ensure that all the loc-ids in ParameterInfo.xml have localzed data in the LocalizedData.xml file.
        //This can throw a different exception. Let Watson handle it?
        LocalizedDataProvider localizedDataProvider(logger);
        if (false == localizedDataProvider.Validate())
        {
            //Throw since there is nothing much we can do.  Outer layer will handle this and put up appropriate 
            //message box.
            CInvalidXmlException ixe(L"Parameterinfo.xml.xml has a #Loc that is not defined in LocalizeData.xml ");
            throw ixe;
        }

        localizedDataProvider.ValidateLocalizedDataLanguageInResourceFolders();

        //------------------------------------------------------------------------------------------------
        //Step 2 Load Data files 
        // a. Load Parameterinfo.xml
        //------------------------------------------------------------------------------------------------

        if (vpEngineState)
        {
            hr = FileRead(&pbBuffer, &cbBuffer, ModuleUtils::GetParameterFilePath(L"ParameterInfo.xml"));
            ExitOnFailure(hr, "Failed to read manifest.");

            hr = ManifestLoadXmlFromBuffer(pbBuffer, cbBuffer, vpEngineState);
            ExitOnFailure(hr, "Failed to load manifest.");

            // set registration paths
            hr = RegistrationSetPaths(&vpEngineState->registration);
            ExitOnFailure(hr, "Failed to set registration paths.");

            // detect resume type
            hr = RegistrationDetectResumeType(&vpEngineState->registration, &vResumeType);
            ExitOnFailure(hr, "Failed to detect resume type.");

            if (BURN_RESUME_TYPE_NONE != vResumeType && BURN_RESUME_TYPE_INVALID != vResumeType)
            {
                cbBuffer = 0;

                hr = RegistrationLoadState(&vpEngineState->registration, &pbBuffer, &cbBuffer);
                if (FAILED(hr))
                {
                    TraceError(hr, "Failed to load engine state.");
                    vResumeType = BURN_RESUME_TYPE_INVALID;
                }
                else
                {
                    hr = CoreDeserializeEngineState(vpEngineState, pbBuffer, cbBuffer);
                    ExitOnFailure(hr, "Failed to deserialize engine state.");
                }
            }

            ReleaseNullBuffer(pbBuffer);
        }

        //------------------------------------------------------------------------------------------------
        //Step 2.5: If engine is running in server mode, start it.
        //------------------------------------------------------------------------------------------------
        
        if (switches.IsRunningServer())
        {
            hr = PipeRunBurnServer(&vpEngineState->variables);
            ExitOnFailure(hr, "Failed to run the server.");
            ExitFunction();
        }


        Operation uioOperation;
        const EngineData engineData(EngineData::CreateEngineData(ModuleUtils::GetParameterFilePath(L"ParameterInfo.xml"), localizedDataProvider.GetLocalizedData(), logger));

        //------------------------------------------------------------------------------------------------
        //Step 3: Refresh the Environment
        //------------------------------------------------------------------------------------------------
        //This is the earliest time we can set the header.  Need to set it asap in case we crash/error out now.
        LPWSTR sczValue = NULL;
        VariableFormatString(&vpEngineState->variables, L"[WatsonText]", &sczValue, NULL);
        WatsonData::WatsonDataStatic()->SetWatsonHeader(sczValue);
        ReleaseStr(sczValue);  //OK to release here since SetWatsonHeader is passed by Value.

        //Scope this so that dataToOperandTemp go out of scope.
        {
            //Creating dataToOperand here since EvaluateEnterMaintenanceModeIf() needs it.
            DataToOperand dataToOperandTemp(uioOperation.GetOperation());

            if (uioOperation.GetOperation() == Operation::uioInstalling) // only allow user modification if not already specified via cmd-line
            {
                if (engineData.EvaluateEnterMaintenanceModeIf(dataToOperandTemp))
                {
                    uioOperation = Operation::uioRepairing;
                }
            }
        }
        //MaintenanceModeIf may have updated the operation, create the latest state here.
        DataToOperand dataToOperand(uioOperation.GetOperation());

        PackageData packageData(engineData);

        // Initialize Ux
        // The Ux logger is initialize after the engine data is initialize so that we can determine if we should create the real UxLogger. 
        m_ccc.InitializeUxLogger(logger
                                , packageData
                                , IronMan::ThreadLanguageSelector::GetLangId()
                                , uioOperation.GetOperation()
                                , switches.QuietMode()
                                , switches.PassiveMode());

        //Log all the information needed.
        // Record the type of operation, e.g. installing, repairing, uninstalling
        m_ccc.LogOperationType(uioOperation.GetOperation(), logger);
        // Log the name and version of the package
        m_ccc.LogPackageNameAndVersion(engineData.GetUi(), logger);
        m_ccc.LogUserCollectionPolicy(engineData.GetConfiguration().GetUserExperienceDataCollectionData().GetPolicy(), logger);

        // Now that we have parsed engine data, generate log file name based on software update "package" name
        // if no specific log file was specified on the command line
        CString specifiedLogFile = switches.GetLogFileSpecification();
        CString specifiedLogFileNameOnly = ModuleUtils::GetFileNameOnlyWithoutExtension(specifiedLogFile);
        if (specifiedLogFile.IsEmpty() ||  specifiedLogFileNameOnly.IsEmpty() )
        {
            CPath before = logger.GetFilePath();
            before.RemoveFileSpec();

            // Get actual name from engine data which has "package" name
            CString strLogFileNameWithoutExtension = LogUtils::GenerateLogFileName(engineData.GetUi().GetName(), specifiedLogFileNameOnly.IsEmpty() ? specifiedLogFile : CString(L"%TEMP%\\") );
            CPath newLogFilePath;
            newLogFilePath.Combine(before,strLogFileNameWithoutExtension);
            // Rename the log file accordingly
            if (true == logger.RenameLog(newLogFilePath))
            {
                m_ccc.RenameLog(before, logger);
            }
        }

        //------------------------------------------------------------------------------------------------
        //Step 4: Post Load Verification
        //------------------------------------------------------------------------------------------------
         // This checks that the info in the ParameterInfo.XML file is matched by physical assets
        // so if a MSP/MSI/EXE/File does not have an URL, then the file must be in the package
        // this is done only during install or create layout, because a local only setup item like
        // an msp probably won't be cached
        if ( (uioOperation.GetOperation() == Operation::uioInstalling
              || uioOperation.GetOperation() == Operation::uioCreateLayout)
            && !ValidateSemantics::ParameterInfoSemanticChecker(engineData, logger) )
        {
            HRESULT hrVerification = E_FAIL;
            LogAndReportMetrics(hrVerification
                                    , L"PISemanticChecker"
                                    , true
                                    , logger);
            return hrVerification;
        }

        //------------------------------------------------------------------------------------------------
        //Step 5: Load UI Resources
        //------------------------------------------------------------------------------------------------
        // Data for HelpUsage
        CSimpleMap<CString, bool> mapSwitchStatus;
        HelpString helpString(engineData.GetConfiguration().GetDisabledCommandLineSwitches(),
                              engineData.GetConfiguration().GetAdditionalCommandLineSwitches(),
                              logger,
                              mapSwitchStatus /* returned */);

        //------------------------------------------------------------------------------------------------
        //Step 6: Handle end user errors
        //
        //------------------------------------------------------------------------------------------------
        // For each switch that should have an associated value, ensure one is being specified
        CString strOptionValueErrorInformation(L"Command-line option error: ");
        if (!switches.OptionValuesAreGood(strOptionValueErrorInformation))
        {
            hr = E_INVALIDARG;
            LogAndReportMetrics(hr
                            , L"InvalidArguments"
                            , false //Don't put up message box
                            , logger);
            LOG(logger, ILogger::Error, strOptionValueErrorInformation);
            //LogUtils::LogFinalResult(hr, logger, m_ccc.GetUxLogger().GetStartingTime());
            return hr;
        }

        //------------------------------------------------------------------------------------------------
        // Step 6: Blockers - both internal and external
        //------------------------------------------------------------------------------------------------
        // Ensure that this is the only instance of the package running
        if (!(switches.IsRunningElevated() || switches.IsRunningServer()))
        {
            OnlyOneInstance onlyOneInstance(engineData.GetConfiguration().GetBlockingMutex()
                                            , engineData.GetUi().GetName()
                                            , logger);
            hr = onlyOneInstance.CheckIfAnotherInstanceOfPackageIsRunning();
            if ( S_OK != hr )
            {
                if (ERROR_INSTALL_ALREADY_RUNNING == hr)
                {
                    m_ccc.GetUxLogger().RecordBlocker(UxEnum::bInternal, L"OneInstance");
                    HRESULT hrBlockers = CustomErrors::StopBlockerHitOrSystemRequirementNotMet;
                    m_ccc.CompleteCCC(hrBlockers, !switches.InteractiveMode(), false, logger);
                    return hrBlockers;
                }
                else
                {
                    LogAndReportMetrics(hr
                                    , L"OneInstance"
                                    , false //Don't put up message box
                                    , logger);
                    return hr;
                }
            }
        }
        else
        {
            LOG(logger, ILogger::Information, L"Elevated instance of the setup. Skipping OneInstance check.");
        }

        BlockChecker blockChecker(engineData.GetBlocks()
                                    , dataToOperand
                                    , engineData.GetUi().GetName()
                                    , logger
                                    , m_ccc.GetUxLogger());

        if (switches.CreateLayout().IsEmpty())
        {
            //Check for global Blocks before going any further
            
            //If the Operation is Uninstall or Repair and the UI is not in Silent or Passive Mode.
            //then the BlockChecker is called after the MaintenanceModeDialog instead of here
            //so that the Operands in the Blockers can know if we are Uninstalling or Repairing
            bool bInMaintenanceMode = (uioOperation.GetOperation() == Operation::uioRepairing) 
                                      || (uioOperation.GetOperation() == Operation::uioUninstalling);

            if (bInMaintenanceMode == false || !switches.InteractiveMode())
            {
                IBlockChecker::IResult& result = blockChecker.ProcessBlocks(); 
                
                bool bQuit = false;
                if (result.SuccessBlockerWasHit())
                {
                    hr = S_OK;
                    bQuit = true;
                }
                else if (result.StopBlockerWasHit(hr))
                {
                    bQuit = true;
                }
                else if (result.WarnBlockerWasHitAndUserCanceled())
                {
                    m_ccc.GetUxLogger().RecordCancelPage(L"Blocker");
                    bQuit = true;
                    hr = HRESULT_FROM_WIN32(ERROR_INSTALL_USEREXIT);
                }

                if (bQuit)
                {
                    m_ccc.CompleteCCC(hr, !switches.InteractiveMode(), false, logger);
                    return hr;
                }
            }
        }
        
        //------------------------------------------------------------------------------------------------
        // Step 7  Execution
        //------------------------------------------------------------------------------------------------
        // where do we download to?  Two possible places:  
        //   in /CreateLayout, it's where the user specifies;
        //   otherwise in a folder under %TEMP%
        // Calculate them both.
        CPath layoutFolder(switches.CreateLayout());
        CPath nonLayoutFolder;
        // If parameter file was embedded use extraction folder as root path for downloads and content search
        if (ModuleUtils::HasEmbeddedContent())
        {
            nonLayoutFolder = ModuleUtils::GetIncludedPayloadsFolderPath();
        }
        else
        {
            // by default, create a folder under %TEMP%, with the Name of the SP
            CString tempPath;
            ::GetTempPath(_MAX_DIR, tempPath.GetBuffer(_MAX_DIR));
            tempPath._ReleaseBuffer();
            tempPath += engineData.GetUi().GetName() + L"_" + engineData.GetUi().GetVersion();
            if ((0 != ::CreateDirectory(tempPath, NULL)) ||
                (::GetLastError() == ERROR_ALREADY_EXISTS))
            {
                nonLayoutFolder = CPath(tempPath);
            }
            else
            {
                nonLayoutFolder = ModuleUtils::GetDllPath();
            }
        }

        CString strSourceLocation(switches.GetSourceLocation());
        if (CString::StringLength(strSourceLocation) > 0)
        {
            SourceLocation::SetPath(strSourceLocation);
        }

        CacheManager cacheManager(engineData.BundleId(), logger);
        
        UberCoordinatorT<FileAuthenticity> coordinator(engineData
                                                        , blockChecker
                                                        , dataToOperand
                                                        , nonLayoutFolder
                                                        , layoutFolder
                                                        , uioOperation.GetOperation()
                                                        , logger
                                                        , m_ccc.GetUxLogger());
        
        CompositeDownloaderT<RetryingDownloader> downloader(    coordinator
                                                                , cacheManager
                                                                , engineData.GetItems().GetRetries()
                                                                , engineData.GetItems().GetDelay()
                                                                , engineData.GetItems().GetCopyPackageFilesFlag()
                                                                , logger, m_ccc.GetUxLogger());
        NotifyController controller1(downloader);

        ILogger* forked = logger.Fork(); // for parallel download and install

        ElevatedController elevatedController (switches.GetPipeName(), *forked);

        OperationData operationData(dataToOperand, packageData);
        CompositePerformer installer(uioOperation.GetOperation()
                                    , coordinator
                                    , cacheManager
                                    , operationData
                                    , coordinator.GetDataProviderToUi()
                                    , engineData.GetItems().GetFailureAction()
                                    , switches.IsRunningElevated()? elevatedController : *forked
                                    , m_ccc.GetUxLogger());
        CopyPerformer copyPerformer(layoutFolder, *forked);
        NotifyController controller2(switches.CreateLayout().IsEmpty() ? static_cast<IPerformer&>(installer) : static_cast<IPerformer&>(copyPerformer));

        // create pipe controllers (even if not used)
        MmioController pipe1(controller1, switches.GetPipeName(), true, logger);
        MmioController pipe2(controller2, switches.GetPipeName(), false, logger);        

        // Create array of controllers
        INotifyController* controllers[] = { &controller1, &controller2 };

        if (!switches.GetPipeName().IsEmpty() && !switches.IsRunningElevated())
        {
            controllers[0] = &pipe1;
            controllers[1] = &pipe2;
        }
        CompositeController composite(controllers, logger, forked);

        coordinator.SetEngineDataProvider(NULL); // hook up UI's data provider
        installer.SetEngineDataProvider(NULL); // hook up UI's data provider
        dataToOperand.SetEngineDataProvider(NULL); // hook up UI's data provider
        
        if (!switches.IsRunningElevated()) // Running elevated. No UI/UX required.
        {
            CPath pthUxBinaries(ModuleUtils::GetUxFolderPath());
            pthUxBinaries.Append(engineData.GetUi().GetDll());

            hr = uxFactory.Load((LPCWSTR)pthUxBinaries, &CBurnController::GetBurnCommand(coordinator.GetDataProviderToUi().GetOperation()), &pUserExperience);
            ExitOnFailure(hr, "Failed to load UX.");
        }

        // Set by an Setup Item if it return a 3010, needed in case another Setup Item return an Error
        bool bRebootPending = false;

        try 
        {
            if (switches.IsRunningElevated()) // Running elevated. No UI/UX required.
            {
                CNullBurnView nullUx;
                //CBurnView nullBurnView(nullUx, logger);
                coordinator.SetBurnView(&nullUx);
                coordinator.SetRunningElevated();

                hr = coordinator.Detect();
                ExitOnRootFailure(hr, "Detect failed.");

                hr = elevatedController.Run(installer, engineData, m_ccc.GetUxLogger());
            }
            else
            {
                // Unelevated process needs to run UI/UX.
                if (pUserExperience)
                {
                    // BurnUX is available and initialized, use it.
                    LOG(logger, ILogger::Result, L"Using BurnUX.dll");

                    CBurnView burnView(*pUserExperience, logger);
                    coordinator.SetBurnView(&burnView);
                    downloader.SetBurnView(&burnView);

                    CBurnController burnCore(coordinator.GetDataProviderToUi(), composite, burnView, logger);
                    coordinator.SetBurnCore(&burnCore);

                    hr = burnCore.RunViewUX();
                }
                else
                {
                    LOG(logger, ILogger::Error, L"Failed to find IBurnUserExperience in " + engineData.GetUi().GetDll());
                    hr = E_UNEXPECTED;
                }
            }
        }
        catch (CException& e)
        {
            wprintf(e.GetMessage());
            LOG(logger, ILogger::Error, e.GetMessage());
            throw;
        }
        catch(...)
        { // this code is here in order to make sure that we always merge the log files back together.
            logger.Merge(forked);
            logger.DeleteFork(forked);
            throw;
        }

        logger.Merge(forked);
        logger.DeleteFork(forked);

        // Recheck to get appropriate return code
        bool bStopOrSuccessBlockWasHit = blockChecker.GetResult().SuccessBlockerWasHit();
        if (false == bStopOrSuccessBlockWasHit)
        {
            HRESULT hrStopBlockerReturnCode = S_OK;
            if (blockChecker.GetResult().StopBlockerWasHit(hrStopBlockerReturnCode))
            {
                bStopOrSuccessBlockWasHit = true;
                hr = hrStopBlockerReturnCode;
            }
        }

        // Finish/tidy up Watson log and send if necessary
        m_ccc.CompleteCCC(hr
                        , !switches.InteractiveMode()
                        , !bStopOrSuccessBlockWasHit          //!pUiMode->GetDataProvider().StopBlockersWasHit()
                        , logger);

        Main::CheckForReboot(bRebootPending || (vpEngineState && vpEngineState->fReboot), switches, engineData.GetUi().GetName(), logger);
        }

    LExit:
        ReleaseBuffer(pbBuffer);
        ReleaseObject(pUserExperience);

        uxFactory.Unload();

        return hr;
    }

    static void CheckForReboot(bool bRebootPending, const CCmdLineSwitches& switches, const CString& packageName, ILogger& logger)
    {
        CheckForReboot<CCmdLineSwitches, RebootManager>(bRebootPending, switches, packageName, logger);        
    }

    template <typename CCmdLineSwitches, typename RebootManager>
    static void CheckForReboot(bool bRebootPending, const CCmdLineSwitches& switches, const CString& packageName, ILogger& logger)    
    {
        // Check for reboot
        if (bRebootPending) // 3010 ERROR_SUCCESS_REBOOT_REQUIRED
        {
            // common logic:  if the NoRestart switch is set, that overrides everything
            if (switches.NoRestartMode())
                return;

            LOG(logger, ILogger::Verbose, L"Rebooting now.");
            RebootManager rebootManager;
            rebootManager.Reboot();
        }
    }
};

typedef MainT<CrossCuttingConcerns> Main;

}

