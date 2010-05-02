//-------------------------------------------------------------------------------------------------
// <copyright file="manifest.cpp" company="Microsoft">
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

extern "C" HRESULT ManifestLoadXmlFromBuffer(
    __in_bcount(cbBuffer) BYTE* pbBuffer,
    __in SIZE_T cbBuffer,
    __in BURN_ENGINE_STATE* pEngineState
    )
{
    HRESULT hr = S_OK;
    IXMLDOMDocument* pixdDocument = NULL;
    IXMLDOMElement* pixeBundle = NULL;

    // load xml document
    hr = XmlLoadDocumentFromBuffer(pbBuffer, cbBuffer, &pixdDocument);
    ExitOnFailure(hr, "Failed to load manifest as XML document.");

    // get bundle element
    hr = pixdDocument->get_documentElement(&pixeBundle);
    ExitOnFailure(hr, "Failed to get bundle element.");

    // parse variables
    hr = VariablesParseFromXml(&pEngineState->variables, pixeBundle);
    ExitOnFailure(hr, "Failed parse searches.");

    // parse searches
    hr = SearchesParseFromXml(&pEngineState->searches, pixeBundle); // TODO: Modularization
    ExitOnFailure(hr, "Failed parse searches.");

    //// parse user experience
    //hr = UserExperienceParseFromXml(&pEngineState->userExperience, pixeBundle);
    //ExitOnFailure(hr, "Failed parse user experience.");

    //// parse registration
    //hr = RegistrationParseFromXml(&pEngineState->registration, pixeBundle);
    //ExitOnFailure(hr, "Failed parse registration.");

    //// parse payloads
    //hr = PayloadsParseFromXml(&pEngineState->payloads, pixeBundle);
    //ExitOnFailure(hr, "Failed parse payloads.");

    //// parse packages
    //hr = PackagesParseFromXml(&pEngineState->packages, pixeBundle);
    //ExitOnFailure(hr, "Failed parse packages.");

LExit:
    ReleaseObject(pixdDocument);
    ReleaseObject(pixeBundle);
    return hr;
}

/********************************************************************
 ParseManifestPayload - parses the payload information from UX, 
    Resource, and package elements and returns an IBurnPayload.
*********************************************************************/
extern "C" HRESULT ParseManifestPayload(
    __in_z LPCWSTR wzBundleId,
    __in_z LPCWSTR wzBundleFileName,
    __in_z_opt LPCWSTR wzSpecialPayloadPath,
    __in IXMLDOMNode* pNode,
    __in DWORD dwPayloadsOffset,
    __in BOOL fTemporary,
    __out IBurnPayload** ppPayload
    )
{
    HRESULT hr = S_OK;
    IXMLDOMNamedNodeMap* pixnnmAttributes = NULL;
    IXMLDOMNode* pixnAttribute = NULL;
    BSTR bstrNodeName = NULL;
    BSTR bstrNodeValue = NULL;
    LPWSTR sczFileName = NULL;
    LPWSTR sczLocalName = NULL;
    LPWSTR sczDownloadSource = NULL;
    LPWSTR sczRelativeDirectory = NULL;
    LPWSTR sczSourceDirectory = NULL;
    LPWSTR sczEmbeddedId = NULL;
    LPWSTR sczSourcePath = NULL;
    DWORD64 cbSize = 0;
    DWORD dwOffset = 0;
    SOURCE_TYPE sourceType = SOURCE_TYPE_EMBEDDED;

    hr = pNode->get_attributes(&pixnnmAttributes);
    ExitOnFailure(hr, "Failed to get attributes on Resource element.");

    while (S_OK == (hr = pixnnmAttributes->nextNode(&pixnAttribute)))
    {
        // get name and value for each attribute
        hr = pixnAttribute->get_nodeName(&bstrNodeName);
        ExitOnFailure(hr, "Failed to get Resource attribute name.");

        hr = XmlGetText(pixnAttribute, &bstrNodeValue);
        ExitOnFailure1(hr, "Failed to get Resource attribute value: %S", bstrNodeName);

        if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, 0, bstrNodeName, -1, L"FileName", -1))
        {
            hr = StrAllocString(&sczFileName, bstrNodeValue, 0);
            ExitOnFailure(hr, "Failed to allocate @FileName.");
        }
        else if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, 0, bstrNodeName, -1, L"DownloadSource", -1))
        {
            hr = StrAllocString(&sczDownloadSource, bstrNodeValue, 0);
            ExitOnFailure(hr, "Failed to allocate @DownloadSource.");
        }
        else if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, 0, bstrNodeName, -1, L"EmbeddedId", -1))
        {
            hr = StrAllocString(&sczEmbeddedId, bstrNodeValue, 0);
            ExitOnFailure(hr, "Failed to allocate @EmbeddedId.");
        }
        else if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, 0, bstrNodeName, -1, L"FileSize", -1))
        {
            cbSize = static_cast<DWORD64>(_wcstoui64(bstrNodeValue, NULL, 10));
        }
        else if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, 0, bstrNodeName, -1, L"Offset", -1))
        {
            dwOffset = static_cast<DWORD>(wcstoul(bstrNodeValue, NULL, 10));
        }
        else if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, 0, bstrNodeName, -1, L"Packaging", -1))
        {
            if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, 0, bstrNodeValue, -1, L"embedded", -1))
            {
                sourceType = SOURCE_TYPE_EMBEDDED;
            }
            else if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, 0, bstrNodeValue, -1, L"download", -1))
            {
                sourceType = SOURCE_TYPE_DOWNLOAD;
            }
            else if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, 0, bstrNodeValue, -1, L"external", -1))
            {
                sourceType = SOURCE_TYPE_EXTERNAL;
            }
        }
        else if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, 0, bstrNodeName, -1, L"RelativeDirectory", -1))
        {
            hr = StrAllocString(&sczRelativeDirectory, bstrNodeValue, 0);
            ExitOnFailure(hr, "Failed to allocate @RelativePath.");
        }

        ReleaseNullBSTR(bstrNodeValue);
        ReleaseNullBSTR(bstrNodeName);
    }

    if (S_FALSE == hr)
    {
        hr = S_OK;
    }
    ExitOnFailure(hr, "Failed to enumerate all payload attributes.");

    // we have the data from the manifest, now create the payload object
    hr = PathConcat(sczRelativeDirectory, sczFileName, &sczLocalName);
    ExitOnFailure(hr, "Failed to allocate local name for resource.");

    if (SOURCE_TYPE_EMBEDDED == sourceType)
    {
        if (!sczEmbeddedId)
        {
            hr = E_UNEXPECTED;
        }
        else
        {
            hr = BurnCreateEmbeddedPayload(wzBundleId, wzBundleFileName, dwPayloadsOffset, sczEmbeddedId, sczLocalName, wzSpecialPayloadPath, fTemporary, ppPayload);
        }
    }
    else if (SOURCE_TYPE_EXTERNAL == sourceType  ||  SOURCE_TYPE_DOWNLOAD == sourceType)
    {
        hr = PathGetDirectory(wzBundleFileName, &sczSourceDirectory);
        ExitOnFailure(hr, "Failed to get source directory for payload.");

        hr = PathConcat(sczSourceDirectory, sczFileName, &sczSourcePath);
        ExitOnFailure2(hr, "Failed to combine source directory and file name: %ls, %ls", sczSourceDirectory, sczFileName);

        hr = BurnCreatePayload(sourceType, wzBundleId, sczSourcePath, sczLocalName, wzSpecialPayloadPath, cbSize, NULL, 0, fTemporary, ppPayload);
    }
    else
    {
        // SOURCE_TYPE_NONE payloads should never happen - Note: if new types of Packaging are added then payload objects will need to be created for them here
        hr = E_UNEXPECTED;
    }
    ExitOnFailure1(hr, "Failed to create payload object for %ls.", sczFileName);

LExit:
    ReleaseBSTR(bstrNodeValue);
    ReleaseBSTR(bstrNodeName);
    ReleaseObject(pixnAttribute);
    ReleaseObject(pixnnmAttributes);
    ReleaseStr(sczFileName);
    ReleaseStr(sczLocalName);
    ReleaseStr(sczDownloadSource);
    ReleaseStr(sczRelativeDirectory);
    ReleaseStr(sczSourceDirectory);
    ReleaseStr(sczEmbeddedId);
    ReleaseStr(sczSourcePath);

    return hr;
}

/********************************************************************
 ParsePayloads - parses child elements such as Resources, plus the parent
    element (package/UX) if applicable, into an array of payloads.
*********************************************************************/
extern "C" HRESULT ParsePayloads(
    __in_z LPCWSTR wzElementName,
    __in_z LPCWSTR wzBundleId,
    __in_z LPCWSTR wzBundleFileName,
    __in_z_opt LPCWSTR wzSpecialPayloadPath,
    __in IXMLDOMNode* pixn,
    __in DWORD dwPayloadsOffset,
    __in BOOL fTemporary,
    __out DWORD* pcPayloads,
    __out IBurnPayload*** prgpPayloads
    )
{
    HRESULT hr = S_OK;
    IXMLDOMNodeList* pNodeList = NULL;
    IXMLDOMNode* pNode = NULL;
    BSTR bstrNodeName = NULL;
    IBurnPayload** rgpNewPayloads = NULL;
    DWORD cPayloads = 0;
    BSTR bstrElementName = ::SysAllocString(wzElementName);

    hr = pixn->selectNodes(bstrElementName, &pNodeList);
    ExitOnFailure(hr, "Failed to select all Resource children.");

    hr = pNodeList->get_length(reinterpret_cast<long*>(&cPayloads));
    ExitOnFailure(hr, "Failed to get the Resource count.");

    // plus one for the parent [package,UX dll] itself
    ++cPayloads;

    rgpNewPayloads = static_cast<IBurnPayload**>(MemAlloc(sizeof(IBurnPayload*) * cPayloads, TRUE));
    ExitOnNull(rgpNewPayloads, hr, E_OUTOFMEMORY, "Failed to allocate memory for payloads.");

    // Create the first payload object from the parent's payload attributes
    hr = ParseManifestPayload(wzBundleId, wzBundleFileName, wzSpecialPayloadPath, pixn, dwPayloadsOffset, fTemporary, rgpNewPayloads);
    ExitOnFailure(hr, "Failed to parse payload from manifest.");

    // now create payload objects for the Resources
    DWORD dwCurrentPayload = 1;
    while (S_OK == (hr = XmlNextElement(pNodeList, &pNode, &bstrNodeName)))
    {
        hr = ParseManifestPayload(wzBundleId, wzBundleFileName, wzSpecialPayloadPath, pNode, dwPayloadsOffset, fTemporary, rgpNewPayloads + dwCurrentPayload);
        ExitOnFailure(hr, "Failed to parse payload element from manifest.");

        ++dwCurrentPayload;
        ReleaseNullBSTR(bstrNodeName);
        ReleaseNullObject(pNode);
    }

    if (S_FALSE == hr)
    {
        hr = S_OK;
    }

    *pcPayloads = cPayloads;
    *prgpPayloads = rgpNewPayloads;
    rgpNewPayloads = NULL;

LExit:
    ReleaseBSTR(bstrNodeName);
    ReleaseBSTR(bstrElementName);
    ReleaseObject(pNode);
    ReleaseObject(pNodeList);
    ReleaseObjectArray(rgpNewPayloads, cPayloads);

    return hr;
}

/********************************************************************
 ParseManifestDocument - parses out a manifest from a loaded XML DOM 
    document.
*********************************************************************/
extern "C" HRESULT ParseManifestDocument(
    __in_z LPCWSTR wzBundleId,
    __in_z LPCWSTR wzBundleFileName,
    __in_z LPCWSTR wzUXWorkingPath,
    __in IXMLDOMDocument* pixd,
    __in DWORD dwPayloadsOffset,
    __in DWORD dwAttachedContainerOffset,
    __out DWORD* pcChainedPayloads,
    __out DWORD* pcUXPayloads,
    __out IBurnPayload*** prgpUxPayloads,
    __out IBurnPayload*** prgpChainedPayloads
    )
{
    Assert(pixd);
    Assert(pcChainedPayloads);
    Assert(prgpUxPayloads);
    Assert(pcUXPayloads);

    HRESULT hr = S_OK;
    IXMLDOMElement* pManifestElement = NULL;
    IXMLDOMNodeList* pNodes = NULL;
    IXMLDOMNode* pNode = NULL;
    BSTR bstrNodeName = NULL;

    IBurnPayload** rgpNewUXPayloads = NULL;
    IBurnPayload** rgpChainedPayloads = NULL;

    //
    // Get the document element and start processing packages.
    //
    hr = pixd->get_documentElement(&pManifestElement);
    ExitOnFailure(hr, "Failed to get BurnManifest document element.");

    hr = pManifestElement->get_childNodes(&pNodes);
    ExitOnFailure(hr, "Failed to get child nodes of manifest document element.");

    while (S_OK == (hr = XmlNextElement(pNodes, &pNode, &bstrNodeName)))
    {
        if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, 0, bstrNodeName, -1, L"UX", -1))
        {
            hr = ParsePayloads(L"Resource", wzBundleId, wzBundleFileName, wzUXWorkingPath, pNode, dwPayloadsOffset, TRUE, pcUXPayloads, &rgpNewUXPayloads);
            ExitOnFailure(hr, "Failed to parse manifest UX.");
        }
        else if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, 0, bstrNodeName, -1, L"Chain", -1))
        {
            hr = ParseManifestChain(wzBundleId, wzBundleFileName, pNode, dwAttachedContainerOffset, pcChainedPayloads, &rgpChainedPayloads);
            ExitOnFailure(hr, "Failed to parse manifest chain.");
        }
        ReleaseNullBSTR(bstrNodeName);
        ReleaseNullObject(pNode);
    }

    if (S_FALSE == hr)
    {
        hr = S_OK;
    }

    // Transfer ownership of payloads memory
    *prgpUxPayloads = rgpNewUXPayloads;
    rgpNewUXPayloads = NULL;
    *prgpChainedPayloads = rgpChainedPayloads;
    rgpChainedPayloads = NULL;

LExit:
    ReleaseBSTR(bstrNodeName);
    ReleaseObject(pNode);
    ReleaseObject(pNodes);
    ReleaseObject(pManifestElement);
    ReleaseObjectArray(rgpNewUXPayloads, *pcUXPayloads);
    ReleaseObjectArray(rgpChainedPayloads, *pcChainedPayloads);

    return hr;
}

/********************************************************************
 ParseCommonPackageAttributes - parses attributes common to multiple
    types of packages
*********************************************************************/
static HRESULT ParseCommonPackageAttributes(
    __in BSTR bstrNodeName,
    __in BSTR bstrNodeValue,
    __out LPWSTR* psczId,
    __out DWORD* pdwAttributes,
    __out LPWSTR* psczInstallCondition,
    __out LPWSTR* psczRollbackInstallCondition
    )
{
    HRESULT hr = S_OK;

    if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, 0, bstrNodeName, -1, L"Id", -1))
    {
        hr = StrAllocString(psczId, bstrNodeValue, 0);
        ExitOnFailure(hr, "Failed to allocate @Id.");
    }
    else if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, 0, bstrNodeName, -1, L"InstallCondition", -1))
    {
        hr = StrAllocString(psczInstallCondition, bstrNodeValue, 0);
        ExitOnFailure(hr, "Failed to allocate @InstallCondition.");
    }
    else if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, 0, bstrNodeName, -1, L"RollbackInstallCondition", -1))
    {
        hr = StrAllocString(psczRollbackInstallCondition, bstrNodeValue, 0);
        ExitOnFailure(hr, "Failed to allocate @RollbackInstallCondition.");
    }
    else
    {
        // not a common attribute
        hr = S_FALSE;
    }

LExit:
    return hr;
}

/********************************************************************
 ParseManifestPackage - parses an Package node's attributes and
 child elements.into payloads and returns the attached payloads array
*********************************************************************/
extern "C" HRESULT ParseManifestPackage(
    __in_z LPCWSTR wzBundleId,
    __in_z LPCWSTR wzBundleFileName,
    __in IXMLDOMNode* pixnMsiPackage,
    __in DWORD dwPayloadsOffset,
    __out DWORD* pcPayloads,
    __out IBurnPayload*** prgpPayloads
    )
{
    Assert(pcPayloads);
    Assert(prgpPayloads);

    HRESULT hr = S_OK;
    IXMLDOMNamedNodeMap* pixnnmAttributes = NULL;
    IXMLDOMNode* pixnAttribute = NULL;
    IXMLDOMNodeList* pNodeList = NULL;
    IXMLDOMNode* pNode = NULL;
    BSTR bstrNodeName = NULL;
    BSTR bstrNodeValue = NULL;
    LPWSTR sczId = NULL;
    LPWSTR sczProductCode = NULL;
    IBurnPayload** rgpPayloads = NULL;

    ULARGE_INTEGER uliProductVersion = { };
    DWORD dwAttributes = 0;
    DWORD cPayloads = 1;
    LPWSTR sczInstallCondition = NULL;
    LPWSTR sczRollbackInstallCondition = NULL;

    // now process the rest of the package's attributes.
    hr = pixnMsiPackage->get_attributes(&pixnnmAttributes);
    ExitOnFailure(hr, "Failed to get attributes on MsiPackage element.");

    while (S_OK == (hr = pixnnmAttributes->nextNode(&pixnAttribute)))
    {
        // get name and value for each attribute
        hr = pixnAttribute->get_nodeName(&bstrNodeName);
        ExitOnFailure(hr, "Failed to get MsiPackage attribute name.");

        hr = XmlGetText(pixnAttribute, &bstrNodeValue);
        ExitOnFailure1(hr, "Failed to get MsiPackage attribute value: %S", bstrNodeName);

        // try common attributes; if that's a no-go, try MsiPackage-specific attributes
        hr = ParseCommonPackageAttributes(bstrNodeName, bstrNodeValue, &sczId, &dwAttributes, &sczInstallCondition, &sczRollbackInstallCondition);
        ExitOnFailure2(hr, "Failed to parse common package attribute: %S=%S", bstrNodeName, bstrNodeValue);

        if (S_FALSE == hr)
        {
            if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, 0, bstrNodeName, -1, L"ProductCode", -1))
            {
                hr = StrAllocString(&sczProductCode, bstrNodeValue, 0);
                ExitOnFailure1(hr, "Failed to convert invalid GUID '%S'", bstrNodeValue);
            }
            else if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, 0, bstrNodeName, -1, L"ProductVersion", -1))
            {
                hr = FileVersionFromString(bstrNodeValue, &uliProductVersion.HighPart, &uliProductVersion.LowPart);
                ExitOnFailure1(hr, "Failed to parse MsiPackage product version. '%S'", bstrNodeValue);
            }
        } 

        ReleaseNullBSTR(bstrNodeValue);
        ReleaseNullBSTR(bstrNodeName);
        ReleaseNullObject(pixnAttribute);
    }

    if (S_FALSE == hr)
    {
        hr = S_OK;
    }
    ExitOnFailure(hr, "Failed to enumerate all attributes on MsiPackage element.");

    // first get all the package's Resource children (e.g., external cabinets)
    hr = ParsePayloads(L"Resource", wzBundleId, wzBundleFileName, NULL, pixnMsiPackage, dwPayloadsOffset, FALSE, &cPayloads, &rgpPayloads);
    ExitOnFailure(hr, "Failed to parse MsiPackage/Resource payloads.");

    // Transfer ownership of payloads memory
    *pcPayloads = cPayloads;
    *prgpPayloads = rgpPayloads;
    rgpPayloads = NULL;

LExit:
    ReleaseStr(sczId);
    ReleaseStr(sczProductCode);
    ReleaseStr(sczInstallCondition);
    ReleaseStr(sczRollbackInstallCondition);
    ReleaseBSTR(bstrNodeValue);
    ReleaseBSTR(bstrNodeName);
    ReleaseObject(pNode);
    ReleaseObject(pNodeList);
    ReleaseObject(pixnAttribute);
    ReleaseObject(pixnnmAttributes);
    ReleaseObjectArray(rgpPayloads, cPayloads);

    return hr;
}
/********************************************************************
 ParseManifestChain - parses out a chain of packages from a loaded 
    manifest XML DOM element.
*********************************************************************/
HRESULT ParseManifestChain(
    __in_z LPCWSTR wzBundleId,
    __in_z LPCWSTR wzBundleFileName,
    __in IXMLDOMNode* pixnChain,
    __in DWORD dwPayloadsOffset,
    __out DWORD* pcChainedPayloads,
    __out IBurnPayload*** prgpUxPayloads
    )
{
    Assert(pixnChain);
    Assert(pcChainedPayloads);
    Assert(prgpUxPayloads);

    HRESULT hr = S_OK;
    IXMLDOMNodeList* pNodeList = NULL;
    IXMLDOMNode* pNode = NULL;
    BSTR bstrNodeName = NULL;
    long cChainedPayloads = 0;
    DWORD dwCurrentPackage = 0;
    DWORD cNewPayloads = 0;
    IBurnPayload** rgpReturnPayloads = NULL;
    IBurnPayload** rgpNewPayload = NULL;

    //
    // First, calculate how many items in the chain and allocate
    // the SETUP_PACKAGES structure.
    //
    hr = pixnChain->get_childNodes(&pNodeList);
    ExitOnFailure(hr, "Failed to select all packages in the manifest Chain.");

    hr = pNodeList->get_length(&cChainedPayloads);
    ExitOnFailure(hr, "Failed to count the number of packages in the manifest Chain.");

    rgpReturnPayloads = static_cast<IBurnPayload**>(MemAlloc(sizeof(IBurnPayload*) * cChainedPayloads, TRUE));
    ExitOnNull(rgpReturnPayloads, hr, E_OUTOFMEMORY, "Failed to allocate package list.");

    //
    // Process the packages in the chain.getting the payload for each
    //
    while (S_OK == (hr = XmlNextElement(pNodeList, &pNode, &bstrNodeName)))
    {
        hr = ParseManifestPackage(wzBundleId, wzBundleFileName, pNode, dwPayloadsOffset, &cNewPayloads, &rgpNewPayload);
        ExitOnFailure(hr, "Failed to parse MsiPackage from manifest chain.");

        // Expand the array if necessary due to child resources being discovered
        if ( (dwCurrentPackage + cNewPayloads) > (DWORD)cChainedPayloads)
        {
            cChainedPayloads = dwCurrentPackage + cNewPayloads;
            LPVOID pv = MemReAlloc(rgpReturnPayloads,sizeof(IBurnPayload*) * cChainedPayloads, FALSE);
            ExitOnNull(pv, hr, E_OUTOFMEMORY, "Failed to reallocate payload array for discovered child resources.");
            rgpReturnPayloads = static_cast<IBurnPayload**>(pv);
        }

        // Copy the new payload objects into the array
        for (DWORD dwIndex = 0; dwIndex < cNewPayloads; ++dwIndex)
        {
            rgpReturnPayloads[dwCurrentPackage] = rgpNewPayload[dwIndex];
            ++dwCurrentPackage;
        }

        ReleaseNullMem(rgpNewPayload);
        cNewPayloads = 0;
        ReleaseNullBSTR(bstrNodeName);
        ReleaseNullObject(pNode);
    }

    // Transfer ownership of payloads memory
    *pcChainedPayloads = cChainedPayloads;
    *prgpUxPayloads = rgpReturnPayloads;
    rgpReturnPayloads = NULL;


LExit:
    ReleaseMem(rgpNewPayload);
    ReleaseBSTR(bstrNodeName);
    ReleaseObject(pNode);
    ReleaseObject(pNodeList);
    ReleaseObjectArray(rgpReturnPayloads, *pcChainedPayloads);
    return hr;
}
