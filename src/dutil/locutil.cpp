//-------------------------------------------------------------------------------------------------
// <copyright file="locutil.cpp" company="Microsoft">
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
//  localization functions
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"

// prototypes
static HRESULT ParseWxl(
    __in IXMLDOMDocument* pixd,
    __out LOC_STRINGSET** ppLocStringSet
    );
static HRESULT ParseWxlStrings(
    __in IXMLDOMElement* pElement,
    __in LOC_STRINGSET* pLocStringSet
    );
static HRESULT ParseWxlString(
    __in IXMLDOMNode* pixn,
    __in DWORD dwIdx,
    __in LOC_STRINGSET* pLocStringSet
    );


/********************************************************************
 LocLoadFromFile - Loads a localization file

*******************************************************************/
extern "C" HRESULT DAPI LocLoadFromFile(
    __in_z LPCWSTR wzWxlFile,
    __out LOC_STRINGSET** ppLocStringSet
    )
{
    HRESULT hr = S_OK;
    IXMLDOMDocument* pixd = NULL;

    hr = XmlLoadDocumentFromFile(wzWxlFile, &pixd);
    ExitOnFailure(hr, "Failed to load WXL file as XML document.");

    hr = ParseWxl(pixd, ppLocStringSet);
    ExitOnFailure(hr, "Failed to parse WXL.");

LExit:
    ReleaseObject(pixd);

    return hr;
}

/********************************************************************
 LocLoadFromFile - Loads a localization file from a module's data resource.

 NOTE: The resource data must be UTF-8 encoded.
*******************************************************************/
HRESULT DAPI LocLoadFromResource(
    __in HMODULE hModule,
    __in_z LPCSTR szResource,
    __out LOC_STRINGSET** ppLocStringSet
    )
{
    HRESULT hr = S_OK;
    LPVOID pvResource = NULL;
    DWORD cbResource = 0;
    LPWSTR sczXml = NULL;
    IXMLDOMDocument* pixd = NULL;

    hr = ResReadData(hModule, szResource, &pvResource, &cbResource);
    ExitOnFailure(hr, "Failed to read theme from resource.");

    hr = StrAllocStringAnsi(&sczXml, reinterpret_cast<LPCSTR>(pvResource), 0, CP_UTF8);
    ExitOnFailure(hr, "Failed to convert XML document data from UTF-8 to unicode string.");

    hr = XmlLoadDocument(sczXml, &pixd);
    ExitOnFailure(hr, "Failed to load theme resource as XML document.");

    hr = ParseWxl(pixd, ppLocStringSet);
    ExitOnFailure(hr, "Failed to parse WXL.");

LExit:
    ReleaseObject(pixd);
    ReleaseStr(sczXml);

    return hr;
}

/********************************************************************
 LocFree - free memory allocated when loading a localization file

*******************************************************************/
extern "C" void DAPI LocFree(
    __in_opt LOC_STRINGSET* pLocStringSet
    )
{
    if (pLocStringSet)
    {
        for (DWORD idx = 0; idx < pLocStringSet->cLocStrings; ++idx)
        {
            ReleaseStr(pLocStringSet->rgLocStrings[idx].wzID);
            ReleaseStr(pLocStringSet->rgLocStrings[idx].wzText);
        }

        ReleaseMem(pLocStringSet->rgLocStrings);
        ReleaseMem(pLocStringSet);
    }
}

/********************************************************************
 LocLocalizeString - replace any !(loc.id) in a string with the
                    correct sub string
*******************************************************************/
extern "C" HRESULT DAPI LocLocalizeString(
    __in const LOC_STRINGSET* pLocStringSet,
    __inout LPWSTR* ppwzInput
    )
{
    Assert(ppwzInput && pLocStringSet);
    HRESULT hr = S_OK;

    for (DWORD i = 0; i < pLocStringSet->cLocStrings; ++i)
    {
        hr = StrReplaceStringAll(ppwzInput, pLocStringSet->rgLocStrings[i].wzID, pLocStringSet->rgLocStrings[i].wzText);
        ExitOnFailure(hr, "Localizing string failed.");
    }

LExit:
    return hr;
}

// helper functions

static HRESULT ParseWxl(
    __in IXMLDOMDocument* pixd,
    __out LOC_STRINGSET** ppLocStringSet
    )
{
    HRESULT hr = S_OK;
    IXMLDOMElement *pWxlElement = NULL;
    LOC_STRINGSET* pLocStringSet = NULL;

    pLocStringSet = static_cast<LOC_STRINGSET*>(MemAlloc(sizeof(LOC_STRINGSET), TRUE));
    ExitOnNull(pLocStringSet, hr, E_OUTOFMEMORY, "Failed to allocate memory for Wxl file.");

    // read the WixLocalization tag
    hr = pixd->get_documentElement(&pWxlElement);
    ExitOnFailure(hr, "Failed to get localization element.");

    // store the strings in a node list
    hr = ParseWxlStrings(pWxlElement, pLocStringSet);
    ExitOnFailure(hr, "Parsing localization strings failed.");

    *ppLocStringSet = pLocStringSet;
    pLocStringSet = NULL;

LExit:
    ReleaseObject(pWxlElement);
    ReleaseMem(pLocStringSet);

    return hr;
}


static HRESULT ParseWxlStrings(
    __in IXMLDOMElement* pElement,
    __in LOC_STRINGSET* pLocStringSet
    )
{
    HRESULT hr = S_OK;
    IXMLDOMNode* pixn = NULL;
    IXMLDOMNodeList* pixnl = NULL;
    DWORD dwIdx = 0;

    hr = XmlSelectNodes(pElement, L"*", &pixnl);
    ExitOnLastError(hr, "Failed to get child nodes of Wxl File.");

    hr = pixnl->get_length(reinterpret_cast<long*>(&pLocStringSet->cLocStrings));
    ExitOnLastError(hr, "Failed to get number of child nodes in Wxl File.");

    pLocStringSet->rgLocStrings = static_cast<LOC_STRING*>(MemAlloc(sizeof(LOC_STRING) * pLocStringSet->cLocStrings, TRUE));
    ExitOnNull(pLocStringSet->rgLocStrings, hr, E_OUTOFMEMORY, "Failed to allocate memory for localization strings.");

    while (S_OK == (hr = XmlNextElement(pixnl, &pixn, NULL)))
    {
        hr = ParseWxlString(pixn, dwIdx, pLocStringSet);
        ExitOnFailure(hr, "Failed to parse localization string.");

        ++dwIdx;
        ReleaseNullObject(pixn);
    }

    hr = S_OK;
    ExitOnFailure(hr, "Failed to enumerate all localization strings.");

LExit:
    if (FAILED(hr) && pLocStringSet->rgLocStrings)
    {
        for (DWORD idx = 0; idx < pLocStringSet->cLocStrings; ++idx)
        {
            ReleaseStr(pLocStringSet->rgLocStrings[idx].wzID);
            ReleaseStr(pLocStringSet->rgLocStrings[idx].wzText);
        }

        ReleaseMem(pLocStringSet->rgLocStrings);
    }

    ReleaseObject(pixn);
    ReleaseObject(pixnl);

    return hr;
}

static HRESULT ParseWxlString(
    __in IXMLDOMNode* pixn,
    __in DWORD dwIdx,
    __in LOC_STRINGSET* pLocStringSet
    )
{
    HRESULT hr = S_OK;
    LOC_STRING* pLocString = NULL;
    BSTR bstrText = NULL;

    pLocString = pLocStringSet->rgLocStrings + dwIdx;

    // Id
    hr = XmlGetAttribute(pixn, L"Id", &bstrText);
    ExitOnFailure(hr, "Failed to get Xml attribute Id in Wxl file.");

    hr = StrAllocFormatted(&pLocString->wzID, L"!(loc.%s)", bstrText);
    ExitOnFailure(hr, "Failed to duplicate Xml attribute Id in Wxl file.");

    ReleaseNullBSTR(bstrText);

    // Overrideable
    hr = XmlGetAttribute(pixn, L"Overridable", &bstrText);
    ExitOnFailure(hr, "Failed to get Xml attribute Overridable.");

    if (S_OK == hr)
    {
        pLocString->bOverridable = CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, 0, bstrText, -1, L"yes", -1);
    }

    ReleaseNullBSTR(bstrText);

    // Text
    hr = XmlGetText(pixn, &bstrText);
    ExitOnFailure(hr, "Failed to get Xml text in Wxl file.");

    hr = StrAllocString(&pLocString->wzText, bstrText, 0);
    ExitOnFailure(hr, "Failed to duplicate Xml text in Wxl file.");

LExit:
    ReleaseBSTR(bstrText);

    return hr;
}
