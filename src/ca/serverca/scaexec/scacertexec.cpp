//-------------------------------------------------------------------------------------------------
// <copyright file="scacertexec.h" company="Microsoft">
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
//    Certificate execution functions for CustomActions
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"

#define SIXTY_FOUR_MEG 64 * 1024 * 1024

// prototypes
static HRESULT ExecuteCertificateOperation(
    __in MSIHANDLE hInstall,
    __in SCA_ACTION saAction,
    __in DWORD dwStoreRoot
    );

static HRESULT ReadCertificateFile(
    __in LPCWSTR wzPath,
    __out BYTE** prgbData,
    __out DWORD* pcbData
    );

static HRESULT InstallCertificate(
    __in HCERTSTORE hStore,
    __in BOOL fUserCertificateStore,
    __in LPCWSTR wzName,
    __in_opt BYTE* rgbData,
    __in DWORD cbData,
    __in_opt LPCWSTR wzPFXPassword
    );

static HRESULT UninstallCertificate(
    __in HCERTSTORE hStore,
    __in BOOL fUserCertificateStore,
    __in LPCWSTR wzName
    );


/* ****************************************************************
 AddUserCertificate - CUSTOM ACTION ENTRY POINT for adding per-user
                      certificates

 * ***************************************************************/
extern "C" UINT __stdcall AddUserCertificate(
    __in MSIHANDLE hInstall
    )
{
    HRESULT hr = S_OK;
    DWORD er = ERROR_SUCCESS;

    hr = WcaInitialize(hInstall, "AddUserCertificate");
    ExitOnFailure(hr, "Failed to initialize AddUserCertificate.");

    hr = ExecuteCertificateOperation(hInstall, SCA_ACTION_INSTALL, CERT_SYSTEM_STORE_CURRENT_USER);
    ExitOnFailure(hr, "Failed to install per-user certificate.");

LExit:
    er = SUCCEEDED(hr) ? ERROR_SUCCESS : ERROR_INSTALL_FAILURE;
    return WcaFinalize(er);
}


/* ****************************************************************
 AddMachineCertificate - CUSTOM ACTION ENTRY POINT for adding 
                         per-machine certificates

 * ***************************************************************/
extern "C" UINT __stdcall AddMachineCertificate(
    __in MSIHANDLE hInstall
    )
{
    HRESULT hr = S_OK;
    DWORD er = ERROR_SUCCESS;

    hr = WcaInitialize(hInstall, "AddMachineCertificate");
    ExitOnFailure(hr, "Failed to initialize AddMachineCertificate.");

    hr = ExecuteCertificateOperation(hInstall, SCA_ACTION_INSTALL, CERT_SYSTEM_STORE_LOCAL_MACHINE);
    ExitOnFailure(hr, "Failed to install per-machine certificate.");

LExit:
    er = SUCCEEDED(hr) ? ERROR_SUCCESS : ERROR_INSTALL_FAILURE;
    return WcaFinalize(er);
}


/* ****************************************************************
 DeleteUserCertificate - CUSTOM ACTION ENTRY POINT for deleting 
                         per-user certificates

 * ***************************************************************/
extern "C" UINT __stdcall DeleteUserCertificate(
    __in MSIHANDLE hInstall
    )
{
    HRESULT hr = S_OK;
    DWORD er = ERROR_SUCCESS;

    hr = WcaInitialize(hInstall, "DeleteUserCertificate");
    ExitOnFailure(hr, "Failed to initialize DeleteUserCertificate.");

    hr = ExecuteCertificateOperation(hInstall, SCA_ACTION_UNINSTALL, CERT_SYSTEM_STORE_CURRENT_USER);
    ExitOnFailure(hr, "Failed to uninstall per-user certificate.");

LExit:
    er = SUCCEEDED(hr) ? ERROR_SUCCESS : ERROR_INSTALL_FAILURE;
    return WcaFinalize(er);
}


/* ****************************************************************
 DeleteMachineCertificate - CUSTOM ACTION ENTRY POINT for deleting
                            per-machine certificates

 * ***************************************************************/
extern "C" UINT __stdcall DeleteMachineCertificate(
    __in MSIHANDLE hInstall
    )
{
    HRESULT hr = S_OK;
    DWORD er = ERROR_SUCCESS;

    hr = WcaInitialize(hInstall, "DeleteMachineCertificate");
    ExitOnFailure(hr, "Failed to initialize DeleteMachineCertificate.");

    hr = ExecuteCertificateOperation(hInstall, SCA_ACTION_UNINSTALL, CERT_SYSTEM_STORE_LOCAL_MACHINE);
    ExitOnFailure(hr, "Failed to uninstall per-machine certificate.");

LExit:
    er = SUCCEEDED(hr) ? ERROR_SUCCESS : ERROR_INSTALL_FAILURE;
    return WcaFinalize(er);
}


static HRESULT ExecuteCertificateOperation(
    __in MSIHANDLE hInstall,
    __in SCA_ACTION saAction,
    __in DWORD dwStoreLocation
    )
{
    //AssertSz(FALSE, "Debug ExecuteCertificateOperation() here.");
    Assert(saAction & SCA_ACTION_INSTALL || saAction & SCA_ACTION_UNINSTALL);

    HRESULT hr = S_OK;
    LPWSTR pwzCaData = NULL;
    LPWSTR pwz;
    LPWSTR pwzName = NULL;
    LPWSTR pwzStore = NULL;
    int iAttributes = 0;
    LPWSTR pwzPFXPassword = NULL;
    LPWSTR pwzFilePath = NULL;
    BYTE* pbData = NULL;
    DWORD cbData = 0;
    DWORD cbPFXPassword = 0;

    BOOL fUserStoreLocation = (CERT_SYSTEM_STORE_CURRENT_USER == dwStoreLocation);
    HCERTSTORE hCertStore = NULL;

    hr = WcaGetProperty(L"CustomActionData", &pwzCaData);
    ExitOnFailure(hr, "Failed to get CustomActionData");

    WcaLog(LOGMSG_TRACEONLY, "CustomActionData: %S", pwzCaData);

    pwz = pwzCaData;
    hr = WcaReadStringFromCaData(&pwz, &pwzName);
    ExitOnFailure(hr, "Failed to parse certificate name.");
    hr = WcaReadStringFromCaData(&pwz, &pwzStore);
    ExitOnFailure(hr, "Failed to parse CustomActionData, StoreName");
    hr = WcaReadIntegerFromCaData(&pwz, &iAttributes);
    ExitOnFailure(hr, "Failed to parse certificate attribute");
    if (SCA_ACTION_INSTALL == saAction) // install operations need more data
    {
        hr = WcaReadStreamFromCaData(&pwz, &pbData, (DWORD_PTR*)&cbData);
        ExitOnFailure(hr, "Failed to parse certificate stream.");

        hr = WcaReadStringFromCaData(&pwz, &pwzPFXPassword);
        ExitOnFailure(hr, "Failed to parse certificate password.");
    }

    // Open the right store.
    hCertStore = ::CertOpenStore(CERT_STORE_PROV_SYSTEM, 0, NULL, dwStoreLocation, pwzStore);
    MessageExitOnNullWithLastError1(hCertStore, hr, msierrCERTFailedOpen, "Failed to open certificate store: %S", pwzStore);

    if (SCA_ACTION_INSTALL == saAction) // install operations need more data
    {
        hr = InstallCertificate(hCertStore, fUserStoreLocation, pwzName, pbData, cbData, pwzPFXPassword);
        ExitOnFailure(hr, "Failed to install certificate.");
    }
    else
    {
        Assert(SCA_ACTION_UNINSTALL == saAction);

        hr = UninstallCertificate(hCertStore, fUserStoreLocation, pwzName);
        ExitOnFailure(hr, "Failed to uninstall certificate.");
    }

LExit:
    if (NULL != pwzPFXPassword && SUCCEEDED(StrSize(pwzPFXPassword, &cbPFXPassword)))
    {
        SecureZeroMemory(pwzPFXPassword, cbPFXPassword);
    }

    if (hCertStore)
    {
        ::CertCloseStore(hCertStore, 0);
    }

    ReleaseMem(pbData);
    ReleaseStr(pwzFilePath);
    ReleaseStr(pwzPFXPassword);
    ReleaseStr(pwzStore);
    ReleaseStr(pwzName);
    ReleaseStr(pwzCaData);
    return hr;
}

static HRESULT AddAdminToSecurityDescriptor(
    __in SECURITY_DESCRIPTOR* pSecurity,
    __out SECURITY_DESCRIPTOR** ppSecurityNew
    )
{
    HRESULT hr = S_OK;
    PACL pAcl = NULL;
    PACL pAclNew = NULL;
    BOOL fValid, fDaclDefaulted;
    ACL_ACE ace[1];
    SECURITY_DESCRIPTOR* pSecurityNew;
    
    if (!::GetSecurityDescriptorDacl(pSecurity, &fValid, &pAcl, &fDaclDefaulted) || !fValid)
    {
        ExitOnLastError(hr, "Failed to get acl from security descriptor");
    }
    
    hr = AclGetWellKnownSid(WinBuiltinAdministratorsSid, &ace[0].psid);
    ExitOnFailure(hr, "failed to get sid for Administrators group");

    ace[0].dwFlags = NO_PROPAGATE_INHERIT_ACE;
    ace[0].dwMask = GENERIC_ALL;

    hr = AclAddToDacl(pAcl, NULL, 0, ace, 1, &pAclNew);
    ExitOnFailure(hr, "failed to add Administrators ACE to ACL");

    hr = AclCreateSecurityDescriptorFromDacl(pAclNew, &pSecurityNew);
    ExitOnLastError(hr, "Failed to create new security descriptor");

    // The DACL is referenced by, not copied into, the security descriptor.  Make sure not to free it.
    pAclNew = NULL;

    *ppSecurityNew = pSecurityNew;
LExit:
    if (pAclNew)
    {
        AclFreeDacl(pAclNew);
    }
    if (ace[0].psid)
    {
        AclFreeSid(ace[0].psid);
    }
    return hr;
}

static HRESULT InstallCertificate(
    __in HCERTSTORE hStore,
    __in BOOL fUserCertificateStore,
    __in LPCWSTR wzName,
    __in_opt BYTE* rgbData,
    __in DWORD cbData,
    __in_opt LPCWSTR wzPFXPassword
    )
{
    HRESULT hr = S_OK;

    HCERTSTORE hPfxCertStore = NULL;
    PCCERT_CONTEXT pCertContext = NULL;
    CERT_BLOB blob = { 0 };
    DWORD dwKeyset = fUserCertificateStore ? CRYPT_USER_KEYSET : CRYPT_MACHINE_KEYSET;
    DWORD dwEncodingType;
    DWORD dwContentType;
    DWORD dwFormatType;
    DWORD dwKeySpec = 0;
    HCRYPTPROV hCsp = NULL;
    LPWSTR pwszTmpContainer = NULL;
    LPWSTR pwszProviderName = NULL;
    DWORD dwProviderType = 0;
    BOOL fAcquired = TRUE;
    SECURITY_DESCRIPTOR* pSecurity = NULL;
    SECURITY_DESCRIPTOR* pSecurityNew = NULL;

    // Figure out what type of blob (certificate or PFX) we're dealing with here.
    blob.pbData = rgbData;
    blob.cbData = cbData;

    if (!::CryptQueryObject(CERT_QUERY_OBJECT_BLOB, &blob, CERT_QUERY_CONTENT_FLAG_ALL, CERT_QUERY_FORMAT_FLAG_ALL, 0, &dwEncodingType, &dwContentType, &dwFormatType, NULL, NULL, (LPCVOID*)&pCertContext))
    {
        ExitWithLastError1(hr, "Failed to parse the certificate blob: %S", wzName);
    }

    if (!pCertContext)
    {
        // If we have a PFX blob, get the first certificate out of the PFX and use that instead of the PFX.
        if (dwContentType & CERT_QUERY_CONTENT_PFX)
        {
            ExitOnNull(wzPFXPassword, hr, E_INVALIDARG, "Failed to import PFX blob because no password was provided");

            // If we fail and our password is blank, also try passing in NULL for the password (according to the docs)
            hPfxCertStore = ::PFXImportCertStore((CRYPT_DATA_BLOB*)&blob, wzPFXPassword, dwKeyset);
            if (NULL == hPfxCertStore && !*wzPFXPassword)
            {
                hPfxCertStore = ::PFXImportCertStore((CRYPT_DATA_BLOB*)&blob, NULL, dwKeyset);
            }
            ExitOnNullWithLastError(hPfxCertStore, hr, "Failed to open PFX file.");

            // There should be at least one certificate in the PFX.
            for(pCertContext = ::CertEnumCertificatesInStore(hPfxCertStore, pCertContext); 
                pCertContext;
                pCertContext = ::CertEnumCertificatesInStore(hPfxCertStore, pCertContext))
            {
                HCRYPTPROV_OR_NCRYPT_KEY_HANDLE hPrivateKey = NULL;

                if (::CryptAcquireCertificatePrivateKey(
                            pCertContext,
                            CRYPT_ACQUIRE_SILENT_FLAG | CRYPT_ACQUIRE_CACHE_FLAG,
                            0,      //pvReserved
                            &hPrivateKey,
                            &dwKeySpec,
                            NULL
                            ))
                {
                    // TODO: We should handle PFXs with multiple certs, we need to determine what is best to do here 
                    //       rather than just breaking on the first cert
                    break;
                }
            }
            ExitOnNull(pCertContext, hr, CRYPT_E_NOT_FOUND, "Could not locate key pair from PFX file.");
        }
        else
        {
            hr = E_UNEXPECTED;
            ExitOnFailure(hr, "Unexpected certificate type processed.");
        }
    }

    // Update the friendly name of the certificate to be configured.
    blob.pbData = (BYTE*)wzName;
    blob.cbData = (lstrlenW(wzName) + 1) * sizeof(WCHAR); // including terminating null

    if (!::CertSetCertificateContextProperty(pCertContext, CERT_FRIENDLY_NAME_PROP_ID, 0, &blob))
    {
        ExitWithLastError1(hr, "Failed to set the friendly name of the certificate: %S", wzName);
    }

    WcaLog(LOGMSG_STANDARD, "Adding certificate: %S", wzName);
    if (!::CertAddCertificateContextToStore(hStore, pCertContext, CERT_STORE_ADD_REPLACE_EXISTING, NULL))
    {
        MessageExitOnLastError(hr, msierrCERTFailedAdd, "Failed to add certificate to the store.");
    }

    if (AT_KEYEXCHANGE == dwKeySpec || AT_SIGNATURE == dwKeySpec)
    {
        // We added a CSP key
        hr = GetCryptProvFromCert(NULL, pCertContext, &hCsp, &dwKeySpec, &fAcquired, &pwszTmpContainer, &pwszProviderName, &dwProviderType);
        ExitOnFailure(hr, "Failed to get handle to CSP");
        
        hr = GetProvSecurityDesc(hCsp, &pSecurity);
        ExitOnFailure(hr, "Failed to get security descriptor of CSP");
        
        hr = AddAdminToSecurityDescriptor(pSecurity, &pSecurityNew);
        ExitOnFailure(hr, "Failed to create new security descriptor");

        hr = SetProvSecurityDesc(hCsp, pSecurityNew);
        ExitOnFailure(hr, "Failed to set Admin ACL on CSP");
    }

    if (CERT_NCRYPT_KEY_SPEC == dwKeySpec)
    {
        // We added a CNG key
        // TODO change ACL on CNG key
    }

    hr = WcaProgressMessage(COST_CERT_ADD, FALSE);
    ExitOnFailure(hr, "Failed to send install progress message.");

LExit:
    if (hCsp)
    {
        FreeCryptProvFromCert(fAcquired, hCsp, NULL, dwProviderType, NULL);
    }

    if (pSecurity)
    {
        MemFree(pSecurity);
    }

    if (pSecurityNew)
    {
        AclFreeSecurityDescriptor(pSecurityNew);
    }

    if (pCertContext)
    {
        ::CertFreeCertificateContext(pCertContext);
    }

    // Close the stores after the context's are released.
    if (hPfxCertStore)
    {
        ::CertCloseStore(hPfxCertStore, 0);
    }

    return hr;
}


static HRESULT UninstallCertificate(
    __in HCERTSTORE hStore,
    __in BOOL fUserCertificateStore,
    __in LPCWSTR wzName
    )
{
    HRESULT hr = S_OK;
    DWORD er = ERROR_SUCCESS;
    PCCERT_CONTEXT pCertContext = NULL;
    CRYPT_KEY_PROV_INFO* pPrivateKeyInfo = NULL;
    DWORD cbPrivateKeyInfo = 0;

    WcaLog(LOGMSG_STANDARD, "Deleting certificate with friendly name: %S", wzName);

    // Loop through all certificates in the store, deleting the ones that match our friendly name.
    while (pCertContext = ::CertFindCertificateInStore(hStore, PKCS_7_ASN_ENCODING | X509_ASN_ENCODING, 0, CERT_FIND_ANY, NULL, pCertContext))
    {
        WCHAR wzFriendlyName[256] = { 0 };
        DWORD cbFriendlyName = sizeof(wzFriendlyName);

        if (::CertGetCertificateContextProperty(pCertContext, CERT_FRIENDLY_NAME_PROP_ID, reinterpret_cast<BYTE*>(wzFriendlyName), &cbFriendlyName) &&
            CSTR_EQUAL == ::CompareStringW(LOCALE_SYSTEM_DEFAULT, 0, wzName, -1, wzFriendlyName, -1))
        {
            PCCERT_CONTEXT pCertContextDelete = ::CertDuplicateCertificateContext(pCertContext); // duplicate the context so we can delete it with out disrupting the looping
            if(pCertContextDelete)
            {
                // Delete the certificate and if successful delete the matching private key as well.
                if (::CertDeleteCertificateFromStore(pCertContextDelete))
                {
                    // If we found private key info, delete it.
                    hr = CertReadProperty(pCertContextDelete, CERT_KEY_PROV_INFO_PROP_ID, &pPrivateKeyInfo);
                    if (SUCCEEDED(hr))
                    {
                        HCRYPTPROV hProvIgnored = NULL; // ignored on deletes.
                        DWORD dwKeyset = fUserCertificateStore ? CRYPT_USER_KEYSET : CRYPT_MACHINE_KEYSET;

                        if (!::CryptAcquireContextW(&hProvIgnored, pPrivateKeyInfo->pwszContainerName, pPrivateKeyInfo->pwszProvName, pPrivateKeyInfo->dwProvType, dwKeyset | CRYPT_DELETEKEYSET | CRYPT_SILENT))
                        {
                            er = ::GetLastError();
                            hr = HRESULT_FROM_WIN32(er);
                        }

                        ReleaseNullMem(pPrivateKeyInfo);
                    }
                    else // don't worry about failures to delete private keys.
                    {
                        hr = S_OK;
                    }
                }
                else
                {
                    er = ::GetLastError();
                    hr = HRESULT_FROM_WIN32(er);
                }

                if (FAILED(hr))
                {
                    WcaLog(LOGMSG_STANDARD, "Failed to delete certificate with friendly name: %S, continuing anyway.  Error: 0x%x", wzFriendlyName, hr);
                }

                pCertContextDelete = NULL;
            }
        }
    }

    hr = WcaProgressMessage(COST_CERT_DELETE, FALSE);
    ExitOnFailure(hr, "Failed to send uninstall progress message.");

LExit:
    ReleaseMem(pPrivateKeyInfo);
    if(pCertContext)
    {
        ::CertFreeCertificateContext(pCertContext);
    }

    return hr;
}
