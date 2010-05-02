//-------------------------------------------------------------------------------------------------
// <copyright file="msuengine.cpp" company="Microsoft">
//    Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//
// <summary>
//    Module: MSU Engine
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"


// internal function declarations


// function definitions

extern "C" HRESULT MsuEngineParsePackageFromXml(
    __in IXMLDOMNode* pixnBundle,
    __in BURN_PACKAGE* pPackage
    )
{
    HRESULT hr = S_OK;

LExit:
    return hr;
}

extern "C" void MsuEnginePackageUninitialize(
    __in BURN_PACKAGE* pPackage
    )
{
}

extern "C" HRESULT MsuEngineDetectPackage(
    __in BURN_PACKAGE* pPackage
    )
{
    HRESULT hr = S_OK;

LExit:
    return hr;
}

//
// Plan - calculates the execute and rollback state for the requested package state.
//
extern "C" HRESULT MsuEnginePlanPackage(
    __in BURN_PACKAGE* pPackage,
    __in BURN_VARIABLES* pVariables
    )
{
    HRESULT hr = S_OK;

LExit:
    return hr;
}

extern "C" HRESULT MsuEngineConfigurePackage(
    __in BURN_PACKAGE* pPackage,
    __in BURN_VARIABLES* pVariables,
    __in BOOL fExecutingRollback
    )
{
    HRESULT hr = S_OK;

LExit:
    return hr;
}


// internal helper functions
