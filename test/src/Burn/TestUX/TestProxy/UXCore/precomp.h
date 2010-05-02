//-------------------------------------------------------------------------------------------------
// <copyright file="precomp.h" company="Microsoft">
//    Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// 
// <summary>
//    Precompiled header for setup chainer/bootstrapper core.
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once

#include <windows.h>
#include <gdiplus.h>
#include <msiquery.h>
#include <objbase.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <stdlib.h>
#include <strsafe.h>
#include <limits.h>

#include "dutil.h"
#include "pathutil.h"
#include "strutil.h"
#include "locutil.h"
#include "thmutil.h"
#include "xmlutil.h"

#include "IBurnCore.h"
#include "IBurnUserExperience.h"

const LPCWSTR DOTNET20 = L"v2.0.50727";
const LPCWSTR WORKSTATION_BUILD = L"wks";

BOOL ManagedDependenciesInstalled();