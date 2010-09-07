//-------------------------------------------------------------------------------------------------
// <copyright file="apply.cpp" company="Microsoft">
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
//    Apply phase functions.
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"


// structs

struct BURN_CACHE_ACQUIRE_PROGRESS_CONTEXT
{
    BURN_USER_EXPERIENCE* pUX;
    BURN_CONTAINER* pContainer;
    BURN_PACKAGE* pPackage;
    BURN_PAYLOAD* pPayload;
    DWORD64 qwCacheProgress;
    DWORD64 qwTotalCacheSize;

    BOOL fCancel;
    BOOL fError;
};

typedef struct _BURN_EXECUTE_CONTEXT
{
    BURN_USER_EXPERIENCE* pUX;
    BURN_PACKAGE* pExecutingPackage;
    DWORD cExecutedPackages;
    DWORD cExecutePackagesTotal;
    DWORD* pcOverallProgressTicks;
} BURN_EXECUTE_CONTEXT;


// internal function declarations

static HRESULT ExtractContainer(
    __in BURN_USER_EXPERIENCE* pUX,
    __in BURN_CONTAINER* pContainer,
    __in_z_opt LPCWSTR wzContainerPath,
    __in_ecount(cExtractPayloads) BURN_EXTRACT_PAYLOAD* rgExtractPayloads,
    __in DWORD cExtractPayloads
    );
static HRESULT AcquireContainerOrPayload(
    __in BURN_USER_EXPERIENCE* pUX,
    __in_opt BURN_CONTAINER* pContainer,
    __in_opt BURN_PACKAGE* pPackage,
    __in_opt BURN_PAYLOAD* pPayload,
    __in LPCWSTR wzDestinationPath,
    __in DWORD64 qwTotalCacheSize,
    __inout DWORD64* pqwTotalCacheProgress
    );
static HRESULT PromptForSource(
    __in BURN_USER_EXPERIENCE* pUX,
    __in_z LPCWSTR wzPackageOrContainerId,
    __in_z_opt LPCWSTR wzPayloadId,
    __in_z LPCWSTR wzLocalSource,
    __in_z_opt LPCWSTR wzDownloadSource,
    __out BOOL* pfRetry,
    __out BOOL* pfDownload
    );
static HRESULT CopyPayload(
    __in BURN_CACHE_ACQUIRE_PROGRESS_CONTEXT* pProgress,
    __in_z LPCWSTR wzSourcePath,
    __in_z LPCWSTR wzDestinationPath,
    __out BOOL* pfRetry
    );
static HRESULT DownloadPayload(
    __in BURN_CACHE_ACQUIRE_PROGRESS_CONTEXT* pProgress,
    __in_z LPCWSTR wzDestinationPath,
    __out BOOL* pfRetry
    );
static DWORD CALLBACK CacheProgressRoutine(
    __in LARGE_INTEGER TotalFileSize,
    __in LARGE_INTEGER TotalBytesTransferred,
    __in LARGE_INTEGER StreamSize,
    __in LARGE_INTEGER StreamBytesTransferred,
    __in DWORD dwStreamNumber,
    __in DWORD dwCallbackReason,
    __in HANDLE hSourceFile,
    __in HANDLE hDestinationFile,
    __in_opt LPVOID lpData
    );
static HRESULT DoExecuteActions(
    __in BURN_ENGINE_STATE* pEngineState,
    __in HANDLE hCacheThread,
    __in BURN_EXECUTE_CONTEXT* pContext,
    __out DWORD* pdwCheckpoint,
    __out BOOL* pfSuspend,
    __out BOOTSTRAPPER_APPLY_RESTART* pRestart
    );
static HRESULT DoRollbackActions(
    __in BURN_ENGINE_STATE* pEngineState,
    __in BURN_EXECUTE_CONTEXT* pContext,
    __in DWORD dwCheckpoint,
    __out BOOTSTRAPPER_APPLY_RESTART* pRestart
    );
static HRESULT ExecuteExePackage(
    __in BURN_ENGINE_STATE* pEngineState,
    __in BURN_EXECUTE_ACTION* pExecuteAction,
    __in BURN_EXECUTE_CONTEXT* pContext,
    __in BOOL fRollback,
    __out BOOL* pfSuspend,
    __out BOOTSTRAPPER_APPLY_RESTART* pRestart
    );
static HRESULT ExecuteMsiPackage(
    __in BURN_ENGINE_STATE* pEngineState,
    __in BURN_EXECUTE_ACTION* pExecuteAction,
    __in BURN_EXECUTE_CONTEXT* pContext,
    __in BOOL fRollback,
    __out BOOL* pfSuspend,
    __out BOOTSTRAPPER_APPLY_RESTART* pRestart
    );
static HRESULT ExecuteMsuPackage(
    __in BURN_ENGINE_STATE* pEngineState,
    __in BURN_EXECUTE_ACTION* pExecuteAction,
    __in BURN_EXECUTE_CONTEXT* pContext,
    __in BOOL fRollback,
    __out BOOL* pfSuspend,
    __out BOOTSTRAPPER_APPLY_RESTART* pRestart
    );
static int GenericExecuteProgress(
    __in LPVOID pvContext,
    __in DWORD dwProgress,
    __in DWORD dwTotal
    );
static int MsiExecuteMessageHandler(
    __in LPVOID pvContext,
    __in BURN_MSI_EXECUTE_MESSAGE* pMessage
    );
static HRESULT ReportOverallProgressTicks(
    __in BURN_USER_EXPERIENCE* pUX,
    __in DWORD cOverallProgressTicksTotal,
    __in DWORD cOverallProgressTicks
    );
static HRESULT ExecutePackageComplete(
    __in BURN_USER_EXPERIENCE* pUX,
    __in BURN_PACKAGE* pPackage,
    __in HRESULT hrOverall,
    __in HRESULT hrExecute,
    __inout BOOTSTRAPPER_APPLY_RESTART* pRestart,
    __out BOOL* pfSuspend
    );


// function definitions

extern "C" HRESULT ApplyElevate(
    __in BURN_USER_EXPERIENCE* pUX,
    __in HWND hwndParent,
    __out HANDLE* phElevatedProcess,
    __out HANDLE* phElevatedPipe
    )
{
    HRESULT hr = S_OK;
    int nResult = IDOK;

    // TODO: enable this call
    // nResult = pUX->OnElevate();
    hr = HRESULT_FROM_VIEW(nResult);
    ExitOnRootFailure(hr, "UX aborted elevation requirement.");

    do
    {
        nResult = IDOK;

        hr = ElevationParentProcessConnect(hwndParent, phElevatedProcess, phElevatedPipe);
        if (HRESULT_FROM_WIN32(ERROR_ALREADY_INITIALIZED) == hr)
        {
            hr = S_OK;
        }
        else if (HRESULT_FROM_WIN32(ERROR_CANCELLED) == hr)
        {
            nResult = pUX->pUserExperience->OnError(NULL, ERROR_CANCELLED, NULL, MB_ICONERROR | MB_RETRYCANCEL);
        }
    } while (IDRETRY == nResult);

LExit:
    return hr;
}

extern "C" HRESULT ApplyRegister(
    __in BURN_ENGINE_STATE* pEngineState
    )
{
    HRESULT hr = S_OK;
    BYTE* pbBuffer = NULL;
    SIZE_T cbBuffer = 0;

    int nResult = pEngineState->userExperience.pUserExperience->OnRegisterBegin();
    hr = HRESULT_FROM_VIEW(nResult);
    ExitOnRootFailure(hr, "UX aborted register begin.");

    if (BOOTSTRAPPER_RESUME_TYPE_NONE == pEngineState->resumeType)
    {
        // begin new session
        hr =  RegistrationSessionBegin(&pEngineState->registration, &pEngineState->userExperience, pEngineState->plan.action, 0, FALSE);
        ExitOnFailure(hr, "Failed to begin registration session.");

        if (pEngineState->registration.fPerMachine)
        {
            hr = ElevationSessionBegin(pEngineState->hElevatedPipe, pEngineState->plan.action, 0);
            ExitOnFailure(hr, "Failed to begin registration session in per-machine process.");
        }
    }
    else
    {
        // resume previous session
        hr =  RegistrationSessionResume(&pEngineState->registration, pEngineState->plan.action, FALSE);
        ExitOnFailure(hr, "Failed to resume registration session.");

        if (pEngineState->registration.fPerMachine)
        {
            hr =  ElevationSessionResume(pEngineState->hElevatedPipe, pEngineState->plan.action);
            ExitOnFailure(hr, "Failed to resume registration session in per-machine process.");
        }
    }

    // save engine state
    hr = CoreSerializeEngineState(pEngineState, &pbBuffer, &cbBuffer);
    ExitOnFailure(hr, "Failed to serialize engine state.");

    if (pEngineState->registration.fPerMachine)
    {
        hr = ElevationSaveState(pEngineState->hElevatedPipe, pbBuffer, cbBuffer);
        ExitOnFailure(hr, "Failed to save engine state in per-machine process.");
    }
    else
    {
        hr = RegistrationSaveState(&pEngineState->registration, pbBuffer, cbBuffer);
        ExitOnFailure(hr, "Failed to save engine state.");
    }

LExit:
    pEngineState->userExperience.pUserExperience->OnRegisterComplete(hr);
    ReleaseBuffer(pbBuffer);

    return hr;
}

extern "C" HRESULT ApplyUnregister(
    __in BURN_ENGINE_STATE* pEngineState,
    __in BOOL fRollback,
    __in BOOL fSuspend,
    __in BOOL fRestartInitiated
    )
{
    HRESULT hr = S_OK;

    pEngineState->userExperience.pUserExperience->OnUnregisterBegin();

    // suspend or end session
    if (fSuspend || fRestartInitiated)
    {
        if (pEngineState->registration.fPerMachine)
        {
            hr = ElevationSessionSuspend(pEngineState->hElevatedPipe, pEngineState->plan.action, fRestartInitiated);
            ExitOnFailure(hr, "Failed to suspend session in per-machine process.");
        }

        hr = RegistrationSessionSuspend(&pEngineState->registration, pEngineState->plan.action, fRestartInitiated, FALSE);
        ExitOnFailure(hr, "Failed to suspend session in per-user process.");
    }
    else
    {
        if (pEngineState->registration.fPerMachine)
        {
            hr = ElevationSessionEnd(pEngineState->hElevatedPipe, pEngineState->plan.action, fRollback);
            ExitOnFailure(hr, "Failed to end session in per-machine process.");
        }

        hr = RegistrationSessionEnd(&pEngineState->registration, pEngineState->plan.action, fRollback, FALSE);
        ExitOnFailure(hr, "Failed to end session in per-user process.");
    }

LExit:
    pEngineState->userExperience.pUserExperience->OnUnregisterComplete(hr);

    return hr;
}

extern "C" HRESULT ApplyCache(
    __in BURN_USER_EXPERIENCE* pUX,
    __in BURN_PLAN* pPlan,
    __in HANDLE hPipe,
    __inout DWORD* pcOverallProgressTicks
    )
{
    HRESULT hr = S_OK;
    BURN_PACKAGE* pStartedPackage = NULL;
    DWORD64 qwCacheProgress = 0;

    int nResult = pUX->pUserExperience->OnCacheBegin();
    hr = HRESULT_FROM_VIEW(nResult);
    ExitOnRootFailure(hr, "UX aborted cache.");

    // cache actions
    for (DWORD i = 0; i < pPlan->cCacheActions; ++i)
    {
        BURN_CACHE_ACTION* pCacheAction = &pPlan->rgCacheActions[i];

        switch (pCacheAction->type)
        {
        case BURN_CACHE_ACTION_TYPE_PACKAGE_START:
            pStartedPackage = pCacheAction->packageStart.pPackage;

            nResult = pUX->pUserExperience->OnCachePackageBegin(pStartedPackage->sczId, pCacheAction->packageStart.cCachePayloads, pCacheAction->packageStart.qwCachePayloadSizeTotal);
            hr = HRESULT_FROM_VIEW(nResult);
            ExitOnRootFailure(hr, "UX aborted package begin cache.");
            break;

        case BURN_CACHE_ACTION_TYPE_ACQUIRE_CONTAINER:
            hr = AcquireContainerOrPayload(pUX, pCacheAction->resolveContainer.pContainer, NULL, NULL, pCacheAction->resolveContainer.sczUnverifiedPath, pPlan->qwCacheSizeTotal, &qwCacheProgress);
            ExitOnFailure(hr, "Failed to acquire container.");
            break;

        case BURN_CACHE_ACTION_TYPE_EXTRACT_CONTAINER:
            hr = ExtractContainer(pUX, pCacheAction->extractContainer.pContainer, pCacheAction->extractContainer.sczContainerUnverifiedPath, pCacheAction->extractContainer.rgPayloads, pCacheAction->extractContainer.cPayloads);
            ExitOnFailure(hr, "Failed to extract container.");
            break;

        case BURN_CACHE_ACTION_TYPE_ACQUIRE_PAYLOAD:
            hr = AcquireContainerOrPayload(pUX, NULL, pCacheAction->resolvePayload.pPackage, pCacheAction->resolvePayload.pPayload, pCacheAction->resolvePayload.sczUnverifiedPath, pPlan->qwCacheSizeTotal, &qwCacheProgress);
            ExitOnFailure(hr, "Failed to acquire payload.");
            break;

        case BURN_CACHE_ACTION_TYPE_CACHE_PAYLOAD:
            if (pCacheAction->cachePayload.pPackage->fPerMachine)
            {
                hr = ElevationCachePayload(hPipe, pCacheAction->cachePayload.pPackage, pCacheAction->cachePayload.pPayload, pCacheAction->cachePayload.sczUnverifiedPath, pCacheAction->cachePayload.fMove);
                ExitOnFailure(hr, "Failed to cache per-machine payload.");
            }
            else
            {
                hr = CachePayload(pCacheAction->cachePayload.pPackage, pCacheAction->cachePayload.pPayload, pCacheAction->cachePayload.sczUnverifiedPath, pCacheAction->cachePayload.fMove);
                ExitOnFailure(hr, "Failed to cache per-user payload.");
            }
            break;

        case BURN_CACHE_ACTION_TYPE_CHECKPOINT:
            AssertSz(pStartedPackage == pCacheAction->checkpoint.pPackage, "Expected package started cached to be the same as the package checkpointed.");

            ++(*pcOverallProgressTicks);

            hr = ReportOverallProgressTicks(pUX, pPlan->cOverallProgressTicksTotal, *pcOverallProgressTicks);
            ExitOnRootFailure(hr, "UX aborted package cache progress.");

            pUX->pUserExperience->OnCachePackageComplete(pStartedPackage->sczId, hr);
            pStartedPackage = NULL;

            if (!::SetEvent(pCacheAction->checkpoint.hEvent))
            {
                ExitWithLastError(hr, "Failed to set checkpoint event.");
            }
            break;

        default:
            hr = E_UNEXPECTED;
            ExitOnFailure1(hr, "Invalid cache action: %d", pCacheAction->type);
        }
    }

LExit:
    if (pStartedPackage)
    {
        pUX->pUserExperience->OnCachePackageComplete(pStartedPackage->sczId, hr);
    }

    pUX->pUserExperience->OnCacheComplete(hr);
    return hr;
}

extern "C" HRESULT ApplyExecute(
    __in BURN_ENGINE_STATE* pEngineState,
    __in HANDLE hCacheThread,
    __inout DWORD* pcOverallProgressTicks,
    __out BOOL* pfRollback,
    __out BOOL* pfSuspend,
    __out BOOTSTRAPPER_APPLY_RESTART* pRestart
    )
{
    HRESULT hr = S_OK;
    DWORD dwCheckpoint = 0;
    BURN_EXECUTE_CONTEXT context = { };
    int nResult = 0;

    context.pUX = &pEngineState->userExperience;
    context.cExecutePackagesTotal = pEngineState->plan.cExecutePackagesTotal;
    context.pcOverallProgressTicks = pcOverallProgressTicks;

    // send execute begin to UX
    nResult = pEngineState->userExperience.pUserExperience->OnExecuteBegin(pEngineState->plan.cExecutePackagesTotal);
    hr = HRESULT_FROM_VIEW(nResult);
    ExitOnRootFailure(hr, "UX aborted execute begin.");

    // do execute actions
    hr = DoExecuteActions(pEngineState, hCacheThread, &context, &dwCheckpoint, pfSuspend, pRestart);
    TraceError(hr, "Failure during execution.");

    if (FAILED(hr) && !pEngineState->fDisableRollback)
    {
        *pfRollback = TRUE;

        // do rollback actions
        HRESULT hrRollback = DoRollbackActions(pEngineState, &context, dwCheckpoint, pRestart);
        UNREFERENCED_PARAMETER(hrRollback);
    }

LExit:
    // send execute complete to UX
    pEngineState->userExperience.pUserExperience->OnExecuteComplete(hr);

    return hr;
}

extern "C" void ApplyClean(
    __in BURN_USER_EXPERIENCE* /*pUX*/,
    __in BURN_PLAN* pPlan,
    __in HANDLE hPipe
    )
{
    HRESULT hr = S_OK;

    for (DWORD i = 0; i < pPlan->cCleanActions; ++i)
    {
        BURN_CLEAN_ACTION* pCleanAction = pPlan->rgCleanActions + i;

        switch (pCleanAction->type)
        {
        case BURN_CLEAN_ACTION_TYPE_BUNDLE:
            if (pPlan->fPerMachine)
            {
                hr = ElevationCleanBundle(hPipe, pCleanAction);
            }

            hr = ApplyCleanBundle(FALSE, pCleanAction->bundle.pBundle);
            break;

        case BURN_CLEAN_ACTION_TYPE_PACKAGE:
            if (pCleanAction->package.pPackage->fPerMachine)
            {
                hr = ElevationCleanPackage(hPipe, pCleanAction);
            }
            else
            {
                hr = CacheRemovePackage(FALSE, pCleanAction->package.pPackage->sczCacheId);
            }
            break;
        }
    }
}

extern "C" HRESULT ApplyCleanBundle(
    __in BOOL fPerMachine,
    __in BURN_RELATED_BUNDLE* pRelatedBundle
    )
{
    HRESULT hr = S_OK;
    LPWSTR sczCommand = NULL;
    STARTUPINFOW si = { };
    PROCESS_INFORMATION pi = { };
    DWORD dwExitCode = 0;

    hr = StrAllocFormatted(&sczCommand, L"\"%ls\" -%ls", pRelatedBundle->sczCachePath, fPerMachine ? BURN_COMMANDLINE_SWITCH_UNCACHE_PER_MACHINE : BURN_COMMANDLINE_SWITCH_UNCACHE_PER_USER);
    ExitOnFailure1(hr, "Failed to format clean command for bundle: %ls", pRelatedBundle->sczId);

    // Launch the related bundle to clean itself up.
    si.cb = sizeof(si);
    if (!::CreateProcessW(pRelatedBundle->sczCachePath, sczCommand, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi))
    {
        ExitWithLastError1(hr, "Failed to start clean bundle command: %ls", sczCommand);
    }

    // Wait for bundle to finish.
    hr = ProcWaitForCompletion(pi.hProcess, INFINITE, &dwExitCode);
    ExitOnFailure1(hr, "Failed to wait for clean bundle to complete: %ls", sczCommand);

    hr = HRESULT_FROM_WIN32(dwExitCode);

LExit:
    ReleaseHandle(pi.hThread);
    ReleaseHandle(pi.hProcess);
    ReleaseStr(sczCommand);
    return hr;
}


// internal helper functions

static HRESULT ExtractContainer(
    __in BURN_USER_EXPERIENCE* /*pUX*/,
    __in BURN_CONTAINER* pContainer,
    __in_z_opt LPCWSTR wzContainerPath,
    __in_ecount(cExtractPayloads) BURN_EXTRACT_PAYLOAD* rgExtractPayloads,
    __in DWORD cExtractPayloads
    )
{
    HRESULT hr = S_OK;
    BURN_CONTAINER_CONTEXT context = { };
    LPWSTR sczExtractPayloadId = NULL;

    hr = ContainerOpen(&context, pContainer, wzContainerPath);
    ExitOnFailure1(hr, "Failed to open container: %ls.", pContainer->sczId);

    while (S_OK == (hr = ContainerNextStream(&context, &sczExtractPayloadId)))
    {
        BOOL fExtracted = FALSE;

        for (DWORD iExtract = 0; iExtract < cExtractPayloads; ++iExtract)
        {
            BURN_EXTRACT_PAYLOAD* pExtract = rgExtractPayloads + iExtract;
            if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, 0, sczExtractPayloadId, -1, pExtract->pPayload->sczSourcePath, -1))
            {
                // TODO: Send progress when extracting stream to file.
                hr = ContainerStreamToFile(&context, pExtract->sczUnverifiedPath);
                ExitOnFailure2(hr, "Failed to extract payload: %ls from container: %ls", sczExtractPayloadId, pContainer->sczId);

                fExtracted = TRUE;
                break;
            }
        }

        if (!fExtracted)
        {
            hr = ContainerSkipStream(&context);
            ExitOnFailure2(hr, "Failed to skip the extraction of payload: %ls from container: %ls", sczExtractPayloadId, pContainer->sczId);
        }
    }

    if (E_NOMOREITEMS == hr)
    {
        hr = S_OK;
    }
    ExitOnFailure1(hr, "Failed to extract all payloads from container: %ls", pContainer->sczId);

LExit:
    ReleaseStr(sczExtractPayloadId);
    ContainerClose(&context);

    return hr;
}

static HRESULT AcquireContainerOrPayload(
    __in BURN_USER_EXPERIENCE* pUX,
    __in_opt BURN_CONTAINER* pContainer,
    __in_opt BURN_PACKAGE* pPackage,
    __in_opt BURN_PAYLOAD* pPayload,
    __in LPCWSTR wzDestinationPath,
    __in DWORD64 qwTotalCacheSize,
    __inout DWORD64* pqwCacheProgress
    )
{
    AssertSz((pContainer || (pPackage && pPayload)) && !(pContainer && pPackage && pPayload), "Must provide a container or a package and a payload.");

    HRESULT hr = S_OK;
    LPWSTR wzPackageOrContainerId = pContainer ? pContainer->sczId : pPackage->sczId;
    LPWSTR wzPayloadId = pPayload ? pPayload->sczKey : NULL;
    LPWSTR sczSourceFullPath = NULL;
    BURN_CACHE_ACQUIRE_PROGRESS_CONTEXT progress = { };
    BOOL fRetry = FALSE;
    BOOL fDownload = FALSE;

    progress.pContainer = pContainer;
    progress.pPackage = pPackage;
    progress.pPayload = pPayload;
    progress.pUX = pUX;
    progress.qwCacheProgress = *pqwCacheProgress;
    progress.qwTotalCacheSize = qwTotalCacheSize;

    do
    {
        LPWSTR wzSourcePath = pContainer ? pContainer->sczSourcePath : pPayload->sczSourcePath;
        if (PathIsAbsolute(wzSourcePath))
        {
            hr = StrAllocString(&sczSourceFullPath, wzSourcePath, 0);
            ExitOnFailure(hr, "Failed to copy full path.");
        }
        else
        {
            hr = PathRelativeToModule(&sczSourceFullPath, wzSourcePath, NULL);
            ExitOnFailure(hr, "Failed to create full path relative to module.");
        }

        fRetry = FALSE;
        progress.fCancel = FALSE;

        // If the file exists locally, copy it.
        if (FileExistsEx(sczSourceFullPath, NULL))
        {
            hr = CopyPayload(&progress, sczSourceFullPath, wzDestinationPath, &fRetry);
            ExitOnFailure(hr, "Failed to copy payload.");
        }
        else // can't find the file locally so prompt for source.
        {
            LPCWSTR wzDownloadUrl = pContainer ? pContainer->downloadSource.sczUrl : pPayload->downloadSource.sczUrl;

            hr = PromptForSource(pUX, wzPackageOrContainerId, wzPayloadId, sczSourceFullPath, wzDownloadUrl, &fRetry, &fDownload);
            ExitOnFailure(hr, "Failed while prompting for source.");

            if (fDownload)
            {
                hr = DownloadPayload(&progress, wzDestinationPath, &fRetry);
                ExitOnFailure(hr, "Failed to download payload.");
            }
        }
    } while (fRetry);
    ExitOnFailure(hr, "Failed to find external payload to cache.");

LExit:
    ReleaseStr(sczSourceFullPath);

    if (SUCCEEDED(hr))
    {
        (*pqwCacheProgress) += pContainer ? pContainer->qwFileSize : pPayload->qwFileSize;
    }

    return hr;
}

static HRESULT PromptForSource(
    __in BURN_USER_EXPERIENCE* pUX,
    __in_z LPCWSTR wzPackageOrContainerId,
    __in_z_opt LPCWSTR wzPayloadId,
    __in_z LPCWSTR wzLocalSource,
    __in_z_opt LPCWSTR wzDownloadSource,
    __out BOOL* pfRetry,
    __out BOOL* pfDownload
    )
{
    HRESULT hr = S_OK;

    UserExperienceDeactivateEngine(pUX);

    int nResult = pUX->pUserExperience->OnResolveSource(wzPackageOrContainerId, wzPayloadId, wzLocalSource, wzDownloadSource);
    switch (nResult)
    {
    case IDRETRY:
        *pfRetry = TRUE;
        break;

    case IDDOWNLOAD:
        if (wzDownloadSource && *wzDownloadSource)
        {
            *pfDownload = TRUE;
        }
        else
        {
            hr = E_INVALIDARG;
        }
        break;

    case IDNOACTION: __fallthrough;
    case IDOK: __fallthrough;
    case IDYES:
        hr = E_FILENOTFOUND;
        break;

    case IDABORT: __fallthrough;
    case IDCANCEL: __fallthrough;
    case IDNO:
        hr = HRESULT_FROM_WIN32(ERROR_INSTALL_USEREXIT);
        break;

    case IDERROR:
        hr = HRESULT_FROM_WIN32(ERROR_INSTALL_FAILURE);
        break;

    default:
        hr = E_INVALIDARG;
        break;
    }

    UserExperienceActivateEngine(pUX, NULL);
    return hr;
}

static HRESULT CopyPayload(
    __in BURN_CACHE_ACQUIRE_PROGRESS_CONTEXT* pProgress,
    __in_z LPCWSTR wzSourcePath,
    __in_z LPCWSTR wzDestinationPath,
    __out BOOL* pfRetry
    )
{
    HRESULT hr = S_OK;
    LPWSTR wzPackageOrContainerId = pProgress->pContainer ? pProgress->pContainer->sczId : pProgress->pPackage->sczId;
    LPWSTR wzPayloadId = pProgress->pPayload ? pProgress->pPayload->sczKey : NULL;

    *pfRetry = FALSE;

    int nResult = pProgress->pUX->pUserExperience->OnCacheAcquireBegin(wzPackageOrContainerId, wzPayloadId, BOOTSTRAPPER_CACHE_OPERATION_COPY, wzSourcePath);
    hr = HRESULT_FROM_VIEW(nResult);
    ExitOnRootFailure(hr, "UX aborted cache copy payload begin.");

    if (!::CopyFileExW(wzSourcePath, wzDestinationPath, CacheProgressRoutine, pProgress, &pProgress->fCancel, COPY_FILE_RESTARTABLE))
    {
        if (pProgress->fCancel)
        {
            hr = HRESULT_FROM_WIN32(ERROR_INSTALL_USEREXIT);
            ExitOnRootFailure2(hr, "UX aborted copy of %s: %ls", pProgress->pContainer ? "container" : "payload", pProgress->pContainer ? wzPackageOrContainerId : wzPayloadId);
        }
        else
        {
            ExitWithLastError(hr, "Failed to copy payload with progress.");
        }
    }

LExit:
    nResult = pProgress->pUX->pUserExperience->OnCacheAcquireComplete(wzPackageOrContainerId, wzPayloadId, hr);
    if (FAILED(hr) && IDRETRY == nResult)
    {
        *pfRetry = TRUE;
        hr = S_OK;
    }

    return hr;
}

static HRESULT DownloadPayload(
    __in BURN_CACHE_ACQUIRE_PROGRESS_CONTEXT* pProgress,
    __in_z LPCWSTR wzDestinationPath,
    __out BOOL* pfRetry
    )
{
    HRESULT hr = S_OK;
    LPWSTR wzPackageOrContainerId = pProgress->pContainer ? pProgress->pContainer->sczId : pProgress->pPackage->sczId;
    LPWSTR wzPayloadId = pProgress->pPayload ? pProgress->pPayload->sczKey : NULL;
    BURN_DOWNLOAD_SOURCE* pDownloadSource = pProgress->pContainer ? &pProgress->pContainer->downloadSource : &pProgress->pPayload->downloadSource;
    BURN_CACHE_CALLBACK callback = { };

    *pfRetry = FALSE;

    int nResult = pProgress->pUX->pUserExperience->OnCacheAcquireBegin(wzPackageOrContainerId, wzPayloadId, BOOTSTRAPPER_CACHE_OPERATION_DOWNLOAD, pDownloadSource->sczUrl);
    hr = HRESULT_FROM_VIEW(nResult);
    ExitOnRootFailure(hr, "UX aborted cache download payload begin.");

    callback.pfnProgress = CacheProgressRoutine;
    callback.pfnCancel = NULL; // TODO: set this
    callback.pv = pProgress;

    // If the protocol is specially marked, "bits" let's use that.
    if (L'b' == pDownloadSource->sczUrl[0] &&
        L'i' == pDownloadSource->sczUrl[1] &&
        L't' == pDownloadSource->sczUrl[2] &&
        L's' == pDownloadSource->sczUrl[3] &&
        (L':' == pDownloadSource->sczUrl[4] || (L's' == pDownloadSource->sczUrl[4] && L':' == pDownloadSource->sczUrl[5]))
        )
    {
        hr = BitsDownloadUrl(&callback, pDownloadSource, wzDestinationPath);
        ExitOnFailure(hr, "Failed to download URL using BITS.");
    }
    else // wininet handles everything else.
    {
        hr = WininetDownloadUrl(&callback, pDownloadSource, wzDestinationPath);
        ExitOnFailure(hr, "Failed to download URL using wininet.");
    }

LExit:
    nResult = pProgress->pUX->pUserExperience->OnCacheAcquireComplete(wzPackageOrContainerId, wzPayloadId, hr);
    if (FAILED(hr) && IDRETRY == nResult)
    {
        *pfRetry = TRUE;
        hr = S_OK;
    }

    return hr;
}

static DWORD CALLBACK CacheProgressRoutine(
    __in LARGE_INTEGER TotalFileSize,
    __in LARGE_INTEGER TotalBytesTransferred,
    __in LARGE_INTEGER /*StreamSize*/,
    __in LARGE_INTEGER /*StreamBytesTransferred*/,
    __in DWORD /*dwStreamNumber*/,
    __in DWORD /*dwCallbackReason*/,
    __in HANDLE /*hSourceFile*/,
    __in HANDLE /*hDestinationFile*/,
    __in_opt LPVOID lpData
    )
{
    DWORD dwResult = PROGRESS_CONTINUE;
    BURN_CACHE_ACQUIRE_PROGRESS_CONTEXT* pProgress = static_cast<BURN_CACHE_ACQUIRE_PROGRESS_CONTEXT*>(lpData);
    LPCWSTR wzPackageOrContainerId = pProgress->pContainer ? pProgress->pContainer->sczId : pProgress->pPackage->sczId;
    LPCWSTR wzPayloadId = pProgress->pPayload ? pProgress->pPayload->sczKey : NULL;
    DWORD64 qwCacheProgress = pProgress->qwCacheProgress + TotalBytesTransferred.QuadPart;
    DWORD dwOverallPercentage = static_cast<DWORD>(qwCacheProgress * 100 / pProgress->qwTotalCacheSize);

    int nResult = pProgress->pUX->pUserExperience->OnCacheAcquireProgress(wzPackageOrContainerId, wzPayloadId, TotalBytesTransferred.QuadPart, TotalFileSize.QuadPart, dwOverallPercentage);
    switch (nResult)
    {
    case IDOK: __fallthrough;
    case IDNOACTION: __fallthrough;
    case IDYES: __fallthrough;
    case IDRETRY: __fallthrough;
    case IDIGNORE: __fallthrough;
    case IDTRYAGAIN: __fallthrough;
    case IDCONTINUE:
        dwResult = PROGRESS_CONTINUE;
        break;

    case IDCANCEL: __fallthrough;
    case IDABORT: __fallthrough;
    case IDNO:
        dwResult = PROGRESS_CANCEL;
        pProgress->fCancel = TRUE;
        break;

    case IDERROR: __fallthrough;
    default:
        dwResult = PROGRESS_CANCEL;
        pProgress->fError = TRUE;
        break;
    }

    return dwResult;
}

static HRESULT DoExecuteActions(
    __in BURN_ENGINE_STATE* pEngineState,
    __in HANDLE hCacheThread,
    __in BURN_EXECUTE_CONTEXT* pContext,
    __out DWORD* pdwCheckpoint,
    __out BOOL* pfSuspend,
    __out BOOTSTRAPPER_APPLY_RESTART* pRestart
    )
{
    HRESULT hr = S_OK;
    HANDLE rghWait[2] = { };
    BOOTSTRAPPER_APPLY_RESTART restart = BOOTSTRAPPER_APPLY_RESTART_NONE;

    rghWait[1] = hCacheThread;

    for (DWORD i = 0; i < pEngineState->plan.cExecuteActions; ++i)
    {
        BURN_EXECUTE_ACTION* pExecuteAction = &pEngineState->plan.rgExecuteActions[i];

        switch (pExecuteAction->type)
        {
        case BURN_EXECUTE_ACTION_TYPE_CHECKPOINT:
            *pdwCheckpoint = pExecuteAction->checkpoint.dwId;
            break;

        case BURN_EXECUTE_ACTION_TYPE_WAIT:
            // wait for cache check-point
            rghWait[0] = pExecuteAction->wait.hEvent;
            switch (::WaitForMultipleObjects(rghWait[1] ? 2 : 1, rghWait, FALSE, INFINITE))
            {
            case WAIT_OBJECT_0:
                break;

            case WAIT_OBJECT_0 + 1:
                if (!::GetExitCodeThread(hCacheThread, (DWORD*)&hr))
                {
                    ExitWithLastError(hr, "Failed to get cache thread exit code.");
                }
                if (SUCCEEDED(hr))
                {
                    hr = E_UNEXPECTED;
                }
                ExitOnFailure(hr, "Cache thread terminated unexpectedly.");

            case WAIT_FAILED: __fallthrough;
            default:
                ExitWithLastError(hr, "Failed to wait for cache check-point.");
            }
            break;

        case BURN_EXECUTE_ACTION_TYPE_EXE_PACKAGE:
            hr = ExecuteExePackage(pEngineState, pExecuteAction, pContext, FALSE, pfSuspend, &restart);
            ExitOnFailure(hr, "Failed to execute EXE package.");
            break;

        case BURN_EXECUTE_ACTION_TYPE_MSI_PACKAGE:
            hr = ExecuteMsiPackage(pEngineState, pExecuteAction, pContext, FALSE, pfSuspend, &restart);
            ExitOnFailure(hr, "Failed to execute MSI package.");
            break;

        case BURN_EXECUTE_ACTION_TYPE_MSU_PACKAGE:
            hr = ExecuteMsuPackage(pEngineState, pExecuteAction, pContext, FALSE, pfSuspend, &restart);
            ExitOnFailure(hr, "Failed to execute MSU package.");
            break;

        case BURN_EXECUTE_ACTION_TYPE_SERVICE_STOP:
        case BURN_EXECUTE_ACTION_TYPE_SERVICE_START:
        default:
            hr = E_UNEXPECTED;
            ExitOnFailure(hr, "Invalid execute action.");
        }

        if (*pRestart < restart)
        {
            *pRestart = restart;
        }

        if (*pfSuspend || BOOTSTRAPPER_APPLY_RESTART_INITIATED == *pRestart)
        {
            ExitFunction();
        }
    }

LExit:
    return hr;
}

static HRESULT DoRollbackActions(
    __in BURN_ENGINE_STATE* pEngineState,
    __in BURN_EXECUTE_CONTEXT* pContext,
    __in DWORD dwCheckpoint,
    __out BOOTSTRAPPER_APPLY_RESTART* pRestart
    )
{
    HRESULT hr = S_OK;
    DWORD iCheckpoint = 0;
    BOOL fSuspendIgnored = FALSE;

    // scan to last checkpoint
    for (DWORD i = 0; i < pEngineState->plan.cRollbackActions; ++i)
    {
        BURN_EXECUTE_ACTION* pRollbackAction = &pEngineState->plan.rgRollbackActions[i];

        if (BURN_EXECUTE_ACTION_TYPE_CHECKPOINT == pRollbackAction->type)
        {
            if (pRollbackAction->checkpoint.dwId == dwCheckpoint)
            {
                iCheckpoint = i;
                break;
            }
        }
    }

    // execute rollback actions
    if (iCheckpoint)
    {
        // i has to be a signed integer so it doesn't get decremented to 0xFFFFFFFF.
        for (int i = iCheckpoint - 1; i >= 0; --i)
        {
            BOOTSTRAPPER_APPLY_RESTART restart = BOOTSTRAPPER_APPLY_RESTART_NONE;
            BURN_EXECUTE_ACTION* pRollbackAction = &pEngineState->plan.rgRollbackActions[i];

            switch (pRollbackAction->type)
            {
            case BURN_EXECUTE_ACTION_TYPE_CHECKPOINT:
                break;

            case BURN_EXECUTE_ACTION_TYPE_EXE_PACKAGE:
                hr = ExecuteExePackage(pEngineState, pRollbackAction, pContext, TRUE, &fSuspendIgnored, &restart);
                TraceError(hr, "Failed to rollback EXE package.");
                hr = S_OK;
                break;

            case BURN_EXECUTE_ACTION_TYPE_MSI_PACKAGE:
                hr = ExecuteMsiPackage(pEngineState, pRollbackAction, pContext, TRUE, &fSuspendIgnored, &restart);
                TraceError(hr, "Failed to rollback MSI package.");
                hr = S_OK;
                break;

            case BURN_EXECUTE_ACTION_TYPE_MSU_PACKAGE:
                hr = ExecuteMsuPackage(pEngineState, pRollbackAction, pContext, TRUE, &fSuspendIgnored, &restart);
                ExitOnFailure(hr, "Failed to rollback MSU package.");
                break;

            case BURN_EXECUTE_ACTION_TYPE_SERVICE_STOP:
            case BURN_EXECUTE_ACTION_TYPE_SERVICE_START:
            default:
                hr = E_UNEXPECTED;
                ExitOnFailure1(hr, "Invalid rollback action: %d.", pRollbackAction->type);
            }

            if (*pRestart < restart)
            {
                *pRestart = restart;
            }
        }
    }

LExit:
    return hr;
}

static HRESULT ExecuteExePackage(
    __in BURN_ENGINE_STATE* pEngineState,
    __in BURN_EXECUTE_ACTION* pExecuteAction,
    __in BURN_EXECUTE_CONTEXT* pContext,
    __in BOOL fRollback,
    __out BOOL* pfSuspend,
    __out BOOTSTRAPPER_APPLY_RESTART* pRestart
    )
{
    HRESULT hr = S_OK;
    HRESULT hrExecute = S_OK;
    int nResult = 0;

    pContext->pExecutingPackage = pExecuteAction->exePackage.pPackage;

    // send package execute begin to UX
    nResult = pEngineState->userExperience.pUserExperience->OnExecutePackageBegin(pExecuteAction->exePackage.pPackage->sczId, !fRollback);
    hr = HRESULT_FROM_VIEW(nResult);
    ExitOnRootFailure(hr, "UX aborted execute EXE package begin.");

    nResult = GenericExecuteProgress(pContext, fRollback ? 1 : 0, 1);
    hr = HRESULT_FROM_VIEW(nResult);
    ExitOnRootFailure(hr, "UX aborted EXE progress.");

    // execute package
    if (pExecuteAction->exePackage.pPackage->fPerMachine)
    {
        hrExecute = ElevationExecuteExePackage(pEngineState->hElevatedPipe, pExecuteAction, &pEngineState->variables, pRestart);
        ExitOnFailure(hrExecute, "Failed to configure per-machine EXE package.");
    }
    else
    {
        hrExecute = ExeEngineExecutePackage(pExecuteAction, &pEngineState->variables, pRestart);
        ExitOnFailure(hrExecute, "Failed to configure per-user EXE package.");
    }

    nResult = GenericExecuteProgress(pContext, fRollback ? 0 : 1, 1);
    hr = HRESULT_FROM_VIEW(nResult);
    ExitOnRootFailure(hr, "UX aborted EXE progress.");

    pContext->cExecutedPackages += fRollback ? -1 : 1;
    (*pContext->pcOverallProgressTicks) += fRollback ? -1 : 1;

    hr = ReportOverallProgressTicks(&pEngineState->userExperience, pEngineState->plan.cOverallProgressTicksTotal, *pContext->pcOverallProgressTicks);
    ExitOnRootFailure(hr, "UX aborted EXE package execute progress.");

LExit:
    hr = ExecutePackageComplete(&pEngineState->userExperience, pExecuteAction->exePackage.pPackage, hr, hrExecute, pRestart, pfSuspend);
    return hr;
}

static HRESULT ExecuteMsiPackage(
    __in BURN_ENGINE_STATE* pEngineState,
    __in BURN_EXECUTE_ACTION* pExecuteAction,
    __in BURN_EXECUTE_CONTEXT* pContext,
    __in BOOL fRollback,
    __out BOOL* pfSuspend,
    __out BOOTSTRAPPER_APPLY_RESTART* pRestart
    )
{
    HRESULT hr = S_OK;
    HRESULT hrExecute = S_OK;
    int nResult = 0;

    pContext->pExecutingPackage = pExecuteAction->msiPackage.pPackage;

    // send package execute begin to UX
    nResult = pEngineState->userExperience.pUserExperience->OnExecutePackageBegin(pExecuteAction->msiPackage.pPackage->sczId, !fRollback);
    hr = HRESULT_FROM_VIEW(nResult);
    ExitOnRootFailure(hr, "UX aborted execute MSI package begin.");

    // execute package
    if (pExecuteAction->msiPackage.pPackage->fPerMachine)
    {
        hrExecute = ElevationExecuteMsiPackage(pEngineState->hElevatedPipe, pExecuteAction, &pEngineState->variables, fRollback, MsiExecuteMessageHandler, pContext, pRestart);
        ExitOnFailure(hrExecute, "Failed to configure per-machine MSI package.");
    }
    else
    {
        hrExecute = MsiEngineExecutePackage(pExecuteAction, &pEngineState->variables, fRollback, MsiExecuteMessageHandler, pContext, pRestart);
        ExitOnFailure(hrExecute, "Failed to configure per-user MSI package.");
    }

    pContext->cExecutedPackages += fRollback ? -1 : 1;
    (*pContext->pcOverallProgressTicks) += fRollback ? -1 : 1;

    hr = ReportOverallProgressTicks(&pEngineState->userExperience, pEngineState->plan.cOverallProgressTicksTotal, *pContext->pcOverallProgressTicks);
    ExitOnRootFailure(hr, "UX aborted MSI package execute progress.");

LExit:
    hr = ExecutePackageComplete(&pEngineState->userExperience, pExecuteAction->msiPackage.pPackage, hr, hrExecute, pRestart, pfSuspend);
    return hr;
}

static HRESULT ExecuteMsuPackage(
    __in BURN_ENGINE_STATE* pEngineState,
    __in BURN_EXECUTE_ACTION* pExecuteAction,
    __in BURN_EXECUTE_CONTEXT* pContext,
    __in BOOL fRollback,
    __out BOOL* pfSuspend,
    __out BOOTSTRAPPER_APPLY_RESTART* pRestart
    )
{
    HRESULT hr = S_OK;
    HRESULT hrExecute = S_OK;
    int nResult = 0;

    pContext->pExecutingPackage = pExecuteAction->msuPackage.pPackage;

    // send package execute begin to UX
    nResult = pEngineState->userExperience.pUserExperience->OnExecutePackageBegin(pExecuteAction->msuPackage.pPackage->sczId, !fRollback);
    hr = HRESULT_FROM_VIEW(nResult);
    ExitOnRootFailure(hr, "UX aborted execute MSU package begin.");

    nResult = GenericExecuteProgress(pContext, fRollback ? 1 : 0, 1);
    hr = HRESULT_FROM_VIEW(nResult);
    ExitOnRootFailure(hr, "UX aborted MSU progress.");

    // execute package
    if (pExecuteAction->msuPackage.pPackage->fPerMachine)
    {
        hrExecute = ElevationExecuteMsuPackage(pEngineState->hElevatedPipe, pExecuteAction, pRestart);
        ExitOnFailure(hrExecute, "Failed to configure per-machine MSU package.");
    }
    else
    {
        hrExecute = E_UNEXPECTED;
        ExitOnFailure(hr, "MSU packages cannot be per-user.");
    }

    nResult = GenericExecuteProgress(pContext, fRollback ? 0 : 1, 1);
    hr = HRESULT_FROM_VIEW(nResult);
    ExitOnRootFailure(hr, "UX aborted MSU progress.");

    pContext->cExecutedPackages += fRollback ? -1 : 1;
    (*pContext->pcOverallProgressTicks) += fRollback ? -1 : 1;

    hr = ReportOverallProgressTicks(&pEngineState->userExperience, pEngineState->plan.cOverallProgressTicksTotal, *pContext->pcOverallProgressTicks);
    ExitOnRootFailure(hr, "UX aborted MSU package execute progress.");

LExit:
    hr = ExecutePackageComplete(&pEngineState->userExperience, pExecuteAction->msuPackage.pPackage, hr, hrExecute, pRestart, pfSuspend);
    return hr;
}

static int GenericExecuteProgress(
    __in LPVOID pvContext,
    __in DWORD dwProgress,
    __in DWORD dwTotal
    )
{
    BURN_EXECUTE_CONTEXT* pContext = (BURN_EXECUTE_CONTEXT*)pvContext;
    DWORD dwProgressPercentage = (dwProgress * 100) / dwTotal;
    DWORD dwOverallProgress = ((pContext->cExecutedPackages * 100 + dwProgressPercentage) * 100) / (pContext->cExecutePackagesTotal * 100);
    return pContext->pUX->pUserExperience->OnExecuteProgress(pContext->pExecutingPackage->sczId, dwProgressPercentage, dwOverallProgress);
}

static int MsiExecuteMessageHandler(
    __in LPVOID pvContext,
    __in BURN_MSI_EXECUTE_MESSAGE* pMessage
    )
{
    BURN_EXECUTE_CONTEXT* pContext = (BURN_EXECUTE_CONTEXT*)pvContext;

    switch (pMessage->type)
    {
    case BURN_MSI_EXECUTE_MESSAGE_PROGRESS:
        {
        DWORD dwOverallProgress = ((pContext->cExecutedPackages * 100 + pMessage->progress.dwPercentage) * 100) / (pContext->cExecutePackagesTotal * 100);
        return pContext->pUX->pUserExperience->OnExecuteProgress(pContext->pExecutingPackage->sczId, pMessage->progress.dwPercentage, dwOverallProgress);
        }

    case BURN_MSI_EXECUTE_MESSAGE_ERROR:
        return pContext->pUX->pUserExperience->OnError(pContext->pExecutingPackage->sczId, pMessage->error.dwErrorCode, pMessage->error.wzMessage, pMessage->error.uiFlags);

    case BURN_MSI_EXECUTE_MESSAGE_MSI_MESSAGE:
        return pContext->pUX->pUserExperience->OnExecuteMsiMessage(pContext->pExecutingPackage->sczId, pMessage->msiMessage.mt, pMessage->msiMessage.uiFlags, pMessage->msiMessage.wzMessage);

    case BURN_MSI_EXECUTE_MESSAGE_MSI_FILES_IN_USE:
        return pContext->pUX->pUserExperience->OnExecuteMsiFilesInUse(pContext->pExecutingPackage->sczId, pMessage->msiFilesInUse.cFiles, pMessage->msiFilesInUse.rgwzFiles);

    default:
        return IDOK;
    }
}

static HRESULT ReportOverallProgressTicks(
    __in BURN_USER_EXPERIENCE* pUX,
    __in DWORD cOverallProgressTicksTotal,
    __in DWORD cOverallProgressTicks
    )
{
    HRESULT hr = S_OK;

    int nResult = pUX->pUserExperience->OnProgress(0, cOverallProgressTicks * 100 / cOverallProgressTicksTotal);
    hr = HRESULT_FROM_VIEW(nResult);

    return hr;
}

static HRESULT ExecutePackageComplete(
    __in BURN_USER_EXPERIENCE* pUX,
    __in BURN_PACKAGE* pPackage,
    __in HRESULT hrOverall,
    __in HRESULT hrExecute,
    __inout BOOTSTRAPPER_APPLY_RESTART* pRestart,
    __out BOOL* pfSuspend
    )
{
    HRESULT hr = FAILED(hrOverall) ? hrOverall : hrExecute; // if the overall function failed use that otherwise use the execution result.

    // send package execute complete to UX
    int nResult = pUX->pUserExperience->OnExecutePackageComplete(pPackage->sczId, hr, *pRestart);
    if (IDRESTART == nResult)
    {
        *pRestart = BOOTSTRAPPER_APPLY_RESTART_INITIATED;
    }
    else if (IDSUSPEND == nResult)
    {
        *pfSuspend = TRUE;
    }

    // If we *only* failed to execute and this package is not vital, say everything is okay.
    if (SUCCEEDED(hrOverall) && FAILED(hrExecute) && !pPackage->fVital)
    {
        LogId(REPORT_STANDARD, MSG_APPLY_CONTINUING_NONVITAL_PACKAGE, pPackage->sczId, hrExecute);
        hr = S_OK;
    }

    return hr;
}
