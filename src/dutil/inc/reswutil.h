//-------------------------------------------------------------------------------------------------
// <copyright file="reswutil.h" company="Microsoft Corporation">
//   Copyright (c) 2004, Microsoft Corporation.
//   This software is released under Common Public License Version 1.0 (CPL).
//   The license and further copyright text can be found in the file LICENSE.TXT
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
// 
// <summary>
//    Resource writer helper functions.
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once


#ifdef __cplusplus
extern "C" {
#endif

HRESULT DAPI ResWriteString(
    __in_z LPCWSTR wzResourceFile,
    __in DWORD dwDataId,
    __in_z LPCWSTR wzData,
    __in WORD wLangId
    );

HRESULT DAPI ResWriteData(
    __in_z LPCWSTR wzResourceFile,
    __in_z LPCSTR szDataName,
    __in PVOID pData,
    __in DWORD cbData
    );

HRESULT DAPI ResImportDataFromFile(
    __in_z LPCWSTR wzTargetFile,
    __in_z LPCWSTR wzSourceFile,
    __in_z LPCSTR szDataName
    );

#ifdef __cplusplus
}
#endif
