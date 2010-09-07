//-------------------------------------------------------------------------------------------------
// <copyright file="msiengine.cpp" company="Microsoft">
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
//    Module: MSI Engine
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"


// constants

const DWORD BURN_MSI_PROGRESS_INVALID = 0xFFFFFFFF;


// structs

typedef struct _BURN_MSI_PROGRESS
{
    DWORD dwTotal;
    DWORD dwCompleted;
    DWORD dwStep;
    BOOL fMoveForward;
    BOOL fEnableActionData;
    BOOL fScriptInProgress;
} BURN_MSI_PROGRESS;

typedef struct _BURN_MSI_EXECUTE_CONTEXT
{
    BOOL fRollback;
    PFN_MSIEXECUTEMESSAGEHANDLER pfnMessageHandler;
    LPVOID pvContext;
    BURN_MSI_PROGRESS rgMsiProgress[64];
    DWORD dwCurrentProgressIndex;
} BURN_MSI_EXECUTE_CONTEXT;


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
static DWORD CheckForRestartErrorCode(
    __in DWORD dwErrorCode,
    __out BOOTSTRAPPER_APPLY_RESTART* pRestart
    );
static INT CALLBACK InstallEngineCallback(
    __in LPVOID pvContext,
    __in UINT uiMessage,
    __in_z_opt LPCWSTR wzMessage
    );
static INT CALLBACK InstallEngineRecordCallback(
    __in LPVOID pvContext,
    __in UINT uiMessage,
    __in_opt MSIHANDLE hRecord
    );
static INT HandleInstallMessage(
    __in BURN_MSI_EXECUTE_CONTEXT* pContext,
    __in INSTALLMESSAGE mt,
    __in UINT uiFlags,
    __in_z LPCWSTR wzMessage,
    __in_opt MSIHANDLE hRecord
    );
static INT HandleInstallProgress(
    __in BURN_MSI_EXECUTE_CONTEXT* pContext,
    __in_z_opt LPCWSTR wzMessage,
    __in_opt MSIHANDLE hRecord
    );
static INT SendProgressUpdate(
    __in BURN_MSI_EXECUTE_CONTEXT* pContext
    );
static void ResetProgress(
    __in BURN_MSI_EXECUTE_CONTEXT* pContext
    );
static INT HandleFilesInUseRecord(
    __in BURN_MSI_EXECUTE_CONTEXT* pContext,
    __in MSIHANDLE hRecord
    );
static DWORD CalculatePhaseProgress(
    __in BURN_MSI_EXECUTE_CONTEXT* pContext,
    __in DWORD dwProgressIndex,
    __in DWORD dwWeightPercentage
    );
static HRESULT ConcatProperties(
    __in BURN_PACKAGE* pPackage,
    __in BURN_VARIABLES* pVariables,
    __in BOOL fRollback,
    __deref_out_z LPWSTR* psczProperties
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

    ReleaseNullObject(pixnNodes);

    // select property nodes
    hr = XmlSelectNodes(pixnMsiPackage, L"MsiProperty", &pixnNodes);
    ExitOnFailure(hr, "Failed to select property nodes.");

    // get property node count
    hr = pixnNodes->get_length((long*)&cNodes);
    ExitOnFailure(hr, "Failed to get property node count.");

    if (cNodes)
    {
        // allocate memory for properties
        pPackage->Msi.rgProperties = (BURN_MSIPROPERTY*)MemAlloc(sizeof(BURN_MSIPROPERTY) * cNodes, TRUE);
        ExitOnNull(pPackage->Msi.rgProperties, hr, E_OUTOFMEMORY, "Failed to allocate memory for MSI property structs.");

        pPackage->Msi.cProperties = cNodes;

        // parse property elements
        for (DWORD i = 0; i < cNodes; ++i)
        {
            BURN_MSIPROPERTY* pProperty = &pPackage->Msi.rgProperties[i];

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

    ReleaseNullObject(pixnNodes);

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

    hr = S_OK;

LExit:
    ReleaseObject(pixnNodes);
    ReleaseObject(pixnNode);
    ReleaseStr(scz);

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
    DWORD er = ERROR_SUCCESS;
    WCHAR wzInstalledVersion[24] = { };
    DWORD cchInstalledVersion = 0;
    INSTALLSTATE installState = INSTALLSTATE_UNKNOWN;
    BOOTSTRAPPER_RELATED_OPERATION operation = BOOTSTRAPPER_RELATED_OPERATION_NONE;
    WCHAR wzProductCode[39] = { };
    DWORD64 qwVersion = 0;
    BOOL fPerMachine = FALSE;
    int nResult = 0;

    // detect self by product code
    // TODO: what to do about MSIINSTALLCONTEXT_USERMANAGED?
    cchInstalledVersion = countof(wzInstalledVersion);
    er = vpfnMsiGetProductInfoExW(pPackage->Msi.sczProductCode, NULL, pPackage->fPerMachine ? MSIINSTALLCONTEXT_MACHINE : MSIINSTALLCONTEXT_USERUNMANAGED, INSTALLPROPERTY_VERSIONSTRING, wzInstalledVersion, &cchInstalledVersion);
    if (ERROR_SUCCESS == er)
    {
        hr = FileVersionFromStringEx(wzInstalledVersion, 0, &pPackage->Msi.qwInstalledVersion);
        ExitOnFailure2(hr, "Failed to convert version: %ls to DWORD64 for ProductCode: %ls", wzInstalledVersion, pPackage->Msi.sczProductCode);

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

            nResult = pUserExperience->pUserExperience->OnDetectRelatedMsiPackage(pPackage->Msi.sczProductCode, pPackage->fPerMachine, pPackage->Msi.qwInstalledVersion, operation);
            hr = HRESULT_FROM_VIEW(nResult);
            ExitOnRootFailure(hr, "UX aborted detect related MSI package.");
        }
    }
    else if (ERROR_UNKNOWN_PRODUCT == er || ERROR_UNKNOWN_PROPERTY == er) // package not present.
    {
        pPackage->currentState = BOOTSTRAPPER_PACKAGE_STATE_ABSENT;
    }
    else
    {
        ExitOnWin32Error1(er, hr, "Failed to get product information for ProductCode: %ls", pPackage->Msi.sczProductCode);
    }

    // detect related packages by upgrade code
    for (DWORD i = 0; i < pPackage->Msi.cRelatedMsis; ++i)
    {
        BURN_RELATED_MSI* pRelatedMsi = &pPackage->Msi.rgRelatedMsis[i];

        for (DWORD iProduct = 0; ; ++iProduct)
        {
            // get product
            er = vpfnMsiEnumRelatedProductsW(pRelatedMsi->sczUpgradeCode, 0, iProduct, wzProductCode);
            if (ERROR_NO_MORE_ITEMS == er)
            {
                break;
            }
            ExitOnWin32Error(er, hr, "Failed to enum related products.");

            // get product version
            cchInstalledVersion = countof(wzInstalledVersion);
            er = vpfnMsiGetProductInfoExW(wzProductCode, NULL, MSIINSTALLCONTEXT_USERUNMANAGED, INSTALLPROPERTY_VERSIONSTRING, wzInstalledVersion, &cchInstalledVersion);
            if (ERROR_UNKNOWN_PRODUCT != er)
            {
                ExitOnWin32Error1(er, hr, "Failed to get version for product in user unmanaged context: %ls", wzProductCode);
                fPerMachine = FALSE;
            }
            else
            {
                er = vpfnMsiGetProductInfoExW(wzProductCode, NULL, MSIINSTALLCONTEXT_MACHINE, INSTALLPROPERTY_VERSIONSTRING, wzInstalledVersion, &cchInstalledVersion);
                if (ERROR_UNKNOWN_PRODUCT != er)
                {
                    ExitOnWin32Error1(er, hr, "Failed to get version for product in machine context: %ls", wzProductCode);
                    fPerMachine = TRUE;
                }
                else
                {
                    continue;
                }
            }

            hr = FileVersionFromStringEx(wzInstalledVersion, 0, &qwVersion);
            ExitOnFailure2(hr, "Failed to convert version: %ls to DWORD64 for ProductCode: %ls", wzInstalledVersion, wzProductCode);

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
            nResult = pUserExperience->pUserExperience->OnDetectRelatedMsiPackage(wzProductCode, fPerMachine, qwVersion, operation);
            hr = HRESULT_FROM_VIEW(nResult);
            ExitOnRootFailure(hr, "UX aborted detect related MSI package.");
        }
    }

    // detect features
    for (DWORD i = 0; i < pPackage->Msi.cFeatures; ++i)
    {
        BURN_MSIFEATURE* pFeature = &pPackage->Msi.rgFeatures[i];

        // get current state
        if (BOOTSTRAPPER_PACKAGE_STATE_PRESENT == pPackage->currentState) // only try to detect features if the product is installed
        {
            installState = vpfnMsiQueryFeatureStateW(pPackage->Msi.sczProductCode, pFeature->sczId);
            if (INSTALLSTATE_INVALIDARG == installState)
            {
                hr = E_INVALIDARG;
                ExitOnRootFailure(hr, "Failed to query feature state.");
            }
            else if (INSTALLSTATE_UNKNOWN == installState) // in case of an upgrade this could happen
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

        // pass to UX
        nResult = pUserExperience->pUserExperience->OnDetectMsiFeature(pPackage->sczId, pFeature->sczId, pFeature->currentState);
        hr = HRESULT_FROM_VIEW(nResult);
        ExitOnRootFailure(hr, "UX aborted detect.");
    }

LExit:
    return hr;
}

//
// Plan - calculates the execute and rollback state for the requested package state.
//
extern "C" HRESULT MsiEnginePlanPackage(
    __in DWORD dwPackageSequence,
    __in BURN_PACKAGE* pPackage,
    __in BURN_PLAN* pPlan,
    __in BURN_LOGGING* pLog,
    __in BURN_VARIABLES* pVariables,
    __in_opt HANDLE hCacheEvent,
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
    switch (BOOTSTRAPPER_PACKAGE_STATE_UNKNOWN != pPackage->expected ? pPackage->expected : pPackage->currentState)
    {
    case BOOTSTRAPPER_PACKAGE_STATE_PRESENT:
        switch (pPackage->requested)
        {
        case BOOTSTRAPPER_REQUEST_STATE_PRESENT: __fallthrough;
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
            rollback = BOOTSTRAPPER_ACTION_STATE_NONE;
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
        hr = PlanAppendExecuteAction(pPlan, &pAction);
        ExitOnFailure(hr, "Failed to append wait action.");

        pAction->type = BURN_EXECUTE_ACTION_TYPE_WAIT;
        pAction->wait.hEvent = hCacheEvent;
    }

    // add execute action
    if (BOOTSTRAPPER_ACTION_STATE_NONE != execute)
    {
        hr = PlanAppendExecuteAction(pPlan, &pAction);
        ExitOnFailure(hr, "Failed to append execute action.");

        pAction->type = BURN_EXECUTE_ACTION_TYPE_MSI_PACKAGE;
        pAction->msiPackage.pPackage = pPackage;
        pAction->msiPackage.action = execute;
        pAction->msiPackage.rgFeatures = rgFeatureActions;
        rgFeatureActions = NULL;

        LoggingSetPackageVariable(dwPackageSequence, pPackage, FALSE, pLog, pVariables, &pAction->msiPackage.sczLogPath); // ignore errors.
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
    }

    // add checkpoints
    if (BOOTSTRAPPER_ACTION_STATE_NONE != execute || BOOTSTRAPPER_ACTION_STATE_NONE != rollback)
    {
        DWORD dwCheckpointId = PlanGetNextCheckpointId();

        // execute checkpoint
        hr = PlanAppendExecuteAction(pPlan, &pAction);
        ExitOnFailure(hr, "Failed to append execute action.");

        pAction->type = BURN_EXECUTE_ACTION_TYPE_CHECKPOINT;
        pAction->checkpoint.dwId = dwCheckpointId;

        // rollback checkpoint
        hr = PlanAppendRollbackAction(pPlan, &pAction);
        ExitOnFailure(hr, "Failed to append rollback action.");

        pAction->type = BURN_EXECUTE_ACTION_TYPE_CHECKPOINT;
        pAction->checkpoint.dwId = dwCheckpointId;
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
    DWORD er = ERROR_SUCCESS;
    BURN_MSI_EXECUTE_CONTEXT context = { };

    LPWSTR sczCachedDirectory = NULL;
    LPWSTR sczMsiPath = NULL;

    DWORD dwUiLevel = INSTALLUILEVEL_NONE | INSTALLUILEVEL_SOURCERESONLY;
    DWORD dwMessageFilter = INSTALLLOGMODE_INITIALIZE | INSTALLLOGMODE_TERMINATE |
                            INSTALLLOGMODE_FATALEXIT | INSTALLLOGMODE_ERROR | INSTALLLOGMODE_WARNING |
                            INSTALLLOGMODE_RESOLVESOURCE | INSTALLLOGMODE_OUTOFDISKSPACE |
                            INSTALLLOGMODE_ACTIONSTART | INSTALLLOGMODE_ACTIONDATA | INSTALLLOGMODE_COMMONDATA|
                            INSTALLLOGMODE_PROGRESS;
    INSTALLUI_HANDLERW pfnPreviousExternalUI = NULL;
    INSTALLUI_HANDLER_RECORD pfnPreviousExternalUIRecord = NULL;
    BOOL fSetPreviousExternalUI = FALSE;
    BOOL fSetPreviousExternalUIRecord = FALSE;

    // default to "verbose" logging
    DWORD dwLogMode =  INSTALLLOGMODE_FATALEXIT | INSTALLLOGMODE_ERROR | INSTALLLOGMODE_WARNING |
                       INSTALLLOGMODE_USER | INSTALLLOGMODE_INFO | INSTALLLOGMODE_RESOLVESOURCE |
                       INSTALLLOGMODE_OUTOFDISKSPACE | INSTALLLOGMODE_ACTIONSTART | INSTALLLOGMODE_ACTIONDATA |
                       INSTALLLOGMODE_COMMONDATA | INSTALLLOGMODE_PROPERTYDUMP | INSTALLLOGMODE_VERBOSE;

    LPWSTR sczProperties = NULL;

    context.fRollback = fRollback;
    context.pfnMessageHandler = pfnMessageHandler;
    context.pvContext = pvContext;

    // get cached MSI path
    hr = CacheGetCompletedPath(pExecuteAction->msiPackage.pPackage->fPerMachine, pExecuteAction->msiPackage.pPackage->sczCacheId, &sczCachedDirectory);
    ExitOnFailure1(hr, "Failed to get cached path for package: %ls", pExecuteAction->msiPackage.pPackage->sczId);

    hr = PathConcat(sczCachedDirectory, pExecuteAction->msiPackage.pPackage->rgPayloads[0].pPayload->sczFilePath, &sczMsiPath);
    ExitOnFailure(hr, "Failed to build MSI path.");

    // Wire up logging and the external UI handler.
    vpfnMsiSetInternalUI(static_cast<INSTALLUILEVEL>(dwUiLevel), NULL);

    // If the external UI record is available (MSI version >= 3.1) use it but fall back to the standard external
    // UI handler if necesary.
    if (vpfnMsiSetExternalUIRecord)
    {
        er = vpfnMsiSetExternalUIRecord(InstallEngineRecordCallback, dwMessageFilter, &context, &pfnPreviousExternalUIRecord);
        ExitOnWin32Error(er, hr, "Failed to wire up external UI record handler.");
        fSetPreviousExternalUIRecord = TRUE;
    }
    else
    {
        pfnPreviousExternalUI = vpfnMsiSetExternalUIW(InstallEngineCallback, dwMessageFilter, &context);
        fSetPreviousExternalUI = TRUE;
    }

    //if (BURN_LOGGING_LEVEL_DEBUG == logLevel)
    //{
    //    dwLogMode | INSTALLLOGMODE_EXTRADEBUG;
    //}

    if (pExecuteAction->msiPackage.sczLogPath && *pExecuteAction->msiPackage.sczLogPath)
    {
        er = vpfnMsiEnableLogW(dwLogMode, pExecuteAction->msiPackage.sczLogPath, 0);
        ExitOnWin32Error2(er, hr, "Failed to enable logging for package: %ls to: %ls", pExecuteAction->msiPackage.pPackage->sczId, pExecuteAction->msiPackage.sczLogPath);
    }

    // set up properties
    hr = ConcatProperties(pExecuteAction->msiPackage.pPackage, pVariables, fRollback, &sczProperties);
    ExitOnFailure(hr, "Failed to add properties to argument string.");

    // add feature action properties
    hr = ConcatFeatureActionProperties(pExecuteAction->msiPackage.pPackage, pExecuteAction->msiPackage.rgFeatures, &sczProperties);
    ExitOnFailure(hr, "Failed to add feature action properties to argument string.");

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

        er = vpfnMsiInstallProductW(sczMsiPath, sczProperties);
        er = CheckForRestartErrorCode(er, pRestart);
        ExitOnWin32Error(er, hr, "Failed to install MSI package.");
        break;

    case BOOTSTRAPPER_ACTION_STATE_MINOR_UPGRADE:
        hr = StrAllocConcat(&sczProperties, L" REINSTALL=ALL REINSTALLMODE=\"vomus\" REBOOT=ReallySuppress", 0);
        ExitOnFailure(hr, "Failed to add reinstall mode and reboot suppression properties on minor upgrade.");

        er = vpfnMsiInstallProductW(sczMsiPath, sczProperties);
        er = CheckForRestartErrorCode(er, pRestart);
        ExitOnWin32Error(er, hr, "Failed to perform minor upgrade of MSI package.");
        break;

    case BOOTSTRAPPER_ACTION_STATE_RECACHE:
        hr = StrAllocConcat(&sczProperties, L" REINSTALL=ALL REINSTALLMODE=\"cemus\" REBOOT=ReallySuppress", 0);
        ExitOnFailure(hr, "Failed to add reinstall mode and reboot suppression properties on repair.");
        __fallthrough;

    case BOOTSTRAPPER_ACTION_STATE_MAINTENANCE:
        er = vpfnMsiConfigureProductExW(pExecuteAction->msiPackage.pPackage->Msi.sczProductCode, INSTALLLEVEL_DEFAULT, INSTALLSTATE_DEFAULT, sczProperties);
        er = CheckForRestartErrorCode(er, pRestart);
        ExitOnWin32Error(er, hr, "Failed to run maintanance mode for MSI package.");
        break;

    case BOOTSTRAPPER_ACTION_STATE_UNINSTALL:
        hr = StrAllocConcat(&sczProperties, L" REBOOT=ReallySuppress", 0);
        ExitOnFailure(hr, "Failed to add reboot suppression property on uninstall.");

        er = vpfnMsiConfigureProductExW(pExecuteAction->msiPackage.pPackage->Msi.sczProductCode, INSTALLLEVEL_DEFAULT, INSTALLSTATE_ABSENT, sczProperties);
        er = CheckForRestartErrorCode(er, pRestart);
        ExitOnWin32Error(er, hr, "Failed to uninstall MSI package.");
        break;
    }

LExit:
    if (fSetPreviousExternalUI)  // unset the UI handler
    {
        vpfnMsiSetExternalUIW(pfnPreviousExternalUI, 0, NULL);
    }

    if (fSetPreviousExternalUIRecord)  // unset the UI record handler
    {
        vpfnMsiSetExternalUIRecord(pfnPreviousExternalUIRecord, 0, NULL, NULL);
    }

    ReleaseStr(sczCachedDirectory);
    ReleaseStr(sczMsiPath);
    ReleaseStr(sczProperties);

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

        // flag that we have a min version
        pRelatedMsi->fMinProvided = TRUE;

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

static DWORD CheckForRestartErrorCode(
    __in DWORD dwErrorCode,
    __out BOOTSTRAPPER_APPLY_RESTART* pRestart
    )
{
    switch (dwErrorCode)
    {
    case ERROR_SUCCESS_REBOOT_REQUIRED:
    case ERROR_SUCCESS_RESTART_REQUIRED:
        *pRestart = BOOTSTRAPPER_APPLY_RESTART_REQUIRED;
        dwErrorCode = ERROR_SUCCESS;
        break;

    case ERROR_SUCCESS_REBOOT_INITIATED:
    case ERROR_INSTALL_SUSPEND:
        *pRestart = BOOTSTRAPPER_APPLY_RESTART_INITIATED;
        dwErrorCode = ERROR_SUCCESS;
        break;
    }

    return dwErrorCode;
}

static INT CALLBACK InstallEngineCallback(
    __in LPVOID pvContext,
    __in UINT uiMessage,
    __in_z_opt LPCWSTR wzMessage
    )
{
    INT nResult = IDNOACTION;
    BURN_MSI_EXECUTE_CONTEXT* pContext = (BURN_MSI_EXECUTE_CONTEXT*)pvContext;
    INSTALLMESSAGE mt = static_cast<INSTALLMESSAGE>(0xFF000000 & uiMessage);
    UINT uiFlags = 0x00FFFFFF & uiMessage;

    if (wzMessage)
    {
        if (INSTALLMESSAGE_PROGRESS == mt)
        {
            nResult = HandleInstallProgress(pContext, wzMessage, NULL);
        }
        else
        {
            nResult = HandleInstallMessage(pContext, mt, uiFlags, wzMessage, NULL);
        }
    }

    return nResult;
}

static INT CALLBACK InstallEngineRecordCallback(
    __in LPVOID pvContext,
    __in UINT uiMessage,
    __in_opt MSIHANDLE hRecord
    )
{
    INT nResult = IDNOACTION;
    HRESULT hr = S_OK;
    BURN_MSI_EXECUTE_CONTEXT* pContext = (BURN_MSI_EXECUTE_CONTEXT*)pvContext;

    INSTALLMESSAGE mt = static_cast<INSTALLMESSAGE>(0xFF000000 & uiMessage);
    UINT uiFlags = 0x00FFFFFF & uiMessage;
    LPWSTR sczMessage = NULL;
    DWORD cchMessage = 0;

    if (hRecord)
    {
        if (INSTALLMESSAGE_PROGRESS == mt)
        {
            nResult = HandleInstallProgress(pContext, NULL, hRecord);
        }
        else
        {
            // create formated message string
#pragma prefast(push)
#pragma prefast(disable:6298) // docs explicitly say this is a valid option for getting the buffer size
            DWORD er = ::MsiFormatRecordW(NULL, hRecord, L"", &cchMessage);
#pragma prefast(pop)
            if (ERROR_MORE_DATA == er || ERROR_SUCCESS == er)
            {
                hr = StrAlloc(&sczMessage, ++cchMessage);
            }
            else
            {
                hr = HRESULT_FROM_WIN32(er);
            }
            ExitOnFailure(hr, "Failed to allocate string for formated message.");

            er = ::MsiFormatRecordW(NULL, hRecord, sczMessage, &cchMessage);
            hr = HRESULT_FROM_WIN32(er);
            ExitOnRootFailure(hr, "Failed to format message record.");

            // Pass to handler including both the formated message and the original record.
            nResult = HandleInstallMessage(pContext, mt, uiFlags, sczMessage, hRecord);
        }
    }

LExit:
    ReleaseStr(sczMessage);
    return nResult;
}

static INT HandleInstallMessage(
    __in BURN_MSI_EXECUTE_CONTEXT* pContext,
    __in INSTALLMESSAGE mt,
    __in UINT uiFlags,
    __in_z LPCWSTR wzMessage,
    __in_opt MSIHANDLE hRecord
    )
{
    INT nResult = IDOK;
    BURN_MSI_EXECUTE_MESSAGE message = { };

Trace2(REPORT_STANDARD, "install[%x]: %ls", pContext->dwCurrentProgressIndex, wzMessage);

    switch (mt)
    {
    case INSTALLMESSAGE_INITIALIZE: // this message is received prior to internal UI initialization, no string data
        ResetProgress(pContext);
        break;

    case INSTALLMESSAGE_TERMINATE: // sent after UI termination, no string data
        break;

    case INSTALLMESSAGE_ACTIONSTART:
        if (BURN_MSI_PROGRESS_INVALID != pContext->dwCurrentProgressIndex && pContext->rgMsiProgress[pContext->dwCurrentProgressIndex].fEnableActionData)
        {
            pContext->rgMsiProgress[pContext->dwCurrentProgressIndex].fEnableActionData = FALSE;
        }

        //nResult = m_pView->OnExecuteMsiMessage(m_pExecutingPackage, mt, uiFlags, wzMessage, hRecord);
        message.type = BURN_MSI_EXECUTE_MESSAGE_MSI_MESSAGE;
        message.msiMessage.mt = mt;
        message.msiMessage.uiFlags = uiFlags;
        message.msiMessage.wzMessage = wzMessage;
        nResult = pContext->pfnMessageHandler(pContext->pvContext, &message);
        break;

    case INSTALLMESSAGE_ACTIONDATA:
        if (BURN_MSI_PROGRESS_INVALID != pContext->dwCurrentProgressIndex && pContext->rgMsiProgress[pContext->dwCurrentProgressIndex].fEnableActionData)
        {
            if (pContext->rgMsiProgress[pContext->dwCurrentProgressIndex].fMoveForward)
            {
                pContext->rgMsiProgress[pContext->dwCurrentProgressIndex].dwCompleted += pContext->rgMsiProgress[pContext->dwCurrentProgressIndex].dwStep;
            }
            else
            {
                pContext->rgMsiProgress[pContext->dwCurrentProgressIndex].dwCompleted -= pContext->rgMsiProgress[pContext->dwCurrentProgressIndex].dwStep;
            }
Trace3(REPORT_STANDARD, "progress[%x]: actiondata, progress: %u  forward: %d", pContext->dwCurrentProgressIndex, pContext->rgMsiProgress[pContext->dwCurrentProgressIndex].dwStep, pContext->rgMsiProgress[pContext->dwCurrentProgressIndex].fMoveForward);

            nResult = SendProgressUpdate(pContext);
        }
        else
        {
            message.type = BURN_MSI_EXECUTE_MESSAGE_MSI_MESSAGE;
            message.msiMessage.mt = mt;
            message.msiMessage.uiFlags = uiFlags;
            message.msiMessage.wzMessage = wzMessage;
            nResult = pContext->pfnMessageHandler(pContext->pvContext, &message);
        }
        break;

    //case INSTALLMESSAGE_RESOLVESOURCE:
    //    m_pView->OnExecuteMsiMessage(m_pExecutingPackage, mt, uiFlags, wzMessage, hRecord);
    //    nResult = IDNOACTION; // always return no action (0) for resolve source.
    //    break;

    case INSTALLMESSAGE_OUTOFDISKSPACE: __fallthrough;
    case INSTALLMESSAGE_FATALEXIT: __fallthrough;
    case INSTALLMESSAGE_ERROR:
        {
        DWORD dwErrorCode = 0;
        if (hRecord)
        {
            dwErrorCode = ::MsiRecordGetInteger(hRecord, 1);
        }

        message.type = BURN_MSI_EXECUTE_MESSAGE_ERROR;
        message.error.dwErrorCode = dwErrorCode;
        message.error.uiFlags = uiFlags;
        message.error.wzMessage = wzMessage;
        nResult = pContext->pfnMessageHandler(pContext->pvContext, &message);
        }
        break;

    //case INSTALLMESSAGE_RMFILESINUSE: __fallthrough;
    case INSTALLMESSAGE_FILESINUSE:
        nResult = HandleFilesInUseRecord(pContext, hRecord);
        break;

/*
    case INSTALLMESSAGE_WARNING:
    case INSTALLMESSAGE_USER:
    case INSTALLMESSAGE_INFO:
    case INSTALLMESSAGE_SHOWDIALOG: // sent prior to display of authored dialog or wizard

#if 0
    case INSTALLMESSAGE_COMMONDATA:
        if (L'1' == wzMessage[0] && L':' == wzMessage[1] && L' ' == wzMessage[2])
        {
            if (L'0' == wzMessage[3])
            {
                // TODO: handle the language common data message.
                lres = IDOK;
                return lres;
            }
            else if (L'1' == wzMessage[3])
            {
                // TODO: really handle sending the caption.
                lres = ::SendSuxMessage(pInstallContext->pSetupUXInformation, SRM_EXEC_SET_CAPTION, uiFlags, reinterpret_cast<LPARAM>(wzMessage + 3));
                return lres;
            }
            else if (L'2' == wzMessage[3])
            {
                // TODO: really handle sending the cancel button status.
                lres = ::SendSuxMessage(pInstallContext->pSetupUXInformation, SRM_EXEC_SET_CANCEL, uiFlags, reinterpret_cast<LPARAM>(wzMessage + 3));
                return lres;
            }
        }
        break;
#endif
*/

    default:
        //nResult = m_pView->OnExecuteMsiMessage(m_pExecutingPackage, mt, uiFlags, wzMessage, hRecord);
        message.type = BURN_MSI_EXECUTE_MESSAGE_MSI_MESSAGE;
        message.msiMessage.mt = mt;
        message.msiMessage.uiFlags = uiFlags;
        message.msiMessage.wzMessage = wzMessage;
        nResult = pContext->pfnMessageHandler(pContext->pvContext, &message);
        break;
    }

    return nResult;
}

static INT HandleInstallProgress(
    __in BURN_MSI_EXECUTE_CONTEXT* pContext,
    __in_z_opt LPCWSTR wzMessage,
    __in_opt MSIHANDLE hRecord
    )
{
    HRESULT hr = S_OK;
    INT nResult = IDOK;
    INT iFields[4] = { };
    INT cFields = 0;
    LPCWSTR pwz = NULL;
    DWORD cch = 0;

    // get field values
    if (hRecord)
    {
        cFields = ::MsiRecordGetFieldCount(hRecord);
        cFields = min(cFields, countof(iFields)); // no buffer overrun if there are more fields than our buffer can hold
        for (INT i = 0; i < cFields; ++i)
        {
            iFields[i] = ::MsiRecordGetInteger(hRecord, i + 1);
        }
    }
    else
    {
        Assert(wzMessage);

        // parse message string
        pwz = wzMessage;
        while (cFields < 4)
        {
            // check if we have the start of a valid part
            if ((L'1' + cFields) != pwz[0] || L':' != pwz[1] || L' ' != pwz[2])
            {
                break;
            }
            pwz += 3;

            // find character count of number
            cch = 0;
            while (pwz[cch] && L' ' != pwz[cch])
            {
                ++cch;
            }

            // parse number
            hr = StrStringToInt32(pwz, cch, &iFields[cFields]);
            ExitOnFailure(hr, "Failed to parse MSI message part.");

            // increment field count
            ++cFields;
        }
    }

#ifdef _DEBUG
WCHAR wz[256];
swprintf_s(wz, countof(wz), L"1: %d 2: %d 3: %d 4: %d", iFields[0], iFields[1], iFields[2], iFields[3]);
Trace2(REPORT_STANDARD, "progress[%x]: %ls", pContext->dwCurrentProgressIndex, wz);
#endif

    // verify that we have enought field values
    if (1 > cFields)
    {
        ExitFunction(); // unknown message, bail
    }

    // hande based on message type
    switch (iFields[0])
    {
    case 0: // master progress reset
        if (4 > cFields)
        {
            Trace2(REPORT_STANDARD, "INSTALLMESSAGE_PROGRESS - Invalid field count %d, '%S'", cFields, wzMessage);
            ExitFunction();
        }
        //Trace3(REPORT_STANDARD, "INSTALLMESSAGE_PROGRESS - MASTER RESET - %d, %d, %d", iFields[1], iFields[2], iFields[3]);

        // Update the index into progress array.
        if (BURN_MSI_PROGRESS_INVALID == pContext->dwCurrentProgressIndex)
        {
            pContext->dwCurrentProgressIndex = 0;
        }
        else if (pContext->dwCurrentProgressIndex + 1 < countof(pContext->rgMsiProgress))
        {
            ++pContext->dwCurrentProgressIndex;
        }
        else
        {
            hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
            ExitOnRootFailure(hr, "(Insufficient space to hold progress information.");
        }

        // we only care about the first stage after script execution has started
        //if (!pEngineInfo->fMsiProgressScriptInProgress && 1 != iFields[3])
        //{
        //    pEngineInfo->fMsiProgressFinished = TRUE;
        //}

        pContext->rgMsiProgress[pContext->dwCurrentProgressIndex].dwTotal = iFields[1];
        pContext->rgMsiProgress[pContext->dwCurrentProgressIndex].dwCompleted = 0 == iFields[2] ? 0 : iFields[1]; // if forward start at 0, if backwards start at max
        pContext->rgMsiProgress[pContext->dwCurrentProgressIndex].fMoveForward = (0 == iFields[2]);
        pContext->rgMsiProgress[pContext->dwCurrentProgressIndex].fEnableActionData = FALSE;
        pContext->rgMsiProgress[pContext->dwCurrentProgressIndex].fScriptInProgress = (1 == iFields[3]);

        if (0 == pContext->dwCurrentProgressIndex)
        {
            // HACK!!! this is a hack courtesy of the Windows Installer team. It seems the script planning phase
            // is always off by "about 50".  So we'll toss an extra 50 ticks on so that the standard progress
            // doesn't go over 100%.  If there are any custom actions, they may blow the total so we'll call this
            // "close" and deal with the rest.
            pContext->rgMsiProgress[pContext->dwCurrentProgressIndex].dwTotal += 50;
        }
        break;

    case 1: // action info
        if (3 > cFields)
        {
            Trace2(REPORT_STANDARD, "INSTALLMESSAGE_PROGRESS - Invalid field count %d, '%S'", cFields, wzMessage);
            ExitFunction();
        }
        //Trace3(REPORT_STANDARD, "INSTALLMESSAGE_PROGRESS - ACTION INFO - %d, %d, %d", iFields[1], iFields[2], iFields[3]);

        if (0 == iFields[2])
        {
            pContext->rgMsiProgress[pContext->dwCurrentProgressIndex].fEnableActionData = FALSE;
        }
        else
        {
            pContext->rgMsiProgress[pContext->dwCurrentProgressIndex].fEnableActionData = TRUE;
            pContext->rgMsiProgress[pContext->dwCurrentProgressIndex].dwStep = iFields[1];
        }
        break;

    case 2: // progress report
        if (2 > cFields)
        {
            Trace2(REPORT_STANDARD, "INSTALLMESSAGE_PROGRESS - Invalid field count %d, '%S'", cFields, wzMessage);
            break;
        }
        //Trace3(REPORT_STANDARD, "INSTALLMESSAGE_PROGRESS - PROGRESS REPORT - %d, %d, %d", iFields[1], iFields[2], iFields[3]);

        if (BURN_MSI_PROGRESS_INVALID == pContext->dwCurrentProgressIndex)
        {
            break;
        }
        else if (0 == pContext->rgMsiProgress[pContext->dwCurrentProgressIndex].dwTotal)
        {
            break;
        }

        // update progress
        if (pContext->rgMsiProgress[pContext->dwCurrentProgressIndex].fMoveForward)
        {
            pContext->rgMsiProgress[pContext->dwCurrentProgressIndex].dwCompleted += iFields[1];
        }
        else
        {
            pContext->rgMsiProgress[pContext->dwCurrentProgressIndex].dwCompleted -= iFields[1];
        }
        break;

    case 3:
        pContext->rgMsiProgress[pContext->dwCurrentProgressIndex].dwTotal += iFields[1];
        break;

    default:
        ExitFunction(); // unknown message, bail
    }

    // If we have a valid progress index, send an update.
    if (BURN_MSI_PROGRESS_INVALID != pContext->dwCurrentProgressIndex)
    {
        nResult = SendProgressUpdate(pContext);
    }

LExit:
    return nResult;
}

static INT SendProgressUpdate(
    __in BURN_MSI_EXECUTE_CONTEXT* pContext
    )
{
    DWORD dwPercentage = 0; // number representing 0 - 100%
    int nResult = IDNOACTION;
    BURN_MSI_EXECUTE_MESSAGE message = { };

    //DWORD dwMsiProgressTotal = pEngineInfo->dwMsiProgressTotal;
    //DWORD dwMsiProgressComplete = pEngineInfo->dwMsiProgressComplete; //min(dwMsiProgressTotal, pEngineInfo->dwMsiProgressComplete);
    //double dProgressGauge = 0;
    //double dProgressStageTotal = (double)pEngineInfo->qwProgressStageTotal;

    // Calculate progress for the phases of Windows Installer.
    // TODO: handle upgrade progress which would add another phase.
    dwPercentage += CalculatePhaseProgress(pContext, 0, 15);
    dwPercentage += CalculatePhaseProgress(pContext, 1, 80);
    dwPercentage += CalculatePhaseProgress(pContext, 2, 5);

    //if (qwTotal) // avoid "divide by zero" if the MSI range is blank.
    //{
    //    // calculate gauge.
    //    double dProgressGauge = static_cast<double>(qwCompleted) / static_cast<double>(qwTotal);
    //    dProgressGauge = (1.0 / (1.0 + exp(3.7 - dProgressGauge * 7.5)) - 0.024127021417669196) / 0.975872978582330804;
    //    qwCompleted = (DWORD)(dProgressGauge * qwTotal);

    //    // calculate progress within range
    //    //qwProgressComplete = (DWORD64)(dwMsiProgressComplete * (dProgressStageTotal / dwMsiProgressTotal));
    //    //qwProgressComplete = min(qwProgressComplete, pEngineInfo->qwProgressStageTotal);
    //}

    dwPercentage = min(dwPercentage, 100);
    //nResult = m_pView->OnExecuteProgress(m_pExecutingPackage, dwPercentage);
    message.type = BURN_MSI_EXECUTE_MESSAGE_PROGRESS;
    message.progress.dwPercentage = dwPercentage;
    nResult = pContext->pfnMessageHandler(pContext->pvContext, &message);

#ifdef _DEBUG
DWORD64 qwCompleted = pContext->rgMsiProgress[pContext->dwCurrentProgressIndex].dwCompleted;
DWORD64 qwTotal = pContext->rgMsiProgress[pContext->dwCurrentProgressIndex].dwTotal;
Trace3(REPORT_STANDARD, "progress: %I64u/%I64u (%u%%)", qwCompleted, qwTotal, dwPercentage);
//AssertSz(qwCompleted <= qwTotal, "Completed progress is larger than total progress.");
#endif

    return nResult;
}

static void ResetProgress(
    __in BURN_MSI_EXECUTE_CONTEXT* pContext
    )
{
    memset(pContext->rgMsiProgress, 0, sizeof(pContext->rgMsiProgress));
    pContext->dwCurrentProgressIndex = BURN_MSI_PROGRESS_INVALID;
}

static DWORD CalculatePhaseProgress(
    __in BURN_MSI_EXECUTE_CONTEXT* pContext,
    __in DWORD dwProgressIndex,
    __in DWORD dwWeightPercentage
    )
{
    DWORD dwPhasePercentage = 0;

    // If we've already passed this progress index, return the maximum percentage possible (the weight)
    if (dwProgressIndex < pContext->dwCurrentProgressIndex)
    {
        dwPhasePercentage = dwWeightPercentage;
    }
    else if (dwProgressIndex == pContext->dwCurrentProgressIndex) // have to do the math for the current progress.
    {
        BURN_MSI_PROGRESS* pProgress = pContext->rgMsiProgress + dwProgressIndex;
        if (pProgress->dwTotal)
        {
            DWORD64 dw64Completed = pContext->fRollback ? pProgress->dwTotal - pProgress->dwCompleted : pProgress->dwCompleted;
            dwPhasePercentage = static_cast<DWORD>(dw64Completed * dwWeightPercentage / pProgress->dwTotal);
        }
    }
    // else we're not there yet so it has to be zero.

    return dwPhasePercentage;
}

static INT HandleFilesInUseRecord(
    __in BURN_MSI_EXECUTE_CONTEXT* pContext,
    __in MSIHANDLE hRecord
    )
{
    HRESULT hr = S_OK;
    DWORD er = ERROR_SUCCESS;
    int nResult = IDOK;
    DWORD cFiles = 0;
    LPWSTR* rgwzFiles = NULL;
    DWORD cch = 0;
    BURN_MSI_EXECUTE_MESSAGE message = { };

    cFiles = ::MsiRecordGetFieldCount(hRecord);

    rgwzFiles = (LPWSTR*)MemAlloc(sizeof(LPWSTR*) * cFiles, TRUE);
    ExitOnNull(rgwzFiles, hr, E_OUTOFMEMORY, "Failed to allocate buffer.");

    for (DWORD i = 0; i < cFiles; ++i)
    {
        // get string from record
#pragma prefast(push)
#pragma prefast(disable:6298)
        er = ::MsiRecordGetStringW(hRecord, i + 1, L"", &cch);
#pragma prefast(pop)
        if (ERROR_MORE_DATA == er)
        {
            hr = StrAlloc(&rgwzFiles[i], ++cch);
            ExitOnFailure(hr, "Failed to allocate string buffer.");

            er = ::MsiRecordGetStringW(hRecord, i + 1, rgwzFiles[i], &cch);
        }
        ExitOnWin32Error1(er, hr, "Failed to get record field as string: %u", i);
    }

    //nResult = m_pBurnView->OnExecuteMsiFilesInUse(m_wzPackageId, cFiles, (LPCWSTR*)rgwzFiles);
    message.type = BURN_MSI_EXECUTE_MESSAGE_PROGRESS;
    message.msiFilesInUse.cFiles = cFiles;
    message.msiFilesInUse.rgwzFiles = (LPCWSTR*)rgwzFiles;
    nResult = pContext->pfnMessageHandler(pContext->pvContext, &message);

LExit:
    if (rgwzFiles)
    {
        for (DWORD i = 0; i <= cFiles; ++i)
        {
            ReleaseStr(rgwzFiles[i]);
        }
        MemFree(rgwzFiles);
    }

    return nResult;
}

static HRESULT ConcatProperties(
    __in BURN_PACKAGE* pPackage,
    __in BURN_VARIABLES* pVariables,
    __in BOOL fRollback,
    __deref_out_z LPWSTR* psczProperties
    )
{
    HRESULT hr = S_OK;
    LPWSTR sczValue = NULL;
    LPWSTR sczEscapedValue = NULL;
    LPWSTR sczProperty = NULL;

    for (DWORD i = 0; i < pPackage->Msi.cProperties; ++i)
    {
        BURN_MSIPROPERTY* pProperty = &pPackage->Msi.rgProperties[i];

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
        ++wzTarget;
        if (L'\"' == *wzTarget)
        {
            *wzTarget = L'\"';
            ++wzTarget;
        }
        ++wzSource;
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
