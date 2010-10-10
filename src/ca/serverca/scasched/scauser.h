#pragma once
//-------------------------------------------------------------------------------------------------
// <copyright file="scauser.h" company="Microsoft">
//    Copyright (c) Microsoft Corporation.  All rights reserved.
//    
//    The use and distribution terms for this software are covered by the
//    Common Public License 1.0 (http://opensource.org/licenses/cpl1.0.php)
//    which can be found in the file CPL.TXT at the root of this distribution.
//    By using this software in any fashion, you are agreeing to be bound by
//    the terms of this license.
//    
//    You must not remove this notice, or any other, from this software.
// </copyright>
// 
// <summary>
//    User functions for CustomActions
// </summary>
//-------------------------------------------------------------------------------------------------

// enums
enum USER_EXISTS
{
    USER_EXISTS_YES,
    USER_EXISTS_NO,
    USER_EXISTS_INDETERMINATE
};

// structs
struct SCA_GROUP
{
    WCHAR wzKey[MAX_DARWIN_KEY + 1];
    WCHAR wzComponent[MAX_DARWIN_KEY + 1];

    WCHAR wzDomain[MAX_DARWIN_COLUMN + 1];
    WCHAR wzName[MAX_DARWIN_COLUMN + 1];

    SCA_GROUP *psgNext;
};

struct SCA_USER
{
    WCHAR wzKey[MAX_DARWIN_KEY + 1];
    WCHAR wzComponent[MAX_DARWIN_KEY + 1];
    INSTALLSTATE isInstalled;
    INSTALLSTATE isAction;

    WCHAR wzDomain[MAX_DARWIN_COLUMN + 1];
    WCHAR wzName[MAX_DARWIN_COLUMN + 1];
    WCHAR wzPassword[MAX_DARWIN_COLUMN + 1];
    INT iAttributes;

    SCA_GROUP *psgGroups;

    SCA_USER *psuNext;
};


// prototypes
HRESULT __stdcall ScaGetUser(
    __in LPCWSTR wzUser, 
    __out SCA_USER* pscau
    );
HRESULT __stdcall ScaGetUserDeferred(
    __in LPCWSTR wzUser, 
    __in WCA_WRAPQUERY_HANDLE hUserQuery,
    __out SCA_USER* pscau
    );
HRESULT __stdcall ScaGetGroup(
    __in LPCWSTR wzGroup, 
    __out SCA_GROUP* pscag
    );
void ScaUserFreeList(
    __in SCA_USER* psuList
    );
void ScaGroupFreeList(
    __in SCA_GROUP* psgList
    );
HRESULT ScaUserRead(
    __inout SCA_USER** ppsuList
    );
HRESULT ScaUserExecute(
    __in SCA_USER *psuList
    );
