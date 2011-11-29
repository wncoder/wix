//-------------------------------------------------------------------------------------------------
// <copyright file="wuautil.cpp" company="Microsoft">
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
//    Windows Update Agent helper functions.
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"


// internal function declarations

static HRESULT GetAutomaticUpdatesService(
    __out IAutomaticUpdates **ppAutomaticUpdates
    );


// function definitions

extern "C" HRESULT DAPI WuaPauseAutomaticUpdates()
{
    HRESULT hr = S_OK;
    IAutomaticUpdates *pAutomaticUpdates = NULL;

    hr = GetAutomaticUpdatesService(&pAutomaticUpdates);
    ExitOnFailure(hr, "Failed to get the Automatic Updates service.");

    hr = pAutomaticUpdates->Pause();
    ExitOnFailure(hr, "Failed to pause the Automatic Updates service.");

LExit:
    ReleaseObject(pAutomaticUpdates);

    return hr;
}

extern "C" HRESULT DAPI WuaResumeAutomaticUpdates()
{
    HRESULT hr = S_OK;
    IAutomaticUpdates *pAutomaticUpdates = NULL;

    hr = GetAutomaticUpdatesService(&pAutomaticUpdates);
    ExitOnFailure(hr, "Failed to get the Automatic Updates service.");

    hr = pAutomaticUpdates->Resume();
    ExitOnFailure(hr, "Failed to resume the Automatic Updates service.");

LExit:
    ReleaseObject(pAutomaticUpdates);

    return hr;
}

extern "C" HRESULT DAPI WuaRestartRequired(
    __out BOOL* pfRestartRequired
    )
{
    HRESULT hr = S_OK;
    ISystemInformation* pSystemInformation = NULL;
    VARIANT_BOOL bRestartRequired;

    hr = ::CoCreateInstance(__uuidof(SystemInformation), NULL, CLSCTX_INPROC_SERVER, __uuidof(ISystemInformation), reinterpret_cast<LPVOID*>(&pSystemInformation));
    ExitOnRootFailure(hr, "Failed to get WUA system information interface.");

    hr = pSystemInformation->get_RebootRequired(&bRestartRequired);
    ExitOnRootFailure(hr, "Failed to determine if restart is required from WUA.");

    *pfRestartRequired = (VARIANT_FALSE != bRestartRequired);

LExit:
    ReleaseObject(pSystemInformation);

    return hr;
}


// internal function definitions

static HRESULT GetAutomaticUpdatesService(
    __out IAutomaticUpdates **ppAutomaticUpdates
    )
{
    HRESULT hr = S_OK;
    CLSID clsidAutomaticUpdates = { };

    hr = ::CLSIDFromProgID(L"Microsoft.Update.AutoUpdate", &clsidAutomaticUpdates);
    ExitOnFailure(hr, "Failed to get CLSID for Microsoft.Update.AutoUpdate.");

    hr = ::CoCreateInstance(clsidAutomaticUpdates, NULL, CLSCTX_INPROC_SERVER, IID_IAutomaticUpdates, reinterpret_cast<LPVOID*>(ppAutomaticUpdates));
    ExitOnFailure(hr, "Failed to create instance of Microsoft.Update.AutoUpdate.");

LExit:
    return hr;
}
