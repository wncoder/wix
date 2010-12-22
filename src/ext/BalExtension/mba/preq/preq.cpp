//-------------------------------------------------------------------------------------------------
// <copyright file="preq.cpp" company="Microsoft">
//    Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//
// <summary>
// MBA prerequisite bootstrapper application.
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"

static HINSTANCE vhInstance = NULL;

extern "C" BOOL WINAPI DllMain(
    IN HINSTANCE hInstance,
    IN DWORD dwReason,
    IN LPVOID /* pvReserved */
    )
{
    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
        ::DisableThreadLibraryCalls(hInstance);
        vhInstance = hInstance;
        break;

    case DLL_PROCESS_DETACH:
        vhInstance = NULL;
        break;
    }

    return TRUE;
}


extern "C" HRESULT WINAPI BootstrapperApplicationCreate(
    __in IBootstrapperEngine* pEngine,
    __in const BOOTSTRAPPER_COMMAND* pCommand,
    __out IBootstrapperApplication** ppApplication
    )
{
    HRESULT hr = S_OK;

    BalInitialize(pEngine);

    hr = CreateBootstrapperApplication(vhInstance, pEngine, pCommand, ppApplication);
    BalExitOnFailure(hr, "Failed to create bootstrapper application interface.");

LExit:
    return hr;
}


extern "C" void WINAPI BootstrapperApplicationDestroy()
{
    BalUninitialize();
}
