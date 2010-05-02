//-------------------------------------------------------------------------------------------------
// <copyright file="cryputil.h" company="Microsoft">
//    Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//
// <summary>
//    Cryptography helper functions.
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once


#ifdef __cplusplus
extern "C" {
#endif


// function declarations

HRESULT CrypHashFile(
    __in_z LPCWSTR wzFilePath,
    __in DWORD dwProvType,
    __in ALG_ID algid,
    __out_bcount(cbHash) BYTE* pbHash,
    __in DWORD cbHash,
    __out_opt DWORD64* pqwBytesHashed
    );

HRESULT CrypHashFileHandle(
    __in HANDLE hFile,
    __in DWORD dwProvType,
    __in ALG_ID algid,
    __out_bcount(cbHash) BYTE* pbHash,
    __in DWORD cbHash,
    __out_opt DWORD64* pqwBytesHashed
    );

#ifdef __cplusplus
}
#endif
