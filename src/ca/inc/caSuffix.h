#pragma once
//-------------------------------------------------------------------------------------------------
// <copyright file="caSuffix.h" company="Microsoft Corporation">
//   Copyright (c) 2004, Microsoft Corporation.
//   This software is released under Common Public License Version 1.0 (CPL).
//   The license and further copyright text can be found in the file LICENSE.TXT
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
// 
// <summary>
//    Platform specific suffix defines/utilities.
//    Must be kept in sync with caSuffix.wxi.
// </summary>
//-------------------------------------------------------------------------------------------------

#if defined _WIN64 // x64 and ia64 get this one
#define PLATFORM_DECORATION(f) f ## L"_64"
#else
#define PLATFORM_DECORATION(f) f
#endif
