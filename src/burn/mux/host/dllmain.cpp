//-------------------------------------------------------------------------------------------------
// <copyright file="config.h" company="Microsoft">
// Copyright (c) Microsoft Corporation. All rights reserved.
// </copyright>
// 
// <summary>
// Entry point and globals for the DLL.
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"

HMODULE g_hInstance = NULL;

#pragma warning( push )
#pragma warning( disable:4100 )
BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        // If this fails, it just results in extra unhandled calls.
        ::DisableThreadLibraryCalls(hModule);
        g_hInstance = hModule;
        break;

    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }

    return TRUE;
}
#pragma warning( pop )
