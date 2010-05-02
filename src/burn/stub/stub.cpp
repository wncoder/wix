//-------------------------------------------------------------------------------------------------
// <copyright file="stub.cpp" company="Microsoft">
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
// Setup chainer/bootstrapper executable for WiX toolset.
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"

#define HRESULT_FROM_RPC(x)      ((HRESULT) ((x) | FACILITY_RPC))

int WINAPI wWinMain(
    __in HINSTANCE hInstance,
    __in_opt HINSTANCE hPrevInstance,
    __in_opt LPWSTR lpCmdLine,
    __in int nCmdShow
    )
{
    ::HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);

    HRESULT hr = S_OK;
    BURN_ENGINE_STATE engineState = { };
    BOOL fComInitialized = FALSE;

    // Ensure that log contains approriate level of information
#ifdef _DEBUG
    LogSetLevel(REPORT_DEBUG, FALSE);
#else
    LogSetLevel(REPORT_VERBOSE, FALSE); // FALSE means don't write an additional text line to the log saying the level changed
#endif

    // initialize COM
    hr = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
    ExitOnFailure(hr, "Failed to initialize COM.");
    fComInitialized = TRUE;

    // initialize platform layer
    PlatformInitialize();

    // initialize XML util
    hr = XmlInitialize();
    ExitOnFailure(hr, "Failed to initialize XML util.");

    // initialize core
    hr = CoreInitialize(&engineState);
    ExitOnFailure(hr, "Failed to initialize core.");

    // call run
    hr = Run(&engineState);
    ExitOnFailure(hr, "Failed to run application.");

LExit:
    CoreUninitialize(&engineState);
    XmlUninitialize();
    if (fComInitialized)
    {
        ::CoUninitialize();
    }

    return FAILED(hr) ? (int)hr : (int)engineState.dwExitCode;
}
