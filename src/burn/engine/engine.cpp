//-------------------------------------------------------------------------------------------------
// <copyright file="engine.cpp" company="Microsoft">
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
//    Module: Core
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"


// constants

// internal function declarations

static HRESULT RunNormal(
    __in HINSTANCE hInstance,
    __in BURN_ENGINE_STATE* pEngineState,
    __in_z_opt LPCWSTR wzCommandLine
    );
static HRESULT RunElevated(
    __in BURN_ENGINE_STATE* pEngineState
    );
static HRESULT RunEmbedded(
    __in HINSTANCE hInstance,
    __in BURN_ENGINE_STATE* pEngineState,
    __in_z_opt LPCWSTR wzCommandLine
    );
static HRESULT RunUncache(
    __in BURN_ENGINE_STATE* pEngineState
    );
static HRESULT RunApplication(
    __in BURN_ENGINE_STATE* pEngineState,
    __out BOOL* pfReloadApp
    );
static HRESULT ProcessMessage(
    __in BURN_ENGINE_STATE* pEngineState,
    __in const MSG* pmsg
    );
static HRESULT DAPI RedirectLoggingOverPipe(
    __in_z LPCSTR szString,
    __in_opt LPVOID pvContext
    );
static HRESULT Restart();


// function definitions

extern "C" HRESULT EngineRun(
    __in HINSTANCE hInstance,
    __in_z_opt LPCWSTR wzCommandLine,
    __in int nCmdShow,
    __out DWORD* pdwExitCode
    )
{
    HRESULT hr = S_OK;
    BOOL fComInitialized = FALSE;
    BOOL fLogInitialized = FALSE;
    BOOL fRegInitialized = FALSE;
    BOOL fWiuInitialized = FALSE;
    BOOL fXmlInitialized = FALSE;
    BURN_ENGINE_STATE engineState = { };
    BOOL fRestart = FALSE;

    // Ensure that log contains approriate level of information
#ifdef _DEBUG
    LogSetLevel(REPORT_DEBUG, FALSE);
#else
    LogSetLevel(REPORT_VERBOSE, FALSE); // FALSE means don't write an additional text line to the log saying the level changed
#endif

    // initialize platform layer
    PlatformInitialize();

    // initialize COM
    hr = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
    ExitOnFailure(hr, "Failed to initialize COM.");
    fComInitialized = TRUE;

    // Initialize dutil.
    LogInitialize(::GetModuleHandleW(NULL));
    fLogInitialized = TRUE;

    hr = RegInitialize();
    ExitOnFailure(hr, "Failed to initialize Regutil.");
    fRegInitialized = TRUE;

    hr = WiuInitialize();
    ExitOnFailure(hr, "Failed to initialize Wiutil.");
    fWiuInitialized = TRUE;

    hr = XmlInitialize();
    ExitOnFailure(hr, "Failed to initialize XML util.");
    fXmlInitialized = TRUE;

    // initialize core
    hr = CoreInitialize(wzCommandLine, nCmdShow, &engineState);
    ExitOnFailure(hr, "Failed to initialize core.");

    // select run mode
    switch (engineState.mode)
    {
    case BURN_MODE_NORMAL:
        hr = RunNormal(hInstance, &engineState, wzCommandLine);
        ExitOnFailure(hr, "Failed to run per-user mode.");
        break;

    case BURN_MODE_ELEVATED:
        hr = RunElevated(&engineState);
        ExitOnFailure(hr, "Failed to run per-machine mode.");
        break;

    case BURN_MODE_EMBEDDED:
        hr = RunEmbedded(hInstance, &engineState, wzCommandLine);
        ExitOnFailure(hr, "Failed to run embedded mode.");
        break;

    case BURN_MODE_UNCACHE_PER_MACHINE: __fallthrough;
    case BURN_MODE_UNCACHE_PER_USER:
        hr = RunUncache(&engineState);
        ExitOnFailure(hr, "Failed to run uncache mode.");
        break;

    default:
        hr = E_UNEXPECTED;
        ExitOnFailure(hr, "Invalid run mode.");
    }

    // set exit code and remember if we are supposed to restart.
    *pdwExitCode = engineState.userExperience.dwExitCode;
    fRestart = engineState.fRestart;

LExit:
    if (fLogInitialized)
    {
        // If anything went wrong but the log was never open, try to open a "failure" log
        // and that will dump anything captured in the log memory buffer to the log.
        if (FAILED(hr) && BURN_LOGGING_STATE_CLOSED == engineState.log.state)
        {
            LogOpen(NULL, L"Setup", L"_Failed", L"txt", FALSE, FALSE, NULL);
        }

        LogUninitialize(FALSE);
    }

    CoreUninitialize(&engineState);

    if (fXmlInitialized)
    {
        XmlUninitialize();
    }

    if (fWiuInitialized)
    {
        WiuUninitialize();
    }

    if (fRegInitialized)
    {
        RegUninitialize();
    }

    if (fComInitialized)
    {
        ::CoUninitialize();
    }

    if (fRestart)
    {
        Restart();
    }

    return hr;
}


// internal function definitions

static HRESULT RunNormal(
    __in HINSTANCE hInstance,
    __in BURN_ENGINE_STATE* pEngineState,
    __in_z_opt LPCWSTR wzCommandLine
    )
{
    HRESULT hr = S_OK;
    BOOL fContinueExecution = TRUE;
    BOOL fReloadApp = FALSE;

    // Initialize logging.
    hr = LoggingOpen(&pEngineState->log, wzCommandLine, &pEngineState->variables);
    ExitOnFailure(hr, "Failed to open log.");

    // Ensure we're on a supported operating system.
    hr = ConditionGlobalCheck(&pEngineState->variables, &pEngineState->condition, pEngineState->command.display, &pEngineState->userExperience.dwExitCode, &fContinueExecution);
    ExitOnFailure(hr, "Failed to check global conditions");

    if (!fContinueExecution)
    {
        LogId(REPORT_STANDARD, MSG_FAILED_CONDITION_CHECK);

        // If the block told us to abort, abort!
        ExitFunction1(hr = S_OK);
    }

    if (pEngineState->userExperience.fSplashScreen && BOOTSTRAPPER_DISPLAY_NONE < pEngineState->command.display)
    {
        SplashScreenCreate(hInstance, NULL, &pEngineState->command.hwndSplashScreen);
    }

    // Query registration state.
    hr = CoreQueryRegistration(pEngineState);
    ExitOnFailure(hr, "Failed to query registration.");

    // Set resume commandline
    hr = RegistrationSetResumeCommand(&pEngineState->registration, &pEngineState->command, &pEngineState->log);
    ExitOnFailure(hr, "Failed to set resume command");

    do
    {
        fReloadApp = FALSE;

        hr = RunApplication(pEngineState, &fReloadApp);
        ExitOnFailure(hr, "Failed while running ");
    } while (fReloadApp);

LExit:
    // end per-machine process if running
    if (pEngineState->hElevatedProcess && INVALID_HANDLE_VALUE != pEngineState->hElevatedPipe)
    {
        PipeTerminateChildProcess(pEngineState->hElevatedProcess, pEngineState->hElevatedPipe);
    }

    // If the splash screen is still around, close it.
    if (::IsWindow(pEngineState->command.hwndSplashScreen))
    {
        ::PostMessageW(pEngineState->command.hwndSplashScreen, WM_CLOSE, 0, 0);
    }

    UserExperienceRemove(&pEngineState->userExperience);

    return hr;
}

static HRESULT RunElevated(
    __in BURN_ENGINE_STATE* pEngineState
    )
{
    HRESULT hr = S_OK;

    // Override logging to write over the pipe.
    LogRedirect(RedirectLoggingOverPipe, pEngineState);

    // connect to per-user process
    hr = PipeChildConnect(pEngineState->sczParentPipeName, pEngineState->sczParentToken, &pEngineState->hElevatedPipe);
    ExitOnFailure(hr, "Failed to connect to parent process.");

    hr = PipeChildConnected(pEngineState->hElevatedPipe);
    ExitOnFailure(hr, "Failed to notify parent process that child is connected.");

    // Pump messages from parent process.
    hr = ElevationChildPumpMessages(pEngineState->hElevatedPipe, &pEngineState->packages, &pEngineState->payloads, &pEngineState->variables, &pEngineState->registration, &pEngineState->userExperience);
    ExitOnFailure(hr, "Failed to pump messages from parent process.");

LExit:
    return hr;
}

static HRESULT RunEmbedded(
    __in HINSTANCE hInstance,
    __in BURN_ENGINE_STATE* pEngineState,
    __in_z_opt LPCWSTR wzCommandLine
    )
{
    HRESULT hr = S_OK;

    // Connect to parent process.
    hr = PipeChildConnect(pEngineState->sczParentPipeName, pEngineState->sczParentToken, &pEngineState->hEmbeddedPipe);
    ExitOnFailure(hr, "Failed to connect to parent process.");

    hr = PipeChildConnected(pEngineState->hEmbeddedPipe);
    ExitOnFailure(hr, "Failed to notify parent process that child is connected.");

    // Now run the application like normal.
    hr = RunNormal(hInstance, pEngineState, wzCommandLine);
    ExitOnFailure(hr, "Failed to run bootstrapper application embedded.");

LExit:
    return hr;
}

static HRESULT RunUncache(
    __in BURN_ENGINE_STATE* pEngineState
    )
{
    HRESULT hr = S_OK;
    BOOL fRunningPerMachine = (BURN_MODE_UNCACHE_PER_MACHINE == pEngineState->mode);

    for (DWORD i = 0; i < pEngineState->packages.cPackages; ++i)
    {
        BURN_PACKAGE* pPackage = pEngineState->packages.rgPackages + i;

        if (fRunningPerMachine == pPackage->fPerMachine)
        {
            hr = CacheRemovePackage(pPackage->fPerMachine, pPackage->sczCacheId);
        }
    }

    if (pEngineState->registration.fRegisterArp && fRunningPerMachine == pEngineState->registration.fPerMachine)
    {
        hr = RegistrationSessionEnd(&pEngineState->registration, BOOTSTRAPPER_ACTION_UNINSTALL, FALSE, fRunningPerMachine, NULL);
    }

    hr = S_OK;

    return hr;
}

static HRESULT RunApplication(
    __in BURN_ENGINE_STATE* pEngineState,
    __out BOOL* pfReloadApp
    )
{
    HRESULT hr = S_OK;
    DWORD dwThreadId = 0;
    IBootstrapperEngine* pEngineForApplication = NULL;
    BOOL fStartupCalled = FALSE;
    BOOL fRet = FALSE;
    MSG msg = { };

    ::PeekMessageW(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);
    dwThreadId = ::GetCurrentThreadId();

    // Load the bootstrapper application.
    hr = EngineForApplicationCreate(pEngineState, dwThreadId, &pEngineForApplication);
    ExitOnFailure(hr, "Failed to create engine for UX.");

    hr = UserExperienceLoad(&pEngineState->userExperience, pEngineForApplication, &pEngineState->command);
    ExitOnFailure(hr, "Failed to load UX.");

    fStartupCalled = TRUE;
    hr = pEngineState->userExperience.pUserExperience->OnStartup();
    ExitOnFailure(hr, "Failed to start bootstrapper application.");

    // Enter the message pump.
    while (0 != (fRet = ::GetMessageW(&msg, NULL, 0, 0)))
    {
        if (-1 == fRet)
        {
            hr = E_UNEXPECTED;
            ExitOnRootFailure(hr, "Unexpected return value from message pump.");
        }
        else
        {
            ProcessMessage(pEngineState, &msg);
        }
    }

    // get exit code
    pEngineState->userExperience.dwExitCode = (DWORD)msg.wParam;

LExit:
    if (fStartupCalled)
    {
        int nResult = pEngineState->userExperience.pUserExperience->OnShutdown();
        if (IDRESTART == nResult)
        {
            pEngineState->fRestart = TRUE;
        }
        else if (IDRELOAD_BOOTSTRAPPER == nResult)
        {
            *pfReloadApp = TRUE;
        }
    }

    // unload UX
    UserExperienceUnload(&pEngineState->userExperience);

    ReleaseObject(pEngineForApplication);

    return hr;
}

static HRESULT ProcessMessage(
    __in BURN_ENGINE_STATE* pEngineState,
    __in const MSG* pmsg
    )
{
    HRESULT hr = S_OK;

    switch (pmsg->message)
    {
    case WM_BURN_DETECT:
        hr = CoreDetect(pEngineState);
        break;

    case WM_BURN_PLAN:
        hr = CorePlan(pEngineState, static_cast<BOOTSTRAPPER_ACTION>(pmsg->lParam));
        break;

    case WM_BURN_APPLY:
        hr = CoreApply(pEngineState, reinterpret_cast<HWND>(pmsg->lParam));
        break;

    case WM_BURN_QUIT:
        hr = CoreQuit(pEngineState, static_cast<int>(pmsg->wParam));
        break;
    }

    return hr;
}

static HRESULT DAPI RedirectLoggingOverPipe(
    __in_z LPCSTR szString,
    __in_opt LPVOID pvContext
    )
{
    HRESULT hr = S_OK;
    BURN_ENGINE_STATE* pEngineState = static_cast<BURN_ENGINE_STATE*>(pvContext);
    BYTE* pbData = NULL;
    SIZE_T cbData = 0;
    DWORD dwResult = 0;

    hr = BuffWriteStringAnsi(&pbData, &cbData, szString);
    ExitOnFailure(hr, "Failed to write string to buffer.");

    hr = PipeSendMessage(pEngineState->hElevatedPipe, static_cast<DWORD>(BURN_PIPE_MESSAGE_TYPE_LOG), pbData, cbData, NULL, NULL, &dwResult);
    ExitOnFailure(hr, "Failed to post log message over pipe.");
    hr = (HRESULT)dwResult;

LExit:
    ReleaseBuffer(pbData);

    return hr;
}

static HRESULT Restart()
{
    HRESULT hr = S_OK;
    HANDLE hProcessToken = NULL;
    TOKEN_PRIVILEGES priv = { };

    if (!::OpenProcessToken(::GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hProcessToken))
    {
        ExitWithLastError(hr, "Failed to get process token.");
    }

    priv.PrivilegeCount = 1;
    priv.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    if (!::LookupPrivilegeValueW(NULL, L"SeShutdownPrivilege", &priv.Privileges[0].Luid))
    {
        ExitWithLastError(hr, "Failed to get shutdown privilege LUID.");
    }

    if (!::AdjustTokenPrivileges(hProcessToken, FALSE, &priv, sizeof(TOKEN_PRIVILEGES), NULL, 0))
    {
        ExitWithLastError(hr, "Failed to adjust token to add shutdown privileges.");
    }

    if (!vpfnInitiateSystemShutdownExW(NULL, NULL, 0, FALSE, TRUE, SHTDN_REASON_MAJOR_APPLICATION | SHTDN_REASON_MINOR_INSTALLATION | SHTDN_REASON_FLAG_PLANNED))
    {
        ExitWithLastError(hr, "Failed to schedule restart.");
    }

LExit:
    ReleaseHandle(hProcessToken);
    return hr;
}
