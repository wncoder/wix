//-------------------------------------------------------------------------------------------------
// <copyright file="cache.h" company="Microsoft">
//    Copyright (c) Microsoft Corporation.  All rights reserved.
//    
//    The use and distribution terms for this software are covered by the
//    Common Public License 1.0 (http://opensource.org/licenses/cpl.php)
//    which can be found in the file CPL.TXT at the root of this distribution.
//    By using this software in any fashion, you are agreeing to be bound by
//    the terms of this license.
//    
//    You must not remove this notice, or any other, from this software.
// </copyright>
// 
// <summary>
//    Burn cache functions.
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once


#ifdef __cplusplus
extern "C" {
#endif


HRESULT CacheGetCompletedPath(
    __in BOOL fPerMachine,
    __in_z LPCWSTR wzCacheId,
    __inout_z LPWSTR* psczCompletedPath
    );
HRESULT CacheDeleteDirectory(
    __in_z LPCWSTR wzDirectory
    );
HRESULT CacheEnsureWorkingDirectory(
    __in_z LPCWSTR wzWorkingPath,
    __out_z_opt LPWSTR* psczWorkingDir
    );
HRESULT CacheCompletePayload(
    __in IBurnPayload* pPayload
    );
HRESULT CacheRemovePackage(
    __in BOOL fPerMachine,
    __in LPCWSTR wzPackageId
    );


#ifdef __cplusplus
}
#endif
