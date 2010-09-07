//-------------------------------------------------------------------------------------------------
// <copyright file="engine.cpp" company="Microsoft">
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
//    Module: Core
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"


// constants

// internal function declarations

static HRESULT RunPerUser(
    __in BURN_ENGINE_STATE* pEngineState,
    __in_z LPCWSTR wzCommandLine
    );
static HRESULT RunPerMachine(
    __in BURN_ENGINE_STATE* pEngineState
    );
static HRESULT RunUncache(
    __in BURN_ENGINE_STATE* pEngineState
    );
static HRESULT ProcessMessage(
    __in BURN_ENGINE_STATE* pEngineState,
    __in const MSG* pmsg
    );
static HRESULT DAPI RedirectElevatedLoggingOverPipe(
    __in_z LPCSTR szString,
    __in_opt LPVOID pvContext
    );
static HRESULT Restart();


// function definitions

extern "C" HRESULT EngineRun(
    __in_z_opt LPCWSTR wzCommandLine,
    __in int nCmdShow,
    __out DWORD* pdwExitCode
    )
{
    HRESULT hr = S_OK;
    BOOL fComInitialized = FALSE;
    BOOL fRegInitialized = FALSE;
    BOOL fXmlInitialized = FALSE;
    BOOL fLogInitialized = FALSE;
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

    LogInitialize(::GetModuleHandleW(NULL));
    fLogInitialized = TRUE;

    // initialize Reg util
    hr = RegInitialize();
    ExitOnFailure(hr, "Failed to initialize Regutil.");
    fRegInitialized = TRUE;

    // initialize XML util
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
        hr = RunPerUser(&engineState, wzCommandLine);
        ExitOnFailure(hr, "Failed to run per-user mode.");
        break;

    case BURN_MODE_ELEVATED:
        hr = RunPerMachine(&engineState);
        ExitOnFailure(hr, "Failed to run per-machine mode.");
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
    CoreUninitialize(&engineState);

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

    if (fRegInitialized)
    {
        RegUninitialize();
    }
    if (fXmlInitialized)
    {
        XmlUninitialize();
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

static HRESULT RunPerUser(
    __in BURN_ENGINE_STATE* pEngineState,
    __in_z LPCWSTR wzCommandLine
    )
{
    HRESULT hr = S_OK;
    DWORD dwThreadId = 0;
    IBootstrapperEngine* pEngineForApplication = NULL;
    BOOL fStartupCalled = FALSE;
    BOOL fRet = FALSE;
    MSG msg = { };

    // We don't open the log in the elevated process because it redirects
    // its log messages to the parent process over the pipe.
    hr = LoggingOpen(&pEngineState->log, wzCommandLine, &pEngineState->variables);
    ExitOnFailure(hr, "Failed to open log.");

    // Initialize this thread's message queue and load the application.
    ::PeekMessageW(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);
    dwThreadId = ::GetCurrentThreadId();

    hr = EngineForApplicationCreate(pEngineState, dwThreadId, &pEngineForApplication);
    ExitOnFailure(hr, "Failed to create engine for UX.");

    hr = UserExperienceLoad(&pEngineState->userExperience, pEngineForApplication, &pEngineState->command);
    ExitOnFailure(hr, "Failed to load UX.");

    // Set resume commandline
    hr = RegistrationSetResumeCommand(&pEngineState->registration, &pEngineState->command, &pEngineState->log);
    ExitOnFailure(hr, "Failed to set resume command");

    // query registration
    hr = CoreQueryRegistration(pEngineState);
    ExitOnFailure(hr, "Failed to query registration.");

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
    }

    // unload UX
    UserExperienceUnload(&pEngineState->userExperience);

    // end per-machine process if running
    if (pEngineState->hElevatedProcess && INVALID_HANDLE_VALUE != pEngineState->hElevatedPipe)
    {
        ElevationParentProcessTerminate(pEngineState->hElevatedProcess, pEngineState->hElevatedPipe);
    }

    ReleaseObject(pEngineForApplication);

    return hr;
}

static HRESULT RunPerMachine(
    __in BURN_ENGINE_STATE* pEngineState
    )
{
    HRESULT hr = S_OK;

    // Override logging to write over the pipe
    LogRedirect(RedirectElevatedLoggingOverPipe, pEngineState);

    // connect to per-user process
    hr = ElevationChildConnect(pEngineState->sczElevatedPipeName, pEngineState->sczElevatedToken, &pEngineState->hElevatedPipe);
    ExitOnFailure(hr, "Failed to connect to per-user process.");

    hr = ElevationChildConnected(pEngineState->hElevatedPipe);
    ExitOnFailure(hr, "Failed to notify parent process that child is connected.");

    // pump messages from per-user process
    hr = ElevationChildPumpMessages(pEngineState->hElevatedPipe, &pEngineState->packages, &pEngineState->payloads, &pEngineState->variables, &pEngineState->registration, &pEngineState->userExperience);
    ExitOnFailure(hr, "Failed to pump messages from per-user process.");

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
        hr = RegistrationSessionEnd(&pEngineState->registration, BOOTSTRAPPER_ACTION_UNINSTALL, FALSE, fRunningPerMachine);
    }

    hr = S_OK;

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

static HRESULT DAPI RedirectElevatedLoggingOverPipe(
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

    hr = ElevationSendMessage(pEngineState->hElevatedPipe, BURN_ELEVATION_MESSAGE_TYPE_LOG, pbData, cbData, &dwResult);
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
