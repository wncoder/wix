//-------------------------------------------------------------------------------------------------
// <copyright file="payload.h" company="Microsoft">
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


#if defined(__cplusplus)
extern "C" {
#endif


// constants

enum BURN_PAYLOAD_PACKAGING
{
    BURN_PAYLOAD_PACKAGING_NONE,
    BURN_PAYLOAD_PACKAGING_DOWNLOAD,
    BURN_PAYLOAD_PACKAGING_EMBEDDED,
    BURN_PAYLOAD_PACKAGING_EXTERNAL,
};


// structs

typedef struct _BURN_PAYLOAD
{
    LPWSTR sczKey;
    BURN_PAYLOAD_PACKAGING packaging;
    DWORD64 qwFileSize;
    LPWSTR sczFilePath; // file path relative to the execute location
    //LPWSTR sczCertificatePublicKey;
    LPWSTR sczSha1Hash;
    LPWSTR sczSha256Hash;
    LPWSTR sczSourcePath;
    LPWSTR sczDownloadUrl;

    LPWSTR sczLocalFilePath;
} BURN_PAYLOAD;

typedef struct _BURN_PAYLOADS
{
    BURN_PAYLOAD* rgPayloads;
    DWORD cPayloads;
} BURN_PAYLOADS;


// functions

HRESULT PayloadsParseFromXml(
    __in BURN_PAYLOADS* pPayloads,
    __in IXMLDOMNode* pixnBundle
    );
void PayloadsUninitialize(
    __in BURN_PAYLOADS* pPayloads
    );
HRESULT PayloadFindEmbeddedBySourcePath(
    __in BURN_PAYLOADS* pPayloads,
    __in_z LPCWSTR wzStreamName,
    __out BURN_PAYLOAD** ppPayload
    );


#if defined(__cplusplus)
}
#endif
