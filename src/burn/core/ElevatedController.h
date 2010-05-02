//-------------------------------------------------------------------------------------------------
// <copyright file="ElevatedController.h" company="Microsoft">
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
#include "CompositePerformer.h"
#include "CacheManager.h"

namespace IronMan
{
class ElevatedController : public MmioChainee, public IProgressObserver, public ILogger
{
    class ElevatedView : public CNullBurnView
    {
        ElevatedController *m_pController;

    public:
        ElevatedView()
        {
            m_pController = NULL;
        }

        virtual int __stdcall OnError(
            __in LPCWSTR wzPackageId,
            __in DWORD dwCode,
            __in_z LPCWSTR wzError,
            __in DWORD dwUIHint
            )
        {
            if (m_pController)
            {
                return m_pController->OnError(wzPackageId, dwCode, wzError, dwUIHint);
            }

            return IDNOACTION;
        }

        virtual int __stdcall OnExecuteMsiMessage(
            __in_z LPCWSTR wzPackageId,
            __in INSTALLMESSAGE mt,
            __in UINT uiFlags,
            __in_z LPCWSTR wzMessage
            )
        {
            if (m_pController)
            {
                return m_pController->OnExecuteMsiMessage(wzPackageId, mt, uiFlags, wzMessage);
            }

            return IDNOACTION;
        }

        virtual int __stdcall OnExecuteMsiFilesInUse(
            __in_z LPCWSTR wzPackageId,
            __in DWORD cFiles,
            __in LPCWSTR* rgwzFiles
            )
        {
            if (m_pController)
            {
                return m_pController->OnExecuteMsiFilesInUse(wzPackageId, cFiles, rgwzFiles);
            }

            return IDNOACTION;
        }


        virtual void SetController(
            __in ElevatedController *pController
            )
        {
            m_pController = pController;
        }
    };

    HANDLE m_hChaineeEvent;
    ICompositePerformer* m_pElevatedCompositePerformer;
    HRESULT m_hrElevatedExecution;
    ElevatedView *m_pElevatedView;
    CString m_bundleId;
    CPath m_unElevatedLogFile;
    IPerformer* m_pPackagePerformer;

public: 
    ElevatedController(LPCWSTR sectionName, ILogger& logger)
        : MmioChainee(sectionName, logger)
        , m_pElevatedCompositePerformer(NULL)
        , m_hrElevatedExecution(S_OK)
        , m_pElevatedView(NULL)
        , m_bundleId(L"")
        , m_pPackagePerformer(NULL)
    {
    }

    ~ElevatedController()
    {
        delete m_pElevatedView;
    }

    // Listen and Delegate
    HRESULT Run(
        __in ICompositePerformer& performer,
        __in const EngineData& engineData,
        __in Ux& uxLogger
        )
    {
        HRESULT hr = S_OK;
        HANDLE hPipe = INVALID_HANDLE_VALUE;


        m_pElevatedCompositePerformer = &performer;
        m_pElevatedView = new ElevatedView();
        
        if (!m_pElevatedView)
        {
            hr = E_OUTOFMEMORY;
            ExitOnFailure(hr, "Failed to create elevated view.");
        }

        m_pElevatedView->SetController(this);
        m_bundleId = engineData.BundleId();

        // connect to per-user process
        hr = ElevationChildConnect(L"BurnPipe", L"{D3C7657C-4DF6-434a-8F91-AC0C565A074C}", &hPipe); // TODO: fix
        ExitOnFailure(hr, "Failed to connect to per-user process.");

        vpEngineState->hElevatedPipe = hPipe;

        // pump messages from per-user process
        hr = ElevationChildPumpMessages(hPipe, &vpEngineState->variables, &vpEngineState->registration, &vpEngineState->userExperience, uxLogger);
        ExitOnFailure(hr, "Failed to pump messages from per-user process.");
        
    LExit:
        return hr;
    }

public:
    //
    // ApplyPackage - Perform item in the elevated process.
    //
    HRESULT ApplyPackage(
        __in UINT operation,
        __in UINT action,
        __in UINT itemIndex,
        __in Ux& uxLogger
        )
    {
        HRESULT hr = S_OK;
        IPerformer* pPackagePerformer = NULL;

        PerformerCustomErrorHandler pcehErrorHandler(uxLogger, *this);
        const ItemBase* pItem = NULL;
        hr = m_pElevatedCompositePerformer->GetPackage(itemIndex, &pItem);
        ExitOnFailure(hr, "Failed to get package.");

        const CustomErrorHandling* pErrorHandler = dynamic_cast<const CustomErrorHandling*>(pItem);
        pcehErrorHandler.Initialize(pErrorHandler);

        hr = m_pElevatedCompositePerformer->CreatePackagePerformer(pItem, (ActionTable::Actions)action, m_pElevatedView, &pPackagePerformer);
        ExitOnFailure(hr, "Failed to create package performer.");
        m_pPackagePerformer = pPackagePerformer;

        bool bRetry = true;
        FirstError localError(*this);
        ResultObserver resultObserver(*this, hr);

        while (!IsAborted() && bRetry)
        {
            // m_hrElevatedExecution gets updated with result of execution in Finished() method.
            m_hrElevatedExecution = S_OK;
            pPackagePerformer->PerformAction(*this);
            localError.SetError(m_hrElevatedExecution);

            bRetry = pcehErrorHandler.Execute(localError, resultObserver);
            if (bRetry)
                localError.ClearError();
        }

        hr = localError.GetError();
        // SetElevatedPackageExecutionResults(hr);

    LExit:
        delete pPackagePerformer;
        m_pPackagePerformer = NULL;

        return hr;
    }

    // When UX resolves source for a package we need to update the location in the elevated process also.
    HRESULT UpdatePackageLocation(unsigned int nIndex, LPCWSTR location, bool fVerify)
    {
        CPath pth(location); 
        return m_pElevatedCompositePerformer->UpdatePackageLocation(nIndex, pth, fVerify);
    }

    // delegating IProgressObserver
    virtual int IProgressObserver::OnProgress(unsigned char soFar)
    {
        //BURN_ELEVATION_MESSAGE_TYPE_PROGRESSSOFAR
        HRESULT hr = S_OK;
        int nResult = IDABORT;
        DWORD dwResult = 0;
        HANDLE hPipe = vpEngineState->hElevatedPipe;
        
        hr = ElevationSendMessage(hPipe, BURN_ELEVATION_MESSAGE_TYPE_PROGRESSSOFAR, &soFar, sizeof(unsigned char), &dwResult);
        ExitOnFailure(hr, "Failed to send progress result message.");

        nResult = dwResult;

        hr = HRESULT_FROM_VIEW(nResult);
        if (FAILED(hr))
        {
            if (m_pPackagePerformer)
            {
                // Set abort flag of the current performer
                m_pPackagePerformer->Abort();
            }
        }

LExit:
        return nResult;
    }

    virtual int OnProgressDetail(unsigned char soFar)
    {
        return IDOK;
    }

    // Called from CompositePerformer when at the end of PerformAction
    virtual void Finished(HRESULT hr)
    {
        m_hrElevatedExecution = hr;
    }
    
    // The send the state change and detailed state change (item infomation) to the observer
    virtual void OnStateChange(IProgressObserver::State enumVal)
    {
        //MmioChainee::SetElevationMessageType(msgProgressOnStateChange);
        //MmioChainee::OnStateChange(enumVal);
    }

    virtual void OnStateChangeDetail(const State enumVal, const CString changeInfo)
    {
        //MmioChainee::SetElevationMessageType(msgProgressOnStateChagneDetail);
        //MmioChainee::OnStateChangeDetail(enumVal, changeInfo);
    }

    // Called when there is a reboot pending
    virtual void OnRebootPending()
    {
        //MmioChainee::OnRebootPending();
    }

    //ILogger Methods
    virtual void Log(LoggingLevel level, LPCWSTR szLogMessage) 
    {
        HRESULT hr = S_OK;
        HANDLE hPipe = vpEngineState->hElevatedPipe;
        BYTE* pbData = NULL;
        SIZE_T cbData = 0;

        hr = BuffWriteNumber(&pbData, &cbData, (DWORD)level);
        ExitOnFailure(hr, "Failed to write operation to message buffer.");

        hr = BuffWriteString(&pbData, &cbData, szLogMessage);
        ExitOnFailure(hr, "Failed to write variable value as string.");

        hr = ElevationPostMessage(hPipe, BURN_ELEVATION_MESSAGE_TYPE_LOG, pbData, cbData);
        ExitOnFailure(hr, "Failed to post result message.");

    LExit:
        ReleaseBuffer(pbData);
        return;
    }

    // Returns file path of the file log used in the unelevated process.
    // Called from the elevated process.
    virtual CPath GetFilePath() 
    {
        return m_unElevatedLogFile;
    }

    // Rest of the ILogger methods are stubbed out as they are not required to be processed
    // in the elevated process.
    virtual void PushSection(LPCWSTR strAction, LPCWSTR strDescription) {}
    virtual void PopSection(LPCWSTR str) {}

    virtual bool RenameLog(const CPath & pthNewName) {return true;}

    //Re-open the file handle and append to the end.  
    virtual void OpenForAppend() {throw E_UNEXPECTED;}

    //Close the file handle.  This is for main for Watson upload. 
    virtual void Close() {throw E_UNEXPECTED;}

    // LogAsIs
    virtual void BeginLogAsIs(LoggingLevel lvl, CString szLogStart) {throw E_UNEXPECTED;}
    virtual void LogLine(LPCWSTR szLine) {throw E_UNEXPECTED;}
    virtual void LogStartList() {throw E_UNEXPECTED;}
    virtual void LogListItem(LPCWSTR item) {throw E_UNEXPECTED;}
    virtual void LogEndList() {throw E_UNEXPECTED;}
    virtual void EndLogAsIs() {throw E_UNEXPECTED;}

    // fork and merge
    virtual ILogger* Fork() 
    { 
        throw E_UNEXPECTED;
        //return this; 
    }
    virtual void Merge(ILogger*) {throw E_UNEXPECTED;}
    virtual void DeleteFork(ILogger*) {throw E_UNEXPECTED;}
    virtual CPath GetForkedName() {throw E_UNEXPECTED; }
    virtual void LogFinalResult(LPCWSTR szLogMessage)  {throw E_UNEXPECTED;}  //Make it an abstract method so that we don't have to declare it everywhere.

private:
    virtual int __stdcall OnError(
        __in LPCWSTR wzPackageId,
        __in DWORD dwCode,
        __in_z LPCWSTR wzError,
        __in DWORD dwUIHint
        )
    {
        int nResult = IDOK;
        DWORD dwResult = 0;
        HRESULT hr = S_OK;
        HANDLE hPipe = vpEngineState->hElevatedPipe;
        BYTE* pbData = NULL;
        SIZE_T cbData = 0;

        // Package Id
        hr = BuffWriteString(&pbData, &cbData, wzPackageId);
        ExitOnFailure(hr, "Failed to write package id as string.");

        // Code
        hr = BuffWriteNumber(&pbData, &cbData, (DWORD)dwCode);
        ExitOnFailure(hr, "Failed to write error code to message buffer.");

        // Error
        hr = BuffWriteString(&pbData, &cbData, wzError);
        ExitOnFailure(hr, "Failed to write error value as string.");

        // Hint
        // Code
        hr = BuffWriteNumber(&pbData, &cbData, (DWORD)dwUIHint);
        ExitOnFailure(hr, "Failed to write UI hint to message buffer.");

        // Send message
        hr = ElevationSendMessage(hPipe, BURN_ELEVATION_MESSAGE_TYPE_ONERROR, pbData, cbData, &dwResult);
        ExitOnFailure(hr, "Failed to post OnError message.");
        nResult = (int)dwResult;

    LExit:
        ReleaseBuffer(pbData);
        return nResult;
    }

    virtual int __stdcall OnExecuteMsiMessage(
        __in_z LPCWSTR wzPackageId,
        __in INSTALLMESSAGE mt,
        __in UINT uiFlags,
        __in_z LPCWSTR wzMessage
        )
    {
        HRESULT hr = S_OK;
        DWORD er = ERROR_SUCCESS;
        DWORD dwResult = 0;
        BYTE* pbData = NULL;
        SIZE_T cbData = 0;
        LPWSTR scz = NULL;
        DWORD cch = 0;

        // package id
        hr = BuffWriteString(&pbData, &cbData, wzPackageId);
        ExitOnFailure(hr, "Failed to write package id as string.");

        // message id
        hr = BuffWriteNumber(&pbData, &cbData, mt);
        ExitOnFailure(hr, "Failed to copy message type into message.");

        // UI flags
        hr = BuffWriteNumber(&pbData, &cbData, uiFlags);
        ExitOnFailure(hr, "Failed to copy UI flags into message.");

        // message
        hr = BuffWriteString(&pbData, &cbData, wzMessage);
        ExitOnFailure(hr, "Failed to copy msi message into message.");

        // send message
        hr = ElevationSendMessage(vpEngineState->hElevatedPipe, BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_MSI_MESSAGE, pbData, cbData, &dwResult);
        ExitOnFailure(hr, "Failed to send BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_MSI_MESSAGE message.");

    LExit:
        ReleaseBuffer(pbData);
        ReleaseStr(scz);

        return (int)dwResult;
    }

    virtual int __stdcall OnExecuteMsiFilesInUse(
        __in_z LPCWSTR wzPackageId,
        __in DWORD cFiles,
        __in LPCWSTR* rgwzFiles
        )
    {
        HRESULT hr = S_OK;
        DWORD er = ERROR_SUCCESS;
        BYTE* pbData = NULL;
        SIZE_T cbData = 0;
        DWORD dwResult = 0;

        // write file name count
        hr = BuffWriteNumber(&pbData, &cbData, cFiles);
        ExitOnFailure(hr, "Failed to write file name count to message buffer.");

        for (DWORD i = 0; i < cFiles; ++i)
        {
            // write file name
            hr = BuffWriteString(&pbData, &cbData, rgwzFiles[i]);
            ExitOnFailure(hr, "Failed to write file name to message buffer.");
        }

        // send message
        hr = ElevationSendMessage(vpEngineState->hElevatedPipe, BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_MSI_FILES_IN_USE, pbData, cbData, &dwResult);
        ExitOnFailure(hr, "Failed to send BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_MSI_FILES_IN_USE message.");

    LExit:
        ReleaseBuffer(pbData);
        return (int)dwResult;
    }

HRESULT ElevationChildPumpMessages(
    __in HANDLE hPipe,
    __in BURN_VARIABLES* pVariables,
    __in BURN_REGISTRATION* pRegistration,
    __in BURN_USER_EXPERIENCE* pUserExperience,
    __in IronMan::Ux& uxLogger
    )
{
    HRESULT hr = S_OK;
    HRESULT hrResult = S_OK;
    BURN_ELEVATION_MESSAGE msg = { };

    hr = ElevationChildConnected(hPipe);
    ExitOnFailure(hr, "Failed to notify parent process that child is connected.");

    // Pump messages from parent process.
    while (S_OK == (hr = ElevationGetMessage(hPipe, &msg)))
    {
        // Process the message.
        switch (msg.dwMessage)
        {
        case BURN_ELEVATION_MESSAGE_TYPE_SESSION_BEGIN:
            hrResult = OnSessionBegin(pRegistration, pUserExperience, (BYTE*)msg.pvData, msg.cbData);
            break;

        case BURN_ELEVATION_MESSAGE_TYPE_SESSION_SUSPEND:
            hrResult = OnSessionSuspend(pRegistration, (BYTE*)msg.pvData, msg.cbData);
            break;

        case BURN_ELEVATION_MESSAGE_TYPE_SESSION_RESUME:
            hrResult = OnSessionResume(pRegistration, (BYTE*)msg.pvData, msg.cbData);
            break;

        case BURN_ELEVATION_MESSAGE_TYPE_SESSION_END:
            hrResult = OnSessionEnd(pRegistration, (BYTE*)msg.pvData, msg.cbData);
            break;

        case BURN_ELEVATION_MESSAGE_TYPE_SAVE_STATE:
            hrResult = OnSaveState(pRegistration, (BYTE*)msg.pvData, msg.cbData);
            break;

        case BURN_ELEVATION_MESSAGE_TYPE_CACHE_UNELEVATED_LOG_FILE:
            hrResult = OnSaveUnElevatedLogFile((BYTE*)msg.pvData, msg.cbData);
            break;

        case BURN_ELEVATION_MESSAGE_TYPE_EXECUTE:
            hrResult = OnExecutePackage(pVariables, (BYTE*)msg.pvData, msg.cbData, uxLogger);
            break;

        case BURN_ELEVATION_MESSAGE_TYPE_ISCACHED:
            hrResult = OnIsCached((BYTE*)msg.pvData, msg.cbData);
            break;

        case BURN_ELEVATION_MESSAGE_TYPE_VERIFYANDCACHE:
            hrResult = OnVerifyAndCachePackage((BYTE*)msg.pvData, msg.cbData);
            break;

        case BURN_ELEVATION_MESSAGE_TYPE_DELETECACHEDPACKAGE:
            hrResult = OnDeleteCachedPackage((BYTE*)msg.pvData, msg.cbData);
            break;

        case BURN_ELEVATION_MESSAGE_TYPE_UPDATEPACKAGELOCATION:
            hrResult = OnUpdatePackageLocation((BYTE*)msg.pvData, msg.cbData);
            break;

        case BURN_ELEVATION_MESSAGE_TYPE_DELETETEMPCACHEDDIRECTORY:
            hrResult = OnDeleteTemporaryCacheDirectory();
            break;

        case BURN_ELEVATION_MESSAGE_TYPE_TERMINATE:
            ExitFunction1(hr = S_OK);

        default:
            hr = E_INVALIDARG;
            ExitOnRootFailure1(hr, "Unexpected elevated message sent to child process, msg: %u", msg.dwMessage);
        }

        // post result message
        hr = ElevationPostMessage(hPipe, BURN_ELEVATION_MESSAGE_TYPE_COMPLETE, &hrResult, sizeof(hrResult));
        ExitOnFailure(hr, "Failed to post result message.");

        ElevationMessageUninitialize(&msg);
    }

    if (S_FALSE == hr)
    {
        hr = S_OK;
    }

LExit:
    ElevationMessageUninitialize(&msg);

    return hr;
}

// TODO: move to elevation.cpp

static HRESULT OnSessionBegin(
    __in BURN_REGISTRATION* pRegistration,
    __in BURN_USER_EXPERIENCE* pUserExperience,
    __in BYTE* pbData,
    __in DWORD cbData
    )
{
    HRESULT hr = S_OK;
    SIZE_T iData = 0;
    DWORD action = 0;
    DWORD64 qwEstimatedSize = 0;

    // deserialize message data
    hr = BuffReadNumber(pbData, cbData, &iData, &action);
    ExitOnFailure(hr, "Failed to read action.");

    hr = BuffReadNumber64(pbData, cbData, &iData, &qwEstimatedSize);
    ExitOnFailure(hr, "Failed to read estimated size.");

    // begin session in per-machine process
    hr =  RegistrationSessionBegin(pRegistration, pUserExperience, (BURN_ACTION)action, qwEstimatedSize, TRUE);
    ExitOnFailure(hr, "Failed to begin registration session.");

LExit:
    return hr;
}

static HRESULT OnSessionSuspend(
    __in BURN_REGISTRATION* pRegistration,
    __in BYTE* pbData,
    __in DWORD cbData
    )
{
    HRESULT hr = S_OK;
    SIZE_T iData = 0;
    DWORD action = 0;
    DWORD fReboot = 0;

    // deserialize message data
    hr = BuffReadNumber(pbData, cbData, &iData, &action);
    ExitOnFailure(hr, "Failed to read action.");

    hr = BuffReadNumber(pbData, cbData, &iData, &fReboot);
    ExitOnFailure(hr, "Failed to read reboot flag.");

    // suspend session in per-machine process
    hr = RegistrationSessionSuspend(pRegistration, (BURN_ACTION)action, (BOOL)fReboot, TRUE);
    ExitOnFailure(hr, "Failed to suspend registration session.");

LExit:
    return hr;
}

static HRESULT OnSessionResume(
    __in BURN_REGISTRATION* pRegistration,
    __in BYTE* pbData,
    __in DWORD cbData
    )
{
    HRESULT hr = S_OK;
    SIZE_T iData = 0;
    DWORD action = 0;

    // deserialize message data
    hr = BuffReadNumber(pbData, cbData, &iData, &action);
    ExitOnFailure(hr, "Failed to read action.");

    // suspend session in per-machine process
    hr = RegistrationSessionResume(pRegistration, (BURN_ACTION)action, TRUE);
    ExitOnFailure(hr, "Failed to suspend registration session.");

LExit:
    return hr;
}

static HRESULT OnSessionEnd(
    __in BURN_REGISTRATION* pRegistration,
    __in BYTE* pbData,
    __in DWORD cbData
    )
{
    HRESULT hr = S_OK;
    SIZE_T iData = 0;
    DWORD action = 0;
    DWORD fRollback = 0;

    // deserialize message data
    hr = BuffReadNumber(pbData, cbData, &iData, &action);
    ExitOnFailure(hr, "Failed to read action.");

    hr = BuffReadNumber(pbData, cbData, &iData, &fRollback);
    ExitOnFailure(hr, "Failed to read rollback flag.");

    // suspend session in per-machine process
    hr = RegistrationSessionEnd(pRegistration, (BURN_ACTION)action, (BOOL)fRollback, TRUE);
    ExitOnFailure(hr, "Failed to suspend registration session.");

LExit:
    return hr;
}

static HRESULT OnSaveState(
    __in BURN_REGISTRATION* pRegistration,
    __in BYTE* pbData,
    __in DWORD cbData
    )
{
    HRESULT hr = S_OK;

    // save state in per-machine process
    hr = RegistrationSaveState(pRegistration, pbData, cbData);
    ExitOnFailure(hr, "Failed to save state.");

LExit:
    return hr;
}

HRESULT OnExecutePackage(
    __in BURN_VARIABLES* pVariables,
    __in BYTE* pbData,
    __in DWORD cbData,
    __in Ux& uxLogger
    )
{
    HRESULT hr = S_OK;
    SIZE_T iData = 0;
    DWORD packageIndex;
    DWORD operation;
    DWORD action;

    // deserialize message data
    hr = VariableDeserialize(pVariables, pbData, cbData, &iData);
    ExitOnFailure(hr, "Failed to read variables.");

    hr = BuffReadNumber(pbData, cbData, &iData, &packageIndex);
    ExitOnFailure(hr, "Failed to read action.");

    hr = BuffReadNumber(pbData, cbData, &iData, &operation);
    ExitOnFailure(hr, "Failed to read action.");

    hr = BuffReadNumber(pbData, cbData, &iData, &action);
    ExitOnFailure(hr, "Failed to read action.");

    // execute package
    hr = ApplyPackage(operation, action, packageIndex, uxLogger);
    ExitOnFailure(hr, "Failed to execute package.");

LExit:
    return hr;
}

//
// When UX resolves source for a package we need to update the location in the elevated process also.
HRESULT OnUpdatePackageLocation(
    __in BYTE* pbData,
    __in DWORD cbData
    )
{
    HRESULT hr = S_OK;
    SIZE_T iData = 0;
    DWORD packageIndex;
    LPWSTR szpath = NULL;

    // deserialize message data

    hr = BuffReadNumber(pbData, cbData, &iData, &packageIndex);
    ExitOnFailure(hr, "Failed to read Package Index.");

    hr = BuffReadString(pbData, cbData, &iData, &szpath);
    ExitOnFailure(hr, "Failed to read Package Location.");

    // Update Package Location
    hr = UpdatePackageLocation(packageIndex, szpath, true);

LExit:
    ReleaseStr(szpath);
    return hr;
}

//------------------------------------------------------------------------------
// OnDeleteCachedPackage
// Attempt to delete cached package.  If cannot delete, then move it to the temp directory
// Also deletes the directory that the cached package is in
//------------------------------------------------------------------------------
HRESULT OnDeleteCachedPackage(
    __in BYTE* pbData,
    __in DWORD cbData
    )
{
    HRESULT hr = S_OK;
    SIZE_T iData = 0;
    DWORD packageIndex;
    LPWSTR wzFile = NULL;

    // deserialize message data
    hr = BuffReadString(pbData, cbData, &iData, &wzFile);
    ExitOnFailure(hr, "Failed to read Package Location.");

    // Update Package Location
    // Delete cached package and directory
    hr = CacheManager::DeleteCachedPackage(wzFile, true, NULL, *this);
    ExitOnFailure(hr, "Failed to delete cached package.");

LExit:
    ReleaseStr(wzFile);
    return hr;
}

//------------------------------------------------------------------------------
// OnSaveUnElevatedLogFile
//------------------------------------------------------------------------------
HRESULT OnSaveUnElevatedLogFile(
    __in BYTE* pbData,
    __in DWORD cbData
    )
{
    HRESULT hr = S_OK;
    SIZE_T iData = 0;
    LPWSTR pwzLogFile=NULL;

    // deserialize message data
    hr = BuffReadString(pbData, cbData, &iData, &pwzLogFile);
    ExitOnFailure(hr, "Failed to read UnElevated log file path.");

    m_unElevatedLogFile = pwzLogFile;

LExit:
    return hr;
}

    //------------------------------------------------------------------------------
    // OnIsCached
    // In elevated process determines if an package is already cached
    // HRESULT S_OK indicates the package is already cached, S_FALSE if not
    //------------------------------------------------------------------------------
HRESULT OnIsCached(
    __in BYTE* pbData,
    __in DWORD cbData
    )
{
    HRESULT hr = S_OK;
    bool bIsCached = false;
    SIZE_T iData = 0;
    DWORD nPackageIndex;

    // deserialize message data
    hr = BuffReadNumber(pbData, cbData, &iData, &nPackageIndex);
    ExitOnFailure(hr, "Failed to read Package Index.");

    // Is Cached?
    bIsCached = m_pElevatedCompositePerformer->IsCached(nPackageIndex);

LExit:
    return (SUCCEEDED(hr) ? (bIsCached ? S_OK : S_FALSE) : hr);
}

//------------------------------------------------------------------------------
// OnVerifyAndCachePackage
// In the elevated process, verify a package and cache it
//------------------------------------------------------------------------------
HRESULT OnVerifyAndCachePackage(
    __in BYTE* pbData,
    __in DWORD cbData
    )
{
    HRESULT hr = S_OK;
    SIZE_T iData = 0;
    DWORD nPackageIndex;

    // deserialize message data
    hr = BuffReadNumber(pbData, cbData, &iData, &nPackageIndex);
    ExitOnFailure(hr, "Failed to read Package Index.");

    LPWSTR location = NULL;
    hr = BuffReadString(pbData, cbData, &iData, &location);
    ExitOnFailure(hr, "Failed to read package location.");

    hr = UpdatePackageLocation(nPackageIndex, location, false);
    ExitOnFailure(hr, "Failed to update package location.");

    hr = m_pElevatedCompositePerformer->VerifyAndCachePackage(nPackageIndex);
    ExitOnFailure(hr, "Failed to verify and cache package.");

LExit:
    return hr;
}

//------------------------------------------------------------------------------
// OnDeleteTemporaryCacheDirectory
// In the elevated process, delete the temporary package cache
//------------------------------------------------------------------------------
HRESULT OnDeleteTemporaryCacheDirectory()
{
    HRESULT hr = S_OK;
    LPWSTR sczCompletedPath = NULL;

    hr = PathGetKnownFolder(CSIDL_COMMON_APPDATA, &sczCompletedPath);
    ExitOnFailure(hr, "Failed to find local per-machine appdata directory.");

    hr = StrAllocConcat(&sczCompletedPath,  L"Apps\\Cache\\temp\\", 0);
    ExitOnFailure1(hr, "Failed to concat Apps\\Cache\\temp to string: %S", sczCompletedPath);

    if (!m_bundleId.IsEmpty())
    {
        hr = StrAllocConcat(&sczCompletedPath, m_bundleId, 0);
        ExitOnFailure1(hr, "Failed to concat bundle Id to string: %S", sczCompletedPath);

        hr = StrAllocConcat(&sczCompletedPath, L"\\", 0);
        ExitOnFailure1(hr, "Failed to concat \\ to string: %S", sczCompletedPath);

        // Delete temp cache directory
        hr = DirEnsureDelete(sczCompletedPath, TRUE, TRUE);
        ExitOnFailure(hr, "Failed to delete cached package.");
    }

LExit:
    ReleaseStr(sczCompletedPath);
    return hr;
}

};
}

