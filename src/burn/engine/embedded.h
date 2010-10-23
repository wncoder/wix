//-------------------------------------------------------------------------------------------------
// <copyright file="embedded.h" company="Microsoft">
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
