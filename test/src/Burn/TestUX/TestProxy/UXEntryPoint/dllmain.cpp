//------------------------------------------------------------------------------
// Copyright (c) Microsoft Corporation. All rights reserved.
//
// Description: 
//------------------------------------------------------------------------------

#include "precomp.h"
#include "CreateUserExperience.h"


extern "C" BOOL WINAPI DllMain(HMODULE hModule,
                               DWORD  ul_reason_for_call,
                               LPVOID /*lpReserved*/
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        ::DisableThreadLibraryCalls(hModule);
        SaveModuleHandle(hModule);
        break;
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}