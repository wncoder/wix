//-------------------------------------------------------------------------------------------------
// <copyright file="plan.h" company="Microsoft">
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

#pragma once


#if defined(__cplusplus)
extern "C" {
#endif


// constants

enum BURN_CACHE_ACTION_TYPE
{
    BURN_CACHE_ACTION_TYPE_NONE,
    BURN_CACHE_ACTION_TYPE_CHECKPOINT,
    BURN_CACHE_ACTION_TYPE_LAYOUT_BUNDLE,
    BURN_CACHE_ACTION_TYPE_PACKAGE_START,
    BURN_CACHE_ACTION_TYPE_PACKAGE_STOP,
    BURN_CACHE_ACTION_TYPE_ROLLBACK_PACKAGE,
    BURN_CACHE_ACTION_TYPE_SYNCPOINT,
    BURN_CACHE_ACTION_TYPE_ACQUIRE_CONTAINER,
    BURN_CACHE_ACTION_TYPE_EXTRACT_CONTAINER,
    BURN_CACHE_ACTION_TYPE_ACQUIRE_PAYLOAD,
    BURN_CACHE_ACTION_TYPE_CACHE_PAYLOAD,
    BURN_CACHE_ACTION_TYPE_LAYOUT_PAYLOAD,
    BURN_CACHE_ACTION_TYPE_TRANSACTION_BOUNDARY,
};

enum BURN_EXECUTE_ACTION_TYPE
{
    BURN_EXECUTE_ACTION_TYPE_NONE,
    BURN_EXECUTE_ACTION_TYPE_CHECKPOINT,
    BURN_EXECUTE_ACTION_TYPE_SYNCPOINT,
    BURN_EXECUTE_ACTION_TYPE_UNCACHE_PACKAGE,
    BURN_EXECUTE_ACTION_TYPE_EXE_PACKAGE,
    BURN_EXECUTE_ACTION_TYPE_MSI_PACKAGE,
    BURN_EXECUTE_ACTION_TYPE_MSP_TARGET,
    BURN_EXECUTE_ACTION_TYPE_MSU_PACKAGE,
    BURN_EXECUTE_ACTION_TYPE_SERVICE_STOP,
    BURN_EXECUTE_ACTION_TYPE_SERVICE_START,
    BURN_EXECUTE_ACTION_TYPE_DEPENDENCY,
    BURN_EXECUTE_ACTION_TYPE_ROLLBACK_BOUNDARY,
};

enum BURN_DEPENDENCY_ACTION
{
    BURN_DEPENDENCY_ACTION_NONE,
    BURN_DEPENDENCY_ACTION_REGISTER,
    BURN_DEPENDENCY_ACTION_UNREGISTER,
};

enum BURN_CLEAN_ACTION_TYPE
{
    BURN_CLEAN_ACTION_TYPE_NONE,
    BURN_CLEAN_ACTION_TYPE_BUNDLE,
    BURN_CLEAN_ACTION_TYPE_PACKAGE,
};


// structs

typedef struct _BURN_EXTRACT_PAYLOAD
{
    BURN_PACKAGE* pPackage;
    BURN_PAYLOAD* pPayload;
    LPWSTR sczUnverifiedPath;
} BURN_EXTRACT_PAYLOAD;

typedef struct _BURN_CACHE_ACTION
{
    BURN_CACHE_ACTION_TYPE type;
    union
    {
        struct
        {
            DWORD dwId;
        } checkpoint;
        struct
        {
            LPWSTR sczLayoutDirectory;
        } bundleLayout;
        struct
        {
            BURN_PACKAGE* pPackage;
            DWORD cCachePayloads;
            DWORD64 qwCachePayloadSizeTotal;
            DWORD iPackageCompleteAction;
        } packageStart;
        struct
        {
            BURN_PACKAGE* pPackage;
        } packageStop;
        struct
        {
            BURN_PACKAGE* pPackage;
        } rollbackPackage;
        struct
        {
            HANDLE hEvent;
        } syncpoint;
        struct
        {
            BURN_CONTAINER* pContainer;
            LPWSTR sczUnverifiedPath;
        } resolveContainer;
        struct
        {
            BURN_CONTAINER* pContainer;
            LPWSTR sczContainerUnverifiedPath;

            BURN_EXTRACT_PAYLOAD* rgPayloads;
            DWORD cPayloads;
        } extractContainer;
        struct
        {
            BURN_PACKAGE* pPackage;
            BURN_PAYLOAD* pPayload;
            LPWSTR sczUnverifiedPath;
        } resolvePayload;
        struct
        {
            BURN_PACKAGE* pPackage;
            BURN_PAYLOAD* pPayload;
            LPWSTR sczUnverifiedPath;
            BOOL fMove;
        } cachePayload;
        struct
        {
            BURN_PACKAGE* pPackage;
            BURN_PAYLOAD* pPayload;
            LPWSTR sczLayoutDirectory;
            LPWSTR sczUnverifiedPath;
            BOOL fMove;
        } layoutPayload;
        struct
        {
            BURN_ROLLBACK_BOUNDARY* pRollbackBoundary;
            HANDLE hEvent;
        } rollbackBoundary;
    };
} BURN_CACHE_ACTION;

typedef struct _BURN_ORDERED_PATCHES
{
    DWORD dwOrder;
    BURN_PACKAGE* pPackage;
} BURN_ORDERED_PATCHES;

typedef struct _BURN_EXECUTE_ACTION
{
    BURN_EXECUTE_ACTION_TYPE type;
    union
    {
        struct
        {
            DWORD dwId;
        } checkpoint;
        struct
        {
            HANDLE hEvent;
        } syncpoint;
        struct
        {
            BURN_PACKAGE* pPackage;
        } uncachePackage;
        struct
        {
            BURN_PACKAGE* pPackage;
            BOOTSTRAPPER_ACTION_STATE action;
            LPWSTR sczBundleName;
        } exePackage;
        struct
        {
            BURN_PACKAGE* pPackage;
            LPWSTR sczProductCode;
            LPWSTR sczLogPath;
            DWORD dwLoggingAttributes;
            BOOTSTRAPPER_ACTION_STATE action;

            BOOTSTRAPPER_FEATURE_ACTION* rgFeatures;

            BURN_ORDERED_PATCHES* rgOrderedPatches;
            DWORD cPatches;
        } msiPackage;
        struct
        {
            BURN_PACKAGE* pPackage;
            LPWSTR sczTargetProductCode;
            BOOL fPerMachineTarget;
            LPWSTR sczLogPath;
            BOOTSTRAPPER_ACTION_STATE action;

            BURN_ORDERED_PATCHES* rgOrderedPatches;
            DWORD cOrderedPatches;
        } mspTarget;
        struct
        {
            BURN_PACKAGE* pPackage;
            LPWSTR sczLogPath;
            BOOTSTRAPPER_ACTION_STATE action;
        } msuPackage;
        struct
        {
            LPWSTR sczServiceName;
        } service;
        struct
        {
            BURN_ROLLBACK_BOUNDARY* pRollbackBoundary;
        } rollbackBoundary;
        struct
        {
            BURN_PACKAGE* pPackage;
            LPWSTR sczBundleProviderKey;
            BURN_DEPENDENCY_ACTION action;
        } dependency;
    };
} BURN_EXECUTE_ACTION;

typedef struct _BURN_CLEAN_ACTION
{
    BURN_PACKAGE* pPackage;
} BURN_CLEAN_ACTION;

typedef struct _BURN_PLAN
{
    BOOTSTRAPPER_ACTION action;
    BOOL fPerMachine;

    DWORD64 qwCacheSizeTotal;
    DWORD cExecutePackagesTotal;
    DWORD cOverallProgressTicksTotal;

    BURN_CACHE_ACTION* rgCacheActions;
    DWORD cCacheActions;

    BURN_CACHE_ACTION* rgRollbackCacheActions;
    DWORD cRollbackCacheActions;

    BURN_EXECUTE_ACTION* rgExecuteActions;
    DWORD cExecuteActions;

    BURN_EXECUTE_ACTION* rgRollbackActions;
    DWORD cRollbackActions;

    BURN_CLEAN_ACTION* rgCleanActions;
    DWORD cCleanActions;
} BURN_PLAN;


// functions

void PlanUninitialize(
    __in BURN_PLAN* pPlan
    );
void PlanUninitializeExecuteAction(
    __in BURN_EXECUTE_ACTION* pExecuteAction
    );
HRESULT PlanDefaultPackageRequestState(
    __in BURN_PACKAGE_TYPE packageType,
    __in BOOTSTRAPPER_PACKAGE_STATE currentState,
    __in BOOTSTRAPPER_ACTION action,
    __in BURN_VARIABLES* pVariables,
    __in_z_opt LPCWSTR wzInstallCondition,
    __in BOOTSTRAPPER_RELATION_TYPE relationType,
    __out BOOTSTRAPPER_REQUEST_STATE* pRequestState
    );
HRESULT PlanLayoutBundle(
    __in BURN_PLAN* pPlan,
    __in_z LPCWSTR wzLayoutDirectory
    );
HRESULT PlanLayoutOnlyPayload(
    __in BURN_PLAN* pPlan,
    __in BURN_PAYLOAD* pPayload,
    __in_z LPCWSTR wzLayoutDirectory
    );
HRESULT PlanLayoutPackage(
    __in BURN_PLAN* pPlan,
    __in BURN_PACKAGE* pPackage,
    __in_z LPCWSTR wzLayoutDirectory
    );
HRESULT PlanCachePackage(
    __in BURN_PLAN* pPlan,
    __in BURN_PACKAGE* pPackage,
    __out HANDLE* phSyncpointEvent
    );
HRESULT PlanCacheSlipstreamMsps(
    __in BURN_PLAN* pPlan,
    __in BURN_PACKAGE* pPackage
    );
HRESULT PlanCleanPackage(
    __in BURN_PLAN* pPlan,
    __in BURN_PACKAGE* pPackage
    );
HRESULT PlanExecuteCacheSyncAndRollback(
    __in BURN_PLAN* pPlan,
    __in BURN_PACKAGE* pPackage,
    __in HANDLE hCacheEvent,
    __in BOOL fPlanPackageCacheRollback
    );
HRESULT PlanExecuteCheckpoint(
    __in BURN_PLAN* pPlan
    );
HRESULT PlanInsertExecuteAction(
    __in DWORD dwIndex,
    __in BURN_PLAN* pPlan,
    __out BURN_EXECUTE_ACTION** ppExecuteAction
    );
HRESULT PlanAppendExecuteAction(
    __in BURN_PLAN* pPlan,
    __out BURN_EXECUTE_ACTION** ppExecuteAction
    );
HRESULT PlanAppendRollbackAction(
    __in BURN_PLAN* pPlan,
    __out BURN_EXECUTE_ACTION** ppExecuteAction
    );
HRESULT PlanRollbackBoundaryBegin(
    __in BURN_PLAN* pPlan,
    __in BURN_ROLLBACK_BOUNDARY* pRollbackBoundary,
    __out HANDLE* phEvent
    );
HRESULT PlanRollbackBoundaryComplete(
    __in BURN_PLAN* pPlan,
    __in HANDLE hEvent
    );
//HRESULT AppendExecuteWaitAction(
//    __in BURN_PLAN* pPlan,
//    __in HANDLE hEvent
//    );
//HRESULT AppendExecuteCheckpointAction(
//    __in BURN_PLAN* pPlan,
//    __in DWORD dwId
//    );

#if defined(__cplusplus)
}
#endif
