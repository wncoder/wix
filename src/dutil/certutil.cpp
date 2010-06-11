//-------------------------------------------------------------------------------------------------
// <copyright file="certutil.cpp" company="Microsoft">
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
//    Certificate helper funtions.
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"

/********************************************************************
CertReadProperty - reads a property from the certificate.

NOTE: call MemFree() on the returned pvValue.
********************************************************************/
extern "C" HRESULT DAPI CertReadProperty(
    __in PCCERT_CONTEXT pCertContext,
    __in DWORD dwProperty,
    __deref_out_bound LPVOID* ppvValue
    )
{
    HRESULT hr = S_OK;
    LPVOID pv = NULL;
    DWORD cb = 0;

    if (!::CertGetCertificateContextProperty(pCertContext, dwProperty, NULL, &cb))
    {
        ExitWithLastError(hr, "Failed to get size of certificate property.");
    }

    pv = MemAlloc(cb, TRUE);
    ExitOnNull(pv, hr, E_OUTOFMEMORY, "Failed to allocate memory for certificate property.");

    if (!::CertGetCertificateContextProperty(pCertContext, dwProperty, pv, &cb))
    {
        ExitWithLastError(hr, "Failed to get certificate property.");
    }

    *ppvValue = pv;
    pv = NULL;

LExit:
    ReleaseMem(pv);
    return hr;
}

extern "C" HRESULT DAPI GetCryptProvFromCert(
      __in_opt HWND hwnd,
      __in PCCERT_CONTEXT pCert,
      __out HCRYPTPROV *phCryptProv,
      __out DWORD *pdwKeySpec,
      __in BOOL *pfDidCryptAcquire,
      __deref_opt_out LPWSTR *ppwszTmpContainer,
      __deref_opt_out LPWSTR *ppwszProviderName,
      __out DWORD *pdwProviderType
      )
{
    HRESULT hr = S_OK;
    HMODULE hMsSign32 = NULL;

    typedef BOOL (WINAPI *GETCRYPTPROVFROMCERTPTR)(HWND, PCCERT_CONTEXT, HCRYPTPROV*, DWORD*,BOOL*,LPWSTR*,LPWSTR*,DWORD*);
    GETCRYPTPROVFROMCERTPTR pGetCryptProvFromCert = NULL;

    hMsSign32 = ::LoadLibraryW(L"MsSign32.dll");
    ExitOnNullWithLastError(hMsSign32, hr, "Failed to get handle to MsSign32.dll");

    pGetCryptProvFromCert = (GETCRYPTPROVFROMCERTPTR)::GetProcAddress(hMsSign32, "GetCryptProvFromCert");
    ExitOnNullWithLastError(hMsSign32, hr, "Failed to get handle to MsSign32.dll");

    if (!pGetCryptProvFromCert(hwnd,
                               pCert,
                               phCryptProv,
                               pdwKeySpec,
                               pfDidCryptAcquire,
                               ppwszTmpContainer,
                               ppwszProviderName,
                               pdwProviderType))
    {
        ExitWithLastError(hr, "Failed to get CSP from cert.");
    }
LExit:
    return hr;
}

extern "C" HRESULT DAPI FreeCryptProvFromCert(
    __in BOOL fAcquired,
    __in HCRYPTPROV hProv,
    __in_opt LPWSTR pwszCapiProvider,
    __in DWORD dwProviderType,
    __in_opt LPWSTR pwszTmpContainer
    )
{
    HRESULT hr = S_OK;
    HMODULE hMsSign32 = NULL;

    typedef void (WINAPI *FREECRYPTPROVFROMCERT)(BOOL, HCRYPTPROV, LPWSTR, DWORD, LPWSTR);
    FREECRYPTPROVFROMCERT pFreeCryptProvFromCert = NULL;

    hMsSign32 = ::LoadLibraryW(L"MsSign32.dll");
    ExitOnNullWithLastError(hMsSign32, hr, "Failed to get handle to MsSign32.dll");

    pFreeCryptProvFromCert = (FREECRYPTPROVFROMCERT)::GetProcAddress(hMsSign32, "FreeCryptProvFromCert");
    ExitOnNullWithLastError(hMsSign32, hr, "Failed to get handle to MsSign32.dll");

    pFreeCryptProvFromCert(fAcquired, hProv, pwszCapiProvider, dwProviderType, pwszTmpContainer);
LExit:
    return hr;
}

extern "C" HRESULT DAPI GetProvSecurityDesc(
    __in HCRYPTPROV hProv,
    __deref_out SECURITY_DESCRIPTOR** ppSecurity)
{
    HRESULT hr = S_OK;
    ULONG ulSize = 0;
    SECURITY_DESCRIPTOR* pSecurity = NULL;

    // Get the size of the security descriptor.
    if (!::CryptGetProvParam(
                             hProv,
                             PP_KEYSET_SEC_DESCR,
                             NULL,
                             &ulSize,
                             DACL_SECURITY_INFORMATION))
    {
        ExitWithLastError(hr, "Error getting security descriptor size for CSP.");
    }

    // Allocate the memory for the security descriptor.
    pSecurity = static_cast<SECURITY_DESCRIPTOR *>(MemAlloc(ulSize, TRUE));
    ExitOnNullWithLastError(pSecurity, hr, "Error allocating memory for CSP DACL");

    // Get the security descriptor.
    if (!::CryptGetProvParam(
                             hProv,
                             PP_KEYSET_SEC_DESCR,
                             (BYTE*)pSecurity,
                             &ulSize,
                             DACL_SECURITY_INFORMATION))
    {
        MemFree(pSecurity);
        ExitWithLastError(hr, "Error getting security descriptor for CSP.");
    }
    *ppSecurity = pSecurity;

LExit:
    return hr;
}


extern "C" HRESULT DAPI SetProvSecurityDesc(
    __in HCRYPTPROV hProv,
    __in SECURITY_DESCRIPTOR* pSecurity)
{
    HRESULT hr = S_OK;

    // Set the new security descriptor.
    if(!::CryptSetProvParam(
                            hProv,
                            PP_KEYSET_SEC_DESCR,
                            (BYTE*)pSecurity,
                            DACL_SECURITY_INFORMATION))
    {
        ExitWithLastError(hr, "Error setting security descriptor for CSP.");
    }
LExit:
    return hr;
}

extern "C" BOOL DAPI CertHasPrivateKey(
    __in PCCERT_CONTEXT pCertContext,
    __out_opt DWORD* pdwKeySpec)
{
    HCRYPTPROV_OR_NCRYPT_KEY_HANDLE hPrivateKey = NULL;
    DWORD dwKeySpec = 0;
    // set CRYPT_ACQUIRE_CACHE_FLAG so that we don't have to release the private key handle
    BOOL fResult = ::CryptAcquireCertificatePrivateKey(
                            pCertContext,
                            CRYPT_ACQUIRE_SILENT_FLAG | CRYPT_ACQUIRE_CACHE_FLAG,
                            0,      //pvReserved
                            &hPrivateKey,
                            &dwKeySpec,
                            NULL
                            );
    if (pdwKeySpec)
    {
        *pdwKeySpec = dwKeySpec;
    }
    return fResult;
}


extern "C" HRESULT DAPI CertInstallSingleCertificate(
    __in HCERTSTORE hStore,
    __in PCCERT_CONTEXT pCertContext,
    __in LPCWSTR wzName
    )
{
    HRESULT hr = S_OK;
    CERT_BLOB blob = { };

    DWORD dwKeySpec = 0;

    HCRYPTPROV hCsp = NULL;
    LPWSTR pwszTmpContainer = NULL;
    LPWSTR pwszProviderName = NULL;
    DWORD dwProviderType = 0;
    BOOL fAcquired = TRUE;

    SECURITY_DESCRIPTOR* pSecurity = NULL;
    SECURITY_DESCRIPTOR* pSecurityNew = NULL;

    // Update the friendly name of the certificate to be configured.
    blob.pbData = (BYTE*)wzName;
    blob.cbData = (lstrlenW(wzName) + 1) * sizeof(WCHAR); // including terminating null

    if (!::CertSetCertificateContextProperty(pCertContext, CERT_FRIENDLY_NAME_PROP_ID, 0, &blob))
    {
        ExitWithLastError1(hr, "Failed to set the friendly name of the certificate: %S", wzName);
    }

    if (!::CertAddCertificateContextToStore(hStore, pCertContext, CERT_STORE_ADD_REPLACE_EXISTING, NULL))
    {
        ExitWithLastError(hr, "Failed to add certificate to the store.");
    }

    // if the certificate has a private key, grant Administrators access
    if (CertHasPrivateKey(pCertContext, &dwKeySpec))
    {
        if (AT_KEYEXCHANGE == dwKeySpec || AT_SIGNATURE == dwKeySpec)
        {
            // We added a CSP key
            hr = GetCryptProvFromCert(NULL, pCertContext, &hCsp, &dwKeySpec, &fAcquired, &pwszTmpContainer, &pwszProviderName, &dwProviderType);
            ExitOnFailure(hr, "Failed to get handle to CSP");

            hr = GetProvSecurityDesc(hCsp, &pSecurity);
            ExitOnFailure(hr, "Failed to get security descriptor of CSP");

            hr = AclAddAdminToSecurityDescriptor(pSecurity, &pSecurityNew);
            ExitOnFailure(hr, "Failed to create new security descriptor");

            hr = SetProvSecurityDesc(hCsp, pSecurityNew);
            ExitOnFailure(hr, "Failed to set Admin ACL on CSP");
        }

        if (CERT_NCRYPT_KEY_SPEC == dwKeySpec)
        {
            // We added a CNG key
            // TODO change ACL on CNG key
        }
    }
LExit:
    if (hCsp)
    {
        FreeCryptProvFromCert(fAcquired, hCsp, NULL, dwProviderType, NULL);
    }

    ReleaseMem(pSecurity);

    if (pSecurityNew)
    {
        AclFreeSecurityDescriptor(pSecurityNew);
    }
    return hr;
}