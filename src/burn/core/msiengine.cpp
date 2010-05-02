//-------------------------------------------------------------------------------------------------
// <copyright file="msiengine.cpp" company="Microsoft">
//    Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//
// <summary>
//    Module: MSI Engine
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"


// internal function declarations

static HRESULT EvaluateActionStateConditions(
    __in BURN_VARIABLES* pVariables,
    __in_z_opt LPCWSTR sczAddLocalCondition,
    __in_z_opt LPCWSTR sczAddSourceCondition,
    __in_z_opt LPCWSTR sczAdvertiseCondition,
    __out BURN_MSIFEATURE_STATE* pState
    );
static HRESULT ConcatProperties(
    __in BURN_PACKAGE* pPackage,
    __in BURN_VARIABLES* pVariables,
    __in BOOL fRollback,
    __inout_z LPWSTR* psczProperties
    );
static HRESULT EscapePropertyArgumentString(
    __in LPCWSTR wzProperty,
    __inout_z LPWSTR* psczEscapedValue
    );
static HRESULT ConcatFeatureActionProperties(
    __in BURN_PACKAGE* pPackage,
    __in BOOL fRollback,
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

    // @ProductCode
    hr = XmlGetAttributeEx(pixnMsiPackage, L"ProductCode", &pPackage->Msi.sczProductCode);
    ExitOnFailure(hr, "Failed to get @ProductCode.");

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

        // parse package elements
        for (DWORD i = 0; i < cNodes; ++i)
        {
            BURN_MSIFEATURE* pFeature = &pPackage->Msi.rgFeatures[i];

            hr = XmlNextElement(pixnNodes, &pixnNode, NULL);
            ExitOnFailure(hr, "Failed to get next node.");

            // @Id
            hr = XmlGetAttributeEx(pixnNode, L"Id", &pFeature->sczId);
            ExitOnFailure(hr, "Failed to get Id attribute.");

            // @AddLocalCondition
            hr = XmlGetAttributeEx(pixnNode, L"AddLocalCondition", &pFeature->sczAddLocalCondition);
            ExitOnFailure(hr, "Failed to get AddLocalCondition attribute.");

            // @AddSourceCondition
            hr = XmlGetAttributeEx(pixnNode, L"AddSourceCondition", &pFeature->sczAddSourceCondition);
            ExitOnFailure(hr, "Failed to get AddSourceCondition attribute.");

            // @AdvertiseCondition
            hr = XmlGetAttributeEx(pixnNode, L"AdvertiseCondition", &pFeature->sczAdvertiseCondition);
            ExitOnFailure(hr, "Failed to get AdvertiseCondition attribute.");

            // @RollbackAddLocalCondition
            hr = XmlGetAttributeEx(pixnNode, L"RollbackAddLocalCondition", &pFeature->sczRollbackAddLocalCondition);
            ExitOnFailure(hr, "Failed to get RollbackAddLocalCondition attribute.");

            // @RollbackAddSourceCondition
            hr = XmlGetAttributeEx(pixnNode, L"RollbackAddSourceCondition", &pFeature->sczRollbackAddSourceCondition);
            ExitOnFailure(hr, "Failed to get RollbackAddSourceCondition attribute.");

            // @RollbackAdvertiseCondition
            hr = XmlGetAttributeEx(pixnNode, L"RollbackAdvertiseCondition", &pFeature->sczRollbackAdvertiseCondition);
            ExitOnFailure(hr, "Failed to get RollbackAdvertiseCondition attribute.");

            // prepare next iteration
            ReleaseNullObject(pixnNode);
        }
    }

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

        // parse package elements
        for (DWORD i = 0; i < cNodes; ++i)
        {
            BURN_MSIPROPERTY* pProperty = &pPackage->Msi.rgProperties[i];

            hr = XmlNextElement(pixnNodes, &pixnNode, NULL);
            ExitOnFailure(hr, "Failed to get next node.");

            // @Id
            hr = XmlGetAttributeEx(pixnNode, L"Id", &pProperty->sczId);
            ExitOnFailure(hr, "Failed to get Id attribute.");

            // @Value
            hr = XmlGetAttributeEx(pixnNode, L"Value", &pProperty->sczValue);
            ExitOnFailure(hr, "Failed to get Value attribute.");

            // @RollbackValue
            hr = XmlGetAttributeEx(pixnNode, L"RollbackValue", &pProperty->sczRollbackValue);
            ExitOnFailure(hr, "Failed to get RollbackValue attribute.");

            // prepare next iteration
            ReleaseNullObject(pixnNode);
        }
    }

    hr = S_OK;

LExit:
    ReleaseObject(pixnNodes);
    ReleaseObject(pixnNode);

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

    // clear struct
    memset(&pPackage->Msi, 0, sizeof(pPackage->Msi));
}

extern "C" HRESULT MsiEngineDetectPackage(
    __in BURN_PACKAGE* pPackage
    )
{
    Trace1(REPORT_STANDARD, "Detecting MSI package 0x%p", pPackage);

    HRESULT hr = S_OK;
    DWORD er = ERROR_SUCCESS;
    WCHAR wzInstalledVersion[24] = { };
    DWORD cchInstalledVersion = countof(wzInstalledVersion);
    INSTALLSTATE installState = INSTALLSTATE_UNKNOWN;

    // TODO: what to do about MSIINSTALLCONTEXT_USERMANAGED?
    er = vpfnMsiGetProductInfoExW(pPackage->Msi.sczProductCode, NULL, pPackage->fPerMachine ? MSIINSTALLCONTEXT_MACHINE : MSIINSTALLCONTEXT_USERUNMANAGED, INSTALLPROPERTY_VERSIONSTRING, wzInstalledVersion, &cchInstalledVersion);
    if (ERROR_SUCCESS == er)
    {
        hr = FileVersionFromStringEx(wzInstalledVersion, 0, &pPackage->Msi.qwInstalledVersion);
        ExitOnFailure2(hr, "Failed to convert version: %S to DWORDs for ProductCode: %S", wzInstalledVersion, pPackage->Msi.sczProductCode);

        pPackage->currentState = PACKAGE_STATE_PRESENT;
    }
    else if (ERROR_UNKNOWN_PRODUCT == er || ERROR_UNKNOWN_PROPERTY == er) // package not present.
    {
        pPackage->currentState = PACKAGE_STATE_ABSENT;
    }
    else
    {
        ExitOnWin32Error1(er, hr, "Failed to get product information for ProductCode: %S", pPackage->Msi.sczProductCode);
    }

    // detect features
    for (DWORD i = 0; i < pPackage->Msi.cFeatures; ++i)
    {
        BURN_MSIFEATURE* pFeature = &pPackage->Msi.rgFeatures[i];

        // get current state
        if (PACKAGE_STATE_PRESENT == pPackage->currentState) // only try to detect features if the product is installed
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
            pFeature->currentState = BURN_MSIFEATURE_STATE_ABSENT;
            break;
        case INSTALLSTATE_ADVERTISED:
            pFeature->currentState = BURN_MSIFEATURE_STATE_ADVERTISED;
            break;
        case INSTALLSTATE_LOCAL:
            pFeature->currentState = BURN_MSIFEATURE_STATE_LOCAL;
            break;
        case INSTALLSTATE_SOURCE:
            pFeature->currentState = BURN_MSIFEATURE_STATE_SOURCE;
            break;
        default:
            hr = E_UNEXPECTED;
            ExitOnRootFailure(hr, "Invalid state value.");
        } 
    }

LExit:
    return hr;
}

//
// Plan - calculates the execute and rollback state for the requested package state.
//
extern "C" HRESULT MsiEnginePlanPackage(
    __in BURN_PACKAGE* pPackage,
    __in BURN_VARIABLES* pVariables
    )
{
    Trace1(REPORT_STANDARD, "Planning MSI package 0x%p", pPackage);

    HRESULT hr = S_OK;
    DWORD64 qwVersion = pPackage->Msi.qwVersion;
    DWORD64 qwInstalledVersion = pPackage->Msi.qwInstalledVersion;
    PACKAGE_STATE state = pPackage->currentState;
    PACKAGE_STATE expected = PACKAGE_STATE_UNKNOWN;
    REQUEST_STATE requested = REQUEST_STATE_NONE;
    ACTION_STATE execute = ACTION_STATE_NONE;
    ACTION_STATE rollback = ACTION_STATE_NONE;
    BURN_MSIFEATURE_STATE featureRequestedState = BURN_MSIFEATURE_STATE_UNKNOWN;
    BURN_MSIFEATURE_STATE featureExpectedState = BURN_MSIFEATURE_STATE_UNKNOWN;
    BOOL fHasRequestedFeature = FALSE;
    BOOL fHasExpectedFeature = FALSE;
    BOOL f = FALSE;

    // plan features
    for (DWORD i = 0; i < pPackage->Msi.cFeatures; ++i)
    {
        BURN_MSIFEATURE* pFeature = &pPackage->Msi.rgFeatures[i];

        hr = EvaluateActionStateConditions(pVariables, pFeature->sczAddLocalCondition, pFeature->sczAddSourceCondition, pFeature->sczAdvertiseCondition, &featureRequestedState);
        ExitOnFailure(hr, "Failed to evaluate requested state conditions.");

        if (BURN_MSIFEATURE_STATE_UNKNOWN != featureRequestedState && BURN_MSIFEATURE_STATE_ABSENT != featureRequestedState)
        {
            fHasRequestedFeature = TRUE;
        }

        hr = EvaluateActionStateConditions(pVariables, pFeature->sczRollbackAddLocalCondition, pFeature->sczRollbackAddSourceCondition, pFeature->sczRollbackAdvertiseCondition, &featureExpectedState);
        ExitOnFailure(hr, "Failed to evaluate expected state conditions.");

        if (BURN_MSIFEATURE_STATE_UNKNOWN != featureExpectedState && BURN_MSIFEATURE_STATE_ABSENT != featureExpectedState)
        {
            fHasExpectedFeature = TRUE;
        }

        // set action state
        pFeature->executeAction = BURN_MSIFEATURE_ACTION_NONE;
        switch (featureRequestedState)
        {
        case BURN_MSIFEATURE_STATE_UNKNOWN:
            pFeature->executeAction = BURN_MSIFEATURE_ACTION_NONE;
            break;
        case BURN_MSIFEATURE_STATE_ABSENT:
            if (BURN_MSIFEATURE_STATE_ABSENT != pFeature->currentState)
            {
                pFeature->executeAction = BURN_MSIFEATURE_ACTION_REMOVE;
            }
            break;
        case BURN_MSIFEATURE_STATE_ADVERTISED:
            if (BURN_MSIFEATURE_STATE_ADVERTISED != pFeature->currentState)
            {
                pFeature->executeAction = BURN_MSIFEATURE_ACTION_ADVERTISE;
            }
            else if (pFeature->fRepair)
            {
                pFeature->executeAction = BURN_MSIFEATURE_ACTION_REINSTALL;
            }
            break;
        case BURN_MSIFEATURE_STATE_LOCAL:
            if (BURN_MSIFEATURE_STATE_LOCAL != pFeature->currentState)
            {
                pFeature->executeAction = BURN_MSIFEATURE_ACTION_ADDLOCAL;
            }
            else if (pFeature->fRepair)
            {
                pFeature->executeAction = BURN_MSIFEATURE_ACTION_REINSTALL;
            }
            break;
        case BURN_MSIFEATURE_STATE_SOURCE:
            if (BURN_MSIFEATURE_STATE_SOURCE != pFeature->currentState)
            {
                pFeature->executeAction = BURN_MSIFEATURE_ACTION_ADDSOURCE;
            }
            else if (pFeature->fRepair)
            {
                pFeature->executeAction = BURN_MSIFEATURE_ACTION_REINSTALL;
            }
            break;
        default:
            hr = E_UNEXPECTED;
            ExitOnRootFailure(hr, "Invalid state value.");
        }

        // rollback action
        if (BURN_MSIFEATURE_ACTION_REINSTALL == pFeature->executeAction)
        {
            pFeature->rollbackAction = BURN_MSIFEATURE_ACTION_REINSTALL;
        }
        else if (BURN_MSIFEATURE_ACTION_NONE != pFeature->executeAction)
        {
            // if we have an expected state, rollback to the expected state else rollback to actuall state
            switch (BURN_MSIFEATURE_STATE_UNKNOWN != featureExpectedState ? featureExpectedState : pFeature->currentState)
            {
            case BURN_MSIFEATURE_STATE_ABSENT:
                pFeature->rollbackAction = BURN_MSIFEATURE_ACTION_REMOVE;
                break;
            case BURN_MSIFEATURE_STATE_ADVERTISED:
                pFeature->rollbackAction = BURN_MSIFEATURE_ACTION_ADVERTISE;
                break;
            case BURN_MSIFEATURE_STATE_LOCAL:
                pFeature->rollbackAction = BURN_MSIFEATURE_ACTION_ADDLOCAL;
                break;
            case BURN_MSIFEATURE_STATE_SOURCE:
                pFeature->rollbackAction = BURN_MSIFEATURE_ACTION_ADDSOURCE;
                break;
            default:
                hr = E_UNEXPECTED;
                ExitOnRootFailure(hr, "Invalid state value.");
            }
        }
        else
        {
            pFeature->rollbackAction = BURN_MSIFEATURE_ACTION_NONE;
        }
    }

    // decide packages requested state
    if (fHasRequestedFeature)
    {
        requested = REQUEST_STATE_PRESENT;
    }
    else if (pPackage->sczInstallCondition)
    {
        // evaluate install condition
        hr = ConditionEvaluate(pVariables, pPackage->sczInstallCondition, &f);
        ExitOnFailure(hr, "Failed to evaluate install condition.");

        requested = f ? REQUEST_STATE_PRESENT : REQUEST_STATE_ABSENT;
    }

    // decide packages expected state
    if (fHasExpectedFeature)
    {
        expected = PACKAGE_STATE_PRESENT;
    }
    else if (pPackage->sczRollbackInstallCondition)
    {
        // evaluate rollback install condition
        hr = ConditionEvaluate(pVariables, pPackage->sczRollbackInstallCondition, &f);
        ExitOnFailure(hr, "Failed to evaluate rollback install condition.");

        expected = f ? PACKAGE_STATE_PRESENT : PACKAGE_STATE_ABSENT;
    }

    // execute action
    if (PACKAGE_STATE_PRESENT == state)
    {
        if (REQUEST_STATE_PRESENT == requested || REQUEST_STATE_REPAIR == requested)
        {
            //
            // Take a look at the version and determine if this is a potential
            // minor upgrade, a major upgrade (just install) otherwise, there
            // is a newer version so no work necessary.
            //
            // minor upgrade "10.3.2.0" > "10.3.1.0" or "10.3.2.1" > "10.3.2.0"
            if ((qwVersion >> 32) == (qwInstalledVersion >> 32) && qwVersion > qwInstalledVersion)
            {
                execute = ACTION_STATE_MINOR_UPGRADE;
            }
            else if (qwVersion > qwInstalledVersion) // major upgrade "11.X.X.X" > "10.X.X.X"
            {
                execute = ACTION_STATE_MAJOR_UPGRADE;
            }
            else if (qwVersion == qwInstalledVersion) // maintenance install "10.X.X.X" = "10.X.X.X"
            {
                execute = (REQUEST_STATE_REPAIR == requested) ? ACTION_STATE_RECACHE : ACTION_STATE_MAINTENANCE;
            }
            else // newer version present "14.X.X.X" < "15.X.X.X", skip
            {
                execute = ACTION_STATE_NONE; // TODO: inform about this state to enable warnings
            }
        }
        else if (PACKAGE_STATE_ABSENT == requested)
        {
            if (!pPackage->fUninstallable) // do not uninstall packages that we don't own.
            {
                execute = ACTION_STATE_NONE;
            }
            else if (qwVersion == qwInstalledVersion) // install "10.X.X.X" = "10.X.X.X"
            {
                execute = ACTION_STATE_UNINSTALL;
            }
            else
            {
                execute = ACTION_STATE_NONE; // TODO: inform about this state to enable warnings
            }
        }
    }
    else // package not installed.
    {
        if (REQUEST_STATE_PRESENT == requested || REQUEST_STATE_REPAIR == requested)
        {
            execute = ACTION_STATE_INSTALL;
        }
        else
        {
            execute = ACTION_STATE_NONE;
        }
    }

    // rollback action
    switch (PACKAGE_STATE_UNKNOWN != expected ? expected : state)
    {
    case PACKAGE_STATE_PRESENT:
        switch (requested)
        {
        case REQUEST_STATE_PRESENT: __fallthrough;
        case REQUEST_STATE_REPAIR:
            rollback = ACTION_STATE_NONE;
            break;
        case REQUEST_STATE_ABSENT:
            rollback = ACTION_STATE_INSTALL;
            break;
        default:
            rollback = ACTION_STATE_NONE;
            break;
        }
        break;
    case PACKAGE_STATE_ABSENT:
        switch (requested)
        {
        case REQUEST_STATE_PRESENT: __fallthrough;
        case REQUEST_STATE_REPAIR:
            rollback = pPackage->fUninstallable ? ACTION_STATE_UNINSTALL : ACTION_STATE_NONE;
            break;
        case REQUEST_STATE_ABSENT:
            rollback = ACTION_STATE_NONE;
            break;
        default:
            rollback = ACTION_STATE_NONE;
            break;
        }
        break;
    default:
        hr = E_INVALIDARG;
        ExitOnRootFailure(hr, "Invalid package detection result encountered.");
    }

    pPackage->executeAction = execute;
    pPackage->rollbackAction = rollback;

LExit:
    return hr;
}

extern "C" HRESULT MsiEngineConfigurePackage(
    __in BURN_PACKAGE* pPackage,
    __in BURN_VARIABLES* pVariables,
    __in BOOL fExecutingRollback
    )
{
    HRESULT hr = S_OK;

    //LPWSTR sczProductCode = NULL;
    //LPWSTR sczMsiPath = NULL;

    //DWORD dwUiLevel = INSTALLUILEVEL_NONE | INSTALLUILEVEL_SOURCERESONLY;
    //DWORD dwMessageFilter = 0;
    //INSTALLUI_HANDLERW pfnPreviousExternalUI = NULL;
    //INSTALLUI_HANDLER_RECORD pfnPreviousExternalUIRecord = NULL;
    //LPWSTR sczLogFile = NULL;
    //BURN_LOGGING_LEVEL logLevel = BURN_LOGGING_LEVEL_UNKNOWN;
    LPWSTR sczProperties = NULL;

    //hr = m_pExecutingPackage->GetProductCode(&sczProductCode);
    //ExitOnFailure(hr, "Failed to get ProductCode.");

    // Always use the first payload as the installation package.
    //m_pExecutingPackage->GetPayload(1, &pPayload);

    //hr = pPayload->GetCompletedPath(&sczMsiPath);
    //ExitOnFailure(hr, "Failed to get completed path for installation payload.");

    /*
    switch (uiLevel)
    {
    case MSI_UI_LEVEL_DEFAULT:
        dwUiLevel = INSTALLUILEVEL_DEFAULT;
        break;
    case MSI_UI_LEVEL_NONE:
        dwUiLevel = INSTALLUILEVEL_NONE;
        if (pEngineInfo->fEnableSourceResPrompt)
        {
            dwUiLevel |= INSTALLUILEVEL_SOURCERESONLY;
        }
        break;
    case MSI_UI_LEVEL_BASIC:
        dwUiLevel = INSTALLUILEVEL_BASIC;
        break;
    case MSI_UI_LEVEL_REDUCED:
        dwUiLevel = INSTALLUILEVEL_REDUCED;
        break;
    case MSI_UI_LEVEL_FULL:
        dwUiLevel = INSTALLUILEVEL_FULL;
        break;
    default:
        ExitFunction1(hr = E_UNEXPECTED);
    }
    */

    // Wire up logging and the external UI handler.
    //m_pInstallationAbstractionLayer->MsiSetInternalUI(static_cast<INSTALLUILEVEL>(dwUiLevel), NULL);

    //dwMessageFilter = INSTALLLOGMODE_INITIALIZE | INSTALLLOGMODE_TERMINATE |
    //                  INSTALLLOGMODE_FATALEXIT | INSTALLLOGMODE_ERROR | INSTALLLOGMODE_WARNING |
    //                  INSTALLLOGMODE_RESOLVESOURCE | INSTALLLOGMODE_OUTOFDISKSPACE |
    //                  INSTALLLOGMODE_ACTIONSTART | INSTALLLOGMODE_ACTIONDATA | INSTALLLOGMODE_COMMONDATA | INSTALLLOGMODE_PROGRESS;

    // If the external UI record is available (MSI version >= 3.1) use it but fall back to the standard external
    // UI handler if necesary.
    //hr = m_pInstallationAbstractionLayer->MsiSetExternalUIRecord(InstallEngineRecordCallback, dwMessageFilter, static_cast<LPVOID>(this), &pfnPreviousExternalUIRecord);
    //if (FAILED(hr))
    //{
    //    hr = m_pInstallationAbstractionLayer->MsiSetExternalUI(InstallEngineCallback, dwMessageFilter, static_cast<LPVOID>(this), &pfnPreviousExternalUI);
    //    ExitOnFailure(hr, "Failed to wire up external UI handler.");
    //}

    // Enable logging based on the manifest settings or global log command-line switch.
    //hr = m_pExecutingPackage->GetLogPath(&sczLogFile, &logLevel);
    //if (SUCCEEDED(hr))
    //{
    //    // default to "normal" logging
    //    DWORD dwLogMode = 
    //        INSTALLLOGMODE_FATALEXIT | INSTALLLOGMODE_ERROR | INSTALLLOGMODE_WARNING |
    //        INSTALLLOGMODE_USER | INSTALLLOGMODE_INFO | INSTALLLOGMODE_RESOLVESOURCE |
    //        INSTALLLOGMODE_OUTOFDISKSPACE | INSTALLLOGMODE_ACTIONSTART | INSTALLLOGMODE_ACTIONDATA |
    //        INSTALLLOGMODE_COMMONDATA | INSTALLLOGMODE_PROPERTYDUMP;
    //    if (BURN_LOGGING_LEVEL_VERBOSE == logLevel)
    //    {
    //        dwLogMode |= INSTALLLOGMODE_VERBOSE;
    //    }
    //    else if (BURN_LOGGING_LEVEL_DEBUG == logLevel)
    //    {
    //        dwLogMode |= INSTALLLOGMODE_VERBOSE | INSTALLLOGMODE_EXTRADEBUG;
    //    }

    //    // always append because InitializeLog truncated the log if overwrite was requested
    //    hr = m_pInstallationAbstractionLayer->MsiEnableLog(dwLogMode, sczLogFile, INSTALLLOGATTRIBUTES_APPEND);
    //    if (SUCCEEDED(hr))
    //    {
    //        LogStringLine(REPORT_STANDARD, L"MSI log file for package '%s' set to: %s", m_pExecutingPackage->Id(), sczLogFile);
    //    }
    //}

    // Ignore all failures setting up the UI handler and
    // logging, they aren't important enough to abort the
    // install attempt.
    hr = S_OK;

    // set up properties
    hr = ConcatProperties(pPackage, pVariables, fExecutingRollback, &sczProperties);
    ExitOnFailure(hr, "Failed to add properties to argument string.");

    // add feature action properties
    hr = ConcatFeatureActionProperties(pPackage, fExecutingRollback, &sczProperties);
    ExitOnFailure(hr, "Failed to add feature action properties to argument string.");

    //
    // Do the actual action.
    //
    switch (fExecutingRollback ? pPackage->rollbackAction : pPackage->executeAction)
    {
    case ACTION_STATE_ADMIN_INSTALL:
        hr = StrAllocConcat(&sczProperties, L" ACTION=ADMIN", 0);
        ExitOnFailure(hr, "Failed to format property: ADMIN");
         __fallthrough;
    case ACTION_STATE_MAJOR_UPGRADE: __fallthrough;
    case ACTION_STATE_INSTALL:
        hr = vpfnMsiInstallProductW(pPackage->sczExecutePath, sczProperties);
        break;

    case ACTION_STATE_MINOR_UPGRADE:
        hr = StrAllocConcat(&sczProperties, L" REINSTALL=ALL REINSTALLMODE=\"vomus\"", 0);
        ExitOnFailure(hr, "failed to format properties: REINSTALL and REINSTALLMODE");

        hr = vpfnMsiInstallProductW(pPackage->sczExecutePath, sczProperties);
        break;

    case ACTION_STATE_MAINTENANCE: __fallthrough;
    case ACTION_STATE_RECACHE:
        hr = StrAllocConcat(&sczProperties, L" REINSTALLMODE=\"vdmus\"", 0);
        ExitOnFailure(hr, "failed to format property: REINSTALLMODE");

        hr = vpfnMsiConfigureProductExW(pPackage->Msi.sczProductCode, INSTALLLEVEL_DEFAULT, INSTALLSTATE_DEFAULT, sczProperties);
        break;

    case ACTION_STATE_UNINSTALL:
        hr = vpfnMsiConfigureProductExW(pPackage->Msi.sczProductCode, INSTALLLEVEL_DEFAULT, INSTALLSTATE_ABSENT, sczProperties);
        break;
    }

LExit:
    //if (pfnPreviousExternalUI)  // unset the UI handler
    //{
    //    m_pInstallationAbstractionLayer->MsiSetExternalUI(pfnPreviousExternalUI, 0, NULL, NULL);
    //}

    //if (pfnPreviousExternalUIRecord)  // unset the UI record handler
    //{
    //    m_pInstallationAbstractionLayer->MsiSetExternalUIRecord(pfnPreviousExternalUIRecord, 0, NULL, NULL);
    //}

    ReleaseStr(sczProperties);
    //ReleaseStr(sczLogFile);

    return hr;
}


// internal helper functions

static HRESULT EvaluateActionStateConditions(
    __in BURN_VARIABLES* pVariables,
    __in_z_opt LPCWSTR sczAddLocalCondition,
    __in_z_opt LPCWSTR sczAddSourceCondition,
    __in_z_opt LPCWSTR sczAdvertiseCondition,
    __out BURN_MSIFEATURE_STATE* pState
    )
{
    HRESULT hr = S_OK;
    BOOL f = FALSE;

    // if no condition was set, return no feature state
    if (!sczAddLocalCondition && !sczAddSourceCondition && !sczAdvertiseCondition)
    {
        *pState = BURN_MSIFEATURE_STATE_UNKNOWN;
        ExitFunction();
    }

    if (sczAddLocalCondition)
    {
        hr = ConditionEvaluate(pVariables, sczAddLocalCondition, &f);
        ExitOnFailure(hr, "Failed to evaluate add local condition.");

        if (f)
        {
            *pState = BURN_MSIFEATURE_STATE_LOCAL;
            ExitFunction();
        }
    }

    if (sczAddSourceCondition)
    {
        hr = ConditionEvaluate(pVariables, sczAddSourceCondition, &f);
        ExitOnFailure(hr, "Failed to evaluate add source condition.");

        if (f)
        {
            *pState = BURN_MSIFEATURE_STATE_SOURCE;
            ExitFunction();
        }
    }

    if (sczAdvertiseCondition)
    {
        hr = ConditionEvaluate(pVariables, sczAdvertiseCondition, &f);
        ExitOnFailure(hr, "Failed to evaluate advertise condition.");

        if (f)
        {
            *pState = BURN_MSIFEATURE_STATE_ADVERTISED;
            ExitFunction();
        }
    }

    // if no condition was true, set to absent
    *pState = BURN_MSIFEATURE_STATE_ABSENT;

LExit:
    return hr;
}

static HRESULT ConcatProperties(
    __in BURN_PACKAGE* pPackage,
    __in BURN_VARIABLES* pVariables,
    __in BOOL fRollback,
    __inout_z LPWSTR* psczProperties
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
        hr = VariableFormatString(pVariables, (fRollback ? pProperty->sczRollbackValue : pProperty->sczValue), &sczValue, NULL);
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
    __in BOOL fRollback,
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

        switch (fRollback ? pFeature->rollbackAction : pFeature->executeAction)
        {
        case BURN_MSIFEATURE_ACTION_ADDLOCAL:
            if (sczAddLocal)
            {
                hr = StrAllocConcat(&sczAddLocal, L",", 0);
                ExitOnFailure(hr, "Failed to concat separator.");
            }
            hr = StrAllocConcat(&sczAddLocal, pFeature->sczId, 0);
            ExitOnFailure(hr, "Failed to concat feature.");
            break;
        case BURN_MSIFEATURE_ACTION_ADDSOURCE:
            if (sczAddSource)
            {
                hr = StrAllocConcat(&sczAddSource, L",", 0);
                ExitOnFailure(hr, "Failed to concat separator.");
            }
            hr = StrAllocConcat(&sczAddSource, pFeature->sczId, 0);
            ExitOnFailure(hr, "Failed to concat feature.");
            break;
        case BURN_MSIFEATURE_ACTION_ADDDEFAULT:
            if (sczAddDefault)
            {
                hr = StrAllocConcat(&sczAddDefault, L",", 0);
                ExitOnFailure(hr, "Failed to concat separator.");
            }
            hr = StrAllocConcat(&sczAddDefault, pFeature->sczId, 0);
            ExitOnFailure(hr, "Failed to concat feature.");
            break;
        case BURN_MSIFEATURE_ACTION_REINSTALL:
            if (sczReinstall)
            {
                hr = StrAllocConcat(&sczReinstall, L",", 0);
                ExitOnFailure(hr, "Failed to concat separator.");
            }
            hr = StrAllocConcat(&sczReinstall, pFeature->sczId, 0);
            ExitOnFailure(hr, "Failed to concat feature.");
            break;
        case BURN_MSIFEATURE_ACTION_ADVERTISE:
            if (sczAdvertise)
            {
                hr = StrAllocConcat(&sczAdvertise, L",", 0);
                ExitOnFailure(hr, "Failed to concat separator.");
            }
            hr = StrAllocConcat(&sczAdvertise, pFeature->sczId, 0);
            ExitOnFailure(hr, "Failed to concat feature.");
            break;
        case BURN_MSIFEATURE_ACTION_REMOVE:
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
