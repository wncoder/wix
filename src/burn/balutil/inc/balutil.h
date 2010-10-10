#pragma once
//-------------------------------------------------------------------------------------------------
// <copyright file="buxutil.h" company="Microsoft">
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
// Burn utility library.
// </summary>
//-------------------------------------------------------------------------------------------------

#include "dutil.h"


#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************
 BalManifestLoad - loads the Application manifest into an XML document.

********************************************************************/
DAPI_(HRESULT) BalManifestLoad(
    __in HMODULE hUXModule,
    __out IXMLDOMDocument** ppixdManifest
    );


#ifdef __cplusplus
}
#endif
