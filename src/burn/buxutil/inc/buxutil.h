#pragma once
//-------------------------------------------------------------------------------------------------
// <copyright file="buxutil.h" company="Microsoft">
//    Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//
// <summary>
// Burn UX utility library.
// </summary>
//-------------------------------------------------------------------------------------------------

#include "dutil.h"


#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************
 BuxManifestLoad - loads the UX manifest into an XML document.

********************************************************************/
DAPI_(HRESULT) BuxManifestLoad(
    __in HMODULE hUXModule,
    __out IXMLDOMDocument** ppixdManifest
    );


#ifdef __cplusplus
}
#endif
