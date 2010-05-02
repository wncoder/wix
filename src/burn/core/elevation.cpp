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
    BURN_ELEVATION_MESSAGE msg = { };

    // TODO: change these write/read calls into single TransactMessage (aka: TransactNamedPipe) call.
    hr = WriteMessage(hPipe, dwMessage, pvData, cbData);
    ExitOnFailure(hr, "Failed to write message to parent process.");

    // read result message
    hr = ElevationGetMessage(hPipe, &msg);
    ExitOnFailure(hr, "Failed to read result message from parent process.");

    if (BURN_ELEVATION_MESSAGE_TYPE_COMPLETE != msg.dwMessage || !msg.pvData || sizeof(DWORD) != msg.cbData)
    {
        hr = E_INVALIDARG;
        ExitOnFailure(hr, "Invalid message returned.");
    }

    *pdwResult = *(DWORD*)msg.pvData;

LExit:
    ElevationMessageUninitialize(&msg);

    return hr;
}

/*******************************************************************
 ElevationParentProcessConnect - Called from the per-user process create the per-machine
                                 process and set up the communication pipe.

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
    HANDLE hPipe = INVALID_HANDLE_VALUE;
    HANDLE hProcess = NULL;

    hr = CreateElevatedInfo(&sczPipeName, &sczClientToken);
    ExitOnFailure(hr, "Failed to allocate elevation information.");

    hr = PathForCurrentProcess(&sczBurnPath, NULL);
    ExitOnFailure(hr, "Failed to get current process path.");

    hr = ParentProcessConnect(hwndParent, sczBurnPath, L"elevated", sczPipeName, sczClientToken, phElevatedProcess, phElevatedPipe);
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
    __in BURN_ACTION action,
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
    __in BURN_ACTION action,
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
    __in BURN_ACTION action
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
    __in BURN_ACTION action,
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

    hr = StrAllocFormatted(psczPipeName, L"BurnPipe", wzGuid); // L"BurnPipe.%s" TODO: fix
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

    hr = StrAllocFormatted(psczClientToken, L"%s", L"{D3C7657C-4DF6-434a-8F91-AC0C565A074C}"); //wzGuid TODO: fix
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

    hr = StrAllocFormatted(&sczParameters, L"-q -burn.%s %s %s", wzOptionName, wzPipeName, wzToken);
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
