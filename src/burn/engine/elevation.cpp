//-------------------------------------------------------------------------------------------------
// <copyright file="elevated.cpp" company="Microsoft">
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
//    Module: Elevation
//
//    Burn elevated process handler.
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"


// internal function declarations

static HRESULT CreateElevatedInfo(
    __inout_z LPWSTR *psczPipeName,
    __inout_z LPWSTR *psczClientToken
    );
static HRESULT CreateElevatedPipe(
    __in LPCWSTR wzPipeName,
    __out HANDLE* phPipe
    );
static HRESULT CreateElevatedProcess(
    __in_opt HWND hwndParent,
    __in_z LPCWSTR wzExecutable,
    __in_z LPCWSTR wzOptionName,
    __in_z LPCWSTR wzPipeName,
    __in_z LPCWSTR wzToken,
    __out HANDLE* phProcess
    );
static HRESULT ConnectElevatedProcess(
    __in LPCWSTR wzToken,
    __in HANDLE hPipe,
    __in HANDLE hProcess
    );
static HRESULT WriteMessage(
    __in HANDLE hPipe,
    __in DWORD dwMessage,
    __in_bcount_opt(cbData) LPVOID pvData,
    __in DWORD cbData
    );
static HRESULT AllocateMessage(
    __in DWORD dwMessage,
    __in_bcount_opt(cbData) LPVOID pvData,
    __in DWORD cbData,
    __out_bcount(cb) LPVOID* ppvMessage,
    __out DWORD* cbMessage
    );
static HRESULT ProcessCommonPackageMessages(
    __in BURN_ELEVATION_MESSAGE* pMsg,
    __out DWORD* pdwResult
    );
static HRESULT OnExecuteMsiFilesInUse(
    __in LPVOID pvData,
    __in DWORD cbData,
    __in PFN_MSIEXECUTEMESSAGEHANDLER pfnMessageHandler,
    __in LPVOID pvContext,
    __out DWORD* pdwResult
    );
static HRESULT OnSessionBegin(
    __in BURN_REGISTRATION* pRegistration,
    __in BURN_USER_EXPERIENCE* pUserExperience,
    __in BYTE* pbData,
    __in DWORD cbData
    );
static HRESULT OnSessionSuspend(
    __in BURN_REGISTRATION* pRegistration,
    __in BYTE* pbData,
    __in DWORD cbData
    );
static HRESULT OnSessionResume(
    __in BURN_REGISTRATION* pRegistration,
    __in BYTE* pbData,
    __in DWORD cbData
    );
static HRESULT OnSessionEnd(
    __in BURN_REGISTRATION* pRegistration,
    __in BYTE* pbData,
    __in DWORD cbData
    );
static HRESULT OnSaveState(
    __in BURN_REGISTRATION* pRegistration,
    __in BYTE* pbData,
    __in DWORD cbData
    );
static HRESULT OnCachePayload(
    __in BURN_PACKAGES* pPackages,
    __in BURN_PAYLOADS* pPayloads,
    __in BYTE* pbData,
    __in DWORD cbData
    );
static HRESULT OnExecuteExePackage(
    __in BURN_PACKAGES* pPackages,
    __in BURN_VARIABLES* pVariables,
    __in BYTE* pbData,
    __in DWORD cbData
    );
static HRESULT OnExecuteMsiPackage(
    __in HANDLE hPipe,
    __in BURN_PACKAGES* pPackages,
    __in BURN_VARIABLES* pVariables,
    __in BYTE* pbData,
    __in DWORD cbData
    );
static HRESULT OnExecuteMspPackage(
    __in HANDLE hPipe,
    __in BURN_PACKAGES* pPackages,
    __in BURN_VARIABLES* pVariables,
    __in BYTE* pbData,
    __in DWORD cbData
    );
static int MsiExecuteMessageHandler(
    __in WIU_MSI_EXECUTE_MESSAGE* pMessage,
    __in_opt LPVOID pvContext
    );
static HRESULT OnExecuteMsuPackage(
    __in BURN_PACKAGES* pPackages,
    __in BYTE* pbData,
    __in DWORD cbData
    );
static HRESULT OnCleanBundle(
    __in BURN_REGISTRATION* pRegistration,
    __in BYTE* pbData,
    __in DWORD cbData
    );
static HRESULT OnCleanPackage(
    __in BURN_PACKAGES* pPackages,
    __in BYTE* pbData,
    __in DWORD cbData
    );


// function definitions

/*******************************************************************
 ElevationMessageUninitialize - 

*******************************************************************/
extern "C" void ElevationMessageUninitialize(
    __in BURN_ELEVATION_MESSAGE *pMsg
    )
{
    if (pMsg->fAllocatedData)
    {
        ReleaseNullMem(pMsg->pvData);
        pMsg->fAllocatedData = FALSE;
    }
}

/*******************************************************************
 ElevationGetMessage - 

*******************************************************************/
extern "C" HRESULT ElevationGetMessage(
    __in HANDLE hPipe,
    __in BURN_ELEVATION_MESSAGE* pMsg
    )
{
    HRESULT hr = S_OK;
    DWORD rgdwMessageAndByteCount[2] = { };
    DWORD cb = 0;
    DWORD cbRead = 0;

    while (cbRead < sizeof(rgdwMessageAndByteCount))
    {
        if (!::ReadFile(hPipe, reinterpret_cast<BYTE*>(rgdwMessageAndByteCount) + cbRead, sizeof(rgdwMessageAndByteCount) - cbRead, &cb, NULL))
        {
            DWORD er = ::GetLastError();
            if (ERROR_MORE_DATA == er)
            {
                hr = S_OK;
            }
            else if (ERROR_BROKEN_PIPE == er) // parent process shut down, time to exit.
            {
                memset(rgdwMessageAndByteCount, 0, sizeof(rgdwMessageAndByteCount));
                hr = S_FALSE;
                break;
            }
            else
            {
                hr = HRESULT_FROM_WIN32(er);
            }
            ExitOnRootFailure(hr, "Failed to read message from pipe.");
        }

        cbRead += cb;
    }

    pMsg->dwMessage = rgdwMessageAndByteCount[0];
    pMsg->cbData = rgdwMessageAndByteCount[1];
    if (pMsg->cbData)
    {
        pMsg->pvData = MemAlloc(pMsg->cbData, FALSE);
        ExitOnNull(pMsg->pvData, hr, E_OUTOFMEMORY, "Failed to allocate data for message.");

        if (!::ReadFile(hPipe, pMsg->pvData, pMsg->cbData, &cb, NULL))
        {
            ExitWithLastError(hr, "Failed to read data for message.");
        }

        pMsg->fAllocatedData = TRUE;
    }

LExit:
    if (!pMsg->fAllocatedData && pMsg->pvData)
    {
        MemFree(pMsg->pvData);
    }

    return hr;
}

/*******************************************************************
 ElevationPostMessage - 

*******************************************************************/
extern "C" HRESULT ElevationPostMessage(
    __in HANDLE hPipe,
    __in DWORD dwMessage,
    __in_bcount_opt(cbData) LPVOID pvData,
    __in DWORD cbData
    )
{
    HRESULT hr = S_OK;

    hr = WriteMessage(hPipe, dwMessage, pvData, cbData);
    ExitOnFailure(hr, "Failed to write message to parent process.");

LExit:
    return hr;
}

/*******************************************************************
 ElevationSendMessage - 

*******************************************************************/
extern "C" HRESULT ElevationSendMessage(
    __in HANDLE hPipe,
    __in DWORD dwMessage,
    __in_bcount_opt(cbData) LPVOID pvData,
    __in DWORD cbData,
    __out DWORD* pdwResult
    )
{
    HRESULT hr = S_OK;
    LPSTR sczMessage = NULL;
    SIZE_T iData = 0;
    BURN_ELEVATION_MESSAGE msg = { };
    DWORD dwResult = 0;

    // TODO: change these write/read calls into single TransactMessage (aka: TransactNamedPipe) call.
    hr = WriteMessage(hPipe, dwMessage, pvData, cbData);
    ExitOnFailure(hr, "Failed to write message to parent process.");

    // pump messages from per-machine process
    while (S_OK == (hr = ElevationGetMessage(hPipe, &msg)))
    {
        switch (msg.dwMessage)
        {
        case BURN_ELEVATION_MESSAGE_TYPE_COMPLETE:
            if (!msg.pvData || sizeof(DWORD) != msg.cbData)
            {
                hr = E_INVALIDARG;
                ExitOnFailure(hr, "No status returned to ElevationSendMessage()");
            }

            *pdwResult = *(DWORD*)msg.pvData;
            ExitFunction1(hr = S_OK);
            break;

        case BURN_ELEVATION_MESSAGE_TYPE_LOG:
            iData = 0;

            hr = BuffReadStringAnsi((BYTE*)msg.pvData, msg.cbData, &iData, &sczMessage);
            ExitOnFailure(hr, "Failed to read log message.");

            hr = LogStringWorkRaw(sczMessage);
            ExitOnFailure1(hr, "Failed to write log message:'%s'.", sczMessage);

            dwResult = (DWORD)hr;
            break;

        default:
            hr = E_INVALIDARG;
            ExitOnFailure1(hr, "Invalid message returned: %u", msg.dwMessage);
            break;
        }

        // post result
        hr = ElevationPostMessage(hPipe, BURN_ELEVATION_MESSAGE_TYPE_COMPLETE, &dwResult, sizeof(dwResult));
        ExitOnFailure(hr, "Failed to post result to per-machine process.");
    }
    ExitOnFailure(hr, "Failed to get message over pipe");

LExit:
    ReleaseStr(sczMessage);
    ElevationMessageUninitialize(&msg);

    return hr;
}

/*******************************************************************
 ElevationParentProcessConnect - Called from the per-user process to create
                                 the per-machine process and set up the
                                 communication pipe.

*******************************************************************/
extern "C" HRESULT ElevationParentProcessConnect(
    __in HWND hwndParent,
    __out HANDLE* phElevatedProcess,
    __out HANDLE* phElevatedPipe
    )
{
    HRESULT hr = S_OK;
    LPWSTR sczPipeName = NULL;
    LPWSTR sczClientToken = NULL;
    LPWSTR sczBurnPath = NULL;

    hr = CreateElevatedInfo(&sczPipeName, &sczClientToken);
    ExitOnFailure(hr, "Failed to allocate elevation information.");

    hr = PathForCurrentProcess(&sczBurnPath, NULL);
    ExitOnFailure(hr, "Failed to get current process path.");

    hr = ParentProcessConnect(hwndParent, sczBurnPath, BURN_COMMANDLINE_SWITCH_ELEVATED, sczPipeName, sczClientToken, phElevatedProcess, phElevatedPipe);
    ExitOnFailure(hr, "Failed to create elevated process.");

LExit:
    ReleaseStr(sczClientToken);
    ReleaseStr(sczPipeName);
    ReleaseStr(sczBurnPath);

    return hr;
}

/*******************************************************************
 ParentProcessConnect - Called from the parent process. Create the child
                        process and set up the communication pipe.

*******************************************************************/
extern "C" HRESULT ParentProcessConnect(
    __in HWND hwndParent,
    __in_z LPCWSTR wzExecutable,
    __in_z LPCWSTR wzOptionName,
    __in_z LPCWSTR wzPipeName,
    __in_z LPCWSTR wzToken,
    __out HANDLE* phElevatedProcess,
    __out HANDLE* phElevatedPipe
    )
{
    HRESULT hr = S_OK;
    HANDLE hPipe = INVALID_HANDLE_VALUE;
    HANDLE hProcess = NULL;

    hr = CreateElevatedPipe(wzPipeName, &hPipe);
    ExitOnFailure(hr, "Failed to create pipe.");

    hr = CreateElevatedProcess(hwndParent, wzExecutable, wzOptionName, wzPipeName, wzToken, &hProcess);
    ExitOnFailure(hr, "Failed to create process.");

    hr = ConnectElevatedProcess(wzToken, hPipe, hProcess);
    ExitOnFailure(hr, "Failed to connect to process.");

    *phElevatedPipe = hPipe;
    hPipe = INVALID_HANDLE_VALUE;
    *phElevatedProcess = hProcess;
    hProcess = NULL;

LExit:
    if (hProcess)
    {
        ::CloseHandle(hProcess);
    }

    ReleaseFileHandle(hPipe);
    return hr;
}

/*******************************************************************
 ElevationParentProcessTerminate - 

*******************************************************************/
extern "C" HRESULT ElevationParentProcessTerminate(
    __in HANDLE hElevatedProcess,
    __in HANDLE hElevatedPipe
    )
{
    HRESULT hr = S_OK;

    // post terminate message
    hr = ElevationPostMessage(hElevatedPipe, BURN_ELEVATION_MESSAGE_TYPE_TERMINATE, NULL, 0);
    ExitOnFailure(hr, "Failed to post terminate message to per-machine process.");

    // wait for process to exit
    if (WAIT_FAILED == ::WaitForSingleObject(hElevatedProcess, INFINITE))
    {
        ExitWithLastError(hr, "Failed to wait for per-machine process exit.");
    }

LExit:
    return hr;
}

/*******************************************************************
 ElevationSessionBegin - 

*******************************************************************/
extern "C" HRESULT ElevationSessionBegin(
    __in HANDLE hPipe,
    __in BOOTSTRAPPER_ACTION action,
    __in DWORD64 qwEstimatedSize
    )
{
    HRESULT hr = S_OK;
    BYTE* pbData = NULL;
    SIZE_T cbData = 0;
    DWORD dwResult = 0;

    // serialize message data
    hr = BuffWriteNumber(&pbData, &cbData, (DWORD)action);
    ExitOnFailure(hr, "Failed to write action to message buffer.");

    hr = BuffWriteNumber64(&pbData, &cbData, qwEstimatedSize);
    ExitOnFailure(hr, "Failed to write estimated size to message buffer.");

    // send message
    hr = ElevationSendMessage(hPipe, BURN_ELEVATION_MESSAGE_TYPE_SESSION_BEGIN, pbData, cbData, &dwResult);
    ExitOnFailure(hr, "Failed to send message to per-machine process.");

    hr = (HRESULT)dwResult;

LExit:
    ReleaseBuffer(pbData);

    return hr;
}

/*******************************************************************
 ElevationSessionSuspend - 

*******************************************************************/
extern "C" HRESULT ElevationSessionSuspend(
    __in HANDLE hPipe,
    __in BOOTSTRAPPER_ACTION action,
    __in BOOL fReboot
    )
{
    HRESULT hr = S_OK;
    BYTE* pbData = NULL;
    SIZE_T cbData = 0;
    DWORD dwResult = 0;

    // serialize message data
    hr = BuffWriteNumber(&pbData, &cbData, (DWORD)action);
    ExitOnFailure(hr, "Failed to write action to message buffer.");

    hr = BuffWriteNumber(&pbData, &cbData, (DWORD)fReboot);
    ExitOnFailure(hr, "Failed to write reboot flag to message buffer.");

    // send message
    hr = ElevationSendMessage(hPipe, BURN_ELEVATION_MESSAGE_TYPE_SESSION_SUSPEND, pbData, cbData, &dwResult);
    ExitOnFailure(hr, "Failed to send message to per-machine process.");

    hr = (HRESULT)dwResult;

LExit:
    ReleaseBuffer(pbData);

    return hr;
}

/*******************************************************************
 ElevationSessionResume - 

*******************************************************************/
extern "C" HRESULT ElevationSessionResume(
    __in HANDLE hPipe,
    __in BOOTSTRAPPER_ACTION action
    )
{
    HRESULT hr = S_OK;
    BYTE* pbData = NULL;
    SIZE_T cbData = 0;
    DWORD dwResult = 0;

    // serialize message data
    hr = BuffWriteNumber(&pbData, &cbData, (DWORD)action);
    ExitOnFailure(hr, "Failed to write action to message buffer.");

    // send message
    hr = ElevationSendMessage(hPipe, BURN_ELEVATION_MESSAGE_TYPE_SESSION_RESUME, pbData, cbData, &dwResult);
    ExitOnFailure(hr, "Failed to send message to per-machine process.");

    hr = (HRESULT)dwResult;

LExit:
    ReleaseBuffer(pbData);

    return hr;
}

/*******************************************************************
 ElevationSessionEnd - 

*******************************************************************/
extern "C" HRESULT ElevationSessionEnd(
    __in HANDLE hPipe,
    __in BOOTSTRAPPER_ACTION action,
    __in BOOL fRollback
    )
{
    HRESULT hr = S_OK;
    BYTE* pbData = NULL;
    SIZE_T cbData = 0;
    DWORD dwResult = 0;

    // serialize message data
    hr = BuffWriteNumber(&pbData, &cbData, (DWORD)action);
    ExitOnFailure(hr, "Failed to write action to message buffer.");

    hr = BuffWriteNumber(&pbData, &cbData, (DWORD)fRollback);
    ExitOnFailure(hr, "Failed to write rollback flag to message buffer.");

    // send message
    hr = ElevationSendMessage(hPipe, BURN_ELEVATION_MESSAGE_TYPE_SESSION_END, pbData, cbData, &dwResult);
    ExitOnFailure(hr, "Failed to send message to per-machine process.");

    hr = (HRESULT)dwResult;

LExit:
    ReleaseBuffer(pbData);

    return hr;
}

/*******************************************************************
 ElevationSaveState - 

*******************************************************************/
HRESULT ElevationSaveState(
    __in HANDLE hPipe,
    __in_bcount(cbBuffer) BYTE* pbBuffer,
    __in SIZE_T cbBuffer
    )
{
    HRESULT hr = S_OK;
    DWORD dwResult = 0;

    // send message
    hr = ElevationSendMessage(hPipe, BURN_ELEVATION_MESSAGE_TYPE_SAVE_STATE, pbBuffer, (DWORD)cbBuffer, &dwResult);
    ExitOnFailure(hr, "Failed to send message to per-machine process.");

    hr = (HRESULT)dwResult;

LExit:
    return hr;
}

extern "C" HRESULT ElevationCachePayload(
    __in HANDLE hPipe,
    __in BURN_PACKAGE* pPackage,
    __in BURN_PAYLOAD* pPayload,
    __in_z LPCWSTR wzUnverifiedPayloadPath,
    __in BOOL fMove
    )
{
    HRESULT hr = S_OK;
    BYTE* pbData = NULL;
    SIZE_T cbData = 0;
    DWORD dwResult = 0;

    // serialize message data
    hr = BuffWriteString(&pbData, &cbData, pPackage->sczId);
    ExitOnFailure(hr, "Failed to write package id to message buffer.");

    hr = BuffWriteString(&pbData, &cbData, pPayload->sczKey);
    ExitOnFailure(hr, "Failed to write payload id to message buffer.");

    hr = BuffWriteString(&pbData, &cbData, wzUnverifiedPayloadPath);
    ExitOnFailure(hr, "Failed to write payload id to message buffer.");

    hr = BuffWriteNumber(&pbData, &cbData, (DWORD)fMove); // TODO: This could be a minor security issue.
    ExitOnFailure(hr, "Failed to write move flag to message buffer.");

    // send message
    hr = ElevationSendMessage(hPipe, BURN_ELEVATION_MESSAGE_TYPE_CACHE_PAYLOAD, pbData, cbData, &dwResult);
    ExitOnFailure(hr, "Failed to send BURN_ELEVATION_MESSAGE_TYPE_CACHE_PAYLOAD message to per-machine process.");

    hr = (HRESULT)dwResult;

LExit:
    ReleaseBuffer(pbData);

    return hr;
}

extern "C" HRESULT ElevationExecuteExePackage(
    __in HANDLE hPipe,
    __in BURN_EXECUTE_ACTION* pExecuteAction,
    __in BURN_VARIABLES* pVariables,
    __out BOOTSTRAPPER_APPLY_RESTART* pRestart
    )
{
    HRESULT hr = S_OK;
    BYTE* pbData = NULL;
    SIZE_T cbData = 0;
    BURN_ELEVATION_MESSAGE msg = { };
    DWORD dwResult = 0;

    // serialize message data
    hr = BuffWriteString(&pbData, &cbData, pExecuteAction->exePackage.pPackage->sczId);
    ExitOnFailure(hr, "Failed to write package id to message buffer.");

    hr = BuffWriteNumber(&pbData, &cbData, (DWORD)pExecuteAction->exePackage.action);
    ExitOnFailure(hr, "Failed to write action to message buffer.");

    hr = VariableSerialize(pVariables, &pbData, &cbData);
    ExitOnFailure(hr, "Failed to write variables.");

    // post message
    hr = ElevationPostMessage(hPipe, BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_EXE_PACKAGE, pbData, cbData);
    ExitOnFailure(hr, "Failed to send BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_EXE_PACKAGE message to per-machine process.");

    // pump messages from per-machine process
    while (S_OK == (hr = ElevationGetMessage(hPipe, &msg)))
    {
        // Process the message.
        switch (msg.dwMessage)
        {
        case BURN_ELEVATION_MESSAGE_TYPE_COMPLETE:
            if (!msg.pvData || sizeof(HRESULT) != msg.cbData)
            {
                hr = E_INVALIDARG;
                ExitOnFailure(hr, "Invalid data for complete message.");
            }

            hr = *(HRESULT*)msg.pvData;
            if (HRESULT_FROM_WIN32(ERROR_SUCCESS_REBOOT_REQUIRED) == hr)
            {
                *pRestart = BOOTSTRAPPER_APPLY_RESTART_REQUIRED;
                hr = S_OK;
            }
            else if (HRESULT_FROM_WIN32(ERROR_SUCCESS_REBOOT_INITIATED) == hr)
            {
                *pRestart = BOOTSTRAPPER_APPLY_RESTART_INITIATED;
                hr = S_OK;
            }

            ExitFunction();
            break;

        default:
            hr = ProcessCommonPackageMessages(&msg, &dwResult);
            ExitOnFailure(hr, "Failed to process common package message.");
            break;
        }

        // post result
        hr = ElevationPostMessage(hPipe, BURN_ELEVATION_MESSAGE_TYPE_COMPLETE, &dwResult, sizeof(dwResult));
        ExitOnFailure(hr, "Failed to post result to per-machine process.");

        ElevationMessageUninitialize(&msg);
    }

    if (S_FALSE == hr)
    {
        hr = S_OK;
    }

LExit:
    ReleaseBuffer(pbData);
    ElevationMessageUninitialize(&msg);

    return hr;
}

extern "C" HRESULT ElevationExecuteMsiPackage(
    __in HANDLE hPipe,
    __in BURN_EXECUTE_ACTION* pExecuteAction,
    __in BURN_VARIABLES* pVariables,
    __in BOOL fRollback,
    __in PFN_MSIEXECUTEMESSAGEHANDLER pfnMessageHandler,
    __in LPVOID pvContext,
    __out BOOTSTRAPPER_APPLY_RESTART* pRestart
    )
{
    HRESULT hr = S_OK;
    BYTE* pbData = NULL;
    SIZE_T cbData = 0;
    BURN_ELEVATION_MESSAGE msg = { };
    SIZE_T iData = 0;
    WIU_MSI_EXECUTE_MESSAGE message = { };
    DWORD dwResult = 0;
    LPWSTR sczMessage = NULL;

    // serialize message data
    // TODO: for patching we might not have a package
    hr = BuffWriteString(&pbData, &cbData, pExecuteAction->msiPackage.pPackage->sczId);
    ExitOnFailure(hr, "Failed to write package id to message buffer.");

    hr = BuffWriteString(&pbData, &cbData, pExecuteAction->msiPackage.sczLogPath);
    ExitOnFailure(hr, "Failed to write package log to message buffer.");

    hr = BuffWriteNumber(&pbData, &cbData, (DWORD)pExecuteAction->msiPackage.action);
    ExitOnFailure(hr, "Failed to write action to message buffer.");

    for (DWORD i = 0; i < pExecuteAction->msiPackage.pPackage->Msi.cFeatures; ++i)
    {
        hr = BuffWriteNumber(&pbData, &cbData, (DWORD)pExecuteAction->msiPackage.rgFeatures[i]);
        ExitOnFailure(hr, "Failed to write feature action to message buffer.");
    }

    // TODO: patches

    hr = VariableSerialize(pVariables, &pbData, &cbData);
    ExitOnFailure(hr, "Failed to write variables.");

    hr = BuffWriteNumber(&pbData, &cbData, (DWORD)fRollback);
    ExitOnFailure(hr, "Failed to write rollback flag to message buffer.");

    // post message
    hr = ElevationPostMessage(hPipe, BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_MSI_PACKAGE, pbData, cbData);
    ExitOnFailure(hr, "Failed to send BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_MSI_PACKAGE message to per-machine process.");

    // pump messages from per-machine process
    while (S_OK == (hr = ElevationGetMessage(hPipe, &msg)))
    {
        // Process the message.
        switch (msg.dwMessage)
        {
        case BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_PROGRESS:
            // read message parameters
            message.type = WIU_MSI_EXECUTE_MESSAGE_PROGRESS;

            hr = BuffReadNumber((BYTE*)msg.pvData, msg.cbData, &iData, &message.progress.dwPercentage);
            ExitOnFailure(hr, "Failed to read progress.");

            // send message
            dwResult = (DWORD)pfnMessageHandler(&message, pvContext);
            break;

        case BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_ERROR:
            // read message parameters
            message.type = WIU_MSI_EXECUTE_MESSAGE_ERROR;

            hr = BuffReadNumber((BYTE*)msg.pvData, msg.cbData, &iData, &message.error.dwErrorCode);
            ExitOnFailure(hr, "Failed to read error code.");

            hr = BuffReadNumber((BYTE*)msg.pvData, msg.cbData, &iData, (DWORD*)&message.error.uiFlags);
            ExitOnFailure(hr, "Failed to read UI flags.");

            hr = BuffReadString((BYTE*)msg.pvData, msg.cbData, &iData, &sczMessage);
            ExitOnFailure(hr, "Failed to read message.");
            message.error.wzMessage = sczMessage;

            // send message
            dwResult = (DWORD)pfnMessageHandler(&message, pvContext);
            break;

        case BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_MSI_MESSAGE:
            // read message parameters
            message.type = WIU_MSI_EXECUTE_MESSAGE_MSI_MESSAGE;

            hr = BuffReadNumber((BYTE*)msg.pvData, msg.cbData, &iData, (DWORD*)&message.msiMessage.mt);
            ExitOnFailure(hr, "Failed to read message type.");

            hr = BuffReadNumber((BYTE*)msg.pvData, msg.cbData, &iData, (DWORD*)&message.msiMessage.uiFlags);
            ExitOnFailure(hr, "Failed to read UI flags.");

            hr = BuffReadString((BYTE*)msg.pvData, msg.cbData, &iData, &sczMessage);
            ExitOnFailure(hr, "Failed to read message.");
            message.msiMessage.wzMessage = sczMessage;

            // send message
            dwResult = (DWORD)pfnMessageHandler(&message, pvContext);
            break;

        case BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_MSI_FILES_IN_USE:
            hr = OnExecuteMsiFilesInUse(msg.pvData, msg.cbData, pfnMessageHandler, pvContext, &dwResult);
            break;

        case BURN_ELEVATION_MESSAGE_TYPE_COMPLETE:
            if (!msg.pvData || sizeof(HRESULT) != msg.cbData)
            {
                hr = E_INVALIDARG;
                ExitOnFailure(hr, "Invalid data for complete message.");
            }

            hr = *(HRESULT*)msg.pvData;
            if (HRESULT_FROM_WIN32(ERROR_SUCCESS_REBOOT_REQUIRED) == hr)
            {
                *pRestart = BOOTSTRAPPER_APPLY_RESTART_REQUIRED;
                hr = S_OK;
            }
            else if (HRESULT_FROM_WIN32(ERROR_SUCCESS_REBOOT_INITIATED) == hr)
            {
                *pRestart = BOOTSTRAPPER_APPLY_RESTART_INITIATED;
                hr = S_OK;
            }

            ExitFunction();
            break;

        default:
            hr = ProcessCommonPackageMessages(&msg, &dwResult);
            ExitOnFailure(hr, "Failed to process common package message.");
            break;
        }

        // post result
        hr = ElevationPostMessage(hPipe, BURN_ELEVATION_MESSAGE_TYPE_COMPLETE, &dwResult, sizeof(dwResult));
        ExitOnFailure(hr, "Failed to post result to per-machine process.");

        // prepare next iteration
        ElevationMessageUninitialize(&msg);
        iData = 0;
        memset(&message, 0, sizeof(WIU_MSI_EXECUTE_MESSAGE));
    }

    if (S_FALSE == hr)
    {
        hr = S_OK;
    }

LExit:
    ReleaseBuffer(pbData);
    ElevationMessageUninitialize(&msg);
    ReleaseStr(sczMessage);

    return hr;
}

extern "C" HRESULT ElevationExecuteMspPackage(
    __in HANDLE hPipe,
    __in BURN_EXECUTE_ACTION* pExecuteAction,
    __in BURN_VARIABLES* pVariables,
    __in BOOL fRollback,
    __in PFN_MSIEXECUTEMESSAGEHANDLER pfnMessageHandler,
    __in LPVOID pvContext,
    __out BOOTSTRAPPER_APPLY_RESTART* pRestart
    )
{
    HRESULT hr = S_OK;
    BYTE* pbData = NULL;
    SIZE_T cbData = 0;
    BURN_ELEVATION_MESSAGE msg = { };
    SIZE_T iData = 0;
    WIU_MSI_EXECUTE_MESSAGE message = { };
    DWORD dwResult = 0;
    LPWSTR sczMessage = NULL;

    // serialize message data
    hr = BuffWriteString(&pbData, &cbData, pExecuteAction->mspTarget.pPackage->sczId);
    ExitOnFailure(hr, "Failed to write package id to message buffer.");

    hr = BuffWriteString(&pbData, &cbData, pExecuteAction->mspTarget.sczTargetProductCode);
    ExitOnFailure(hr, "Failed to write target product code to message buffer.");

    hr = BuffWriteString(&pbData, &cbData, pExecuteAction->mspTarget.sczLogPath);
    ExitOnFailure(hr, "Failed to write package log to message buffer.");

    hr = BuffWriteNumber(&pbData, &cbData, (DWORD)pExecuteAction->mspTarget.action);
    ExitOnFailure(hr, "Failed to write action to message buffer.");

    hr = BuffWriteNumber(&pbData, &cbData, pExecuteAction->mspTarget.cOrderedPatches);
    ExitOnFailure(hr, "Failed to write count of ordered patches to message buffer.");

    for (DWORD i = 0; i < pExecuteAction->mspTarget.cOrderedPatches; ++i)
    {
        hr = BuffWriteNumber(&pbData, &cbData, pExecuteAction->mspTarget.rgOrderedPatches[i].dwOrder);
        ExitOnFailure(hr, "Failed to write ordered patch order to message buffer.");

        hr = BuffWriteString(&pbData, &cbData, pExecuteAction->mspTarget.rgOrderedPatches[i].pPackage->sczId);
        ExitOnFailure(hr, "Failed to write ordered patch id to message buffer.");
    }

    hr = VariableSerialize(pVariables, &pbData, &cbData);
    ExitOnFailure(hr, "Failed to write variables.");

    hr = BuffWriteNumber(&pbData, &cbData, (DWORD)fRollback);
    ExitOnFailure(hr, "Failed to write rollback flag to message buffer.");

    // post message
    hr = ElevationPostMessage(hPipe, BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_MSP_PACKAGE, pbData, cbData);
    ExitOnFailure(hr, "Failed to send BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_MSP_PACKAGE message to per-machine process.");

    // pump messages from per-machine process
    while (S_OK == (hr = ElevationGetMessage(hPipe, &msg)))
    {
        // Process the message.
        switch (msg.dwMessage)
        {
        case BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_PROGRESS:
            // read message parameters
            message.type = WIU_MSI_EXECUTE_MESSAGE_PROGRESS;

            hr = BuffReadNumber((BYTE*)msg.pvData, msg.cbData, &iData, &message.progress.dwPercentage);
            ExitOnFailure(hr, "Failed to read progress.");

            // send message
            dwResult = (DWORD)pfnMessageHandler(&message, pvContext);
            break;

        case BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_ERROR:
            // read message parameters
            message.type = WIU_MSI_EXECUTE_MESSAGE_ERROR;

            hr = BuffReadNumber((BYTE*)msg.pvData, msg.cbData, &iData, &message.error.dwErrorCode);
            ExitOnFailure(hr, "Failed to read error code.");

            hr = BuffReadNumber((BYTE*)msg.pvData, msg.cbData, &iData, (DWORD*)&message.error.uiFlags);
            ExitOnFailure(hr, "Failed to read UI flags.");

            hr = BuffReadString((BYTE*)msg.pvData, msg.cbData, &iData, &sczMessage);
            ExitOnFailure(hr, "Failed to read message.");
            message.error.wzMessage = sczMessage;

            // send message
            dwResult = (DWORD)pfnMessageHandler(&message, pvContext);
            break;

        case BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_MSI_MESSAGE:
            // read message parameters
            message.type = WIU_MSI_EXECUTE_MESSAGE_MSI_MESSAGE;

            hr = BuffReadNumber((BYTE*)msg.pvData, msg.cbData, &iData, (DWORD*)&message.msiMessage.mt);
            ExitOnFailure(hr, "Failed to read message type.");

            hr = BuffReadNumber((BYTE*)msg.pvData, msg.cbData, &iData, (DWORD*)&message.msiMessage.uiFlags);
            ExitOnFailure(hr, "Failed to read UI flags.");

            hr = BuffReadString((BYTE*)msg.pvData, msg.cbData, &iData, &sczMessage);
            ExitOnFailure(hr, "Failed to read message.");
            message.msiMessage.wzMessage = sczMessage;

            // send message
            dwResult = (DWORD)pfnMessageHandler(&message, pvContext);
            break;

        case BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_MSI_FILES_IN_USE:
            hr = OnExecuteMsiFilesInUse(msg.pvData, msg.cbData, pfnMessageHandler, pvContext, &dwResult);
            break;

        case BURN_ELEVATION_MESSAGE_TYPE_COMPLETE:
            if (!msg.pvData || sizeof(HRESULT) != msg.cbData)
            {
                hr = E_INVALIDARG;
                ExitOnFailure(hr, "Invalid data for complete message.");
            }

            hr = *(HRESULT*)msg.pvData;
            if (HRESULT_FROM_WIN32(ERROR_SUCCESS_REBOOT_REQUIRED) == hr)
            {
                *pRestart = BOOTSTRAPPER_APPLY_RESTART_REQUIRED;
                hr = S_OK;
            }
            else if (HRESULT_FROM_WIN32(ERROR_SUCCESS_REBOOT_INITIATED) == hr)
            {
                *pRestart = BOOTSTRAPPER_APPLY_RESTART_INITIATED;
                hr = S_OK;
            }

            ExitFunction();
            break;

        default:
            hr = ProcessCommonPackageMessages(&msg, &dwResult);
            ExitOnFailure(hr, "Failed to process common package message.");
            break;
        }

        // post result
        hr = ElevationPostMessage(hPipe, BURN_ELEVATION_MESSAGE_TYPE_COMPLETE, &dwResult, sizeof(dwResult));
        ExitOnFailure(hr, "Failed to post result to per-machine process.");

        // prepare next iteration
        ElevationMessageUninitialize(&msg);
        iData = 0;
        memset(&message, 0, sizeof(WIU_MSI_EXECUTE_MESSAGE));
    }

    if (S_FALSE == hr)
    {
        hr = S_OK;
    }

LExit:
    ReleaseBuffer(pbData);
    ElevationMessageUninitialize(&msg);
    ReleaseStr(sczMessage);

    return hr;
}

extern "C" HRESULT ElevationExecuteMsuPackage(
    __in HANDLE hPipe,
    __in BURN_EXECUTE_ACTION* pExecuteAction,
    __out BOOTSTRAPPER_APPLY_RESTART* pRestart
    )
{
    HRESULT hr = S_OK;
    BYTE* pbData = NULL;
    SIZE_T cbData = 0;
    BURN_ELEVATION_MESSAGE msg = { };
    DWORD dwResult = 0;

    // serialize message data
    hr = BuffWriteString(&pbData, &cbData, pExecuteAction->msuPackage.pPackage->sczId);
    ExitOnFailure(hr, "Failed to write package id to message buffer.");

    hr = BuffWriteString(&pbData, &cbData, pExecuteAction->msuPackage.sczLogPath);
    ExitOnFailure(hr, "Failed to write package log to message buffer.");

    hr = BuffWriteNumber(&pbData, &cbData, (DWORD)pExecuteAction->msuPackage.action);
    ExitOnFailure(hr, "Failed to write action to message buffer.");

    // post message
    hr = ElevationPostMessage(hPipe, BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_MSU_PACKAGE, pbData, cbData);
    ExitOnFailure(hr, "Failed to send BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_MSU_PACKAGE message to per-machine process.");

    // pump messages from per-machine process
    while (S_OK == (hr = ElevationGetMessage(hPipe, &msg)))
    {
        // Process the message.
        switch (msg.dwMessage)
        {
        case BURN_ELEVATION_MESSAGE_TYPE_COMPLETE:
            if (!msg.pvData || sizeof(HRESULT) != msg.cbData)
            {
                hr = E_INVALIDARG;
                ExitOnFailure(hr, "Invalid data for complete message.");
            }

            hr = *(HRESULT*)msg.pvData;
            if (HRESULT_FROM_WIN32(ERROR_SUCCESS_REBOOT_REQUIRED) == hr)
            {
                *pRestart = BOOTSTRAPPER_APPLY_RESTART_REQUIRED;
                hr = S_OK;
            }
            else if (HRESULT_FROM_WIN32(ERROR_SUCCESS_REBOOT_INITIATED) == hr)
            {
                *pRestart = BOOTSTRAPPER_APPLY_RESTART_INITIATED;
                hr = S_OK;
            }

            ExitFunction();
            break;

        default:
            hr = ProcessCommonPackageMessages(&msg, &dwResult);
            ExitOnFailure(hr, "Failed to process common package message.");
            break;
        }

        // post result
        hr = ElevationPostMessage(hPipe, BURN_ELEVATION_MESSAGE_TYPE_COMPLETE, &dwResult, sizeof(dwResult));
        ExitOnFailure(hr, "Failed to post result to per-machine process.");

        ElevationMessageUninitialize(&msg);
    }

    if (S_FALSE == hr)
    {
        hr = S_OK;
    }

LExit:
    ReleaseBuffer(pbData);
    ElevationMessageUninitialize(&msg);

    return hr;
}

extern "C" HRESULT ElevationCleanBundle(
    __in HANDLE hPipe,
    __in BURN_CLEAN_ACTION* pCleanAction
    )
{
    Assert(BURN_CLEAN_ACTION_TYPE_BUNDLE == pCleanAction->type);

    HRESULT hr = S_OK;
    BYTE* pbData = NULL;
    SIZE_T cbData = 0;
    DWORD dwResult = 0;

    // serialize message data
    hr = BuffWriteString(&pbData, &cbData, pCleanAction->bundle.pBundle->sczId);
    ExitOnFailure(hr, "Failed to write clean bundle id to message buffer.");

    // send message
    hr = ElevationSendMessage(hPipe, BURN_ELEVATION_MESSAGE_TYPE_CLEAN_BUNDLE, pbData, cbData, &dwResult);
    ExitOnFailure(hr, "Failed to send BURN_ELEVATION_MESSAGE_TYPE_CLEAN_BUNDLE message to per-machine process.");

    hr = (HRESULT)dwResult;

LExit:
    ReleaseBuffer(pbData);

    return hr;
}

extern "C" HRESULT ElevationCleanPackage(
    __in HANDLE hPipe,
    __in BURN_CLEAN_ACTION* pCleanAction
    )
{
    Assert(BURN_CLEAN_ACTION_TYPE_PACKAGE == pCleanAction->type);

    HRESULT hr = S_OK;
    BYTE* pbData = NULL;
    SIZE_T cbData = 0;
    DWORD dwResult = 0;

    // serialize message data
    hr = BuffWriteString(&pbData, &cbData, pCleanAction->package.pPackage->sczId);
    ExitOnFailure(hr, "Failed to write clean package id to message buffer.");

    // send message
    hr = ElevationSendMessage(hPipe, BURN_ELEVATION_MESSAGE_TYPE_CLEAN_PACKAGE, pbData, cbData, &dwResult);
    ExitOnFailure(hr, "Failed to send BURN_ELEVATION_MESSAGE_TYPE_CLEAN_PACKAGE message to per-machine process.");

    hr = (HRESULT)dwResult;

LExit:
    ReleaseBuffer(pbData);

    return hr;
}

/*******************************************************************
 ElevationChildConnect - Called from the per-machine process to connect back
                         to the pipe provided by the per-user process.

*******************************************************************/
extern "C" HRESULT ElevationChildConnect(
    __in_z LPCWSTR wzPipeName,
    __in_z LPCWSTR wzToken,
    __out HANDLE* phPipe
    )
{
    HRESULT hr = S_OK;
    LPWSTR sczPipeName = NULL;
    HANDLE hPipe = INVALID_HANDLE_VALUE;
    DWORD cbVerificationToken = 0;
    LPWSTR sczVerificationToken = NULL;
    DWORD dwRead = 0;

    hr = StrAllocFormatted(&sczPipeName, L"\\\\.\\pipe\\%s", wzPipeName);
    ExitOnFailure(hr, "Failed to allocate name of parent pipe.");

    // Connect to the parent.
    hPipe = ::CreateFileW(sczPipeName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (INVALID_HANDLE_VALUE == hPipe)
    {
        ExitWithLastError1(hr, "Failed to open parent pipe: %S", sczPipeName)
    }

    // Read the verification token.
    if (!::ReadFile(hPipe, &cbVerificationToken, sizeof(cbVerificationToken), &dwRead, NULL))
    {
        ExitWithLastError(hr, "Failed to read size of verification token from parent pipe.");
    }

    if (255 < cbVerificationToken / sizeof(WCHAR))
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
        ExitOnRootFailure(hr, "Verification token from parent is too big.");
    }

    hr = StrAlloc(&sczVerificationToken, cbVerificationToken / sizeof(WCHAR) + 1);
    ExitOnFailure(hr, "Failed to allocate buffer for verification token.");

    if (!::ReadFile(hPipe, sczVerificationToken, cbVerificationToken, &dwRead, NULL))
    {
        ExitWithLastError(hr, "Failed to read size of verification token from parent pipe.");
    }

    // Verify the tokens match.
    if (CSTR_EQUAL != ::CompareStringW(LOCALE_NEUTRAL, 0, sczVerificationToken, -1, wzToken, -1))
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
        ExitOnRootFailure(hr, "Verification token from parent does not match.");
    }

    *phPipe = hPipe;
    hPipe = INVALID_HANDLE_VALUE;

LExit:
    ReleaseStr(sczVerificationToken);
    ReleaseFileHandle(hPipe);
    ReleaseStr(sczPipeName);
    return hr;
}

/*******************************************************************
 ElevationChildConnected - 

*******************************************************************/
extern "C" HRESULT ElevationChildConnected(
    HANDLE hPipe
    )
{
    HRESULT hr = S_OK;
    DWORD dwAck = 1;
    DWORD cb = 0;

    if (!::WriteFile(hPipe, &dwAck, sizeof(dwAck), &cb, NULL))
    {
        ExitWithLastError(hr, "Failed to inform parent process that elevated child is running.");
    }

LExit:
    return hr;
}

extern "C" HRESULT ElevationChildPumpMessages(
    __in HANDLE hPipe,
    __in BURN_PACKAGES* pPackages,
    __in BURN_PAYLOADS* pPayloads,
    __in BURN_VARIABLES* pVariables,
    __in BURN_REGISTRATION* pRegistration,
    __in BURN_USER_EXPERIENCE* pUserExperience
    )
{
    HRESULT hr = S_OK;
    HRESULT hrResult = S_OK;
    BURN_ELEVATION_MESSAGE msg = { };

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

        case BURN_ELEVATION_MESSAGE_TYPE_CACHE_PAYLOAD:
            hrResult = OnCachePayload(pPackages, pPayloads, (BYTE*)msg.pvData, msg.cbData);
            break;

        case BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_EXE_PACKAGE:
            hrResult = OnExecuteExePackage(pPackages, pVariables, (BYTE*)msg.pvData, msg.cbData);
            break;

        case BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_MSI_PACKAGE:
            hrResult = OnExecuteMsiPackage(hPipe, pPackages, pVariables, (BYTE*)msg.pvData, msg.cbData);
            break;

        case BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_MSP_PACKAGE:
            hrResult = OnExecuteMspPackage(hPipe, pPackages, pVariables, (BYTE*)msg.pvData, msg.cbData);
            break;

        case BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_MSU_PACKAGE:
            hrResult = OnExecuteMsuPackage(pPackages, (BYTE*)msg.pvData, msg.cbData);
            break;

        case BURN_ELEVATION_MESSAGE_TYPE_CLEAN_BUNDLE:
            hrResult = OnCleanBundle(pRegistration, (BYTE*)msg.pvData, msg.cbData);
            break;

        case BURN_ELEVATION_MESSAGE_TYPE_CLEAN_PACKAGE:
            hrResult = OnCleanPackage(pPackages, (BYTE*)msg.pvData, msg.cbData);
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


// internal function definitions

static HRESULT CreateElevatedInfo(
    __inout_z LPWSTR *psczPipeName,
    __inout_z LPWSTR *psczClientToken
    )
{
    HRESULT hr = S_OK;
    RPC_STATUS rs = RPC_S_OK;
    UUID guid = { };
    WCHAR wzGuid[39];

    // Create the unique pipe name.
    rs = ::UuidCreate(&guid);
    hr = HRESULT_FROM_RPC(rs);
    ExitOnFailure(hr, "Failed to create pipe guid.");

    if (!::StringFromGUID2(guid, wzGuid, countof(wzGuid)))
    {
        hr = E_OUTOFMEMORY;
        ExitOnRootFailure(hr, "Failed to convert pipe guid into string.");
    }

    hr = StrAllocFormatted(psczPipeName, L"BurnPipe.%s", wzGuid);
    ExitOnFailure(hr, "Failed to allocate pipe name.");

    // Create the unique client token.
    rs = ::UuidCreate(&guid);
    hr = HRESULT_FROM_RPC(rs);
    ExitOnRootFailure(hr, "Failed to create pipe guid.");

    if (!::StringFromGUID2(guid, wzGuid, countof(wzGuid)))
    {
        hr = E_OUTOFMEMORY;
        ExitOnRootFailure(hr, "Failed to convert pipe guid into string.");
    }

    hr = StrAllocFormatted(psczClientToken, L"%s", wzGuid);
    ExitOnFailure(hr, "Failed to allocate client token.");

LExit:
    return hr;
}

static HRESULT CreateElevatedPipe(
    __in LPCWSTR wzPipeName,
    __out HANDLE* phPipe
    )
{
    HRESULT hr = S_OK;
    LPWSTR sczFullPipeName = NULL;

    hr = StrAllocFormatted(&sczFullPipeName, L"\\\\.\\pipe\\%s", wzPipeName);
    ExitOnFailure1(hr, "Failed to allocate full name of pipe: %S", wzPipeName);

    // TODO: use a different security descriptor to lock down this named pipe even more than it is now.
    // TODO: consider using overlapped IO to do waits on the pipe and still be able to cancel and such.
    *phPipe = ::CreateNamedPipeW(sczFullPipeName, PIPE_ACCESS_DUPLEX | FILE_FLAG_FIRST_PIPE_INSTANCE, PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT, 1, 64 * 1024, 64 * 1024, 1, NULL);
    if (INVALID_HANDLE_VALUE == *phPipe)
    {
        ExitWithLastError1(hr, "Failed to create elevated pipe: %S", sczFullPipeName);
    }

LExit:
    ReleaseStr(sczFullPipeName);
    return hr;
}

static HRESULT CreateElevatedProcess(
    __in_opt HWND hwndParent,
    __in_z LPCWSTR wzExecutable,
    __in_z LPCWSTR wzOptionName,
    __in_z LPCWSTR wzPipeName,
    __in_z LPCWSTR wzToken,
    __out HANDLE* phProcess
    )
{
    HRESULT hr = S_OK;
    LPWSTR sczParameters = NULL;
    OS_VERSION osVersion = OS_VERSION_UNKNOWN;
    DWORD dwServicePack = 0;
    SHELLEXECUTEINFOW info = { };

    hr = StrAllocFormatted(&sczParameters, L"-q -%s %s %s", wzOptionName, wzPipeName, wzToken);
    ExitOnFailure(hr, "Failed to allocate parameters for elevated process.");

    OsGetVersion(&osVersion, &dwServicePack);

    info.cbSize = sizeof(SHELLEXECUTEINFOW);
    info.fMask = SEE_MASK_FLAG_DDEWAIT | SEE_MASK_FLAG_NO_UI | SEE_MASK_NOCLOSEPROCESS;
    info.hwnd = hwndParent;
    info.lpFile = wzExecutable;
    info.lpParameters = sczParameters;
    info.lpVerb = (OS_VERSION_VISTA > osVersion) ? L"open" : L"runas";
    info.nShow = SW_HIDE;

    if (!vpfnShellExecuteExW(&info))
    {
        ExitWithLastError2(hr, "Failed to launch elevated process: %S %S", wzExecutable, sczParameters);
    }

    *phProcess = info.hProcess;
    info.hProcess = NULL;

LExit:
    if (info.hProcess)
    {
        ::CloseHandle(info.hProcess);
    }

    ReleaseStr(sczParameters);
    return hr;
}

static HRESULT ConnectElevatedProcess(
    __in LPCWSTR wzToken,
    __in HANDLE hPipe,
    __in HANDLE hProcess
    )
{
    HRESULT hr = S_OK;
    DWORD cbToken = lstrlenW(wzToken) * sizeof(WCHAR);
    DWORD dwAck = 0;
    DWORD cb = 0;

    UNREFERENCED_PARAMETER(hProcess); // TODO: use the hProcess to determine if the elevated process dies before/while waiting for pipe communcation.

    if (!::ConnectNamedPipe(hPipe, NULL))
    {
        DWORD er = ::GetLastError();
        if (ERROR_PIPE_CONNECTED != er)
        {
            hr = HRESULT_FROM_WIN32(er);
            ExitOnRootFailure(hr, "Failed to connect elevated pipe.");
        }
    }

    // Prove we are the one that created the elevated process by passing the token.
    if (!::WriteFile(hPipe, &cbToken, sizeof(cbToken), &cb, NULL))
    {
        ExitWithLastError(hr, "Failed to write token length to elevated pipe.");
    }

    if (!::WriteFile(hPipe, wzToken, cbToken, &cb, NULL))
    {
        ExitWithLastError(hr, "Failed to write token to elevated pipe.");
    }

    // Wait until the elevated process responds that it is ready to go.
    if (!::ReadFile(hPipe, &dwAck, sizeof(dwAck), &cb, NULL))
    {
        ExitWithLastError(hr, "Failed to read ACK from elevated pipe.");
    }

    if (1 != dwAck)
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
        ExitOnRootFailure1(hr, "Incorrect ACK from elevated pipe: %u", dwAck);
    }

LExit:
    return hr;
}

static HRESULT WriteMessage(
    __in HANDLE hPipe,
    __in DWORD dwMessage,
    __in_bcount_opt(cbData) LPVOID pvData,
    __in DWORD cbData
    )
{
    HRESULT hr = S_OK;
    LPVOID pv = NULL;
    DWORD cb = 0;

    hr = AllocateMessage(dwMessage, pvData, cbData, &pv, &cb);
    ExitOnFailure(hr, "Failed to allocate message to write.");

    // Write the message.
    DWORD cbWrote = 0;
    DWORD cbTotalWritten = 0;
    do
    {
        if (!::WriteFile(hPipe, pv, cb - cbTotalWritten, &cbWrote, NULL))
        {
            ExitWithLastError(hr, "Failed to write message type to pipe.");
        }

        cbTotalWritten += cbWrote;
    } while (cbTotalWritten < cb);

LExit:
    ReleaseMem(pv);
    return hr;
}

static HRESULT AllocateMessage(
    __in DWORD dwMessage,
    __in_bcount_opt(cbData) LPVOID pvData,
    __in DWORD cbData,
    __out_bcount(cb) LPVOID* ppvMessage,
    __out DWORD* cbMessage
    )
{
    HRESULT hr = S_OK;
    LPVOID pv = NULL;
    DWORD cb = 0;

    // If no data was provided, ensure the count of bytes is zero.
    if (!pvData)
    {
        cbData = 0;
    }

    // Allocate the message.
    cb = sizeof(dwMessage) + sizeof(cbData) + cbData;
    pv = MemAlloc(cb, FALSE);
    ExitOnNull(pv, hr, E_OUTOFMEMORY, "Failed to allocate memory for message.");

    memcpy_s(pv, cb, &dwMessage, sizeof(dwMessage));
    memcpy_s(static_cast<BYTE*>(pv) + sizeof(dwMessage), cb - sizeof(dwMessage), &cbData, sizeof(cbData));
    if (cbData)
    {
        memcpy_s(static_cast<BYTE*>(pv) + sizeof(dwMessage) + sizeof(cbData), cb - sizeof(dwMessage) - sizeof(cbData), pvData, cbData);
    }

    *cbMessage = cb;
    *ppvMessage = pv;
    pv = NULL;

LExit:
    ReleaseMem(pv);
    return hr;
}

static HRESULT ProcessCommonPackageMessages(
    __in BURN_ELEVATION_MESSAGE* pMsg,
    __out DWORD* pdwResult
    )
{
    HRESULT hr = S_OK;
    SIZE_T iData = 0;
    LPSTR sczMessage = NULL;

    switch (pMsg->dwMessage)
    {
    case BURN_ELEVATION_MESSAGE_TYPE_LOG:
        iData = 0;

        hr = BuffReadStringAnsi((BYTE*)pMsg->pvData, pMsg->cbData, &iData, &sczMessage);
        ExitOnFailure(hr, "Failed to read log message.");

        hr = LogStringWorkRaw(sczMessage);
        ExitOnFailure1(hr, "Failed to write log message:'%s'.", sczMessage);

        *pdwResult = (DWORD)hr;
        break;

    default:
        hr = E_INVALIDARG;
        ExitOnFailure1(hr, "Invalid message returned:%u", pMsg->dwMessage);
        break;
    }

LExit:
    ReleaseStr(sczMessage);

    return hr;
}

static HRESULT OnExecuteMsiFilesInUse(
    __in LPVOID pvData,
    __in DWORD cbData,
    __in PFN_MSIEXECUTEMESSAGEHANDLER pfnMessageHandler,
    __in LPVOID pvContext,
    __out DWORD* pdwResult
    )
{
    HRESULT hr = S_OK;
    SIZE_T iData = 0;
    DWORD cFiles = 0;
    LPWSTR* rgwzFiles = NULL;
    WIU_MSI_EXECUTE_MESSAGE message = { };

    // read message parameters
    hr = BuffReadNumber((BYTE*)pvData, cbData, &iData, &cFiles);
    ExitOnFailure(hr, "Failed to read file count.");

    rgwzFiles = (LPWSTR*)MemAlloc(sizeof(LPWSTR*) * cFiles, TRUE);
    ExitOnNull(rgwzFiles, hr, E_OUTOFMEMORY, "Failed to allocate buffer.");

    for (DWORD i = 0; i < cFiles; ++i)
    {
        hr = BuffReadString((BYTE*)pvData, cbData, &iData, &rgwzFiles[i]);
        ExitOnFailure1(hr, "Failed to read file name: %u", i);
    }

    // send message
    message.type = WIU_MSI_EXECUTE_MESSAGE_MSI_MESSAGE;
    message.msiFilesInUse.cFiles = cFiles;
    message.msiFilesInUse.rgwzFiles = (LPCWSTR*)rgwzFiles;
    *pdwResult = (DWORD)pfnMessageHandler(&message, pvContext);

LExit:
    if (rgwzFiles)
    {
        for (DWORD i = 0; i <= cFiles; ++i)
        {
            ReleaseStr(rgwzFiles[i]);
        }
        MemFree(rgwzFiles);
    }

    return hr;
}

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
    hr =  RegistrationSessionBegin(pRegistration, pUserExperience, (BOOTSTRAPPER_ACTION)action, qwEstimatedSize, TRUE);
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
    hr = RegistrationSessionSuspend(pRegistration, (BOOTSTRAPPER_ACTION)action, (BOOL)fReboot, TRUE);
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
    hr = RegistrationSessionResume(pRegistration, (BOOTSTRAPPER_ACTION)action, TRUE);
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
    hr = RegistrationSessionEnd(pRegistration, (BOOTSTRAPPER_ACTION)action, (BOOL)fRollback, TRUE);
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

static HRESULT OnCachePayload(
    __in BURN_PACKAGES* pPackages,
    __in BURN_PAYLOADS* pPayloads,
    __in BYTE* pbData,
    __in DWORD cbData
    )
{
    HRESULT hr = S_OK;
    SIZE_T iData = 0;
    LPWSTR scz = NULL;
    BURN_PACKAGE* pPackage = NULL;
    BURN_PAYLOAD* pPayload = NULL;
    LPWSTR sczUnverifiedPayloadPath = NULL;
    BOOL fMove = FALSE;

    // deserialize message data
    hr = BuffReadString(pbData, cbData, &iData, &scz);
    ExitOnFailure(hr, "Failed to read package id.");

    hr = PackageFindById(pPackages, scz, &pPackage);
    ExitOnFailure1(hr, "Failed to find package: %ls", scz);

    hr = BuffReadString(pbData, cbData, &iData, &scz);
    ExitOnFailure(hr, "Failed to read payload id.");

    hr = PayloadFindById(pPayloads, scz, &pPayload);
    ExitOnFailure1(hr, "Failed to find payload: %ls", scz);

    hr = BuffReadString(pbData, cbData, &iData, &sczUnverifiedPayloadPath);
    ExitOnFailure(hr, "Failed to read unverified payload path.");

    hr = BuffReadNumber(pbData, cbData, &iData, (DWORD*)&fMove);
    ExitOnFailure(hr, "Failed to read move flag.");

    // cache payload
    hr = CachePayload(pPackage, pPayload, sczUnverifiedPayloadPath, fMove);
    ExitOnFailure(hr, "Failed to cache payload.");

LExit:
    ReleaseStr(scz);
    ReleaseStr(sczUnverifiedPayloadPath);

    return hr;
}

static HRESULT OnExecuteExePackage(
    __in BURN_PACKAGES* pPackages,
    __in BURN_VARIABLES* pVariables,
    __in BYTE* pbData,
    __in DWORD cbData
    )
{
    HRESULT hr = S_OK;
    SIZE_T iData = 0;
    LPWSTR sczPackage = NULL;
    BURN_EXECUTE_ACTION executeAction = { };
    BOOTSTRAPPER_APPLY_RESTART exeRestart = BOOTSTRAPPER_APPLY_RESTART_NONE;

    executeAction.type = BURN_EXECUTE_ACTION_TYPE_EXE_PACKAGE;

    // deserialize message data
    hr = BuffReadString(pbData, cbData, &iData, &sczPackage);
    ExitOnFailure(hr, "Failed to read action.");

    hr = BuffReadNumber(pbData, cbData, &iData, (DWORD*)&executeAction.exePackage.action);
    ExitOnFailure(hr, "Failed to read action.");

    hr = VariableDeserialize(pVariables, pbData, cbData, &iData);
    ExitOnFailure(hr, "Failed to read variables.");

    hr = PackageFindById(pPackages, sczPackage, &executeAction.exePackage.pPackage);
    ExitOnFailure1(hr, "Failed to find package: %ls", sczPackage);

    // execute EXE package
    hr = ExeEngineExecutePackage(&executeAction, pVariables, &exeRestart);
    ExitOnFailure(hr, "Failed to execute EXE package.");

LExit:
    ReleaseStr(sczPackage);
    PlanUninitializeExecuteAction(&executeAction);

    if (SUCCEEDED(hr))
    {
        if (BOOTSTRAPPER_APPLY_RESTART_REQUIRED == exeRestart)
        {
            hr = HRESULT_FROM_WIN32(ERROR_SUCCESS_REBOOT_REQUIRED);
        }
        else if (BOOTSTRAPPER_APPLY_RESTART_INITIATED == exeRestart)
        {
            hr = HRESULT_FROM_WIN32(ERROR_SUCCESS_REBOOT_INITIATED);
        }
    }

    return hr;
}

static HRESULT OnExecuteMsiPackage(
    __in HANDLE hPipe,
    __in BURN_PACKAGES* pPackages,
    __in BURN_VARIABLES* pVariables,
    __in BYTE* pbData,
    __in DWORD cbData
    )
{
    HRESULT hr = S_OK;
    SIZE_T iData = 0;
    LPWSTR sczPackage = NULL;
    BOOL fRollback = 0;
    BURN_EXECUTE_ACTION executeAction = { };
    BOOTSTRAPPER_APPLY_RESTART msiRestart = BOOTSTRAPPER_APPLY_RESTART_NONE;

    executeAction.type = BURN_EXECUTE_ACTION_TYPE_MSI_PACKAGE;

    // deserialize message data
    hr = BuffReadString(pbData, cbData, &iData, &sczPackage);
    ExitOnFailure(hr, "Failed to read action.");

    hr = PackageFindById(pPackages, sczPackage, &executeAction.msiPackage.pPackage);
    ExitOnFailure1(hr, "Failed to find package: %ls", sczPackage);

    hr = BuffReadString(pbData, cbData, &iData, &executeAction.msiPackage.sczLogPath);
    ExitOnFailure(hr, "Failed to read package log.");

    hr = BuffReadNumber(pbData, cbData, &iData, (DWORD*)&executeAction.msiPackage.action);
    ExitOnFailure(hr, "Failed to read action.");

    if (executeAction.msiPackage.pPackage->Msi.cFeatures)
    {
        executeAction.msiPackage.rgFeatures = (BOOTSTRAPPER_FEATURE_ACTION*)MemAlloc(executeAction.msiPackage.pPackage->Msi.cFeatures * sizeof(BOOTSTRAPPER_FEATURE_ACTION), TRUE);
        ExitOnNull(executeAction.msiPackage.rgFeatures, hr, E_OUTOFMEMORY, "Failed to allocate memory for feature actions.");

        for (DWORD i = 0; i < executeAction.msiPackage.pPackage->Msi.cFeatures; ++i)
        {
            hr = BuffReadNumber(pbData, cbData, &iData, (DWORD*)&executeAction.msiPackage.rgFeatures[i]);
            ExitOnFailure(hr, "Failed to read feature action.");
        }
    }

    // TODO: patches

    hr = VariableDeserialize(pVariables, pbData, cbData, &iData);
    ExitOnFailure(hr, "Failed to read variables.");

    hr = BuffReadNumber(pbData, cbData, &iData, (DWORD*)&fRollback);
    ExitOnFailure(hr, "Failed to read rollback flag.");

    // execute MSI package
    hr = MsiEngineExecutePackage(&executeAction, pVariables, fRollback, MsiExecuteMessageHandler, hPipe, &msiRestart);
    ExitOnFailure(hr, "Failed to execute MSI package.");

LExit:
    ReleaseStr(sczPackage);
    PlanUninitializeExecuteAction(&executeAction);

    if (SUCCEEDED(hr))
    {
        if (BOOTSTRAPPER_APPLY_RESTART_REQUIRED == msiRestart)
        {
            hr = HRESULT_FROM_WIN32(ERROR_SUCCESS_REBOOT_REQUIRED);
        }
        else if (BOOTSTRAPPER_APPLY_RESTART_INITIATED == msiRestart)
        {
            hr = HRESULT_FROM_WIN32(ERROR_SUCCESS_REBOOT_INITIATED);
        }
    }

    return hr;
}

static HRESULT OnExecuteMspPackage(
    __in HANDLE hPipe,
    __in BURN_PACKAGES* pPackages,
    __in BURN_VARIABLES* pVariables,
    __in BYTE* pbData,
    __in DWORD cbData
    )
{
    HRESULT hr = S_OK;
    SIZE_T iData = 0;
    LPWSTR sczPackage = NULL;
    BOOL fRollback = 0;
    BURN_EXECUTE_ACTION executeAction = { };
    BOOTSTRAPPER_APPLY_RESTART restart = BOOTSTRAPPER_APPLY_RESTART_NONE;

    executeAction.type = BURN_EXECUTE_ACTION_TYPE_MSP_TARGET;

    // deserialize message data
    hr = BuffReadString(pbData, cbData, &iData, &sczPackage);
    ExitOnFailure(hr, "Failed to read action.");

    hr = PackageFindById(pPackages, sczPackage, &executeAction.mspTarget.pPackage);
    ExitOnFailure1(hr, "Failed to find package: %ls", sczPackage);

    executeAction.mspTarget.fPerMachineTarget = TRUE; // we're in the elevated process, clearly we're targeting a per-machine product.

    hr = BuffReadString(pbData, cbData, &iData, &executeAction.mspTarget.sczTargetProductCode);
    ExitOnFailure(hr, "Failed to read package log.");

    hr = BuffReadString(pbData, cbData, &iData, &executeAction.mspTarget.sczLogPath);
    ExitOnFailure(hr, "Failed to read package log.");

    hr = BuffReadNumber(pbData, cbData, &iData, (DWORD*)&executeAction.mspTarget.action);
    ExitOnFailure(hr, "Failed to read action.");

    hr = BuffReadNumber(pbData, cbData, &iData, (DWORD*)&executeAction.mspTarget.cOrderedPatches);
    ExitOnFailure(hr, "Failed to read count of ordered patches.");

    if (executeAction.mspTarget.cOrderedPatches)
    {
        executeAction.mspTarget.rgOrderedPatches = (BURN_ORDERED_PATCHES*)MemAlloc(executeAction.mspTarget.cOrderedPatches * sizeof(BURN_ORDERED_PATCHES), TRUE);
        ExitOnNull(executeAction.mspTarget.rgOrderedPatches, hr, E_OUTOFMEMORY, "Failed to allocate memory for ordered patches.");

        for (DWORD i = 0; i < executeAction.mspTarget.cOrderedPatches; ++i)
        {
            hr = BuffReadNumber(pbData, cbData, &iData, &executeAction.mspTarget.rgOrderedPatches->dwOrder);
            ExitOnFailure(hr, "Failed to read ordered patch order number.");

            hr = BuffReadString(pbData, cbData, &iData, &sczPackage);
            ExitOnFailure(hr, "Failed to read ordered patch package id.");

            hr = PackageFindById(pPackages, sczPackage, &executeAction.mspTarget.rgOrderedPatches->pPackage);
            ExitOnFailure1(hr, "Failed to find ordered patch package: %ls", sczPackage);
        }
    }

    hr = VariableDeserialize(pVariables, pbData, cbData, &iData);
    ExitOnFailure(hr, "Failed to read variables.");

    hr = BuffReadNumber(pbData, cbData, &iData, (DWORD*)&fRollback);
    ExitOnFailure(hr, "Failed to read rollback flag.");

    // execute MSP package
    hr = MspEngineExecutePackage(&executeAction, pVariables, fRollback, MsiExecuteMessageHandler, hPipe, &restart);
    ExitOnFailure(hr, "Failed to execute MSP package.");

LExit:
    ReleaseStr(sczPackage);
    PlanUninitializeExecuteAction(&executeAction);

    if (SUCCEEDED(hr))
    {
        if (BOOTSTRAPPER_APPLY_RESTART_REQUIRED == restart)
        {
            hr = HRESULT_FROM_WIN32(ERROR_SUCCESS_REBOOT_REQUIRED);
        }
        else if (BOOTSTRAPPER_APPLY_RESTART_INITIATED == restart)
        {
            hr = HRESULT_FROM_WIN32(ERROR_SUCCESS_REBOOT_INITIATED);
        }
    }

    return hr;
}

static int MsiExecuteMessageHandler(
    __in WIU_MSI_EXECUTE_MESSAGE* pMessage,
    __in_opt LPVOID pvContext
    )
{
    HRESULT hr = S_OK;
    int nResult = IDOK;
    HANDLE hPipe = (HANDLE)pvContext;
    BYTE* pbData = NULL;
    SIZE_T cbData = 0;
    DWORD dwMessage = 0;

    switch (pMessage->type)
    {
    case WIU_MSI_EXECUTE_MESSAGE_PROGRESS:
        // serialize message data
        hr = BuffWriteNumber(&pbData, &cbData, pMessage->progress.dwPercentage);
        ExitOnFailure(hr, "Failed to write error code to message buffer.");

        // set message id
        dwMessage = BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_PROGRESS;
        break;

    case WIU_MSI_EXECUTE_MESSAGE_ERROR:
        // serialize message data
        hr = BuffWriteNumber(&pbData, &cbData, pMessage->error.dwErrorCode);
        ExitOnFailure(hr, "Failed to write error code to message buffer.");

        hr = BuffWriteNumber(&pbData, &cbData, (DWORD)pMessage->error.uiFlags);
        ExitOnFailure(hr, "Failed to write UI flags to message buffer.");

        hr = BuffWriteString(&pbData, &cbData, pMessage->error.wzMessage);
        ExitOnFailure(hr, "Failed to write message to message buffer.");

        // set message id
        dwMessage = BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_ERROR;
        break;

    case WIU_MSI_EXECUTE_MESSAGE_MSI_MESSAGE:
        // serialize message data
        hr = BuffWriteNumber(&pbData, &cbData, (DWORD)pMessage->msiMessage.mt);
        ExitOnFailure(hr, "Failed to write MSI message type to message buffer.");

        hr = BuffWriteNumber(&pbData, &cbData, (DWORD)pMessage->msiMessage.uiFlags);
        ExitOnFailure(hr, "Failed to write UI flags to message buffer.");

        hr = BuffWriteString(&pbData, &cbData, pMessage->msiMessage.wzMessage);
        ExitOnFailure(hr, "Failed to write message to message buffer.");

        // set message id
        dwMessage = BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_MSI_MESSAGE;
        break;

    case WIU_MSI_EXECUTE_MESSAGE_MSI_FILES_IN_USE:
        // serialize message data
        hr = BuffWriteNumber(&pbData, &cbData, pMessage->msiFilesInUse.cFiles);
        ExitOnFailure(hr, "Failed to write file name count to message buffer.");

        for (DWORD i = 0; i < pMessage->msiFilesInUse.cFiles; ++i)
        {
            hr = BuffWriteString(&pbData, &cbData, pMessage->msiFilesInUse.rgwzFiles[i]);
            ExitOnFailure(hr, "Failed to write file name to message buffer.");
        }

        // set message id
        dwMessage = BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_MSI_FILES_IN_USE;
        break;

    default:
        hr = E_UNEXPECTED;
        ExitOnFailure1(hr, "Invalid message type: %d", pMessage->type);
    }

    // send message
    hr = ElevationSendMessage(hPipe, dwMessage, pbData, cbData, (DWORD*)&nResult);
    ExitOnFailure(hr, "Failed to send message to per-machine process.");

LExit:
    ReleaseBuffer(pbData);

    return nResult;
}

static HRESULT OnExecuteMsuPackage(
    __in BURN_PACKAGES* pPackages,
    __in BYTE* pbData,
    __in DWORD cbData
    )
{
    HRESULT hr = S_OK;
    SIZE_T iData = 0;
    LPWSTR sczPackage = NULL;
    BURN_EXECUTE_ACTION executeAction = { };
    BOOTSTRAPPER_APPLY_RESTART restart = BOOTSTRAPPER_APPLY_RESTART_NONE;

    executeAction.type = BURN_EXECUTE_ACTION_TYPE_MSU_PACKAGE;

    // deserialize message data
    hr = BuffReadString(pbData, cbData, &iData, &sczPackage);
    ExitOnFailure(hr, "Failed to read package id.");

    hr = BuffReadString(pbData, cbData, &iData, &executeAction.msuPackage.sczLogPath);
    ExitOnFailure(hr, "Failed to read package log.");

    hr = BuffReadNumber(pbData, cbData, &iData, (DWORD*)&executeAction.msuPackage.action);
    ExitOnFailure(hr, "Failed to read action.");

    hr = PackageFindById(pPackages, sczPackage, &executeAction.msuPackage.pPackage);
    ExitOnFailure1(hr, "Failed to find package: %ls", sczPackage);

    // execute MSU package
    hr = MsuEngineExecutePackage(&executeAction, &restart);
    ExitOnFailure(hr, "Failed to execute MSU package.");

LExit:
    ReleaseStr(sczPackage);
    PlanUninitializeExecuteAction(&executeAction);

    if (SUCCEEDED(hr))
    {
        if (BOOTSTRAPPER_APPLY_RESTART_REQUIRED == restart)
        {
            hr = HRESULT_FROM_WIN32(ERROR_SUCCESS_REBOOT_REQUIRED);
        }
        else if (BOOTSTRAPPER_APPLY_RESTART_INITIATED == restart)
        {
            hr = HRESULT_FROM_WIN32(ERROR_SUCCESS_REBOOT_INITIATED);
        }
    }

    return hr;
}

static HRESULT OnCleanBundle(
    __in BURN_REGISTRATION* pRegistration,
    __in BYTE* pbData,
    __in DWORD cbData
    )
{
    HRESULT hr = S_OK;
    SIZE_T iData = 0;
    LPWSTR sczBundleId = NULL;
    BURN_RELATED_BUNDLE* pRelatedBundle = NULL;

    // deserialize message data
    hr = BuffReadString(pbData, cbData, &iData, &sczBundleId);
    ExitOnFailure(hr, "Failed to read bundle id.");

    hr = RegistrationLoadRelatedBundle(pRegistration, sczBundleId);
    ExitOnFailure1(hr, "Failed to load registration for bundle: %ls", sczBundleId);

    pRelatedBundle = pRegistration->rgRelatedBundles + pRegistration->cRelatedBundles - 1;

    // Remove the bundle from the cache.
    hr = ApplyCleanBundle(TRUE, pRelatedBundle);
    ExitOnFailure1(hr, "Failed to clean related bundle: %ls", sczBundleId);

LExit:
    ReleaseStr(sczBundleId);
    return hr;
}

static HRESULT OnCleanPackage(
    __in BURN_PACKAGES* pPackages,
    __in BYTE* pbData,
    __in DWORD cbData
    )
{
    HRESULT hr = S_OK;
    SIZE_T iData = 0;
    LPWSTR sczPackage = NULL;
    BURN_PACKAGE* pPackage = NULL;

    // deserialize message data
    hr = BuffReadString(pbData, cbData, &iData, &sczPackage);
    ExitOnFailure(hr, "Failed to read package id.");

    hr = PackageFindById(pPackages, sczPackage, &pPackage);
    ExitOnFailure1(hr, "Failed to find package: %ls", sczPackage);

    // Remove the package from the cache.
    hr = CacheRemovePackage(TRUE, pPackage->sczCacheId);
    ExitOnFailure1(hr, "Failed to remove from cache package: %ls", pPackage->sczId);

LExit:
    ReleaseStr(sczPackage);
    return hr;
}
