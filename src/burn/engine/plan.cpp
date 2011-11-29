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
    __in BOOL fPermanent,
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
static BOOL ProcessSharedPayload(
    __in BURN_PLAN* pPlan,
    __in BURN_PAYLOAD* pPayload
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

    if (pPlan->rgPlannedProviders)
    {
        ReleaseDependencyArray(pPlan->rgPlannedProviders, pPlan->cPlannedProviders);
    }

    memset(pPlan, 0, sizeof(BURN_PLAN));
}

extern "C" void PlanUninitializeExecuteAction(
    __in BURN_EXECUTE_ACTION* pExecuteAction
    )
{
    switch (pExecuteAction->type)
    {
    case BURN_EXECUTE_ACTION_TYPE_EXE_PACKAGE:
        ReleaseStr(pExecuteAction->exePackage.sczIgnoreDependencies);
        break;

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
    __in BOOL fPermanent,
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
    // the package is superseded then default to doing nothing except during uninstall of
    // MSPs. Superseded patches are actually installed and can be removed.
    else if (BOOTSTRAPPER_PACKAGE_STATE_SUPERSEDED == currentState && !(BOOTSTRAPPER_ACTION_UNINSTALL == action && BURN_PACKAGE_TYPE_MSP == packageType))
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
        hr = GetActionDefaultRequestState(action, fPermanent, &defaultRequestState);
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
    __in_z LPCWSTR wzExecutableName,
    __in BURN_VARIABLES* pVariables,
    __in BURN_PAYLOADS* pPayloads,
    __out_z LPWSTR* psczLayoutDirectory
    )
{
    HRESULT hr = S_OK;
    BURN_CACHE_ACTION* pCacheAction = NULL;
    LPWSTR sczLayoutDirectory = NULL;

    // Get the layout directory.
    hr = VariableGetString(pVariables, BURN_BUNDLE_LAYOUT_DIRECTORY, &sczLayoutDirectory);
    if (E_NOTFOUND == hr) // if not set, use the current directory as the layout directory.
    {
        hr = DirGetCurrent(&sczLayoutDirectory);
        ExitOnFailure(hr, "Failed to get current directory as layout directory.");
    }
    ExitOnFailure(hr, "Failed to get bundle layout directory property.");

    hr = PathBackslashTerminate(&sczLayoutDirectory);
    ExitOnFailure(hr, "Failed to ensure layout directory is backslash terminated.");

    // Plan the layout of the bundle engine itself.
    hr = AppendCacheAction(pPlan, &pCacheAction);
    ExitOnFailure(hr, "Failed to append bundle start action.");

    pCacheAction->type = BURN_CACHE_ACTION_TYPE_LAYOUT_BUNDLE;

    hr = StrAllocString(&pCacheAction->bundleLayout.sczExecutableName, wzExecutableName, 0);
    ExitOnFailure(hr, "Failed to to copy executable name for bundle.");

    hr = StrAllocString(&pCacheAction->bundleLayout.sczLayoutDirectory, sczLayoutDirectory, 0);
    ExitOnFailure(hr, "Failed to to copy layout directory for bundle.");

    hr = CacheCalculateBundleLayoutWorkingPath(pPlan->wzBundleId, &pCacheAction->bundleLayout.sczUnverifiedPath);
    ExitOnFailure(hr, "Failed to calculate bundle layout working path.");

    ++pPlan->cOverallProgressTicksTotal;

    // Plan the layout of layout-only payloads.
    for (DWORD i = 0; i < pPayloads->cPayloads; ++i)
    {
        BURN_PAYLOAD* pPayload = pPayloads->rgPayloads + i;
        if (pPayload->fLayoutOnly)
        {
            hr = AppendCacheOrLayoutPayloadAction(pPlan, NULL, pPayload, sczLayoutDirectory);
            ExitOnFailure(hr, "Failed to plan layout payload.");
        }
    }

    *psczLayoutDirectory = sczLayoutDirectory;
    sczLayoutDirectory = NULL;

LExit:
    ReleaseStr(sczLayoutDirectory);

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

    // Update the start action with the location of the complete action.
    pPlan->rgCacheActions[iPackageStartAction].packageStart.iPackageCompleteAction = pPlan->cCacheActions - 1;

    ++pPlan->cOverallProgressTicksTotal;

LExit:
    return hr;
}

extern "C" HRESULT PlanExecutePackage(
    __in BOOTSTRAPPER_DISPLAY display,
    __in BURN_USER_EXPERIENCE* pUserExperience,
    __in BURN_PLAN* pPlan,
    __in BURN_PACKAGE* pPackage,
    __in BURN_LOGGING* pLog,
    __in BURN_VARIABLES* pVariables,
    __in LPCWSTR wzBundleProviderKey,
    __inout HANDLE* phSyncpointEvent,
    __out BOOTSTRAPPER_ACTION_STATE* pExecuteAction,
    __out BOOTSTRAPPER_ACTION_STATE* pRollbackAction,
    __out BURN_DEPENDENCY_ACTION* pDependencyAction,
    __out BOOL* pfPlannedCachePackage,
    __out BOOL* pfPlannedCleanPackage
    )
{
    HRESULT hr = S_OK;
    BOOL fNeedsCache = FALSE;
    BOOL fPlannedCachePackage = FALSE;
    BOOL fPlannedCleanPackage = FALSE;

    // Calculate execute actions.
    switch (pPackage->type)
    {
    case BURN_PACKAGE_TYPE_EXE:
        hr = ExeEnginePlanCalculatePackage(pPackage);
        break;

    case BURN_PACKAGE_TYPE_MSI:
        hr = MsiEnginePlanCalculatePackage(pPackage, pVariables, pUserExperience);
        break;

    case BURN_PACKAGE_TYPE_MSP:
        hr = MspEnginePlanCalculatePackage(pPackage, pUserExperience);
        break;

    case BURN_PACKAGE_TYPE_MSU:
        hr = MsuEnginePlanCalculatePackage(pPackage);
        break;

    default:
        hr = E_UNEXPECTED;
        ExitOnFailure(hr, "Invalid package type.");
    }
    ExitOnFailure1(hr, "Failed to calculate plan actions for package: %ls", pPackage->sczId);

    // Calculate package states based on reference count and plan certain dependency actions prior to planning the package execute action.
    hr = DependencyPlanPackageBegin(pPackage, pPlan, wzBundleProviderKey, pDependencyAction);
    ExitOnFailure1(hr, "Failed to plan dependency actions to unregister package: %ls", pPackage->sczId);

    // Exe packages require the package for all operations (even uninstall).
    if (BURN_PACKAGE_TYPE_EXE == pPackage->type)
    {
        fNeedsCache = (BOOTSTRAPPER_ACTION_STATE_NONE != pPackage->execute || BOOTSTRAPPER_ACTION_STATE_NONE != pPackage->rollback);
    }
    else // the other engine types can uninstall without the original package.
    {
        fNeedsCache = (BOOTSTRAPPER_ACTION_STATE_UNINSTALL < pPackage->execute || BOOTSTRAPPER_ACTION_STATE_UNINSTALL < pPackage->rollback);
    }

    if (fNeedsCache)
    {
        if (pPackage->fCache)
        {
            // Add the cache size to the estimated size
            pPlan->qwEstimatedSize += pPackage->qwSize;
        }

        // If the package needs to be cached and is not already cached, plan the cache action first. Even packages that are marked
        // Cache='no' need to be cached so we can check the hash and ensure the correct package is installed. We'll clean up Cache='no'
        // packages after applying the changes.
        if (!pPackage->fCached)
        {
            // If this is an MSI package with slipstream MSPs, ensure the MSPs are cached first.
            if (BURN_PACKAGE_TYPE_MSI == pPackage->type && 0 < pPackage->Msi.cSlipstreamMspPackages)
            {
                hr = PlanCacheSlipstreamMsps(pPlan, pPackage);
                ExitOnFailure(hr, "Failed to plan slipstream patches for package.");
            }

            hr = PlanCachePackage(pPlan, pPackage, phSyncpointEvent);
            ExitOnFailure(hr, "Failed to plan cache package.");

            fPlannedCachePackage = TRUE;
        }
    }

    // Add the installed size to estimated size if it will be on the machine at the end of the install
    if (BOOTSTRAPPER_REQUEST_STATE_PRESENT == pPackage->requested || (BOOTSTRAPPER_PACKAGE_STATE_PRESENT == pPackage->currentState && (BOOTSTRAPPER_REQUEST_STATE_ABSENT != pPackage->requested && BOOTSTRAPPER_REQUEST_STATE_CACHE != pPackage->requested)))
    {
        // MSP packages get cached automatically by windows installer with any embedded cabs, so include that in the size as well
        if (BURN_PACKAGE_TYPE_MSP == pPackage->type)
        {
            pPlan->qwEstimatedSize += pPackage->qwSize;
        }

        pPlan->qwEstimatedSize += pPackage->qwInstallSize;
    }

    // Add execute actions.
    switch (pPackage->type)
    {
    case BURN_PACKAGE_TYPE_EXE:
        hr = ExeEnginePlanAddPackage(NULL, pPackage, pPlan, pLog, pVariables, *phSyncpointEvent, fPlannedCachePackage);
        break;

    case BURN_PACKAGE_TYPE_MSI:
        hr = MsiEnginePlanAddPackage(NULL, display, pPackage, pPlan, pLog, pVariables, *phSyncpointEvent, fPlannedCachePackage);
        break;

    case BURN_PACKAGE_TYPE_MSP:
        hr = MspEnginePlanAddPackage(NULL, display, pPackage, pPlan, pLog, pVariables, *phSyncpointEvent, fPlannedCachePackage);
        break;

    case BURN_PACKAGE_TYPE_MSU:
        hr = MsuEnginePlanAddPackage(NULL, pPackage, pPlan, pLog, pVariables, *phSyncpointEvent, fPlannedCachePackage);
        break;

    default:
        hr = E_UNEXPECTED;
        ExitOnFailure(hr, "Invalid package type.");
    }
    ExitOnFailure1(hr, "Failed to add plan actions for package: %ls", pPackage->sczId);

    // Plan certain dependency actions after planning the package execute action.
    hr = DependencyPlanPackageComplete(pPackage, pPlan, wzBundleProviderKey, pDependencyAction);
    ExitOnFailure1(hr, "Failed to plan dependency actions to register package: %ls", pPackage->sczId);

    // If we are going to take any action on this package, add progress for it.
    if (BOOTSTRAPPER_ACTION_STATE_NONE != pPackage->execute || BOOTSTRAPPER_ACTION_STATE_NONE != pPackage->rollback)
    {
        LoggingIncrementPackageSequence();

        ++pPlan->cExecutePackagesTotal;
        ++pPlan->cOverallProgressTicksTotal;

        // If package is per-machine and is being executed, flag the plan to be per-machine as well.
        if (pPackage->fPerMachine)
        {
            pPlan->fPerMachine = TRUE;
        }
    }

    // If the package is scheduled to be cached or is already cached but we are removing it or the package
    // is not supposed to stay cached then ensure the package is cleaned up.
    if ((fPlannedCachePackage || pPackage->fCached) && (BOOTSTRAPPER_ACTION_STATE_UNINSTALL == pPackage->execute || !pPackage->fCache))
    {
        hr = PlanCleanPackage(pPlan, pPackage);
        ExitOnFailure(hr, "Failed to plan clean package.");

        fPlannedCleanPackage = TRUE;
    }

    *pExecuteAction = pPackage->execute;
    *pRollbackAction = pPackage->rollback;
    *pfPlannedCachePackage = fPlannedCachePackage;
    *pfPlannedCleanPackage = fPlannedCleanPackage;

LExit:
    return hr;
}

extern "C" HRESULT PlanRelatedBundles(
    __in BOOTSTRAPPER_ACTION action,
    __in BURN_USER_EXPERIENCE* pUserExperience,
    __in BURN_RELATED_BUNDLES* pRelatedBundles,
    __in DWORD64 qwBundleVersion,
    __in BURN_PLAN* pPlan,
    __in BURN_LOGGING* pLog,
    __in BURN_VARIABLES* pVariables,
    __inout HANDLE* phSyncpointEvent,
    __in DWORD dwExecuteActionEarlyIndex
    )
{
    HRESULT hr = S_OK;
    LPWSTR sczIgnoreDependencies = NULL;
    BOOTSTRAPPER_REQUEST_STATE defaultRequested = BOOTSTRAPPER_REQUEST_STATE_NONE;

    // Get the list of dependencies to ignore to pass to related bundles.
    hr = DependencyPlanRelatedBundles(pPlan, &sczIgnoreDependencies);
    ExitOnFailure(hr, "Failed to get the list of dependencies to ignore.");

    for (DWORD i = 0; i < pRelatedBundles->cRelatedBundles; ++i)
    {
        DWORD *pdwInsertIndex = NULL;
        BURN_RELATED_BUNDLE* pRelatedBundle = pRelatedBundles->rgRelatedBundles + i;
        BOOTSTRAPPER_REQUEST_STATE requested = BOOTSTRAPPER_REQUEST_STATE_NONE;

        switch (pRelatedBundle->relationType)
        {
        case BOOTSTRAPPER_RELATION_UPGRADE:
            requested = qwBundleVersion > pRelatedBundle->qwVersion ? BOOTSTRAPPER_REQUEST_STATE_ABSENT : BOOTSTRAPPER_REQUEST_STATE_NONE;
            break;
        case BOOTSTRAPPER_RELATION_PATCH: __fallthrough;
        case BOOTSTRAPPER_RELATION_ADDON:
            // Addon and patch bundles will be passed a list of dependencies to ignore for planning.
            hr = StrAllocString(&pRelatedBundle->package.Exe.sczIgnoreDependencies, sczIgnoreDependencies, 0);
            ExitOnFailure(hr, "Failed to copy the list of dependencies to ignore.");

            if (BOOTSTRAPPER_ACTION_UNINSTALL == action)
            {
                requested = BOOTSTRAPPER_REQUEST_STATE_ABSENT;
                // Uninstall addons early in the chain, before other packages are installed
                pdwInsertIndex = &dwExecuteActionEarlyIndex;
            }
            else if (BOOTSTRAPPER_ACTION_REPAIR == action)
            {
                requested = BOOTSTRAPPER_REQUEST_STATE_REPAIR;
            }
            break;
        case BOOTSTRAPPER_RELATION_DETECT:
            break;
        default:
            hr = E_UNEXPECTED;
            ExitOnFailure1(hr, "Unexpected relation type encountered during plan: %d", pRelatedBundle->relationType);
            break;
        }

        defaultRequested = requested;

        int nResult = pUserExperience->pUserExperience->OnPlanRelatedBundle(pRelatedBundle->package.sczId, &requested);
        hr = HRESULT_FROM_VIEW(nResult);
        ExitOnRootFailure(hr, "UX aborted plan related bundle.");

        // Log when the UX changed the bundle state so the engine doesn't get blamed for planning the wrong thing.
        if (requested != defaultRequested)
        {
            LogId(REPORT_STANDARD, MSG_PLANNED_BUNDLE_UX_CHANGED_REQUEST, pRelatedBundle->package.sczId, LoggingRequestStateToString(requested), LoggingRequestStateToString(defaultRequested));
        }

        if (BOOTSTRAPPER_REQUEST_STATE_NONE != requested)
        {
            pRelatedBundle->package.requested = requested;

            hr = ExeEnginePlanCalculatePackage(&pRelatedBundle->package);
            ExitOnFailure1(hr, "Failed to calcuate plan for related bundle: %ls", pRelatedBundle->package.sczId);

            hr = ExeEnginePlanAddPackage(pdwInsertIndex, &pRelatedBundle->package, pPlan, pLog, pVariables, *phSyncpointEvent, FALSE);
            ExitOnFailure1(hr, "Failed to add to plan related bundle: %ls", pRelatedBundle->package.sczId);

            LoggingIncrementPackageSequence();
            ++pPlan->cExecutePackagesTotal;
            ++pPlan->cOverallProgressTicksTotal;

            // If package is per-machine and is being executed, flag the plan to be per-machine as well.
            if (pRelatedBundle->package.fPerMachine)
            {
                pPlan->fPerMachine = TRUE;
            }
        }
    }

LExit:
    ReleaseStr(sczIgnoreDependencies);

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

    // Update the start action with the location of the complete action.
    pPlan->rgCacheActions[iPackageStartAction].packageStart.iPackageCompleteAction = pPlan->cCacheActions - 1;

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

    case BURN_CACHE_ACTION_TYPE_LAYOUT_BUNDLE:
        ReleaseStr(pCacheAction->bundleLayout.sczExecutableName);
        ReleaseStr(pCacheAction->bundleLayout.sczLayoutDirectory);
        ReleaseStr(pCacheAction->bundleLayout.sczUnverifiedPath);
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
    __in BOOL fPermanent,
    __out BOOTSTRAPPER_REQUEST_STATE* pRequestState
    )
{
    HRESULT hr = S_OK;

    switch (action)
    {
    case BOOTSTRAPPER_ACTION_INSTALL:
        *pRequestState = BOOTSTRAPPER_REQUEST_STATE_PRESENT;
        break;

    case BOOTSTRAPPER_ACTION_REPAIR:
        *pRequestState = BOOTSTRAPPER_REQUEST_STATE_REPAIR;
        break;

    case BOOTSTRAPPER_ACTION_UNINSTALL:
        *pRequestState = fPermanent ? BOOTSTRAPPER_REQUEST_STATE_NONE : BOOTSTRAPPER_REQUEST_STATE_ABSENT;
        break;

    case BOOTSTRAPPER_ACTION_MODIFY:
        *pRequestState = BOOTSTRAPPER_REQUEST_STATE_NONE;
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
    LPWSTR sczPayloadWorkingPath = NULL;

    hr = CacheCalculatePayloadWorkingPath(pPlan->wzBundleId, pPayload, &sczPayloadWorkingPath);
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
        hr = StrAllocString(&pExtractPayload->sczUnverifiedPath, sczPayloadWorkingPath, 0);
        ExitOnFailure(hr, "Failed to copy unverified path for payload to extract.");
        ++pCacheAction->extractContainer.cPayloads;

        pCacheAction = NULL;
    }
    else // add a payload acquire action to the plan.
    {
        // If a payload is shared and processed/acquired/downloaded before, skip the acquire operation
        if (!ProcessSharedPayload(pPlan, pPayload))
        {
            hr = AppendCacheAction(pPlan, &pCacheAction);
            ExitOnFailure(hr, "Failed to append cache action to acquire payload.");

            pCacheAction->type = BURN_CACHE_ACTION_TYPE_ACQUIRE_PAYLOAD;
            pCacheAction->resolvePayload.pPackage = pPackage;
            pCacheAction->resolvePayload.pPayload = pPayload;
            hr = StrAllocString(&pCacheAction->resolvePayload.sczUnverifiedPath, sczPayloadWorkingPath, 0);
            ExitOnFailure(hr, "Failed to copy unverified path for payload to acquire.");

            pCacheAction = NULL;
        }
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
        pCacheAction->cachePayload.sczUnverifiedPath = sczPayloadWorkingPath;
        sczPayloadWorkingPath = NULL;
    }
    else
    {
        hr = StrAllocString(&pCacheAction->layoutPayload.sczLayoutDirectory, wzLayoutDirectory, 0);
        ExitOnFailure(hr, "Failed to copy layout directory into plan.");

        pCacheAction->type = BURN_CACHE_ACTION_TYPE_LAYOUT_PAYLOAD;
        pCacheAction->layoutPayload.pPackage = pPackage;
        pCacheAction->layoutPayload.pPayload = pPayload;
        pCacheAction->layoutPayload.fMove = TRUE;
        pCacheAction->layoutPayload.sczUnverifiedPath = sczPayloadWorkingPath;
        sczPayloadWorkingPath = NULL;
    }

    pCacheAction = NULL;

LExit:
    ReleaseStr(sczPayloadWorkingPath);
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
    LPWSTR sczContainerWorkingPath = NULL;
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
        hr = CacheCaclulateContainerWorkingPath(pPlan->wzBundleId, pContainer, &sczContainerWorkingPath);
        ExitOnFailure(hr, "Failed to calculate unverified path for container.");

        hr = AppendCacheAction(pPlan, &pAcquireContainerAction);
        ExitOnFailure(hr, "Failed to append acquire container action to plan.");

        pAcquireContainerAction->type = BURN_CACHE_ACTION_TYPE_ACQUIRE_CONTAINER;
        pAcquireContainerAction->resolveContainer.pContainer = pContainer;
        pAcquireContainerAction->resolveContainer.sczUnverifiedPath = sczContainerWorkingPath;
        sczContainerWorkingPath = NULL;

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
    ReleaseStr(sczContainerWorkingPath);

    return hr;
}

static BOOL ProcessSharedPayload(
    __in BURN_PLAN* pPlan,
    __in BURN_PAYLOAD* pPayload
    )
{
    DWORD cMove = 0;
    DWORD cAcquirePayload = 0;

    for (DWORD i = 0; i < pPlan->cCacheActions; ++i)
    {
        BURN_CACHE_ACTION* pCacheAction = pPlan->rgCacheActions + i;

        if (BURN_CACHE_ACTION_TYPE_ACQUIRE_PAYLOAD == pCacheAction->type &&
            pCacheAction->resolvePayload.pPayload == pPayload)
        {
            cAcquirePayload++;
        }
        else if (BURN_CACHE_ACTION_TYPE_CACHE_PAYLOAD == pCacheAction->type &&
                 pCacheAction->cachePayload.pPayload == pPayload &&
                 pCacheAction->cachePayload.fMove)
        {
            // Since we found a shared payload, change its operation from MOVE to COPY if necessary
            pCacheAction->cachePayload.fMove = FALSE;

            // In theory, we can early exit the for loop at this point. We decide to walk through the whole
            // list to make sure there is no broken data here.
            cMove++;
        }
        else if (BURN_CACHE_ACTION_TYPE_LAYOUT_PAYLOAD == pCacheAction->type &&
                 pCacheAction->layoutPayload.pPayload == pPayload &&
                 pCacheAction->layoutPayload.fMove)
        {
            // Since we found a shared payload, change its operation from MOVE to COPY if necessary
            pCacheAction->layoutPayload.fMove = FALSE;

            // In theory, we can early exit the for loop at this point. We decide to walk through the whole
            // list to make sure there is no broken data here.
            cMove++;
        }
    }

#ifdef DEBUG
    if (cAcquirePayload > 0)
    {
        AssertSz(cAcquirePayload < 2, "Shared payload should not be acquired more than once.");
        AssertSz(cMove == 1, "Shared payload should be moved once and only once.");
    }
#endif

    return (cAcquirePayload > 0);
}
