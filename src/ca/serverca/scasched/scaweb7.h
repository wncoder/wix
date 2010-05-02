#pragma once
//-------------------------------------------------------------------------------------------------
// <copyright file="scaweb7.h" company="Microsoft">
//    Copyright (c) Microsoft Corporation.  All rights reserved.
//
//    The use and distribution terms for this software are covered by the
//    Common Public License 1.0 (http://opensource.org/licenses/cpl.php)
//    which can be found in the file CPL.TXT at the root of this distribution.
//    By using this software in any fashion, you are agreeing to be bound by
//    the terms of this license.
//
//    You must not remove this notice, or any other, from this software.
// </copyright>
//
// <summary>
//    IIS Web functions for CustomActions
// </summary>
//-------------------------------------------------------------------------------------------------

//#include "scawebaddr.h"
#include "scawebapp.h"
#include "scawebprop.h"
#include "scahttpheader.h"
#include "scaweberr.h"
#include "scassl.h"
#include "scaapppool.h"
#include "scaweblog.h"

// globals
#define MAX_ADDRESSES_PER_WEB 10

// structs
struct SCA_WEB7
{
    // darwin information
    WCHAR wzKey[MAX_DARWIN_KEY + 1];
    WCHAR wzComponent[MAX_DARWIN_KEY + 1];
    BOOL fHasComponent;
    INSTALLSTATE isInstalled;
    INSTALLSTATE isAction;

    int iSiteId;

    // metabase information
    WCHAR wzWebBase[METADATA_MAX_NAME_LEN + 1];
    BOOL fBaseExists;

    // iis configuation information
    SCA_WEB_ADDRESS swaBinding;

    SCA_WEB_ADDRESS swaExtraAddresses[MAX_ADDRESSES_PER_WEB + 1];
    DWORD cExtraAddresses;

    WCHAR wzDirectory[MAX_PATH];
    WCHAR wzDescription[MAX_DARWIN_COLUMN + 1];

    int iState;
    int iAttributes;

    BOOL fHasProperties;
    SCA_WEB_PROPERTIES swp;

    BOOL fHasApplication;
    SCA_WEB_APPLICATION swapp;

    BOOL fHasSecurity;
    int dwAccessPermissions;
    int iConnectionTimeout;

    SCA_WEB_SSL_CERTIFICATE* pswscList;
    SCA_HTTP_HEADER* pshhList;
    SCA_WEB_ERROR* psweList;

    BOOL fHasLog;
    SCA_WEB_LOG swl;

    SCA_WEB7* pswNext;
};


// prototypes
HRESULT ScaWebsRead7(
    __in SCA_WEB7** ppswList,
    __in SCA_HTTP_HEADER** pshhList,
    __in SCA_WEB_ERROR** psweList
    );

HRESULT ScaWebsGetBase7(
    __in SCA_WEB7* pswList,
    __in LPCWSTR pswWebKey,
    __in SCA_WEB7** pswWeb
    );

HRESULT ScaWebsInstall7(
    __in SCA_WEB7* pswList,
    __in SCA_APPPOOL * psapList
    );

HRESULT ScaWebsUninstall7(
    __in SCA_WEB7* pswList
    );

void ScaWebsFreeList7(
    __in SCA_WEB7* pswHead
    );
