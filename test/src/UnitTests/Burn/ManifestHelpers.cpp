//-------------------------------------------------------------------------------------------------
// <copyright file="ManifestHelpers.cpp" company="Microsoft">
//    Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//
// <summary>
//    Manifest helper functions for unit tests for Burn.
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"


using namespace System;
using namespace Microsoft::VisualStudio::TestTools::UnitTesting;


namespace Microsoft
{
namespace Tools
{
namespace WindowsInstallerXml
{
namespace Test
{
namespace Bootstrapper
{
    void LoadBundleXmlHelper(LPCWSTR wzDocument, IXMLDOMElement** ppixeBundle)
    {
        HRESULT hr = S_OK;
        IXMLDOMDocument* pixdDocument = NULL;
        try
        {
            hr = XmlLoadDocument(wzDocument, &pixdDocument);
            TestThrowOnFailure(hr, L"Failed to load XML document.");

            hr = pixdDocument->get_documentElement(ppixeBundle);
            TestThrowOnFailure(hr, L"Failed to get bundle element.");
        }
        finally
        {
            ReleaseObject(pixdDocument);
        }
    }
}
}
}
}
}
