//-------------------------------------------------------------------------------------------------
// <copyright file="core.h" company="Microsoft">
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
//    Module: Core
//
//    Setup chainer/bootstrapper core for WiX toolset.
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once


#if defined(__cplusplus)
extern "C" {
#endif


// structs

typedef struct _BURN_ENGINE_STATE
{
    // synchronization
    CRITICAL_SECTION csActive; // Any call from the UX that reads or alters the engine state
                               // needs to be syncronized through this critical section.
                               // Note: The engine must never do a UX callback while in this critical section.

    BOOL fActive;              // Indicates that the engine is currently active with one of the execution
                               // steps (detect, plan, apply), and cannot accept requests from the UX.
                               // This flag should be cleared by the engine prior to UX callbacks that
                               // allows altering of the engine state.
    DWORD dwExitCode;

    // UX flow controll
    BOOL fSuspend;             // Is TRUE when UX made Suspend() call on core.
    BOOL fForcedReboot;        // Is TRUE when UX made Reboot() call on core.
    BOOL fCancelled;           // Is TRUE when UX return cancel on UX OnXXX() methods.
    BOOL fReboot;              // Is TRUE when UX confirms OnRestartRequried().

    HANDLE hElevatedProcess;
    HANDLE hElevatedPipe;

    // engine data
    BURN_COMMAND command;
    BURN_VARIABLES variables;
    BURN_SEARCHES searches;
    BURN_USER_EXPERIENCE userExperience;
    BURN_REGISTRATION registration;
    BURN_PAYLOADS payloads;
    BURN_PACKAGES packages;
} BURN_ENGINE_STATE;


// function declarations

HRESULT CoreInitialize(
    __in BURN_ENGINE_STATE* pEngineState
    );
void CoreUninitialize(
    __in BURN_ENGINE_STATE* pEngineState
    );
HRESULT Run(
    __in BURN_ENGINE_STATE* pEngineState
    );
HRESULT CoreSerializeEngineState(
    __in BURN_ENGINE_STATE* pEngineState,
    __inout BYTE** ppbBuffer,
    __inout SIZE_T* piBuffer
    );
HRESULT CoreDeserializeEngineState(
    __in BURN_ENGINE_STATE* pEngineState,
    __in_bcount(cbBuffer) BYTE* pbBuffer,
    __in SIZE_T cbBuffer
    );


#if defined(__cplusplus)
}
#endif
