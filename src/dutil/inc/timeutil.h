#pragma once
//-------------------------------------------------------------------------------------------------
// <copyright file="timeutil.h" company="Microsoft Corporation">
//   Copyright (c) 2004, Microsoft Corporation.
//   This software is released under Common Public License Version 1.0 (CPL).
//   The license and further copyright text can be found in the file LICENSE.TXT
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
// 
// <summary>
//  Time helper functions.
// </summary>
//-------------------------------------------------------------------------------------------------


#ifdef __cplusplus
extern "C" {
#endif

HRESULT DAPI TimeFromString(
    __in_z LPCWSTR wzTime,
    __out FILETIME* pFileTime
    );
HRESULT DAPI TimeCurrentTime(
    __deref_out_z LPWSTR* ppwz,
    __in BOOL fGMT
    );
HRESULT DAPI TimeCurrentDateTime(
    __deref_out_z LPWSTR* ppwz,
    __in BOOL fGMT
    );
HRESULT DAPI TimeSystemDateTime(
    __deref_out_z LPWSTR* ppwz,
    __in const SYSTEMTIME *pst,
    __in BOOL fGMT
    );

#ifdef __cplusplus
}
#endif

