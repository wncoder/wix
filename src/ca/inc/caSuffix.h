#pragma once
//-------------------------------------------------------------------------------------------------
// <copyright file="caSuffix.h" company="Microsoft">
//    Copyright (c) Microsoft Corporation.  All rights reserved.
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
