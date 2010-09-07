//-------------------------------------------------------------------------------------------------
// <copyright file="plan.h" company="Microsoft">
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

#pragma once


#if defined(__cplusplus)
extern "C" {
#endif


// constants

enum BURN_CACHE_ACTION_TYPE
{
    BURN_CACHE_ACTION_TYPE_NONE,
    BURN_CACHE_ACTION_TYPE_PACKAGE_START,
    BURN_CACHE_ACTION_TYPE_CHECKPOINT,
    BURN_CACHE_ACTION_TYPE_ACQUIRE_CONTAINER,
    BURN_CACHE_ACTION_TYPE_EXTRACT_CONTAINER,
    BURN_CACHE_ACTION_TYPE_ACQUIRE_PAYLOAD,
    BURN_CACHE_ACTION_TYPE_CACHE_PAYLOAD,
};

enum BURN_EXECUTE_ACTION_TYPE
{
    BURN_EXECUTE_ACTION_TYPE_NONE,
    BURN_EXECUTE_ACTION_TYPE_CHECKPOINT,
    BURN_EXECUTE_ACTION_TYPE_WAIT,
    BURN_EXECUTE_ACTION_TYPE_EXE_PACKAGE,
    BURN_EXECUTE_ACTION_TYPE_MSI_PACKAGE,
    BURN_EXECUTE_ACTION_TYPE_MSU_PACKAGE,
    BURN_EXECUTE_ACTION_TYPE_SERVICE_STOP,
    BURN_EXECUTE_ACTION_TYPE_SERVICE_START,
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
            BURN_PACKAGE* pPackage;
            DWORD cCachePayloads;
            DWORD64 qwCachePayloadSizeTotal;
        } packageStart;
        struct
        {
            BURN_PACKAGE* pPackage;
            HANDLE hEvent;
        } checkpoint;
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
    };
} BURN_CACHE_ACTION;

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
        } wait;
        struct
        {
            BURN_PACKAGE* pPackage;
            BOOTSTRAPPER_ACTION_STATE action;
        } exePackage;
        struct
        {
            BURN_PACKAGE* pPackage;
            LPWSTR sczProductCode;
            LPWSTR sczLogPath;
            BOOTSTRAPPER_ACTION_STATE action;

            BOOTSTRAPPER_FEATURE_ACTION* rgFeatures;

            BURN_PACKAGE** rgpPatches;
            DWORD cPatches;
        } msiPackage;
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
    };
} BURN_EXECUTE_ACTION;

typedef struct _BURN_CLEAN_ACTION
{
    BURN_CLEAN_ACTION_TYPE type;
    union
    {
        struct
        {
            BURN_RELATED_BUNDLE* pBundle;
        } bundle;
        struct
        {
            BURN_PACKAGE* pPackage;
        } package;
    };
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
    __in BOOTSTRAPPER_PACKAGE_STATE currentState,
    __in BOOTSTRAPPER_ACTION action,
    __in BURN_VARIABLES* pVariables,
    __in_z_opt LPCWSTR wzInstallCondition,
    __out BOOTSTRAPPER_REQUEST_STATE* pRequestState
    );
HRESULT PlanCachePackage(
    __in BURN_PLAN* pPlan,
    __in BURN_PACKAGE* pPackage,
    __out HANDLE* phCheckpointEvent
    );
HRESULT PlanCleanBundle(
    __in BURN_PLAN* pPlan,
    __in BURN_RELATED_BUNDLE* pRelatedBundle
    );
HRESULT PlanCleanPackage(
    __in BURN_PLAN* pPlan,
    __in BURN_PACKAGE* pPackage
    );
DWORD PlanGetNextCheckpointId();
HRESULT PlanAppendExecuteAction(
    __in BURN_PLAN* pPlan,
    __out BURN_EXECUTE_ACTION** ppExecuteAction
    );
HRESULT PlanAppendRollbackAction(
    __in BURN_PLAN* pPlan,
    __out BURN_EXECUTE_ACTION** ppExecuteAction
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
