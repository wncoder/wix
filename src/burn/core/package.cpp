//-------------------------------------------------------------------------------------------------
// <copyright file="package.cpp" company="Microsoft">
//    Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//
// <summary>
//    Module: Core
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"


// function definitions

extern "C" HRESULT PackagesParseFromXml(
    __in BURN_PACKAGES* pPackages,
    __in IXMLDOMNode* pixnBundle
    )
{
    HRESULT hr = S_OK;
    IXMLDOMNodeList* pixnNodes = NULL;
    IXMLDOMNode* pixnNode = NULL;
    DWORD cNodes = 0;
    BSTR bstrNodeName = NULL;
    //LPWSTR scz = NULL;

    // select package nodes
    hr = XmlSelectNodes(pixnBundle, L"Chain/ExePackage|Chain/MsiPackage|Chain/MsuPackage", &pixnNodes);
    ExitOnFailure(hr, "Failed to select package nodes.");

    // get package node count
    hr = pixnNodes->get_length((long*)&cNodes);
    ExitOnFailure(hr, "Failed to get package node count.");

    if (!cNodes)
    {
        ExitFunction1(hr = S_OK);
    }

    // allocate memory for packages
    pPackages->rgPackages = (BURN_PACKAGE*)MemAlloc(sizeof(BURN_PACKAGE) * cNodes, TRUE);
    ExitOnNull(pPackages->rgPackages, hr, E_OUTOFMEMORY, "Failed to allocate memory for package structs.");

    pPackages->cPackages = cNodes;

    // parse package elements
    for (DWORD i = 0; i < cNodes; ++i)
    {
        BURN_PACKAGE* pPackage = &pPackages->rgPackages[i];

        hr = XmlNextElement(pixnNodes, &pixnNode, &bstrNodeName);
        ExitOnFailure(hr, "Failed to get next node.");

        // @Id
        hr = XmlGetAttributeEx(pixnNode, L"Id", &pPackage->sczId);
        ExitOnFailure(hr, "Failed to get Id attribute.");

        // @Uninstallable
        pPackage->fUninstallable = TRUE;
        hr = XmlGetYesNoAttribute(pixnNode, L"Uninstallable", &pPackage->fUninstallable);
        if (E_NOTFOUND != hr)
        {
            ExitOnFailure(hr, "Failed to get @Uninstallable.");
        }

        // @Vital
        pPackage->fVital = TRUE;
        hr = XmlGetYesNoAttribute(pixnNode, L"Vital", &pPackage->fVital);
        if (E_NOTFOUND != hr)
        {
            ExitOnFailure(hr, "Failed to get @Vital.");
        }

        // read type specific attributes
        if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, 0, bstrNodeName, -1, L"ExePackage", -1))
        {
            pPackage->Type = BURN_PACKAGE_TYPE_EXE;

            hr = ExeEngineParsePackageFromXml(pixnNode, pPackage); // TODO: Modularization
            ExitOnFailure(hr, "Failed to parse EXE package.");
        }
        else if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, 0, bstrNodeName, -1, L"MsiPackage", -1))
        {
            pPackage->Type = BURN_PACKAGE_TYPE_MSI;

            hr = MsiEngineParsePackageFromXml(pixnNode, pPackage); // TODO: Modularization
            ExitOnFailure(hr, "Failed to parse MSI package.");
        }
        else if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, 0, bstrNodeName, -1, L"MsuPackage", -1))
        {
            pPackage->Type = BURN_PACKAGE_TYPE_MSI;

            hr = MsuEngineParsePackageFromXml(pixnNode, pPackage); // TODO: Modularization
            ExitOnFailure(hr, "Failed to parse MSU package.");
        }
        else
        {
            // ignore other package types for now
        }

        // prepare next iteration
        ReleaseNullObject(pixnNode);
        ReleaseNullBSTR(bstrNodeName);
    }

    hr = S_OK;

LExit:
    ReleaseObject(pixnNodes);
    ReleaseObject(pixnNode);
    ReleaseBSTR(bstrNodeName);
    //ReleaseStr(scz);
    return hr;
}

extern "C" void PackagesUninitialize(
    __in BURN_PACKAGES* pPackages
    )
{
    if (pPackages->rgPackages)
    {
        for (DWORD i = 0; i < pPackages->cPackages; ++i)
        {
            BURN_PACKAGE* pPackage = &pPackages->rgPackages[i];

            ReleaseStr(pPackage->sczId);
            ReleaseStr(pPackage->sczInstallCondition);
            ReleaseStr(pPackage->sczRollbackInstallCondition);
            ReleaseStr(pPackage->sczExecutePath);

            switch (pPackage->Type)
            {
            case BURN_PACKAGE_TYPE_EXE:
                ExeEnginePackageUninitialize(pPackage); // TODO: Modularization
                break;
            case BURN_PACKAGE_TYPE_MSI:
                MsiEnginePackageUninitialize(pPackage); // TODO: Modularization
                break;
            case BURN_PACKAGE_TYPE_MSP:
                break;
            }
        }
        MemFree(pPackages->rgPackages);
    }
}
