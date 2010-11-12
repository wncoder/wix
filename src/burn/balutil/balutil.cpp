//-------------------------------------------------------------------------------------------------
// <copyright file="balutil.cpp" company="Microsoft">
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
// Bootstrapper Application Layer utility library.
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"

LPCWSTR BAL_MANIFEST_FILENAME = L"BootstrapperApplicationData.xml";
const DWORD VARIABLE_GROW_FACTOR = 80;

// prototypes


DAPI_(HRESULT) BalManifestLoad(
    __in HMODULE hBootstrapperApplicationModule,
    __out IXMLDOMDocument** ppixdManifest
    )
{
    HRESULT hr = S_OK;
    LPWSTR sczPath = NULL;

    hr = PathRelativeToModule(&sczPath, BAL_MANIFEST_FILENAME, hBootstrapperApplicationModule);
    ExitOnFailure1(hr, "Failed to get path to bootstrapper application manifest: %ls", BAL_MANIFEST_FILENAME);

    hr = XmlLoadDocumentFromFile(sczPath, ppixdManifest);
    ExitOnFailure2(hr, "Failed to load bootstrapper application manifest '%ls' from path: %ls", BAL_MANIFEST_FILENAME, sczPath);

LExit:
    ReleaseStr(sczPath);
    return hr;
}


DAPI_(BOOL) BalStringVariableExists(
    __in IBootstrapperEngine* pEngine,
    __in_z LPCWSTR wzVariable
    )
{
    HRESULT hr = S_OK;
    DWORD cch = 0;

    hr = pEngine->GetVariableString(wzVariable, NULL, 0);
    return E_MOREDATA == hr; // string exists only if there are more than zero characters in the variable.
}


DAPI_(HRESULT) BalGetStringVariable(
    __in IBootstrapperEngine* pEngine,
    __in_z LPCWSTR wzVariable,
    __inout LPWSTR* psczValue
    )
{
    HRESULT hr = S_OK;
    DWORD cch = 0;

    if (*psczValue)
    {
        hr = StrMaxLength(psczValue, reinterpret_cast<DWORD_PTR*>(&cch));
        ExitOnFailure(hr, "Failed to determine length of value.");
    }

    hr = pEngine->GetVariableString(wzVariable, *psczValue, &cch);
    if (E_MOREDATA == hr)
    {
        hr = StrAlloc(psczValue, cch + 1);
        ExitOnFailure(hr, "Failed to allocate value.");

        hr = pEngine->GetVariableString(wzVariable, *psczValue, &cch);
    }

LExit:
    return hr;
}


DAPIV_(HRESULT) BalLog(
    __in IBootstrapperEngine* pEngine,
    __in BOOTSTRAPPER_LOG_LEVEL level,
    __in_z __format_string LPCSTR szFormat,
    ...
    )
{
    HRESULT hr = S_OK;
    va_list args;
    LPSTR sczFormattedAnsi = NULL;
    LPWSTR sczMessage = NULL;

    va_start(args, szFormat);
    hr = StrAnsiAllocFormattedArgs(&sczFormattedAnsi, szFormat, args);
    va_end(args);
    ExitOnFailure(hr, "Failed to format log string.");

    hr = StrAllocStringAnsi(&sczMessage, sczFormattedAnsi, 0, CP_UTF8);
    ExitOnFailure(hr, "Failed to convert log string to Unicode.");

    hr = pEngine->Log(level, sczMessage);

LExit:
    ReleaseStr(sczMessage);
    ReleaseStr(sczFormattedAnsi);
    return hr;
}
