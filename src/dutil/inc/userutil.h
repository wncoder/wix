#pragma once
//-------------------------------------------------------------------------------------------------
// <copyright file="userutil.h" company="Microsoft Corporation">
//   Copyright (c) 2004, Microsoft Corporation.
//   This software is released under Common Public License Version 1.0 (CPL).
//   The license and further copyright text can be found in the file LICENSE.TXT
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
// 
// <summary>
//    User helper functions.
// </summary>
//-------------------------------------------------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif

HRESULT DAPI UserBuildDomainUserName(
    __out_ecount_z(cchDest) LPWSTR wzDest,
    __in int cchDest,
    __in_z LPCWSTR pwzName,
    __in_z LPCWSTR pwzDomain
    );

HRESULT DAPI UserCheckIsMember(
    __in_z LPCWSTR pwzName,
    __in_z LPCWSTR pwzDomain,
    __in_z LPCWSTR pwzGroupName,
    __in_z LPCWSTR pwzGroupDomain,
    __out LPBOOL lpfMember
    );

HRESULT DAPI UserCreateADsPath(
    __in_z LPCWSTR wzObjectDomain, 
    __in_z LPCWSTR wzObjectName,
    __out BSTR *pbstrAdsPath
    );

#ifdef __cplusplus
}
#endif
