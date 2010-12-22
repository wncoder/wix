//-------------------------------------------------------------------------------------------------
// <copyright file="precomp.h" company="Microsoft">
//    Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// 
// <summary>
//    Precompiled header for MBA prerequisite bootstrapper application.
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once

#include <windows.h>
#include <shlobj.h>
#include <strsafe.h>

#include <dutil.h>
#include <pathutil.h>
#include <shelutil.h>
#include <strutil.h>
#include <thmutil.h>

#include "BalBaseBootstrapperApplication.h"
#include "balutil.h"

HRESULT CreateBootstrapperApplication(
    __in HMODULE hModule,
    __in IBootstrapperEngine* pEngine,
    __in const BOOTSTRAPPER_COMMAND* pCommand,
    __out IBootstrapperApplication **ppApplication
    );
