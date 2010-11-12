//-------------------------------------------------------------------------------------------------
// <copyright file="core.h" company="Microsoft">
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
//    Module: Core
//
//    Setup chainer/bootstrapper core for WiX toolset.
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once


#if defined(__cplusplus)
extern "C" {
#endif


// constants

const LPCWSTR BURN_COMMANDLINE_SWITCH_ELEVATED = L"burn.elevated";
const LPCWSTR BURN_COMMANDLINE_SWITCH_EMBEDDED = L"burn.embedded";
const LPCWSTR BURN_COMMANDLINE_SWITCH_UNCACHE_PER_MACHINE = L"burn.uncache.machine";
const LPCWSTR BURN_COMMANDLINE_SWITCH_UNCACHE_PER_USER = L"burn.uncache.user";
const LPCWSTR BURN_COMMANDLINE_SWITCH_LOG_APPEND = L"burn.log.append";

enum BURN_MODE
{
    BURN_MODE_NORMAL,
    BURN_MODE_ELEVATED,
    BURN_MODE_EMBEDDED,
    BURN_MODE_UNCACHE_PER_MACHINE,
    BURN_MODE_UNCACHE_PER_USER,
};


// structs

typedef struct _BURN_ENGINE_STATE
{
    // synchronization
    CRITICAL_SECTION csActive; // Any call from the UX that reads or alters the engine state
                               // needs to be syncronized through this critical section.
                               // Note: The engine must never do a UX callback while in this critical section.

    // UX flow control
    //BOOL fSuspend;             // Is TRUE when UX made Suspend() call on core.
    //BOOL fForcedReboot;        // Is TRUE when UX made Reboot() call on core.
    //BOOL fCancelled;           // Is TRUE when UX return cancel on UX OnXXX() methods.
    //BOOL fReboot;              // Is TRUE when UX confirms OnRestartRequried().
    BOOL fRestart;               // Set TRUE when UX returns IDRESTART during Apply().

    // engine data
    BOOTSTRAPPER_COMMAND command;
    BURN_VARIABLES variables;
    BURN_CONDITION condition;
    BURN_SEARCHES searches;
    BURN_USER_EXPERIENCE userExperience;
    BURN_REGISTRATION registration;
    BURN_CONTAINERS containers;
    BURN_PAYLOADS payloads;
    BURN_PACKAGES packages;

    BOOL fDisableRollback;
    BOOL fParallelCacheAndExecute;

    BURN_LOGGING log;

    BURN_PLAN plan;

    BURN_MODE mode;

    HANDLE hElevatedProcess;
    HANDLE hElevatedPipe;

    HANDLE hEmbeddedProcess;
    HANDLE hEmbeddedPipe;

    LPWSTR sczParentPipeName;
    LPWSTR sczParentToken;

    BURN_RESUME_MODE resumeMode;
} BURN_ENGINE_STATE;


// function declarations

HRESULT CoreInitialize(
    __in_z_opt LPCWSTR wzCommandLine,
    __in int nCmdShow,
    __in BURN_ENGINE_STATE* pEngineState
    );
void CoreUninitialize(
    __in BURN_ENGINE_STATE* pEngineState
    );
HRESULT CoreSerializeEngineState(
    __in BURN_ENGINE_STATE* pEngineState,
    __inout BYTE** ppbBuffer,
    __inout SIZE_T* piBuffer
    );
HRESULT CoreQueryRegistration(
    __in BURN_ENGINE_STATE* pEngineState
    );
//HRESULT CoreDeserializeEngineState(
//    __in BURN_ENGINE_STATE* pEngineState,
//    __in_bcount(cbBuffer) BYTE* pbBuffer,
//    __in SIZE_T cbBuffer
//    );
HRESULT CoreDetect(
    __in BURN_ENGINE_STATE* pEngineState
    );
HRESULT CorePlan(
    __in BURN_ENGINE_STATE* pEngineState,
    __in BOOTSTRAPPER_ACTION action
    );
HRESULT CoreApply(
    __in BURN_ENGINE_STATE* pEngineState,
    __in_opt HWND hwndParent
    );
HRESULT CoreQuit(
    __in BURN_ENGINE_STATE* pEngineState,
    __in int nExitCode
    );
HRESULT CoreSaveEngineState(
    __in BURN_ENGINE_STATE* pEngineState
    );

#if defined(__cplusplus)
}
#endif
