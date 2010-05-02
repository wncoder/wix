//-------------------------------------------------------------------------------------------------
// <copyright file="UnElevatedController.h" company="Microsoft">
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
// Class: UnElevatedController
//
// This class manages communication and synchronization in the unelevated process.
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

namespace IronMan
{
class UnElevatedController : public IronMan::IProgressObserver
{
public:
    IProgressObserver& m_unElevatedObserver;
    ILogger& m_logger;
    IBurnView *m_pBurnView;

public:
    // Mmio chainer will create section by given name.
    // Event is also created by the Mmio chainer and name is saved in the
    // mapped file data strcuture.
    UnElevatedController(
        __in IProgressObserver& unElevatedObserver, 
        __in ILogger& logger,
        __in IBurnView *pBurnView
        ) 
        : m_unElevatedObserver(unElevatedObserver) 
        , m_logger(logger)
        , m_pBurnView(pBurnView)
    {}

    ~UnElevatedController()
    {
    }

    HRESULT SaveUnElevatedLogFile(void)
    {
        HANDLE hPipe = vpEngineState->hElevatedPipe;
        BYTE* pbData = NULL;
        SIZE_T cbData = 0;
        HRESULT hr = S_OK;

        // serialize message data
        hr = BuffWriteString(&pbData, &cbData, (LPCWSTR)m_logger.GetFilePath());
        ExitOnFailure(hr, "Failed to write log file path to message buffer.");

        // send message
        DWORD dwResult = -1;
        hr = ElevationSendMessage(hPipe, BURN_ELEVATION_MESSAGE_TYPE_CACHE_UNELEVATED_LOG_FILE, pbData, cbData, &dwResult);
        ExitOnFailure(hr, "Failed to send BURN_ELEVATION_MESSAGE_TYPE_CACHE_UNELEVATED_LOG_FILE message to per-machine process.");

        hr = dwResult;
        m_unElevatedObserver.Finished(hr);

LExit:
        ReleaseBuffer(pbData);
        return hr;
    }

    //------------------------------------------------------------------------------
    // IsCached
    // Calls into the elevated process to determine if an package is already cached
    // HRESULT S_OK indicates the package is already cached
    //------------------------------------------------------------------------------
    HRESULT IsCached(const UINT nPackageIndex)
    {
        HANDLE hPipe = vpEngineState->hElevatedPipe;
        BYTE* pbData = NULL;
        SIZE_T cbData = 0;
        HRESULT hr = S_OK;

        // serialize message data
        hr = BuffWriteNumber(&pbData, &cbData, (DWORD)nPackageIndex);
        ExitOnFailure(hr, "Failed to write package index to message buffer.");

        // send message
        DWORD dwResult = -1;
        hr = ElevationSendMessage(hPipe, BURN_ELEVATION_MESSAGE_TYPE_ISCACHED, pbData, cbData, &dwResult);
        ExitOnFailure(hr, "Failed to send BURN_ELEVATION_MESSAGE_TYPE_ISCACHED message to per-machine process.");

        hr = dwResult;
        m_unElevatedObserver.Finished(hr);

LExit:
        ReleaseBuffer(pbData);
        return hr;
    }

    //------------------------------------------------------------------------------
    // UpdatePackageLocation
    // Calls into the elevated process to update the location of the package
    //------------------------------------------------------------------------------
    HRESULT UpdatePackageLocation(UINT packageIndex, LPCTSTR szpath)
    {
        HANDLE hPipe = vpEngineState->hElevatedPipe;
        BYTE* pbData = NULL;
        SIZE_T cbData = 0;
        BURN_ELEVATION_MESSAGE msg = { };
        HRESULT hr = S_OK;
        LPWSTR sczMessage = NULL;
        DWORD dwLogLevel = 0;
        SIZE_T iData = 0;

        // serialize message data
        hr = BuffWriteNumber(&pbData, &cbData, (DWORD)packageIndex);
        ExitOnFailure(hr, "Failed to write package index to message buffer.");

        hr = BuffWriteString(&pbData, &cbData, szpath);
        ExitOnFailure(hr, "Failed to write Package Location to message buffer.");

        // send message
        hr = ElevationPostMessage(hPipe, BURN_ELEVATION_MESSAGE_TYPE_UPDATEPACKAGELOCATION, pbData, cbData);
        ExitOnFailure(hr, "Failed to send BURN_ELEVATION_MESSAGE_TYPE_UPDATEPACKAGELOCATION message to per-machine process.");

        // pump messages from per-machine process
        while (S_OK == (hr = ElevationGetMessage(hPipe, &msg)))
        {
            // Process the message.
            switch (msg.dwMessage)
            {
            case BURN_ELEVATION_MESSAGE_TYPE_LOG:
                iData = 0;

                hr = BuffReadNumber((BYTE*)msg.pvData, msg.cbData, &iData, &dwLogLevel);
                ExitOnFailure(hr, "Failed to read log level.");

                hr = BuffReadString((BYTE*)msg.pvData, msg.cbData, &iData, &sczMessage);
                ExitOnFailure(hr, "Failed to read log message.");

                m_logger.Log((IronMan::ILogger::LoggingLevel)dwLogLevel, sczMessage);
                break;

            case BURN_ELEVATION_MESSAGE_TYPE_COMPLETE:
                if (!msg.pvData || sizeof(HRESULT) != msg.cbData)
                {
                    hr = E_INVALIDARG;
                    ExitOnFailure(hr, "Invalid data for complete message.");
                }
                hr = *(HRESULT*)msg.pvData;
                ExitFunction();
                break;

            default:
                hr = E_INVALIDARG;
                ExitOnRootFailure1(hr, "Unexpected elevated message sent to child process, msg: %u", msg.dwMessage);
            }

            ElevationMessageUninitialize(&msg);
        }

        if (S_FALSE == hr)
        {
            hr = S_OK;
        }

LExit:
        ReleaseBuffer(pbData);
        ReleaseStr(sczMessage);
        ElevationMessageUninitialize(&msg);
        return hr;
    }

    //------------------------------------------------------------------------------
    // VerifyAndCachePackage
    // Calls into the elevated process to verify a package and cache it
    //------------------------------------------------------------------------------
    HRESULT VerifyAndCachePackage(const UINT nPackageIndex, LPCWSTR pwzUpdatedPackageLocation)
    {
        HANDLE hPipe = vpEngineState->hElevatedPipe;
        BYTE* pbData = NULL;
        SIZE_T cbData = 0;
        BURN_ELEVATION_MESSAGE msg = { };
        HRESULT hr = S_OK;
        LPWSTR sczMessage = NULL;
        DWORD dwLogLevel = 0;
        SIZE_T iData = 0;

        // serialize message data
        hr = BuffWriteNumber(&pbData, &cbData, (DWORD)nPackageIndex);
        ExitOnFailure(hr, "Failed to write package index to message buffer.");

        hr = BuffWriteString(&pbData, &cbData, (LPCWSTR)pwzUpdatedPackageLocation);
        ExitOnFailure(hr, "Failed to write package location.");

        // send message
        hr = ElevationPostMessage(hPipe, BURN_ELEVATION_MESSAGE_TYPE_VERIFYANDCACHE, pbData, cbData);
        ExitOnFailure(hr, "Failed to send BURN_ELEVATION_MESSAGE_TYPE_VERIFYANDCACHE message to per-machine process.");

        // pump messages from per-machine process
        while (S_OK == (hr = ElevationGetMessage(hPipe, &msg)))
        {
            // Process the message.
            switch (msg.dwMessage)
            {
            case BURN_ELEVATION_MESSAGE_TYPE_LOG:
                iData = 0;

                hr = BuffReadNumber((BYTE*)msg.pvData, msg.cbData, &iData, &dwLogLevel);
                ExitOnFailure(hr, "Failed to read log level.");

                hr = BuffReadString((BYTE*)msg.pvData, msg.cbData, &iData, &sczMessage);
                ExitOnFailure(hr, "Failed to read log message.");

                m_logger.Log((IronMan::ILogger::LoggingLevel)dwLogLevel, sczMessage);
                break;

            case BURN_ELEVATION_MESSAGE_TYPE_COMPLETE:
                if (!msg.pvData || sizeof(HRESULT) != msg.cbData)
                {
                    hr = E_INVALIDARG;
                    ExitOnFailure(hr, "Invalid data for complete message.");
                }
                hr = *(HRESULT*)msg.pvData;
                ExitFunction();
                break;

            default:
                hr = E_INVALIDARG;
                ExitOnRootFailure1(hr, "Unexpected elevated message sent to child process, msg: %u", msg.dwMessage);
            }

            ElevationMessageUninitialize(&msg);
        }

        if (S_FALSE == hr)
        {
            hr = S_OK;
        }

LExit:
        ReleaseBuffer(pbData);
        ReleaseStr(sczMessage);
        ElevationMessageUninitialize(&msg);
        return hr;
    }

    //------------------------------------------------------------------------------
    // DeleteCachedPackage
    // Attempt to delete cached package.  If cannot delete, then move it to the temp directory
    // Also deletes the directory that the cached package is in
    //------------------------------------------------------------------------------
    HRESULT DeleteCachedPackage(LPCTSTR wzFile)
    {
        HANDLE hPipe = vpEngineState->hElevatedPipe;
        BYTE* pbData = NULL;
        SIZE_T cbData = 0;
        HRESULT hr = S_OK;
        DWORD dwResult = 0;

        // serialize message data
        hr = BuffWriteString(&pbData, &cbData, (LPCWSTR)wzFile);
        ExitOnFailure(hr, "Failed to write cache package location.");

        // send message
        hr = ElevationSendMessage(hPipe, BURN_ELEVATION_MESSAGE_TYPE_DELETECACHEDPACKAGE, pbData, cbData, &dwResult);
        ExitOnFailure(hr, "Failed to send BURN_ELEVATION_MESSAGE_TYPE_DELETECACHEDPACKAGE message.");
        hr = (HRESULT)dwResult;

LExit:
        ReleaseBuffer(pbData);
        return hr;
    }

    //------------------------------------------------------------------------------
    // DeleteTemporaryCacheDirectory
    // Calls into the elevated process to delete the PerMachine temp cache dir
    //------------------------------------------------------------------------------
    HRESULT DeleteTemporaryCacheDirectory()
    {
        HANDLE hPipe = vpEngineState->hElevatedPipe;
        BYTE* pbData = NULL;
        SIZE_T cbData = 0;
        HRESULT hr = S_OK;

        // send message
        DWORD dwResult = -1;
        hr = ElevationSendMessage(hPipe, BURN_ELEVATION_MESSAGE_TYPE_DELETETEMPCACHEDDIRECTORY, pbData, cbData, &dwResult);
        ExitOnFailure(hr, "Failed to send BURN_ELEVATION_MESSAGE_TYPE_DELETETEMPCACHEDDIRECTORY message to per-machine process.");

        hr = dwResult;
        m_unElevatedObserver.Finished(hr);

LExit:
        ReleaseBuffer(pbData);
        return hr;
    }

    //
    // ApplyPackage - Brokers applying given package in the elevated process. 
    //                           Called for each package that needs to run in the elevated package. 
    // 
    void ApplyPackage(UINT packageIndex, UINT operation, UINT action, HRESULT& hr, BOOL &fAbort)
    {
        HANDLE hPipe = vpEngineState->hElevatedPipe;
        BURN_VARIABLES* pVariables = &vpEngineState->variables;
        BYTE* pbData = NULL;
        SIZE_T cbData = 0;
        DWORD dwResult = 0;
        BURN_ELEVATION_MESSAGE msg = { };
        SIZE_T iData = 0;
        int nResult = IDOK;
        DWORD dwLogLevel = 0;
        LPWSTR sczMessage = NULL;

        // serialize message data
        hr = VariableSerialize(pVariables, &pbData, &cbData);
        ExitOnFailure(hr, "Failed to write variables.");

        hr = BuffWriteNumber(&pbData, &cbData, (DWORD)packageIndex);
        ExitOnFailure(hr, "Failed to write package index to message buffer.");

        hr = BuffWriteNumber(&pbData, &cbData, (DWORD)operation);
        ExitOnFailure(hr, "Failed to write operation to message buffer.");

        hr = BuffWriteNumber(&pbData, &cbData, (DWORD)action);
        ExitOnFailure(hr, "Failed to write action to message buffer.");

        // send message
        hr = ElevationPostMessage(hPipe, BURN_ELEVATION_MESSAGE_TYPE_EXECUTE, pbData, cbData);
        ExitOnFailure(hr, "Failed to send BURN_ELEVATION_MESSAGE_TYPE_EXECUTE message to per-machine process.");

        // pump messages from per-machine process
        while (S_OK == (hr = ElevationGetMessage(hPipe, &msg)))
        {
            // Process the message.
            switch (msg.dwMessage)
            {
            case BURN_ELEVATION_MESSAGE_TYPE_PROGRESSSOFAR:
                if (!msg.pvData || sizeof(unsigned char) != msg.cbData)
                {
                    hr = E_INVALIDARG;
                    ExitOnFailure(hr, "Invalid data for progress so far message.");
                }
                nResult = m_unElevatedObserver.OnProgress(*(unsigned char*)msg.pvData);
                hr = ElevationPostMessage(hPipe, BURN_ELEVATION_MESSAGE_TYPE_COMPLETE, &nResult, sizeof(nResult));
                ExitOnFailure(hr, "Failed to send OnProgress result to elevated process");

                hr = HRESULT_FROM_VIEW(nResult);
                if (FAILED(hr))
                {
                    fAbort = TRUE;
                    m_logger.Log(IronMan::ILogger::Error, L"User interface commanded engine to abort");
                }
                break;

            case BURN_ELEVATION_MESSAGE_TYPE_PROGRESSONSTATECHANGE:
                if (!msg.pvData || sizeof(IronMan::IProgressObserver::State) != msg.cbData)
                {
                    hr = E_INVALIDARG;
                    ExitOnFailure(hr, "Invalid data for progress state change message.");
                }
                m_unElevatedObserver.OnStateChange(*(IronMan::IProgressObserver::State*)msg.pvData);
                break;

            case BURN_ELEVATION_MESSAGE_TYPE_LOG:
                iData = 0;

                hr = BuffReadNumber((BYTE*)msg.pvData, msg.cbData, &iData, &dwLogLevel);
                ExitOnFailure(hr, "Failed to read log level.");

                hr = BuffReadString((BYTE*)msg.pvData, msg.cbData, &iData, &sczMessage);
                ExitOnFailure(hr, "Failed to read log level.");

                m_logger.Log((IronMan::ILogger::LoggingLevel)dwLogLevel, sczMessage);
                break;

            case BURN_ELEVATION_MESSAGE_TYPE_COMPLETE:
                if (!msg.pvData || sizeof(HRESULT) != msg.cbData)
                {
                    hr = E_INVALIDARG;
                    ExitOnFailure(hr, "Invalid data for complete message.");
                }
                hr = *(HRESULT*)msg.pvData;
                m_unElevatedObserver.Finished(hr);
                ExitFunction();
                break;

            case BURN_ELEVATION_MESSAGE_TYPE_ONERROR:
                nResult = ProcessOnError(msg.pvData, msg.cbData);
                hr = ElevationPostMessage(hPipe, BURN_ELEVATION_MESSAGE_TYPE_COMPLETE, &nResult, sizeof(nResult));
                ExitOnFailure(hr, "Failed to send OnError result to elevated process.");
                break;

            case BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_MSI_MESSAGE:
                nResult = OnExecuteMsiMessage(msg.pvData, msg.cbData);

                hr = ElevationPostMessage(hPipe, BURN_ELEVATION_MESSAGE_TYPE_COMPLETE, &nResult, sizeof(nResult));
                ExitOnFailure(hr, "Failed to post BURN_ELEVATION_MESSAGE_TYPE_COMPLETE result to elevated process.");
                break;

            case BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_MSI_FILES_IN_USE:
                nResult = OnExecuteMsiFilesInUse(msg.pvData, msg.cbData);

                hr = ElevationPostMessage(hPipe, BURN_ELEVATION_MESSAGE_TYPE_COMPLETE, &nResult, sizeof(nResult));
                ExitOnFailure(hr, "Failed to send OnExecuteMsiFilesInUse result to elevated process.");
                break;

            default:
                hr = E_INVALIDARG;
                ExitOnRootFailure1(hr, "Unexpected elevated message sent to child process, msg: %u", msg.dwMessage);
            }

            ElevationMessageUninitialize(&msg);
        }

        if (S_FALSE == hr)
        {
            hr = S_OK;
        }
        

        //hr = (HRESULT)dwResult;

LExit:
        ReleaseBuffer(pbData);
        ReleaseStr(sczMessage);
        ElevationMessageUninitialize(&msg);

    } // DoPerformActionInElevatedProcess

private: 
    
    int ProcessOnError(
        __in LPVOID pvData,
        __in DWORD cbData)
    {
        HRESULT hr = S_OK;
        SIZE_T iData = 0;
        int nResult = IDOK;
        DWORD dwError = 0;
        DWORD dwUIFlags = 0;
        LPWSTR sczMessage = NULL;
        LPWSTR sczPackageId = NULL;

        // Package Id   
        hr = BuffReadString((BYTE*)pvData, cbData, &iData, &sczPackageId);
        ExitOnFailure(hr, "Failed to read package Id.");

        // Error code
        hr = BuffReadNumber((BYTE*)pvData, cbData, &iData, &dwError);
        ExitOnFailure(hr, "Failed to read errro code.");

        // Error string
        hr = BuffReadString((BYTE*)pvData, cbData, &iData, &sczMessage);
        ExitOnFailure(hr, "Failed to read error string.");
        
        // UI hint
        hr = BuffReadNumber((BYTE*)pvData, cbData, &iData, &dwUIFlags);
        ExitOnFailure(hr, "Failed to read UI hint.");

        nResult = m_pBurnView->OnError(sczPackageId, dwError, sczMessage, dwUIFlags);

    LExit:
        ReleaseStr(sczPackageId);
        ReleaseStr(sczMessage);

        return nResult;
    }

    int OnExecuteMsiMessage(
        __in LPVOID pvData,
        __in DWORD cbData
        )
    {
        HRESULT hr = S_OK;
        DWORD er = ERROR_SUCCESS;
        int nResult = IDOK;
        SIZE_T iData = 0;
        LPWSTR sczPackageId = NULL;
        DWORD mt = 0;
        DWORD uiFlags = 0;
        LPWSTR sczMessage = NULL;
        DWORD cFields = 0;
        LPWSTR scz = NULL;

        // package id
        hr = BuffReadString((BYTE*)pvData, cbData, &iData, &sczPackageId);
        ExitOnFailure(hr, "Failed to read package Id.");

        hr = BuffReadNumber((BYTE*)pvData, cbData, &iData, &mt);
        ExitOnFailure(hr, "Failed to read message id.");

        hr = BuffReadNumber((BYTE*)pvData, cbData, &iData, &uiFlags);
        ExitOnFailure(hr, "Failed to read UI flags.");

        hr = BuffReadString((BYTE*)pvData, cbData, &iData, &sczMessage);
        ExitOnFailure(hr, "Failed to read message.");

        nResult = m_pBurnView->OnExecuteMsiMessage(sczPackageId, (INSTALLMESSAGE)mt, uiFlags, sczMessage);

    LExit:
        ReleaseStr(sczPackageId);
        ReleaseStr(sczMessage);
        ReleaseStr(scz);

        return nResult;
    }

    int OnExecuteMsiFilesInUse(
        __in LPVOID pvData,
        __in DWORD cbData
        )
    {
        HRESULT hr = S_OK;
        DWORD er = ERROR_SUCCESS;
        int nResult = IDOK;
        SIZE_T iData = 0;
        LPWSTR sczPackageId = NULL;
        DWORD cFiles = 0;
        LPWSTR* rgwzFiles = NULL;
        DWORD cch = 0;

        // package id
        hr = BuffReadString((BYTE*)pvData, cbData, &iData, &sczPackageId);
        ExitOnFailure(hr, "Failed to read package Id.");

        // read files
        hr = BuffReadNumber((BYTE*)pvData, cbData, &iData, &cFiles);
        ExitOnFailure(hr, "Failed to read file count.");

        rgwzFiles = (LPWSTR*)MemAlloc(sizeof(LPWSTR*) * cFiles, TRUE);
        ExitOnNull(rgwzFiles, hr, E_OUTOFMEMORY, "Failed to allocate buffer.");

        for (DWORD i = 0; i < cFiles; ++i)
        {
            hr = BuffReadString((BYTE*)pvData, cbData, &iData, &rgwzFiles[i]);
            ExitOnFailure1(hr, "Failed to read file name: %u", i);
        }

        nResult = m_pBurnView->OnExecuteMsiFilesInUse(sczPackageId, cFiles, (LPCWSTR*)rgwzFiles);

    LExit:
        ReleaseStr(sczPackageId);

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

    // IProgressObserver
    virtual int OnProgress(unsigned char soFar)
    {
        return m_unElevatedObserver.OnProgress(soFar);
    }

    virtual int OnProgressDetail(unsigned char soFar)
    {
        return IDOK;
    }

    virtual void Finished(HRESULT hr)
    {
        m_unElevatedObserver.Finished(hr);
    }

    virtual void OnStateChange(State enumVal)  
    {
    }

    virtual void OnStateChangeDetail (const State enumVal, const CString changeInfo) 
    {
    }
    virtual void OnRebootPending()
    {
    }
};
}
