//-------------------------------------------------------------------------------------------------
// <copyright file="core.cpp" company="Microsoft">
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
//
//    Setup chainer/bootstrapper core for WiX toolset.
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"

// structs

struct BURN_CACHE_THREAD_CONTEXT
{
    BURN_ENGINE_STATE* pEngineState;
    DWORD* pcOverallProgressTicks;
};


// internal function declarations

static HRESULT ParseCommandLine(
    __in_z_opt LPCWSTR wzCommandLine,
    __in BOOTSTRAPPER_COMMAND* pCommand,
    __out BURN_MODE* pMode,
    __out BOOL *pfAppend,
    __inout_z LPWSTR* psczLogFile,
    __inout_z LPWSTR* psczElevatedPipeName,
    __inout_z LPWSTR* psczElevatedToken
    );
static HRESULT DetectPackagePayloadsCached(
    __in BURN_PACKAGE* pPackage
    );
static DWORD WINAPI CacheThreadProc(
    __in LPVOID lpThreadParameter
    );


// function definitions

extern "C" HRESULT CoreInitialize(
    __in_z_opt LPCWSTR wzCommandLine,
    __in int nCmdShow,
    __in BURN_ENGINE_STATE* pEngineState
    )
{
    HRESULT hr = S_OK;
    LPWSTR sczStreamName = NULL;
    BYTE* pbBuffer = NULL;
    SIZE_T cbBuffer = 0;
    BURN_CONTAINER_CONTEXT containerContext = { };

    // initialize structure
    ::InitializeCriticalSection(&pEngineState->csActive);
    ::InitializeCriticalSection(&pEngineState->userExperience.csEngineActive);
    pEngineState->hElevatedPipe = INVALID_HANDLE_VALUE;

    // parse command line
    hr = ParseCommandLine(wzCommandLine, &pEngineState->command, &pEngineState->mode, &pEngineState->log.fAppend, &pEngineState->log.sczPath, &pEngineState->sczElevatedPipeName, &pEngineState->sczElevatedToken);
    ExitOnFailure(hr, "Failed to parse command line.");

    pEngineState->command.nCmdShow = nCmdShow;

    // initialize built-in variables
    hr = VariableInitializeBuiltIn(&pEngineState->variables);
    ExitOnFailure(hr, "Failed to initialize built-in variables.");

    // open attached UX container
    hr = ContainerOpenUX(&containerContext);
    ExitOnFailure(hr, "Failed to open attached UX container.");

    // load manifest
    hr = ContainerNextStream(&containerContext, &sczStreamName);
    ExitOnFailure(hr, "Failed to open manifest stream.");

    hr = ContainerStreamToBuffer(&containerContext, &pbBuffer, &cbBuffer);
    ExitOnFailure(hr, "Failed to get manifest stream from container.");

    hr = ManifestLoadXmlFromBuffer(pbBuffer, cbBuffer, pEngineState);
    ExitOnFailure(hr, "Failed to load manifest.");

    // set registration paths
    hr = RegistrationSetPaths(&pEngineState->registration);
    ExitOnFailure(hr, "Failed to set registration paths.");

    if (BURN_MODE_NORMAL == pEngineState->mode)
    {
        // Extract all UX payloads to temp directory.
        hr = PathCreateTempDirectory(NULL, L"UX%d", 999999, &pEngineState->userExperience.sczTempDirectory);
        ExitOnFailure(hr, "Failed to get unique temporary folder for UX.");

        hr = PayloadExtractFromContainer(&pEngineState->userExperience.payloads, NULL, &containerContext, pEngineState->userExperience.sczTempDirectory);
        ExitOnFailure(hr, "Failed to extract UX payloads.");
    }

LExit:
    ContainerClose(&containerContext);
    ReleaseStr(sczStreamName);
    ReleaseMem(pbBuffer);

    return hr;
}

extern "C" void CoreUninitialize(
    __in BURN_ENGINE_STATE* pEngineState
    )
{
    ::DeleteCriticalSection(&pEngineState->userExperience.csEngineActive);
    ::DeleteCriticalSection(&pEngineState->csActive);

    VariablesUninitialize(&pEngineState->variables);
    SearchesUninitialize(&pEngineState->searches);
    UserExperienceUninitialize(&pEngineState->userExperience);
    RegistrationUninitialize(&pEngineState->registration);
    PayloadsUninitialize(&pEngineState->payloads);
    PackagesUninitialize(&pEngineState->packages);
    ReleaseStr(pEngineState->command.wzCommandLine);

    ReleaseStr(pEngineState->log.sczExtension);
    ReleaseStr(pEngineState->log.sczPrefix);
    ReleaseStr(pEngineState->log.sczPath);
    ReleaseStr(pEngineState->log.sczPathVariable);

    ReleaseHandle(pEngineState->hElevatedPipe);
    ReleaseHandle(pEngineState->hElevatedProcess);

    // clear struct
    memset(pEngineState, 0, sizeof(BURN_ENGINE_STATE));
}

extern "C" HRESULT CoreSerializeEngineState(
    __in BURN_ENGINE_STATE* pEngineState,
    __inout BYTE** ppbBuffer,
    __inout SIZE_T* piBuffer
    )
{
    HRESULT hr = S_OK;

    hr = VariableSerialize(&pEngineState->variables, ppbBuffer, piBuffer);
    ExitOnFailure(hr, "Failed to serialize variables.");

LExit:
    return hr;
}

extern "C" HRESULT CoreQueryRegistration(
    __in BURN_ENGINE_STATE* pEngineState
    )
{
    HRESULT hr = S_OK;
    BYTE* pbBuffer = NULL;
    SIZE_T cbBuffer = 0;
    SIZE_T iBuffer = 0;

    // detect resume type
    hr = RegistrationDetectResumeType(&pEngineState->registration, &pEngineState->resumeType);
    ExitOnFailure(hr, "Failed to detect resume type.");

    if (BOOTSTRAPPER_RESUME_TYPE_NONE != pEngineState->resumeType && BOOTSTRAPPER_RESUME_TYPE_INVALID != pEngineState->resumeType)
    {
        // load resume state
        hr = RegistrationLoadState(&pEngineState->registration, &pbBuffer, &cbBuffer);
        if (FAILED(hr))
        {
            TraceError(hr, "Failed to load engine state.");
            pEngineState->resumeType = BOOTSTRAPPER_RESUME_TYPE_INVALID;
            hr = S_OK;
        }
        else
        {
            // deserialize variables
            hr = VariableDeserialize(&pEngineState->variables, pbBuffer, cbBuffer, &iBuffer);
            ExitOnFailure(hr, "Failed to deserialize variables.");
        }
    }

LExit:
    ReleaseBuffer(pbBuffer);

    return hr;
}

extern "C" HRESULT CoreDetect(
    __in BURN_ENGINE_STATE* pEngineState
    )
{
    HRESULT hr = S_OK;
    BOOL fActivated = FALSE;
    BURN_PACKAGE* pPackage = NULL;
    HRESULT hrFirstPackageFailure = S_OK;

    LogId(REPORT_STANDARD, MSG_DETECT_BEGIN, pEngineState->packages.cPackages);

    hr = UserExperienceActivateEngine(&pEngineState->userExperience, &fActivated);
    ExitOnFailure(hr, "Engine cannot start detect because it is busy with another action.");

    int nResult = pEngineState->userExperience.pUserExperience->OnDetectBegin(pEngineState->packages.cPackages);
    hr = HRESULT_FROM_VIEW(nResult);
    ExitOnRootFailure(hr, "UX aborted detect begin.");

    hr = SearchesExecute(&pEngineState->searches, &pEngineState->variables);
    ExitOnFailure(hr, "Failed to execute searches.");

    hr = RegistrationDetectRelatedBundles(&pEngineState->userExperience, &pEngineState->registration);
    ExitOnFailure(hr, "Failed to detect bundles.");

    for (DWORD i = 0; i < pEngineState->packages.cPackages; ++i)
    {
        pPackage = pEngineState->packages.rgPackages + i;

        nResult = pEngineState->userExperience.pUserExperience->OnDetectPackageBegin(pPackage->sczId);
        hr = HRESULT_FROM_VIEW(nResult);
        ExitOnRootFailure(hr, "UX aborted detect package begin.");

        // Detect the cache state of the package.
        hr = DetectPackagePayloadsCached(pPackage);
        ExitOnFailure1(hr, "Failed to detect if payloads are all cached for package: %ls", pPackage->sczId);

        // Use the correct engine to detect the package.
        switch (pPackage->type)
        {
        case BURN_PACKAGE_TYPE_EXE:
            hr = ExeEngineDetectPackage(pPackage, &pEngineState->variables);
            break;

        case BURN_PACKAGE_TYPE_MSI:
            hr = MsiEngineDetectPackage(pPackage, &pEngineState->userExperience);
            break;

        case BURN_PACKAGE_TYPE_MSU:
            hr = MsuEngineDetectPackage(pPackage, &pEngineState->variables);
            break;

        default:
            hr = E_NOTIMPL;
            ExitOnRootFailure(hr, "Package type not supported by detect yet.");
        }

        // If the package detection failed, ensure the package state is set to unknown.
        if (FAILED(hr))
        {
            if (SUCCEEDED(hrFirstPackageFailure))
            {
                hrFirstPackageFailure = hr;
            }

            pPackage->currentState = BOOTSTRAPPER_PACKAGE_STATE_UNKNOWN;
            LogErrorId(hr, MSG_FAILED_DETECT_PACKAGE, pPackage->sczId, NULL, NULL);
        }
        // TODO: consider how to notify the UX that a package is cached.
        //else if (BOOTSTRAPPER_PACKAGE_STATE_CACHED > pPackage->currentState && pPackage->fCached)
        //{
        //     pPackage->currentState = BOOTSTRAPPER_PACKAGE_STATE_CACHED;
        //}

        LogId(REPORT_STANDARD, MSG_DETECTED_PACKAGE, pPackage->sczId, LoggingPackageStateToString(pPackage->currentState), LoggingBoolToString(pPackage->fCached));
        pEngineState->userExperience.pUserExperience->OnDetectPackageComplete(pPackage->sczId, hr, pPackage->currentState);
    }

LExit:
    if (SUCCEEDED(hr))
    {
        hr = hrFirstPackageFailure;
    }

    if (fActivated)
    {
        UserExperienceDeactivateEngine(&pEngineState->userExperience);
    }

    pEngineState->userExperience.pUserExperience->OnDetectComplete(hr);

    LogId(REPORT_STANDARD, MSG_DETECT_COMPLETE, hr);

    return hr;
}

extern "C" HRESULT CorePlan(
    __in BURN_ENGINE_STATE* pEngineState,
    __in BOOTSTRAPPER_ACTION action
    )
{
    HRESULT hr = S_OK;
    BOOL fActivated = FALSE;
    BURN_PACKAGE* pPackage = NULL;
    DWORD dwPackageSequence = 0;
    HANDLE hCheckpointEvent = NULL;
    BOOTSTRAPPER_REQUEST_STATE defaultRequested = BOOTSTRAPPER_REQUEST_STATE_NONE;
    BOOTSTRAPPER_ACTION_STATE executeAction = BOOTSTRAPPER_ACTION_STATE_NONE;
    BOOTSTRAPPER_ACTION_STATE rollbackAction = BOOTSTRAPPER_ACTION_STATE_NONE;
    BOOL fPlannedCachePackage = FALSE;
    BOOL fPlannedCleanPackage = FALSE;

    LogId(REPORT_STANDARD, MSG_PLAN_BEGIN, pEngineState->packages.cPackages, LoggingBurnActionToString(action));

    hr = UserExperienceActivateEngine(&pEngineState->userExperience, &fActivated);
    ExitOnFailure(hr, "Engine cannot start plan because it is busy with another action.");

    int nResult = pEngineState->userExperience.pUserExperience->OnPlanBegin(pEngineState->packages.cPackages);
    hr = HRESULT_FROM_VIEW(nResult);
    ExitOnRootFailure(hr, "UX aborted plan begin.");

    // Always reset the plan.
    PlanUninitialize(&pEngineState->plan);

    // Remember the overall action state in the plan since it shapes the changes
    // we make everywhere.
    pEngineState->plan.action = action;

    // If the registration of this bundle is per-machine then the plan needs to
    // be per-machine as well.
    if (pEngineState->registration.fPerMachine)
    {
        pEngineState->plan.fPerMachine = TRUE;
    }

    // Plan the packages.
    for (DWORD i = 0; i < pEngineState->packages.cPackages; ++i)
    {
        DWORD iPackage = (BOOTSTRAPPER_ACTION_UNINSTALL == action) ? pEngineState->packages.cPackages - 1 - i : i;
        pPackage = pEngineState->packages.rgPackages + iPackage;

        executeAction = BOOTSTRAPPER_ACTION_STATE_NONE;
        rollbackAction = BOOTSTRAPPER_ACTION_STATE_NONE;
        fPlannedCachePackage = FALSE;
        fPlannedCleanPackage = FALSE;

        // Remember the default requested state so the engine doesn't get blamed for planning the wrong thing if the UX changes it.
        hr = PlanDefaultPackageRequestState(pPackage->currentState, action, &pEngineState->variables, pPackage->sczInstallCondition, &defaultRequested);
        ExitOnFailure(hr, "Failed to set default package state.");

        pPackage->requested = defaultRequested;

        nResult = pEngineState->userExperience.pUserExperience->OnPlanPackageBegin(pPackage->sczId, &pPackage->requested);
        hr = HRESULT_FROM_VIEW(nResult);
        ExitOnRootFailure(hr, "UX aborted plan package begin.");

        // If the the package is in a requested state, plan it.
        if (BOOTSTRAPPER_REQUEST_STATE_NONE != pPackage->requested)
        {
            // If the package is not supposed to be absent then ensure it is cached. Even packages that are
            // marked Cache='no' need to be cached so we can check the hash and ensure the correct package is
            // installed. We'll clean up non-cached packages after installing them.
            if (BOOTSTRAPPER_REQUEST_STATE_ABSENT < pPackage->requested && !pPackage->fCached)
            {
                hr = PlanCachePackage(&pEngineState->plan, pPackage, &hCheckpointEvent);
                ExitOnFailure(hr, "Failed to plan cache package.");

                fPlannedCachePackage = TRUE;
            }

            // plan execute actions
            switch (pPackage->type)
            {
            case BURN_PACKAGE_TYPE_EXE:
                hr = ExeEnginePlanPackage(dwPackageSequence, pPackage, &pEngineState->plan, &pEngineState->log, &pEngineState->variables, hCheckpointEvent, &executeAction, &rollbackAction);
                break;

            case BURN_PACKAGE_TYPE_MSI:
                hr = MsiEnginePlanPackage(dwPackageSequence, pPackage, &pEngineState->plan, &pEngineState->log, &pEngineState->variables, hCheckpointEvent, &pEngineState->userExperience, &executeAction, &rollbackAction);
                break;

            //case BURN_PACKAGE_TYPE_MSP:
            //    hr = MspEnginePlanPackage(pPackage, &pEngineState->variables, &pEngineState->plan, hCheckpointEvent, &executeAction, &rollbackAction);
            //    break;

            case BURN_PACKAGE_TYPE_MSU:
                hr = MsuEnginePlanPackage(dwPackageSequence, pPackage, &pEngineState->plan, &pEngineState->log, &pEngineState->variables, hCheckpointEvent, &executeAction, &rollbackAction);
                break;

            default:
                hr = E_UNEXPECTED;
                ExitOnFailure(hr, "Invalid package type.");
            }
            ExitOnFailure1(hr, "Failed to plan execute actions for package: &ls", pPackage->sczId);

            // If we are going to take any action on this package, add progress for it.
            if (BOOTSTRAPPER_ACTION_STATE_NONE != executeAction || BOOTSTRAPPER_ACTION_STATE_NONE != rollbackAction)
            {
                ++pEngineState->plan.cExecutePackagesTotal;
                ++pEngineState->plan.cOverallProgressTicksTotal;
                ++dwPackageSequence;

                // If package is per-machine and is being executed, flag the plan to be per-machine as well.
                if (pPackage->fPerMachine)
                {
                    pEngineState->plan.fPerMachine = TRUE;
                }
            }

            // On removal of the package add the package to the list to be cleaned up. Also, if the package
            // is scheduled to be cached or is somehow already cached but it is not supposed to stay cached
            // then clean up.
            if (BOOTSTRAPPER_REQUEST_STATE_ABSENT == pPackage->requested || ((fPlannedCachePackage || pPackage->fCached) && !pPackage->fCache))
            {
                hr = PlanCleanPackage(&pEngineState->plan, pPackage);
                ExitOnFailure(hr, "Failed to plan clean package.");

                fPlannedCleanPackage = TRUE;
            }
        }

        LogId(REPORT_STANDARD, MSG_PLANNED_PACKAGE, pPackage->sczId, LoggingPackageStateToString(pPackage->currentState), LoggingRequestStateToString(defaultRequested), LoggingRequestStateToString(pPackage->requested), LoggingActionStateToString(executeAction), LoggingActionStateToString(rollbackAction), LoggingBoolToString(fPlannedCachePackage), LoggingBoolToString(fPlannedCleanPackage));

        pEngineState->userExperience.pUserExperience->OnPlanPackageComplete(pPackage->sczId, hr, pPackage->currentState, pPackage->requested, executeAction, rollbackAction);
    }

    // Always plan the clean up of related bundles last.
    for (DWORD i = 0; i < pEngineState->registration.cRelatedBundles; ++i)
    {
        BURN_RELATED_BUNDLE* pRelatedBundle = pEngineState->registration.rgRelatedBundles + i;
        BOOTSTRAPPER_REQUEST_STATE requested = pEngineState->registration.qwVersion > pRelatedBundle->qwVersion ? BOOTSTRAPPER_REQUEST_STATE_ABSENT : BOOTSTRAPPER_REQUEST_STATE_NONE;

        BOOTSTRAPPER_REQUEST_STATE defaultRequested = requested;

        nResult = pEngineState->userExperience.pUserExperience->OnPlanRelatedBundle(pRelatedBundle->sczId, &requested);
        hr = HRESULT_FROM_VIEW(nResult);
        ExitOnRootFailure(hr, "UX aborted plan related bundle.");

        // Log when the UX changed the bundle state so the engine doesn't get blamed for planning the wrong thing.
        if (requested != defaultRequested)
        {
            LogId(REPORT_STANDARD, MSG_PLANNED_BUNDLE_UX_CHANGED_REQUEST, pRelatedBundle->sczId, LoggingRequestStateToString(requested), LoggingRequestStateToString(defaultRequested));
        }

        if (BOOTSTRAPPER_REQUEST_STATE_ABSENT == requested)
        {
            hr = PlanCleanBundle(&pEngineState->plan, pRelatedBundle);
            ExitOnFailure(hr, "Failed to plan clean bundle.");
        }
    }

LExit:
    if (fActivated)
    {
        UserExperienceDeactivateEngine(&pEngineState->userExperience);
    }

    pEngineState->userExperience.pUserExperience->OnPlanComplete(hr);

    LogId(REPORT_STANDARD, MSG_PLAN_COMPLETE, hr);

    return hr;
}

extern "C" HRESULT CoreApply(
    __in BURN_ENGINE_STATE* pEngineState,
    __in_opt HWND hwndParent
    )
{
    HRESULT hr = S_OK;
    BOOL fActivated = FALSE;
    DWORD cOverallProgressTicks = 0;
    HANDLE hCacheThread = NULL;
    BOOL fRegistered = FALSE;
    BOOL fRollback = FALSE;
    BOOL fSuspend = FALSE;
    BOOTSTRAPPER_APPLY_RESTART restart = BOOTSTRAPPER_APPLY_RESTART_NONE;
    BURN_CACHE_THREAD_CONTEXT cacheThreadContext = { };

    LogId(REPORT_STANDARD, MSG_APPLY_BEGIN);

    hr = UserExperienceActivateEngine(&pEngineState->userExperience, &fActivated);
    ExitOnFailure(hr, "Engine cannot start apply because it is busy with another action.");

    int nResult = pEngineState->userExperience.pUserExperience->OnApplyBegin();
    hr = HRESULT_FROM_VIEW(nResult);
    ExitOnRootFailure(hr, "UX aborted apply begin.");

    // If the plan contains per-machine contents but we have not created
    // the elevated process yet, let's make that happen.
    if (pEngineState->plan.fPerMachine && !pEngineState->hElevatedProcess)
    {
        hr = ApplyElevate(&pEngineState->userExperience, hwndParent, &pEngineState->hElevatedProcess, &pEngineState->hElevatedPipe);
        ExitOnFailure(hr, "Failed to elevate.");
    }

    hr = ApplyRegister(pEngineState);
    ExitOnFailure(hr, "Failed to register bundle.");
    fRegistered = TRUE;

    // cache actions
    if (pEngineState->fParallelCacheAndExecute)
    {
        cacheThreadContext.pEngineState = pEngineState;
        cacheThreadContext.pcOverallProgressTicks = &cOverallProgressTicks;

        // create extraction thread
        hCacheThread = ::CreateThread(NULL, 0, CacheThreadProc, &cacheThreadContext, 0, NULL);
        ExitOnNullWithLastError(hCacheThread, hr, "Failed to create extraction thread.");
    }
    else
    {
        hr = ApplyCache(&pEngineState->userExperience, &pEngineState->plan, pEngineState->hElevatedPipe, &cOverallProgressTicks);
        ExitOnFailure(hr, "Failed to cache packages.");
    }

    // execute
    hr = ApplyExecute(pEngineState, hCacheThread, &cOverallProgressTicks, &fRollback, &fSuspend, &restart);
    ExitOnFailure(hr, "Failed to execute apply.");

    // wait for cache thread to terminate
    if (hCacheThread)
    {
        if (WAIT_OBJECT_0 != ::WaitForSingleObject(hCacheThread, INFINITE))
        {
            ExitWithLastError(hr, "Failed to wait for cache thread to terminate.");
        }

        if (!::GetExitCodeThread(hCacheThread, (DWORD*)&hr))
        {
            ExitWithLastError(hr, "Failed to get cache thread exit code.");
        }
    }

    if (fSuspend || BOOTSTRAPPER_APPLY_RESTART_INITIATED == restart)
    {
        ExitFunction();
    }

    ApplyClean(&pEngineState->userExperience, &pEngineState->plan, pEngineState->hElevatedPipe);

LExit:
    if (fRegistered)
    {
        ApplyUnregister(pEngineState, fRollback, fSuspend, BOOTSTRAPPER_APPLY_RESTART_INITIATED == restart);
    }

    if (fActivated)
    {
        UserExperienceDeactivateEngine(&pEngineState->userExperience);
    }

    ReleaseHandle(hCacheThread);

    nResult = pEngineState->userExperience.pUserExperience->OnApplyComplete(hr, restart);
    if (IDRESTART == nResult || BOOTSTRAPPER_APPLY_RESTART_INITIATED == restart)
    {
        pEngineState->fRestart = TRUE;
    }

    LogId(REPORT_STANDARD, MSG_APPLY_COMPLETE, hr, LoggingBoolToString(pEngineState->fRestart));

    return hr;
}

extern "C" HRESULT CoreQuit(
    __in BURN_ENGINE_STATE* /*pEngineState*/,
    __in int nExitCode
    )
{
    HRESULT hr = S_OK;

    // TODO: SFBUG:3007072 - save engine state if the bundle is still cached.

    LogId(REPORT_STANDARD, MSG_SHUTDOWN, nExitCode);

    ::PostQuitMessage(nExitCode); // go bye-bye.

    return hr;
}


// internal helper functions

static HRESULT ParseCommandLine(
    __in_z_opt LPCWSTR wzCommandLine,
    __in BOOTSTRAPPER_COMMAND* pCommand,
    __out BURN_MODE* pMode,
    __out BOOL *pfAppend,
    __inout_z LPWSTR* psczLogFile,
    __inout_z LPWSTR* psczElevatedPipeName,
    __inout_z LPWSTR* psczElevatedToken
    )
{
    HRESULT hr = S_OK;
    int argc = 0;
    LPWSTR* argv = NULL;
    BOOL fUnknownArg = FALSE;

    if (wzCommandLine && *wzCommandLine)
    {
        argv = ::CommandLineToArgvW(wzCommandLine, &argc);
        ExitOnNullWithLastError(argv, hr, "Failed to get command line.");
    }

    for (int i = 0; i < argc; ++i)
    {
        fUnknownArg = FALSE;

        if (argv[i][0] == L'-' || argv[i][0] == L'/')
        {
            if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, &argv[i][1], -1, L"l", -1) ||
                CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, &argv[i][1], -1, L"log", -1))
            {
                *pfAppend = FALSE;

                if (i + 1 >= argc)
                {
                    ExitOnRootFailure(hr = E_INVALIDARG, "Must specify a path for log.");
                }

                ++i;

                hr = StrAllocString(psczLogFile, argv[i], 0);
                ExitOnFailure(hr, "Failed to copy log file path.");
            }
            else if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, &argv[i][1], -1, L"?", -1) ||
                     CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, &argv[i][1], -1, L"h", -1) ||
                     CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, &argv[i][1], -1, L"help", -1))
            {
                pCommand->action = BOOTSTRAPPER_ACTION_HELP;
            }
            else if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, &argv[i][1], -1, L"q", -1) ||
                     CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, &argv[i][1], -1, L"quiet", -1) ||
                     CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, &argv[i][1], -1, L"s", -1) ||
                     CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, &argv[i][1], -1, L"silent", -1))
            {
                pCommand->display = BOOTSTRAPPER_DISPLAY_NONE;

                if (BOOTSTRAPPER_RESTART_UNKNOWN == pCommand->restart)
                {
                    pCommand->restart = BOOTSTRAPPER_RESTART_AUTOMATIC;
                }
            }
            else if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, &argv[i][1], -1, L"passive", -1))
            {
                pCommand->display = BOOTSTRAPPER_DISPLAY_PASSIVE;

                if (BOOTSTRAPPER_RESTART_UNKNOWN == pCommand->restart)
                {
                    pCommand->restart = BOOTSTRAPPER_RESTART_AUTOMATIC;
                }
            }
            else if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, &argv[i][1], -1, L"norestart", -1))
            {
                pCommand->restart = BOOTSTRAPPER_RESTART_NEVER;
            }
            else if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, &argv[i][1], -1, L"forcerestart", -1))
            {
                pCommand->restart = BOOTSTRAPPER_RESTART_ALWAYS;
            }
            else if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, &argv[i][1], -1, L"promptrestart", -1))
            {
                pCommand->restart = BOOTSTRAPPER_RESTART_PROMPT;
            }
            else if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, &argv[i][1], -1, L"uninstall", -1))
            {
                if (BOOTSTRAPPER_ACTION_HELP != pCommand->action)
                {
                    pCommand->action = BOOTSTRAPPER_ACTION_UNINSTALL;
                }
            }
            else if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, &argv[i][1], -1, L"repair", -1))
            {
                if (BOOTSTRAPPER_ACTION_HELP != pCommand->action)
                {
                    pCommand->action = BOOTSTRAPPER_ACTION_REPAIR;
                }
            }
            else if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, &argv[i][1], -1, L"package", -1) ||
                     CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, &argv[i][1], -1, L"update", -1))
            {
                if (BOOTSTRAPPER_ACTION_UNKNOWN == pCommand->action)
                {
                    pCommand->action = BOOTSTRAPPER_ACTION_INSTALL;
                }
            }
            else if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, &argv[i][1], -1, BURN_COMMANDLINE_SWITCH_LOG_APPEND, -1))
            {
                if (i + 1 >= argc)
                {
                    ExitOnRootFailure(hr = E_INVALIDARG, "Must specify a path for append log.");
                }

                ++i;

                hr = StrAllocString(psczLogFile, argv[i], 0);
                ExitOnFailure(hr, "Failed to copy append log file path.");

                *pfAppend = TRUE;
            }
            else if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, &argv[i][1], -1, BURN_COMMANDLINE_SWITCH_ELEVATED, -1))
            {
                if (i + 2 >= argc)
                {
                    ExitOnRootFailure(hr = E_INVALIDARG, "Must specify the parent and child elevation tokens.");
                }

                *pMode = BURN_MODE_ELEVATED;

                ++i;

                hr = StrAllocString(psczElevatedPipeName, argv[i], 0);
                ExitOnFailure(hr, "Failed to copy elevated pipe name.");

                ++i;

                hr = StrAllocString(psczElevatedToken, argv[i], 0);
                ExitOnFailure(hr, "Failed to copy elevation token.");
            }
            else if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, &argv[i][1], -1, BURN_COMMANDLINE_SWITCH_UNCACHE_PER_MACHINE, -1))
            {
                *pMode = BURN_MODE_UNCACHE_PER_MACHINE;
            }
            else if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, &argv[i][1], -1, BURN_COMMANDLINE_SWITCH_UNCACHE_PER_USER, -1))
            {
                *pMode = BURN_MODE_UNCACHE_PER_USER;
            }
            else
            {
                fUnknownArg = TRUE;
            }
        }
        else
        {
            fUnknownArg = TRUE;
        }

        if (fUnknownArg)
        {
            if (pCommand->wzCommandLine && *pCommand->wzCommandLine)
            {
                hr = StrAllocConcat(&pCommand->wzCommandLine, L" ", 0);
                ExitOnFailure(hr, "Failed to append space.");
            }

            // Remember command-line switch to pass off to UX.
            hr = StrAllocConcat(&pCommand->wzCommandLine, &argv[i][0], 0);
            ExitOnFailure(hr, "Failed to copy command line parameter.");
        }
    }

    // Set the defaults if nothing was set above.
    if (BOOTSTRAPPER_ACTION_UNKNOWN == pCommand->action)
    {
        pCommand->action = BOOTSTRAPPER_ACTION_INSTALL;
    }

    if (BOOTSTRAPPER_DISPLAY_UNKNOWN == pCommand->display)
    {
        pCommand->display = BOOTSTRAPPER_DISPLAY_FULL;
    }

    if (BOOTSTRAPPER_RESTART_UNKNOWN == pCommand->restart)
    {
        pCommand->restart = BOOTSTRAPPER_RESTART_PROMPT;
    }

LExit:
    if (argv)
    {
        ::LocalFree(argv);
    }

    return hr;
}

static HRESULT DetectPackagePayloadsCached(
    __in BURN_PACKAGE* pPackage
    )
{
    HRESULT hr = S_OK;
    LPWSTR sczCachePath = NULL;
    BOOL fAllPayloadsCached = FALSE;
    LPWSTR sczPayloadCachePath = NULL;
    LONGLONG llSize = 0;

    if (pPackage->sczCacheId && *pPackage->sczCacheId)
    {
        hr = CacheGetCompletedPath(pPackage->fPerMachine, pPackage->sczCacheId, &sczCachePath);
        ExitOnFailure(hr, "Failed to get completed cache path.");

        fAllPayloadsCached = TRUE; // assume all payloads will be cached.

        for (DWORD i = 0; i < pPackage->cPayloads; ++i)
        {
            BURN_PACKAGE_PAYLOAD* pPackagePayload = pPackage->rgPayloads + i;

            hr = PathConcat(sczCachePath, pPackagePayload->pPayload->sczFilePath, &sczPayloadCachePath);
            ExitOnFailure(hr, "Failed to concat payload cache path.");

            // TODO: should we do a full on hash verification on the file to ensure the exact right
            //       file is cached?
            hr = FileSize(sczPayloadCachePath, &llSize);
            if (SUCCEEDED(hr) && static_cast<DWORD64>(llSize) == pPackagePayload->pPayload->qwFileSize)
            {
                // TODO: should we log that the payload was cached?
                pPackagePayload->fCached = TRUE;
            }
            else
            {
                fAllPayloadsCached = FALSE; // found a payload that was not cached so our assumption above was wrong.

                // TODO: should we log that the payload was not cached?
                hr = S_OK;
            }
        }
    }

    pPackage->fCached = fAllPayloadsCached;

LExit:
    ReleaseStr(sczPayloadCachePath);
    ReleaseStr(sczCachePath);
    return hr;
}

static DWORD WINAPI CacheThreadProc(
    __in LPVOID lpThreadParameter
    )
{
    HRESULT hr = S_OK;
    BURN_CACHE_THREAD_CONTEXT* pContext = reinterpret_cast<BURN_CACHE_THREAD_CONTEXT*>(lpThreadParameter);
    BURN_ENGINE_STATE* pEngineState = pContext->pEngineState;
    DWORD* pcOverallProgressTicks = pContext->pcOverallProgressTicks;
    BOOL fComInitialized = FALSE;

    // initialize COM
    hr = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
    ExitOnFailure(hr, "Failed to initialize COM.");
    fComInitialized = TRUE;

    // cache packages
    hr = ApplyCache(&pEngineState->userExperience, &pEngineState->plan, pEngineState->hElevatedPipe, pcOverallProgressTicks);
    ExitOnFailure(hr, "Failed to cache packages.");

LExit:
    if (fComInitialized)
    {
        ::CoUninitialize();
    }

    return (DWORD)hr;
}
