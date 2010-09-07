//-------------------------------------------------------------------------------------------------
// <copyright file="precomp.h" company="Microsoft">
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
//    Precompiled header for setup chainer/bootstrapper core.
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once


// In "development builds" have all the ExitXxx macros log.
#ifdef BURN_DEVELOPMENT_BUILD
#define ExitTrace LogErrorString
#define ExitTrace1 LogErrorString
#define ExitTrace2 LogErrorString
#define ExitTrace3 LogErrorString
#endif

#include <wixver.h>

#include <windows.h>
#include <Bits.h>
#include <math.h>
#include <msiquery.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <softpub.h>
#include <strsafe.h>
#include <intsafe.h>

#include <dutil.h>
#include <buffutil.h>
#include <cabutil.h>
#include <certutil.h>
#include <cryputil.h>
#include <dirutil.h>
#include <fileutil.h>
#include <logutil.h>
#include <memutil.h>
#include <osutil.h>
#include <pathutil.h>
#include <procutil.h>
#include <regutil.h>
#include <resrutil.h>
#include <shelutil.h>
#include <strutil.h>
#include <svcutil.h>
#include <wiutil.h>
#include <xmlutil.h>

#include "IBootstrapperEngine.h"
#include "IBootstrapperApplication.h"

#include "platform.h"
#include "variant.h"
#include "variable.h"
#include "condition.h"
#include "search.h"
#include "container.h"
#include "payload.h"
#include "package.h"
#include "cabextract.h"
#include "cache.h"
#include "bitsengine.h"
#include "downloadengine.h"
#include "userexperience.h"
#include "logging.h"
#include "registration.h"
#include "plan.h"
#include "exeengine.h"
#include "msiengine.h"
#include "msuengine.h"
#include "elevation.h"
#include "core.h"
#include "apply.h"
#include "manifest.h"
#include "pipe.h"

#include "EngineForApplication.h"
#include "engine.messages.h"
