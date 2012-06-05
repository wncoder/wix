//-------------------------------------------------------------------------------------------------
// <copyright file="precomp.h" company="Microsoft Corporation">
//   Copyright (c) 2004, Microsoft Corporation.
//   This software is released under Common Public License Version 1.0 (CPL).
//   The license and further copyright text can be found in the file LICENSE.TXT
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
// 
//-------------------------------------------------------------------------------------------------
#pragma once

#include <windows.h>
#include <msiquery.h>
#include <wchar.h>

const WCHAR MAGIC_MULTISZ_DELIM = 128;

#include "wixstrsafe.h"
#include "wcautil.h"
#include "wcalog.h"
#include "wcawow64.h"
#include "wcawrapquery.h"
#include "wiutil.h"
#include "fileutil.h"
#include "memutil.h"
#include "strutil.h"
