//-------------------------------------------------------------------------------------------------
// <copyright file="embedded.h" company="Microsoft">
//    Copyright (c) Microsoft Corporation.  All rights reserved.
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

typedef enum _BURN_EMBEDDED_MESSAGE_TYPE
{
    BURN_EMBEDDED_MESSAGE_TYPE_UNKNOWN,
    BURN_EMBEDDED_MESSAGE_TYPE_LAUNCH_CHILD,
    BURN_EMBEDDED_MESSAGE_TYPE_ERROR,
    BURN_EMBEDDED_MESSAGE_TYPE_PROGRESS,
} BURN_EMBEDDED_MESSAGE_TYPE;


// Parent (per-user process) side functions.
HRESULT EmbeddedLaunchChildProcess(
    __in BURN_PACKAGE* pPackage,
    __in BURN_USER_EXPERIENCE* pUX,
    __in HANDLE hElevatedPipe,
    __in_z LPCWSTR wzExecutablePath,
    __in_z LPCWSTR wzCommandLine,
    __out HANDLE* phProcess
    );

// Child (per-machine process) side functions.
HRESULT EmbeddedParentLaunchChildProcess(
    __in HANDLE hParentPipe,
    __in_z LPCWSTR wzPipeName,
    __in_z LPCWSTR wzClientToken,
    __out HANDLE* phElevatedProcess
    );

#ifdef __cplusplus
}
#endif
