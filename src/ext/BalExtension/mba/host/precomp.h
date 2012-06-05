//-------------------------------------------------------------------------------------------------
// <copyright file="precomp.h" company="Microsoft Corporation">
//   Copyright (c) 2004, Microsoft Corporation.
//   This software is released under Common Public License Version 1.0 (CPL).
//   The license and further copyright text can be found in the file LICENSE.TXT
//   LICENSE.TXT at the root directory of the distribution.
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
#include <regutil.h>
#include <strutil.h>
#include <xmlutil.h>

#include "IBootstrapperEngine.h"
#include "IBootstrapperApplication.h"
#include "IBootstrapperApplicationFactory.h"

#include "balutil.h"
