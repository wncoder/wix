//-------------------------------------------------------------------------------------------------
// <copyright file="cache.h" company="Microsoft">
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
//    Burn cache functions.
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once


#ifdef __cplusplus
extern "C" {
#endif

typedef int (WINAPI *LPCANCEL_ROUTINE)(
    __in HRESULT hrError,
    __in_z_opt LPCWSTR wzError,
    __in BOOL fAllowRetry,
    __in_opt LPVOID pvContext
    );

// structs

typedef struct _BURN_CACHE_CALLBACK
{
    LPPROGRESS_ROUTINE pfnProgress;
    LPCANCEL_ROUTINE pfnCancel;
    LPVOID pv;
} BURN_CACHE_CALLBACK;


// functions

HRESULT CacheGetOriginalSourcePath(
    __in BURN_VARIABLES* pVariables,
    __in_z_opt LPCWSTR wzRelativePath,
    __out_z_opt LPWSTR* psczOriginalSource
    );
HRESULT CacheCalculatePayloadUnverifiedPath(
    __in_opt BURN_PACKAGE* pPackage,
    __in BURN_PAYLOAD* pPayload,
    __deref_out_z LPWSTR* psczUnverifiedPath
    );
HRESULT CacheCaclulateContainerUnverifiedPath(
    __in BURN_CONTAINER* pContainer,
    __deref_out_z LPWSTR* psczUnverifiedPath
    );
HRESULT CacheGetCompletedPath(
    __in BOOL fPerMachine,
    __in_z LPCWSTR wzCacheId,
    __inout_z LPWSTR* psczCompletedPath
    );
HRESULT CacheGetResumePath(
    __in_z LPCWSTR wzWorkingPath,
    __inout_z LPWSTR* psczResumePath
    );
HRESULT CacheEnsureWorkingDirectory(
    __in_z LPCWSTR wzWorkingPath,
    __out_z_opt LPWSTR* psczWorkingDir
    );
HRESULT CacheSendProgressCallback(
    __in BURN_CACHE_CALLBACK* pCallback,
    __in DWORD64 dw64Progress,
    __in DWORD64 dw64Total,
    __in HANDLE hDestinationFile
    );
void CacheSendErrorCallback(
    __in BURN_CACHE_CALLBACK* pCallback,
    __in HRESULT hrError,
    __in_z_opt LPCWSTR wzError,
    __out_opt BOOL* pfRetry
    );
extern "C" HRESULT CacheBundle(
    __in BURN_REGISTRATION* pRegistration,
    __in BURN_USER_EXPERIENCE* pUserExperience,
    __in_z LPCWSTR wzExecutablePath
    );
HRESULT CachePayload(
    __in_opt BURN_PACKAGE* pPackage,
    __in BURN_PAYLOAD* pPayload,
    __in_z_opt LPCWSTR wzLayoutDirectory,
    __in_z LPCWSTR wzUnverifiedPayloadPath,
    __in BOOL fMove
    );
HRESULT CacheRemoveBundle(
    __in BOOL fPerMachine,
    __in LPCWSTR wzPackageId
    );
HRESULT CacheRemovePackage(
    __in BOOL fPerMachine,
    __in LPCWSTR wzPackageId
    );
HRESULT CacheVerifyPayloadSignature(
    __in BURN_PAYLOAD* pPayload,
    __in_z LPCWSTR wzUnverifiedPayloadPath,
    __in HANDLE hFile
    );


#ifdef __cplusplus
}
#endif
