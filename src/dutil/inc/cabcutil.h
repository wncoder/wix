#pragma once
//-------------------------------------------------------------------------------------------------
// <copyright file="cabcutil.h" company="Microsoft Corporation">
//   Copyright (c) 2004, Microsoft Corporation.
//   This software is released under Common Public License Version 1.0 (CPL).
//   The license and further copyright text can be found in the file LICENSE.TXT
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
// 
// <summary>
//    Header for cabinet creation helper functions.
// </summary>
//-------------------------------------------------------------------------------------------------

#include <fci.h>
#include <fcntl.h>
#include <msi.h>

#define CAB_MAX_SIZE 0x7FFFFFFF   // (see KB: Q174866)

#ifdef __cplusplus
extern "C" {
#endif

extern const int CABC_HANDLE_BYTES;

// time vs. space trade-off
enum COMPRESSION_TYPE 
{ 
    COMPRESSION_TYPE_NONE, // fastest
    COMPRESSION_TYPE_LOW, 
    COMPRESSION_TYPE_MEDIUM,
    COMPRESSION_TYPE_HIGH, // smallest
    COMPRESSION_TYPE_MSZIP
};

// functions
HRESULT DAPI CabCBegin(
    __in_z LPCWSTR wzCab,
    __in_z LPCWSTR wzCabDir,
    __in DWORD dwMaxFiles,
    __in DWORD dwMaxSize,
    __in DWORD dwMaxThresh,
    __in COMPRESSION_TYPE ct,
    __out_bcount(CABC_HANDLE_BYTES) HANDLE *phContext
    );
HRESULT DAPI CabCNextCab(
    __in_bcount(CABC_HANDLE_BYTES) HANDLE hContext
    );
HRESULT DAPI CabCAddFile(
    __in_z LPCWSTR wzFile,
    __in_z_opt LPCWSTR wzToken,
    __in_opt PMSIFILEHASHINFO pmfHash,
    __in_bcount(CABC_HANDLE_BYTES) HANDLE hContext
    );
HRESULT DAPI CabCFinish(
    __in_bcount(CABC_HANDLE_BYTES) HANDLE hContext
    );
void DAPI CabCCancel(
    __in_bcount(CABC_HANDLE_BYTES) HANDLE hContext
    );

#ifdef __cplusplus
}
#endif
