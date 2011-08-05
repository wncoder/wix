//-------------------------------------------------------------------------------------------------
// <copyright file="elevation.h" company="Microsoft">
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
    BURN_ELEVATION_MESSAGE_TYPE_SESSION_RESUME,
    BURN_ELEVATION_MESSAGE_TYPE_SESSION_END,
    BURN_ELEVATION_MESSAGE_TYPE_DETECT_RELATED_BUNDLES,
    BURN_ELEVATION_MESSAGE_TYPE_SAVE_STATE,
    BURN_ELEVATION_MESSAGE_TYPE_CACHE_PAYLOAD,
    BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_EXE_PACKAGE,
    BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_MSI_PACKAGE,
    BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_MSP_PACKAGE,
    BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_MSU_PACKAGE,
    BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_DEPENDENCY,
    BURN_ELEVATION_MESSAGE_TYPE_LAUNCH_EMBEDDED_CHILD,
    BURN_ELEVATION_MESSAGE_TYPE_CLEAN_PACKAGE,

    BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_PROGRESS,
    BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_ERROR,
    BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_MSI_MESSAGE,
    BURN_ELEVATION_MESSAGE_TYPE_EXECUTE_FILES_IN_USE,
} BURN_ELEVATION_MESSAGE_TYPE;


// Parent (per-user process) side functions.
HRESULT ElevationSessionBegin(
    __in HANDLE hPipe,
    __in BOOTSTRAPPER_ACTION action,
    __in DWORD64 qwEstimatedSize
    );
HRESULT ElevationSessionResume(
    __in HANDLE hPipe,
    __in BOOTSTRAPPER_ACTION action
    );
HRESULT ElevationSessionEnd(
    __in HANDLE hPipe,
    __in BOOTSTRAPPER_ACTION action,
    __in BOOL fRollback,
    __in BOOL fSuspend,
    __in BOOTSTRAPPER_APPLY_RESTART restart
    );
HRESULT ElevationDetectRelatedBundles(
    __in HANDLE hPipe
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
    __in PFN_GENERICMESSAGEHANDLER pfnGenericExecuteProgress,
    __in LPVOID pvContext,
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
HRESULT ElevationExecuteMspPackage(
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
    __in PFN_GENERICMESSAGEHANDLER pfnGenericExecuteProgress,
    __in LPVOID pvContext,
    __out BOOTSTRAPPER_APPLY_RESTART* pRestart
    );
HRESULT ElevationExecuteDependencyAction(
    __in HANDLE hPipe,
    __in BURN_EXECUTE_ACTION* pExecuteAction
    );
HRESULT ElevationLaunchElevatedChild(
    __in HANDLE hPipe,
    __in BURN_PACKAGE* pPackage,
    __in LPCWSTR wzPipeName,
    __in LPCWSTR wzPipeToken,
    __out DWORD* pdwChildPid
    );
HRESULT ElevationCleanPackage(
    __in HANDLE hPipe,
    __in BURN_PACKAGE* pPackage
    );

// Child (per-machine process) side functions.
HRESULT ElevationChildPumpMessages(
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
    );

#ifdef __cplusplus
}
#endif
