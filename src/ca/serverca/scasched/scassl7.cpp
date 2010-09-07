//-------------------------------------------------------------------------------------------------
// <copyright file="scassl7.cpp" company="Microsoft">
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
//    IIS SSL functions for CustomActions
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"
static LPCWSTR vcsSslCertificateQuery = L"SELECT `Certificate`.`StoreName`, `CertificateHash`.`Hash` FROM `Certificate`, `CertificateHash`, `IIsWebSiteCertificates` WHERE `Certificate`.`Certificate`=`CertificateHash`.`Certificate_` AND `CertificateHash`.`Certificate_`=`IIsWebSiteCertificates`.`Certificate_` AND `IIsWebSiteCertificates`.`Web_`=?";
enum eSslCertificateQuery { scqStoreName = 1, scqHash, scqWeb };

static HRESULT AddSslCertificateToList(
    __in SCA_WEB_SSL_CERTIFICATE** ppswscList
    );


HRESULT ScaSslCertificateRead7(
    __in_z LPCWSTR wzWebId,
    __inout SCA_WEB_SSL_CERTIFICATE** ppswscList
    )
{
    HRESULT hr = S_OK;

    PMSIHANDLE hView, hRec;
    SCA_WEB_SSL_CERTIFICATE* pswsc = NULL;
    LPWSTR pwzData = NULL;

    hr = WcaTableExists(L"IIsWebSiteCertificates");
    if (S_FALSE == hr)
    {
        ExitFunction1(hr = S_OK);
    }
    ExitOnFailure(hr, "Failed to determine if IIsWebSiteCertificates table existed.");

    hRec = ::MsiCreateRecord(1);
    hr = WcaSetRecordString(hRec, 1, wzWebId);
    ExitOnFailure(hr, "Failed to set record to look up web site.");

    hr = WcaOpenView(vcsSslCertificateQuery, &hView);
    ExitOnFailure(hr, "Failed to open view on IIsWebSiteCertificates table.");

    hr = WcaExecuteView(hView, hRec);
    ExitOnFailure(hr, "Failed to execute view on IIsWebSiteCertificates table.");

    // Get the certificate information.
    while (S_OK == (hr = WcaFetchRecord(hView, &hRec)))
    {
        hr = AddSslCertificateToList(ppswscList);
        ExitOnFailure(hr, "failed to add ssl certificate to list");

        pswsc = *ppswscList;

        hr = WcaGetRecordString(hRec, scqStoreName, &pwzData);
        ExitOnFailure(hr, "Failed to get web ssl certificate store name.");

        hr = ::StringCchCopyW(pswsc->wzStoreName, countof(pswsc->wzStoreName), pwzData);
        ExitOnFailure(hr, "Failed to copy web ssl certificate store name.");

        hr = WcaGetRecordString(hRec, scqHash, &pwzData);
        ExitOnFailure(hr, "Failed to get hash for web ssl certificate.");

        hr = StrHexDecode(pwzData, pswsc->rgbSHA1Hash, countof(pswsc->rgbSHA1Hash));
        ExitOnFailure2(hr, "Failed to decode certificate hash for web: %S, data: %S", wzWebId, pwzData);
    }

    if (E_NOMOREITEMS == hr)
    {
        hr = S_OK;
    }
    ExitOnFailure(hr, "Failed to read IIsWebSiteCertificates table.");

LExit:
    ReleaseStr(pwzData);

    return hr;
}


HRESULT ScaSslCertificateWrite7(
    __in_z LPCWSTR wzWebBase,
    __in SCA_WEB_SSL_CERTIFICATE* pswscList
    )
{
    HRESULT hr = S_OK;    
    WCHAR wzEncodedCertificateHash[CB_CERTIFICATE_HASH * 2 + 1] = { 0 };

    for (SCA_WEB_SSL_CERTIFICATE* pswsc = pswscList; pswsc; pswsc = pswsc->pNext)
    {   
        hr = ScaWriteConfigID(IIS_SSL_BINDING);
        ExitOnFailure(hr, "Failed write SSL binding ID");
        hr = ScaWriteConfigID(IIS_CREATE);                      // Need to determine site action
        ExitOnFailure(hr, "Failed write binding action");

        hr = ScaWriteConfigString(wzWebBase);                   //site name key
        ExitOnFailure(hr, "Failed to write SSL website");
        hr = ScaWriteConfigString(pswsc->wzStoreName);          //ssl store name
        ExitOnFailure(hr, "Failed to write SSL store name");
        
        hr = StrHexEncode(pswsc->rgbSHA1Hash, countof(pswsc->rgbSHA1Hash), wzEncodedCertificateHash, countof(wzEncodedCertificateHash));
        ExitOnFailure(hr, "Failed to encode SSL hash");

        hr = ScaWriteConfigString(wzEncodedCertificateHash);    //ssl hash
        ExitOnFailure(hr, "Failed to write SSL hash");
    }
LExit:

    return hr;
}


void ScaSslCertificateFreeList7(
    __in SCA_WEB_SSL_CERTIFICATE* pswscList
    )
{
    SCA_WEB_SSL_CERTIFICATE* pswscDelete = pswscList;

    while (pswscList)
    {
        pswscDelete = pswscList;
        pswscList = pswscList->pNext;

        MemFree(pswscDelete);
    }
}


static HRESULT AddSslCertificateToList(
    __in SCA_WEB_SSL_CERTIFICATE** ppswscList
    )
{
    HRESULT hr = S_OK;

    SCA_WEB_SSL_CERTIFICATE* pswsc = static_cast<SCA_WEB_SSL_CERTIFICATE*>(MemAlloc(sizeof(SCA_WEB_SSL_CERTIFICATE), TRUE));
    ExitOnNull(pswsc, hr, E_OUTOFMEMORY, "failed to allocate memory for new SSL certificate list element");

    pswsc->pNext = *ppswscList;
    *ppswscList = pswsc;

LExit:
    return hr;
}
