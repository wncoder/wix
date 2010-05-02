#pragma once
//-------------------------------------------------------------------------------------------------
// <copyright file="scawebapp7.h" company="Microsoft">
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
//    IIS Web Application functions for CustomActions
// </summary>
//-------------------------------------------------------------------------------------------------


HRESULT ScaGetWebApplication7(
    __in MSIHANDLE hViewApplications,
    __in_z LPCWSTR pwzApplication,
    SCA_WEB_APPLICATION* pswapp
    );



HRESULT ScaWriteWebApplication7(
    __in_z LPCWSTR wzWebName,
    __in_z LPCWSTR wzRootOfWeb,
    SCA_WEB_APPLICATION* pswapp,
    SCA_APPPOOL * psapList
    );
