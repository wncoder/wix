//-------------------------------------------------------------------------------------------------
// <copyright file="payload.cpp" company="Microsoft">
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

#include "precomp.h"


// function definitions

extern "C" HRESULT PayloadsParseFromXml(
    __in BURN_PAYLOADS* pPayloads,
    __in IXMLDOMNode* pixnBundle
    )
{
    HRESULT hr = S_OK;
    IXMLDOMNodeList* pixnNodes = NULL;
    IXMLDOMNode* pixnNode = NULL;
    DWORD cNodes = 0;
    LPWSTR scz = NULL;

    // select payload nodes
    hr = XmlSelectNodes(pixnBundle, L"Payload", &pixnNodes);
    ExitOnFailure(hr, "Failed to select payload nodes.");

    // get payload node count
    hr = pixnNodes->get_length((long*)&cNodes);
    ExitOnFailure(hr, "Failed to get payload node count.");

    if (!cNodes)
    {
        ExitFunction();
    }

    // allocate memory for payloads
    pPayloads->rgPayloads = (BURN_PAYLOAD*)MemAlloc(sizeof(BURN_PAYLOAD) * cNodes, TRUE);
    ExitOnNull(pPayloads->rgPayloads, hr, E_OUTOFMEMORY, "Failed to allocate memory for payload structs.");

    pPayloads->cPayloads = cNodes;

    // parse search elements
    for (DWORD i = 0; i < cNodes; ++i)
    {
        BURN_PAYLOAD* pPayload = &pPayloads->rgPayloads[i];

        hr = XmlNextElement(pixnNodes, &pixnNode, NULL);
        ExitOnFailure(hr, "Failed to get next node.");

        // @Id
        hr = XmlGetAttributeEx(pixnNode, L"Id", &pPayload->sczKey);
        ExitOnFailure(hr, "Failed to get @Id.");

        // @FilePath
        hr = XmlGetAttributeEx(pixnNode, L"FilePath", &pPayload->sczFilePath);
        ExitOnFailure(hr, "Failed to get @FilePath.");

        // @Packaging
        hr = XmlGetAttributeEx(pixnNode, L"Packaging", &scz);
        ExitOnFailure(hr, "Failed to get @Packaging.");

        if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, 0, scz, -1, L"download", -1))
        {
            pPayload->packaging = BURN_PAYLOAD_PACKAGING_DOWNLOAD;
        }
        else if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, 0, scz, -1, L"embedded", -1))
        {
            pPayload->packaging = BURN_PAYLOAD_PACKAGING_EMBEDDED;
        }
        else if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, 0, scz, -1, L"external", -1))
        {
            pPayload->packaging = BURN_PAYLOAD_PACKAGING_EXTERNAL;
        }
        else
        {
            hr = E_INVALIDARG;
            ExitOnFailure1(hr, "Invalid value for @Packaging: %S", scz);
        }

        // @SourcePath
        hr = XmlGetAttributeEx(pixnNode, L"SourcePath", &pPayload->sczSourcePath);
        if (E_NOTFOUND != hr || BURN_PAYLOAD_PACKAGING_DOWNLOAD != pPayload->packaging)
        {
            ExitOnFailure(hr, "Failed to get @SourcePath.");
        }

        // @DownloadUrl
        hr = XmlGetAttributeEx(pixnNode, L"DownloadUrl", &pPayload->sczDownloadUrl);
        if (E_NOTFOUND != hr || BURN_PAYLOAD_PACKAGING_DOWNLOAD == pPayload->packaging)
        {
            ExitOnFailure(hr, "Failed to get @DownloadUrl.");
        }

        // @FileSize
        hr = XmlGetAttributeEx(pixnNode, L"FileSize", &scz);
        if (E_NOTFOUND != hr)
        {
            ExitOnFailure(hr, "Failed to get @FileSize.");

            hr = StrStringToUInt64(scz, 0, &pPayload->qwFileSize);
            ExitOnFailure(hr, "Failed to parse @FileSize.");
        }

        // @Sha1Hash
        hr = XmlGetAttributeEx(pixnNode, L"Sha1Hash", &pPayload->sczSha1Hash);
        if (E_NOTFOUND != hr)
        {
            ExitOnFailure(hr, "Failed to get @Sha1Hash.");
        }

        // @Sha256Hash
        hr = XmlGetAttributeEx(pixnNode, L"Sha256Hash", &pPayload->sczSha256Hash);
        if (E_NOTFOUND != hr)
        {
            ExitOnFailure(hr, "Failed to get @Sha256Hash.");
        }

        // prepare next iteration
        ReleaseNullObject(pixnNode);
    }

    hr = S_OK;

LExit:
    ReleaseObject(pixnNodes);
    ReleaseObject(pixnNode);
    ReleaseStr(scz);

    return hr;
}

extern "C" void PayloadsUninitialize(
    __in BURN_PAYLOADS* pPayloads
    )
{
    if (pPayloads->rgPayloads)
    {
        for (DWORD i = 0; i < pPayloads->cPayloads; ++i)
        {
            BURN_PAYLOAD* pPayload = &pPayloads->rgPayloads[i];

            ReleaseStr(pPayload->sczKey);
            ReleaseStr(pPayload->sczFilePath);
            ReleaseStr(pPayload->sczSha1Hash);
            ReleaseStr(pPayload->sczSha256Hash);
            ReleaseStr(pPayload->sczSourcePath);
            ReleaseStr(pPayload->sczDownloadUrl);
            ReleaseStr(pPayload->sczLocalFilePath);
        }
        MemFree(pPayloads->rgPayloads);
    }
}

extern "C" HRESULT PayloadFindEmbeddedBySourcePath(
    __in BURN_PAYLOADS* pPayloads,
    __in_z LPCWSTR wzStreamName,
    __out BURN_PAYLOAD** ppPayload
    )
{
    HRESULT hr = S_OK;
    BURN_PAYLOAD* pPayload = NULL;

    for (DWORD i = 0; i < pPayloads->cPayloads; ++i)
    {
        pPayload = &pPayloads->rgPayloads[i];
        if (BURN_PAYLOAD_PACKAGING_EMBEDDED == pPayload->packaging)
        {
            if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, 0, pPayload->sczSourcePath, -1, wzStreamName, -1))
            {
                *ppPayload = pPayload;
                ExitFunction1(hr = S_OK);
            }
        }
    }

    hr = E_NOTFOUND;

LExit:
    return hr;
}
