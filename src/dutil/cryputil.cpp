//-------------------------------------------------------------------------------------------------
// <copyright file="cryputil.cpp" company="Microsoft">
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
//    Cryptography helper functions.
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"


// function definitions

extern "C" HRESULT CrypHashFile(
    __in LPCWSTR wzFilePath,
    __in DWORD dwProvType,
    __in ALG_ID algid,
    __out_bcount(cbHash) BYTE* pbHash,
    __in DWORD cbHash,
    __out_opt DWORD64* pqwBytesHashed
    )
{
    HRESULT hr = S_OK;
    HANDLE hFile = INVALID_HANDLE_VALUE;

    // open input file
    hFile = ::CreateFileW(wzFilePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
    if (INVALID_HANDLE_VALUE == hFile)
    {
        ExitWithLastError(hr, "Failed to open input file.");
    }

    hr = CrypHashFileHandle(hFile, dwProvType, algid, pbHash, cbHash, pqwBytesHashed);
    ExitOnFailure1(hr, "Failed to hash file: %ls", wzFilePath);

LExit:
    ReleaseFileHandle(hFile);

    return hr;
}


extern "C" HRESULT CrypHashFileHandle(
    __in HANDLE hFile,
    __in DWORD dwProvType,
    __in ALG_ID algid,
    __out_bcount(cbHash) BYTE* pbHash,
    __in DWORD cbHash,
    __out_opt DWORD64* pqwBytesHashed
    )
{
    HRESULT hr = S_OK;
    HCRYPTPROV hProv = NULL;
    HCRYPTHASH hHash = NULL;
    DWORD cbRead = 0;
    BYTE rgbBuffer[4096] = { };
    const LARGE_INTEGER liZero = { };

    // get handle to the crypto provider
    if (!::CryptAcquireContext(&hProv, NULL, NULL, dwProvType, CRYPT_VERIFYCONTEXT))
    {
        ExitWithLastError(hr, "Failed to acquire crypto context.");
    }

    // initiate hash
    if (!::CryptCreateHash(hProv, algid, 0, 0, &hHash))
    {
        ExitWithLastError(hr, "Failed to initiate hash.");
    }

    for (;;)
    {
        // read data block
        if (!::ReadFile(hFile, rgbBuffer, sizeof(rgbBuffer), &cbRead, NULL))
        {
            ExitWithLastError(hr, "Failed to read data block.");
        }

        if (!cbRead)
        {
            break; // end of file
        }

        // hash data block
        if (!::CryptHashData(hHash, rgbBuffer, cbRead, 0))
        {
            ExitWithLastError(hr, "Failed to hash data block.");
        }
    }

    // get hash value
    if (!::CryptGetHashParam(hHash, HP_HASHVAL, pbHash, &cbHash, 0))
    {
        ExitWithLastError(hr, "Failed to get hash value.");
    }

    if (pqwBytesHashed)
    {
        if (!::SetFilePointerEx(hFile, liZero, (LARGE_INTEGER*)pqwBytesHashed, FILE_CURRENT))
        {
            ExitWithLastError(hr, "Failed to get file pointer.");
        }
    }

LExit:
    if (hHash)
    {
        ::CryptDestroyHash(hHash);
    }
    if (hProv)
    {
        ::CryptReleaseContext(hProv, 0);
    }

    return hr;
}
