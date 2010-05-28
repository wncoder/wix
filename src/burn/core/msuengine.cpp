//-------------------------------------------------------------------------------------------------
// <copyright file="msuengine.cpp" company="Microsoft">
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
