//------------------------------------------------------------------------------
// Copyright (c) Microsoft Corporation. All rights reserved.
//
// Description: Defines the entry point BurnView requires and defines
//              a required helper function for DLL that implements DLLMain
//------------------------------------------------------------------------------

#include "precomp.h"
#include "CreateUserExperience.h"


HMODULE vhInstance = NULL;


extern "C" void SaveModuleHandle(_In_ HMODULE hModule)
{
    vhInstance = hModule;
}


extern "C" HRESULT WINAPI SetupUXCreate(
    _In_ BURN_COMMAND* pCommand,
    _Deref_out_ IBurnUserExperience **ppUX    
    )
{
   HRESULT hr = S_OK; 

    //::MessageBoxA(NULL, "Attach Debugger C++ (StartUp)", "Attach Debugger (StartUp)", MB_OK);
    if (vhInstance == NULL)
    {
        hr = E_HANDLE;
    }
    else if ( (BURN_DISPLAY_FULL != pCommand->display || BURN_DISPLAY_PASSIVE != pCommand->display || BURN_DISPLAY_NONE != pCommand->display) && 
		ManagedDependenciesInstalled() )
    {
        hr = CreateUserExperience(vhInstance, pCommand, ppUX);
        ExitOnFailure(hr, "Failed to create user experience interface.");
    }
    else
    {
        // this UX requires .NET so return ERROR_NOT_SUPPORTED if our
        // min version of .NET is not found 
        hr = ERROR_NOT_SUPPORTED;
    }

LExit:
    return hr;
}