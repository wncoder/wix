//-------------------------------------------------------------------------------------------------
// <copyright file="buxutil.cpp" company="Microsoft">
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
// Burn UX utility library.
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"

LPCWSTR UX_MANIFEST_FILENAME = L"UxManifest.xml";

// prototypes


DAPI_(HRESULT) BuxManifestLoad(
    __in HMODULE hUXModule,
    __out IXMLDOMDocument** ppixdManifest
    )
{
    HRESULT hr = S_OK;
    LPWSTR sczUxPath = NULL;

    hr = PathRelativeToModule(&sczUxPath, UX_MANIFEST_FILENAME, hUXModule);
    ExitOnFailure(hr, "Failed to get path to ux.manifest.");

    hr = XmlLoadDocumentFromFile(sczUxPath, ppixdManifest);
    ExitOnFailure1(hr, "Failed to load ux.manifest: %ls", sczUxPath);

LExit:
    ReleaseStr(sczUxPath);
    return hr;
}
