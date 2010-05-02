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

PFN_MSIGETCOMPONENTPATHW vpfnMsiGetComponentPathW;
PFN_MSILOCATECOMPONENTW vpfnMsiLocateComponentW;
PFN_MSIGETPRODUCTINFOEXW vpfnMsiGetProductInfoExW;
PFN_MSIQUERYFEATURESTATEW vpfnMsiQueryFeatureStateW;
PFN_MSIINSTALLPRODUCTW vpfnMsiInstallProductW;
PFN_MSICONFIGUREPRODUCTEXW vpfnMsiConfigureProductExW;
PFN_REGCREATEKEYEXW vpfnRegCreateKeyExW;
PFN_REGOPENKEYEXW vpfnRegOpenKeyExW;
PFN_REGDELETEKEYW vpfnRegDeleteKeyW;
PFN_REGENUMKEYEXW vpfnRegEnumKeyExW;
PFN_REGQUERYINFOKEYW vpfnRegQueryInfoKeyW;
PFN_SHELLEXECUTEEXW vpfnShellExecuteExW;


// function definitions

extern "C" void PlatformInitialize()
{
    vpfnMsiGetComponentPathW = ::MsiGetComponentPathW;
    vpfnMsiLocateComponentW = ::MsiLocateComponentW;
    vpfnMsiGetProductInfoExW = ::MsiGetProductInfoExW;
    vpfnMsiQueryFeatureStateW = ::MsiQueryFeatureStateW;
    vpfnMsiInstallProductW = ::MsiInstallProductW;
    vpfnMsiConfigureProductExW = ::MsiConfigureProductExW;
    vpfnRegCreateKeyExW = ::RegCreateKeyExW;
    vpfnRegOpenKeyExW = ::RegOpenKeyExW;
    vpfnRegDeleteKeyW = ::RegDeleteKeyW;
    vpfnRegEnumKeyExW = ::RegEnumKeyExW;
    vpfnRegQueryInfoKeyW = ::RegQueryInfoKeyW;
    vpfnShellExecuteExW = ::ShellExecuteExW;
}
