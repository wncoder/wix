#pragma once
//-------------------------------------------------------------------------------------------------
// <copyright file="wcawow64.h" company="Microsoft Corporation">
//   Copyright (c) 2004, Microsoft Corporation.
//   This software is released under Common Public License Version 1.0 (CPL).
//   The license and further copyright text can be found in the file LICENSE.TXT
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
// 
// <summary>
//    Windows Installer XML CustomAction utility library for Wow64 API-related functionality.
// </summary>
//-------------------------------------------------------------------------------------------------

#include "wcautil.h"

#ifdef __cplusplus
extern "C" {
#endif

HRESULT WIXAPI WcaInitializeWow64();
BOOL WIXAPI WcaIsWow64Process();
BOOL WIXAPI WcaIsWow64Initialized();
HRESULT WIXAPI WcaDisableWow64FSRedirection();
HRESULT WIXAPI WcaRevertWow64FSRedirection();
HRESULT WIXAPI WcaFinalizeWow64();

#ifdef __cplusplus
}
#endif
