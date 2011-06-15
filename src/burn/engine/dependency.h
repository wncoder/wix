//-------------------------------------------------------------------------------------------------
// <copyright file="dependency.h" company="Microsoft">
//    Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//
// <summary>
//    Dependency functions for Burn.
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once

#if defined(__cplusplus)
extern "C" {
#endif

// function declarations

/********************************************************************
 DependencyParseProvidersFromXml - parses dependency information
  from the manifest for the specified package.

*********************************************************************/
HRESULT DependencyParseProvidersFromXml(
    __in BURN_PACKAGE* pPackage,
    __in IXMLDOMNode* pixnPackage
    );

/********************************************************************
 DependencyPlanPackage - Updates the dependency registration action
  depending on the current state and planned action for the package.

*********************************************************************/
HRESULT DependencyPlanPackage(
    __in BURN_PACKAGE* pPackage,
    __in BURN_PLAN* pPlan,
    __in_z LPCWSTR wzBundleProviderKey,
    __in BOOTSTRAPPER_ACTION_STATE executeAction,
    __in BOOTSTRAPPER_ACTION_STATE rollbackAction,
    __out BURN_DEPENDENCY_ACTION* pDependencyAction
    );

/********************************************************************
 DependencyExecuteAction - Registers or unregisters dependency
  information for the package contained within the action.

*********************************************************************/
HRESULT DependencyExecuteAction(
    __in BURN_EXECUTE_ACTION* pAction,
    __in BOOL fPerMachine
    );

/********************************************************************
 DependencyRegister - Registers the bundle dependency provider key.

*********************************************************************/
HRESULT DependencyRegister(
    __in const BURN_REGISTRATION* pRegistration
    );

/********************************************************************
 DependencyRegisterDependent - Registers the bundle as a dependent of each
  dependency provided by the specified package.

 Note: Returns S_FALSE if the dependency package is missing.
  Call DepCheckDependencies to check for missing dependency packages.

 Note: If the bundle and package are installed in different contexts
  (per-user vs. per-machine) dependency registration is skipped
  and this function returns S_FALSE.
*********************************************************************/
HRESULT DependencyRegisterDependent(
    __in_z LPCWSTR wzBundleProviderKey,
    __in BOOL fPerMachine,
    __in const BURN_PACKAGE* pPackage
    );

/********************************************************************
 DependencyUnregister - Removes the bundle dependency provider key.

 Note: Does not check for existing dependents.
*********************************************************************/
HRESULT DependencyUnregister(
    __in const BURN_REGISTRATION* pRegistration
    );

/********************************************************************
 DependencyUnregisterDependent - Removes the bundle as a dependent of each
  dependency provided by the specified package.

*********************************************************************/
HRESULT DependencyUnregisterDependent(
    __in_z LPCWSTR wzBundleProviderKey,
    __in BOOL fPerMachine,
    __in const BURN_PACKAGE* pPackage
    );

#if defined(__cplusplus)
}
#endif
