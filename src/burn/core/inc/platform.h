//-------------------------------------------------------------------------------------------------
// <copyright file="platform.h" company="Microsoft">
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
//    Module: Core
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once


#if defined(__cplusplus)
extern "C" {
#endif


// typedefs

typedef INSTALLSTATE (WINAPI *PFN_MSIGETCOMPONENTPATHW)(
    __in LPCWSTR szProduct,
    __in LPCWSTR szComponent,
    __out_ecount_opt(*pcchBuf) LPWSTR lpPathBuf,
    __inout_opt LPDWORD pcchBuf
    );
typedef INSTALLSTATE (WINAPI *PFN_MSILOCATECOMPONENTW)(
    __in LPCWSTR szComponent,
    __out_ecount_opt(*pcchBuf) LPWSTR lpPathBuf,
    __inout_opt LPDWORD pcchBuf
    );
typedef UINT (WINAPI *PFN_MSIGETPRODUCTINFOEXW)(
    __in LPCWSTR szProductCode,
    __in_opt LPCWSTR szUserSid,
    __in MSIINSTALLCONTEXT dwContext,
    __in LPCWSTR szProperty,
    __out_ecount_opt(*pcchValue) LPWSTR szValue,
    __inout_opt LPDWORD pcchValue
    );
typedef INSTALLSTATE (WINAPI *PFN_MSIQUERYFEATURESTATEW)(
    __in LPCWSTR szProduct,
    __in LPCWSTR szFeature
    );
typedef UINT (WINAPI *PFN_MSIINSTALLPRODUCTW)(
    __in LPCWSTR szPackagePath,
    __in_opt LPCWSTR szCommandLine
    );
typedef UINT (WINAPI *PFN_MSICONFIGUREPRODUCTEXW)(
    __in LPCWSTR szProduct,
    __in int iInstallLevel,
    __in INSTALLSTATE eInstallState,
    __in_opt LPCWSTR szCommandLine
    );
typedef LSTATUS (APIENTRY *PFN_REGCREATEKEYEXW)(
    __in HKEY hKey,
    __in LPCWSTR lpSubKey,
    __reserved DWORD Reserved,
    __in_opt LPWSTR lpClass,
    __in DWORD dwOptions,
    __in REGSAM samDesired,
    __in_opt CONST LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    __out PHKEY phkResult,
    __out_opt LPDWORD lpdwDisposition
    );
typedef LSTATUS (APIENTRY *PFN_REGOPENKEYEXW)(
    __in HKEY hKey,
    __in_opt LPCWSTR lpSubKey,
    __reserved DWORD ulOptions,
    __in REGSAM samDesired,
    __out PHKEY phkResult
    );
typedef LSTATUS (APIENTRY *PFN_REGDELETEKEYW)(
    __in HKEY hKey,
    __in LPCWSTR lpSubKey
    );
typedef LSTATUS (APIENTRY *PFN_REGENUMKEYEXW)(
    __in         HKEY hKey,
    __in         DWORD dwIndex,
    __out        LPTSTR lpName,
    __inout      LPDWORD lpcName,
    __reserved   LPDWORD lpReserved,
    __inout      LPTSTR lpClass,
    __inout_opt  LPDWORD lpcClass,
    __out_opt    PFILETIME lpftLastWriteTime
    );
typedef LSTATUS (APIENTRY *PFN_REGQUERYINFOKEYW)(
    __in         HKEY hKey,
    __out        LPTSTR lpClass,
    __inout_opt  LPDWORD lpcClass,
    __reserved   LPDWORD lpReserved,
    __out_opt    LPDWORD lpcSubKeys,
    __out_opt    LPDWORD lpcMaxSubKeyLen,
    __out_opt    LPDWORD lpcMaxClassLen,
    __out_opt    LPDWORD lpcValues,
    __out_opt    LPDWORD lpcMaxValueNameLen,
    __out_opt    LPDWORD lpcMaxValueLen,
    __out_opt    LPDWORD lpcbSecurityDescriptor,
    __out_opt    PFILETIME lpftLastWriteTime
    );
typedef BOOL (STDAPICALLTYPE *PFN_SHELLEXECUTEEXW)(
    __inout LPSHELLEXECUTEINFOW lpExecInfo
    );


// variable declarations

extern PFN_MSIGETCOMPONENTPATHW vpfnMsiGetComponentPathW;
extern PFN_MSILOCATECOMPONENTW vpfnMsiLocateComponentW;
extern PFN_MSIGETPRODUCTINFOEXW vpfnMsiGetProductInfoExW;
extern PFN_MSIQUERYFEATURESTATEW vpfnMsiQueryFeatureStateW;
extern PFN_MSIINSTALLPRODUCTW vpfnMsiInstallProductW;
extern PFN_MSICONFIGUREPRODUCTEXW vpfnMsiConfigureProductExW;
extern PFN_REGCREATEKEYEXW vpfnRegCreateKeyExW;
extern PFN_REGOPENKEYEXW vpfnRegOpenKeyExW;
extern PFN_REGDELETEKEYW vpfnRegDeleteKeyW;
extern PFN_REGENUMKEYEXW vpfnRegEnumKeyExW;
extern PFN_REGQUERYINFOKEYW vpfnRegQueryInfoKeyW;
extern PFN_SHELLEXECUTEEXW vpfnShellExecuteExW;


// function declarations

void PlatformInitialize();


#if defined(__cplusplus)
}
#endif
