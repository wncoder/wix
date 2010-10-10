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

// prototypes


DAPI_(HRESULT) BalManifestLoad(
    __in HMODULE hBootstrapperApplicationModule,
    __out IXMLDOMDocument** ppixdManifest
    )
{
    HRESULT hr = S_OK;
    LPWSTR sczPath = NULL;

    hr = PathRelativeToModule(&sczPath, BAL_MANIFEST_FILENAME, hBootstrapperApplicationModule);
    ExitOnFailure(hr, "Failed to get path to BootstrapperApplicationData.xml.");

    hr = XmlLoadDocumentFromFile(sczPath, ppixdManifest);
    ExitOnFailure1(hr, "Failed to load BootstrapperApplicationData.xml: %ls", sczPath);

LExit:
    ReleaseStr(sczPath);
    return hr;
}
