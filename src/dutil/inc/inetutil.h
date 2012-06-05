#pragma once
//-------------------------------------------------------------------------------------------------
// <copyright file="inetutil.h" company="Microsoft Corporation">
//   Copyright (c) 2004, Microsoft Corporation.
//   This software is released under Common Public License Version 1.0 (CPL).
//   The license and further copyright text can be found in the file LICENSE.TXT
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
// 
// <summary>
//    Internet utilites.
// </summary>
//-------------------------------------------------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif

#define ReleaseInternet(h) if (h) { ::InternetCloseHandle(h); h = NULL; }
#define ReleaseNullInternet(h) if (h) { ::InternetCloseHandle(h); h = NULL; }


// functions
HRESULT DAPI InternetGetSizeByHandle(
    __in HINTERNET hiFile,
    __out LONGLONG* pllSize
    );

HRESULT DAPI InternetGetCreateTimeByHandle(
    __in HINTERNET hiFile,
    __out LPFILETIME pft
    );

HRESULT DAPI InternetQueryInfoString(
    __in HINTERNET h,
    __in DWORD dwInfo,
    __deref_out_z LPWSTR* psczValue
    );

HRESULT DAPI InternetQueryInfoNumber(
    __in HINTERNET h,
    __in DWORD dwInfo,
    __out LONG* plInfo
    );

#ifdef __cplusplus
}
#endif

