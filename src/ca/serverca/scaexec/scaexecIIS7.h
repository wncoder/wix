#pragma once
//-------------------------------------------------------------------------------------------------
// <copyright file="scaexecIIS7.h" company="Microsoft Corporation">
//   Copyright (c) 2004, Microsoft Corporation.
//   This software is released under Common Public License Version 1.0 (CPL).
//   The license and further copyright text can be found in the file LICENSE.TXT
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
// 
// <summary>
//    execution CustomActions for IIS7
// </summary>
//-------------------------------------------------------------------------------------------------

HRESULT IIS7ConfigChanges(MSIHANDLE hInstall, __inout LPWSTR pwzData);
