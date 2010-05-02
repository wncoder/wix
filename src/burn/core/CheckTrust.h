//-------------------------------------------------------------------------------------------------
// <copyright file="CheckTrust.h" company="Microsoft">
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
//    The set of classes are used to handle the signature verification of a signed file.
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once

#include "Interfaces\ILogger.h"
#include "LogSignatureDecorator.h"
#include "ValidCertificate.h"
#include "common\IronManAssert.h"

namespace IronMan
{

template <typename CryptAPI> 
class FileSignatureT
{
private:
    ILogger& m_logger;
    CString m_file;
    const AcceptableCertificates &m_acceptableCertificates;

public:

    FileSignatureT(const CString& file, const AcceptableCertificates &acceptableCertificates, ILogger& logger) 
        : m_file(file), 
        m_logger(logger),
        m_acceptableCertificates(acceptableCertificates)
    {
    }

    virtual ~FileSignatureT(void)
    {
    }

    HRESULT Verify(const CString& friendlyName)
    {
        CString section(L" Signature could not be verified for " + friendlyName); // the default
        PUSHLOGSECTIONPOP(m_logger, GetFilename(), L"Verifying signature for " + friendlyName, section);
        HRESULT hr = this->VerifyWinTrust();
        if (SUCCEEDED(hr))
        {
            hr = this->ValidateSignerCert(friendlyName);
        }

        if (SUCCEEDED(hr))
        {
            section = L" Signature verified successfully for " + friendlyName; 
            LOG(m_logger, ILogger::Verbose, m_file + L" - " + section);
        }
        else  
        {
            CString log;
            log.Format(L"Signature verification for file %s (%s) failed with error 0x%x (%s)", friendlyName, GetFilename(), hr, AtlGetErrorDescription(hr));
            LOG(m_logger, ILogger::Warning, m_file + L" - " + log);
        }

        return hr;
    }

private:
    static const DWORD SHA1_HASH_LEN = 20;
    static const DWORD ENCODING = X509_ASN_ENCODING | PKCS_7_ASN_ENCODING;
    CryptAPI api;
    
    LPWSTR GetFilename(void) 
    {
        return m_file.GetBuffer();
    }

    HRESULT VerifyWinTrust(void)
    {
        WINTRUST_FILE_INFO fileInfo = {0};
        fileInfo.cbStruct = sizeof(WINTRUST_FILE_INFO);
        fileInfo.pcwszFilePath = GetFilename();

        WINTRUST_DATA wintrustData = {0};
        wintrustData.cbStruct = sizeof(WINTRUST_DATA);
        wintrustData.dwProvFlags = WTD_REVOCATION_CHECK_CHAIN_EXCLUDE_ROOT | WTD_CACHE_ONLY_URL_RETRIEVAL | WTD_SAFER_FLAG;
        wintrustData.dwStateAction = WTD_STATEACTION_VERIFY;
        wintrustData.dwUIChoice = WTD_UI_NONE;
        wintrustData.dwUnionChoice = WTD_CHOICE_FILE;
        wintrustData.pFile = &fileInfo;

        GUID guidPublishedSoftware = WINTRUST_ACTION_GENERIC_VERIFY_V2;

        // Get the state data (hWVTStateData member has a handle to the state data)
        LONG lResult =  api.WinVerifyTrust(0, &guidPublishedSoftware, &wintrustData);
        
        if (lResult == 0)
        {
            // Ensure we close the handle hWVTStateData retrieved above
            wintrustData.dwStateAction = WTD_STATEACTION_CLOSE;
            api.WinVerifyTrust(NULL, &guidPublishedSoftware, &wintrustData);
            return S_OK;
        }

        return HRESULT_FROM_WIN32(lResult);
    }

    class SafeCertFreeCertificateContext
    {
        PCCERT_CONTEXT& _pCertContext;
    public:
        //Constructor
        SafeCertFreeCertificateContext(PCCERT_CONTEXT& pCertContext)
            : _pCertContext(pCertContext) {}

        //Destructor
        ~SafeCertFreeCertificateContext(void) 
         {
            if (_pCertContext != NULL) 
            {
                ::CertFreeCertificateContext(_pCertContext); 
            }
            _pCertContext = NULL;
        }
    };

    class SafeCryptMsgClose
    {
        HCRYPTMSG& _hCryptMsg;
    public:
        //Constructor
        SafeCryptMsgClose(HCRYPTMSG& hCryptMsg) 
            : _hCryptMsg(hCryptMsg) {}

        //Destructor
        ~SafeCryptMsgClose(void) 
        {
            if (_hCryptMsg != NULL) 
            {
                ::CryptMsgClose(_hCryptMsg);
            }
            _hCryptMsg = NULL;
        }
    };

    class SafeCertCloseStore
    {
        HCERTSTORE& _hCertStore;
    public:
        //Constructor
        SafeCertCloseStore(HCERTSTORE& hCertStore) 
            : _hCertStore(hCertStore) {}

        //Destructor
        ~SafeCertCloseStore(void)
        {
            if (_hCertStore != NULL)
            {
                ::CertCloseStore(_hCertStore, 0);
            }
            _hCertStore = NULL;
        }
    };

    class SafeCertFreeCertificateChain
    {
        PCCERT_CHAIN_CONTEXT& _pChain;
    public:
        //Constructor
        SafeCertFreeCertificateChain(PCCERT_CHAIN_CONTEXT& pChain) : _pChain(pChain) {}

        //Destructor
        ~SafeCertFreeCertificateChain(void) 
        { 
            if (_pChain != NULL)
            {
                ::CertFreeCertificateChain(_pChain); 
            }
            _pChain = NULL;
        }
    };


    HRESULT ValidateSignerCert(const CString& friendlyName)
    {
        HCRYPTMSG hCryptMsg = NULL;
        SafeCryptMsgClose safeCryptMsgClose(hCryptMsg);

        HCERTSTORE hCertStore = NULL;
        SafeCertCloseStore safeCertCloseStore(hCertStore);
       
        HRESULT hr = this->GetCertificateStore(hCryptMsg, hCertStore);
        if (FAILED(hr))
        {
            LOG(m_logger, ILogger::Verbose, friendlyName + L"(" + m_file + L")"+ L": failed to get certificate. Error: " + AtlGetErrorDescription(hr));
            return hr;
        }

        PCCERT_CONTEXT pCertContext = NULL;
        SafeCertFreeCertificateContext safeCertFreeCertificateContext(pCertContext);

        hr = this->GetCertContext(hCryptMsg, pCertContext);
        if (FAILED(hr))
            return hr;

        return VerifyCertificateChainPolicy(hCryptMsg, hCertStore, pCertContext);
    }

    HRESULT GetCertificateStore(HCRYPTMSG& hCryptMsg, HCERTSTORE& hCertStore)
    {
        BOOL bRet = api.CryptQueryObject(CERT_QUERY_OBJECT_FILE, 
                                       GetFilename(), 
                                       CERT_QUERY_CONTENT_FLAG_PKCS7_SIGNED_EMBED,  // the content type is Embedded PKCS7 signed message
                                       CERT_QUERY_FORMAT_FLAG_ALL,                  // expected format type: any type
                                       0,     
                                       NULL,  
                                       NULL,  
                                       NULL,
                                       &hCertStore,   // a handle of a certificate store that includes all of the certificates, CRLs, and CTLs in the object.
                                       &hCryptMsg,   // a handle of an opened message.
                                       NULL);

        return bRet ? S_OK : HRESULT_FROM_WIN32(GetLastError());
    }

    HRESULT GetCertContext(HCRYPTMSG hCryptMsg, PCCERT_CONTEXT& pCertContext)
    {
        BOOL bRet = api.CryptMsgGetAndVerifySigner(hCryptMsg,     // the message to be verified
                                                 0,
                                                 NULL,
                                                 CMSG_SIGNER_ONLY_FLAG,     // Return the signer without doing the signature verification
                                                 &pCertContext,             // the signer's certificate context.
                                                 NULL); 

        return bRet ? S_OK : HRESULT_FROM_WIN32(GetLastError());
    }

    HRESULT VerifyCertificateChainPolicy(HCRYPTMSG hCryptMsg, HCERTSTORE hCertStore, PCCERT_CONTEXT pCertContext)
    {
        // If we obtain a valid timestamp use this date for CertGetCertificateChain, if not use today's date
        // Builds a certificate chain context starting from an end certificate and going back, if possible, 
        // to a trusted root certificate.
        FILETIME ftTimeStamp = {0};
        HRESULT hrTimeStamp = this->GetDateFromCryptMsg(hCryptMsg, ftTimeStamp);

        CERT_CHAIN_PARA ChainPara = {0};
        ChainPara.RequestedUsage.dwType = USAGE_MATCH_TYPE_AND;

        PCCERT_CHAIN_CONTEXT pChainContext = NULL;
        SafeCertFreeCertificateChain safeCertFreeCertificateChain(pChainContext);

        BOOL bRes = api.CertGetCertificateChain(NULL, 
                                              pCertContext, 
                                              (SUCCEEDED(hrTimeStamp)) ? &ftTimeStamp : NULL, 
                                              hCertStore, 
                                              &ChainPara,
                                              0, 
                                              NULL, 
                                              &pChainContext);

        if (bRes == false)
        {
            return HRESULT_FROM_WIN32(GetLastError());
        }

        CERT_CHAIN_POLICY_PARA BasePolicyPara = {0};
        BasePolicyPara.cbSize = sizeof(BasePolicyPara);

        CERT_CHAIN_POLICY_STATUS BasePolicyStatus = {0};
        BasePolicyStatus.cbSize = sizeof(BasePolicyStatus);

        // checks a certificate chain to verify its validity
        ::SetLastError(ERROR_SUCCESS);
        bRes = api.CertVerifyCertificateChainPolicy(CERT_CHAIN_POLICY_BASE, pChainContext, &BasePolicyPara, &BasePolicyStatus);
        if (FALSE == bRes)
        {
            if (BasePolicyStatus.dwError != ERROR_SUCCESS)
            {
                return HRESULT_FROM_WIN32(BasePolicyStatus.dwError);
            }

            if (::GetLastError() != ERROR_SUCCESS)
            {
                return HRESULT_FROM_WIN32(::GetLastError());
            }
                
            return TRUST_E_FAIL;

        }
        else if (BasePolicyStatus.dwError != ERROR_SUCCESS)
        {
            return HRESULT_FROM_WIN32(BasePolicyStatus.dwError);
        }

        const PCERT_SIMPLE_CHAIN pChain = pChainContext->rgpChain[0];
        DWORD cChainElement = pChain->cElement;

        // Get thumbprint of certificate
        DWORD cbData = 0;
        BYTE* pbSigningThumbprint = NULL;
        if (SUCCEEDED(GetThumbprint(pChain->rgpElement[0]->pCertContext, &cbData, (void**)&pbSigningThumbprint)) )
        {
            if (cChainElement >= 2)
            {
                PCCERT_CONTEXT pCert = pChain->rgpElement[cChainElement - 1]->pCertContext;

                if (pCert)
                {
                    // Make sure that the certificate is signed by an acceptable root and matches the thumbprint if the authoring requires that too
                    return this->EnsureAcceptableCertificates(pCert,pbSigningThumbprint);
                }

            }

            free(pbSigningThumbprint);
        }

        return TRUST_E_FAIL;
    }

    // Get the SHA1 Thumbprint of the certificate as a BYTE array
    HRESULT GetThumbprint(PCCERT_CONTEXT pCertContext, DWORD *pcbData, void** ppvData)
    {
        //  Declare and initialize local variables.
        HRESULT hr = E_FAIL;
        WCHAR pswzNameString[256];

        if (!pcbData || !ppvData)
        {
            hr = E_POINTER;
        }
        else
        {
            //  Retrieve the subject name from the certificate.
            if(CertGetNameString(
                pCertContext,
                CERT_NAME_SIMPLE_DISPLAY_TYPE,
                0,
                NULL,
                pswzNameString,
                128))
            {
                LOGEX(m_logger, ILogger::Verbose, L"Getting Thumbprint for Certificate %s.", pswzNameString); 
            }

            //  Retrieve the SHA1 Thumbprint
            if(CertGetCertificateContextProperty(
                pCertContext, 
                CERT_SHA1_HASH_PROP_ID, 
                NULL, 
                pcbData) )
            {
                //--------------------------------------------------------------------
                // The call succeeded. Use the size to allocate memory for the 
                // property.
                if( (*ppvData = (void*)malloc(*pcbData)) )
                {
                    //--------------------------------------------------------------------
                    // Allocation succeeded. Retrieve the property data.
                    if(CertGetCertificateContextProperty(
                        pCertContext,
                        CERT_SHA1_HASH_PROP_ID,
                        *ppvData, 
                        pcbData) )
                    {
                        WCHAR wzThumbprint[2*(SHA1_HASH_LEN+1)] = {};
                        StrHexEncode((BYTE *)(*ppvData),SHA1_HASH_LEN,wzThumbprint,2*SHA1_HASH_LEN);
                        LOGEX(m_logger, ILogger::Verbose, L"Thumbprint for \"%s\" is %s .", pswzNameString, wzThumbprint); 
                        hr = S_OK;
                    }
                }
            }
        }

        return hr;
    }

    // Check that the top level certificate contains the public key for an acceptable root and
    // if the authoring also restricts to a particular signing certificate thumbprint enforce that too
    HRESULT EnsureAcceptableCertificates(PCCERT_CONTEXT pCert, const BYTE* pbSigningThumbprint)
    {
        BYTE rgbKeyId[SHA1_HASH_LEN];
        DWORD cbKeyId = SHA1_HASH_LEN;

        if (pCert->pCertInfo == NULL)
        {
            return TRUST_E_FAIL;
        }
        
        BOOL result = api.CryptHashPublicKeyInfo(NULL, 
                                               CALG_SHA1, 
                                               0, 
                                               X509_ASN_ENCODING, 
                                               &pCert->pCertInfo->SubjectPublicKeyInfo, 
                                               rgbKeyId, 
                                               &cbKeyId);

        if (!result)
        {
            return HRESULT_FROM_WIN32(GetLastError());
        }
        
        if ( ValidCertificate::IsValidCertificate(m_acceptableCertificates, pbSigningThumbprint, rgbKeyId, SHA1_HASH_LEN) )
        {
            return S_OK;
        }
        else
        {
            return SEC_E_CERT_UNKNOWN;
        }
    }

    HRESULT GetDateFromCryptMsg(HCRYPTMSG hCryptMsg, FILETIME& ftTimeStamp)
    {
        HRESULT hr = TRUST_E_FAIL;

        DWORD dwSignerInfo = 0;
        BOOL fResult = api.CryptMsgGetParam(hCryptMsg, CMSG_SIGNER_INFO_PARAM, 0, NULL, &dwSignerInfo);
        if (!fResult)
        {
            return HRESULT_FROM_WIN32(GetLastError());
        }

        PCMSG_SIGNER_INFO pSignerInfo = (PCMSG_SIGNER_INFO) LocalAlloc(LPTR, dwSignerInfo);
        if (!pSignerInfo)
        {
            return E_OUTOFMEMORY;
        }
        
        fResult = api.CryptMsgGetParam(hCryptMsg, CMSG_SIGNER_INFO_PARAM, 0, (PVOID)pSignerInfo, &dwSignerInfo);
        if (!fResult)
        {
            return HRESULT_FROM_WIN32(GetLastError());
        }

        hr = this->GetSigningTimeOfCounterSigner(*pSignerInfo, ftTimeStamp);

        if (pSignerInfo)
        {
            LocalFree(pSignerInfo);
        }

        return hr;
    }

    template<typename CryptAPI>
    class CryptMessageSignerT
    {
        PCMSG_SIGNER_INFO m_pCounterSigner;
        const CMSG_SIGNER_INFO& m_signer;
        CryptAPI api;

    public:
        //Constructor
        CryptMessageSignerT(const CMSG_SIGNER_INFO& signer) 
            : m_signer(signer)
            , m_pCounterSigner(NULL)
        {
        }

        //Destructor
        ~CryptMessageSignerT(void)
        {
            if (m_pCounterSigner)
            {
                ::LocalFree(m_pCounterSigner);
            }
        }

        const CMSG_SIGNER_INFO& GetCounterSigner(HRESULT& hr)
        {
            hr = GetCounterSignerInternal();

            if (FAILED(hr))
            {
                IMASSERT(m_pCounterSigner == NULL);
                m_pCounterSigner = (PCMSG_SIGNER_INFO)::LocalAlloc(LPTR, sizeof(CMSG_SIGNER_INFO));
            }
            else
            {
                // Should never be NULL if it succeeds
                IMASSERT(m_pCounterSigner != NULL);
            }

            return *m_pCounterSigner;
        }

    private:
        HRESULT GetCounterSignerInternal(void)
        {
            bool bFoundCounterSigner = false;
            UINT indexCounterSign = 0;

            // Loop through unathenticated attributes for szOID_RSA_counterSign OID.
            for (UINT n = 0; n < m_signer.UnauthAttrs.cAttr; n++)
            {
                if (lstrcmpA(m_signer.UnauthAttrs.rgAttr[n].pszObjId, szOID_RSA_counterSign) == 0)
                {
                    bFoundCounterSigner = true;
                    indexCounterSign = n;
                    break;
                }
            }

            if (bFoundCounterSigner == false)
            {
                return TRUST_E_FAIL;
            }

            const CRYPT_INTEGER_BLOB& counterSignerBlob = m_signer.UnauthAttrs.rgAttr[indexCounterSign].rgValue[0];

            // Get size of CMSG_SIGNER_INFO structure.
            DWORD dwSize = 0;
            BOOL fResult = api.CryptDecodeObject(ENCODING, 
                                               PKCS7_SIGNER_INFO, 
                                               counterSignerBlob.pbData,
                                               counterSignerBlob.cbData,
                                               0,
                                               NULL,
                                               &dwSize);
            if (fResult == FALSE)
            {
                return HRESULT_FROM_WIN32(GetLastError());
            }

            
            PCMSG_SIGNER_INFO pCounterSignerInfo = (PCMSG_SIGNER_INFO)::LocalAlloc(LPTR, dwSize);
            if (pCounterSignerInfo == NULL)
            {
                return E_OUTOFMEMORY;
            }

            // Decode and get CMSG_SIGNER_INFO structure for timestamp certificate.
            fResult = api.CryptDecodeObject(ENCODING, 
                                          PKCS7_SIGNER_INFO, 
                                          counterSignerBlob.pbData, 
                                          counterSignerBlob.cbData, 
                                          0, 
                                          (PVOID)pCounterSignerInfo, 
                                          &dwSize);
            if (fResult)
            {
                m_pCounterSigner = pCounterSignerInfo;
                return S_OK;
            }
            else
            {
                LocalFree(pCounterSignerInfo);
                return HRESULT_FROM_WIN32(GetLastError());
            }
        }
    };

    HRESULT GetSigningTimeOfCounterSigner(const CMSG_SIGNER_INFO& signerInfo, FILETIME& localSigningTime)
    {
        // Get the CounterSigner Information
        HRESULT hr = S_OK;
        CryptMessageSignerT<CryptAPI> signer(signerInfo);
        const CMSG_SIGNER_INFO& counterSigner = signer.GetCounterSigner(hr);
        if (FAILED(hr))
        {
            return hr;
        }
        
        // Loop through authenticated attributes and find szOID_RSA_signingTime OID.
        bool bFoundRSASigningTime = false;
        UINT indexRSASigningTime = 0;
        for (DWORD n = 0; n < counterSigner.AuthAttrs.cAttr; n++)
        {         
            if (lstrcmpA(szOID_RSA_signingTime, counterSigner.AuthAttrs.rgAttr[n].pszObjId) == 0)
            {            
                bFoundRSASigningTime = true;
                indexRSASigningTime = n;
                break;
            }
        }

        if (bFoundRSASigningTime == false)
        {
            return TRUST_E_FAIL;
        }

        const CRYPT_INTEGER_BLOB& rsaSigningTimeBlob = counterSigner.AuthAttrs.rgAttr[indexRSASigningTime].rgValue[0];
        
        // Decode and get FILETIME structure.
        FILETIME fileTime;
        DWORD dwData = sizeof(fileTime);
        BOOL fResult = api.CryptDecodeObject(ENCODING, 
                                           szOID_RSA_signingTime, 
                                           rsaSigningTimeBlob.pbData,
                                           rsaSigningTimeBlob.cbData,
                                           0,
                                           (PVOID)&fileTime,
                                           &dwData);

        if (fResult)
        {
            fResult = api.FileTimeToLocalFileTime(&fileTime, &localSigningTime);
        }
        
        return fResult ? S_OK : HRESULT_FROM_WIN32(GetLastError());
    }
};

class WindowsCryptAPI
{
public:
    virtual LONG WinVerifyTrust(HWND hwnd, GUID *pgActionID, LPVOID pWVTData)
    {
#pragma warning (push)
#pragma warning( disable:25028 )
        return ::WinVerifyTrust(hwnd, pgActionID, pWVTData);
#pragma warning (pop)
    }

    virtual BOOL CertGetCertificateChain (HCERTCHAINENGINE hChainEngine, PCCERT_CONTEXT pCertContext, 
                                          LPFILETIME pTime, HCERTSTORE hAdditionalStore, PCERT_CHAIN_PARA pChainPara, 
                                          DWORD dwFlags, LPVOID pvReserved, PCCERT_CHAIN_CONTEXT* ppChainContext) 
    {
        return ::CertGetCertificateChain(hChainEngine, pCertContext, pTime, hAdditionalStore, pChainPara, 
                                         dwFlags, pvReserved, ppChainContext); 
    }
    
    virtual BOOL CertVerifyCertificateChainPolicy(LPCSTR pszPolicyOID, PCCERT_CHAIN_CONTEXT pChainContext, 
                                                  PCERT_CHAIN_POLICY_PARA pPolicyPara, 
                                                  PCERT_CHAIN_POLICY_STATUS pPolicyStatus) 
    {
        return ::CertVerifyCertificateChainPolicy(pszPolicyOID, pChainContext, pPolicyPara, pPolicyStatus); 
    }
    
    virtual BOOL CryptQueryObject(DWORD dwObjectType,  const void *pvObject, DWORD dwExpectedContentTypeFlags, 
                                  DWORD dwExpectedFormatTypeFlags, DWORD dwFlags, DWORD *pdwMsgAndCertEncodingType, 
                                  DWORD *pdwContentType, DWORD *pdwFormatType, HCERTSTORE *phCertStore, 
                                  HCRYPTMSG  *phMsg, const void **ppvContext)
    {
        return ::CryptQueryObject(dwObjectType,  pvObject, dwExpectedContentTypeFlags, dwExpectedFormatTypeFlags, 
                                  dwFlags, pdwMsgAndCertEncodingType, pdwContentType, pdwFormatType, phCertStore, 
                                  phMsg, ppvContext); 
    }
    
    virtual BOOL CryptMsgGetAndVerifySigner(HCRYPTMSG hCryptMsg, DWORD cSignerStore, HCERTSTORE *rghSignerStore, 
                                            DWORD dwFlags, PCCERT_CONTEXT *ppSigner, DWORD *pdwSignerIndex) 
    {
        return ::CryptMsgGetAndVerifySigner(hCryptMsg, cSignerStore, rghSignerStore, dwFlags, ppSigner, pdwSignerIndex); 
    }
    
    virtual BOOL CryptHashPublicKeyInfo(HCRYPTPROV_LEGACY hCryptProv, ALG_ID Algid, DWORD dwFlags, 
                                        DWORD dwCertEncodingType, PCERT_PUBLIC_KEY_INFO pInfo, 
                                        BYTE *pbComputedHash, DWORD *pcbComputedHash) 
    {
        
        if (pInfo && pbComputedHash && pcbComputedHash)
        {
            return ::CryptHashPublicKeyInfo(hCryptProv, Algid, dwFlags, dwCertEncodingType, 
                                                    pInfo, pbComputedHash, pcbComputedHash); 
        }
        else
        {
            ::SetLastError(ERROR_INVALID_PARAMETER);
            return 0;
        }
    }
    
    
    virtual BOOL CryptMsgGetParam(HCRYPTMSG hCryptMsg, DWORD dwParamType, DWORD dwIndex, void *pvData, DWORD *pcbData) 
    {
        if (pcbData && (*pcbData == 0 || pvData != NULL))
        {
            return ::CryptMsgGetParam(hCryptMsg, dwParamType, dwIndex, pvData, pcbData); 
        }
        else
        {
            ::SetLastError(ERROR_INVALID_PARAMETER);
            return 0;
        }
    }

    virtual BOOL CryptDecodeObject(DWORD dwCertEncodingType, LPCSTR lpszStructType, const BYTE *pbEncoded, 
                                DWORD cbEncoded, DWORD dwFlags, void *pvStructInfo, DWORD *pcbStructInfo) 
    {
        if (pbEncoded && pcbStructInfo && (*pcbStructInfo == 0 || pvStructInfo != NULL))
        {
            return ::CryptDecodeObject(dwCertEncodingType, lpszStructType, pbEncoded, cbEncoded, 
                                                            dwFlags, pvStructInfo, pcbStructInfo); 
        }
        else
        {
            ::SetLastError(ERROR_INVALID_PARAMETER);
            return 0;
        }
    }
    
    
    virtual BOOL FileTimeToLocalFileTime(CONST FILETIME *lpFileTime, LPFILETIME lpLocalFileTime)
    {
        return ::FileTimeToLocalFileTime(lpFileTime, lpLocalFileTime);
    }
};

typedef FileSignatureT<WindowsCryptAPI> FileSignature;

class FileAuthenticity
{
    CString& m_hash;
    const CPath& m_dst;
    ULONGLONG m_itemSize;
    ILogger& m_logger;
    const CString m_friendlyName;
    const AcceptableCertificates &m_acceptableCertificates;

public:

    //constructor
    FileAuthenticity(
        const CString& friendlyName,
        const CPath& dst,
        CString& hash, 
        ULONGLONG itemSize,
        ILogger& logger)
        :
        m_dst(dst),
        m_hash(hash),
        m_friendlyName(friendlyName),
        m_itemSize(itemSize),
        m_logger(logger),
        m_acceptableCertificates(Configuration::GetAcceptableCertificates(NULL,logger))
    {
    }

    HRESULT Verify()
    {
        HRESULT hr = S_OK;
        DWORD64 qwHashedBytes;
        WIN32_FILE_ATTRIBUTE_DATA data = { };
        BYTE rgbExpectedHash[20] = { };
        BYTE rgbActualHash[20] = { };

        // check signature
        FileSignature fileSignature(m_dst, m_acceptableCertificates, m_logger);
        hr = fileSignature.Verify(m_friendlyName);
        if (SUCCEEDED(hr))
        {
            LOG(m_logger, ILogger::Information, L"Signature verification succeeded for " + m_friendlyName);
            ExitFunction();
        }

        LOG(m_logger, ILogger::Information, L"Signature verification failed. Trying to verify hash " + m_friendlyName + L"... ");

        // get expected hash
        if (m_hash.IsEmpty())
        {
            LOG(m_logger, ILogger::Warning, L"No FileHash provided. Cannot perform FileHash verification for " + m_friendlyName);
            ExitFunction();
        }

        hr = StrHexDecode((LPCWSTR)m_hash, rgbExpectedHash, sizeof(rgbExpectedHash));
        ExitOnFailure(hr, "Failed to decode expected hash.");

        // get file hash
        hr = CrypHashFile((LPCWSTR)m_dst, PROV_RSA_FULL, CALG_SHA1, rgbActualHash, sizeof(rgbActualHash), &qwHashedBytes);
        if (FAILED(hr))
        {
            CString msg;
            msg.Format(L"Hash verification failed for %s.  HRESULT = 0x%x", m_friendlyName, hr);
            LOG(m_logger, ILogger::Error, msg);
            
            ExitOnFailure1(hr, "Failed to calculate hash for %s", (LPCWSTR)m_friendlyName);
        }

        // compare hashes
        C_ASSERT(20 == sizeof(rgbExpectedHash) && 20 == sizeof(rgbActualHash)); // memcmp() call below expects buffers to be 20 bytes
        if (0 != memcmp(rgbExpectedHash, rgbActualHash, 20))
        {
            hr = CRYPT_E_HASH_VALUE;

            CString msg;
            msg.Format(L"Hash verification failed for %s.  HRESULT = 0x%x", m_friendlyName, hr);
            LOG(m_logger, ILogger::Error, msg);

            ExitOnFailure1(hr, "Hash mismatch for %s", (LPCWSTR)m_friendlyName);
        }

        // compare file size
        if (m_itemSize != qwHashedBytes)
        {
            LOG(m_logger, ILogger::Error, L"Hash verification succeeded but file size does not match for " + m_friendlyName);

            hr = CRYPT_E_HASH_VALUE;
            ExitOnFailure1(hr, "File size mismatch for %s", (LPCWSTR)m_friendlyName);
        }

        LOG(m_logger, ILogger::Information, L"Hash verification succeeded for " + m_friendlyName);

    LExit:
        return hr;
    }
};

} // IronMan
