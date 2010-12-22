//-------------------------------------------------------------------------------------------------
// <copyright file="precomp.h" company="Microsoft">
// Copyright (c) Microsoft Corporation. All rights reserved.
// </copyright>
// 
// <summary>
// Pre-compiled header.
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once

#include <windows.h>
#include <msiquery.h>
#include <mscoree.h>

#import <mscorlib.tlb> raw_interfaces_only rename("ReportEvent", "mscorlib_ReportEvent")

#include <dutil.h>
#include <pathutil.h>
#include <strutil.h>

#include "IBootstrapperEngine.h"
#include "IBootstrapperApplication.h"
#include "IBootstrapperApplicationFactory.h"

#include "balutil.h"
