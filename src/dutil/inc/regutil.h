//-------------------------------------------------------------------------------------------------
// <copyright file="regutil.h" company="Microsoft">
//    Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// 
// <summary>
//    Registry helper functions.
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once


#ifdef __cplusplus
extern "C" {
#endif

#define ReleaseRegKey(h) if (h) { ::RegCloseKey(h); h = NULL; }


#ifdef __cplusplus
}
#endif

