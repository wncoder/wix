#pragma once
//-------------------------------------------------------------------------------------------------
// <copyright file="scassl.h" company="Microsoft Corporation">
//   Copyright (c) 2004, Microsoft Corporation.
//   This software is released under Common Public License Version 1.0 (CPL).
//   The license and further copyright text can be found in the file LICENSE.TXT
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
// 
// <summary>
//    SSL functions for CustomActions
// </summary>
//-------------------------------------------------------------------------------------------------

// misc macros
#define MD_SSL_CERT_HASH                ( IIS_MD_SSL_BASE+6 )
#define MD_SSL_CERT_STORE_NAME          ( IIS_MD_SSL_BASE+11 )
//#define WIDE(x)		WIDE2(x)
//#define WIDE2(x)	L ## x


// structs
struct SCA_WEB_SSL_CERTIFICATE
{
    WCHAR wzStoreName[65];
    BYTE rgbSHA1Hash[CB_CERTIFICATE_HASH];

    SCA_WEB_SSL_CERTIFICATE* pNext;
};


// prototypes
HRESULT ScaSslCertificateRead(
    __in LPCWSTR wzWebId,
    __in WCA_WRAPQUERY_HANDLE hSslCertQuery,
    __inout SCA_WEB_SSL_CERTIFICATE** ppswscList
    );

HRESULT ScaSslCertificateWriteMetabase(
    __in IMSAdminBase* piMetabase,
    __in LPCWSTR wzWebBase,
    __in SCA_WEB_SSL_CERTIFICATE* pswscList
    );

void ScaSslCertificateFreeList(
    __in SCA_WEB_SSL_CERTIFICATE* pswscList
    );
