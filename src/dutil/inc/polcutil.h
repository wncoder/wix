#pragma once
//-------------------------------------------------------------------------------------------------
// <copyright file="polcutil.h" company="Microsoft">
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
