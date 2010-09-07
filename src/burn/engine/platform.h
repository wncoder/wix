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

typedef BOOL (WINAPI *PFN_INITIATESYSTEMSHUTDOWNEXW)(
    __in_opt LPWSTR lpMachineName,
    __in_opt LPWSTR lpMessage,
    __in DWORD dwTimeout,
    __in BOOL bForceAppsClosed,
    __in BOOL bRebootAfterShutdown,
    __in DWORD dwReason
    );
typedef UINT (WINAPI *PFN_MSIENABLELOGW)(
    __in DWORD dwLogMode,
    __in_z LPCWSTR szLogFile,
    __in DWORD dwLogAttributes
    );
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
typedef INSTALLUILEVEL (WINAPI *PFN_MSISETINTERNALUI)(
    __in INSTALLUILEVEL dwUILevel,
    __inout_opt HWND *phWnd
    );
typedef UINT (WINAPI *PFN_MSISETEXTERNALUIRECORD)(
    __in_opt INSTALLUI_HANDLER_RECORD puiHandler,
    __in DWORD dwMessageFilter,
    __in_opt LPVOID pvContext,
    __out_opt PINSTALLUI_HANDLER_RECORD ppuiPrevHandler
    );
typedef INSTALLUI_HANDLERW (WINAPI *PFN_MSISETEXTERNALUIW)(
    __in_opt INSTALLUI_HANDLERW puiHandler,
    __in DWORD dwMessageFilter,
    __in_opt LPVOID pvContext
    );
typedef UINT (WINAPI *PFN_MSIENUMRELATEDPRODUCTSW)(
    __in LPCWSTR lpUpgradeCode,
    __reserved DWORD dwReserved,
    __in DWORD iProductIndex,
    __out_ecount(MAX_GUID_CHARS+1) LPWSTR lpProductBuf
    );
typedef BOOL (STDAPICALLTYPE *PFN_SHELLEXECUTEEXW)(
    __inout LPSHELLEXECUTEINFOW lpExecInfo
    );


// variable declarations

extern PFN_INITIATESYSTEMSHUTDOWNEXW vpfnInitiateSystemShutdownExW;
extern PFN_MSIENABLELOGW vpfnMsiEnableLogW;
extern PFN_MSIGETCOMPONENTPATHW vpfnMsiGetComponentPathW;
extern PFN_MSILOCATECOMPONENTW vpfnMsiLocateComponentW;
extern PFN_MSIGETPRODUCTINFOEXW vpfnMsiGetProductInfoExW;
extern PFN_MSIQUERYFEATURESTATEW vpfnMsiQueryFeatureStateW;
extern PFN_MSIINSTALLPRODUCTW vpfnMsiInstallProductW;
extern PFN_MSICONFIGUREPRODUCTEXW vpfnMsiConfigureProductExW;
extern PFN_MSISETINTERNALUI vpfnMsiSetInternalUI;
extern PFN_MSISETEXTERNALUIRECORD vpfnMsiSetExternalUIRecord;
extern PFN_MSISETEXTERNALUIW vpfnMsiSetExternalUIW;
extern PFN_MSIENUMRELATEDPRODUCTSW vpfnMsiEnumRelatedProductsW;
extern PFN_SHELLEXECUTEEXW vpfnShellExecuteExW;


// function declarations

void PlatformInitialize();


#if defined(__cplusplus)
}
#endif
