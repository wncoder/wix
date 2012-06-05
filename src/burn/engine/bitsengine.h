//-------------------------------------------------------------------------------------------------
// <copyright file="bitsengine.h" company="Microsoft Corporation">
//   Copyright (c) 2004, Microsoft Corporation.
//   This software is released under Common Public License Version 1.0 (CPL).
//   The license and further copyright text can be found in the file LICENSE.TXT
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
// 
// <summary>
//    Setup chainer/bootstrapper BITS based download engine for WiX toolset.
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once


#ifdef __cplusplus
extern "C" {
#endif

// structs


// functions

HRESULT BitsDownloadUrl(
    __in BURN_CACHE_CALLBACK* pCallback,
    __in BURN_DOWNLOAD_SOURCE* pDownloadSource,
    __in LPCWSTR wzDestinationPath
    );


#ifdef __cplusplus
}
#endif
