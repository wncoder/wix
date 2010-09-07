//-------------------------------------------------------------------------------------------------
// <copyright file="package.cpp" company="Microsoft">
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


// internal function declarations

static HRESULT ParsePayloadRefsFromXml(
    __in BURN_PACKAGE* pPackage,
    __in BURN_PAYLOADS* pPayloads,
    __in IXMLDOMNode* pixnPackage
    );


// function definitions

extern "C" HRESULT PackagesParseFromXml(
    __in BURN_PACKAGES* pPackages,
    __in BURN_PAYLOADS* pPayloads,
    __in IXMLDOMNode* pixnBundle
    )
{
    HRESULT hr = S_OK;
    IXMLDOMNodeList* pixnNodes = NULL;
    IXMLDOMNode* pixnNode = NULL;
    DWORD cNodes = 0;
    BSTR bstrNodeName = NULL;

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
        ExitOnFailure(hr, "Failed to get @Id.");

        // @Cache
        hr = XmlGetYesNoAttribute(pixnNode, L"Cache", &pPackage->fCache);
        ExitOnFailure(hr, "Failed to get @Cache.");

        // @CacheId
        hr = XmlGetAttributeEx(pixnNode, L"CacheId", &pPackage->sczCacheId);
        ExitOnFailure(hr, "Failed to get @CacheId.");

        // @PerMachine
        hr = XmlGetYesNoAttribute(pixnNode, L"PerMachine", &pPackage->fPerMachine);
        ExitOnFailure(hr, "Failed to get @PerMachine.");

        // @Permanent
        hr = XmlGetYesNoAttribute(pixnNode, L"Permanent", &pPackage->fUninstallable);
        ExitOnFailure(hr, "Failed to get @Permanent.");
        pPackage->fUninstallable = !pPackage->fUninstallable; // TODO: change "Uninstallable" variable name to permanent, until then Uninstallable is the opposite of Permanent so fix the variable.

        // @Vital
        hr = XmlGetYesNoAttribute(pixnNode, L"Vital", &pPackage->fVital);
        ExitOnFailure(hr, "Failed to get @Vital.");

        // @LogPathVariable
        hr = XmlGetAttributeEx(pixnNode, L"LogPathVariable", &pPackage->sczLogPathVariable);
        if (E_NOTFOUND != hr)
        {
            ExitOnFailure(hr, "Failed to get @LogPathVariable.");
        }

        // @RollbackLogPathVariable
        hr = XmlGetAttributeEx(pixnNode, L"RollbackLogPathVariable", &pPackage->sczRollbackLogPathVariable);
        if (E_NOTFOUND != hr)
        {
            ExitOnFailure(hr, "Failed to get @RollbackLogPathVariable.");
        }

        // @InstallCondition
        hr = XmlGetAttributeEx(pixnNode, L"InstallCondition", &pPackage->sczInstallCondition);
        if (E_NOTFOUND != hr)
        {
            ExitOnFailure(hr, "Failed to get @InstallCondition.");
        }

        // read type specific attributes
        if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, 0, bstrNodeName, -1, L"ExePackage", -1))
        {
            pPackage->type = BURN_PACKAGE_TYPE_EXE;

            hr = ExeEngineParsePackageFromXml(pixnNode, pPackage); // TODO: Modularization
            ExitOnFailure(hr, "Failed to parse EXE package.");
        }
        else if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, 0, bstrNodeName, -1, L"MsiPackage", -1))
        {
            pPackage->type = BURN_PACKAGE_TYPE_MSI;

            hr = MsiEngineParsePackageFromXml(pixnNode, pPackage); // TODO: Modularization
            ExitOnFailure(hr, "Failed to parse MSI package.");
        }
        else if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, 0, bstrNodeName, -1, L"MsuPackage", -1))
        {
            pPackage->type = BURN_PACKAGE_TYPE_MSU;

            hr = MsuEngineParsePackageFromXml(pixnNode, pPackage); // TODO: Modularization
            ExitOnFailure(hr, "Failed to parse MSU package.");
        }
        else
        {
            // ignore other package types for now
        }

        // parse payload references
        hr = ParsePayloadRefsFromXml(pPackage, pPayloads, pixnNode);
        ExitOnFailure(hr, "Failed to parse payload references.");

        // prepare next iteration
        ReleaseNullObject(pixnNode);
        ReleaseNullBSTR(bstrNodeName);
    }

    hr = S_OK;

LExit:
    ReleaseObject(pixnNodes);
    ReleaseObject(pixnNode);
    ReleaseBSTR(bstrNodeName);

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
            ReleaseStr(pPackage->sczLogPathVariable);
            ReleaseStr(pPackage->sczRollbackLogPathVariable);
            ReleaseStr(pPackage->sczInstallCondition);
            ReleaseStr(pPackage->sczRollbackInstallCondition);
            ReleaseStr(pPackage->sczCacheId);

            ReleaseMem(pPackage->rgPayloads);

            switch (pPackage->type)
            {
            case BURN_PACKAGE_TYPE_EXE:
                ExeEnginePackageUninitialize(pPackage); // TODO: Modularization
                break;
            case BURN_PACKAGE_TYPE_MSI:
                MsiEnginePackageUninitialize(pPackage); // TODO: Modularization
                break;
            case BURN_PACKAGE_TYPE_MSP:
                break;
            case BURN_PACKAGE_TYPE_MSU:
                MsuEnginePackageUninitialize(pPackage); // TODO: Modularization
                break;
            }
        }
        MemFree(pPackages->rgPackages);
    }

    // clear struct
    memset(pPackages, 0, sizeof(BURN_PACKAGES));
}

extern "C" HRESULT PackageFindById(
    __in BURN_PACKAGES* pPackages,
    __in_z LPCWSTR wzId,
    __out BURN_PACKAGE** ppPackage
    )
{
    HRESULT hr = S_OK;
    BURN_PACKAGE* pPackage = NULL;

    for (DWORD i = 0; i < pPackages->cPackages; ++i)
    {
        pPackage = &pPackages->rgPackages[i];

        if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, 0, pPackage->sczId, -1, wzId, -1))
        {
            *ppPackage = pPackage;
            ExitFunction1(hr = S_OK);
        }
    }

    hr = E_NOTFOUND;

LExit:
    return hr;
}


// internal function declarations

static HRESULT ParsePayloadRefsFromXml(
    __in BURN_PACKAGE* pPackage,
    __in BURN_PAYLOADS* pPayloads,
    __in IXMLDOMNode* pixnPackage
    )
{
    HRESULT hr = S_OK;
    IXMLDOMNodeList* pixnNodes = NULL;
    IXMLDOMNode* pixnNode = NULL;
    DWORD cNodes = 0;
    LPWSTR sczId = NULL;

    // select package nodes
    hr = XmlSelectNodes(pixnPackage, L"PayloadRef", &pixnNodes);
    ExitOnFailure(hr, "Failed to select package nodes.");

    // get package node count
    hr = pixnNodes->get_length((long*)&cNodes);
    ExitOnFailure(hr, "Failed to get package node count.");

    if (!cNodes)
    {
        ExitFunction1(hr = S_OK);
    }

    // allocate memory for payload pointers
    pPackage->rgPayloads = (BURN_PACKAGE_PAYLOAD*)MemAlloc(sizeof(BURN_PACKAGE_PAYLOAD) * cNodes, TRUE);
    ExitOnNull(pPackage->rgPayloads, hr, E_OUTOFMEMORY, "Failed to allocate memory for package payloads.");

    pPackage->cPayloads = cNodes;

    // parse package elements
    for (DWORD i = 0; i < cNodes; ++i)
    {
        BURN_PACKAGE_PAYLOAD* pPackagePayload = &pPackage->rgPayloads[i];

        hr = XmlNextElement(pixnNodes, &pixnNode, NULL);
        ExitOnFailure(hr, "Failed to get next node.");

        // @Id
        hr = XmlGetAttributeEx(pixnNode, L"Id", &sczId);
        ExitOnFailure(hr, "Failed to get Id attribute.");

        // find payload
        hr = PayloadFindById(pPayloads, sczId, &pPackagePayload->pPayload);
        ExitOnFailure(hr, "Failed to find payload.");

        // prepare next iteration
        ReleaseNullObject(pixnNode);
    }

    hr = S_OK;

LExit:
    ReleaseObject(pixnNodes);
    ReleaseObject(pixnNode);
    ReleaseStr(sczId);

    return hr;
}
