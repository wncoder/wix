//-------------------------------------------------------------------------------------------------
// <copyright file="elevatation.h" company="Microsoft">
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
//    Burn elevation process handler.
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once


#ifdef __cplusplus
extern "C" {
#endif

typedef enum _BURN_ELEVATION_MESSAGE_TYPE
{
    BURN_ELEVATION_MESSAGE_TYPE_UNKNOWN,
    BURN_ELEVATION_MESSAGE_TYPE_SESSION_BEGIN,
    BURN_ELEVATION_MESSAGE_TYPE_SESSION_SUSPEND,
    BURN_ELEVATION_MESSAGE_TYPE_SESSION_RESUME,
    BURN_ELEVATION_MESSAGE_TYPE_SESSION_END,
    BURN_ELEVATION_MESSAGE_TYPE_SAVE_STATE,
    BURN_ELEVATION_MESSAGE_TYPE_CACHE_PAYLOAD,
    BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_EXE_PACKAGE,
    BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_MSI_PACKAGE,
    BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_MSU_PACKAGE,
    BURN_ELEVATION_MESSAGE_TYPE_CLEAN_BUNDLE,
    BURN_ELEVATION_MESSAGE_TYPE_CLEAN_PACKAGE,

    BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_PROGRESS,
    BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_ERROR,
    BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_MSI_MESSAGE,
    BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_MSI_FILES_IN_USE,

    BURN_ELEVATION_MESSAGE_TYPE_EXECUTE,
    BURN_ELEVATION_MESSAGE_TYPE_LOG,
    BURN_ELEVATION_MESSAGE_TYPE_COMPLETE,
    BURN_ELEVATION_MESSAGE_TYPE_TERMINATE,
} BURN_ELEVATION_MESSAGE_TYPE;

typedef struct _BURN_ELEVATION_MESSAGE
{
    DWORD dwMessage;
    DWORD cbData;

    BOOL fAllocatedData;
    LPVOID pvData;
} BURN_ELEVATION_MESSAGE;


// Common functions.
void ElevationMessageUninitialize(
    __in BURN_ELEVATION_MESSAGE* pMsg
    );

HRESULT ElevationGetMessage(
    __in HANDLE hPipe,
    __in BURN_ELEVATION_MESSAGE* pMsg
    );
HRESULT ElevationPostMessage(
    __in HANDLE hPipe,
    __in DWORD dwMessage,
    __in_bcount_opt(cbData) LPVOID pvData,
    __in DWORD cbData
    );
HRESULT ElevationSendMessage(
    __in HANDLE hPipe,
    __in DWORD dwMessage,
    __in_bcount_opt(cbData) LPVOID pvData,
    __in DWORD cbData,
    __out DWORD* pdwResult
    );

// Parent (per-user process) side functions.
HRESULT ElevationParentProcessConnect(
    __in HWND hwndParent,
    __out HANDLE* phElevationProcess,
    __out HANDLE* phElevationPipe
    );

HRESULT ParentProcessConnect(
    __in HWND hwndParent,
    __in_z LPCWSTR wzExecutable,
    __in_z LPCWSTR wzOptionName,
    __in_z LPCWSTR wzPipeName,
    __in_z LPCWSTR wzToken,
    __out HANDLE* phElevatedProcess,
    __out HANDLE* phElevatedPipe
    );

HRESULT ElevationParentProcessTerminate(
    __in HANDLE hElevatedProcess,
    __in HANDLE hElevatedPipe
    );

HRESULT ElevationSessionBegin(
    __in HANDLE hPipe,
    __in BOOTSTRAPPER_ACTION action,
    __in DWORD64 qwEstimatedSize
    );
HRESULT ElevationSessionSuspend(
    __in HANDLE hPipe,
    __in BOOTSTRAPPER_ACTION action,
    __in BOOL fReboot
    );
HRESULT ElevationSessionResume(
    __in HANDLE hPipe,
    __in BOOTSTRAPPER_ACTION action
    );
HRESULT ElevationSessionEnd(
    __in HANDLE hPipe,
    __in BOOTSTRAPPER_ACTION action,
    __in BOOL fRollback
    );
HRESULT ElevationSaveState(
    __in HANDLE hPipe,
    __in_bcount(cbBuffer) BYTE* pbBuffer,
    __in SIZE_T cbBuffer
    );
HRESULT ElevationCachePayload(
    __in HANDLE hPipe,
    __in BURN_PACKAGE* pPackage,
    __in BURN_PAYLOAD* pPayload,
    __in_z LPCWSTR wzUnverifiedPayloadPath,
    __in BOOL fMove
    );
HRESULT ElevationExecuteExePackage(
    __in HANDLE hPipe,
    __in BURN_EXECUTE_ACTION* pExecuteAction,
    __in BURN_VARIABLES* pVariables,
    __out BOOTSTRAPPER_APPLY_RESTART* pRestart
    );
HRESULT ElevationExecuteMsiPackage(
    __in HANDLE hPipe,
    __in BURN_EXECUTE_ACTION* pExecuteAction,
    __in BURN_VARIABLES* pVariables,
    __in BOOL fRollback,
    __in PFN_MSIEXECUTEMESSAGEHANDLER pfnMessageHandler,
    __in LPVOID pvContext,
    __out BOOTSTRAPPER_APPLY_RESTART* pRestart
    );
HRESULT ElevationExecuteMsuPackage(
    __in HANDLE hPipe,
    __in BURN_EXECUTE_ACTION* pExecuteAction,
    __out BOOTSTRAPPER_APPLY_RESTART* pRestart
    );
HRESULT ElevationCleanBundle(
    __in HANDLE hPipe,
    __in BURN_CLEAN_ACTION* pCleanAction
    );
HRESULT ElevationCleanPackage(
    __in HANDLE hPipe,
    __in BURN_CLEAN_ACTION* pCleanAction
    );

// Child (per-machine) side functions.
HRESULT ElevationChildConnect(
    __in_z LPCWSTR wzPipeName,
    __in_z LPCWSTR wzToken,
    __out HANDLE* phPipe
    );
HRESULT ElevationChildConnected(
    __in HANDLE hPipe
    );
HRESULT ElevationChildPumpMessages(
    __in HANDLE hPipe,
    __in BURN_PACKAGES* pPackages,
    __in BURN_PAYLOADS* pPayloads,
    __in BURN_VARIABLES* pVariables,
    __in BURN_REGISTRATION* pRegistration,
    __in BURN_USER_EXPERIENCE* pUserExperience
    );

#ifdef __cplusplus
}
#endif
