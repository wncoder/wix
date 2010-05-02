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

#define _CRT_RAND_S
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS
#define SECURITY_WIN32

#define SM_SERVERR2    89

#include <windows.h>
#include <tchar.h>
#include <wmistr.h>
#include <evntrace.h>
#include <math.h>
#include <msi.h>
#include <msiquery.h>
#include <msidefs.h>
#include <objbase.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <stdlib.h>
#include <strsafe.h>
#include <intsafe.h>
#include <taskschd.h>
#include <stddef.h>
#include <oaidl.h>
#include <security.h>
#include <wmistr.h>
#include <psapi.h>
#include <softpub.h>
#include <taskschd.h>
#include <msxml2.h>
#include <tlhelp32.h>
#include <wuapi.h>

#define ReleaseBuffer _ReleaseBuffer

#include <atlbase.h>
#include <atlstr.h>
#include <atlpath.h>
#include <atlsimpcoll.h>
#include <atlfile.h>
#include <atltime.h>
#include <atlutil.h>
#include <atlsecurity.h>

#undef ReleaseBuffer

#include <dutil.h>
#include <buffutil.h>
#include <cabutil.h>
#include <memutil.h>
#include <dirutil.h>
#include <fileutil.h>
#include <logutil.h>
#include <osutil.h>
#include <pathutil.h>
#include <regutil.h>
#include <resrutil.h>
#include <strutil.h>
#include <wiutil.h>
#include <xmlutil.h>
#include <cryputil.h>
#include <osutil.h>
#include <shelutil.h>

#include "IBurnCore.h"
#include "IBurnPayload.h"
#include "IBurnUserExperience.h"
#include "IBurnView.h"
#include "payloads.h"
#include "UnknownImpl.h"

#include "cache.h"
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
#include "pipe.h"
