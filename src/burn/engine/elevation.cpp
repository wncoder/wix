//-------------------------------------------------------------------------------------------------
// <copyright file="elevation.cpp" company="Microsoft">
//    Copyright (c) Microsoft Corporation.  All rights reserved.
//    
//    The use and distribution terms for this software are covered by the
//    Common Public License 1.0 (http://opensource.org/licenses/cpl1.0.php)
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


const DWORD BURN_TIMEOUT = 5 * 60 * 1000; // TODO: is 5 minutes good?

// struct

typedef struct _BURN_ELEVATION_GENERIC_MESSAGE_CONTEXT
{
    PFN_GENERICMESSAGEHANDLER pfnGenericMessageHandler;
    LPVOID pvContext;
} BURN_ELEVATION_GENERIC_MESSAGE_CONTEXT;

typedef struct _BURN_ELEVATION_MSI_MESSAGE_CONTEXT
{
    PFN_MSIEXECUTEMESSAGEHANDLER pfnMessageHandler;
    LPVOID pvContext;
} BURN_ELEVATION_MSI_MESSAGE_CONTEXT;

typedef struct _BURN_ELEVATION_CHILD_MESSAGE_CONTEXT
{
    DWORD dwLoggingTlsId;
    HANDLE hPipe;
    BURN_PACKAGES* pPackages;
    BURN_RELATED_BUNDLES* pRelatedBundles;
    BURN_PAYLOADS* pPayloads;
    BURN_VARIABLES* pVariables;
    BURN_REGISTRATION* pRegistration;
    BURN_USER_EXPERIENCE* pUserExperience;
} BURN_ELEVATION_CHILD_MESSAGE_CONTEXT;


// internal function declarations

static DWORD WINAPI ElevatedChildCacheThreadProc(
    __in LPVOID lpThreadParameter
    );
static HRESULT WaitForElevatedChildCacheThread(
    __in HANDLE hCacheThread,
    __in DWORD dwExpectedExitCode
    );
static HRESULT ProcessGenericExecuteMessages(
    __in BURN_PIPE_MESSAGE* pMsg,
    __in_opt LPVOID pvContext,
    __out DWORD* pdwResult
    );
static HRESULT ProcessMsiPackageMessages(
    __in BURN_PIPE_MESSAGE* pMsg,
    __in_opt LPVOID pvContext,
    __out DWORD* pdwResult
    );
static HRESULT ProcessElevatedChildMessage(
    __in BURN_PIPE_MESSAGE* pMsg,
    __in_opt LPVOID pvContext,
    __out DWORD* pdwResult
    );
static HRESULT ProcessElevatedChildCacheMessage(
    __in BURN_PIPE_MESSAGE* pMsg,
    __in_opt LPVOID pvContext,
    __out DWORD* pdwResult
    );
static HRESULT ProcessResult(
    __in DWORD dwResult,
    __out BOOTSTRAPPER_APPLY_RESTART* pRestart
    );
static HRESULT OnGenericExecuteFilesInUse(
    __in LPVOID pvData,
    __in DWORD cbData,
    __in PFN_GENERICMESSAGEHANDLER pfnMessageHandler,
    __in LPVOID pvContext,
    __out DWORD* pdwResult
    );
static HRESULT OnSessionBegin(
    __in BURN_REGISTRATION* pRegistration,
    __in BURN_VARIABLES* pVariables,
    __in BURN_USER_EXPERIENCE* pUserExperience,
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
    __in HANDLE hPipe,
    __in BURN_PACKAGES* pPackages,
    __in BURN_RELATED_BUNDLES* pRelatedBundles,
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
static HRESULT OnExecuteMsuPackage(
    __in HANDLE hPipe,
    __in BURN_PACKAGES* pPackages,
    __in BYTE* pbData,
    __in DWORD cbData
    );
static HRESULT OnExecuteDependencyAction(
    __in BURN_PACKAGES* pPackages,
    __in BYTE* pbData,
    __in DWORD cbData
    );
static int GenericExecuteMessageHandler(
    __in GENERIC_EXECUTE_MESSAGE* pMessage,
    __in LPVOID pvContext
    );
static int MsiExecuteMessageHandler(
    __in WIU_MSI_EXECUTE_MESSAGE* pMessage,
    __in_opt LPVOID pvContext
    );
static HRESULT OnCleanPackage(
    __in BURN_PACKAGES* pPackages,
    __in BYTE* pbData,
    __in DWORD cbData
    );
static HRESULT OnDetectRelatedBundles(
    __in BURN_REGISTRATION *pRegistration,
    __in BYTE* pbData,
    __in DWORD cbData
    );


// function definitions

extern "C" HRESULT ElevationElevate(
    __in BURN_ENGINE_STATE* pEngineState,
    __in HWND hwndParent
    )
{
    Assert(BURN_MODE_ELEVATED != pEngineState->mode);
    Assert(!pEngineState->companionConnection.sczName);
    Assert(!pEngineState->companionConnection.sczSecret);
    Assert(!pEngineState->companionConnection.hProcess);
    Assert(!pEngineState->companionConnection.dwProcessId);
    Assert(INVALID_HANDLE_VALUE == pEngineState->companionConnection.hPipe);
    Assert(INVALID_HANDLE_VALUE == pEngineState->companionConnection.hCachePipe);

    HRESULT hr = S_OK;
    int nResult = IDOK;
    LPWSTR sczEngineWorkingPath = NULL;
    HANDLE hPipesCreatedEvent = INVALID_HANDLE_VALUE;

    nResult = pEngineState->userExperience.pUserExperience->OnElevate();
    hr = HRESULT_FROM_VIEW(nResult);
    ExitOnRootFailure(hr, "UX aborted elevation requirement.");

    hr = CacheBundleToWorkingDirectory(pEngineState->registration.sczId, &pEngineState->userExperience.payloads, &pEngineState->section, &sczEngineWorkingPath);
    ExitOnFailure(hr, "Failed to cache engine to working directory.");

    hr = PipeCreateNameAndSecret(&pEngineState->companionConnection.sczName, &pEngineState->companionConnection.sczSecret);
    ExitOnFailure(hr, "Failed to create pipe name and client token.");

    hr = PipeCreatePipes(&pEngineState->companionConnection, TRUE, &hPipesCreatedEvent);
    ExitOnFailure(hr, "Failed to create pipe and cache pipe.");

    do
    {
        nResult = IDOK;

        // Create the elevated process and if successful, wait for it to connect.
        hr = PipeLaunchChildProcess(sczEngineWorkingPath, &pEngineState->companionConnection, TRUE, hwndParent);
        if (SUCCEEDED(hr))
        {
            hr = PipeWaitForChildConnect(&pEngineState->companionConnection);
            ExitOnFailure(hr, "Failed to connect to elevated child process.");
        }
        else if (HRESULT_FROM_WIN32(ERROR_CANCELLED) == hr ||   // the user clicked "Cancel" on the elevation prompt or
                 HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED) == hr) // the elevation prompt timed out, provide the notification with the option to retry.
        {
            nResult = pEngineState->userExperience.pUserExperience->OnError(NULL, ERROR_CANCELLED, NULL, MB_ICONERROR | MB_RETRYCANCEL, 0, NULL);
        }
    } while (IDRETRY == nResult);
    ExitOnFailure(hr, "Failed to elevate.");

LExit:
    ReleaseHandle(hPipesCreatedEvent);
    ReleaseStr(sczEngineWorkingPath);

    if (FAILED(hr))
    {
        PipeConnectionUninitialize(&pEngineState->companionConnection);
    }

    return hr;
}

/*******************************************************************
 ElevationSessionBegin - 

*******************************************************************/
extern "C" HRESULT ElevationSessionBegin(
    __in HANDLE hPipe,
    __in BOOTSTRAPPER_ACTION action,
    __in BURN_VARIABLES* pVariables,
    __in DWORD64 qwEstimatedSize,
    __in_z LPCWSTR wzResumeCommandLine
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

    hr = BuffWriteString(&pbData, &cbData, wzResumeCommandLine);
    ExitOnFailure(hr, "Failed to write resume command line to message buffer.");

    hr = VariableSerialize(pVariables, FALSE, &pbData, &cbData);
    ExitOnFailure(hr, "Failed to write variables.");

    // send message
    hr = PipeSendMessage(hPipe, BURN_ELEVATION_MESSAGE_TYPE_SESSION_BEGIN, pbData, cbData, NULL, NULL, &dwResult);
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
    __in_z LPCWSTR wzResumeCommandLine
    )
{
    HRESULT hr = S_OK;
    BYTE* pbData = NULL;
    SIZE_T cbData = 0;
    DWORD dwResult = 0;

    // serialize message data
    hr = BuffWriteString(&pbData, &cbData, wzResumeCommandLine);
    ExitOnFailure(hr, "Failed to write resume command line to message buffer.");

    // send message
    hr = PipeSendMessage(hPipe, BURN_ELEVATION_MESSAGE_TYPE_SESSION_RESUME, pbData, cbData, NULL, NULL, &dwResult);
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
    __in BOOL fRollback,
    __in BOOL fSuspend,
    __in BOOTSTRAPPER_APPLY_RESTART restart
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

    hr = BuffWriteNumber(&pbData, &cbData, (DWORD)fSuspend);
    ExitOnFailure(hr, "Failed to write suspend flag to message buffer.");

    hr = BuffWriteNumber(&pbData, &cbData, (DWORD)restart);
    ExitOnFailure(hr, "Failed to write restart enum to message buffer.");

    // send message
    hr = PipeSendMessage(hPipe, BURN_ELEVATION_MESSAGE_TYPE_SESSION_END, pbData, cbData, NULL, NULL, &dwResult);
    ExitOnFailure(hr, "Failed to send message to per-machine process.");

    hr = (HRESULT)dwResult;

LExit:
    ReleaseBuffer(pbData);

    return hr;
}

HRESULT ElevationDetectRelatedBundles(
    __in HANDLE hPipe
    )
{
    HRESULT hr = S_OK;
    BYTE* pbData = NULL;
    SIZE_T cbData = 0;
    DWORD dwResult = 0;

    // send message
    hr = PipeSendMessage(hPipe, BURN_ELEVATION_MESSAGE_TYPE_DETECT_RELATED_BUNDLES, pbData, cbData, NULL, NULL, &dwResult);
    ExitOnFailure(hr, "Failed to send BURN_ELEVATION_MESSAGE_TYPE_DETECT_RELATED_BUNDLES message to per-machine process.");

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
    hr = PipeSendMessage(hPipe, BURN_ELEVATION_MESSAGE_TYPE_SAVE_STATE, pbBuffer, (DWORD)cbBuffer, NULL, NULL, &dwResult);
    ExitOnFailure(hr, "Failed to send message to per-machine process.");

    hr = (HRESULT)dwResult;

LExit:
    return hr;
}

/*******************************************************************
 ElevationCachePayload - 

*******************************************************************/
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
    hr = PipeSendMessage(hPipe, BURN_ELEVATION_MESSAGE_TYPE_CACHE_PAYLOAD, pbData, cbData, NULL, NULL, &dwResult);
    ExitOnFailure(hr, "Failed to send BURN_ELEVATION_MESSAGE_TYPE_CACHE_PAYLOAD message to per-machine process.");

    hr = (HRESULT)dwResult;

LExit:
    ReleaseBuffer(pbData);

    return hr;
}

/*******************************************************************
 ElevationExecuteExePackage - 

*******************************************************************/
extern "C" HRESULT ElevationExecuteExePackage(
    __in HANDLE hPipe,
    __in BURN_EXECUTE_ACTION* pExecuteAction,
    __in BURN_VARIABLES* pVariables,
    __in PFN_GENERICMESSAGEHANDLER pfnGenericMessageHandler,
    __in LPVOID pvContext,
    __out BOOTSTRAPPER_APPLY_RESTART* pRestart
    )
{
    HRESULT hr = S_OK;
    BYTE* pbData = NULL;
    SIZE_T cbData = 0;
    BURN_ELEVATION_GENERIC_MESSAGE_CONTEXT context = { };
    DWORD dwResult = 0;

    // serialize message data
    hr = BuffWriteString(&pbData, &cbData, pExecuteAction->exePackage.pPackage->sczId);
    ExitOnFailure(hr, "Failed to write package id to message buffer.");

    hr = BuffWriteNumber(&pbData, &cbData, (DWORD)pExecuteAction->exePackage.action);
    ExitOnFailure(hr, "Failed to write action to message buffer.");

    hr = VariableSerialize(pVariables, FALSE, &pbData, &cbData);
    ExitOnFailure(hr, "Failed to write variables.");

    // send message
    context.pfnGenericMessageHandler = pfnGenericMessageHandler;
    context.pvContext = pvContext;

    hr = PipeSendMessage(hPipe, BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_EXE_PACKAGE, pbData, cbData, ProcessGenericExecuteMessages, &context, &dwResult);
    ExitOnFailure(hr, "Failed to send BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_EXE_PACKAGE message to per-machine process.");

    hr = ProcessResult(dwResult, pRestart);

LExit:
    ReleaseBuffer(pbData);

    return hr;
}

/*******************************************************************
 ElevationExecuteMsiPackage - 

*******************************************************************/
extern "C" HRESULT ElevationExecuteMsiPackage(
    __in HANDLE hPipe,
    __in_opt HWND hwndParent,
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
    BURN_ELEVATION_MSI_MESSAGE_CONTEXT context = { };
    DWORD dwResult = 0;

    // serialize message data
    // TODO: for patching we might not have a package
    hr = BuffWriteString(&pbData, &cbData, pExecuteAction->msiPackage.pPackage->sczId);
    ExitOnFailure(hr, "Failed to write package id to message buffer.");

    hr = BuffWriteNumber(&pbData, &cbData, (DWORD)hwndParent);
    ExitOnFailure(hr, "Failed to write parent hwnd to message buffer.");

    hr = BuffWriteString(&pbData, &cbData, pExecuteAction->msiPackage.sczLogPath);
    ExitOnFailure(hr, "Failed to write package log to message buffer.");

    hr = BuffWriteNumber(&pbData, &cbData, (DWORD)pExecuteAction->msiPackage.uiLevel);
    ExitOnFailure(hr, "Failed to write UI level to message buffer.");

    hr = BuffWriteNumber(&pbData, &cbData, (DWORD)pExecuteAction->msiPackage.action);
    ExitOnFailure(hr, "Failed to write action to message buffer.");

    for (DWORD i = 0; i < pExecuteAction->msiPackage.pPackage->Msi.cFeatures; ++i)
    {
        hr = BuffWriteNumber(&pbData, &cbData, (DWORD)pExecuteAction->msiPackage.rgFeatures[i]);
        ExitOnFailure(hr, "Failed to write feature action to message buffer.");
    }

    // TODO: patches

    hr = VariableSerialize(pVariables, FALSE, &pbData, &cbData);
    ExitOnFailure(hr, "Failed to write variables.");

    hr = BuffWriteNumber(&pbData, &cbData, (DWORD)fRollback);
    ExitOnFailure(hr, "Failed to write rollback flag to message buffer.");


    // send message
    context.pfnMessageHandler = pfnMessageHandler;
    context.pvContext = pvContext;

    hr = PipeSendMessage(hPipe, BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_MSI_PACKAGE, pbData, cbData, ProcessMsiPackageMessages, &context, &dwResult);
    ExitOnFailure(hr, "Failed to send BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_MSI_PACKAGE message to per-machine process.");

    hr = ProcessResult(dwResult, pRestart);

LExit:
    ReleaseBuffer(pbData);

    return hr;
}

/*******************************************************************
 ElevationExecuteMspPackage - 

*******************************************************************/
extern "C" HRESULT ElevationExecuteMspPackage(
    __in HANDLE hPipe,
    __in_opt HWND hwndParent,
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
    BURN_ELEVATION_MSI_MESSAGE_CONTEXT context = { };
    DWORD dwResult = 0;

    // serialize message data
    hr = BuffWriteString(&pbData, &cbData, pExecuteAction->mspTarget.pPackage->sczId);
    ExitOnFailure(hr, "Failed to write package id to message buffer.");

    hr = BuffWriteNumber(&pbData, &cbData, (DWORD)hwndParent);
    ExitOnFailure(hr, "Failed to write parent hwnd to message buffer.");

    hr = BuffWriteString(&pbData, &cbData, pExecuteAction->mspTarget.sczTargetProductCode);
    ExitOnFailure(hr, "Failed to write target product code to message buffer.");

    hr = BuffWriteString(&pbData, &cbData, pExecuteAction->mspTarget.sczLogPath);
    ExitOnFailure(hr, "Failed to write package log to message buffer.");

    hr = BuffWriteNumber(&pbData, &cbData, (DWORD)pExecuteAction->mspTarget.uiLevel);
    ExitOnFailure(hr, "Failed to write UI level to message buffer.");

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

    hr = VariableSerialize(pVariables, FALSE, &pbData, &cbData);
    ExitOnFailure(hr, "Failed to write variables.");

    hr = BuffWriteNumber(&pbData, &cbData, (DWORD)fRollback);
    ExitOnFailure(hr, "Failed to write rollback flag to message buffer.");

    // send message
    context.pfnMessageHandler = pfnMessageHandler;
    context.pvContext = pvContext;

    hr = PipeSendMessage(hPipe, BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_MSP_PACKAGE, pbData, cbData, ProcessMsiPackageMessages, &context, &dwResult);
    ExitOnFailure(hr, "Failed to send BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_MSP_PACKAGE message to per-machine process.");

    hr = ProcessResult(dwResult, pRestart);

LExit:
    ReleaseBuffer(pbData);

    return hr;
}

/*******************************************************************
 ElevationExecuteMsuPackage - 

*******************************************************************/
extern "C" HRESULT ElevationExecuteMsuPackage(
    __in HANDLE hPipe,
    __in BURN_EXECUTE_ACTION* pExecuteAction,
    __in PFN_GENERICMESSAGEHANDLER pfnGenericMessageHandler,
    __in LPVOID pvContext,
    __out BOOTSTRAPPER_APPLY_RESTART* pRestart
    )
{
    HRESULT hr = S_OK;
    BYTE* pbData = NULL;
    SIZE_T cbData = 0;
    BURN_ELEVATION_GENERIC_MESSAGE_CONTEXT context = { };
    DWORD dwResult = 0;

    // serialize message data
    hr = BuffWriteString(&pbData, &cbData, pExecuteAction->msuPackage.pPackage->sczId);
    ExitOnFailure(hr, "Failed to write package id to message buffer.");

    hr = BuffWriteString(&pbData, &cbData, pExecuteAction->msuPackage.sczLogPath);
    ExitOnFailure(hr, "Failed to write package log to message buffer.");

    hr = BuffWriteNumber(&pbData, &cbData, (DWORD)pExecuteAction->msuPackage.action);
    ExitOnFailure(hr, "Failed to write action to message buffer.");

    // send message
    context.pfnGenericMessageHandler = pfnGenericMessageHandler;
    context.pvContext = pvContext;

    hr = PipeSendMessage(hPipe, BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_MSU_PACKAGE, pbData, cbData, ProcessGenericExecuteMessages, &context, &dwResult);
    ExitOnFailure(hr, "Failed to send BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_MSU_PACKAGE message to per-machine process.");

    hr = ProcessResult(dwResult, pRestart);

LExit:
    ReleaseBuffer(pbData);

    return hr;
}

extern "C" HRESULT ElevationExecuteDependencyAction(
    __in HANDLE hPipe,
    __in BURN_EXECUTE_ACTION* pExecuteAction
    )
{
    HRESULT hr = S_OK;
    BYTE* pbData = NULL;
    SIZE_T cbData = 0;
    DWORD dwResult = 0;
    BOOTSTRAPPER_APPLY_RESTART restart = BOOTSTRAPPER_APPLY_RESTART_NONE;

    // Serialize the message data.
    hr = BuffWriteString(&pbData, &cbData, pExecuteAction->dependency.pPackage->sczId);
    ExitOnFailure(hr, "Failed to write package id to message buffer.");

    hr = BuffWriteString(&pbData, &cbData, pExecuteAction->dependency.sczBundleProviderKey);
    ExitOnFailure(hr, "Failed to write bundle dependency key to message buffer.");

    hr = BuffWriteNumber(&pbData, &cbData, pExecuteAction->dependency.action);
    ExitOnFailure(hr, "Failed to write action to message buffer.");

    // Send the message.
    hr = PipeSendMessage(hPipe, BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_DEPENDENCY, pbData, cbData, NULL, NULL, &dwResult);
    ExitOnFailure(hr, "Failed to send BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_DEPENDENCY message to per-machine process.");

    // Ignore the restart since this action only results in registry writes.
    hr = ProcessResult(dwResult, &restart);

LExit:
    ReleaseBuffer(pbData);

    return hr;
}

/*******************************************************************
 ElevationCleanPackage - 

*******************************************************************/
extern "C" HRESULT ElevationCleanPackage(
    __in HANDLE hPipe,
    __in BURN_PACKAGE* pPackage
    )
{
    HRESULT hr = S_OK;
    BYTE* pbData = NULL;
    SIZE_T cbData = 0;
    DWORD dwResult = 0;

    // serialize message data
    hr = BuffWriteString(&pbData, &cbData, pPackage->sczId);
    ExitOnFailure(hr, "Failed to write clean package id to message buffer.");

    // send message
    hr = PipeSendMessage(hPipe, BURN_ELEVATION_MESSAGE_TYPE_CLEAN_PACKAGE, pbData, cbData, NULL, NULL, &dwResult);
    ExitOnFailure(hr, "Failed to send BURN_ELEVATION_MESSAGE_TYPE_CLEAN_PACKAGE message to per-machine process.");

    hr = (HRESULT)dwResult;

LExit:
    ReleaseBuffer(pbData);

    return hr;
}

/*******************************************************************
 ElevationChildPumpMessages - 

*******************************************************************/
extern "C" HRESULT ElevationChildPumpMessages(
    __in DWORD dwLoggingTlsId,
    __in HANDLE hPipe,
    __in HANDLE hCachePipe,
    __in BURN_PACKAGES* pPackages,
    __in BURN_RELATED_BUNDLES* pRelatedBundles,
    __in BURN_PAYLOADS* pPayloads,
    __in BURN_VARIABLES* pVariables,
    __in BURN_REGISTRATION* pRegistration,
    __in BURN_USER_EXPERIENCE* pUserExperience,
    __out DWORD* pdwChildExitCode
    )
{
    HRESULT hr = S_OK;
    BURN_ELEVATION_CHILD_MESSAGE_CONTEXT cacheContext = { };
    BURN_ELEVATION_CHILD_MESSAGE_CONTEXT context = { };
    HANDLE hCacheThread = NULL;
    DWORD dwResult = 0;

    cacheContext.dwLoggingTlsId = dwLoggingTlsId;
    cacheContext.hPipe = hCachePipe;
    cacheContext.pPackages = pPackages;
    cacheContext.pRelatedBundles = pRelatedBundles;
    cacheContext.pPayloads = pPayloads;
    cacheContext.pVariables = pVariables;
    cacheContext.pRegistration = pRegistration;
    cacheContext.pUserExperience = pUserExperience;

    context.dwLoggingTlsId = dwLoggingTlsId;
    context.hPipe = hPipe;
    context.pPackages = pPackages;
    context.pRelatedBundles = pRelatedBundles;
    context.pPayloads = pPayloads;
    context.pVariables = pVariables;
    context.pRegistration = pRegistration;
    context.pUserExperience = pUserExperience;

    hCacheThread = ::CreateThread(NULL, 0, ElevatedChildCacheThreadProc, &cacheContext, 0, NULL);
    ExitOnNullWithLastError(hCacheThread, hr, "Failed to create elevated cache thread.");

    hr = PipePumpMessages(hPipe, ProcessElevatedChildMessage, &context, &dwResult);
    ExitOnFailure(hr, "Failed to pump messages in child process.");

    // Wait for the cache thread and verify it gets the right result but don't fail if things
    // don't work out.
    WaitForElevatedChildCacheThread(hCacheThread, dwResult);

    *pdwChildExitCode = dwResult;

LExit:
    ReleaseHandle(hCacheThread);

    return hr;
}


// internal function definitions

static DWORD WINAPI ElevatedChildCacheThreadProc(
    __in LPVOID lpThreadParameter
    )
{
    HRESULT hr = S_OK;
    BURN_ELEVATION_CHILD_MESSAGE_CONTEXT* pContext = reinterpret_cast<BURN_ELEVATION_CHILD_MESSAGE_CONTEXT*>(lpThreadParameter);
    BOOL fComInitialized = FALSE;
    DWORD dwResult = 0;

    if (!::TlsSetValue(pContext->dwLoggingTlsId, pContext->hPipe))
    {
        ExitWithLastError(hr, "Failed to set elevated cache pipe into thread local storage for logging.");
    }

    // initialize COM
    hr = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
    ExitOnFailure(hr, "Failed to initialize COM.");
    fComInitialized = TRUE;

    hr = PipePumpMessages(pContext->hPipe, ProcessElevatedChildCacheMessage, pContext, &dwResult);
    ExitOnFailure(hr, "Failed to pump messages in child process.");

    hr = (HRESULT)dwResult;

LExit:
    if (fComInitialized)
    {
        ::CoUninitialize();
    }

    return (DWORD)hr;
}

static HRESULT WaitForElevatedChildCacheThread(
    __in HANDLE hCacheThread,
    __in DWORD dwExpectedExitCode
    )
{
    HRESULT hr = S_OK;
    DWORD dwExitCode = ERROR_SUCCESS;

    if (WAIT_OBJECT_0 != ::WaitForSingleObject(hCacheThread, BURN_TIMEOUT))
    {
        ExitWithLastError(hr, "Failed to wait for cache thread to terminate.");
    }

    if (!::GetExitCodeThread(hCacheThread, &dwExitCode))
    {
        ExitWithLastError(hr, "Failed to get cache thread exit code.");
    }

    AssertSz(dwExitCode == dwExpectedExitCode, "Cache thread should have exited with the expected exit code.");

LExit:
    return hr;
}

static HRESULT ProcessGenericExecuteMessages(
    __in BURN_PIPE_MESSAGE* pMsg,
    __in_opt LPVOID pvContext,
    __out DWORD* pdwResult
    )
{
    HRESULT hr = S_OK;
    SIZE_T iData = 0;
    BURN_ELEVATION_GENERIC_MESSAGE_CONTEXT* pContext = static_cast<BURN_ELEVATION_GENERIC_MESSAGE_CONTEXT*>(pvContext);
    DWORD dwProgress = 0;
    DWORD dwTotal = 0;
    DWORD dwResult = 0;
    GENERIC_EXECUTE_MESSAGE message = { };

    // Process the message.
    switch (pMsg->dwMessage)
    {
    case BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_PROGRESS:
        // read message parameters
        hr = BuffReadNumber((BYTE*)pMsg->pvData, pMsg->cbData, &iData, &dwProgress);
        ExitOnFailure(hr, "Failed to read progress.");

        hr = BuffReadNumber((BYTE*)pMsg->pvData, pMsg->cbData, &iData, &dwTotal);
        ExitOnFailure(hr, "Failed to read total.");

        message.type = GENERIC_EXECUTE_MESSAGE_PROGRESS;
        message.progress.dwPercentage = 100 * dwProgress / dwTotal;
        // send message
        dwResult = (DWORD)pContext->pfnGenericMessageHandler(&message, pContext->pvContext);
        break;

    case BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_FILES_IN_USE:
        hr = OnGenericExecuteFilesInUse(pMsg->pvData, pMsg->cbData, pContext->pfnGenericMessageHandler, pContext->pvContext, &dwResult);
        break;
    default:
        hr = E_INVALIDARG;
        ExitOnRootFailure(hr, "Invalid package message.");
        break;
    }

    *pdwResult = dwResult;

LExit:
    return hr;
}

static HRESULT ProcessMsiPackageMessages(
    __in BURN_PIPE_MESSAGE* pMsg,
    __in_opt LPVOID pvContext,
    __out DWORD* pdwResult
    )
{
    HRESULT hr = S_OK;
    SIZE_T iData = 0;
    WIU_MSI_EXECUTE_MESSAGE message = { };
    DWORD cMsiData = 0;
    LPWSTR* rgwzMsiData = NULL;
    BURN_ELEVATION_MSI_MESSAGE_CONTEXT* pContext = static_cast<BURN_ELEVATION_MSI_MESSAGE_CONTEXT*>(pvContext);
    DWORD dwResult = 0;
    LPWSTR sczMessage = NULL;

    // Read MSI extended message data.
    hr = BuffReadNumber((BYTE*)pMsg->pvData, pMsg->cbData, &iData, &cMsiData);
    ExitOnFailure(hr, "Failed to read MSI data count.");

    if (cMsiData)
    {
        rgwzMsiData = (LPWSTR*)MemAlloc(sizeof(LPWSTR*) * cMsiData, TRUE);
        ExitOnNull(rgwzMsiData, hr, E_OUTOFMEMORY, "Failed to allocate buffer to read MSI data.");

        for (DWORD i = 0; i < cMsiData; ++i)
        {
            hr = BuffReadString((BYTE*)pMsg->pvData, pMsg->cbData, &iData, &rgwzMsiData[i]);
            ExitOnFailure1(hr, "Failed to read MSI data: %u", i);
        }
    }

    // Process the rest of the message.
    switch (pMsg->dwMessage)
    {
    case BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_PROGRESS:
        // read message parameters
        message.type = WIU_MSI_EXECUTE_MESSAGE_PROGRESS;

        hr = BuffReadNumber((BYTE*)pMsg->pvData, pMsg->cbData, &iData, &message.progress.dwPercentage);
        ExitOnFailure(hr, "Failed to read progress.");

        // send message
        dwResult = (DWORD)pContext->pfnMessageHandler(&message, pContext->pvContext);
        break;

    case BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_ERROR:
        // read message parameters
        message.type = WIU_MSI_EXECUTE_MESSAGE_ERROR;

        hr = BuffReadNumber((BYTE*)pMsg->pvData, pMsg->cbData, &iData, &message.error.dwErrorCode);
        ExitOnFailure(hr, "Failed to read error code.");

        hr = BuffReadNumber((BYTE*)pMsg->pvData, pMsg->cbData, &iData, (DWORD*)&message.error.uiFlags);
        ExitOnFailure(hr, "Failed to read UI flags.");

        hr = BuffReadString((BYTE*)pMsg->pvData, pMsg->cbData, &iData, &sczMessage);
        ExitOnFailure(hr, "Failed to read message.");
        message.error.wzMessage = sczMessage;

        // send message
        dwResult = (DWORD)pContext->pfnMessageHandler(&message, pContext->pvContext);
        break;

    case BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_MSI_MESSAGE:
        // read message parameters
        message.type = WIU_MSI_EXECUTE_MESSAGE_MSI_MESSAGE;

        hr = BuffReadNumber((BYTE*)pMsg->pvData, pMsg->cbData, &iData, (DWORD*)&message.msiMessage.mt);
        ExitOnFailure(hr, "Failed to read message type.");

        hr = BuffReadNumber((BYTE*)pMsg->pvData, pMsg->cbData, &iData, (DWORD*)&message.msiMessage.uiFlags);
        ExitOnFailure(hr, "Failed to read UI flags.");

        hr = BuffReadString((BYTE*)pMsg->pvData, pMsg->cbData, &iData, &sczMessage);
        ExitOnFailure(hr, "Failed to read message.");
        message.msiMessage.wzMessage = sczMessage;

        // send message
        dwResult = (DWORD)pContext->pfnMessageHandler(&message, pContext->pvContext);
        break;

    case BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_FILES_IN_USE:
        message.type = WIU_MSI_EXECUTE_MESSAGE_MSI_MESSAGE;
        message.msiFilesInUse.cFiles = cMsiData;
        message.msiFilesInUse.rgwzFiles = (LPCWSTR*)rgwzMsiData;

        // send message
        dwResult = (DWORD)pContext->pfnMessageHandler(&message, pContext->pvContext);
        break;

    default:
        hr = E_INVALIDARG;
        ExitOnRootFailure(hr, "Invalid package message.");
        break;
    }

    *pdwResult = dwResult;

LExit:
    ReleaseStr(sczMessage);

    if (rgwzMsiData)
    {
        for (DWORD i = 0; i < cMsiData; ++i)
        {
            ReleaseStr(rgwzMsiData[i]);
        }

        MemFree(rgwzMsiData);
    }

    return hr;
}

static HRESULT ProcessElevatedChildMessage(
    __in BURN_PIPE_MESSAGE* pMsg,
    __in_opt LPVOID pvContext,
    __out DWORD* pdwResult
    )
{
    HRESULT hr = S_OK;
    BURN_ELEVATION_CHILD_MESSAGE_CONTEXT* pContext = static_cast<BURN_ELEVATION_CHILD_MESSAGE_CONTEXT*>(pvContext);
    HRESULT hrResult = S_OK;
    DWORD dwPid = 0;

    switch (pMsg->dwMessage)
    {
    case BURN_ELEVATION_MESSAGE_TYPE_SESSION_BEGIN:
        hrResult = OnSessionBegin(pContext->pRegistration, pContext->pVariables, pContext->pUserExperience, (BYTE*)pMsg->pvData, pMsg->cbData);
        break;

    case BURN_ELEVATION_MESSAGE_TYPE_SESSION_RESUME:
        hrResult = OnSessionResume(pContext->pRegistration, (BYTE*)pMsg->pvData, pMsg->cbData);
        break;

    case BURN_ELEVATION_MESSAGE_TYPE_SESSION_END:
        hrResult = OnSessionEnd(pContext->pRegistration, (BYTE*)pMsg->pvData, pMsg->cbData);
        break;

    case BURN_ELEVATION_MESSAGE_TYPE_DETECT_RELATED_BUNDLES:
        hrResult = OnDetectRelatedBundles(pContext->pRegistration, (BYTE*)pMsg->pvData, pMsg->cbData);
        break;

    case BURN_ELEVATION_MESSAGE_TYPE_SAVE_STATE:
        hrResult = OnSaveState(pContext->pRegistration, (BYTE*)pMsg->pvData, pMsg->cbData);
        break;

    case BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_EXE_PACKAGE:
        hrResult = OnExecuteExePackage(pContext->hPipe, pContext->pPackages, pContext->pRelatedBundles, pContext->pVariables, (BYTE*)pMsg->pvData, pMsg->cbData);
        break;

    case BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_MSI_PACKAGE:
        hrResult = OnExecuteMsiPackage(pContext->hPipe, pContext->pPackages, pContext->pVariables, (BYTE*)pMsg->pvData, pMsg->cbData);
        break;

    case BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_MSP_PACKAGE:
        hrResult = OnExecuteMspPackage(pContext->hPipe, pContext->pPackages, pContext->pVariables, (BYTE*)pMsg->pvData, pMsg->cbData);
        break;

    case BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_MSU_PACKAGE:
        hrResult = OnExecuteMsuPackage(pContext->hPipe, pContext->pPackages, (BYTE*)pMsg->pvData, pMsg->cbData);
        break;

    case BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_DEPENDENCY:
        hrResult = OnExecuteDependencyAction(pContext->pPackages, (BYTE*)pMsg->pvData, pMsg->cbData);
        break;

    case BURN_ELEVATION_MESSAGE_TYPE_CLEAN_PACKAGE:
        hrResult = OnCleanPackage(pContext->pPackages, (BYTE*)pMsg->pvData, pMsg->cbData);
        break;

    default:
        hr = E_INVALIDARG;
        ExitOnRootFailure1(hr, "Unexpected elevated message sent to child process, msg: %u", pMsg->dwMessage);
    }

    *pdwResult = dwPid ? dwPid : (DWORD)hrResult;

LExit:
    return hr;
}

static HRESULT ProcessElevatedChildCacheMessage(
    __in BURN_PIPE_MESSAGE* pMsg,
    __in_opt LPVOID pvContext,
    __out DWORD* pdwResult
    )
{
    HRESULT hr = S_OK;
    BURN_ELEVATION_CHILD_MESSAGE_CONTEXT* pContext = static_cast<BURN_ELEVATION_CHILD_MESSAGE_CONTEXT*>(pvContext);
    HRESULT hrResult = S_OK;

    switch (pMsg->dwMessage)
    {
    case BURN_ELEVATION_MESSAGE_TYPE_CACHE_PAYLOAD:
        hrResult = OnCachePayload(pContext->pPackages, pContext->pPayloads, (BYTE*)pMsg->pvData, pMsg->cbData);
        break;

    case BURN_ELEVATION_MESSAGE_TYPE_CLEAN_PACKAGE:
        hrResult = OnCleanPackage(pContext->pPackages, (BYTE*)pMsg->pvData, pMsg->cbData);
        break;

    default:
        hr = E_INVALIDARG;
        ExitOnRootFailure1(hr, "Unexpected elevated cache message sent to child process, msg: %u", pMsg->dwMessage);
    }

    *pdwResult = (DWORD)hrResult;

LExit:
    return hr;
}

static HRESULT ProcessResult(
    __in DWORD dwResult,
    __out BOOTSTRAPPER_APPLY_RESTART* pRestart
    )
{
    HRESULT hr = static_cast<HRESULT>(dwResult);
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

    return hr;
}

static HRESULT OnGenericExecuteFilesInUse(
    __in LPVOID pvData,
    __in DWORD cbData,
    __in PFN_GENERICMESSAGEHANDLER pfnMessageHandler,
    __in LPVOID pvContext,
    __out DWORD* pdwResult
    )
{
    HRESULT hr = S_OK;
    SIZE_T iData = 0;
    DWORD cFiles = 0;
    LPWSTR* rgwzFiles = NULL;
    GENERIC_EXECUTE_MESSAGE message = { };

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
    message.type = GENERIC_EXECUTE_MESSAGE_FILES_IN_USE;
    message.filesInUse.cFiles = cFiles;
    message.filesInUse.rgwzFiles = (LPCWSTR*)rgwzFiles;
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
    __in BURN_VARIABLES* pVariables,
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

    hr = BuffReadString(pbData, cbData, &iData, &pRegistration->sczResumeCommandLine);
    ExitOnFailure(hr, "Failed to read resume command line.");

    hr = VariableDeserialize(pVariables, pbData, cbData, &iData);
    ExitOnFailure(hr, "Failed to read variables.");

    // begin session in per-machine process
    hr =  RegistrationSessionBegin(pRegistration, pVariables, pUserExperience, (BOOTSTRAPPER_ACTION)action, qwEstimatedSize, TRUE);
    ExitOnFailure(hr, "Failed to begin registration session.");

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

    // deserialize message data
    hr = BuffReadString(pbData, cbData, &iData, &pRegistration->sczResumeCommandLine);
    ExitOnFailure(hr, "Failed to read resume command line.");

    // suspend session in per-machine process
    hr = RegistrationSessionResume(pRegistration, TRUE);
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
    DWORD dwRollback = 0;
    DWORD dwSuspend = 0;
    DWORD dwRestart = 0;

    // deserialize message data
    hr = BuffReadNumber(pbData, cbData, &iData, &action);
    ExitOnFailure(hr, "Failed to read action.");

    hr = BuffReadNumber(pbData, cbData, &iData, &dwRollback);
    ExitOnFailure(hr, "Failed to read rollback flag.");

    hr = BuffReadNumber(pbData, cbData, &iData, &dwSuspend);
    ExitOnFailure(hr, "Failed to read suspend flag.");

    hr = BuffReadNumber(pbData, cbData, &iData, &dwRestart);
    ExitOnFailure(hr, "Failed to read restart enum.");

    // suspend session in per-machine process
    hr = RegistrationSessionEnd(pRegistration, (BOOTSTRAPPER_ACTION)action, (BOOL)dwRollback, (BOOL)dwSuspend, (BOOTSTRAPPER_APPLY_RESTART)dwRestart, TRUE, NULL);
    ExitOnFailure(hr, "Failed to suspend registration session.");

LExit:
    return hr;
}

static HRESULT OnDetectRelatedBundles(
    __in BURN_REGISTRATION *pRegistration,
    __in BYTE* /*pbData*/,
    __in DWORD /*cbData*/
    )
{
    HRESULT hr = S_OK;

    hr = RegistrationDetectRelatedBundles(TRUE, NULL, pRegistration, NULL);
    ExitOnFailure(hr, "Failed to detect related bundles in elevated process");

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
    hr = CachePayload(pPackage->fPerMachine, pPayload, pPackage->sczCacheId, NULL, sczUnverifiedPayloadPath, fMove);
    ExitOnFailure(hr, "Failed to cache payload.");

LExit:
    ReleaseStr(scz);
    ReleaseStr(sczUnverifiedPayloadPath);

    return hr;
}

static HRESULT OnExecuteExePackage(
    __in HANDLE hPipe,
    __in BURN_PACKAGES* pPackages,
    __in BURN_RELATED_BUNDLES* pRelatedBundles,
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
    ExitOnFailure(hr, "Failed to read exe package.");

    hr = BuffReadNumber(pbData, cbData, &iData, (DWORD*)&executeAction.exePackage.action);
    ExitOnFailure(hr, "Failed to read action.");

    hr = VariableDeserialize(pVariables, pbData, cbData, &iData);
    ExitOnFailure(hr, "Failed to read variables.");

    hr = PackageFindById(pPackages, sczPackage, &executeAction.exePackage.pPackage);
    if (E_NOTFOUND == hr)
    {
        hr = PackageFindRelatedById(pRelatedBundles, sczPackage, &executeAction.exePackage.pPackage);
    }
    ExitOnFailure1(hr, "Failed to find package: %ls", sczPackage);

    // execute EXE package
    hr = ExeEngineExecutePackage(&executeAction, pVariables, GenericExecuteMessageHandler, hPipe, &exeRestart);
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
    HWND hwndParent = NULL;
    BOOL fRollback = 0;
    BURN_EXECUTE_ACTION executeAction = { };
    BOOTSTRAPPER_APPLY_RESTART msiRestart = BOOTSTRAPPER_APPLY_RESTART_NONE;

    executeAction.type = BURN_EXECUTE_ACTION_TYPE_MSI_PACKAGE;

    // deserialize message data
    hr = BuffReadString(pbData, cbData, &iData, &sczPackage);
    ExitOnFailure(hr, "Failed to read action.");

    hr = PackageFindById(pPackages, sczPackage, &executeAction.msiPackage.pPackage);
    ExitOnFailure1(hr, "Failed to find package: %ls", sczPackage);

    hr = BuffReadNumber(pbData, cbData, &iData, (DWORD*)&hwndParent);
    ExitOnFailure(hr, "Failed to read parent hwnd.");

    hr = BuffReadString(pbData, cbData, &iData, &executeAction.msiPackage.sczLogPath);
    ExitOnFailure(hr, "Failed to read package log.");

    hr = BuffReadNumber(pbData, cbData, &iData, (DWORD*)&executeAction.msiPackage.uiLevel);
    ExitOnFailure(hr, "Failed to read UI level.");

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
    hr = MsiEngineExecutePackage(hwndParent, &executeAction, pVariables, fRollback, MsiExecuteMessageHandler, hPipe, &msiRestart);
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
    HWND hwndParent = NULL;
    BOOL fRollback = 0;
    BURN_EXECUTE_ACTION executeAction = { };
    BOOTSTRAPPER_APPLY_RESTART restart = BOOTSTRAPPER_APPLY_RESTART_NONE;

    executeAction.type = BURN_EXECUTE_ACTION_TYPE_MSP_TARGET;

    // deserialize message data
    hr = BuffReadString(pbData, cbData, &iData, &sczPackage);
    ExitOnFailure(hr, "Failed to read action.");

    hr = PackageFindById(pPackages, sczPackage, &executeAction.mspTarget.pPackage);
    ExitOnFailure1(hr, "Failed to find package: %ls", sczPackage);

    hr = BuffReadNumber(pbData, cbData, &iData, (DWORD*)&hwndParent);
    ExitOnFailure(hr, "Failed to read parent hwnd.");

    executeAction.mspTarget.fPerMachineTarget = TRUE; // we're in the elevated process, clearly we're targeting a per-machine product.

    hr = BuffReadString(pbData, cbData, &iData, &executeAction.mspTarget.sczTargetProductCode);
    ExitOnFailure(hr, "Failed to read package log.");

    hr = BuffReadString(pbData, cbData, &iData, &executeAction.mspTarget.sczLogPath);
    ExitOnFailure(hr, "Failed to read package log.");

    hr = BuffReadNumber(pbData, cbData, &iData, (DWORD*)&executeAction.mspTarget.uiLevel);
    ExitOnFailure(hr, "Failed to read UI level.");

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
    hr = MspEngineExecutePackage(hwndParent, &executeAction, pVariables, fRollback, MsiExecuteMessageHandler, hPipe, &restart);
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

static HRESULT OnExecuteMsuPackage(
    __in HANDLE hPipe,
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

    hr = BuffReadNumber(pbData, cbData, &iData, reinterpret_cast<DWORD*>(&executeAction.msuPackage.action));
    ExitOnFailure(hr, "Failed to read action.");

    hr = PackageFindById(pPackages, sczPackage, &executeAction.msuPackage.pPackage);
    ExitOnFailure1(hr, "Failed to find package: %ls", sczPackage);

    // execute MSU package
    hr = MsuEngineExecutePackage(&executeAction, GenericExecuteMessageHandler, hPipe, &restart);
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

static HRESULT OnExecuteDependencyAction(
    __in BURN_PACKAGES* pPackages,
    __in BYTE* pbData,
    __in DWORD cbData
    )
{
    HRESULT hr = S_OK;
    SIZE_T iData = 0;
    LPWSTR sczPackage = NULL;
    BURN_EXECUTE_ACTION executeAction = { };

    executeAction.type = BURN_EXECUTE_ACTION_TYPE_DEPENDENCY;

    // Deserialize the message data.
    hr = BuffReadString(pbData, cbData, &iData, &sczPackage);
    ExitOnFailure(hr, "Failed to read package id from message buffer.");

    hr = BuffReadString(pbData, cbData, &iData, &executeAction.dependency.sczBundleProviderKey);
    ExitOnFailure(hr, "Failed to read bundle dependency key from message buffer.");

    hr = BuffReadNumber(pbData, cbData, &iData, reinterpret_cast<DWORD*>(&executeAction.dependency.action));
    ExitOnFailure(hr, "Failed to read action.");

    // Find the package again.
    hr = PackageFindById(pPackages, sczPackage, &executeAction.dependency.pPackage);
    ExitOnFailure1(hr, "Failed to find the package: %ls", sczPackage);

    // Execute the dependency action.
    hr = DependencyExecuteAction(&executeAction, TRUE);
    ExitOnFailure(hr, "Failed to execute dependency action.");

LExit:
    return hr;
}

static int GenericExecuteMessageHandler(
    __in GENERIC_EXECUTE_MESSAGE* pMessage,
    __in LPVOID pvContext
    )
{
    HRESULT hr = S_OK;
    int nResult = IDOK;
    HANDLE hPipe = (HANDLE)pvContext;
    BYTE* pbData = NULL;
    SIZE_T cbData = 0;

    // serialize message data
    hr = BuffWriteNumber(&pbData, &cbData, pMessage->progress.dwPercentage);
    ExitOnFailure(hr, "Failed to write progress percentage to message buffer.");

    hr = BuffWriteNumber(&pbData, &cbData, 100);
    ExitOnFailure(hr, "Failed to write progress total to message buffer.");

    // send message
    hr = PipeSendMessage(hPipe, BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_PROGRESS, pbData, cbData, NULL, NULL, reinterpret_cast<DWORD*>(&nResult));
    ExitOnFailure(hr, "Failed to send message to per-user process.");

LExit:
    ReleaseBuffer(pbData);

    return nResult;
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

    // Always send any extra data via the struct first.
    hr = BuffWriteNumber(&pbData, &cbData, pMessage->cData);
    ExitOnFailure(hr, "Failed to write MSI data count to message buffer.");

    for (DWORD i = 0; i < pMessage->cData; ++i)
    {
        hr = BuffWriteString(&pbData, &cbData, pMessage->rgwzData[i]);
        ExitOnFailure(hr, "Failed to write MSI data to message buffer.");
    }

    switch (pMessage->type)
    {
    case WIU_MSI_EXECUTE_MESSAGE_PROGRESS:
        // serialize message data
        hr = BuffWriteNumber(&pbData, &cbData, pMessage->progress.dwPercentage);
        ExitOnFailure(hr, "Failed to write progress percentage to message buffer.");

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
        // NOTE: we do not serialize message data here because all our files in use are in the data above.

        // set message id
        dwMessage = BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_FILES_IN_USE;
        break;

    default:
        hr = E_UNEXPECTED;
        ExitOnFailure1(hr, "Invalid message type: %d", pMessage->type);
    }

    // send message
    hr = PipeSendMessage(hPipe, dwMessage, pbData, cbData, NULL, NULL, (DWORD*)&nResult);
    ExitOnFailure(hr, "Failed to send message to per-machine process.");

LExit:
    ReleaseBuffer(pbData);

    return nResult;
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
