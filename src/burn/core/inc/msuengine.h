//-------------------------------------------------------------------------------------------------
// <copyright file="msuengine.h" company="Microsoft">
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
