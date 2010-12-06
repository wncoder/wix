//-------------------------------------------------------------------------------------------------
// <copyright file="pipe.cpp" company="Microsoft">
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
//    Burn Client Server pipe communication handler.
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"

static HRESULT AllocatePipeMessage(
    __in DWORD dwMessage,
    __in_bcount_opt(cbData) LPVOID pvData,
    __in DWORD cbData,
    __out_bcount(cb) LPVOID* ppvMessage,
    __out DWORD* cbMessage
    );
static void FreePipeMessage(
    __in BURN_PIPE_MESSAGE *pMsg
    );
static HRESULT WritePipeMessage(
    __in HANDLE hPipe,
    __in DWORD dwMessage,
    __in_bcount_opt(cbData) LPVOID pvData,
    __in DWORD cbData
    );
static HRESULT GetPipeMessage(
    __in HANDLE hPipe,
    __in BURN_PIPE_MESSAGE* pMsg
    );
static HRESULT CreateChildProcess(
    __in BOOL fElevate,
    __in_opt HWND hwndParent,
    __in_z LPCWSTR wzExecutable,
    __in_z LPCWSTR wzOptionName,
    __in_z LPCWSTR wzPipeName,
    __in_z LPCWSTR wzToken,
    __out HANDLE* phProcess
    );


/*******************************************************************
 PipeSendMessage - 

*******************************************************************/
extern "C" HRESULT PipeSendMessage(
    __in HANDLE hPipe,
    __in DWORD dwMessage,
    __in_bcount_opt(cbData) LPVOID pvData,
    __in DWORD cbData,
    __in_opt PFN_PIPE_MESSAGE_CALLBACK pfnCallback,
    __in_opt LPVOID pvContext,
    __out DWORD* pdwResult
    )
{
    HRESULT hr = S_OK;
    DWORD dwResult = 0;

    hr = WritePipeMessage(hPipe, dwMessage, pvData, cbData);
    ExitOnFailure(hr, "Failed to write send message to pipe.");

    hr = PipePumpMessages(hPipe, pfnCallback, pvContext, &dwResult);
    ExitOnFailure(hr, "Failed to pump messages during send message to pipe.");

    *pdwResult = dwResult;

LExit:
    return hr;
}

/*******************************************************************
 PipePumpMessages - 

*******************************************************************/
extern "C" HRESULT PipePumpMessages(
    __in HANDLE hPipe,
    __in_opt PFN_PIPE_MESSAGE_CALLBACK pfnCallback,
    __in_opt LPVOID pvContext,
    __out DWORD* pdwResult
    )
{
    HRESULT hr = S_OK;
    BURN_PIPE_MESSAGE msg = { };
    SIZE_T iData = 0;
    LPSTR sczMessage = NULL;
    DWORD dwResult = 0;

    // Pump messages from child process.
    while (S_OK == (hr = GetPipeMessage(hPipe, &msg)))
    {
        switch (msg.dwMessage)
        {
        case BURN_PIPE_MESSAGE_TYPE_LOG:
            iData = 0;

            hr = BuffReadStringAnsi((BYTE*)msg.pvData, msg.cbData, &iData, &sczMessage);
            ExitOnFailure(hr, "Failed to read log message.");

            hr = LogStringWorkRaw(sczMessage);
            ExitOnFailure1(hr, "Failed to write log message:'%s'.", sczMessage);

            dwResult = static_cast<DWORD>(hr);
            break;

        case BURN_PIPE_MESSAGE_TYPE_COMPLETE:
            if (!msg.pvData || sizeof(DWORD) != msg.cbData)
            {
                hr = E_INVALIDARG;
                ExitOnRootFailure(hr, "No status returned to PipePumpMessages()");
            }

            *pdwResult = *static_cast<DWORD*>(msg.pvData);
            ExitFunction1(hr = S_OK);

        case BURN_PIPE_MESSAGE_TYPE_TERMINATE:
            *pdwResult = 0;
            ExitFunction1(hr = S_OK);

        default:
            if (pfnCallback)
            {
                hr = pfnCallback(&msg, pvContext, &dwResult);
            }
            else
            {
                hr = E_INVALIDARG;
            }
            ExitOnFailure1(hr, "Failed to process message: %u", msg.dwMessage);
            break;
        }

        // post result
        hr = WritePipeMessage(hPipe, static_cast<DWORD>(BURN_PIPE_MESSAGE_TYPE_COMPLETE), &dwResult, sizeof(dwResult));
        ExitOnFailure(hr, "Failed to post result to child process.");

        FreePipeMessage(&msg);
    }
    ExitOnFailure(hr, "Failed to get message over pipe");

    if (S_FALSE == hr)
    {
        hr = S_OK;
    }

LExit:
    ReleaseStr(sczMessage);
    FreePipeMessage(&msg);

    return hr;
}

/*******************************************************************
 PipeCreatePipeNameAndToken - 

*******************************************************************/
extern "C" HRESULT PipeCreatePipeNameAndToken(
    __out HANDLE* phPipe,
    __out_z LPWSTR *psczPipeName,
    __out_z LPWSTR *psczPipeToken
    )
{
    HRESULT hr = S_OK;
    RPC_STATUS rs = RPC_S_OK;
    UUID guid = { };
    WCHAR wzGuid[39];
    LPWSTR sczPipeName = NULL;
    LPWSTR sczFullPipeName = NULL;
    LPWSTR sczPipeToken = NULL;

    // Create the unique pipe name.
    rs = ::UuidCreate(&guid);
    hr = HRESULT_FROM_RPC(rs);
    ExitOnFailure(hr, "Failed to create pipe guid.");

    if (!::StringFromGUID2(guid, wzGuid, countof(wzGuid)))
    {
        hr = E_OUTOFMEMORY;
        ExitOnRootFailure(hr, "Failed to convert pipe guid into string.");
    }

    hr = StrAllocFormatted(&sczPipeName, L"BurnPipe.%s", wzGuid);
    ExitOnFailure(hr, "Failed to allocate pipe name.");

    hr = StrAllocFormatted(&sczFullPipeName, L"\\\\.\\pipe\\%ls", sczPipeName);
    ExitOnFailure1(hr, "Failed to allocate full name of pipe: %ls", sczPipeName);

    // Create the unique client token.
    rs = ::UuidCreate(&guid);
    hr = HRESULT_FROM_RPC(rs);
    ExitOnRootFailure(hr, "Failed to create pipe guid.");

    if (!::StringFromGUID2(guid, wzGuid, countof(wzGuid)))
    {
        hr = E_OUTOFMEMORY;
        ExitOnRootFailure(hr, "Failed to convert pipe guid into string.");
    }

    hr = StrAllocFormatted(&sczPipeToken, L"%ls", wzGuid);
    ExitOnFailure(hr, "Failed to allocate client token.");

    // TODO: use a different security descriptor to lock down this named pipe even more than it is now.
    // TODO: consider using overlapped IO to do waits on the pipe and still be able to cancel and such.
    *phPipe = ::CreateNamedPipeW(sczFullPipeName, PIPE_ACCESS_DUPLEX | FILE_FLAG_FIRST_PIPE_INSTANCE, PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT, 1, 64 * 1024, 64 * 1024, 1, NULL);
    if (INVALID_HANDLE_VALUE == *phPipe)
    {
        ExitWithLastError1(hr, "Failed to create pipe: %ls", sczFullPipeName);
    }

    *psczPipeName = sczPipeName;
    sczPipeName = NULL;
    *psczPipeToken = sczPipeToken;
    sczPipeToken = NULL;

LExit:
    ReleaseStr(sczPipeToken);
    ReleaseStr(sczFullPipeName);
    ReleaseStr(sczPipeName);

    return hr;
}

/*******************************************************************
 PipeParentProcessConnect - Called from the per-user process to create
                            the per-machine process and set up the
                            communication pipe.

*******************************************************************/
extern "C" HRESULT PipeLaunchChildProcess(
    __in BOOL fElevate,
    __in_z LPWSTR sczPipeName,
    __in_z LPWSTR sczPipeToken,
    __in_opt HWND hwndParent,
    __out HANDLE* phChildProcess
    )
{
    HRESULT hr = S_OK;
    LPWSTR sczBurnPath = NULL;
    HANDLE hProcess = NULL;

    hr = PathForCurrentProcess(&sczBurnPath, NULL);
    ExitOnFailure(hr, "Failed to get current process path.");

    hr = CreateChildProcess(fElevate, hwndParent, sczBurnPath, BURN_COMMANDLINE_SWITCH_ELEVATED, sczPipeName, sczPipeToken, &hProcess);
    ExitOnFailure(hr, "Failed to create process.");

    *phChildProcess = hProcess;
    hProcess = NULL;

LExit:
    ReleaseHandle(hProcess);
    ReleaseStr(sczBurnPath);

    return hr;
}

/*******************************************************************
 PipeWaitForChildConnect - 

*******************************************************************/
extern "C" HRESULT PipeWaitForChildConnect(
    __in HANDLE hPipe,
    __in LPCWSTR wzToken,
    __in HANDLE hProcess
    )
{
    HRESULT hr = S_OK;
    DWORD cbToken = lstrlenW(wzToken) * sizeof(WCHAR);
    DWORD dwAck = 0;
    DWORD cb = 0;

    UNREFERENCED_PARAMETER(hProcess); // TODO: use the hProcess to determine if the child process dies before/while waiting for pipe communcation.

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

/*******************************************************************
 PipeTerminateChildProcess - 

*******************************************************************/
extern "C" HRESULT PipeTerminateChildProcess(
    __in HANDLE hProcess,
    __in HANDLE hPipe
    )
{
    HRESULT hr = S_OK;

    hr = WritePipeMessage(hPipe, static_cast<DWORD>(BURN_PIPE_MESSAGE_TYPE_TERMINATE), NULL, 0);
    ExitOnFailure(hr, "Failed to post terminate message to child process.");

    if (WAIT_FAILED == ::WaitForSingleObject(hProcess, INFINITE))
    {
        ExitWithLastError(hr, "Failed to wait for child process exit.");
    }

LExit:
    return hr;
}

/*******************************************************************
 PipeChildConnect - Called from the child process to connect back
                    to the pipe provided by the parent process.

*******************************************************************/
extern "C" HRESULT PipeChildConnect(
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
        ExitWithLastError1(hr, "Failed to open parent pipe: %ls", sczPipeName)
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
 PipeChildConnected - 

*******************************************************************/
extern "C" HRESULT PipeChildConnected(
    HANDLE hPipe
    )
{
    HRESULT hr = S_OK;
    DWORD dwAck = 1;
    DWORD cb = 0;

    if (!::WriteFile(hPipe, &dwAck, sizeof(dwAck), &cb, NULL))
    {
        ExitWithLastError(hr, "Failed to inform parent process that child is running.");
    }

LExit:
    return hr;
}


static HRESULT AllocatePipeMessage(
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

static void FreePipeMessage(
    __in BURN_PIPE_MESSAGE *pMsg
    )
{
    if (pMsg->fAllocatedData)
    {
        ReleaseNullMem(pMsg->pvData);
        pMsg->fAllocatedData = FALSE;
    }
}

static HRESULT WritePipeMessage(
    __in HANDLE hPipe,
    __in DWORD dwMessage,
    __in_bcount_opt(cbData) LPVOID pvData,
    __in DWORD cbData
    )
{
    HRESULT hr = S_OK;
    LPVOID pv = NULL;
    DWORD cb = 0;

    hr = AllocatePipeMessage(dwMessage, pvData, cbData, &pv, &cb);
    ExitOnFailure(hr, "Failed to allocate message to write.");

    // Write the message.
    DWORD cbWrote = 0;
    DWORD cbTotalWritten = 0;
    while (cbTotalWritten < cb)
    {
        if (!::WriteFile(hPipe, pv, cb - cbTotalWritten, &cbWrote, NULL))
        {
            ExitWithLastError(hr, "Failed to write message type to pipe.");
        }

        cbTotalWritten += cbWrote;
    }

LExit:
    ReleaseMem(pv);
    return hr;
}

static HRESULT GetPipeMessage(
    __in HANDLE hPipe,
    __in BURN_PIPE_MESSAGE* pMsg
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

static HRESULT CreateChildProcess(
    __in BOOL fElevate,
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

    hr = StrAllocFormatted(&sczParameters, L"-q -%ls %ls %ls", wzOptionName, wzPipeName, wzToken);
    ExitOnFailure(hr, "Failed to allocate parameters for elevated process.");

    OsGetVersion(&osVersion, &dwServicePack);

    info.cbSize = sizeof(SHELLEXECUTEINFOW);
    info.fMask = SEE_MASK_FLAG_DDEWAIT | SEE_MASK_FLAG_NO_UI | SEE_MASK_NOCLOSEPROCESS;
    info.hwnd = hwndParent;
    info.lpFile = wzExecutable;
    info.lpParameters = sczParameters;
    info.lpVerb = (OS_VERSION_VISTA > osVersion) && fElevate ? L"open" : L"runas";
    info.nShow = SW_HIDE;

    if (!vpfnShellExecuteExW(&info))
    {
        ExitWithLastError2(hr, "Failed to launch elevated process: %ls %ls", wzExecutable, sczParameters);
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
