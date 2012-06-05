#pragma once
//-------------------------------------------------------------------------------------------------
// <copyright file="scafilter7.h" company="Microsoft Corporation">
//   Copyright (c) 2004, Microsoft Corporation.
//   This software is released under Common Public License Version 1.0 (CPL).
//   The license and further copyright text can be found in the file LICENSE.TXT
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
// 
// <summary>
//    IIS Filter functions for CustomActions
// </summary>
//-------------------------------------------------------------------------------------------------

#include "scaweb.h"

// prototypes
UINT __stdcall ScaFiltersRead7(
    __in SCA_WEB7* pswList,
    __in WCA_WRAPQUERY_HANDLE hWebBaseQuery, 
    __inout SCA_FILTER** ppsfList,
    __inout LPWSTR *ppwzCustomActionData
    );

HRESULT ScaFiltersInstall7(
    SCA_FILTER* psfList
    );

HRESULT ScaFiltersUninstall7(
    SCA_FILTER* psfList
    );
