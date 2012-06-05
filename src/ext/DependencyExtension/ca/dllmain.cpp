//-------------------------------------------------------------------------------------------------
// <copyright file="dllmain.cpp" company="Microsoft Corporation">
//   Copyright (c) 2004, Microsoft Corporation.
//   This software is released under Common Public License Version 1.0 (CPL).
//   The license and further copyright text can be found in the file LICENSE.TXT
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
// 
// <summary>
//    WixDepCA DllMain function.
// </summary>
//-------------------------------------------------------------------------------------------------
#include "precomp.h"

/********************************************************************
DllMain - standard entry point for all WiX custom actions.

********************************************************************/
extern "C" BOOL WINAPI DllMain(
    IN HINSTANCE hInstance,
    IN ULONG ulReason,
    IN LPVOID)
{
    switch(ulReason)
    {
    case DLL_PROCESS_ATTACH:
        WcaGlobalInitialize(hInstance);
        ::DisableThreadLibraryCalls(hInstance);
        break;

    case DLL_PROCESS_DETACH:
        WcaGlobalFinalize();
        break;
    }

    return TRUE;
}