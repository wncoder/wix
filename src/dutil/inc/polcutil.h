#pragma once
//-------------------------------------------------------------------------------------------------
// <copyright file="polcutil.h" company="Microsoft Corporation">
//   Copyright (c) 2004, Microsoft Corporation.
//   This software is released under Common Public License Version 1.0 (CPL).
//   The license and further copyright text can be found in the file LICENSE.TXT
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
// 
// <summary>
//    Header for Policy utility functions.
// </summary>
//-------------------------------------------------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif

/********************************************************************
PolcReadNumber - reads a number from policy.

NOTE: S_FALSE returned if policy not set.
NOTE: out is set to default on S_FALSE or any error.
********************************************************************/
HRESULT DAPI PolcReadNumber(
    __in_z LPCWSTR wzPolicyPath,
    __in_z LPCWSTR wzPolicyName,
    __in DWORD dwDefault,
    __out DWORD* pdw
    );

/********************************************************************
PolcReadString - reads a string from policy.

NOTE: S_FALSE returned if policy not set.
NOTE: out is set to default on S_FALSE or any error.
********************************************************************/
HRESULT DAPI PolcReadString(
    __in_z LPCWSTR wzPolicyPath,
    __in_z LPCWSTR wzPolicyName,
    __in_z_opt LPCWSTR wzDefault,
    __deref_out_z LPWSTR* pscz
    );

#ifdef __cplusplus
}
#endif
