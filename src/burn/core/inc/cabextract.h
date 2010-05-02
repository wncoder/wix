//-------------------------------------------------------------------------------------------------
// <copyright file="cabextract.h" company="Microsoft">
//    Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//
// <summary>
//    Module: Cabinet
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once


#if defined(__cplusplus)
extern "C" {
#endif


// function declarations

void CabExtractInitialize();
HRESULT CabExtractOpen(
    __in BURN_CONTAINER_CONTEXT* pContext,
    __in LPCWSTR wzFilePath
    );
HRESULT CabExtractNextStream(
    __in BURN_CONTAINER_CONTEXT* pContext,
    __inout_z LPWSTR* psczStreamName
    );
HRESULT CabExtractStreamToFile(
    __in BURN_CONTAINER_CONTEXT* pContext,
    __in_z LPCWSTR wzFileName
    );
HRESULT CabExtractStreamToBuffer(
    __in BURN_CONTAINER_CONTEXT* pContext,
    __out BYTE** ppbBuffer,
    __out SIZE_T* pcbBuffer
    );
HRESULT CabExtractClose(
    __in BURN_CONTAINER_CONTEXT* pContext
    );


#if defined(__cplusplus)
}
#endif
