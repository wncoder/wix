//-------------------------------------------------------------------------------------------------
// <copyright file="cache.cpp" company="Microsoft">
//    Copyright (c) Microsoft Corporation.  All rights reserved.
//    
//    The use and distribution terms for this software are covered by the
//    Common Public License 1.0 (http://opensource.org/licenses/cpl1.0.php)
//    which can be found in the file CPL.TXT at the root of this distribution.
//    By using this software in any fashion, you are agreeing to be bound by
//    the terms of this license.
//    
//    You must not remove this notice, or any other, from this software.
// </copyright>
// 
// <summary>
//    Burn cache functions.
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"

static HRESULT VerifyPayloadHash(
    __in BURN_PAYLOAD* pPayload,
    __in_z LPCWSTR wzUnverifiedPayloadPath,
    __in HANDLE hFile
    );
static HRESULT VerifyPayloadWithCatalog(
    __in BURN_PAYLOAD* pPayload,
    __in_z LPCWSTR wzUnverifiedPayloadPath,
    __in HANDLE hFile
    );
static HRESULT GetVerifiedCertificateChain(
    __in HCRYPTMSG hCryptMsg,
    __in HCERTSTORE hCertStore,
    __in PCCERT_CONTEXT pCertContext,
    __out PCCERT_CHAIN_CONTEXT* ppChainContext
    );


extern "C" HRESULT CacheCalculatePayloadUnverifiedPath(
    __in_opt BURN_PACKAGE* /* pPackage */,
    __in BURN_PAYLOAD* pPayload,
    __deref_out_z LPWSTR* psczUnverifiedPath
    )
{
    HRESULT hr = S_OK;

    hr = StrAlloc(psczUnverifiedPath, MAX_PATH);
    ExitOnFailure(hr, "Failed to allocate memory for the unverified path for a payload.");

    if (0 == ::GetTempPathW(MAX_PATH, *psczUnverifiedPath))
    {
        ExitWithLastError(hr, "Failed to get temp path for payload unverified path.");
    }

    hr = StrAllocConcat(psczUnverifiedPath, pPayload->sczKey, 0);
    ExitOnFailure(hr, "Failed to append SHA1 hash as payload unverified path.");

LExit:
    return hr;
}

extern "C" HRESULT CacheCaclulateContainerUnverifiedPath(
    __in BURN_CONTAINER* pContainer,
    __deref_out_z LPWSTR* psczUnverifiedPath
    )
{
    HRESULT hr = S_OK;

    hr = StrAlloc(psczUnverifiedPath, MAX_PATH);
    ExitOnFailure(hr, "Failed to allocate memory for the unverified path for a container.");

    if (0 == ::GetTempPathW(MAX_PATH, *psczUnverifiedPath))
    {
        ExitWithLastError(hr, "Failed to get temp path for container unverified path.");
    }

    hr = StrAllocConcat(psczUnverifiedPath, pContainer->sczHash, 0);
    ExitOnFailure(hr, "Failed to append SHA1 hash as container unverified path.");

LExit:
    return hr;
}

extern "C" HRESULT CacheGetCompletedPath(
    __in BOOL fPerMachine,
    __in_z LPCWSTR wzCacheId,
    __inout_z LPWSTR* psczCompletedPath
    )
{
    HRESULT hr = S_OK;
    LPWSTR sczLocalAppData = NULL;

    hr = PathGetKnownFolder(fPerMachine ? CSIDL_COMMON_APPDATA : CSIDL_LOCAL_APPDATA, &sczLocalAppData);
    ExitOnFailure1(hr, "Failed to find local %hs appdata directory.", fPerMachine ? "per-machine" : "per-user");

    hr = StrAllocFormatted(psczCompletedPath, L"%lsPackage Cache\\%ls", sczLocalAppData, wzCacheId);
    ExitOnFailure(hr, "Failed to format cache path.");

LExit:
    ReleaseStr(sczLocalAppData);
    return hr;
}

extern "C" HRESULT CacheGetResumePath(
    __in_z LPCWSTR wzWorkingPath,
    __inout_z LPWSTR* psczResumePath
    )
{
    HRESULT hr = S_OK;

    hr = StrAllocFormatted(psczResumePath, L"%ls.R", wzWorkingPath);
    ExitOnFailure(hr, "Failed to create resume path.");

LExit:
    return hr;
}


extern "C" HRESULT CacheEnsureWorkingDirectory(
    __in_z LPCWSTR wzWorkingPath,
    __out_z_opt LPWSTR* psczWorkingDir
    )
{
    HRESULT hr = S_OK;
    LPWSTR sczWorkingDir = NULL;

    hr = PathGetDirectory(wzWorkingPath, &sczWorkingDir);
    ExitOnFailure(hr, "Failed to get working directory for payload.");

    hr = DirEnsureExists(sczWorkingDir, NULL);
    ExitOnFailure(hr, "Failed create working directory for payload.");

    if (psczWorkingDir)
    {
        hr = StrAllocString(psczWorkingDir, sczWorkingDir, 0);
        ExitOnFailure(hr, "Failed to copy working directory.");
    }

LExit:
    ReleaseStr(sczWorkingDir);
    return hr;
}


extern "C" HRESULT CacheSendProgressCallback(
    __in BURN_CACHE_CALLBACK* pCallback,
    __in DWORD64 dw64Progress,
    __in DWORD64 dw64Total,
    __in HANDLE hDestinationFile
    )
{
    static LARGE_INTEGER LARGE_INTEGER_ZERO = { };

    HRESULT hr = S_OK;
    DWORD dwResult = PROGRESS_CONTINUE;
    LARGE_INTEGER liTotalSize = { };
    LARGE_INTEGER liTotalTransferred = { };

    if (pCallback->pfnProgress)
    {
        liTotalSize.QuadPart = dw64Total;
        liTotalTransferred.QuadPart = dw64Progress;

        dwResult = (*pCallback->pfnProgress)(liTotalSize, liTotalTransferred, LARGE_INTEGER_ZERO, LARGE_INTEGER_ZERO, 1, CALLBACK_CHUNK_FINISHED, INVALID_HANDLE_VALUE, hDestinationFile, pCallback->pv);
        switch (dwResult)
        {
        case PROGRESS_CONTINUE:
            hr = S_OK;
            break;

        case PROGRESS_CANCEL: __fallthrough; // TODO: should cancel and stop be treated differently?
        case PROGRESS_STOP:
            hr = HRESULT_FROM_WIN32(ERROR_INSTALL_USEREXIT);
            ExitOnRootFailure(hr, "UX aborted on download progress.");

        case PROGRESS_QUIET: // Not actually an error, just an indication to the caller to stop requesting progress.
            pCallback->pfnProgress = NULL;
            hr = S_OK;
            break;

        default:
            hr = E_UNEXPECTED;
            ExitOnRootFailure(hr, "Invalid return code from progress routine.");
        }
    }

LExit:
    return hr;
}


extern "C" void CacheSendErrorCallback(
    __in BURN_CACHE_CALLBACK* pCallback,
    __in HRESULT hrError,
    __in_z_opt LPCWSTR wzError,
    __out_opt BOOL* pfRetry
    )
{
    if (pfRetry)
    {
        *pfRetry = FALSE;
    }

    if (pCallback->pfnCancel)
    {
        int nResult = (*pCallback->pfnCancel)(hrError, wzError, pfRetry != NULL, pCallback->pv);
        if (pfRetry && IDRETRY == nResult)
        {
            *pfRetry = TRUE;
        }
    }
}


extern "C" HRESULT CachePayload(
    __in_opt BURN_PACKAGE* pPackage,
    __in BURN_PAYLOAD* pPayload,
    __in_z_opt LPCWSTR wzLayoutDirectory,
    __in_z LPCWSTR wzUnverifiedPayloadPath,
    __in BOOL fMove
    )
{
    HRESULT hr = S_OK;
    LPWSTR sczCachedDirectory = NULL;
    LPWSTR sczCachedPath = NULL;
    HANDLE hFile = INVALID_HANDLE_VALUE;

    if (NULL == wzLayoutDirectory)
    {
        AssertSz(pPackage, "Package is required when caching.");

        hr = CacheGetCompletedPath(pPackage->fPerMachine, pPackage->sczCacheId, &sczCachedDirectory);
        ExitOnFailure1(hr, "Failed to get cached path for package: %ls", pPackage->sczId);
    }
    else
    {
        hr = StrAllocString(&sczCachedDirectory, wzLayoutDirectory, 0);
        ExitOnFailure(hr, "Failed to copy layout directory.");
    }

    hr = PathConcat(sczCachedDirectory, pPayload->sczFilePath, &sczCachedPath);
    ExitOnFailure(hr, "Failed to concat complete cached path.");

    // Get the payload on disk actual hash.
    hFile = ::CreateFileW(wzUnverifiedPayloadPath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
    if (INVALID_HANDLE_VALUE == hFile)
    {
        ExitWithLastError1(hr, "Failed to open payload in working path: %ls", wzUnverifiedPayloadPath);
    }
    
    // If the payload has a certificate root public key identifier provided, verify the certificate.
    if (pPayload->pbCertificateRootPublicKeyIdentifier)
    {
        hr = CacheVerifyPayloadSignature(pPayload, wzUnverifiedPayloadPath, hFile);
        ExitOnFailure1(hr, "Failed to verify payload signature: %ls", sczCachedPath);
    }
    else if (pPayload->pCatalog) // If catalog files are specified, attempt to verify the file with a catalog file
    {
        hr = VerifyPayloadWithCatalog(pPayload, wzUnverifiedPayloadPath, hFile);
        ExitOnFailure1(hr, "Failed to verify payload signature: %ls", sczCachedPath);
    }
    else if (pPayload->pbHash) // the payload should have a hash we can use to verify it.
    {
        hr = VerifyPayloadHash(pPayload, wzUnverifiedPayloadPath, hFile);
        ExitOnFailure1(hr, "Failed to verify payload hash: %ls", sczCachedPath);
    }

    LogStringLine(REPORT_STANDARD, "Caching payload from working path '%ls' to path '%ls'", wzUnverifiedPayloadPath, sczCachedPath);

    if (fMove)
    {
        hr = FileEnsureMove(wzUnverifiedPayloadPath, sczCachedPath, TRUE, TRUE);
        ExitOnFailure2(hr, "Failed to move %ls to %ls", wzUnverifiedPayloadPath, sczCachedPath);
    }
    else
    {
        hr = FileEnsureCopy(wzUnverifiedPayloadPath, sczCachedPath, TRUE);
        ExitOnFailure2(hr, "Failed to copy %ls to %ls", wzUnverifiedPayloadPath, sczCachedPath);
    }

LExit:
    ReleaseFileHandle(hFile);
    ReleaseStr(sczCachedPath);
    ReleaseStr(sczCachedDirectory);
    return hr;
}


extern "C" HRESULT CacheRemovePackage(
    __in BOOL fPerMachine,
    __in LPCWSTR wzPackageId
    )
{
    HRESULT hr = S_OK;
    LPWSTR sczRootCacheDirectory = NULL;
    LPWSTR sczDirectory = NULL;
    LPWSTR sczFiles = NULL;
    HANDLE hFind = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATAW wfd;

    LPWSTR sczFile = NULL;
    WCHAR wzTempDirectory[MAX_PATH];
    WCHAR wzTempPath[MAX_PATH];

    hr = CacheGetCompletedPath(fPerMachine, L"", &sczRootCacheDirectory);
    ExitOnFailure(hr, "Failed to calculate root cache path.");

    hr = PathConcat(sczRootCacheDirectory, wzPackageId, &sczDirectory);
    ExitOnFailure(hr, "Failed to combine package id to root cache path.");

    LogId(REPORT_STANDARD, MSG_UNCACHE_PACKAGE, wzPackageId, sczDirectory);

    hr = PathConcat(sczDirectory, L"*.*", &sczFiles);
    ExitOnFailure(hr, "Failed to allocate path to all files in bundle directory.");

    if (!::GetTempPathW(countof(wzTempDirectory), wzTempDirectory))
    {
        ExitWithLastError(hr, "Failed to get temp directory.");
    }

    hFind = ::FindFirstFileW(sczFiles, &wfd);
    if (INVALID_HANDLE_VALUE == hFind)
    {
        ExitOnLastError1(hr, "Failed to get first file in bundle directory: %ls", sczDirectory);
    }

    do
    {
        // Skip the dot directories.
        if (L'.' == wfd.cFileName[0] && (L'\0' == wfd.cFileName[1] || (L'.' == wfd.cFileName[1] && L'\0' == wfd.cFileName[2])))
        {
            continue;
        }

        hr = PathConcat(sczDirectory, wfd.cFileName, &sczFile);
        ExitOnFailure(hr, "Failed to allocate file name to move.");

        if (!::GetTempFileNameW(wzTempDirectory, L"BRN", 0, wzTempPath))
        {
            ExitWithLastError(hr, "Failed to get temp file to move to.");
        }

        // Clear any attributes that might get in our way.
        if (wfd.dwFileAttributes & FILE_ATTRIBUTE_READONLY || wfd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN || wfd.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM)
        {
            if (!::SetFileAttributesW(sczFile, FILE_ATTRIBUTE_NORMAL))
            {
                ExitWithLastError1(hr, "Failed to remove attributes from file: %ls", sczFile);
            }
        }

        // We'll ignore failures to remove files for now. The directory delete below may
        // be more successful.
        ::MoveFileExW(sczFile, wzTempPath, MOVEFILE_REPLACE_EXISTING);
    } while (::FindNextFileW(hFind, &wfd));

    hr = DirEnsureDeleteEx(sczDirectory, DIR_DELETE_FILES | DIR_DELETE_RECURSE | DIR_DELETE_SCHEDULE);
    ExitOnFailure1(hr, "Failed to remove cached directory: %ls", sczDirectory);

    // Try to remove root package cache in the off chance it is now empty.
    ::RemoveDirectoryW(sczRootCacheDirectory);

LExit:
    ReleaseFileFindHandle(hFind);
    ReleaseStr(sczFile);
    ReleaseStr(sczFiles);
    ReleaseStr(sczDirectory);
    ReleaseStr(sczRootCacheDirectory);

    return hr;
}


extern "C" HRESULT CacheVerifyPayloadSignature(
    __in BURN_PAYLOAD* pPayload,
    __in_z LPCWSTR wzUnverifiedPayloadPath,
    __in HANDLE hFile
    )
{
    HRESULT hr = S_OK;
    LONG er = ERROR_SUCCESS;

    GUID guidAuthenticode = WINTRUST_ACTION_GENERIC_VERIFY_V2;
    WINTRUST_FILE_INFO wfi = { };
    WINTRUST_DATA wtd = { };

    HCERTSTORE hCertStore = NULL;
    HCRYPTMSG hCryptMsg = NULL;
    PCCERT_CONTEXT pCertContext = NULL;
    PCCERT_CHAIN_CONTEXT pChainContext = NULL;
    PCCERT_CONTEXT pChainElementCertContext = NULL;

    BYTE rgbPublicKeyIdentifier[SHA1_HASH_LEN] = { };
    DWORD cbPublicKeyIdentifier = sizeof(rgbPublicKeyIdentifier);
    BYTE* pbThumbprint = NULL;
    DWORD cbThumbprint = 0;

    // Verify the payload.
    wfi.cbStruct = sizeof(wfi);
    wfi.pcwszFilePath = wzUnverifiedPayloadPath;
    wfi.hFile = hFile;

    wtd.cbStruct = sizeof(wtd);
    wtd.dwUnionChoice = WTD_CHOICE_FILE;
    wtd.pFile = &wfi;
    wtd.dwStateAction = WTD_STATEACTION_VERIFY;
    wtd.dwProvFlags = WTD_REVOCATION_CHECK_NONE;
    wtd.dwUIChoice = WTD_UI_NONE;
    wtd.fdwRevocationChecks = WTD_REVOKE_NONE;

    er = ::WinVerifyTrust(static_cast<HWND>(INVALID_HANDLE_VALUE), &guidAuthenticode, &wtd);
    hr = HRESULT_FROM_WIN32(er);
    ExitOnFailure1(hr, "Failed authenticode verification of payload: %ls", wzUnverifiedPayloadPath);

    // Get handles to the embedded authenticode (PKCS7) cerificate which includes all the certificates, CRLs and CTLs.
    if (!::CryptQueryObject(CERT_QUERY_OBJECT_FILE, wzUnverifiedPayloadPath, CERT_QUERY_CONTENT_FLAG_PKCS7_SIGNED_EMBED, CERT_QUERY_FORMAT_FLAG_ALL, 0, NULL, NULL, NULL, &hCertStore, &hCryptMsg, NULL))
    {
        ExitWithLastError1(hr, "Failed to get authenticode certificate embedded in: %ls", wzUnverifiedPayloadPath);
    }

    if (!::CryptMsgGetAndVerifySigner(hCryptMsg, 0, NULL, CMSG_SIGNER_ONLY_FLAG, &pCertContext, NULL))
    {
        ExitWithLastError(hr, "Failed to get certificate context from embedded authenticode certificate.");
    }

    // Get the certificate chain.
    hr = GetVerifiedCertificateChain(hCryptMsg, hCertStore, pCertContext, &pChainContext);
    ExitOnFailure(hr, "Failed to get certificate chain for authenticode certificate.");

    // Walk up the chain looking for a certificate in the chain that matches our expected public key identifier
    // and thumbprint (if a thumbprint was provided).
    hr = E_NOTFOUND; // assume we won't find a match.
    for (DWORD i = 0; i < pChainContext->rgpChain[0]->cElement; ++i)
    {
        pChainElementCertContext = pChainContext->rgpChain[0]->rgpElement[i]->pCertContext;

        // Get the certificate's public key identifier.
        if (!::CryptHashPublicKeyInfo(NULL, CALG_SHA1, 0, X509_ASN_ENCODING, &pChainElementCertContext->pCertInfo->SubjectPublicKeyInfo, rgbPublicKeyIdentifier, &cbPublicKeyIdentifier))
        {
            ExitWithLastError(hr, "Failed to get certificate public key identifier.");
        }

        // Compare the certificate's public key identifier with the payload's public key identifier. If they
        // match, we're one step closer to the a positive result.
        if (pPayload->cbCertificateRootPublicKeyIdentifier == cbPublicKeyIdentifier &&
            0 == memcmp(pPayload->pbCertificateRootPublicKeyIdentifier, rgbPublicKeyIdentifier, cbPublicKeyIdentifier))
        {
            // If the payload specified a thumbprint for the certificate, verify it.
            if (pPayload->pbCertificateRootThumbprint)
            {
                hr = CertReadProperty(pChainElementCertContext, CERT_SHA1_HASH_PROP_ID, &pbThumbprint, &cbThumbprint);
                ExitOnFailure(hr, "Failed to read certificate thumbprint.");

                if (pPayload->cbCertificateRootThumbprint == cbThumbprint &&
                    0 == memcmp(pPayload->pbCertificateRootThumbprint, pbThumbprint, cbThumbprint))
                {
                    // If we got here, we found that our payload public key identifier and thumbprint
                    // matched an element in the certficate chain.
                    hr = S_OK;
                    break;
                }
            }
            else // no thumbprint match necessary so we're good to go.
            {
                hr = S_OK;
                break;
            }
        }
    }
    ExitOnFailure(hr, "Failed to verify expected payload certificate with any certificate in the actual certificate chain.");

LExit:
    ReleaseMem(pbThumbprint);
    ReleaseCertChain(pChainContext);
    ReleaseCertContext(pCertContext);
    ReleaseCertStore(hCertStore);
    ReleaseCryptMsg(hCryptMsg);

    return hr;
}


static HRESULT VerifyPayloadHash(
    __in BURN_PAYLOAD* pPayload,
    __in_z LPCWSTR wzUnverifiedPayloadPath,
    __in HANDLE hFile
    )
{
    HRESULT hr = S_OK;
    BYTE rgbActualHash[SHA1_HASH_LEN] = { };
    DWORD64 qwHashedBytes;

    // TODO: create a cryp hash file that sends progress.
    hr = CrypHashFileHandle(hFile, PROV_RSA_FULL, CALG_SHA1, rgbActualHash, sizeof(rgbActualHash), &qwHashedBytes);
    ExitOnFailure1(hr, "Failed to calculate hash for payload in working path: %ls", wzUnverifiedPayloadPath);

    // Compare hashes.
    if (pPayload->cbHash != sizeof(rgbActualHash) || 0 != memcmp(pPayload->pbHash, rgbActualHash, SHA1_HASH_LEN))
    {
        hr = CRYPT_E_HASH_VALUE;
        ExitOnFailure1(hr, "Hash mismatch for payload in working path: %ls", wzUnverifiedPayloadPath);
    }

LExit:
    return hr;
}

static HRESULT VerifyPayloadWithCatalog(
    __in BURN_PAYLOAD* pPayload,
    __in_z LPCWSTR wzUnverifiedPayloadPath,
    __in HANDLE hFile
    )
{
    HRESULT hr = S_FALSE;
    DWORD dwError = ERROR_SUCCESS;
    WINTRUST_DATA WinTrustData = { };
    WINTRUST_CATALOG_INFO WinTrustCatalogInfo = { };
    GUID gSubSystemDriver = WINTRUST_ACTION_GENERIC_VERIFY_V2;
    LPWSTR sczLowerCaseFile = NULL;
    LPWSTR pCurrent = NULL;
    LPWSTR sczName = NULL;
    DWORD dwHashSize = 0;
    DWORD dwTagSize;
    DWORD error;
    LPBYTE pbHash = NULL;

    // Get lower case file name.  Older operating systems need a lower case file
    // to match in the catalog
    hr = StrAllocString(&sczLowerCaseFile, wzUnverifiedPayloadPath, 0);
    ExitOnFailure(hr, "Failed to allocate memory");
    
    // Go through each character doing the lower case of each letter
    pCurrent = sczLowerCaseFile;
    while ('\0' != *pCurrent)
    {
        *pCurrent = (WCHAR)_tolower(*pCurrent);
        pCurrent++;
    }

    // Get file hash
    CryptCATAdminCalcHashFromFileHandle(hFile, &dwHashSize, pbHash, 0);
    error = GetLastError();
    if (ERROR_INSUFFICIENT_BUFFER == error)
    {
        pbHash = (LPBYTE)MemAlloc(dwHashSize, TRUE);
        if (!CryptCATAdminCalcHashFromFileHandle(hFile, &dwHashSize, pbHash, 0))
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            ExitOnFailure(hr, "Failed to get file hash.");
        }
    }
    else
    {
        hr = HRESULT_FROM_WIN32(error);
        ExitOnFailure(hr, "Failed to get file hash.");
    }

    // Make the hash into a string.  This is the member tag for the catalog
    dwTagSize = (dwHashSize * 2) + 1;
    hr = StrAlloc(&sczName, dwTagSize);
    ExitOnFailure(hr, "Failed to allocate string.");
    hr = StrHexEncode(pbHash, dwHashSize, sczName, dwTagSize);
    ExitOnFailure(hr, "Failed to encode file hash.");

    // Set up the WinVerifyTrust structures
    WinTrustData.cbStruct = sizeof(WINTRUST_DATA);
    WinTrustData.dwUIChoice = WTD_UI_NONE;
    WinTrustData.fdwRevocationChecks = WTD_REVOKE_NONE;
    WinTrustData.dwUnionChoice = WTD_CHOICE_CATALOG;
    WinTrustData.dwStateAction = WTD_STATEACTION_VERIFY;
    WinTrustData.dwProvFlags = WTD_REVOCATION_CHECK_NONE;
    WinTrustData.pCatalog = &WinTrustCatalogInfo;

    WinTrustCatalogInfo.cbStruct = sizeof(WINTRUST_CATALOG_INFO);
    WinTrustCatalogInfo.pbCalculatedFileHash = pbHash;
    WinTrustCatalogInfo.cbCalculatedFileHash = dwHashSize;
    WinTrustCatalogInfo.hMemberFile = hFile;
    WinTrustCatalogInfo.pcwszMemberTag = sczName;
    WinTrustCatalogInfo.pcwszMemberFilePath = sczLowerCaseFile;
    WinTrustCatalogInfo.pcwszCatalogFilePath = pPayload->pCatalog->sczLocalFilePath;
    hr = WinVerifyTrust(static_cast<HWND>(INVALID_HANDLE_VALUE), &gSubSystemDriver, &WinTrustData);

    // WinVerifyTrust returns 0 for success, a few different Win32 error codes if it can't
    // find the provider, and any other error code is provider specific, so may not
    // be an actual Win32 error code
    hr = HRESULT_FROM_WIN32(hr);
    ExitOnFailure1(hr, "Could not verify file %ls.", wzUnverifiedPayloadPath);

    // Need to close the WinVerifyTrust action
    WinTrustData.dwStateAction = WTD_STATEACTION_CLOSE;
    hr = WinVerifyTrust(static_cast<HWND>(INVALID_HANDLE_VALUE), &gSubSystemDriver, &WinTrustData);
    hr = HRESULT_FROM_WIN32(hr);
    ExitOnFailure(hr, "Could not close verify handle.");

LExit:
    ReleaseStr(sczLowerCaseFile);
    ReleaseStr(sczName);
    ReleaseMem(pbHash);

    return hr;
}


static HRESULT GetVerifiedCertificateChain(
    __in HCRYPTMSG hCryptMsg,
    __in HCERTSTORE hCertStore,
    __in PCCERT_CONTEXT pCertContext,
    __out PCCERT_CHAIN_CONTEXT* ppChainContext
    )
{
    HRESULT hr = S_OK;
    CMSG_SIGNER_INFO* pSignerInfo = NULL;

    FILETIME ftTimestamp = { };
    CERT_CHAIN_PARA chainPara = { };
    CERT_CHAIN_POLICY_PARA basePolicyPara = { };
    CERT_CHAIN_POLICY_STATUS basePolicyStatus = { };
    PCCERT_CHAIN_CONTEXT pChainContext = NULL;

    // Get the signer information and its signing timestamp from the certificate. We want the
    // signing timestamp so we can correctly verify the certificate chain.
    hr = CrypMsgGetParam(hCryptMsg, CMSG_SIGNER_INFO_PARAM, 0, reinterpret_cast<LPVOID*>(&pSignerInfo), NULL);
    ExitOnFailure(hr, "Failed to get certificate signer information.");

    HRESULT hrTimestamp = CertGetAuthenticodeSigningTimestamp(pSignerInfo, &ftTimestamp);

    // Get the certificate chain.
    chainPara.RequestedUsage.dwType = USAGE_MATCH_TYPE_AND;
    if (!::CertGetCertificateChain(NULL, pCertContext, SUCCEEDED(hrTimestamp) ? &ftTimestamp : NULL, hCertStore, &chainPara, 0, NULL, &pChainContext))
    {
        ExitWithLastError(hr, "Failed to get certificate chain.");
    }

    basePolicyPara.cbSize = sizeof(basePolicyPara);
    basePolicyStatus.cbSize = sizeof(basePolicyStatus);
    if (!::CertVerifyCertificateChainPolicy(CERT_CHAIN_POLICY_BASE, pChainContext, &basePolicyPara, &basePolicyStatus))
    {
        ExitWithLastError(hr, "Failed to verify certificate chain policy.");
    }

    hr = HRESULT_FROM_WIN32(basePolicyStatus.dwError);
    ExitOnFailure(hr, "Failed to verify certificate chain policy status.");

    *ppChainContext = pChainContext;
    pChainContext = NULL;

LExit:
    ReleaseCertChain(pChainContext);
    ReleaseMem(pSignerInfo);

    return hr;
}
