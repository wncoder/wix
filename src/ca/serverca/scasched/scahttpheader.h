#pragma once
//-------------------------------------------------------------------------------------------------
// <copyright file="scahttpheader.h" company="Microsoft Corporation">
//   Copyright (c) 2004, Microsoft Corporation.
//   This software is released under Common Public License Version 1.0 (CPL).
//   The license and further copyright text can be found in the file LICENSE.TXT
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
// 
// <summary>
//    IIS HTTP Header functions for CustomActions
// </summary>
//-------------------------------------------------------------------------------------------------

enum eHttpHeaderParentType { hhptVDir = 1, hhptWeb };

struct SCA_HTTP_HEADER
{
    int iParentType;
    WCHAR wzParentValue[MAX_DARWIN_KEY + 1];

    WCHAR wzName[MAX_PATH];
    WCHAR wzValue[MAX_PATH];
    int iAttributes;

    SCA_HTTP_HEADER* pshhNext;
};

// prototypes
HRESULT ScaHttpHeaderRead(
    __in SCA_HTTP_HEADER **ppshhList,
    __inout LPWSTR *ppwzCustomActionData
    );
void ScaHttpHeaderFreeList(
    __in SCA_HTTP_HEADER *pshhList
    );
HRESULT ScaHttpHeaderCheckList(
    __in SCA_HTTP_HEADER* pshhList
    );
HRESULT ScaGetHttpHeader(
    __in int iParentType,
    __in LPCWSTR wzParentValue,
    __in SCA_HTTP_HEADER** ppshhList,
    __out SCA_HTTP_HEADER** ppshhOut
    );
HRESULT ScaWriteHttpHeader(
    __in IMSAdminBase* piMetabase,
    LPCWSTR wzRoot,
    SCA_HTTP_HEADER* pshhList
    );
