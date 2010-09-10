//-------------------------------------------------------------------------------------------------
// <copyright file="mspengine.h" company="Microsoft">
//    Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//
// <summary>
//    Module: MSP Engine
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once


#if defined(__cplusplus)
extern "C" {
#endif


// constants


// structures


// typedefs


// function declarations

HRESULT MspEngineParsePackageFromXml(
    __in IXMLDOMNode* pixnBundle,
    __in BURN_PACKAGE* pPackage
    );
void MspEnginePackageUninitialize(
    __in BURN_PACKAGE* pPackage
    );
HRESULT MspEngineDetectInitialize(
    __in BURN_PACKAGES* pPackages
    );
HRESULT MspEngineDetectPackage(
    __in BURN_PACKAGE* pPackage,
    __in BURN_USER_EXPERIENCE* pUserExperience
    );
HRESULT MspEnginePlanPackage(
    __in DWORD dwPackageSequence,
    __in BURN_PACKAGE* pPackage,
    __in BURN_PLAN* pPlan,
    __in BURN_LOGGING* pLog,
    __in BURN_VARIABLES* pVariables,
    __in_opt HANDLE hCacheEvent,
    __in BURN_USER_EXPERIENCE* pUserExperience,
    __out BOOTSTRAPPER_ACTION_STATE* pExecuteAction,
    __out BOOTSTRAPPER_ACTION_STATE* pRollbackAction
    );
HRESULT MspEngineExecutePackage(
    __in BURN_EXECUTE_ACTION* pExecuteAction,
    __in BURN_VARIABLES* pVariables,
    __in BOOL fRollback,
    __in PFN_MSIEXECUTEMESSAGEHANDLER pfnMessageHandler,
    __in LPVOID pvContext,
    __out BOOTSTRAPPER_APPLY_RESTART* pRestart
    );


#if defined(__cplusplus)
}
#endif
