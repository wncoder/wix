//-------------------------------------------------------------------------------------------------
// <copyright file="msiengine.cpp" company="Microsoft">
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
//    Module: MSI Engine
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"


// constants


// structs



// internal function declarations

static HRESULT ParseRelatedMsiFromXml(
    __in IXMLDOMNode* pixnRelatedMsi,
    __in BURN_RELATED_MSI* pRelatedMsi
    );
static HRESULT EvaluateActionStateConditions(
    __in BURN_VARIABLES* pVariables,
    __in_z_opt LPCWSTR sczAddLocalCondition,
    __in_z_opt LPCWSTR sczAddSourceCondition,
    __in_z_opt LPCWSTR sczAdvertiseCondition,
    __out BOOTSTRAPPER_FEATURE_STATE* pState
    );
static HRESULT CalculateFeatureAction(
    __in BOOTSTRAPPER_FEATURE_STATE currentState,
    __in BOOTSTRAPPER_FEATURE_STATE requestedState,
    __in BOOL fRepair,
    __out BOOTSTRAPPER_FEATURE_ACTION* pFeatureAction,
    __out BOOL* pfDelta
    );
static HRESULT EscapePropertyArgumentString(
    __in LPCWSTR wzProperty,
    __inout_z LPWSTR* psczEscapedValue
    );
static HRESULT ConcatFeatureActionProperties(
    __in BURN_PACKAGE* pPackage,
    __in BOOTSTRAPPER_FEATURE_ACTION* rgFeatureActions,
    __inout_z LPWSTR* psczArguments
    );
static HRESULT ConcatPatchProperty(
    __in BURN_PACKAGE* pPackage,
    __inout_z LPWSTR* psczArguments
    );


// function definitions

extern "C" HRESULT MsiEngineParsePackageFromXml(
    __in IXMLDOMNode* pixnMsiPackage,
    __in BURN_PACKAGE* pPackage
    )
{
    HRESULT hr = S_OK;
    IXMLDOMNodeList* pixnNodes = NULL;
    IXMLDOMNode* pixnNode = NULL;
    DWORD cNodes = 0;
    LPWSTR scz = NULL;

    // @ProductCode
    hr = XmlGetAttributeEx(pixnMsiPackage, L"ProductCode", &pPackage->Msi.sczProductCode);
    ExitOnFailure(hr, "Failed to get @ProductCode.");

    // @Version
    hr = XmlGetAttributeEx(pixnMsiPackage, L"Version", &scz);
    ExitOnFailure(hr, "Failed to get @Version.");

    hr = FileVersionFromStringEx(scz, 0, &pPackage->Msi.qwVersion);
    ExitOnFailure1(hr, "Failed to parse @Version: %ls", scz);

    // select feature nodes
    hr = XmlSelectNodes(pixnMsiPackage, L"MsiFeature", &pixnNodes);
    ExitOnFailure(hr, "Failed to select feature nodes.");

    // get feature node count
    hr = pixnNodes->get_length((long*)&cNodes);
    ExitOnFailure(hr, "Failed to get feature node count.");

    if (cNodes)
    {
        // allocate memory for features
        pPackage->Msi.rgFeatures = (BURN_MSIFEATURE*)MemAlloc(sizeof(BURN_MSIFEATURE) * cNodes, TRUE);
        ExitOnNull(pPackage->Msi.rgFeatures, hr, E_OUTOFMEMORY, "Failed to allocate memory for MSI feature structs.");

        pPackage->Msi.cFeatures = cNodes;

        // parse feature elements
        for (DWORD i = 0; i < cNodes; ++i)
        {
            BURN_MSIFEATURE* pFeature = &pPackage->Msi.rgFeatures[i];

            hr = XmlNextElement(pixnNodes, &pixnNode, NULL);
            ExitOnFailure(hr, "Failed to get next node.");

            // @Id
            hr = XmlGetAttributeEx(pixnNode, L"Id", &pFeature->sczId);
            ExitOnFailure(hr, "Failed to get @Id.");

            // @AddLocalCondition
            hr = XmlGetAttributeEx(pixnNode, L"AddLocalCondition", &pFeature->sczAddLocalCondition);
            if (E_NOTFOUND != hr)
            {
                ExitOnFailure(hr, "Failed to get @AddLocalCondition.");
            }

            // @AddSourceCondition
            hr = XmlGetAttributeEx(pixnNode, L"AddSourceCondition", &pFeature->sczAddSourceCondition);
            if (E_NOTFOUND != hr)
            {
                ExitOnFailure(hr, "Failed to get @AddSourceCondition.");
            }

            // @AdvertiseCondition
            hr = XmlGetAttributeEx(pixnNode, L"AdvertiseCondition", &pFeature->sczAdvertiseCondition);
            if (E_NOTFOUND != hr)
            {
                ExitOnFailure(hr, "Failed to get @AdvertiseCondition.");
            }

            // @RollbackAddLocalCondition
            hr = XmlGetAttributeEx(pixnNode, L"RollbackAddLocalCondition", &pFeature->sczRollbackAddLocalCondition);
            if (E_NOTFOUND != hr)
            {
                ExitOnFailure(hr, "Failed to get @RollbackAddLocalCondition.");
            }

            // @RollbackAddSourceCondition
            hr = XmlGetAttributeEx(pixnNode, L"RollbackAddSourceCondition", &pFeature->sczRollbackAddSourceCondition);
            if (E_NOTFOUND != hr)
            {
                ExitOnFailure(hr, "Failed to get @RollbackAddSourceCondition.");
            }

            // @RollbackAdvertiseCondition
            hr = XmlGetAttributeEx(pixnNode, L"RollbackAdvertiseCondition", &pFeature->sczRollbackAdvertiseCondition);
            if (E_NOTFOUND != hr)
            {
                ExitOnFailure(hr, "Failed to get @RollbackAdvertiseCondition.");
            }

            // prepare next iteration
            ReleaseNullObject(pixnNode);
        }
    }

    ReleaseNullObject(pixnNodes); // done with the MsiFeature elements.

    hr = MsiEngineParsePropertiesFromXml(pixnMsiPackage, &pPackage->Msi.rgProperties, &pPackage->Msi.cProperties);
    ExitOnFailure(hr, "Failed to parse properties from XML.");

    // select related MSI nodes
    hr = XmlSelectNodes(pixnMsiPackage, L"RelatedPackage", &pixnNodes);
    ExitOnFailure(hr, "Failed to select related MSI nodes.");

    // get related MSI node count
    hr = pixnNodes->get_length((long*)&cNodes);
    ExitOnFailure(hr, "Failed to get related MSI node count.");

    if (cNodes)
    {
        // allocate memory for related MSIs
        pPackage->Msi.rgRelatedMsis = (BURN_RELATED_MSI*)MemAlloc(sizeof(BURN_RELATED_MSI) * cNodes, TRUE);
        ExitOnNull(pPackage->Msi.rgRelatedMsis, hr, E_OUTOFMEMORY, "Failed to allocate memory for related MSI structs.");

        pPackage->Msi.cRelatedMsis = cNodes;

        // parse related MSI elements
        for (DWORD i = 0; i < cNodes; ++i)
        {
            hr = XmlNextElement(pixnNodes, &pixnNode, NULL);
            ExitOnFailure(hr, "Failed to get next node.");

            // parse related MSI element
            hr = ParseRelatedMsiFromXml(pixnNode, &pPackage->Msi.rgRelatedMsis[i]);
            ExitOnFailure(hr, "Failed to parse related MSI element.");

            // prepare next iteration
            ReleaseNullObject(pixnNode);
        }
    }

    ReleaseNullObject(pixnNodes); // done with the RelatedPackage elements.

    // Select slipstream MSP nodes.
    hr = XmlSelectNodes(pixnMsiPackage, L"SlipstreamMsp", &pixnNodes);
    ExitOnFailure(hr, "Failed to select related MSI nodes.");

    hr = pixnNodes->get_length((long*)&cNodes);
    ExitOnFailure(hr, "Failed to get related MSI node count.");

    if (cNodes)
    {
        pPackage->Msi.rgpSlipstreamMspPackages = reinterpret_cast<BURN_PACKAGE**>(MemAlloc(sizeof(BURN_PACKAGE*) * cNodes, TRUE));
        ExitOnNull(pPackage->Msi.rgpSlipstreamMspPackages, hr, E_OUTOFMEMORY, "Failed to allocate memory for slipstream MSP packages.");

        pPackage->Msi.rgsczSlipstreamMspPackageIds = reinterpret_cast<LPWSTR*>(MemAlloc(sizeof(LPWSTR*) * cNodes, TRUE));
        ExitOnNull(pPackage->Msi.rgsczSlipstreamMspPackageIds, hr, E_OUTOFMEMORY, "Failed to allocate memory for slipstream MSP ids.");

        pPackage->Msi.cSlipstreamMspPackages = cNodes;

        // Parse slipstream MSP Ids.
        for (DWORD i = 0; i < cNodes; ++i)
        {
            hr = XmlNextElement(pixnNodes, &pixnNode, NULL);
            ExitOnFailure(hr, "Failed to get next slipstream MSP node.");

            hr = XmlGetAttributeEx(pixnNode, L"Id", pPackage->Msi.rgsczSlipstreamMspPackageIds + i);
            ExitOnFailure(hr, "Failed to parse slipstream MSP ids.");

            ReleaseNullObject(pixnNode);
        }
    }

    hr = S_OK;

LExit:
    ReleaseObject(pixnNodes);
    ReleaseObject(pixnNode);
    ReleaseStr(scz);

    return hr;
}

extern "C" HRESULT MsiEngineParsePropertiesFromXml(
    __in IXMLDOMNode* pixnPackage,
    __out BURN_MSIPROPERTY** prgProperties,
    __out DWORD* pcProperties
    )
{
    HRESULT hr = S_OK;
    IXMLDOMNodeList* pixnNodes = NULL;
    IXMLDOMNode* pixnNode = NULL;
    DWORD cNodes = 0;

    BURN_MSIPROPERTY* pProperties = NULL;

    // select property nodes
    hr = XmlSelectNodes(pixnPackage, L"MsiProperty", &pixnNodes);
    ExitOnFailure(hr, "Failed to select property nodes.");

    // get property node count
    hr = pixnNodes->get_length((long*)&cNodes);
    ExitOnFailure(hr, "Failed to get property node count.");

    if (cNodes)
    {
        // allocate memory for properties
        pProperties = (BURN_MSIPROPERTY*)MemAlloc(sizeof(BURN_MSIPROPERTY) * cNodes, TRUE);
        ExitOnNull(pProperties, hr, E_OUTOFMEMORY, "Failed to allocate memory for MSI property structs.");

        // parse property elements
        for (DWORD i = 0; i < cNodes; ++i)
        {
            BURN_MSIPROPERTY* pProperty = &pProperties[i];

            hr = XmlNextElement(pixnNodes, &pixnNode, NULL);
            ExitOnFailure(hr, "Failed to get next node.");

            // @Id
            hr = XmlGetAttributeEx(pixnNode, L"Id", &pProperty->sczId);
            ExitOnFailure(hr, "Failed to get @Id.");

            // @Value
            hr = XmlGetAttributeEx(pixnNode, L"Value", &pProperty->sczValue);
            ExitOnFailure(hr, "Failed to get @Value.");

            // @RollbackValue
            hr = XmlGetAttributeEx(pixnNode, L"RollbackValue", &pProperty->sczRollbackValue);
            if (E_NOTFOUND != hr)
            {
                ExitOnFailure(hr, "Failed to get @RollbackValue.");
            }

            // prepare next iteration
            ReleaseNullObject(pixnNode);
        }
    }

    *pcProperties = cNodes;
    *prgProperties = pProperties;
    pProperties = NULL;

    hr = S_OK;

LExit:
    ReleaseNullObject(pixnNodes);
    ReleaseMem(pProperties);

    return hr;
}

extern "C" void MsiEnginePackageUninitialize(
    __in BURN_PACKAGE* pPackage
    )
{
    ReleaseStr(pPackage->Msi.sczProductCode);

    // free features
    if (pPackage->Msi.rgFeatures)
    {
        for (DWORD i = 0; i < pPackage->Msi.cFeatures; ++i)
        {
            BURN_MSIFEATURE* pFeature = &pPackage->Msi.rgFeatures[i];

            ReleaseStr(pFeature->sczId);
            ReleaseStr(pFeature->sczAddLocalCondition);
            ReleaseStr(pFeature->sczAddSourceCondition);
            ReleaseStr(pFeature->sczAdvertiseCondition);
            ReleaseStr(pFeature->sczRollbackAddLocalCondition);
            ReleaseStr(pFeature->sczRollbackAddSourceCondition);
            ReleaseStr(pFeature->sczRollbackAdvertiseCondition);
        }
        MemFree(pPackage->Msi.rgFeatures);
    }

    // free properties
    if (pPackage->Msi.rgProperties)
    {
        for (DWORD i = 0; i < pPackage->Msi.cProperties; ++i)
        {
            BURN_MSIPROPERTY* pProperty = &pPackage->Msi.rgProperties[i];

            ReleaseStr(pProperty->sczId);
            ReleaseStr(pProperty->sczValue);
            ReleaseStr(pProperty->sczRollbackValue);
        }
        MemFree(pPackage->Msi.rgProperties);
    }

    // free related MSIs
    if (pPackage->Msi.rgRelatedMsis)
    {
        for (DWORD i = 0; i < pPackage->Msi.cRelatedMsis; ++i)
        {
            BURN_RELATED_MSI* pRelatedMsi = &pPackage->Msi.rgRelatedMsis[i];

            ReleaseStr(pRelatedMsi->sczUpgradeCode);
            ReleaseMem(pRelatedMsi->rgdwLanguages);
        }
        MemFree(pPackage->Msi.rgRelatedMsis);
    }

    // free slipstream MSPs
    if (pPackage->Msi.rgsczSlipstreamMspPackageIds)
    {
        for (DWORD i = 0; i < pPackage->Msi.cSlipstreamMspPackages; ++i)
        {
            ReleaseStr(pPackage->Msi.rgsczSlipstreamMspPackageIds[i]);
        }

        MemFree(pPackage->Msi.rgsczSlipstreamMspPackageIds);
    }

    if (pPackage->Msi.rgpSlipstreamMspPackages)
    {
        MemFree(pPackage->Msi.rgpSlipstreamMspPackages);
    }

    // clear struct
    memset(&pPackage->Msi, 0, sizeof(pPackage->Msi));
}

extern "C" HRESULT MsiEngineDetectPackage(
    __in BURN_PACKAGE* pPackage,
    __in BURN_USER_EXPERIENCE* pUserExperience
    )
{
    Trace1(REPORT_STANDARD, "Detecting MSI package 0x%p", pPackage);

    HRESULT hr = S_OK;
    LPWSTR sczInstalledVersion = NULL;
    INSTALLSTATE installState = INSTALLSTATE_UNKNOWN;
    BOOTSTRAPPER_RELATED_OPERATION operation = BOOTSTRAPPER_RELATED_OPERATION_NONE;
    WCHAR wzProductCode[MAX_GUID_CHARS + 1] = { };
    DWORD64 qwVersion = 0;
    BOOL fPerMachine = FALSE;
    int nResult = 0;

    // detect self by product code
    // TODO: what to do about MSIINSTALLCONTEXT_USERMANAGED?
    hr = WiuGetProductInfoEx(pPackage->Msi.sczProductCode, NULL, pPackage->fPerMachine ? MSIINSTALLCONTEXT_MACHINE : MSIINSTALLCONTEXT_USERUNMANAGED, INSTALLPROPERTY_VERSIONSTRING, &sczInstalledVersion);
    if (SUCCEEDED(hr))
    {
        hr = FileVersionFromStringEx(sczInstalledVersion, 0, &pPackage->Msi.qwInstalledVersion);
        ExitOnFailure2(hr, "Failed to convert version: %ls to DWORD64 for ProductCode: %ls", sczInstalledVersion, pPackage->Msi.sczProductCode);

        // compare versions
        if (pPackage->Msi.qwVersion < pPackage->Msi.qwInstalledVersion)
        {
            operation = BOOTSTRAPPER_RELATED_OPERATION_DOWNGRADE;
            pPackage->currentState = BOOTSTRAPPER_PACKAGE_STATE_SUPERSEDED;
        }
        else
        {
            if (pPackage->Msi.qwVersion > pPackage->Msi.qwInstalledVersion)
            {
                operation = BOOTSTRAPPER_RELATED_OPERATION_MINOR_UPDATE;
            }
            pPackage->currentState = BOOTSTRAPPER_PACKAGE_STATE_PRESENT;
        }

        // report related MSI package to UX
        if (BOOTSTRAPPER_RELATED_OPERATION_NONE != operation)
        {
            LogId(REPORT_STANDARD, MSG_DETECTED_RELATED_PACKAGE, pPackage->Msi.sczProductCode, LoggingPerMachineToString(pPackage->fPerMachine), LoggingVersionToString(pPackage->Msi.qwInstalledVersion), LoggingRelatedOperationToString(operation));

            nResult = pUserExperience->pUserExperience->OnDetectRelatedMsiPackage(pPackage->sczId, pPackage->Msi.sczProductCode, pPackage->fPerMachine, pPackage->Msi.qwInstalledVersion, operation);
            hr = HRESULT_FROM_VIEW(nResult);
            ExitOnRootFailure(hr, "UX aborted detect related MSI package.");
        }
    }
    else if (HRESULT_FROM_WIN32(ERROR_UNKNOWN_PRODUCT) == hr || HRESULT_FROM_WIN32(ERROR_UNKNOWN_PROPERTY) == hr) // package not present.
    {
        pPackage->currentState = BOOTSTRAPPER_PACKAGE_STATE_ABSENT;
        hr = S_OK;
    }
    else
    {
        ExitOnFailure1(hr, "Failed to get product information for ProductCode: %ls", pPackage->Msi.sczProductCode);
    }

    // detect related packages by upgrade code
    for (DWORD i = 0; i < pPackage->Msi.cRelatedMsis; ++i)
    {
        BURN_RELATED_MSI* pRelatedMsi = &pPackage->Msi.rgRelatedMsis[i];

        for (DWORD iProduct = 0; ; ++iProduct)
        {
            // get product
            hr = WiuEnumRelatedProducts(pRelatedMsi->sczUpgradeCode, iProduct, wzProductCode);
            if (E_NOMOREITEMS == hr)
            {
                hr = S_OK;
                break;
            }
            ExitOnFailure(hr, "Failed to enum related products.");

            // get product version
            hr = WiuGetProductInfoEx(wzProductCode, NULL, MSIINSTALLCONTEXT_USERUNMANAGED, INSTALLPROPERTY_VERSIONSTRING, &sczInstalledVersion);
            if (HRESULT_FROM_WIN32(ERROR_UNKNOWN_PRODUCT) != hr)
            {
                ExitOnFailure1(hr, "Failed to get version for product in user unmanaged context: %ls", wzProductCode);
                fPerMachine = FALSE;
            }
            else
            {
                hr = WiuGetProductInfoEx(wzProductCode, NULL, MSIINSTALLCONTEXT_MACHINE, INSTALLPROPERTY_VERSIONSTRING, &sczInstalledVersion);
                if (HRESULT_FROM_WIN32(ERROR_UNKNOWN_PRODUCT) != hr)
                {
                    ExitOnFailure1(hr, "Failed to get version for product in machine context: %ls", wzProductCode);
                    fPerMachine = TRUE;
                }
                else
                {
                    hr = S_OK;
                    continue;
                }
            }

            hr = FileVersionFromStringEx(sczInstalledVersion, 0, &qwVersion);
            ExitOnFailure2(hr, "Failed to convert version: %ls to DWORD64 for ProductCode: %ls", sczInstalledVersion, wzProductCode);

            // compare versions
            if (pRelatedMsi->fMinProvided && (pRelatedMsi->fMinInclusive ? (qwVersion < pRelatedMsi->qwMinVersion) : (qwVersion <= pRelatedMsi->qwMinVersion)))
            {
                continue;
            }

            if (pRelatedMsi->fMaxProvided && (pRelatedMsi->fMaxInclusive ? (qwVersion > pRelatedMsi->qwMaxVersion) : (qwVersion >= pRelatedMsi->qwMaxVersion)))
            {
                continue;
            }

            // pass to UX
            operation = pRelatedMsi->fOnlyDetect ? BOOTSTRAPPER_RELATED_OPERATION_NONE : BOOTSTRAPPER_RELATED_OPERATION_MAJOR_UPGRADE;
            nResult = pUserExperience->pUserExperience->OnDetectRelatedMsiPackage(pPackage->sczId, wzProductCode, fPerMachine, qwVersion, operation);
            hr = HRESULT_FROM_VIEW(nResult);
            ExitOnRootFailure(hr, "UX aborted detect related MSI package.");
        }
    }

    // detect features
    LogId(REPORT_STANDARD, MSG_DETECT_MSI_FEATURES, pPackage->Msi.cFeatures, pPackage->sczId);
    
    for (DWORD i = 0; i < pPackage->Msi.cFeatures; ++i)
    {
        BURN_MSIFEATURE* pFeature = &pPackage->Msi.rgFeatures[i];

        // get current state
        if (BOOTSTRAPPER_PACKAGE_STATE_PRESENT == pPackage->currentState) // only try to detect features if the product is installed
        {
            hr = WiuQueryFeatureState(pPackage->Msi.sczProductCode, pFeature->sczId, &installState);
            ExitOnFailure(hr, "Failed to query feature state.");

            if (INSTALLSTATE_UNKNOWN == installState) // in case of an upgrade this could happen
            {
                installState = INSTALLSTATE_ABSENT;
            }
        }
        else
        {
            installState = INSTALLSTATE_ABSENT;
        }

        // set current state
        switch (installState)
        {
        case INSTALLSTATE_ABSENT:
            pFeature->currentState = BOOTSTRAPPER_FEATURE_STATE_ABSENT;
            break;
        case INSTALLSTATE_ADVERTISED:
            pFeature->currentState = BOOTSTRAPPER_FEATURE_STATE_ADVERTISED;
            break;
        case INSTALLSTATE_LOCAL:
            pFeature->currentState = BOOTSTRAPPER_FEATURE_STATE_LOCAL;
            break;
        case INSTALLSTATE_SOURCE:
            pFeature->currentState = BOOTSTRAPPER_FEATURE_STATE_SOURCE;
            break;
        default:
            hr = E_UNEXPECTED;
            ExitOnRootFailure(hr, "Invalid state value.");
        }

        LogId(REPORT_STANDARD, MSG_DETECTED_MSI_FEATURE, pFeature->sczId, LoggingMsiFeatureStateToString(pFeature->currentState));

        // pass to UX
        nResult = pUserExperience->pUserExperience->OnDetectMsiFeature(pPackage->sczId, pFeature->sczId, pFeature->currentState);
        hr = HRESULT_FROM_VIEW(nResult);
        ExitOnRootFailure(hr, "UX aborted detect.");
    }

LExit:
    ReleaseStr(sczInstalledVersion);

    return hr;
}

//
// Plan - calculates the execute and rollback state for the requested package state.
//
extern "C" HRESULT MsiEnginePlanPackage(
    __in DWORD dwPackageSequence,
    __in_opt DWORD *pdwInsertSequence,
    __in BURN_PACKAGE* pPackage,
    __in BURN_PLAN* pPlan,
    __in BURN_LOGGING* pLog,
    __in BURN_VARIABLES* pVariables,
    __in_opt HANDLE hCacheEvent,
    __in BOOL fPlanPackageCacheRollback,
    __in BURN_USER_EXPERIENCE* pUserExperience,
    __out BOOTSTRAPPER_ACTION_STATE* pExecuteAction,
    __out BOOTSTRAPPER_ACTION_STATE* pRollbackAction
    )
{
    Trace1(REPORT_STANDARD, "Planning MSI package 0x%p", pPackage);

    HRESULT hr = S_OK;
    DWORD64 qwVersion = pPackage->Msi.qwVersion;
    DWORD64 qwInstalledVersion = pPackage->Msi.qwInstalledVersion;
    BOOTSTRAPPER_ACTION_STATE execute = BOOTSTRAPPER_ACTION_STATE_NONE;
    BOOTSTRAPPER_ACTION_STATE rollback = BOOTSTRAPPER_ACTION_STATE_NONE;
    BOOTSTRAPPER_FEATURE_STATE featureRequestedState = BOOTSTRAPPER_FEATURE_STATE_UNKNOWN;
    BOOTSTRAPPER_FEATURE_STATE featureExpectedState = BOOTSTRAPPER_FEATURE_STATE_UNKNOWN;
    BURN_EXECUTE_ACTION* pAction = NULL;
    BOOTSTRAPPER_FEATURE_ACTION* rgFeatureActions = NULL;
    BOOTSTRAPPER_FEATURE_ACTION* rgRollbackFeatureActions = NULL;
    BOOL fFeatureActionDelta = FALSE;
    BOOL fRollbackFeatureActionDelta = FALSE;
    int nResult = 0;

    if (pPackage->Msi.cFeatures)
    {
        LogId(REPORT_STANDARD, MSG_PLAN_MSI_FEATURES, pPackage->Msi.cFeatures, pPackage->sczId);

        // allocate array for feature actions
        rgFeatureActions = (BOOTSTRAPPER_FEATURE_ACTION*)MemAlloc(sizeof(BOOTSTRAPPER_FEATURE_ACTION) * pPackage->Msi.cFeatures, TRUE);
        ExitOnNull(rgFeatureActions, hr, E_OUTOFMEMORY, "Failed to allocate memory for feature actions.");

        rgRollbackFeatureActions = (BOOTSTRAPPER_FEATURE_ACTION*)MemAlloc(sizeof(BOOTSTRAPPER_FEATURE_ACTION) * pPackage->Msi.cFeatures, TRUE);
        ExitOnNull(rgRollbackFeatureActions, hr, E_OUTOFMEMORY, "Failed to allocate memory for rollback feature actions.");

        // plan features
        for (DWORD i = 0; i < pPackage->Msi.cFeatures; ++i)
        {
            BURN_MSIFEATURE* pFeature = &pPackage->Msi.rgFeatures[i];

            // evaluate feature conditions
            hr = EvaluateActionStateConditions(pVariables, pFeature->sczAddLocalCondition, pFeature->sczAddSourceCondition, pFeature->sczAdvertiseCondition, &featureRequestedState);
            ExitOnFailure(hr, "Failed to evaluate requested state conditions.");

            hr = EvaluateActionStateConditions(pVariables, pFeature->sczRollbackAddLocalCondition, pFeature->sczRollbackAddSourceCondition, pFeature->sczRollbackAdvertiseCondition, &featureExpectedState);
            ExitOnFailure(hr, "Failed to evaluate expected state conditions.");

            // send MSI feature plan message to UX
            nResult = pUserExperience->pUserExperience->OnPlanMsiFeature(pPackage->sczId, pFeature->sczId, &featureRequestedState);
            hr = HRESULT_FROM_VIEW(nResult);
            ExitOnRootFailure(hr, "UX aborted plan MSI feature.");

            // calculate feature actions
            hr = CalculateFeatureAction(pFeature->currentState, featureRequestedState, pFeature->fRepair, &rgFeatureActions[i], &fFeatureActionDelta);
            ExitOnFailure(hr, "Failed to calculate execute feature state.");

            hr = CalculateFeatureAction(featureRequestedState, BOOTSTRAPPER_FEATURE_STATE_UNKNOWN != featureExpectedState ? featureExpectedState : pFeature->currentState, FALSE, &rgRollbackFeatureActions[i], &fRollbackFeatureActionDelta);
            ExitOnFailure(hr, "Failed to calculate rollback feature state.");

            LogId(REPORT_STANDARD, MSG_PLANNED_MSI_FEATURE, pFeature->sczId, LoggingMsiFeatureStateToString(pFeature->currentState), LoggingMsiFeatureStateToString(featureRequestedState), LoggingMsiFeatureActionToString(rgFeatureActions[i]), LoggingMsiFeatureActionToString(rgRollbackFeatureActions[i]));
        }
    }

    // execute action
    switch (pPackage->currentState)
    {
    case BOOTSTRAPPER_PACKAGE_STATE_PRESENT:
        if (BOOTSTRAPPER_REQUEST_STATE_PRESENT == pPackage->requested || BOOTSTRAPPER_REQUEST_STATE_REPAIR == pPackage->requested)
        {
            // Take a look at the version and determine if this is a potential
            // minor upgrade (same ProductCode newer ProductVersion), otherwise,
            // there is a newer version so no work necessary.
            if (qwVersion > qwInstalledVersion)
            {
                execute = BOOTSTRAPPER_ACTION_STATE_MINOR_UPGRADE;
            }
            else if (qwVersion == qwInstalledVersion) // maintenance install "10.X.X.X" = "10.X.X.X"
            {
                execute = (BOOTSTRAPPER_REQUEST_STATE_REPAIR == pPackage->requested) ? BOOTSTRAPPER_ACTION_STATE_RECACHE : (fFeatureActionDelta ? BOOTSTRAPPER_ACTION_STATE_MAINTENANCE : BOOTSTRAPPER_ACTION_STATE_NONE);
            }
            else // newer version present "14.X.X.X" < "15.X.X.X", skip
            {
                execute = BOOTSTRAPPER_ACTION_STATE_NONE; // TODO: inform about this state to enable warnings
            }
        }
        else if (BOOTSTRAPPER_PACKAGE_STATE_ABSENT == pPackage->requested)
        {
            if (!pPackage->fUninstallable) // do not uninstall packages that we don't own.
            {
                execute = BOOTSTRAPPER_ACTION_STATE_NONE;
            }
            else //if (qwVersion == qwInstalledVersion) // install "10.X.X.X" = "10.X.X.X"
            {
                execute = BOOTSTRAPPER_ACTION_STATE_UNINSTALL;
            }
            //else
            //{
            //    execute = BOOTSTRAPPER_ACTION_STATE_NONE; // TODO: inform about this state to enable warnings
            //}
        }
        break;

    case BOOTSTRAPPER_PACKAGE_STATE_ABSENT: __fallthrough;
    case BOOTSTRAPPER_PACKAGE_STATE_CACHED: __fallthrough;
    case BOOTSTRAPPER_PACKAGE_STATE_SUPERSEDED:
        if (BOOTSTRAPPER_REQUEST_STATE_PRESENT == pPackage->requested || BOOTSTRAPPER_REQUEST_STATE_REPAIR == pPackage->requested)
        {
            execute = BOOTSTRAPPER_ACTION_STATE_INSTALL;
        }
        else if (BOOTSTRAPPER_PACKAGE_STATE_SUPERSEDED == pPackage->currentState && BOOTSTRAPPER_REQUEST_STATE_ABSENT == pPackage->requested && pPackage->fUninstallable)
        {
            execute = BOOTSTRAPPER_ACTION_STATE_UNINSTALL;
        }
        else
        {
            execute = BOOTSTRAPPER_ACTION_STATE_NONE;
        }
        break;

    default:
        hr = E_INVALIDARG;
        ExitOnRootFailure1(hr, "Invalid package current state result encountered during plan: %d", pPackage->currentState);
    }

    // rollback action
    BOOTSTRAPPER_PACKAGE_STATE state = BOOTSTRAPPER_PACKAGE_STATE_UNKNOWN != pPackage->expected ? pPackage->expected : pPackage->currentState;
    switch (state)
    {
    case BOOTSTRAPPER_PACKAGE_STATE_PRESENT:
        switch (pPackage->requested)
        {
        case BOOTSTRAPPER_REQUEST_STATE_PRESENT:
            rollback = fRollbackFeatureActionDelta ? BOOTSTRAPPER_ACTION_STATE_MAINTENANCE : BOOTSTRAPPER_ACTION_STATE_NONE;
            break;
        case BOOTSTRAPPER_REQUEST_STATE_REPAIR:
            rollback = BOOTSTRAPPER_ACTION_STATE_NONE;
            break;
        case BOOTSTRAPPER_REQUEST_STATE_ABSENT:
            rollback = BOOTSTRAPPER_ACTION_STATE_INSTALL;
            break;
        default:
            rollback = BOOTSTRAPPER_ACTION_STATE_NONE;
            break;
        }
        break;

    case BOOTSTRAPPER_PACKAGE_STATE_ABSENT: __fallthrough;
    case BOOTSTRAPPER_PACKAGE_STATE_CACHED: __fallthrough;
    case BOOTSTRAPPER_PACKAGE_STATE_SUPERSEDED:
        switch (pPackage->requested)
        {
        case BOOTSTRAPPER_REQUEST_STATE_PRESENT: __fallthrough;
        case BOOTSTRAPPER_REQUEST_STATE_REPAIR:
            rollback = pPackage->fUninstallable ? BOOTSTRAPPER_ACTION_STATE_UNINSTALL : BOOTSTRAPPER_ACTION_STATE_NONE;
            break;
        case BOOTSTRAPPER_REQUEST_STATE_ABSENT:
            rollback = BOOTSTRAPPER_PACKAGE_STATE_SUPERSEDED == state ? BOOTSTRAPPER_ACTION_STATE_INSTALL : BOOTSTRAPPER_ACTION_STATE_NONE;
            break;
        default:
            rollback = BOOTSTRAPPER_ACTION_STATE_NONE;
            break;
        }
        break;

    default:
        hr = E_INVALIDARG;
        ExitOnRootFailure(hr, "Invalid package detection result encountered.");
    }

    // add wait for cache
    if (hCacheEvent)
    {
        hr = PlanExecuteCacheSyncAndRollback(pPlan, pPackage, hCacheEvent, fPlanPackageCacheRollback);
        ExitOnFailure(hr, "Failed to plan package cache syncpoint");
    }

    // add execute action
    if (BOOTSTRAPPER_ACTION_STATE_NONE != execute)
    {
        if (NULL != pdwInsertSequence)
        {
            hr = PlanInsertExecuteAction(*pdwInsertSequence, pPlan, &pAction);
            ExitOnFailure(hr, "Failed to insert execute action.");
        }
        else
        {
            hr = PlanAppendExecuteAction(pPlan, &pAction);
            ExitOnFailure(hr, "Failed to append execute action.");
        }

        pAction->type = BURN_EXECUTE_ACTION_TYPE_MSI_PACKAGE;
        pAction->msiPackage.pPackage = pPackage;
        pAction->msiPackage.action = execute;
        pAction->msiPackage.rgFeatures = rgFeatureActions;
        rgFeatureActions = NULL;

        LoggingSetPackageVariable(dwPackageSequence, pPackage, FALSE, pLog, pVariables, &pAction->msiPackage.sczLogPath); // ignore errors.
        pAction->msiPackage.dwLoggingAttributes = pLog->dwAttributes;
    }

    // add rollback action
    if (BOOTSTRAPPER_ACTION_STATE_NONE != rollback)
    {
        hr = PlanAppendRollbackAction(pPlan, &pAction);
        ExitOnFailure(hr, "Failed to append rollback action.");

        pAction->type = BURN_EXECUTE_ACTION_TYPE_MSI_PACKAGE;
        pAction->msiPackage.pPackage = pPackage;
        pAction->msiPackage.action = rollback;
        pAction->msiPackage.rgFeatures = rgRollbackFeatureActions;
        rgRollbackFeatureActions = NULL;

        LoggingSetPackageVariable(dwPackageSequence, pPackage, TRUE, pLog, pVariables, &pAction->msiPackage.sczLogPath); // ignore errors.
        pAction->msiPackage.dwLoggingAttributes = pLog->dwAttributes;
    }

    // return values
    *pExecuteAction = execute;
    *pRollbackAction = rollback;

LExit:
    ReleaseMem(rgFeatureActions);
    ReleaseMem(rgRollbackFeatureActions);

    return hr;
}

extern "C" HRESULT MsiEngineExecutePackage(
    __in BURN_EXECUTE_ACTION* pExecuteAction,
    __in BURN_VARIABLES* pVariables,
    __in BOOL fRollback,
    __in PFN_MSIEXECUTEMESSAGEHANDLER pfnMessageHandler,
    __in LPVOID pvContext,
    __out BOOTSTRAPPER_APPLY_RESTART* pRestart
    )
{
    HRESULT hr = S_OK;
    WIU_MSI_EXECUTE_CONTEXT context = { };
    WIU_RESTART restart = WIU_RESTART_NONE;

    LPWSTR sczCachedDirectory = NULL;
    LPWSTR sczMsiPath = NULL;
    LPWSTR sczProperties = NULL;

    // Default to "verbose" logging and set extra debug mode only if explicitly required.
    DWORD dwLogMode = INSTALLLOGMODE_VERBOSE;

    if (pExecuteAction->msiPackage.dwLoggingAttributes & BURN_LOGGING_ATTRIBUTE_EXTRADEBUG)
    {
        dwLogMode |= INSTALLLOGMODE_EXTRADEBUG;
    }

    // get cached MSI path
    hr = CacheGetCompletedPath(pExecuteAction->msiPackage.pPackage->fPerMachine, pExecuteAction->msiPackage.pPackage->sczCacheId, &sczCachedDirectory);
    ExitOnFailure1(hr, "Failed to get cached path for package: %ls", pExecuteAction->msiPackage.pPackage->sczId);

    hr = PathConcat(sczCachedDirectory, pExecuteAction->msiPackage.pPackage->rgPayloads[0].pPayload->sczFilePath, &sczMsiPath);
    ExitOnFailure(hr, "Failed to build MSI path.");

    // Wire up the external UI handler and logging.
    hr = WiuInitializeExternalUI(pfnMessageHandler, pvContext, fRollback, &context);
    ExitOnFailure(hr, "Failed to initialize external UI handler.");

    if (pExecuteAction->msiPackage.sczLogPath && *pExecuteAction->msiPackage.sczLogPath)
    {
        hr = WiuEnableLog(dwLogMode, pExecuteAction->msiPackage.sczLogPath, 0);
        ExitOnFailure2(hr, "Failed to enable logging for package: %ls to: %ls", pExecuteAction->msiPackage.pPackage->sczId, pExecuteAction->msiPackage.sczLogPath);
    }

    // set up properties
    hr = MsiEngineConcatProperties(pExecuteAction->msiPackage.pPackage->Msi.rgProperties, pExecuteAction->msiPackage.pPackage->Msi.cProperties, pVariables, fRollback, &sczProperties);
    ExitOnFailure(hr, "Failed to add properties to argument string.");

    // add feature action properties
    hr = ConcatFeatureActionProperties(pExecuteAction->msiPackage.pPackage, pExecuteAction->msiPackage.rgFeatures, &sczProperties);
    ExitOnFailure(hr, "Failed to add feature action properties to argument string.");

    // add patch properties, except on uninstall because that can confuse the Windows Installer in some situations.
    if (BOOTSTRAPPER_ACTION_STATE_UNINSTALL != pExecuteAction->msiPackage.action)
    {
        hr = ConcatPatchProperty(pExecuteAction->msiPackage.pPackage, &sczProperties);
        ExitOnFailure(hr, "Failed to add patch properties to argument string.");
    }

    LogId(REPORT_STANDARD, MSG_APPLYING_PACKAGE, pExecuteAction->msiPackage.pPackage->sczId, LoggingActionStateToString(pExecuteAction->msiPackage.action), sczMsiPath, sczProperties);

    //
    // Do the actual action.
    //
    switch (pExecuteAction->msiPackage.action)
    {
    case BOOTSTRAPPER_ACTION_STATE_ADMIN_INSTALL:
        hr = StrAllocConcat(&sczProperties, L" ACTION=ADMIN", 0);
        ExitOnFailure(hr, "Failed to format property: ADMIN");
         __fallthrough;

    case BOOTSTRAPPER_ACTION_STATE_MAJOR_UPGRADE: __fallthrough;
    case BOOTSTRAPPER_ACTION_STATE_INSTALL:
        hr = StrAllocConcat(&sczProperties, L" REBOOT=ReallySuppress", 0);
        ExitOnFailure(hr, "Failed to add reboot suppression property on install.");

        hr = WiuInstallProduct(sczMsiPath, sczProperties, &restart);
        ExitOnFailure(hr, "Failed to install MSI package.");
        break;

    case BOOTSTRAPPER_ACTION_STATE_MINOR_UPGRADE:
        hr = StrAllocConcat(&sczProperties, L" REINSTALL=ALL REINSTALLMODE=\"vomus\" REBOOT=ReallySuppress", 0);
        ExitOnFailure(hr, "Failed to add reinstall mode and reboot suppression properties on minor upgrade.");

        hr = WiuInstallProduct(sczMsiPath, sczProperties, &restart);
        ExitOnFailure(hr, "Failed to perform minor upgrade of MSI package.");
        break;

    case BOOTSTRAPPER_ACTION_STATE_RECACHE:
        hr = StrAllocConcat(&sczProperties, L" REINSTALL=ALL REINSTALLMODE=\"cemus\" REBOOT=ReallySuppress", 0);
        ExitOnFailure(hr, "Failed to add reinstall mode and reboot suppression properties on repair.");
        __fallthrough;

    case BOOTSTRAPPER_ACTION_STATE_MAINTENANCE:
        hr = WiuConfigureProductEx(pExecuteAction->msiPackage.pPackage->Msi.sczProductCode, INSTALLLEVEL_DEFAULT, INSTALLSTATE_DEFAULT, sczProperties, &restart);
        ExitOnFailure(hr, "Failed to run maintanance mode for MSI package.");
        break;

    case BOOTSTRAPPER_ACTION_STATE_UNINSTALL:
        hr = StrAllocConcat(&sczProperties, L" REBOOT=ReallySuppress", 0);
        ExitOnFailure(hr, "Failed to add reboot suppression property on uninstall.");

        hr = WiuConfigureProductEx(pExecuteAction->msiPackage.pPackage->Msi.sczProductCode, INSTALLLEVEL_DEFAULT, INSTALLSTATE_ABSENT, sczProperties, &restart);
        ExitOnFailure(hr, "Failed to uninstall MSI package.");
        break;
    }

LExit:
    WiuUninitializeExternalUI(&context);

    ReleaseStr(sczProperties);
    ReleaseStr(sczMsiPath);
    ReleaseStr(sczCachedDirectory);

    switch (restart)
    {
        case WIU_RESTART_NONE:
            *pRestart = BOOTSTRAPPER_APPLY_RESTART_NONE;
            break;

        case WIU_RESTART_REQUIRED:
            *pRestart = BOOTSTRAPPER_APPLY_RESTART_REQUIRED;
            break;

        case WIU_RESTART_INITIATED:
            *pRestart = BOOTSTRAPPER_APPLY_RESTART_INITIATED;
            break;
    }

    return hr;
}

extern "C" HRESULT MsiEngineConcatProperties(
    __in_ecount(cProperties) BURN_MSIPROPERTY* rgProperties,
    __in DWORD cProperties,
    __in BURN_VARIABLES* pVariables,
    __in BOOL fRollback,
    __deref_out_z LPWSTR* psczProperties
    )
{
    HRESULT hr = S_OK;
    LPWSTR sczValue = NULL;
    LPWSTR sczEscapedValue = NULL;
    LPWSTR sczProperty = NULL;

    for (DWORD i = 0; i < cProperties; ++i)
    {
        BURN_MSIPROPERTY* pProperty = &rgProperties[i];

        // format property value
        hr = VariableFormatString(pVariables, (fRollback && pProperty->sczRollbackValue) ? pProperty->sczRollbackValue : pProperty->sczValue, &sczValue, NULL);
        ExitOnFailure(hr, "Failed to format property value.");

        // escape property value
        hr = EscapePropertyArgumentString(sczValue, &sczEscapedValue);
        ExitOnFailure(hr, "Failed to escape string.");

        // build part
        hr = StrAllocFormatted(&sczProperty, L" %s%=\"%s\"", pProperty->sczId, sczEscapedValue);
        ExitOnFailure(hr, "Failed to format property string part.");

        // append to property string
        hr = StrAllocConcat(psczProperties, sczProperty, 0);
        ExitOnFailure(hr, "Failed to append property string part.");
    }

LExit:
    ReleaseStr(sczValue);
    ReleaseStr(sczEscapedValue);
    ReleaseStr(sczProperty);
    return hr;
}


// internal helper functions

static HRESULT ParseRelatedMsiFromXml(
    __in IXMLDOMNode* pixnRelatedMsi,
    __in BURN_RELATED_MSI* pRelatedMsi
    )
{
    HRESULT hr = S_OK;
    IXMLDOMNodeList* pixnNodes = NULL;
    IXMLDOMNode* pixnNode = NULL;
    DWORD cNodes = 0;
    LPWSTR scz = NULL;

    // @Id
    hr = XmlGetAttributeEx(pixnRelatedMsi, L"Id", &pRelatedMsi->sczUpgradeCode);
    ExitOnFailure(hr, "Failed to get @Id.");

    // @MinVersion
    hr = XmlGetAttributeEx(pixnRelatedMsi, L"MinVersion", &scz);
    if (E_NOTFOUND != hr)
    {
        ExitOnFailure(hr, "Failed to get @MinVersion.");

        hr = FileVersionFromStringEx(scz, 0, &pRelatedMsi->qwMinVersion);
        ExitOnFailure1(hr, "Failed to parse @MinVersion: %ls", scz);

        // flag that we have a min version
        pRelatedMsi->fMinProvided = TRUE;

        // @MinInclusive
        hr = XmlGetYesNoAttribute(pixnRelatedMsi, L"MinInclusive", &pRelatedMsi->fMinInclusive);
        ExitOnFailure(hr, "Failed to get @MinInclusive.");
    }

    // @MaxVersion
    hr = XmlGetAttributeEx(pixnRelatedMsi, L"MaxVersion", &scz);
    if (E_NOTFOUND != hr)
    {
        ExitOnFailure(hr, "Failed to get @MaxVersion.");

        hr = FileVersionFromStringEx(scz, 0, &pRelatedMsi->qwMaxVersion);
        ExitOnFailure1(hr, "Failed to parse @MaxVersion: %ls", scz);

        // flag that we have a max version
        pRelatedMsi->fMaxProvided = TRUE;

        // @MaxInclusive
        hr = XmlGetYesNoAttribute(pixnRelatedMsi, L"MaxInclusive", &pRelatedMsi->fMaxInclusive);
        ExitOnFailure(hr, "Failed to get @MaxInclusive.");
    }

    // @OnlyDetect
    hr = XmlGetYesNoAttribute(pixnRelatedMsi, L"OnlyDetect", &pRelatedMsi->fOnlyDetect);
    ExitOnFailure(hr, "Failed to get @OnlyDetect.");

    // select language nodes
    hr = XmlSelectNodes(pixnRelatedMsi, L"Language", &pixnNodes);
    ExitOnFailure(hr, "Failed to select language nodes.");

    // get language node count
    hr = pixnNodes->get_length((long*)&cNodes);
    ExitOnFailure(hr, "Failed to get language node count.");

    if (cNodes)
    {
        // @LangInclusive
        hr = XmlGetYesNoAttribute(pixnRelatedMsi, L"LangInclusive", &pRelatedMsi->fLangInclusive);
        ExitOnFailure(hr, "Failed to get @LangInclusive.");

        // allocate memory for language IDs
        pRelatedMsi->rgdwLanguages = (DWORD*)MemAlloc(sizeof(DWORD) * cNodes, TRUE);
        ExitOnNull(pRelatedMsi->rgdwLanguages, hr, E_OUTOFMEMORY, "Failed to allocate memory for language IDs.");

        pRelatedMsi->cLanguages = cNodes;

        // parse language elements
        for (DWORD i = 0; i < cNodes; ++i)
        {
            hr = XmlNextElement(pixnNodes, &pixnNode, NULL);
            ExitOnFailure(hr, "Failed to get next node.");

            // @Id
            hr = XmlGetAttributeNumber(pixnNode, L"Id", &pRelatedMsi->rgdwLanguages[i]);
            ExitOnFailure(hr, "Failed to get Language/@Id.");

            // prepare next iteration
            ReleaseNullObject(pixnNode);
        }
    }

    hr = S_OK;

LExit:
    ReleaseObject(pixnNodes);
    ReleaseObject(pixnNode);
    ReleaseStr(scz);

    return hr;
}

static HRESULT EvaluateActionStateConditions(
    __in BURN_VARIABLES* pVariables,
    __in_z_opt LPCWSTR sczAddLocalCondition,
    __in_z_opt LPCWSTR sczAddSourceCondition,
    __in_z_opt LPCWSTR sczAdvertiseCondition,
    __out BOOTSTRAPPER_FEATURE_STATE* pState
    )
{
    HRESULT hr = S_OK;
    BOOL fCondition = FALSE;

    // if no condition was set, return no feature state
    if (!sczAddLocalCondition && !sczAddSourceCondition && !sczAdvertiseCondition)
    {
        *pState = BOOTSTRAPPER_FEATURE_STATE_UNKNOWN;
        ExitFunction();
    }

    if (sczAddLocalCondition)
    {
        hr = ConditionEvaluate(pVariables, sczAddLocalCondition, &fCondition);
        ExitOnFailure(hr, "Failed to evaluate add local condition.");

        if (fCondition)
        {
            *pState = BOOTSTRAPPER_FEATURE_STATE_LOCAL;
            ExitFunction();
        }
    }

    if (sczAddSourceCondition)
    {
        hr = ConditionEvaluate(pVariables, sczAddSourceCondition, &fCondition);
        ExitOnFailure(hr, "Failed to evaluate add source condition.");

        if (fCondition)
        {
            *pState = BOOTSTRAPPER_FEATURE_STATE_SOURCE;
            ExitFunction();
        }
    }

    if (sczAdvertiseCondition)
    {
        hr = ConditionEvaluate(pVariables, sczAdvertiseCondition, &fCondition);
        ExitOnFailure(hr, "Failed to evaluate advertise condition.");

        if (fCondition)
        {
            *pState = BOOTSTRAPPER_FEATURE_STATE_ADVERTISED;
            ExitFunction();
        }
    }

    // if no condition was true, set to absent
    *pState = BOOTSTRAPPER_FEATURE_STATE_ABSENT;

LExit:
    return hr;
}

static HRESULT CalculateFeatureAction(
    __in BOOTSTRAPPER_FEATURE_STATE currentState,
    __in BOOTSTRAPPER_FEATURE_STATE requestedState,
    __in BOOL fRepair,
    __out BOOTSTRAPPER_FEATURE_ACTION* pFeatureAction,
    __out BOOL* pfDelta
    )
{
    HRESULT hr = S_OK;

    *pFeatureAction = BOOTSTRAPPER_FEATURE_ACTION_NONE;
    switch (requestedState)
    {
    case BOOTSTRAPPER_FEATURE_STATE_UNKNOWN:
        *pFeatureAction = BOOTSTRAPPER_FEATURE_ACTION_NONE;
        break;

    case BOOTSTRAPPER_FEATURE_STATE_ABSENT:
        if (BOOTSTRAPPER_FEATURE_STATE_ABSENT != currentState)
        {
            *pFeatureAction = BOOTSTRAPPER_FEATURE_ACTION_REMOVE;
        }
        break;

    case BOOTSTRAPPER_FEATURE_STATE_ADVERTISED:
        if (BOOTSTRAPPER_FEATURE_STATE_ADVERTISED != currentState)
        {
            *pFeatureAction = BOOTSTRAPPER_FEATURE_ACTION_ADVERTISE;
        }
        else if (fRepair)
        {
            *pFeatureAction = BOOTSTRAPPER_FEATURE_ACTION_REINSTALL;
        }
        break;

    case BOOTSTRAPPER_FEATURE_STATE_LOCAL:
        if (BOOTSTRAPPER_FEATURE_STATE_LOCAL != currentState)
        {
            *pFeatureAction = BOOTSTRAPPER_FEATURE_ACTION_ADDLOCAL;
        }
        else if (fRepair)
        {
            *pFeatureAction = BOOTSTRAPPER_FEATURE_ACTION_REINSTALL;
        }
        break;

    case BOOTSTRAPPER_FEATURE_STATE_SOURCE:
        if (BOOTSTRAPPER_FEATURE_STATE_SOURCE != currentState)
        {
            *pFeatureAction = BOOTSTRAPPER_FEATURE_ACTION_ADDSOURCE;
        }
        else if (fRepair)
        {
            *pFeatureAction = BOOTSTRAPPER_FEATURE_ACTION_REINSTALL;
        }
        break;

    default:
        hr = E_UNEXPECTED;
        ExitOnRootFailure(hr, "Invalid state value.");
    }

    *pfDelta = (BOOTSTRAPPER_FEATURE_ACTION_NONE != *pFeatureAction);

LExit:
    return hr;
}

static HRESULT EscapePropertyArgumentString(
    __in LPCWSTR wzProperty,
    __inout_z LPWSTR* psczEscapedValue
    )
{
    HRESULT hr = S_OK;
    DWORD cch = 0;
    DWORD cchEscape = 0;
    LPCWSTR wzSource = NULL;
    LPWSTR wzTarget = NULL;

    // count characters to escape
    wzSource = wzProperty;
    while (*wzSource)
    {
        ++cch;
        if (L'\"' == *wzSource)
        {
            ++cchEscape;
        }
        ++wzSource;
    }

    // allocate target buffer
    hr = StrAlloc(psczEscapedValue, cch + cchEscape + 1); // character count, plus escape character count, plus null terminator
    ExitOnFailure(hr, "Failed to allocate string buffer.");

    // write to target buffer
    wzSource = wzProperty;
    wzTarget = *psczEscapedValue;
    while (*wzSource)
    {
        *wzTarget = *wzSource;
        if (L'\"' == *wzTarget)
        {
            ++wzTarget;
            *wzTarget = L'\"';
        }

        ++wzSource;
        ++wzTarget;
    }

    *wzTarget = L'\0'; // add null terminator

LExit:
    return hr;
}

static HRESULT ConcatFeatureActionProperties(
    __in BURN_PACKAGE* pPackage,
    __in BOOTSTRAPPER_FEATURE_ACTION* rgFeatureActions,
    __inout_z LPWSTR* psczArguments
    )
{
    HRESULT hr = S_OK;
    LPWSTR scz = NULL;
    LPWSTR sczAddLocal = NULL;
    LPWSTR sczAddSource = NULL;
    LPWSTR sczAddDefault = NULL;
    LPWSTR sczReinstall = NULL;
    LPWSTR sczAdvertise = NULL;
    LPWSTR sczRemove = NULL;

    // features
    for (DWORD i = 0; i < pPackage->Msi.cFeatures; ++i)
    {
        BURN_MSIFEATURE* pFeature = &pPackage->Msi.rgFeatures[i];

        switch (rgFeatureActions[i])
        {
        case BOOTSTRAPPER_FEATURE_ACTION_ADDLOCAL:
            if (sczAddLocal)
            {
                hr = StrAllocConcat(&sczAddLocal, L",", 0);
                ExitOnFailure(hr, "Failed to concat separator.");
            }
            hr = StrAllocConcat(&sczAddLocal, pFeature->sczId, 0);
            ExitOnFailure(hr, "Failed to concat feature.");
            break;

        case BOOTSTRAPPER_FEATURE_ACTION_ADDSOURCE:
            if (sczAddSource)
            {
                hr = StrAllocConcat(&sczAddSource, L",", 0);
                ExitOnFailure(hr, "Failed to concat separator.");
            }
            hr = StrAllocConcat(&sczAddSource, pFeature->sczId, 0);
            ExitOnFailure(hr, "Failed to concat feature.");
            break;

        case BOOTSTRAPPER_FEATURE_ACTION_ADDDEFAULT:
            if (sczAddDefault)
            {
                hr = StrAllocConcat(&sczAddDefault, L",", 0);
                ExitOnFailure(hr, "Failed to concat separator.");
            }
            hr = StrAllocConcat(&sczAddDefault, pFeature->sczId, 0);
            ExitOnFailure(hr, "Failed to concat feature.");
            break;

        case BOOTSTRAPPER_FEATURE_ACTION_REINSTALL:
            if (sczReinstall)
            {
                hr = StrAllocConcat(&sczReinstall, L",", 0);
                ExitOnFailure(hr, "Failed to concat separator.");
            }
            hr = StrAllocConcat(&sczReinstall, pFeature->sczId, 0);
            ExitOnFailure(hr, "Failed to concat feature.");
            break;

        case BOOTSTRAPPER_FEATURE_ACTION_ADVERTISE:
            if (sczAdvertise)
            {
                hr = StrAllocConcat(&sczAdvertise, L",", 0);
                ExitOnFailure(hr, "Failed to concat separator.");
            }
            hr = StrAllocConcat(&sczAdvertise, pFeature->sczId, 0);
            ExitOnFailure(hr, "Failed to concat feature.");
            break;

        case BOOTSTRAPPER_FEATURE_ACTION_REMOVE:
            if (sczRemove)
            {
                hr = StrAllocConcat(&sczRemove, L",", 0);
                ExitOnFailure(hr, "Failed to concat separator.");
            }
            hr = StrAllocConcat(&sczRemove, pFeature->sczId, 0);
            ExitOnFailure(hr, "Failed to concat feature.");
            break;
        }
    }

    if (sczAddLocal)
    {
        hr = StrAllocFormatted(&scz, L" ADDLOCAL=\"%s\"", sczAddLocal, 0);
        ExitOnFailure(hr, "Failed to format ADDLOCAL string.");

        hr = StrAllocConcat(psczArguments, scz, 0);
        ExitOnFailure(hr, "Failed to concat argument string.");
    }
    if (sczAddSource)
    {
        hr = StrAllocFormatted(&scz, L" ADDSOURCE=\"%s\"", sczAddSource, 0);
        ExitOnFailure(hr, "Failed to format ADDSOURCE string.");

        hr = StrAllocConcat(psczArguments, scz, 0);
        ExitOnFailure(hr, "Failed to concat argument string.");
    }
    if (sczAddDefault)
    {
        hr = StrAllocFormatted(&scz, L" ADDDEFAULT=\"%s\"", sczAddDefault, 0);
        ExitOnFailure(hr, "Failed to format ADDDEFAULT string.");

        hr = StrAllocConcat(psczArguments, scz, 0);
        ExitOnFailure(hr, "Failed to concat argument string.");
    }
    if (sczReinstall)
    {
        hr = StrAllocFormatted(&scz, L" REINSTALL=\"%s\"", sczReinstall, 0);
        ExitOnFailure(hr, "Failed to format REINSTALL string.");

        hr = StrAllocConcat(psczArguments, scz, 0);
        ExitOnFailure(hr, "Failed to concat argument string.");
    }
    if (sczAdvertise)
    {
        hr = StrAllocFormatted(&scz, L" ADVERTISE=\"%s\"", sczAdvertise, 0);
        ExitOnFailure(hr, "Failed to format ADVERTISE string.");

        hr = StrAllocConcat(psczArguments, scz, 0);
        ExitOnFailure(hr, "Failed to concat argument string.");
    }
    if (sczRemove)
    {
        hr = StrAllocFormatted(&scz, L" REMOVE=\"%s\"", sczRemove, 0);
        ExitOnFailure(hr, "Failed to format REMOVE string.");

        hr = StrAllocConcat(psczArguments, scz, 0);
        ExitOnFailure(hr, "Failed to concat argument string.");
    }

LExit:
    ReleaseStr(scz);
    ReleaseStr(sczAddLocal);
    ReleaseStr(sczAddSource);
    ReleaseStr(sczAddDefault);
    ReleaseStr(sczReinstall);
    ReleaseStr(sczAdvertise);
    ReleaseStr(sczRemove);

    return hr;
}

static HRESULT ConcatPatchProperty(
    __in BURN_PACKAGE* pPackage,
    __inout_z LPWSTR* psczArguments
    )
{
    HRESULT hr = S_OK;
    LPWSTR sczCachedDirectory = NULL;
    LPWSTR sczMspPath = NULL;
    LPWSTR sczPatches = NULL;

    for (DWORD i = 0; i < pPackage->Msi.cSlipstreamMspPackages; ++i)
    {
        BURN_PACKAGE* pMspPackage = pPackage->Msi.rgpSlipstreamMspPackages[i];
        AssertSz(BURN_PACKAGE_TYPE_MSP == pMspPackage->type, "Only MSP packages can be slipstream patches.");

        hr = CacheGetCompletedPath(pMspPackage->fPerMachine, pMspPackage->sczCacheId, &sczCachedDirectory);
        ExitOnFailure1(hr, "Failed to get cached path for MSP package: %ls", pMspPackage->sczId);

        hr = PathConcat(sczCachedDirectory, pMspPackage->rgPayloads[0].pPayload->sczFilePath, &sczMspPath);
        ExitOnFailure(hr, "Failed to build MSP path.");

        if (!sczPatches)
        {
            hr = StrAllocConcat(&sczPatches, L" PATCH=\"", 0);
            ExitOnFailure(hr, "Failed to prefix with PATCH property.");
        }
        else
        {
            hr = StrAllocConcat(&sczPatches, L";", 0);
            ExitOnFailure(hr, "Failed to semi-colon delimit patches.");
        }

        hr = StrAllocConcat(&sczPatches, sczMspPath, 0);
        ExitOnFailure(hr, "Failed to append patch path.");
    }

    if (sczPatches)
    {
        hr = StrAllocConcat(&sczPatches, L"\"", 0);
        ExitOnFailure(hr, "Failed to close the quoted PATCH property.");

        hr = StrAllocConcat(psczArguments, sczPatches, 0);
        ExitOnFailure(hr, "Failed to append PATCH property.");
    }

LExit:
    ReleaseStr(sczMspPath);
    ReleaseStr(sczCachedDirectory);
    ReleaseStr(sczPatches);
    return hr;
}
