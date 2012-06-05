#pragma once
//-------------------------------------------------------------------------------------------------
// <copyright file="scaapppool7.h" company="Microsoft Corporation">
//   Copyright (c) 2004, Microsoft Corporation.
//   This software is released under Common Public License Version 1.0 (CPL).
//   The license and further copyright text can be found in the file LICENSE.TXT
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
// 
// <summary>
//    IIS Application Pool functions for CustomActions
// </summary>
//-------------------------------------------------------------------------------------------------

#include "scauser.h"

// Identity
#define APATTR_NETSERVICE 0x0001 // Network Service
#define APATTR_LOCSERVICE 0x0002 // Local Service
#define APATTR_LOCSYSTEM 0x0004 // Local System
#define APATTR_OTHERUSER 0x0008 // Other User

// prototypes
HRESULT ScaFindAppPool7(
    __in LPCWSTR wzAppPool,
    __out_ecount(cchName) LPWSTR wzName,
    __in DWORD cchName,
    __in SCA_APPPOOL *psapList
    );

HRESULT ScaAppPoolInstall7(
    __in SCA_APPPOOL* psapList
    );

HRESULT ScaAppPoolUninstall7(
    __in SCA_APPPOOL* psapList
    );

HRESULT ScaWriteAppPool7(
    __in const SCA_APPPOOL* psap
    );

HRESULT ScaRemoveAppPool7(
    __in const SCA_APPPOOL* psap
    );
