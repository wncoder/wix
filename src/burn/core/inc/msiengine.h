//-------------------------------------------------------------------------------------------------
// <copyright file="msiengine.h" company="Microsoft">
//    Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//
// <summary>
//    Module: MSI Engine
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once


#if defined(__cplusplus)
extern "C" {
#endif


// function declarations

HRESULT MsiEngineParsePackageFromXml(
    __in IXMLDOMNode* pixnBundle,
    __in BURN_PACKAGE* pPackage
    );
void MsiEnginePackageUninitialize(
    __in BURN_PACKAGE* pPackage
    );
HRESULT MsiEngineDetectPackage(
    __in BURN_PACKAGE* pPackage
    );
HRESULT MsiEnginePlanPackage(
    __in BURN_PACKAGE* pPackage,
    __in BURN_VARIABLES* pVariables
    );
HRESULT MsiEngineConfigurePackage(
    __in BURN_PACKAGE* pPackage,
    __in BURN_VARIABLES* pVariables,
    __in BOOL fExecutingRollback
    );


#if defined(__cplusplus)
}
#endif
