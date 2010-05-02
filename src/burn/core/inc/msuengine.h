//-------------------------------------------------------------------------------------------------
// <copyright file="msuengine.h" company="Microsoft">
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

HRESULT MsuEngineParsePackageFromXml(
    __in IXMLDOMNode* pixnMsiPackage,
    __in BURN_PACKAGE* pPackage
    );
void MsuEnginePackageUninitialize(
    __in BURN_PACKAGE* pPackage
    );
HRESULT MsuEngineDetectPackage(
    __in BURN_PACKAGE* pPackage
    );
HRESULT MsuEnginePlanPackage(
    __in BURN_PACKAGE* pPackage,
    __in BURN_VARIABLES* pVariables
    );
HRESULT MsuEngineConfigurePackage(
    __in BURN_PACKAGE* pPackage,
    __in BURN_VARIABLES* pVariables,
    __in BOOL fExecutingRollback
    );


#if defined(__cplusplus)
}
#endif
