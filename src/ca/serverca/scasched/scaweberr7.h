#pragma once
//-------------------------------------------------------------------------------------------------
// <copyright file="scaweberr7.h" company="Microsoft">
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
//    IIS Web Error functions for CustomActions
// </summary>
//-------------------------------------------------------------------------------------------------

// prototypes
HRESULT ScaWebErrorRead7(
    SCA_WEB_ERROR **ppsweList
    );

void ScaWebErrorFreeList7(SCA_WEB_ERROR *psweList);

HRESULT ScaWebErrorCheckList7(SCA_WEB_ERROR* psweList);

HRESULT ScaGetWebError7(
    int iParentType,
    __in_z LPCWSTR wzParentValue,
    SCA_WEB_ERROR **ppsweList,
    SCA_WEB_ERROR **ppsweOut
    );

HRESULT ScaWriteWebError7(
    __in_z LPCWSTR wzWebName,
    __in_z LPCWSTR wzRoot,
    SCA_WEB_ERROR* psweList
    );

