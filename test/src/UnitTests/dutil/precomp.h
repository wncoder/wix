//-------------------------------------------------------------------------------------------------
// <copyright file="precomp.h" company="Microsoft Corporation">
//   Copyright (c) 2004, Microsoft Corporation.
//   This software is released under Common Public License Version 1.0 (CPL).
//   The license and further copyright text can be found in the file LICENSE.TXT
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
// 
// <summary>
//    Precompiled header for DUtil unit tests.
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once

#include <windows.h>
#include <strsafe.h>
#include <ShlObj.h>

// Include error.h before dutil.h
#include "error.h"
#include <dutil.h>

#include <dictutil.h>
#include <dirutil.h>
#include <fileutil.h>
#include <iniutil.h>
#include <memutil.h>
#include <pathutil.h>
#include <strutil.h>

#pragma managed
#include <vcclr.h>
