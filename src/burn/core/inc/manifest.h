//-------------------------------------------------------------------------------------------------
// <copyright file="manifest.h" company="Microsoft">
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

interface IBurnPayload; // forward declare.

#if defined(__cplusplus)
extern "C" {
#endif


// function declarations

HRESULT ManifestLoadXmlFromBuffer(
    __in_bcount(cbBuffer) BYTE* pbBuffer,
    __in SIZE_T cbBuffer,
    __in BURN_ENGINE_STATE* pEngineState
    );

HRESULT ParseManifestDocument(
    __in_z LPCWSTR wzBundleId,
    __in_z LPCWSTR wzBundleFileName,
    __in_z LPCWSTR wzUXWorkingPath,
    __in IXMLDOMDocument* pixd,
    __in DWORD dwPayloadsOffset,
    __in DWORD dwAttachedContainerOffset,
    __out DWORD* pcChainedPayloads,
    __out DWORD* pcUXPayloads,
    __out IBurnPayload*** prgpUxPayloads,
    __out IBurnPayload*** prgpChainedPayloads
    );

HRESULT ParseManifestChain(
    __in_z LPCWSTR wzBundleId,
    __in_z LPCWSTR wzBundleFileName,
    __in IXMLDOMNode* pixnChain,
    __in DWORD dwPayloadsOffset,
    __out DWORD* pcPackages,
    __out IBurnPayload*** prgpPayloads
    );
#if defined(__cplusplus)
}
#endif
