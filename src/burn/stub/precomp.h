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
//    Precompiled header for setup stub.
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once

#include <windows.h>
#include <msiquery.h>
#include <objbase.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <stdlib.h>
#include <strsafe.h>

#include <dutil.h>
#include <memutil.h>
#include <dirutil.h>
#include <fileutil.h>
#include <logutil.h>
#include <pathutil.h>
#include <resrutil.h>
#include <strutil.h>
#include <xmlutil.h>

#include "IBurnCore.h"
#include "IBurnUserExperience.h"

#include "IBurnPayload.h"

#include "platform.h"
#include "variant.h"
#include "variable.h"
#include "condition.h"
#include "search.h"
#include "payload.h"
#include "container.h"
#include "cabextract.h"
#include "cache.h"
#include "userexperience.h"
#include "registration.h"
#include "elevation.h"
#include "package.h"
#include "exeengine.h"
#include "msiengine.h"
#include "msuengine.h"
#include "core.h"
#include "manifest.h"
