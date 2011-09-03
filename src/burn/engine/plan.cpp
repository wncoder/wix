//-------------------------------------------------------------------------------------------------
// <copyright file="plan.cpp" company="Microsoft">
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


// internal function definitions

static void UninitializeCacheAction(
    __in BURN_CACHE_ACTION* pCacheAction
    );
static HRESULT GetActionDefaultRequestState(
    __in BOOTSTRAPPER_ACTION action,
    __out BOOTSTRAPPER_REQUEST_STATE* pRequestState
    );
static BOOL AlreadyPlannedCachePackage(
    __in BURN_PLAN* pPlan,
    __in_z LPCWSTR wzPackageId,
    __out HANDLE* phSyncpointEvent
    );
static DWORD GetNextCheckpointId();
static HRESULT AppendCacheAction(
    __in BURN_PLAN* pPlan,
    __out BURN_CACHE_ACTION** ppCacheAction
    );
static HRESULT AppendRollbackCacheAction(
    __in BURN_PLAN* pPlan,
    __out BURN_CACHE_ACTION** ppCacheAction
    );
static HRESULT AppendCacheOrLayoutPayloadAction(
    __in BURN_PLAN* pPlan,
    __in_opt BURN_PACKAGE* pPackage,
    __in BURN_PAYLOAD* pPayload,
    __in_z_opt LPCWSTR wzLayoutDirectory
    );
static HRESULT CreateOrFindContainerExtractAction(
    __in BURN_PLAN* pPlan,
    __in BURN_CONTAINER* pContainer,
    __out BURN_CACHE_ACTION** ppContainerExtractAction
    );


// function definitions

extern "C" void PlanUninitialize(
    __in BURN_PLAN* pPlan
    )
{
    if (pPlan->rgCacheActions)
    {
        for (DWORD i = 0; i < pPlan->cCacheActions; ++i)
        {
            UninitializeCacheAction(&pPlan->rgCacheActions[i]);
        }
        MemFree(pPlan->rgCacheActions);
    }

    if (pPlan->rgExecuteActions)
    {
        for (DWORD i = 0; i < pPlan->cExecuteActions; ++i)
        {
            PlanUninitializeExecuteAction(&pPlan->rgExecuteActions[i]);
        }
        MemFree(pPlan->rgExecuteActions);
    }

    if (pPlan->rgRollbackActions)
    {
        for (DWORD i = 0; i < pPlan->cRollbackActions; ++i)
        {
            PlanUninitializeExecuteAction(&pPlan->rgRollbackActions[i]);
        }
        MemFree(pPlan->rgRollbackActions);
    }

    if (pPlan->rgCleanActions)
    {
        // Nothing needs to be freed inside clean actions today.
        MemFree(pPlan->rgCleanActions);
    }

    memset(pPlan, 0, sizeof(BURN_PLAN));
}

extern "C" void PlanUninitializeExecuteAction(
    __in BURN_EXECUTE_ACTION* pExecuteAction
    )
{
    switch (pExecuteAction->type)
    {
    case BURN_EXECUTE_ACTION_TYPE_MSI_PACKAGE:
        ReleaseStr(pExecuteAction->msiPackage.sczProductCode);
        ReleaseStr(pExecuteAction->msiPackage.sczLogPath);
        ReleaseMem(pExecuteAction->msiPackage.rgFeatures);
        ReleaseMem(pExecuteAction->msiPackage.rgOrderedPatches);
        break;

    case BURN_EXECUTE_ACTION_TYPE_MSP_TARGET:
        ReleaseStr(pExecuteAction->mspTarget.sczTargetProductCode);
        ReleaseStr(pExecuteAction->mspTarget.sczLogPath);
        ReleaseMem(pExecuteAction->mspTarget.rgOrderedPatches);
        break;

    case BURN_EXECUTE_ACTION_TYPE_MSU_PACKAGE:
        ReleaseStr(pExecuteAction->msuPackage.sczLogPath);
        break;

    case BURN_EXECUTE_ACTION_TYPE_SERVICE_STOP: __fallthrough;
    case BURN_EXECUTE_ACTION_TYPE_SERVICE_START:
        ReleaseStr(pExecuteAction->service.sczServiceName);
        break;

    case BURN_EXECUTE_ACTION_TYPE_DEPENDENCY:
        ReleaseStr(pExecuteAction->dependency.sczBundleProviderKey);
        break;
    }
}

extern "C" HRESULT PlanDefaultPackageRequestState(
    __in BURN_PACKAGE_TYPE packageType,
    __in BOOTSTRAPPER_PACKAGE_STATE currentState,
    __in BOOTSTRAPPER_ACTION action,
    __in BURN_VARIABLES* pVariables,
    __in_z_opt LPCWSTR wzInstallCondition,
    __in BOOTSTRAPPER_RELATION_TYPE relationType,
    __out BOOTSTRAPPER_REQUEST_STATE* pRequestState
    )
{
    HRESULT hr = S_OK;
    BOOTSTRAPPER_REQUEST_STATE defaultRequestState = BOOTSTRAPPER_REQUEST_STATE_NONE;
    BOOL fCondition = FALSE;

    // If doing layout, then always default to requesting the file be cached.
    if (BOOTSTRAPPER_ACTION_LAYOUT == action)
    {
        *pRequestState = BOOTSTRAPPER_REQUEST_STATE_CACHE;
    }
    // the package is superseded then default to doing nothing except during uninstall.
    else if (BOOTSTRAPPER_PACKAGE_STATE_SUPERSEDED == currentState && BOOTSTRAPPER_ACTION_UNINSTALL != action)
    {
        *pRequestState = BOOTSTRAPPER_REQUEST_STATE_NONE;
    }
    else if (BOOTSTRAPPER_RELATION_PATCH == relationType && BURN_PACKAGE_TYPE_MSP == packageType)
    {
        // If we're run from a related bundle as a patch, don't do anything to our MSP packages inside
        *pRequestState = BOOTSTRAPPER_REQUEST_STATE_NONE;
    }
    else // pick the best option for the action state and install condition.
    {
        hr = GetActionDefaultRequestState(action, &defaultRequestState);
        ExitOnFailure(hr, "Failed to get default request state for action.");

        // If there is an install condition (and we're doing an install) evaluate the condition
        // to determine whether to use the default request state or make the package absent.
        if (BOOTSTRAPPER_ACTION_UNINSTALL != action && wzInstallCondition && *wzInstallCondition)
        {
            hr = ConditionEvaluate(pVariables, wzInstallCondition, &fCondition);
            ExitOnFailure(hr, "Failed to evaluate install condition.");

            *pRequestState = fCondition ? defaultRequestState : BOOTSTRAPPER_REQUEST_STATE_ABSENT;
        }
        else // just set the package to the default request state.
        {
            *pRequestState = defaultRequestState;
        }
    }

LExit:
    return hr;
}

extern "C" HRESULT PlanLayoutBundle(
    __in BURN_PLAN* pPlan,
    __in_z LPCWSTR wzLayoutDirectory
    )
{
    HRESULT hr = S_OK;
    BURN_CACHE_ACTION* pCacheAction = NULL;

    hr = AppendCacheAction(pPlan, &pCacheAction);
    ExitOnFailure(hr, "Failed to append bundle start action.");

    pCacheAction->type = BURN_CACHE_ACTION_TYPE_LAYOUT_BUNDLE;

    hr = StrAllocString(&pCacheAction->bundleLayout.sczLayoutDirectory, wzLayoutDirectory, 0);
    ExitOnFailure(hr, "Failed to to copy layout directory for bundle.");

    ++pPlan->cOverallProgressTicksTotal;

LExit:
    return hr;
}

extern "C" HRESULT PlanLayoutOnlyPayload(
    __in BURN_PLAN* pPlan,
    __in BURN_PAYLOAD* pPayload,
    __in_z LPCWSTR wzLayoutDirectory
    )
{
    HRESULT hr = S_OK;

    hr = AppendCacheOrLayoutPayloadAction(pPlan, NULL, pPayload, wzLayoutDirectory);
    ExitOnFailure(hr, "Failed to append payload layout only action.");

LExit:
    return hr;
}

extern "C" HRESULT PlanLayoutPackage(
    __in BURN_PLAN* pPlan,
    __in BURN_PACKAGE* pPackage,
    __in_z LPCWSTR wzLayoutDirectory
    )
{
    HRESULT hr = S_OK;
    BURN_CACHE_ACTION* pCacheAction = NULL;
    DWORD iPackageStartAction = 0;

    hr = AppendCacheAction(pPlan, &pCacheAction);
    ExitOnFailure(hr, "Failed to append package start action.");

    pCacheAction->type = BURN_CACHE_ACTION_TYPE_PACKAGE_START;
    pCacheAction->packageStart.pPackage = pPackage;

    // Remember the index for the package start action (which is now the last in the cache
    // actions array) because the array may be resized later and move around in memory.
    iPackageStartAction = pPlan->cCacheActions - 1;

    // If any of the package payloads are not cached, add them to the plan.
    for (DWORD i = 0; i < pPackage->cPayloads; ++i)
    {
        BURN_PACKAGE_PAYLOAD* pPackagePayload = &pPackage->rgPayloads[i];

        hr = AppendCacheOrLayoutPayloadAction(pPlan, pPackage, pPackagePayload->pPayload, wzLayoutDirectory);
        ExitOnFailure(hr, "Failed to append cache action.");

        Assert(BURN_CACHE_ACTION_TYPE_PACKAGE_START == pPlan->rgCacheActions[iPackageStartAction].type);
        ++pPlan->rgCacheActions[iPackageStartAction].packageStart.cCachePayloads;
        pPlan->rgCacheActions[iPackageStartAction].packageStart.qwCachePayloadSizeTotal += pPackagePayload->pPayload->qwFileSize;
    }

    // Create package stop action.
    hr = AppendCacheAction(pPlan, &pCacheAction);
    ExitOnFailure(hr, "Failed to append cache action.");

    pCacheAction->type = BURN_CACHE_ACTION_TYPE_PACKAGE_STOP;
    pCacheAction->packageStop.pPackage = pPackage;

    ++pPlan->cOverallProgressTicksTotal;

LExit:
    return hr;
}

extern "C" HRESULT PlanCachePackage(
    __in BURN_PLAN* pPlan,
    __in BURN_PACKAGE* pPackage,
    __out HANDLE* phSyncpointEvent
    )
{
    AssertSz(pPackage->sczCacheId && *pPackage->sczCacheId, "PlanCachePackage() expects the package to have a cache id.");

    HRESULT hr = S_OK;
    BURN_CACHE_ACTION* pCacheAction = NULL;
    DWORD dwCheckpoint = 0;
    DWORD iPackageStartAction = 0;

    BOOL fPlanned = AlreadyPlannedCachePackage(pPlan, pPackage->sczId, phSyncpointEvent);
    if (fPlanned)
    {
        ExitFunction();
    }

    // Cache checkpoints happen before the package is cached because downloading packages'
    // payloads will not roll themselves back the way installation packages rollback on
    // failure automatically.
    dwCheckpoint = GetNextCheckpointId();

    hr = AppendCacheAction(pPlan, &pCacheAction);
    ExitOnFailure(hr, "Failed to append package start action.");

    pCacheAction->type = BURN_CACHE_ACTION_TYPE_CHECKPOINT;
    pCacheAction->checkpoint.dwId = dwCheckpoint;

    hr = AppendRollbackCacheAction(pPlan, &pCacheAction);
    ExitOnFailure(hr, "Failed to append rollback cache action.");

    pCacheAction->type = BURN_CACHE_ACTION_TYPE_CHECKPOINT;
    pCacheAction->checkpoint.dwId = dwCheckpoint;

    // Plan the package start.
    hr = AppendCacheAction(pPlan, &pCacheAction);
    ExitOnFailure(hr, "Failed to append package start action.");

    pCacheAction->type = BURN_CACHE_ACTION_TYPE_PACKAGE_START;
    pCacheAction->packageStart.pPackage = pPackage;

    // Remember the index for the package start action (which is now the last in the cache
    // actions array) because we have to update this action after processing all the payloads
    // and the array may be resized later which would move a pointer around in memory.
    iPackageStartAction = pPlan->cCacheActions - 1;

    // Create a package cache rollback action.
    hr = AppendRollbackCacheAction(pPlan, &pCacheAction);
    ExitOnFailure(hr, "Failed to append rollback cache action.");

    pCacheAction->type = BURN_CACHE_ACTION_TYPE_ROLLBACK_PACKAGE;
    pCacheAction->rollbackPackage.pPackage = pPackage;

    // If any of the package payloads are not cached, add them to the plan.
    for (DWORD i = 0; i < pPackage->cPayloads; ++i)
    {
        BURN_PACKAGE_PAYLOAD* pPackagePayload = &pPackage->rgPayloads[i];

        if (!pPackagePayload->fCached)
        {
            hr = AppendCacheOrLayoutPayloadAction(pPlan, pPackage, pPackagePayload->pPayload, NULL);
            ExitOnFailure(hr, "Failed to append payload cache action.");

            Assert(BURN_CACHE_ACTION_TYPE_PACKAGE_START == pPlan->rgCacheActions[iPackageStartAction].type);
            ++pPlan->rgCacheActions[iPackageStartAction].packageStart.cCachePayloads;
            pPlan->rgCacheActions[iPackageStartAction].packageStart.qwCachePayloadSizeTotal += pPackagePayload->pPayload->qwFileSize;

            // If we're caching something from a per-machine package, ensure
            // the plan is per-machine as well.
            if (pPackage->fPerMachine)
            {
                pPlan->fPerMachine = TRUE;
            }
        }
    }

    // Create package stop action.
    hr = AppendCacheAction(pPlan, &pCacheAction);
    ExitOnFailure(hr, "Failed to append cache action.");

    pCacheAction->type = BURN_CACHE_ACTION_TYPE_PACKAGE_STOP;
    pCacheAction->packageStop.pPackage = pPackage;

    // Create syncpoint action.
    hr = AppendCacheAction(pPlan, &pCacheAction);
    ExitOnFailure(hr, "Failed to append cache action.");

    pCacheAction->type = BURN_CACHE_ACTION_TYPE_SYNCPOINT;
    pCacheAction->syncpoint.hEvent = ::CreateEventW(NULL, TRUE, FALSE, NULL);
    ExitOnNullWithLastError(pCacheAction->syncpoint.hEvent, hr, "Failed to create syncpoint event.");

    *phSyncpointEvent = pCacheAction->syncpoint.hEvent;

    ++pPlan->cOverallProgressTicksTotal;

LExit:
    return hr;
}

extern "C" HRESULT PlanCacheSlipstreamMsps(
    __in BURN_PLAN* pPlan,
    __in BURN_PACKAGE* pPackage
    )
{
    HRESULT hr = S_OK;
    HANDLE hIgnored = NULL;

    AssertSz(BURN_PACKAGE_TYPE_MSI == pPackage->type, "Only MSI packages can have slipstream patches.");

    for (DWORD i = 0; i < pPackage->Msi.cSlipstreamMspPackages; ++i)
    {
        BURN_PACKAGE* pMspPackage = pPackage->Msi.rgpSlipstreamMspPackages[i];
        AssertSz(BURN_PACKAGE_TYPE_MSP == pMspPackage->type, "Only MSP packages can be slipstream patches.");

        hr = PlanCachePackage(pPlan, pMspPackage, &hIgnored);
        ExitOnFailure1(hr, "Failed to plan slipstream MSP: %ls", pMspPackage->sczId);
    }

LExit:
    return hr;
}

extern "C" HRESULT PlanCleanPackage(
    __in BURN_PLAN* pPlan,
    __in BURN_PACKAGE* pPackage
    )
{
    HRESULT hr = S_OK;
    BURN_CLEAN_ACTION* pCleanAction = NULL;

    hr = MemEnsureArraySize(reinterpret_cast<LPVOID*>(&pPlan->rgCleanActions), pPlan->cCleanActions + 1, sizeof(BURN_CLEAN_ACTION), 5);
    ExitOnFailure(hr, "Failed to grow plan's array of clean actions.");

    pCleanAction = pPlan->rgCleanActions + pPlan->cCleanActions;
    ++pPlan->cCleanActions;

    pCleanAction->pPackage = pPackage;

LExit:
    return hr;
}

extern "C" HRESULT PlanExecuteCacheSyncAndRollback(
    __in BURN_PLAN* pPlan,
    __in BURN_PACKAGE* pPackage,
    __in HANDLE hCacheEvent,
    __in BOOL fPlanPackageCacheRollback
    )
{
    HRESULT hr = S_OK;
    BURN_EXECUTE_ACTION* pAction = NULL;

    hr = PlanAppendExecuteAction(pPlan, &pAction);
    ExitOnFailure(hr, "Failed to append wait action for caching.");

    pAction->type = BURN_EXECUTE_ACTION_TYPE_SYNCPOINT;
    pAction->syncpoint.hEvent = hCacheEvent;

    if (fPlanPackageCacheRollback)
    {
        hr = PlanAppendRollbackAction(pPlan, &pAction);
        ExitOnFailure(hr, "Failed to append rollback action.");

        pAction->type = BURN_EXECUTE_ACTION_TYPE_UNCACHE_PACKAGE;
        pAction->uncachePackage.pPackage = pPackage;

        hr = PlanExecuteCheckpoint(pPlan);
        ExitOnFailure(hr, "Failed to append execute checkpoint for cache rollback.");
    }

LExit:
    return hr;
}

extern "C" HRESULT PlanExecuteCheckpoint(
    __in BURN_PLAN* pPlan
    )
{
    HRESULT hr = S_OK;
    BURN_EXECUTE_ACTION* pAction = NULL;
    DWORD dwCheckpointId = GetNextCheckpointId();

    // execute checkpoint
    hr = PlanAppendExecuteAction(pPlan, &pAction);
    ExitOnFailure(hr, "Failed to append execute action.");

    pAction->type = BURN_EXECUTE_ACTION_TYPE_CHECKPOINT;
    pAction->checkpoint.dwId = dwCheckpointId;

    // rollback checkpoint
    hr = PlanAppendRollbackAction(pPlan, &pAction);
    ExitOnFailure(hr, "Failed to append rollback action.");

    pAction->type = BURN_EXECUTE_ACTION_TYPE_CHECKPOINT;
    pAction->checkpoint.dwId = dwCheckpointId;

LExit:
    return hr;
}

extern "C" HRESULT PlanInsertExecuteAction(
    __in DWORD dwIndex,
    __in BURN_PLAN* pPlan,
    __out BURN_EXECUTE_ACTION** ppExecuteAction
    )
{
    HRESULT hr = S_OK;

    hr = MemInsertIntoArray((void**)&pPlan->rgExecuteActions, dwIndex, 1, pPlan->cExecuteActions + 1, sizeof(BURN_EXECUTE_ACTION), 5);
    ExitOnFailure(hr, "Failed to grow plan's array of execute actions.");

    *ppExecuteAction = pPlan->rgExecuteActions + dwIndex;
    ++pPlan->cExecuteActions;

LExit:
    return hr;
}

extern "C" HRESULT PlanAppendExecuteAction(
    __in BURN_PLAN* pPlan,
    __out BURN_EXECUTE_ACTION** ppExecuteAction
    )
{
    HRESULT hr = S_OK;

    hr = MemEnsureArraySize((void**)&pPlan->rgExecuteActions, pPlan->cExecuteActions + 1, sizeof(BURN_EXECUTE_ACTION), 5);
    ExitOnFailure(hr, "Failed to grow plan's array of execute actions.");

    *ppExecuteAction = pPlan->rgExecuteActions + pPlan->cExecuteActions;
    ++pPlan->cExecuteActions;

LExit:
    return hr;
}

extern "C" HRESULT PlanAppendRollbackAction(
    __in BURN_PLAN* pPlan,
    __out BURN_EXECUTE_ACTION** ppRollbackAction
    )
{
    HRESULT hr = S_OK;

    hr = MemEnsureArraySize((void**)&pPlan->rgRollbackActions, pPlan->cRollbackActions + 1, sizeof(BURN_EXECUTE_ACTION), 5);
    ExitOnFailure(hr, "Failed to grow plan's array of rollback actions.");

    *ppRollbackAction = pPlan->rgRollbackActions + pPlan->cRollbackActions;
    ++pPlan->cRollbackActions;

LExit:
    return hr;
}

extern "C" HRESULT PlanRollbackBoundaryBegin(
    __in BURN_PLAN* pPlan,
    __in BURN_ROLLBACK_BOUNDARY* pRollbackBoundary,
    __out HANDLE* phEvent
    )
{
    HRESULT hr = S_OK;
    BURN_EXECUTE_ACTION* pExecuteAction = NULL;

    // Create sync event.
    *phEvent = ::CreateEventW(NULL, TRUE, FALSE, NULL);
    ExitOnNullWithLastError(*phEvent, hr, "Failed to create event.");

    // Add begin rollback boundary to execute plan.
    hr = PlanAppendExecuteAction(pPlan, &pExecuteAction);
    ExitOnFailure(hr, "Failed to append rollback boundary begin action.");

    pExecuteAction->type = BURN_EXECUTE_ACTION_TYPE_ROLLBACK_BOUNDARY;
    pExecuteAction->rollbackBoundary.pRollbackBoundary = pRollbackBoundary;

    // Add begin rollback boundary to rollback plan.
    hr = PlanAppendRollbackAction(pPlan, &pExecuteAction);
    ExitOnFailure(hr, "Failed to append rollback boundary begin action.");

    pExecuteAction->type = BURN_EXECUTE_ACTION_TYPE_ROLLBACK_BOUNDARY;
    pExecuteAction->rollbackBoundary.pRollbackBoundary = pRollbackBoundary;

    // Add execute sync-point.
    hr = PlanAppendExecuteAction(pPlan, &pExecuteAction);
    ExitOnFailure(hr, "Failed to append wait for rollback boundary action.");

    pExecuteAction->type = BURN_EXECUTE_ACTION_TYPE_SYNCPOINT;
    pExecuteAction->syncpoint.hEvent = *phEvent;

LExit:
    return hr;
}

extern "C" HRESULT PlanRollbackBoundaryComplete(
    __in BURN_PLAN* pPlan,
    __in HANDLE hEvent
    )
{
    HRESULT hr = S_OK;
    BURN_CACHE_ACTION* pCacheAction = NULL;
    BURN_EXECUTE_ACTION* pExecuteAction = NULL;
    DWORD dwCheckpointId = 0;

    // Add cache sync-point.
    hr = AppendCacheAction(pPlan, &pCacheAction);
    ExitOnFailure(hr, "Failed to add syncpoint to cache plan.");

    pCacheAction->type = BURN_CACHE_ACTION_TYPE_SYNCPOINT;
    pCacheAction->syncpoint.hEvent = hEvent;

    // Add checkpoints.
    dwCheckpointId = GetNextCheckpointId();

    hr = PlanAppendExecuteAction(pPlan, &pExecuteAction);
    ExitOnFailure(hr, "Failed to append execute action.");

    pExecuteAction->type = BURN_EXECUTE_ACTION_TYPE_CHECKPOINT;
    pExecuteAction->checkpoint.dwId = dwCheckpointId;

    hr = PlanAppendRollbackAction(pPlan, &pExecuteAction);
    ExitOnFailure(hr, "Failed to append rollback action.");

    pExecuteAction->type = BURN_EXECUTE_ACTION_TYPE_CHECKPOINT;
    pExecuteAction->checkpoint.dwId = dwCheckpointId;

LExit:
    return hr;
}


// internal function definitions
static void UninitializeCacheAction(
    __in BURN_CACHE_ACTION* pCacheAction
    )
{
    switch (pCacheAction->type)
    {
    case BURN_CACHE_ACTION_TYPE_SYNCPOINT:
        ReleaseHandle(pCacheAction->syncpoint.hEvent);
        break;

    case BURN_CACHE_ACTION_TYPE_ACQUIRE_CONTAINER:
        ReleaseStr(pCacheAction->resolveContainer.sczUnverifiedPath);
        break;

    case BURN_CACHE_ACTION_TYPE_EXTRACT_CONTAINER:
        ReleaseStr(pCacheAction->extractContainer.sczContainerUnverifiedPath);
        ReleaseMem(pCacheAction->extractContainer.rgPayloads);
        break;

    case BURN_CACHE_ACTION_TYPE_ACQUIRE_PAYLOAD:
        ReleaseStr(pCacheAction->resolvePayload.sczUnverifiedPath);
        break;

    case BURN_CACHE_ACTION_TYPE_CACHE_PAYLOAD:
        ReleaseStr(pCacheAction->cachePayload.sczUnverifiedPath);
        break;
    }
}

static HRESULT GetActionDefaultRequestState(
    __in BOOTSTRAPPER_ACTION action,
    __out BOOTSTRAPPER_REQUEST_STATE* pRequestState
    )
{
    HRESULT hr = S_OK;

    switch (action)
    {
    case BOOTSTRAPPER_ACTION_INSTALL:
        *pRequestState= BOOTSTRAPPER_REQUEST_STATE_PRESENT;
        break;

    case BOOTSTRAPPER_ACTION_REPAIR:
        *pRequestState= BOOTSTRAPPER_REQUEST_STATE_REPAIR;
        break;

    case BOOTSTRAPPER_ACTION_UNINSTALL:
        *pRequestState= BOOTSTRAPPER_REQUEST_STATE_ABSENT;
        break;

    case BOOTSTRAPPER_ACTION_MODIFY:
        *pRequestState= BOOTSTRAPPER_REQUEST_STATE_NONE;
        break;

    default:
        hr = E_INVALIDARG;
        ExitOnRootFailure(hr, "Invalid action state.");
    }

LExit:
        return hr;
}

static BOOL AlreadyPlannedCachePackage(
    __in BURN_PLAN* pPlan,
    __in_z LPCWSTR wzPackageId,
    __out HANDLE* phSyncpointEvent
    )
{
    BOOL fPlanned = FALSE;

    for (DWORD iCacheAction = 0; iCacheAction < pPlan->cCacheActions; ++iCacheAction)
    {
        BURN_CACHE_ACTION* pCacheAction = pPlan->rgCacheActions + iCacheAction;

        if (BURN_CACHE_ACTION_TYPE_PACKAGE_STOP == pCacheAction->type)
        {
            if (CSTR_EQUAL == ::CompareStringW(LOCALE_NEUTRAL, 0, pCacheAction->packageStop.pPackage->sczId, -1, wzPackageId, -1))
            {
                if (iCacheAction + 1 < pPlan->cCacheActions && BURN_CACHE_ACTION_TYPE_SYNCPOINT == pPlan->rgCacheActions[iCacheAction + 1].type)
                {
                    *phSyncpointEvent = pPlan->rgCacheActions[iCacheAction + 1].syncpoint.hEvent;
                }

                fPlanned = TRUE;
                break;
            }
        }
    }

    return fPlanned;
}

static DWORD GetNextCheckpointId()
{
    static DWORD dwCounter = 0;
    return ++dwCounter;
}

static HRESULT AppendCacheAction(
    __in BURN_PLAN* pPlan,
    __out BURN_CACHE_ACTION** ppCacheAction
    )
{
    HRESULT hr = S_OK;

    hr = MemEnsureArraySize(reinterpret_cast<LPVOID*>(&pPlan->rgCacheActions), pPlan->cCacheActions + 1, sizeof(BURN_CACHE_ACTION), 5);
    ExitOnFailure(hr, "Failed to grow plan's array of cache actions.");

    *ppCacheAction = pPlan->rgCacheActions + pPlan->cCacheActions;
    ++pPlan->cCacheActions;

LExit:
    return hr;
}

static HRESULT AppendRollbackCacheAction(
    __in BURN_PLAN* pPlan,
    __out BURN_CACHE_ACTION** ppCacheAction
    )
{
    HRESULT hr = S_OK;

    hr = MemEnsureArraySize(reinterpret_cast<LPVOID*>(&pPlan->rgRollbackCacheActions), pPlan->cRollbackCacheActions + 1, sizeof(BURN_CACHE_ACTION), 5);
    ExitOnFailure(hr, "Failed to grow plan's array of rollback cache actions.");

    *ppCacheAction = pPlan->rgRollbackCacheActions + pPlan->cRollbackCacheActions;
    ++pPlan->cRollbackCacheActions;

LExit:
    return hr;
}

static HRESULT AppendCacheOrLayoutPayloadAction(
    __in BURN_PLAN* pPlan,
    __in_opt BURN_PACKAGE* pPackage,
    __in BURN_PAYLOAD* pPayload,
    __in_z_opt LPCWSTR wzLayoutDirectory
    )
{
    HRESULT hr = S_OK;
    BURN_CACHE_ACTION* pCacheAction = NULL;
    LPWSTR sczPayloadUnverifiedPath = NULL;

    hr = CacheCalculatePayloadUnverifiedPath(pPackage, pPayload, &sczPayloadUnverifiedPath);
    ExitOnFailure(hr, "Failed to calculate unverified path for payload.");

    // If the payload is in a container, ensure the container is being acquired
    // then add this payload to the list of payloads to extract already in the plan.
    if (pPayload->pContainer)
    {
        hr = CreateOrFindContainerExtractAction(pPlan, pPayload->pContainer, &pCacheAction);
        ExitOnFailure(hr, "Failed to find container extract action.");

        Assert(BURN_CACHE_ACTION_TYPE_EXTRACT_CONTAINER == pCacheAction->type);

        hr = MemEnsureArraySize(reinterpret_cast<LPVOID*>(&pCacheAction->extractContainer.rgPayloads), pCacheAction->extractContainer.cPayloads + 1, sizeof(BURN_EXTRACT_PAYLOAD), 5);
        ExitOnFailure(hr, "Failed to grow list of payloads to extract from container.");

        BURN_EXTRACT_PAYLOAD* pExtractPayload = pCacheAction->extractContainer.rgPayloads + pCacheAction->extractContainer.cPayloads;
        pExtractPayload->pPackage = pPackage;
        pExtractPayload->pPayload = pPayload;
        hr = StrAllocString(&pExtractPayload->sczUnverifiedPath, sczPayloadUnverifiedPath, 0);
        ExitOnFailure(hr, "Failed to copy unverified path for payload to extract.");
        ++pCacheAction->extractContainer.cPayloads;

        pCacheAction = NULL;
    }
    else // add a payload acquire action to the plan.
    {
        hr = AppendCacheAction(pPlan, &pCacheAction);
        ExitOnFailure(hr, "Failed to append cache action to acquire payload.");

        pCacheAction->type = BURN_CACHE_ACTION_TYPE_ACQUIRE_PAYLOAD;
        pCacheAction->resolvePayload.pPackage = pPackage;
        pCacheAction->resolvePayload.pPayload = pPayload;
        hr = StrAllocString(&pCacheAction->resolvePayload.sczUnverifiedPath, sczPayloadUnverifiedPath, 0);
        ExitOnFailure(hr, "Failed to copy unverified path for payload to acquire.");

        pCacheAction = NULL;
    }

    hr = AppendCacheAction(pPlan, &pCacheAction);
    ExitOnFailure(hr, "Failed to append cache action to cache payload.");

    pPlan->qwCacheSizeTotal += pPayload->qwFileSize;

    if (NULL == wzLayoutDirectory)
    {
        pCacheAction->type = BURN_CACHE_ACTION_TYPE_CACHE_PAYLOAD;
        pCacheAction->cachePayload.pPackage = pPackage;
        pCacheAction->cachePayload.pPayload = pPayload;
        pCacheAction->cachePayload.fMove = TRUE;
        pCacheAction->cachePayload.sczUnverifiedPath = sczPayloadUnverifiedPath;
        sczPayloadUnverifiedPath = NULL;
    }
    else
    {
        hr = StrAllocString(&pCacheAction->layoutPayload.sczLayoutDirectory, wzLayoutDirectory, 0);
        ExitOnFailure(hr, "Failed to copy layout directory into plan.");

        pCacheAction->type = BURN_CACHE_ACTION_TYPE_LAYOUT_PAYLOAD;
        pCacheAction->layoutPayload.pPackage = pPackage;
        pCacheAction->layoutPayload.pPayload = pPayload;
        pCacheAction->layoutPayload.fMove = TRUE;
        pCacheAction->layoutPayload.sczUnverifiedPath = sczPayloadUnverifiedPath;
        sczPayloadUnverifiedPath = NULL;
    }

     // TODO: Add to the cache size when we send progress for verification: pPlan->qwTotalCacheSize += pPayload->qwFileSize;

    pCacheAction = NULL;

LExit:
    ReleaseStr(sczPayloadUnverifiedPath);
    return hr;
}

static HRESULT CreateOrFindContainerExtractAction(
    __in BURN_PLAN* pPlan,
    __in BURN_CONTAINER* pContainer,
    __out BURN_CACHE_ACTION** ppContainerExtractAction
    )
{
    HRESULT hr = S_OK;
    DWORD iAction = 0;
    LPWSTR sczUnverifiedPath = NULL;
    BURN_CACHE_ACTION* pAcquireContainerAction = NULL;

    *ppContainerExtractAction = NULL;

    // If the container is not attached let's try to find an existing action
    // in the plan to acquire the container.
    if (!pContainer->fPrimary)
    {
        for (iAction = 0; iAction < pPlan->cCacheActions; ++iAction)
        {
            BURN_CACHE_ACTION* pCacheAction = pPlan->rgCacheActions + iAction;
            if (BURN_CACHE_ACTION_TYPE_ACQUIRE_CONTAINER == pCacheAction->type && pCacheAction->resolveContainer.pContainer == pContainer)
            {
                pAcquireContainerAction = pCacheAction;
                break;
            }
#ifdef DEBUG
            else if (BURN_CACHE_ACTION_TYPE_EXTRACT_CONTAINER == pCacheAction->type && pCacheAction->extractContainer.pContainer == pContainer)
            {
                hr = E_UNEXPECTED;
                AssertSz(FALSE, "Container extraction action should not be present in plan before there is an acquire action.");
                ExitOnRootFailure(hr, "Container extraction action should not be present in plan before there is an acquire action.");
            }
#endif
        }
    }

    // If the container is attached (no need to acquire it) or there was
    // already a plan to acquire the container then maybe there is already
    // an action to extract the container.
    if (pContainer->fPrimary || pAcquireContainerAction)
    {
        // Continue searching the plan to see if there is already an action
        // to extract our container.
        for (/* start where we left off above */; iAction < pPlan->cCacheActions; ++iAction)
        {
            BURN_CACHE_ACTION* pCacheAction = pPlan->rgCacheActions + iAction;
            if (BURN_CACHE_ACTION_TYPE_EXTRACT_CONTAINER == pCacheAction->type && pCacheAction->extractContainer.pContainer == pContainer)
            {
                *ppContainerExtractAction = pCacheAction;
                break;
            }
        }
    }
    else // ensure the container is acquired before we try to extract from it.
    {
        hr = CacheCaclulateContainerUnverifiedPath(pContainer, &sczUnverifiedPath);
        ExitOnFailure(hr, "Failed to calculate unverified path for container.");

        hr = AppendCacheAction(pPlan, &pAcquireContainerAction);
        ExitOnFailure(hr, "Failed to append acquire container action to plan.");

        pAcquireContainerAction->type = BURN_CACHE_ACTION_TYPE_ACQUIRE_CONTAINER;
        pAcquireContainerAction->resolveContainer.pContainer = pContainer;
        pAcquireContainerAction->resolveContainer.sczUnverifiedPath = sczUnverifiedPath;
        sczUnverifiedPath = NULL;

        pPlan->qwCacheSizeTotal += pContainer->qwFileSize;
    }

    // If we did not find an action for extracting payloads from this
    // container, create it now.
    if (!*ppContainerExtractAction)
    {
        hr = AppendCacheAction(pPlan, ppContainerExtractAction);
        ExitOnFailure(hr, "Failed to append cache action to extract payloads from container.");

        (*ppContainerExtractAction)->type = BURN_CACHE_ACTION_TYPE_EXTRACT_CONTAINER;
        (*ppContainerExtractAction)->extractContainer.pContainer = pContainer;
        if (pAcquireContainerAction)
        {
            hr = StrAllocString(&(*ppContainerExtractAction)->extractContainer.sczContainerUnverifiedPath, pAcquireContainerAction->resolveContainer.sczUnverifiedPath, 0);
            ExitOnFailure(hr, "Failed to copy container unverified path to cache action to extract container.");
        }
    }

LExit:
    ReleaseStr(sczUnverifiedPath);
    return hr;
}
