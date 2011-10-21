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
//    Localization helper functions.
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"

// Vista-and-later functions
static PFN_GETUSERDEFAULTLOCALENAME vpfnGetUserDefaultLocaleName = NULL;
static PFN_GETUSERDEFAULTLOCALENAME vpfnGetUserDefaultLocaleNameFromLibrary = NULL;
static PFN_GETSYSTEMDEFAULTLOCALENAME vpfnGetSystemDefaultLocaleName = NULL;
static PFN_GETSYSTEMDEFAULTLOCALENAME vpfnGetSystemDefaultLocaleNameFromLibrary = NULL;
static PFN_LOCALENAMETOLCID vpfnLocaleNameToLCID = NULL;
static PFN_LOCALENAMETOLCID vpfnLocaleNameToLCIDFromLibrary = NULL;
static HMODULE vhmodKernel32 = NULL;


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


extern "C" HRESULT DAPI LocInitialize(
    )
{
    HRESULT hr = S_OK;

    hr = LoadSystemLibrary(L"Kernel32.dll", &vhmodKernel32);
    ExitOnFailure(hr, "Failed to load Kernel32.dll");

    // Ignore failures
    vpfnGetUserDefaultLocaleNameFromLibrary = reinterpret_cast<PFN_GETUSERDEFAULTLOCALENAME>(::GetProcAddress(vhmodKernel32, "GetUserDefaultLocaleName"));
    if (NULL == vpfnGetUserDefaultLocaleName)
    {
        vpfnGetUserDefaultLocaleName = vpfnGetUserDefaultLocaleNameFromLibrary;
    }

    vpfnGetSystemDefaultLocaleNameFromLibrary = reinterpret_cast<PFN_GETSYSTEMDEFAULTLOCALENAME>(::GetProcAddress(vhmodKernel32, "GetSystemDefaultLocaleName"));
    if (NULL == vpfnGetSystemDefaultLocaleName)
    {
        vpfnGetSystemDefaultLocaleName = vpfnGetSystemDefaultLocaleNameFromLibrary;
    }

    vpfnLocaleNameToLCIDFromLibrary = reinterpret_cast<PFN_LOCALENAMETOLCID>(::GetProcAddress(vhmodKernel32, "LocaleNameToLCID"));
    if (NULL == vpfnLocaleNameToLCID)
    {
        vpfnLocaleNameToLCID = vpfnLocaleNameToLCIDFromLibrary;
    }

LExit:
    return hr;
}

extern "C" void DAPI LocUninitialize(
    )
{
    if (vhmodKernel32)
    {
        ::FreeLibrary(vhmodKernel32);
        vhmodKernel32 = NULL;
        vpfnGetUserDefaultLocaleNameFromLibrary = NULL;
        vpfnGetSystemDefaultLocaleNameFromLibrary = NULL;
        vpfnLocaleNameToLCIDFromLibrary = NULL;
    }
}

extern "C" HRESULT DAPI LocProbeForFile(
    __in_z LPCWSTR wzBasePath,
    __in_z LPCWSTR wzLocFileName,
    __in_z_opt LPCWSTR wzLanguage,
    __inout LPWSTR* psczPath
    )
{
    HRESULT hr = S_OK;
    LPWSTR sczProbePath = NULL;
    LCID lcid = 0;
    LPWSTR sczLcidFile = NULL;

    // If a language was specified, look for a loc file in that as a directory.
    if (wzLanguage && *wzLanguage)
    {
        hr = PathConcat(wzBasePath, wzLanguage, &sczProbePath);
        ExitOnFailure(hr, "Failed to concat base path to language.");

        hr = PathConcat(sczProbePath, wzLocFileName, &sczProbePath);
        ExitOnFailure(hr, "Failed to concat loc file name to probe path.");

        if (FileExistsEx(sczProbePath, NULL))
        {
            ExitFunction();
        }
    }

    // Look for a loc file in the current user locale/LCID.
    if (vpfnGetUserDefaultLocaleName && vpfnLocaleNameToLCID)
    {
        // prefer the Vista-and-later locale functions
        WCHAR wzLocaleName[LOCALE_NAME_MAX_LENGTH] = { };
        if (0 == vpfnGetUserDefaultLocaleName(wzLocaleName, countof(wzLocaleName)))
        {
            ExitWithLastError(hr, "Failed to get user locale name.");
        }

        lcid = vpfnLocaleNameToLCID(wzLocaleName, 0);
    }

    if (0 == lcid)
    {
        lcid = ::GetUserDefaultLCID();
    }

    hr = StrAllocFormatted(&sczLcidFile, L"%u\\%ls", lcid, wzLocFileName);
    ExitOnFailure(hr, "Failed to format user lcid.");

    hr = PathConcat(wzBasePath, sczLcidFile, &sczProbePath);
    ExitOnFailure(hr, "Failed to concat user lcid file name to base path.");

    if (FileExistsEx(sczProbePath, NULL))
    {
        ExitFunction();
    }

    // Look for a loc file in the current system locale/LCID.
    lcid = 0;
    if (vpfnGetSystemDefaultLocaleName && vpfnLocaleNameToLCID)
    {
        // prefer the Vista-and-later locale functions
        WCHAR wzLocaleName[LOCALE_NAME_MAX_LENGTH] = { };
        if (0 == vpfnGetSystemDefaultLocaleName(wzLocaleName, countof(wzLocaleName)))
        {
            ExitWithLastError(hr, "Failed to get system locale name.");
        }

        lcid = vpfnLocaleNameToLCID(wzLocaleName, 0);
    }

    if (0 == lcid)
    {
        lcid = ::GetSystemDefaultLCID();
    }

    hr = StrAllocFormatted(&sczLcidFile, L"%u\\%ls", lcid, wzLocFileName);
    ExitOnFailure(hr, "Failed to format system lcid.");

    hr = PathConcat(wzBasePath, sczLcidFile, &sczProbePath);
    ExitOnFailure(hr, "Failed to concat system lcid file name to base path.");

    if (FileExistsEx(sczProbePath, NULL))
    {
        ExitFunction();
    }

    // Finally, look for the loc file in the base path.
    hr = PathConcat(wzBasePath, wzLocFileName, &sczProbePath);
    ExitOnFailure(hr, "Failed to concat loc file name to base path.");

    if (!FileExistsEx(sczProbePath, NULL))
    {
        hr = E_FILENOTFOUND;
    }

LExit:
    if (SUCCEEDED(hr))
    {
        hr = StrAllocString(psczPath, sczProbePath, 0);
    }

    ReleaseStr(sczLcidFile);
    ReleaseStr(sczProbePath);

    return hr;
}

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

extern "C" HRESULT DAPI LocLoadFromResource(
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

extern "C" void DAPI LocFree(
    __in_opt LOC_STRINGSET* pLocStringSet
    )
{
    if (pLocStringSet)
    {
        for (DWORD idx = 0; idx < pLocStringSet->cLocStrings; ++idx)
        {
            ReleaseStr(pLocStringSet->rgLocStrings[idx].wzId);
            ReleaseStr(pLocStringSet->rgLocStrings[idx].wzText);
        }

        ReleaseMem(pLocStringSet->rgLocStrings);
        ReleaseMem(pLocStringSet);
    }
}

extern "C" HRESULT DAPI LocLocalizeString(
    __in const LOC_STRINGSET* pLocStringSet,
    __inout LPWSTR* ppsczInput
    )
{
    Assert(ppsczInput && pLocStringSet);
    HRESULT hr = S_OK;

    for (DWORD i = 0; i < pLocStringSet->cLocStrings; ++i)
    {
        hr = StrReplaceStringAll(ppsczInput, pLocStringSet->rgLocStrings[i].wzId, pLocStringSet->rgLocStrings[i].wzText);
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
            ReleaseStr(pLocStringSet->rgLocStrings[idx].wzId);
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

    hr = StrAllocFormatted(&pLocString->wzId, L"#(loc.%s)", bstrText);
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
