//-------------------------------------------------------------------------------------------------
// <copyright file="host.cpp" company="Microsoft">
// Copyright (c) Microsoft Corporation. All rights reserved.
// </copyright>
// 
// <summary>
// Source for the UX host class.
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"
#include "host.h"

// Include the generated assembly name macros.
#include <mux.h>

static ICLRRuntimeHost *g_pCLRHost;

EXTERN_C HRESULT WINAPI SetupUXCreate(
    __deref_in const BURN_COMMAND *pCommand,
    __out IBurnUserExperience **ppUX
    )
{
   HRESULT hr = S_OK; 

    if (NULL == g_hInstance)
    {
        hr = E_HANDLE;
    }
    else 
    {
        // Assumes that COM was already initialized on the thread.
        hr = CreateUX(pCommand, ppUX);
        ExitOnFailure(hr, "Failed to create the UX.");
    }
   
LExit:

    return hr;
}

HRESULT CreateUX(__in const BURN_COMMAND *pCommand, __out IBurnUserExperience **ppUX)
{
    HRESULT hr = S_OK;
    IBootstrapperUserExperienceFactory *pUXFactory = NULL;

    hr = CreateUXFactory(&pUXFactory);
    ExitOnFailure(hr, "Failed to create the UX factory to create the UX.");

    hr = pUXFactory->Create(pCommand, ppUX);
    ExitOnFailure(hr, "Failed to create the UX.");

LExit:

    ReleaseNullObject(pUXFactory);

    return hr;
}

HRESULT CreateUXFactory(__out IBootstrapperUserExperienceFactory **ppUXFactory)
{
    HRESULT hr = S_OK;
    ICLRRuntimeHost *pCLRHost = NULL;

    // The CLR must first be loaded.
    hr = GetCLRHost(&pCLRHost);
    ExitOnFailure(hr, "Failed to create the CLR host.");

    // Add an additional reference to keep the CLR host alive even after we release it below.
    pCLRHost->AddRef();

    hr = SetAppDomainManager(pCLRHost);
    ExitOnFailure(hr, "Failed to set the AppDomainManager class type information to create the UX factory.");

    hr = pCLRHost->Start();
    ExitOnFailure(hr, "Failed to start the CLR host.");

    // The 2.0 hosting APIs are our baseline but are deprecated with 4.0,
    // but will not be removed from mscoree.dll for compatibility.
    #pragma warning(push)
    #pragma warning(disable:4996)

    // Now create the UX factory.
    hr = ::ClrCreateManagedInstance(
        L"Microsoft.Tools.WindowsInstallerXml.Bootstrapper.UserExperienceFactory, " MUX_ASSEMBLY_FULL_NAME,
        __uuidof(IBootstrapperUserExperienceFactory),
        (LPVOID*) ppUXFactory);
    ExitOnFailure(hr, "Failed to create the UX factory.");

    #pragma warning(pop)

LExit:

    ReleaseNullObject(pCLRHost);

    return hr;
}

HRESULT GetAppBase(__out LPWSTR *ppwzAppBase)
{
    HRESULT hr = S_OK;
    LPWSTR pwzFullPath = NULL;
    LPWSTR pwzAppBase = NULL;

    hr = PathForCurrentProcess(&pwzFullPath, NULL);
    ExitOnFailure(hr, "Failed to get the full process path.");

    hr = PathGetDirectory(pwzFullPath, ppwzAppBase);
    ExitOnFailure(hr, "Failed to get the directory of the full process path.");

LExit:

    ReleaseStr(pwzAppBase);
    ReleaseStr(pwzFullPath);

    return hr;
}

HRESULT GetCLRHost(__out ICLRRuntimeHost **ppCLRHost)
{
    HRESULT hr = S_OK;
    LPWSTR pwzAppBase = NULL;
    LPWSTR pwzConfigPath = NULL;
    UINT uiMode = 0;

    // Cache the CLR host to be shutdown later. This can occur on a different thread.
    if (!g_pCLRHost)
    {
        hr = GetAppBase(&pwzAppBase);
        ExitOnFailure(hr, "Failed to get the application base path for the configuration file.");

        hr = PathConcat(pwzAppBase, L"mux.config", &pwzConfigPath);
        ExitOnFailure(hr, "Failed to get the full path to the application configuration file.");

        // Disable message boxes from being displayed on error and blocking execution.
        uiMode = ::SetErrorMode(0);
        ::SetErrorMode(uiMode | SEM_FAILCRITICALERRORS);

        // The 2.0 hosting APIs are our baseline but are deprecated with 4.0,
        // but will not be removed from mscoree.dll for compatibility.
        #pragma warning(push)
        #pragma warning(disable:4996)

        hr = ::CorBindToCurrentRuntime(
            pwzConfigPath,
            CLSID_CLRRuntimeHost,
            IID_ICLRRuntimeHost,
            (LPVOID*) &g_pCLRHost);
        ExitOnFailure(hr, "Failed to create the CLR host using the application configuration file path.");

        #pragma warning(pop)
    }

    *ppCLRHost = g_pCLRHost;

LExit:

    // Restore the previous error mode.
    ::SetErrorMode(uiMode);

    ReleaseStr(pwzConfigPath);
    ReleaseStr(pwzAppBase);

    return hr;
}

HRESULT SetAppDomainManager(__deref_in ICLRRuntimeHost *pCLRHost)
{
    HRESULT hr = S_OK;
    ICLRControl *pCLRControl = NULL;

    hr = pCLRHost->GetCLRControl(&pCLRControl);
    ExitOnFailure(hr, "Failed to get the ICLRControl object to set the AppDomainManager.");

    hr = pCLRControl->SetAppDomainManagerType(MUX_ASSEMBLY_FULL_NAME, L"Microsoft.Tools.WindowsInstallerXml.Bootstrapper.MuxDomainManager");
    ExitOnFailure(hr, "Failed to set the AppDomainManager type information for the CLR host.");

LExit:

    ReleaseNullObject(pCLRControl);

    return hr;
}
