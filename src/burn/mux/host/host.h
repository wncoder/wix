//-------------------------------------------------------------------------------------------------
// <copyright file="host.h" company="Microsoft">
// Copyright (c) Microsoft Corporation. All rights reserved.
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
// Header for the UX host class.
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once

#include <mscoree.h>
#include <objidl.h>
#include "IBurnUserExperience.h"
#include "IBootstrapperUserExperienceFactory.h"

extern HMODULE g_hInstance;

// Creates the UX and returns it for the engine.
HRESULT CreateUX(__in const BURN_COMMAND *pCommand, __out IBurnUserExperience **ppUX);

// Creates the UX factory to create the managed UX in the default AppDomain.
HRESULT CreateUXFactory(__out IBootstrapperUserExperienceFactory **ppUXFactory);

// Gets the application base path.
HRESULT GetAppBase(__out LPWSTR *ppwzAppBase);

// Gets the CLR host and caches it.
HRESULT GetCLRHost(__out ICLRRuntimeHost **ppCLRHost);

// Sets the manager class type for creating and configurating new AppDomains.
HRESULT SetAppDomainManager(__deref_in ICLRRuntimeHost *pCLRHost);
