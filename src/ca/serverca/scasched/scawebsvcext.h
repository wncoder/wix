#pragma once
//-------------------------------------------------------------------------------------------------
// <copyright file="scawebsvcext.h" company="Microsoft Corporation">
//   Copyright (c) 2004, Microsoft Corporation.
//   This software is released under Common Public License Version 1.0 (CPL).
//   The license and further copyright text can be found in the file LICENSE.TXT
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
// 
// <summary>
//    IIS Web Service Extension functions for CustomActions
// </summary>
//-------------------------------------------------------------------------------------------------

enum SCA_WEBSVCEXT_ATTRIBUTES { SWSEATTRIB_ALLOW = 1, SWSEATTRIB_UIDELETABLE = 2 };

struct SCA_WEBSVCEXT
{
    // darwin information
    INSTALLSTATE isInstalled;
    INSTALLSTATE isAction;

    // iis configuation information
    WCHAR wzFile[MAX_PATH + 1];
    WCHAR wzDescription[MAX_DARWIN_COLUMN + 1];
    WCHAR wzGroup[MAX_DARWIN_COLUMN + 1];

    int iAttributes;

    SCA_WEBSVCEXT* psWseNext;
};

HRESULT __stdcall ScaWebSvcExtRead(
    __in SCA_WEBSVCEXT** ppsWseList,
    __inout LPWSTR *ppwzCustomActionData
    );

HRESULT ScaWebSvcExtCommit(
    __in IMSAdminBase* piMetabase,
    __in SCA_WEBSVCEXT* psWseList
    );

void ScaWebSvcExtFreeList(
    __in SCA_WEBSVCEXT* psWseList
    );
