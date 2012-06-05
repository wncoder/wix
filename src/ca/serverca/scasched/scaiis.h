#pragma once
//-------------------------------------------------------------------------------------------------
// <copyright file="scaiis.h" company="Microsoft Corporation">
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

HRESULT ScaMetabaseTransaction(__in_z LPCWSTR wzBackup);

HRESULT ScaCreateWeb(IMSAdminBase* piMetabase, LPCWSTR wzWeb, LPCWSTR wzWebBase);

HRESULT ScaDeleteApp(IMSAdminBase* piMetabase, LPCWSTR wzWebRoot);

HRESULT ScaCreateApp(IMSAdminBase* piMetabase, LPCWSTR wzWebRoot,
                     DWORD dwIsolation);

HRESULT ScaCreateMetabaseKey(IMSAdminBase* piMetabase, LPCWSTR wzRootKey,
                             LPCWSTR wzSubKey);

HRESULT ScaDeleteMetabaseKey(IMSAdminBase* piMetabase, LPCWSTR wzRootKey,
                             LPCWSTR wzSubKey);

HRESULT ScaWriteMetabaseValue(IMSAdminBase* piMetabase, LPCWSTR wzRootKey,
                              LPCWSTR wzSubKey, DWORD dwIdentifier,
                              DWORD dwAttributes, DWORD dwUserType,
                              DWORD dwDataType, LPVOID pvData);

HRESULT ScaDeleteMetabaseValue(IMSAdminBase* piMetabase, LPCWSTR wzRootKey,
                              LPCWSTR wzSubKey, DWORD dwIdentifier,
                              DWORD dwDataType);

HRESULT ScaWriteConfigurationScript(LPCWSTR pwzCaScriptKey);

HRESULT ScaAddToIisConfiguration(LPCWSTR pwzData, DWORD dwCost);

HRESULT ScaLoadMetabase(IMSAdminBase** piMetabase);