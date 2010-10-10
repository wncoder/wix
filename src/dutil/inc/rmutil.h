#pragma once
//-------------------------------------------------------------------------------------------------
// <copyright file="rmutil.h" company="Microsoft">
//    Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// 
// <summary>
//    Header for Restart Manager utility functions.
// </summary>
//-------------------------------------------------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _RMU_SESSION *PRMU_SESSION;

HRESULT DAPI RmuJoinSession(
    __out PRMU_SESSION *ppSession,
    __in_z LPCWSTR wzSessionKey
    );

HRESULT DAPI RmuAddFile(
    __in PRMU_SESSION pSession,
    __in_z LPCWSTR wzPath
    );

HRESULT DAPI RmuAddService(
    __in PRMU_SESSION pSession,
    __in_z LPCWSTR wzServiceName
    );

HRESULT DAPI RmuRegisterResources(
    __in PRMU_SESSION pSession
    );

HRESULT DAPI RmuEndSession(
    __in PRMU_SESSION pSession
    );

#ifdef __cplusplus
}
#endif
