//-------------------------------------------------------------------------------------------------
// <copyright file="embedded.cpp" company="Microsoft">
//    Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// 
// <summary>
//    Module: Embedded
//
//    Burn embedded process handler.
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"


// struct

struct BURN_EMBEDDED_CALLBACK_CONTEXT
{
    HANDLE hElevatedPipe;
    IBootstrapperApplication* pUX;
    BURN_PACKAGE* pPackage;
};

// internal function declarations

static HRESULT ProcessEmbeddedMessages(
    __in BURN_PIPE_MESSAGE* pMsg,
    __in_opt LPVOID pvContext,
    __out DWORD* pdwResult
    );
static HRESULT LaunchElevatedChildProcess(
    __in BURN_EMBEDDED_CALLBACK_CONTEXT* pContext,
    __in_bcount(cbData) BYTE* pbData,
    __in DWORD cbData,
    __out DWORD* pdwChildPid
    );
static HRESULT ProcessEmbeddedErrorMessage(
    __in BURN_EMBEDDED_CALLBACK_CONTEXT* pContext,
    __in_bcount(cbData) BYTE* pbData,
    __in DWORD cbData,
    __out DWORD* pdwResult
    );
static HRESULT ProcessEmbeddedProgress(
    __in BURN_EMBEDDED_CALLBACK_CONTEXT* pContext,
    __in_bcount(cbData) BYTE* pbData,
    __in DWORD cbData,
    __out DWORD* pdwResult
    );

// function definitions

/*******************************************************************
 EmbeddedLaunchChildProcess - 

*******************************************************************/
extern "C" HRESULT EmbeddedLaunchChildProcess(
    __in BURN_PACKAGE* pPackage,
    __in BURN_USER_EXPERIENCE* pUX,
    __in HANDLE hElevatedPipe,
    __in_z LPCWSTR wzExecutablePath,
    __in_z LPCWSTR wzCommandLine,
    __out HANDLE* phProcess
    )
{
    HRESULT hr = S_OK;
    BURN_EMBEDDED_CALLBACK_CONTEXT context = { };
    HANDLE hPipe = INVALID_HANDLE_VALUE;
    LPWSTR sczPipeName = NULL;
    LPWSTR sczClientToken = NULL;
    LPWSTR sczCommand = NULL;
    STARTUPINFOW si = { };
    PROCESS_INFORMATION pi = { };
    DWORD dwResult = 0;

    context.pPackage = pPackage;
    context.hElevatedPipe = hElevatedPipe;
    context.pUX = pUX->pUserExperience;

    hr = PipeCreatePipeNameAndToken(&hPipe, &sczPipeName, &sczClientToken);
    ExitOnFailure(hr, "Failed to create embedded pipe.");

    hr = StrAllocFormatted(&sczCommand, L"%s -%s %s %s", wzCommandLine, BURN_COMMANDLINE_SWITCH_EMBEDDED, sczPipeName, sczClientToken);
    ExitOnFailure(hr, "Failed to allocate embedded command.");

    if (!::CreateProcessW(wzExecutablePath, sczCommand, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi))
    {
        ExitWithLastError1(hr, "Failed to create embedded process atpath: %ls", wzExecutablePath);
    }

    hr = PipeWaitForChildConnect(hPipe, sczClientToken, pi.hProcess);
    ExitOnFailure(hr, "Failed to wait for embedded process to connect to pipe.");

    hr = PipePumpMessages(hPipe, ProcessEmbeddedMessages, &context, &dwResult);
    ExitOnFailure(hr, "Failed to process messages from embedded message.");

    hr = static_cast<HRESULT>(dwResult);
    ExitOnFailure(hr, "Failure in embedded process.");

    *phProcess = pi.hProcess;
    pi.hProcess = NULL;

LExit:
    ReleaseHandle(pi.hThread);
    ReleaseHandle(pi.hProcess);

    ReleaseStr(sczCommand);
    ReleaseStr(sczClientToken);
    ReleaseStr(sczPipeName);
    ReleaseFileHandle(hPipe);

    return hr;
}

/*******************************************************************
 EmbeddedParentLaunchChildProcess - 

*******************************************************************/
extern "C" HRESULT EmbeddedParentLaunchChildProcess(
    __in HANDLE hParentPipe,
    __in_z LPCWSTR wzPipeName,
    __in_z LPCWSTR wzClientToken,
    __out HANDLE* phElevatedProcess
    )
{
    HRESULT hr = S_OK;
    BYTE* pbData = NULL;
    DWORD cbData = 0;
    DWORD dwElevatedPid = 0;
    HANDLE hElevatedProcess = NULL;

    // Send the pipe name and client token to the parent process so it can
    // create the elevated process for us (without another elevation prompt).
    // Get the elevated process id back as a result.
    hr = BuffWriteString(&pbData, &cbData, wzPipeName);
    ExitOnFailure(hr, "Failed to write pipe name to buffer.");

    hr = BuffWriteString(&pbData, &cbData, wzClientToken);
    ExitOnFailure(hr, "Failed to write client token to buffer.");

    hr = PipeSendMessage(hParentPipe, BURN_EMBEDDED_MESSAGE_TYPE_LAUNCH_CHILD, pbData, cbData, NULL, NULL, &dwElevatedPid);
    ExitOnFailure(hr, "Failed to send embedded launch child message to parent process.");

    // Get a handle to our elevated process via the PID so we can wait for the
    // process to either connect to the pipe or exit (as an error).
    hElevatedProcess = ::OpenProcess(SYNCHRONIZE, FALSE, dwElevatedPid);
    ExitOnNullWithLastError1(hElevatedProcess, hr, "Failed to open embedded elevated process with PID: %u", dwElevatedPid);

    *phElevatedProcess = hElevatedProcess;
    hElevatedProcess = NULL;

LExit:
    ReleaseHandle(hElevatedProcess);

    return hr;
}


// internal function definitions

static HRESULT ProcessEmbeddedMessages(
    __in BURN_PIPE_MESSAGE* pMsg,
    __in_opt LPVOID pvContext,
    __out DWORD* pdwResult
    )
{
    HRESULT hr = S_OK;
    BURN_EMBEDDED_CALLBACK_CONTEXT* pContext = static_cast<BURN_EMBEDDED_CALLBACK_CONTEXT*>(pvContext);
    DWORD dwResult = 0;

    // Process the message.
    switch (pMsg->dwMessage)
    {
    case BURN_EMBEDDED_MESSAGE_TYPE_LAUNCH_CHILD:
        hr = LaunchElevatedChildProcess(pContext, static_cast<BYTE*>(pMsg->pvData), pMsg->cbData, &dwResult);
        ExitOnFailure(hr, "Failed to launch elevated process for child embedded process.");
        break;

    case BURN_EMBEDDED_MESSAGE_TYPE_ERROR:
        hr = ProcessEmbeddedErrorMessage(pContext, static_cast<BYTE*>(pMsg->pvData), pMsg->cbData, &dwResult);
        ExitOnFailure(hr, "Failed to process embedded error message.");
        break;

    case BURN_EMBEDDED_MESSAGE_TYPE_PROGRESS:
        hr = ProcessEmbeddedProgress(pContext, static_cast<BYTE*>(pMsg->pvData), pMsg->cbData, &dwResult);
        ExitOnFailure(hr, "Failed to process embedded progress message.");
        break;

    default:
        hr = E_INVALIDARG;
        ExitOnRootFailure1(hr, "Unexpected embedded message sent to child process, msg: %u", pMsg->dwMessage);
    }

    *pdwResult = dwResult;

LExit:
    return hr;
}

static HRESULT LaunchElevatedChildProcess(
    __in BURN_EMBEDDED_CALLBACK_CONTEXT* pContext,
    __in_bcount(cbData) BYTE* pbData,
    __in DWORD cbData,
    __out DWORD* pdwChildPid
    )
{
    HRESULT hr = S_OK;
    DWORD iData = 0;
    LPWSTR sczPipeName = NULL;
    LPWSTR sczPipeToken = NULL;

    hr = BuffReadString(pbData, cbData, &iData, &sczPipeName);
    ExitOnFailure(hr, "Failed to read pipe name from buffer.");

    hr = BuffReadString(pbData, cbData, &iData, &sczPipeToken);
    ExitOnFailure(hr, "Failed to read pipe token from buffer.");

    hr = ElevationLaunchElevatedChild(pContext->hElevatedPipe, pContext->pPackage, sczPipeName, sczPipeToken, pdwChildPid);
    ExitOnFailure(hr, "Failed to have elevated process launch elevated child for embedded.");

LExit:
    ReleaseStr(sczPipeToken);
    ReleaseStr(sczPipeName);

    return hr;
}

static HRESULT ProcessEmbeddedErrorMessage(
    __in BURN_EMBEDDED_CALLBACK_CONTEXT* pContext,
    __in_bcount(cbData) BYTE* pbData,
    __in DWORD cbData,
    __out DWORD* pdwResult
    )
{
    HRESULT hr = S_OK;
    DWORD iData = 0;
    DWORD dwErrorCode = 0;
    LPWSTR sczMessage = NULL;
    DWORD dwUIHint = 0;

    hr = BuffReadNumber(pbData, cbData, &iData, &dwErrorCode);
    ExitOnFailure(hr, "Failed to read error code from buffer.");

    hr = BuffReadString(pbData, cbData, &iData, &sczMessage);
    ExitOnFailure(hr, "Failed to read error message from buffer.");

    hr = BuffReadNumber(pbData, cbData, &iData, &dwUIHint);
    ExitOnFailure(hr, "Failed to read UI hint from buffer.");

    *pdwResult = pContext->pUX->OnError(pContext->pPackage->sczId, dwErrorCode, sczMessage, dwUIHint);

LExit:
    ReleaseStr(sczMessage);

    return hr;
}

static HRESULT ProcessEmbeddedProgress(
    __in BURN_EMBEDDED_CALLBACK_CONTEXT* pContext,
    __in_bcount(cbData) BYTE* pbData,
    __in DWORD cbData,
    __out DWORD* pdwResult
    )
{
    HRESULT hr = S_OK;
    DWORD iData = 0;
    DWORD dwProgressPercentage = 0;
    DWORD dwOverallProgressPercentage = 0;

    hr = BuffReadNumber(pbData, cbData, &iData, &dwProgressPercentage);
    ExitOnFailure(hr, "Failed to read progress from buffer.");

    hr = BuffReadNumber(pbData, cbData, &iData, &dwOverallProgressPercentage);
    ExitOnFailure(hr, "Failed to read overall progress from buffer.");

    *pdwResult = pContext->pUX->OnExecuteProgress(pContext->pPackage->sczId, dwProgressPercentage, dwOverallProgressPercentage);

LExit:
    return hr;
}
