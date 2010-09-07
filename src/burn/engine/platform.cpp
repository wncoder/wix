//-------------------------------------------------------------------------------------------------
// <copyright file="platform.cpp" company="Microsoft">
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

#include "precomp.h"


// variables

PFN_INITIATESYSTEMSHUTDOWNEXW vpfnInitiateSystemShutdownExW;
PFN_MSIENABLELOGW vpfnMsiEnableLogW;
PFN_MSIGETCOMPONENTPATHW vpfnMsiGetComponentPathW;
PFN_MSILOCATECOMPONENTW vpfnMsiLocateComponentW;
PFN_MSIGETPRODUCTINFOEXW vpfnMsiGetProductInfoExW;
PFN_MSIQUERYFEATURESTATEW vpfnMsiQueryFeatureStateW;
PFN_MSIINSTALLPRODUCTW vpfnMsiInstallProductW;
PFN_MSICONFIGUREPRODUCTEXW vpfnMsiConfigureProductExW;
PFN_MSISETINTERNALUI vpfnMsiSetInternalUI;
PFN_MSISETEXTERNALUIRECORD vpfnMsiSetExternalUIRecord;
PFN_MSISETEXTERNALUIW vpfnMsiSetExternalUIW;
PFN_MSIENUMRELATEDPRODUCTSW vpfnMsiEnumRelatedProductsW;
PFN_SHELLEXECUTEEXW vpfnShellExecuteExW;


// function definitions

extern "C" void PlatformInitialize()
{
    HMODULE hMsi = ::GetModuleHandleW(L"msi");
    vpfnInitiateSystemShutdownExW = ::InitiateSystemShutdownExW;
    vpfnMsiConfigureProductExW = ::MsiConfigureProductExW;
    vpfnMsiEnableLogW = ::MsiEnableLogW;
    vpfnMsiEnumRelatedProductsW = ::MsiEnumRelatedProductsW;
    vpfnMsiGetComponentPathW = ::MsiGetComponentPathW;
    vpfnMsiGetProductInfoExW = ::MsiGetProductInfoExW;
    vpfnMsiInstallProductW = ::MsiInstallProductW;
    vpfnMsiLocateComponentW = ::MsiLocateComponentW;
    vpfnMsiQueryFeatureStateW = ::MsiQueryFeatureStateW;
    vpfnMsiSetInternalUI = ::MsiSetInternalUI;
    vpfnMsiSetExternalUIRecord = (PFN_MSISETEXTERNALUIRECORD)::GetProcAddress(hMsi, "MsiSetExternalUIRecord");
    vpfnMsiSetExternalUIW = ::MsiSetExternalUIW;
    vpfnShellExecuteExW = ::ShellExecuteExW;
}
