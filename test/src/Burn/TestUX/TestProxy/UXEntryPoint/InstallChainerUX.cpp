//------------------------------------------------------------------------------
// Copyright (c) Microsoft Corporation. All rights reserved.
//
// Description: CInstallChainerUX implements the IBurnUserExperience in order
//              to provide an interface for a burnstub.exe created chainer
//              setup application
//
//              WARNING: ensure ...\Microsoft.Net\Framework\v2.0.50727 is 
//              found first in your PATH otherwise you may not be importing 
//              the correct version of mscorlib.tlb
//-----------------------------------------------------------------------------

#include "precomp.h"
#include "CreateUserExperience.h"
#include "uxcore.h"

//
// CreateUserExperience - creates a new IBurnUserExperience object.
//
HRESULT CreateUserExperience(
    _In_ HMODULE hModule,
    _In_ BURN_COMMAND* pCommand,
    _Out_ IBurnUserExperience **ppUX
    )
{
    HRESULT hr = S_OK;
    CChainerUXCore* pUX = NULL;

    pUX = new CChainerUXCore(hModule,
                             pCommand,
                             L"ManagedUX.dll",                  // managed UX assembly
                             L"BurnSampleWPFUI.LaunchWindow"   // managed UX class
                             );   
    ExitOnNull(pUX, hr, E_OUTOFMEMORY, "Failed to create new standard UX object.");

    *ppUX = pUX;
    pUX = NULL;

LExit:
    return hr;
}