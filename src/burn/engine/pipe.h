//-------------------------------------------------------------------------------------------------
// <copyright file="pipe.h" company="Microsoft">
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

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef enum _BURN_PIPE_MESSAGE_TYPE
{
    BURN_PIPE_MESSAGE_TYPE_LOG = 0xF0000001,
    BURN_PIPE_MESSAGE_TYPE_COMPLETE = 0xF0000002,
    BURN_PIPE_MESSAGE_TYPE_TERMINATE = 0xF0000003,
} BURN_PIPE_MESSAGE_TYPE;

typedef struct _BURN_PIPE_MESSAGE
{
    DWORD dwMessage;
    DWORD cbData;

    BOOL fAllocatedData;
    LPVOID pvData;
} BURN_PIPE_MESSAGE;

typedef HRESULT (*PFN_PIPE_MESSAGE_CALLBACK)(
    __in BURN_PIPE_MESSAGE* pMsg,
    __in_opt LPVOID pvContext,
    __out DWORD* pdwResult
    );


// Common functions.
HRESULT PipeSendMessage(
    __in HANDLE hPipe,
    __in DWORD dwMessage,
    __in_bcount_opt(cbData) LPVOID pvData,
    __in DWORD cbData,
    __in_opt PFN_PIPE_MESSAGE_CALLBACK pfnCallback,
    __in_opt LPVOID pvContext,
    __out DWORD* pdwResult
    );
HRESULT PipePumpMessages(
    __in HANDLE hPipe,
    __in_opt PFN_PIPE_MESSAGE_CALLBACK pfnCallback,
    __in_opt LPVOID pvContext,
    __out DWORD* pdwResult
    );

// Parent functions.
HRESULT PipeCreatePipeNameAndToken(
    __out HANDLE* phPipe,
    __out_opt HANDLE* phCachePipe,
    __out_z LPWSTR *psczPipeName,
    __out_z LPWSTR *psczClientToken
    );
HRESULT PipeLaunchChildProcess(
    __in BOOL fElevate,
    __in_z LPWSTR sczPipeName,
    __in_z LPWSTR sczClientToken,
    __in_opt HWND hwndParent,
    __out HANDLE* phElevationProcess
    );
HRESULT PipeWaitForChildConnect(
    __in HANDLE hPipe,
    __in LPCWSTR wzToken,
    __in HANDLE hProcess
    );
HRESULT PipeTerminateChildProcess(
    __in HANDLE hProcess,
    __in HANDLE hPipe,
    __in HANDLE hCachePipe
    );

// Child functions.
HRESULT PipeChildConnect(
    __in_z LPCWSTR wzPipeName,
    __in_z LPCWSTR wzToken,
    __in BOOL fCachePipe,
    __out HANDLE* phPipe
    );

#ifdef __cplusplus
}
#endif
