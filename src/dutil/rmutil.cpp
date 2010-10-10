//-------------------------------------------------------------------------------------------------
// <copyright file="rmutil.cpp" company="Microsoft">
//    Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// 
// <summary>
//    Restart Manager utility funtions.
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"
#include <restartmanager.h>

typedef DWORD (WINAPI *PFNRMJOINSESSION)(
    __out DWORD *pSessionHandle,
    __in_z const WCHAR strSessionKey[]
    );

typedef DWORD (WINAPI *PFNRMENDSESSION)(
    __in DWORD dwSessionHandle
    );

typedef DWORD (WINAPI *PFNRMREGISTERRESOURCES)(
    __in DWORD dwSessionHandle,
    __in UINT nFiles,
    __in_z_opt LPWSTR *rgsFilenames,
    __in UINT nApplications,
    __in_opt RM_UNIQUE_PROCESS *rgApplications,
    __in UINT nServices,
    __in_z_opt LPWSTR *rgsServiceNames
    );

typedef struct _RMU_SESSION
{
    CRITICAL_SECTION cs;
    DWORD dwSessionHandle;
    BOOL fStartedSessionHandle;

    UINT cFilenames;
    LPWSTR *rgsczFilenames;

    UINT cServiceNames;
    LPWSTR *rgsczServiceNames;

} RMU_SESSION;

static volatile LONG vcRmuInitialized = 0;
static HMODULE vhModule = NULL;
static PFNRMJOINSESSION vpfnRmJoinSession = NULL;
static PFNRMENDSESSION vpfnRmEndSession = NULL;
static PFNRMREGISTERRESOURCES vpfnRmRegisterResources = NULL;

static HRESULT RmuInitialize();
static void RmuUninitialize();

/********************************************************************
RmuJoinSession - Joins an existing Restart Manager session.

********************************************************************/
extern "C" HRESULT DAPI RmuJoinSession(
    __out PRMU_SESSION *ppSession,
    __in_z LPCWSTR wzSessionKey
    )
{
    HRESULT hr = S_OK;
    DWORD er = ERROR_SUCCESS;
    PRMU_SESSION pSession = NULL;
    HMODULE hModule = NULL;
    PFNRMJOINSESSION pfnRmJoinSession = NULL;

    *ppSession = NULL;

    pSession = static_cast<PRMU_SESSION>(MemAlloc(sizeof(RMU_SESSION), TRUE));
    ExitOnNull(pSession, hr, E_OUTOFMEMORY, "Failed to allocate the RMU_SESSION structure.");

    hr = RmuInitialize();
    ExitOnFailure(hr, "Failed to initialize Restart Manager.");

    er = vpfnRmJoinSession(&pSession->dwSessionHandle, wzSessionKey);
    ExitOnWin32Error1(er, hr, "Failed to join Restart Manager session %ls.", wzSessionKey);

    ::InitializeCriticalSection(&pSession->cs);
    *ppSession = pSession;

LExit:
    if (FAILED(hr))
    {
        ReleaseNullMem(pSession);
    }

    return hr;
}

/********************************************************************
RmuAddFile - Adds the file path to the Restart Manager session.

You should call this multiple times as necessary before calling
RmuRegisterResources.

********************************************************************/
extern "C" HRESULT DAPI RmuAddFile(
    __in PRMU_SESSION pSession,
    __in_z LPCWSTR wzPath
    )
{
    HRESULT hr = S_OK;

    ::EnterCriticalSection(&pSession->cs);

    // Create or grow the jagged array.
    hr = StrArrayAllocString(&pSession->rgsczFilenames, &pSession->cFilenames, wzPath, 0);
    ExitOnFailure(hr, "Failed to add the filename to the array.");

LExit:
    ::LeaveCriticalSection(&pSession->cs);
    return hr;
}

/********************************************************************
RmuAddService - Adds the service name to the Restart Manager session.

You should call this multiple times as necessary before calling
RmuRegisterResources.

********************************************************************/
extern "C" HRESULT DAPI RmuAddService(
    __in PRMU_SESSION pSession,
    __in_z LPCWSTR wzServiceName
    )
{
    HRESULT hr = S_OK;

    ::EnterCriticalSection(&pSession->cs);

    hr = StrArrayAllocString(&pSession->rgsczServiceNames, &pSession->cServiceNames, wzServiceName, 0);
    ExitOnFailure(hr, "Failed to add the service name to the array.");

LExit:
    ::LeaveCriticalSection(&pSession->cs);
    return hr;
}

/********************************************************************
RmuRegisterResources - Registers resources for the Restart Manager.

This should be called rarely because it is expensive to run. Call
functions like RmuAddFile for multiple resources then commit them
as a batch of updates to RmuRegisterResources.

Duplicate resources appear to be handled by Restart Manager.
Only one WM_QUERYENDSESSION is being sent for each top-level window.

********************************************************************/
extern "C" HRESULT DAPI RmuRegisterResources(
    __in PRMU_SESSION pSession
    )
{
    HRESULT hr = S_OK;
    DWORD er = ERROR_SUCCESS;
    HMODULE hModule = NULL;
    PFNRMREGISTERRESOURCES pfnRmRegisterResources = NULL;

    AssertSz(vcRmuInitialized, "Restart Manager was not properly initialized.");

    ::EnterCriticalSection(&pSession->cs);

    er = vpfnRmRegisterResources(
        pSession->dwSessionHandle,
        pSession->cFilenames,
        pSession->rgsczFilenames,
        0,
        NULL,
        pSession->cServiceNames,
        pSession->rgsczServiceNames
        );
    ExitOnWin32Error(er, hr, "Failed to register the resources with the Restart Manager session.");

    // Empty the arrays if registered in case additional resources are added later.
    ReleaseNullStrArray(pSession->rgsczFilenames, pSession->cFilenames);
    ReleaseNullStrArray(pSession->rgsczServiceNames, pSession->cServiceNames);

LExit:
    ::LeaveCriticalSection(&pSession->cs);
    return hr;
}

/********************************************************************
RmuEndSession - Ends the session.

If the session was joined by RmuJoinSession, any remaining resources
are registered before the session is ended (left).

********************************************************************/
extern "C" HRESULT DAPI RmuEndSession(
    __in PRMU_SESSION pSession
    )
{
    HRESULT hr = S_OK;
    DWORD er = ERROR_SUCCESS;
    HMODULE hModule = NULL;
    PFNRMENDSESSION pfnRmEndSession = NULL;

    AssertSz(vcRmuInitialized, "Restart Manager was not properly initialized.");

    // Make sure all resources are registered if we joined the session.
    if (!pSession->fStartedSessionHandle)
    {
        hr = RmuRegisterResources(pSession);
        ExitOnFailure(hr, "Failed to register remaining resources.");
    }

    er = vpfnRmEndSession(pSession->dwSessionHandle);
    ExitOnWin32Error(er, hr, "Failed to end the Restart Manager session.");

LExit:
    ::DeleteCriticalSection(&pSession->cs);

    ReleaseNullStrArray(pSession->rgsczFilenames, pSession->cFilenames);
    ReleaseNullStrArray(pSession->rgsczServiceNames, pSession->cServiceNames);
    ReleaseNullMem(pSession);

    RmuUninitialize();

    return hr;
}

static HRESULT RmuInitialize()
{
    HRESULT hr = S_OK;
    HMODULE hModule = NULL;

    LONG iRef = ::InterlockedIncrement(&vcRmuInitialized);
    if (1 == iRef && !vhModule)
    {
        hModule = ::LoadLibraryW(L"rstrtmgr.dll");
        ExitOnNullWithLastError(hModule, hr, "Failed to load the rstrtmgr.dll module.");

        vpfnRmJoinSession = reinterpret_cast<PFNRMJOINSESSION>(::GetProcAddress(hModule, "RmJoinSession"));
        ExitOnNullWithLastError(vpfnRmJoinSession, hr, "Failed to get the RmJoinSession procedure from rstrtmgr.dll.");

        vpfnRmRegisterResources = reinterpret_cast<PFNRMREGISTERRESOURCES>(::GetProcAddress(hModule, "RmRegisterResources"));
        ExitOnNullWithLastError(vpfnRmRegisterResources, hr, "Failed to get the RmRegisterResources procedure from rstrtmgr.dll.");

        vpfnRmEndSession = reinterpret_cast<PFNRMENDSESSION>(::GetProcAddress(hModule, "RmEndSession"));
        ExitOnNullWithLastError(vpfnRmEndSession, hr, "Failed to get the RmEndSession procedure from rstrtmgr.dll.");

        vhModule = hModule;
    }

LExit:
    return hr;
}

static void RmuUninitialize()
{
    LONG iRef = ::InterlockedDecrement(&vcRmuInitialized);
    if (0 == iRef && vhModule)
    {
        vpfnRmJoinSession = NULL;
        vpfnRmEndSession = NULL;
        vpfnRmRegisterResources = NULL;

        ::FreeLibrary(vhModule);
        vhModule = NULL;
    }
}
