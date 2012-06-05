//-------------------------------------------------------------------------------------------------
// <copyright file="EngineForApplication.h" company="Microsoft Corporation">
//   Copyright (c) 2004, Microsoft Corporation.
//   This software is released under Common Public License Version 1.0 (CPL).
//   The license and further copyright text can be found in the file LICENSE.TXT
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
//
// <summary>
//    Module: Core
//
//    Setup chainer/bootstrapper UX core for WiX toolset.
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once


#if defined(__cplusplus)
extern "C" {
#endif

// constants

enum WM_BURN
{
    WM_BURN_FIRST = WM_APP + 0xFFF, // this enum value must always be first.

    WM_BURN_DETECT,
    WM_BURN_PLAN,
    WM_BURN_ELEVATE,
    WM_BURN_APPLY,
    WM_BURN_QUIT,

    WM_BURN_LAST, // this enum value must always be last.
};

// function declarations

HRESULT EngineForApplicationCreate(
    __in BURN_ENGINE_STATE* pEngineState,
    __in DWORD dwThreadId,
    __out IBootstrapperEngine** ppEngineForApplication
    );

#if defined(__cplusplus)
}
#endif
