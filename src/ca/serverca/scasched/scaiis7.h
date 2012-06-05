#pragma once
//-------------------------------------------------------------------------------------------------
// <copyright file="scaiis7.h" company="Microsoft Corporation">
//   Copyright (c) 2004, Microsoft Corporation.
//   This software is released under Common Public License Version 1.0 (CPL).
//   The license and further copyright text can be found in the file LICENSE.TXT
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
// 
// <summary>
//    IIS functions for CustomActions
// </summary>
//-------------------------------------------------------------------------------------------------

HRESULT ScaScheduleIIS7Configuration();

HRESULT ScaIIS7ConfigTransaction(__in_z LPCWSTR wzBackup);

HRESULT ScaCreateApp7(__in_z LPCWSTR wzWebRoot, DWORD dwIsolation);

HRESULT ScaDeleteConfigElement(IIS_CONFIG_ACTION emElement, LPCWSTR wzSubKey);

HRESULT ScaWriteConfigString(__in_z const LPCWSTR wzValue);

HRESULT ScaWriteConfigID(IIS_CONFIG_ACTION emID);

HRESULT ScaWriteConfigInteger(DWORD dwValue);