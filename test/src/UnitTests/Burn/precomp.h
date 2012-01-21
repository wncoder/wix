//-------------------------------------------------------------------------------------------------
// <copyright file="precomp.h" company="Microsoft">
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
//    Precompiled header for Burn unit tests.
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once

#include <windows.h>
#include <Bits.h>
#include <msiquery.h>
#include <objbase.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <stdlib.h>
#include <strsafe.h>

#include <dutil.h>
#include <buffutil.h>
#include <dirutil.h>
#include <fileutil.h>
#include <logutil.h>
#include <memutil.h>
#include <pathutil.h>
#include <regutil.h>
#include <resrutil.h>
#include <shelutil.h>
#include <strutil.h>
#include <wiutil.h>
#include <xmlutil.h>
#include <dictutil.h>
#include <deputil.h>

#include <wixver.h>

#include "IBootstrapperEngine.h"
#include "IBootstrapperApplication.h"

#include "platform.h"
#include "variant.h"
#include "variable.h"
#include "condition.h"
#include "search.h"
#include "section.h"
#include "container.h"
#include "catalog.h"
#include "payload.h"
#include "cabextract.h"
#include "userexperience.h"
#include "package.h"
#include "registration.h"
#include "plan.h"
#include "pipe.h"
#include "logging.h"
#include "core.h"
#include "cache.h"
#include "downloadengine.h"
#include "apply.h"
#include "exeengine.h"
#include "msiengine.h"
#include "mspengine.h"
#include "msuengine.h"
#include "dependency.h"
#include "elevation.h"
#include "embedded.h"
#include "manifest.h"
#include "splashscreen.h"
#include "bitsengine.h"

#pragma managed
#include <vcclr.h>

#include "BurnTestException.h"
#include "BurnUnitTest.h"
#include "VariableHelpers.h"
#include "ManifestHelpers.h"
