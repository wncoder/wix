//-------------------------------------------------------------------------------------------------
// <copyright file="dependency.cpp" company="Microsoft">
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
//    Dependency functions for Burn.
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"

// constants

static LPCWSTR vcszIgnoreDependencies = L"IGNOREDEPENDENCIES";


// functions

extern "C" HRESULT DependencyParseProvidersFromXml(
    __in BURN_PACKAGE* pPackage,
    __in IXMLDOMNode* pixnPackage
    )
{
    HRESULT hr = S_OK;
    IXMLDOMNodeList* pixnNodes = NULL;
    DWORD cNodes = 0;
    IXMLDOMNode* pixnNode = NULL;

    // Select dependency provider nodes.
    hr = XmlSelectNodes(pixnPackage, L"Provides", &pixnNodes);
    ExitOnFailure(hr, "Failed to select dependency provider nodes.");

    // Get dependency provider node count.
    hr = pixnNodes->get_length((long*)&cNodes);
    ExitOnFailure(hr, "Failed to get the dependency provider node count.");

    if (!cNodes)
    {
        ExitFunction1(hr = S_OK);
    }

    // Allocate memory for dependency provider pointers.
    pPackage->rgDependencyProviders = (BURN_DEPENDENCY_PROVIDER*)MemAlloc(sizeof(BURN_DEPENDENCY_PROVIDER) * cNodes, TRUE);
    ExitOnNull(pPackage->rgDependencyProviders, hr, E_OUTOFMEMORY, "Failed to allocate memory for dependency providers.");

    pPackage->cDependencyProviders = cNodes;
    pPackage->fDependencyProvidersImported = FALSE;

    // Parse dependency provider elements.
    for (DWORD i = 0; i < cNodes; i++)
    {
        BURN_DEPENDENCY_PROVIDER* pDependencyProvider = &pPackage->rgDependencyProviders[i];

        hr = XmlNextElement(pixnNodes, &pixnNode, NULL);
        ExitOnFailure(hr, "Failed to get the next dependency provider node.");

        // @Key
        hr = XmlGetAttributeEx(pixnNode, L"Key", &pDependencyProvider->sczKey);
        ExitOnFailure(hr, "Failed to get the Key attribute.");

        // @Imported
        hr = XmlGetYesNoAttribute(pixnNode, L"Imported", &pDependencyProvider->fImported);
        if (E_NOTFOUND != hr)
        {
            ExitOnFailure(hr, "Failed to get the Imported attribute.");
        }
        else
        {
            pDependencyProvider->fImported = FALSE;
            hr = S_OK;
        }

        // Set whether any dependency provider was imported.
        pPackage->fDependencyProvidersImported |= pDependencyProvider->fImported;

        // Prepare next iteration.
        ReleaseNullObject(pixnNode);
    }

    hr = S_OK;

LExit:
    ReleaseObject(pixnNode);
    ReleaseObject(pixnNodes);

    return hr;
}

extern "C" HRESULT DependencyPlanPackage(
    __in BURN_PACKAGE* pPackage,
    __in BURN_PLAN* pPlan,
    __in_z LPCWSTR wzBundleProviderKey,
    __in BOOTSTRAPPER_ACTION_STATE executeAction,
    __in BOOTSTRAPPER_ACTION_STATE rollbackAction,
    __out BURN_DEPENDENCY_ACTION* pDependencyAction
    )
{
    HRESULT hr = S_OK;
    BURN_DEPENDENCY_ACTION dependencyExecuteAction = BURN_DEPENDENCY_ACTION_NONE;
    BURN_DEPENDENCY_ACTION dependencyRollbackAction = BURN_DEPENDENCY_ACTION_NONE;
    BURN_EXECUTE_ACTION *pAction = NULL;

    // Set the dependency registration action based on current and requested state.
    switch (pPackage->currentState)
    {
    case BOOTSTRAPPER_PACKAGE_STATE_ABSENT: __fallthrough;
    case BOOTSTRAPPER_PACKAGE_STATE_SUPERSEDED: __fallthrough;
    case BOOTSTRAPPER_PACKAGE_STATE_OBSOLETE:
        switch (executeAction)
        {
        case BOOTSTRAPPER_ACTION_STATE_INSTALL: __fallthrough;
        case BOOTSTRAPPER_ACTION_STATE_REPAIR: __fallthrough;
        case BOOTSTRAPPER_ACTION_STATE_MODIFY: __fallthrough;
        case BOOTSTRAPPER_ACTION_STATE_MINOR_UPGRADE: __fallthrough;
        case BOOTSTRAPPER_ACTION_STATE_MAJOR_UPGRADE: __fallthrough;
        case BOOTSTRAPPER_ACTION_STATE_PATCH:
            dependencyExecuteAction = BURN_DEPENDENCY_ACTION_REGISTER;
            break;

        // Make sure bundle dependency registration is cleaned up.
        case BOOTSTRAPPER_ACTION_STATE_NONE: __fallthrough;
        case BOOTSTRAPPER_ACTION_STATE_UNINSTALL:
            dependencyExecuteAction = BURN_DEPENDENCY_ACTION_UNREGISTER;
            break;
        }

        switch (rollbackAction)
        {
        case BOOTSTRAPPER_ACTION_STATE_UNINSTALL:
            dependencyRollbackAction = BURN_DEPENDENCY_ACTION_UNREGISTER;
            break;
        }
        break;
    case BOOTSTRAPPER_PACKAGE_STATE_PRESENT:
        switch (executeAction)
        {
        case BOOTSTRAPPER_ACTION_STATE_NONE: __fallthrough;
        case BOOTSTRAPPER_ACTION_STATE_INSTALL: __fallthrough;
        case BOOTSTRAPPER_ACTION_STATE_REPAIR: __fallthrough;
        case BOOTSTRAPPER_ACTION_STATE_MODIFY: __fallthrough;
        case BOOTSTRAPPER_ACTION_STATE_MINOR_UPGRADE: __fallthrough;
        case BOOTSTRAPPER_ACTION_STATE_MAJOR_UPGRADE: __fallthrough;
        case BOOTSTRAPPER_ACTION_STATE_PATCH:
            dependencyExecuteAction = BURN_DEPENDENCY_ACTION_REGISTER;
            break;

        case BOOTSTRAPPER_ACTION_STATE_UNINSTALL:
            dependencyExecuteAction = BURN_DEPENDENCY_ACTION_UNREGISTER;
            break;
        }

        switch (rollbackAction)
        {
        case BOOTSTRAPPER_ACTION_STATE_UNINSTALL:
            dependencyRollbackAction = BURN_DEPENDENCY_ACTION_UNREGISTER;
            break;

        case BOOTSTRAPPER_ACTION_STATE_INSTALL:
            dependencyRollbackAction = BURN_DEPENDENCY_ACTION_REGISTER;
            break;
        }
        break;
    }

    // Add the execute plan.
    if (BURN_DEPENDENCY_ACTION_NONE != dependencyExecuteAction)
    {
        hr = PlanAppendExecuteAction(pPlan, &pAction);
        ExitOnFailure(hr, "Failed to append execute action.");

        pAction->type = BURN_EXECUTE_ACTION_TYPE_DEPENDENCY;
        pAction->dependency.pPackage = pPackage;
        pAction->dependency.action = dependencyExecuteAction;

        hr = StrAllocString(&pAction->dependency.sczBundleProviderKey, wzBundleProviderKey, 0);
        ExitOnFailure(hr, "Failed to copy the bundle dependency key.");
    }

    // Add the rollback plan.
    if (BURN_DEPENDENCY_ACTION_NONE != dependencyRollbackAction)
    {
        hr = PlanAppendRollbackAction(pPlan, &pAction);
        ExitOnFailure(hr, "Failed to append rollback action.");

        pAction->type = BURN_EXECUTE_ACTION_TYPE_DEPENDENCY;
        pAction->dependency.pPackage = pPackage;
        pAction->dependency.action = dependencyRollbackAction;

        hr = StrAllocString(&pAction->dependency.sczBundleProviderKey, wzBundleProviderKey, 0);
        ExitOnFailure(hr, "Failed to copy the bundle dependency key.");
    }

    // Add the checkpoint.
    if (BOOTSTRAPPER_ACTION_STATE_NONE != executeAction || BOOTSTRAPPER_ACTION_STATE_NONE != rollbackAction)
    {
        hr = PlanExecuteCheckpoint(pPlan);
        ExitOnFailure(hr, "Failed to append execute checkpoint.");
    }

    // Return the dependency execution action.
    *pDependencyAction = dependencyExecuteAction;

LExit:
    return hr;
}

extern "C" HRESULT DependencyExecuteAction(
    __in BURN_EXECUTE_ACTION* pAction,
    __in BOOL fPerMachine
    )
{
    AssertSz(BURN_EXECUTE_ACTION_TYPE_DEPENDENCY == pAction->type, "Execute action type not supported by this function.");

    HRESULT hr = S_OK;

    // Register or unregister the bundle as a dependent of each package dependency provider.
    if (BURN_DEPENDENCY_ACTION_REGISTER == pAction->dependency.action)
    {
        hr = DependencyRegisterDependent(pAction->dependency.sczBundleProviderKey, fPerMachine, pAction->dependency.pPackage);
        ExitOnFailure(hr, "Failed to register the bundle as a dependent of the package.");
    }
    else if (BURN_DEPENDENCY_ACTION_UNREGISTER == pAction->dependency.action)
    {
        hr = DependencyUnregisterDependent(pAction->dependency.sczBundleProviderKey, fPerMachine, pAction->dependency.pPackage);
        ExitOnFailure(hr, "Failed to unregister the bundle as a dependent of the package.");
    }

LExit:
    return hr;
}

extern "C" HRESULT DependencyRegister(
    __in const BURN_REGISTRATION* pRegistration
    )
{
    HRESULT hr = S_OK;
    LPWSTR sczVersion = NULL;

    hr = FileVersionToStringEx(pRegistration->qwVersion, &sczVersion);
    ExitOnFailure(hr, "Failed to format the registration version string.");

    LogId(REPORT_VERBOSE, MSG_DEPENDENCY_BUNDLE_REGISTER, pRegistration->sczProviderKey, sczVersion);

    // Register the bundle provider key.
    hr = DepRegisterDependency(pRegistration->hkRoot, pRegistration->sczProviderKey, pRegistration->sczId, sczVersion, 0);
    ExitOnFailure(hr, "Failed to register the bundle dependency provider key.");

LExit:
    ReleaseStr(sczVersion);

    return hr;
}

extern "C" HRESULT DependencyRegisterDependent(
    __in_z LPCWSTR wzBundleProviderKey,
    __in BOOL fPerMachine,
    __in const BURN_PACKAGE* pPackage
    )
{
    HRESULT hr = S_OK;
    HKEY hkRoot = fPerMachine ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER;

    // Do not register Burn as a dependent of packages in a different install context.
    if (fPerMachine != pPackage->fPerMachine)
    {
        LogId(REPORT_STANDARD, MSG_DEPENDENCY_PACKAGE_SKIP_WRONGSCOPE, pPackage->sczId, LoggingPerMachineToString(fPerMachine), LoggingPerMachineToString(pPackage->fPerMachine));
        ExitFunction1(hr = S_OK);
    }

    // Loop through each package provider and remove the bundle dependency key.
    if (pPackage->rgDependencyProviders)
    {
        LogId(REPORT_VERBOSE, MSG_DEPENDENCY_PACKAGE_REGISTER, pPackage->sczId);

        for (DWORD i = 0; i < pPackage->cDependencyProviders; ++i)
        {
            hr = DepRegisterDependent(hkRoot, pPackage->rgDependencyProviders[i].sczKey, wzBundleProviderKey, NULL, NULL, 0);
            ExitOnFailure(hr, "Failed to register the bundle dependency key from a package dependency provider.");
        }
    }

LExit:
    return hr;
}

extern "C" HRESULT DependencyUnregister(
    __in const BURN_REGISTRATION* pRegistration
    )
{
    HRESULT hr = S_OK;

    LogId(REPORT_VERBOSE, MSG_DEPENDENCY_BUNDLE_UNREGISTER, pRegistration->sczProviderKey);

    // Remove the bundle provider key.
    hr = DepUnregisterDependency(pRegistration->hkRoot, pRegistration->sczProviderKey);
    if (E_FILENOTFOUND != hr)
    {
        ExitOnFailure(hr, "Failed to remove the bundle dependency provider key.");
    }
    else
    {
        hr = S_OK;
    }

LExit:
    return hr;
}

extern "C" HRESULT DependencyUnregisterDependent(
    __in_z LPCWSTR wzBundleProviderKey,
    __in BOOL fPerMachine,
    __in const BURN_PACKAGE* pPackage
    )
{
    HRESULT hr = S_OK;
    HKEY hkRoot = fPerMachine ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER;

    // Should be no registration to remove since we don't write keys across contexts.
    if (fPerMachine != pPackage->fPerMachine)
    {
        LogId(REPORT_STANDARD, MSG_DEPENDENCY_PACKAGE_SKIP_WRONGSCOPE, pPackage->sczId, LoggingPerMachineToString(fPerMachine), LoggingPerMachineToString(pPackage->fPerMachine));
        ExitFunction1(hr = S_OK);
    }

    // Loop through each package provider and remove the bundle dependency key.
    if (pPackage->rgDependencyProviders)
    {
        LogId(REPORT_VERBOSE, MSG_DEPENDENCY_PACKAGE_UNREGISTER, pPackage->sczId);

        for (DWORD i = 0; i < pPackage->cDependencyProviders; ++i)
        {
            hr = DepUnregisterDependent(hkRoot, pPackage->rgDependencyProviders[i].sczKey, wzBundleProviderKey);
            if (E_FILENOTFOUND != hr)
            {
                ExitOnFailure(hr, "Failed to remove the bundle dependency key from a package dependency provider.");
            }
            else
            {
                hr = S_OK;
            }
        }
    }

LExit:
    return hr;
}
