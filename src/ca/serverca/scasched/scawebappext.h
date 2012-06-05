#pragma once
//-------------------------------------------------------------------------------------------------
// <copyright file="scawebappext.h" company="Microsoft Corporation">
//   Copyright (c) 2004, Microsoft Corporation.
//   This software is released under Common Public License Version 1.0 (CPL).
//   The license and further copyright text can be found in the file LICENSE.TXT
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
// 
// <summary>
//    Functions for dealing with Web Application Extensions in Server CustomActions
// </summary>
//-------------------------------------------------------------------------------------------------

// structs
struct SCA_WEB_APPLICATION_EXTENSION
{
    WCHAR wzExtension[MAX_DARWIN_COLUMN + 1];

    WCHAR wzVerbs[MAX_DARWIN_COLUMN + 1];
    WCHAR wzExecutable[MAX_DARWIN_COLUMN + 1];
    int iAttributes;

    SCA_WEB_APPLICATION_EXTENSION* pswappextNext;
};


// prototypes
HRESULT ScaWebAppExtensionsRead(
    __in LPCWSTR wzApplication,
    __in WCA_WRAPQUERY_HANDLE hWebAppExtQuery,
    __inout SCA_WEB_APPLICATION_EXTENSION** ppswappextList
    );

HRESULT ScaWebAppExtensionsWrite(
    __in IMSAdminBase* piMetabase,
    __in LPCWSTR wzRootOfWeb,
    __in SCA_WEB_APPLICATION_EXTENSION* pswappextList
    );

void ScaWebAppExtensionsFreeList(
    __in SCA_WEB_APPLICATION_EXTENSION* pswappextList
    );
