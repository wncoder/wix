#pragma once
//-------------------------------------------------------------------------------------------------
// <copyright file="wcalog.h" company="Microsoft Corporation">
//   Copyright (c) 2004, Microsoft Corporation.
//   This software is released under Common Public License Version 1.0 (CPL).
//   The license and further copyright text can be found in the file LICENSE.TXT
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
// 
// <summary>
//    Private header for internal logging functions
// </summary>
//-------------------------------------------------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif

BOOL WIXAPI IsVerboseLogging();
HRESULT WIXAPI SetVerboseLoggingAtom(BOOL bValue);

#ifdef __cplusplus
}
#endif
