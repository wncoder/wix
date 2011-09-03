//-------------------------------------------------------------------------------------------------
// <copyright file="uithread.h" company="Microsoft">
//    Copyright (c) Microsoft Corporation.  All rights reserved.
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
