#pragma once
//-------------------------------------------------------------------------------------------------
// <copyright file="cpisubsexec.h" company="Microsoft Corporation">
//   Copyright (c) 2004, Microsoft Corporation.
//   This software is released under Common Public License Version 1.0 (CPL).
//   The license and further copyright text can be found in the file LICENSE.TXT
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
// 
// <summary>
//    COM+ subscription functions for CustomActions
// </summary>
//-------------------------------------------------------------------------------------------------


// function prototypes

HRESULT CpiConfigureSubscriptions(
	LPWSTR* ppwzData,
	HANDLE hRollbackFile
	);
HRESULT CpiRollbackConfigureSubscriptions(
	LPWSTR* ppwzData,
	CPI_ROLLBACK_DATA* pRollbackDataList
	);
