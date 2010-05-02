//-------------------------------------------------------------------------------------------------
// <copyright file="payloads.h" company="Microsoft">
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
//    Payload management functions for Burn bundles.
// </summary>
//-------------------------------------------------------------------------------------------------

HRESULT PayloadGetManifest(
    __in_z_opt LPCWSTR wzSourcePath,
    __out LPBYTE* pwzManifest,
    __out DWORD* cbManifest,
    __out_z LPWSTR* psczBundleId,
    __out DWORD* pdwContainerOffset,
    __out DWORD* pdwAttachedContainerOffset
    );
