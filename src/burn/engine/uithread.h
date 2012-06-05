//-------------------------------------------------------------------------------------------------
// <copyright file="uithread.h" company="Microsoft Corporation">
//   Copyright (c) 2004, Microsoft Corporation.
//   This software is released under Common Public License Version 1.0 (CPL).
//   The license and further copyright text can be found in the file LICENSE.TXT
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
//
// <summary>
//    Message handler window
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once


#if defined(__cplusplus)
extern "C" {
#endif


// functions

HRESULT UiCreateMessageWindow(
    __in HINSTANCE hInstance,
    __in BURN_ENGINE_STATE* pEngineState
    );

void UiCloseMessageWindow(
    __in BURN_ENGINE_STATE* pEngineState
    );

#if defined(__cplusplus)
}
#endif
