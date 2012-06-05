#pragma once
//-------------------------------------------------------------------------------------------------
// <copyright file="scaweberr.h" company="Microsoft Corporation">
//   Copyright (c) 2004, Microsoft Corporation.
//   This software is released under Common Public License Version 1.0 (CPL).
//   The license and further copyright text can be found in the file LICENSE.TXT
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
// 
// <summary>
//    IIS Web Error functions for CustomActions
// </summary>
//-------------------------------------------------------------------------------------------------

enum eWebErrorParentType { weptVDir = 1, weptWeb };

struct SCA_WEB_ERROR
{
	int iErrorCode;
	int iSubCode;

	int iParentType;
	WCHAR wzParentValue[MAX_DARWIN_KEY + 1];

	WCHAR wzFile[MAX_PATH];
	WCHAR wzURL[MAX_PATH]; // TODO: this needs to be bigger than MAX_PATH
	
	SCA_WEB_ERROR *psweNext;
};

// prototypes
HRESULT ScaWebErrorRead(
                        SCA_WEB_ERROR **ppsweList,
                        __inout LPWSTR *ppwzCustomActionData
                        );
void ScaWebErrorFreeList(SCA_WEB_ERROR *psweList);
HRESULT ScaWebErrorCheckList(SCA_WEB_ERROR* psweList);
HRESULT ScaGetWebError(int iParentType, LPCWSTR wzParentValue, SCA_WEB_ERROR **ppsweList, SCA_WEB_ERROR **ppsweOut);
HRESULT ScaWriteWebError(IMSAdminBase* piMetabase, int iParentType, LPCWSTR wzRoot, SCA_WEB_ERROR* psweList);

