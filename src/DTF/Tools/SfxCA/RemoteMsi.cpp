//---------------------------------------------------------------------
// <copyright file="RemoteMsi.cpp" company="Microsoft">
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
// <summary>
// Part of the Deployment Tools Foundation project.
// </summary>
//---------------------------------------------------------------------

#include "precomp.h"
#include "RemoteMsiSession.h"


static __success(return == 0) UINT EnsureBufSize(__deref_out_ecount(cchRequired) wchar_t** pszBuf, __inout DWORD* pcchBuf, DWORD cchRequired)
{
	if (*pcchBuf < cchRequired)
	{
		if (*pszBuf != NULL)
		{
			delete[] *pszBuf;
		}

		*pcchBuf = cchRequired;
		*pszBuf = new wchar_t[cchRequired];

		if (*pszBuf == NULL)
		{
			return ERROR_OUTOFMEMORY;
		}
	}

	return ERROR_SUCCESS;
}

typedef int (WINAPI *PMsiFunc_I_I)(int in1, __out int* out1);
typedef int (WINAPI *PMsiFunc_II_I)(int in1, int in2, __out int* out1);
typedef int (WINAPI *PMsiFunc_IS_I)(int in1, __in_z wchar_t* in2, __out int* out1);
typedef int (WINAPI *PMsiFunc_ISI_I)(int in1, __in_z wchar_t* in2, int in3, __out int* out1);
typedef int (WINAPI *PMsiFunc_ISII_I)(int in1, __in_z wchar_t* in2, int in3, int in4, __out int* out1);
typedef int (WINAPI *PMsiFunc_IS_II)(int in1, __in_z wchar_t* in2, __out int* out1, __out int* out2);
typedef int (WINAPI *PMsiFunc_I_S)(int in1, __out_ecount_full(*cchOut1) wchar_t* out1, __inout DWORD* cchOut1);
typedef int (WINAPI *PMsiFunc_II_S)(int in1, int in2, __out_ecount_full(*cchOut1) wchar_t* out1, __inout DWORD* cchOut1);
typedef int (WINAPI *PMsiFunc_IS_S)(int in1, __in_z wchar_t* in2, __out_ecount_full(*cchOut1) wchar_t* out1, __inout DWORD* cchOut1);
typedef int (WINAPI *PMsiFunc_ISII_SII)(int in1, __in_z wchar_t* in2, int in3, int in4, __out_ecount_full(*cchOut1) wchar_t* out1, __inout DWORD* cchOut1, __out int* out2, __out int* out3);

UINT MsiFunc_I_I(PMsiFunc_I_I func, const RemoteMsiSession::RequestData* pReq, RemoteMsiSession::RequestData* pResp)
{
	int in1 = pReq->fields[0].iValue;
	int out1;
	UINT ret = (UINT) func(in1, &out1);
	if (ret == 0)
	{
		pResp->fields[1].vt = VT_I4;
		pResp->fields[1].iValue = out1;
	}
	return ret;
}

UINT MsiFunc_II_I(PMsiFunc_II_I func, const RemoteMsiSession::RequestData* pReq, RemoteMsiSession::RequestData* pResp)
{
	int in1 = pReq->fields[0].iValue;
	int in2 = pReq->fields[1].iValue;
	int out1;
	UINT ret = (UINT) func(in1, in2, &out1);
	if (ret == 0)
	{
		pResp->fields[1].vt = VT_I4;
		pResp->fields[1].iValue = out1;
	}
	return ret;
}

UINT MsiFunc_IS_I(PMsiFunc_IS_I func, const RemoteMsiSession::RequestData* pReq, RemoteMsiSession::RequestData* pResp)
{
	int in1 = pReq->fields[0].iValue;
	wchar_t* in2 = pReq->fields[1].szValue;
	int out1;
	UINT ret = (UINT) func(in1, in2, &out1);
	if (ret == 0)
	{
		pResp->fields[1].vt = VT_I4;
		pResp->fields[1].iValue = out1;
	}
	return ret;
}

UINT MsiFunc_ISI_I(PMsiFunc_ISI_I func, const RemoteMsiSession::RequestData* pReq, RemoteMsiSession::RequestData* pResp)
{
	int in1 = pReq->fields[0].iValue;
	wchar_t* in2 = pReq->fields[1].szValue;
	int in3 = pReq->fields[2].iValue;
	int out1;
	UINT ret = (UINT) func(in1, in2, in3, &out1);
	if (ret == 0)
	{
		pResp->fields[1].vt = VT_I4;
		pResp->fields[1].iValue = out1;
	}
	return ret;
}

UINT MsiFunc_ISII_I(PMsiFunc_ISII_I func, const RemoteMsiSession::RequestData* pReq, RemoteMsiSession::RequestData* pResp)
{
	int in1 = pReq->fields[0].iValue;
	wchar_t* in2 = pReq->fields[1].szValue;
	int in3 = pReq->fields[2].iValue;
	int in4 = pReq->fields[3].iValue;
	int out1;
	UINT ret = (UINT) func(in1, in2, in3, in4, &out1);
	if (ret == 0)
	{
		pResp->fields[1].vt = VT_I4;
		pResp->fields[1].iValue = out1;
	}
	return ret;
}

UINT MsiFunc_IS_II(PMsiFunc_IS_II func, const RemoteMsiSession::RequestData* pReq, RemoteMsiSession::RequestData* pResp)
{
	int in1 = pReq->fields[0].iValue;
	wchar_t* in2 = pReq->fields[1].szValue;
	int out1, out2;
	UINT ret = (UINT) func(in1, in2, &out1, &out2);
	if (ret == 0)
	{
		pResp->fields[1].vt = VT_I4;
		pResp->fields[1].iValue = out1;
		pResp->fields[2].vt = VT_I4;
		pResp->fields[2].iValue = out2;
	}
	return ret;
}

UINT MsiFunc_I_S(PMsiFunc_I_S func, const RemoteMsiSession::RequestData* pReq, RemoteMsiSession::RequestData* pResp, __deref_inout_ecount(cchBuf) wchar_t*& szBuf, __inout DWORD& cchBuf)
{
	int in1 = pReq->fields[0].iValue;
	szBuf[0] = L'\0';
	DWORD cchValue = cchBuf;
	UINT ret = (UINT) func(in1, szBuf, &cchValue);
	if (ret == ERROR_MORE_DATA)
	{
		ret = EnsureBufSize(&szBuf, &cchBuf, ++cchValue);
		if (ret == 0)
		{
			ret = (UINT) func(in1, szBuf, &cchValue);
		}
	}
	if (ret == 0)
	{
		pResp->fields[1].vt = VT_LPWSTR;
		pResp->fields[1].szValue = szBuf;
	}
	return ret;
}

UINT MsiFunc_II_S(PMsiFunc_II_S func, const RemoteMsiSession::RequestData* pReq, RemoteMsiSession::RequestData* pResp, __deref_inout_ecount(cchBuf) wchar_t*& szBuf, __inout DWORD& cchBuf)
{
	int in1 = pReq->fields[0].iValue;
	int in2 = pReq->fields[1].iValue;
	szBuf[0] = L'\0';
	DWORD cchValue = cchBuf;
	UINT ret = (UINT) func(in1, in2, szBuf, &cchValue);
	if (ret == ERROR_MORE_DATA)
	{
		ret = EnsureBufSize(&szBuf, &cchBuf, ++cchValue);
		if (ret == 0)
		{
			ret = (UINT) func(in1, in2, szBuf, &cchValue);
		}
	}
	if (ret == 0)
	{
		pResp->fields[1].vt = VT_LPWSTR;
		pResp->fields[1].szValue = szBuf;
	}
	return ret;
}

UINT MsiFunc_IS_S(PMsiFunc_IS_S func, const RemoteMsiSession::RequestData* pReq, RemoteMsiSession::RequestData* pResp, __deref_inout_ecount(cchBuf) wchar_t*& szBuf, __inout DWORD& cchBuf)
{
	int in1 = pReq->fields[0].iValue;
	wchar_t* in2 = pReq->fields[1].szValue;
	szBuf[0] = L'\0';
	DWORD cchValue = cchBuf;
	UINT ret = (UINT) func(in1, in2, szBuf, &cchValue);
	if (ret == ERROR_MORE_DATA)
	{
		ret = EnsureBufSize(&szBuf, &cchBuf, ++cchValue);
		if (ret == 0)
		{
			ret = (UINT) func(in1, in2, szBuf, &cchValue);
		}
	}
	if (ret == 0)
	{
		pResp->fields[1].vt = VT_LPWSTR;
		pResp->fields[1].szValue = szBuf;
	}
	return ret;
}

UINT MsiFunc_ISII_SII(PMsiFunc_ISII_SII func, const RemoteMsiSession::RequestData* pReq, RemoteMsiSession::RequestData* pResp, __deref_inout_ecount(cchBuf) wchar_t*& szBuf, __inout DWORD& cchBuf)
{
	int in1 = pReq->fields[0].iValue;
	wchar_t* in2 = pReq->fields[1].szValue;
	int in3 = pReq->fields[2].iValue;
	int in4 = pReq->fields[3].iValue;
	szBuf[0] = L'\0';
	DWORD cchValue = cchBuf;
	int out2, out3;
	UINT ret = (UINT) func(in1, in2, in3, in4, szBuf, &cchValue, &out2, &out3);
	if (ret == ERROR_MORE_DATA)
	{
		ret = EnsureBufSize(&szBuf, &cchBuf, ++cchValue);
		if (ret == 0)
		{
			ret = (UINT) func(in1, in2, in3, in4, szBuf, &cchValue, &out2, &out3);
		}
	}
	if (ret == 0)
	{
		pResp->fields[1].vt = VT_LPWSTR;
		pResp->fields[1].szValue = szBuf;
		pResp->fields[2].vt = VT_I4;
		pResp->fields[2].iValue = out2;
		pResp->fields[3].vt = VT_I4;
		pResp->fields[3].iValue = out3;
	}
	return ret;
}

void RemoteMsiSession::ProcessRequest(RequestId id, const RequestData* pReq, RequestData* pResp)
{
	DWORD cchBuf1 = 1024;
	static wchar_t* szBuf1 = new wchar_t[cchBuf1];

    ZeroMemory(pResp, sizeof(RequestData));
	UINT ret = 0;

	switch (id)
    {
		case RemoteMsiSession::EndSession:
		{
			this->ExitCode = pReq->fields[0].iValue;
		}
		break;
		case RemoteMsiSession::MsiCloseHandle:
		{
			MSIHANDLE h = (MSIHANDLE) pReq->fields[0].iValue;
			ret = ::MsiCloseHandle(h);
		}
		break;
		case RemoteMsiSession::MsiProcessMessage:
		{
			MSIHANDLE hInstall = (MSIHANDLE) pReq->fields[0].iValue;
			INSTALLMESSAGE eMessageType = (INSTALLMESSAGE) pReq->fields[1].iValue;
			MSIHANDLE hRecord = (MSIHANDLE) pReq->fields[2].iValue;
			ret = ::MsiProcessMessage(hInstall, eMessageType, hRecord);
		}
		break;
		case RemoteMsiSession::MsiGetProperty:
		{
			ret = MsiFunc_IS_S((PMsiFunc_IS_S) ::MsiGetProperty, pReq, pResp, szBuf1, cchBuf1);
		}
		break;
		case RemoteMsiSession::MsiSetProperty:
		{
			MSIHANDLE hInstall = (MSIHANDLE) pReq->fields[0].iValue;
			const wchar_t* szName = pReq->fields[1].szValue;
			const wchar_t* szValue = pReq->fields[2].szValue;
			ret = ::MsiSetProperty(hInstall, szName, szValue);
		}
		break;
		case RemoteMsiSession::MsiCreateRecord:
		{
			UINT cParams = pReq->fields[0].uiValue;
			ret = ::MsiCreateRecord(cParams);
		}
		break;
		case RemoteMsiSession::MsiRecordGetFieldCount:
		{
			MSIHANDLE hRecord = (MSIHANDLE) pReq->fields[0].iValue;
			ret = ::MsiRecordGetFieldCount(hRecord);
		}
		break;
		case RemoteMsiSession::MsiRecordGetInteger:
		{
			MSIHANDLE hRecord = (MSIHANDLE) pReq->fields[0].iValue;
			UINT iField = pReq->fields[1].uiValue;
			ret = ::MsiRecordGetInteger(hRecord, iField);
		}
		break;
		case RemoteMsiSession::MsiRecordSetInteger:
		{
			MSIHANDLE hRecord = (MSIHANDLE) pReq->fields[0].iValue;
			UINT iField = pReq->fields[1].uiValue;
			int iValue = pReq->fields[2].iValue;
			ret = ::MsiRecordSetInteger(hRecord, iField, iValue);
		}
		break;
		case RemoteMsiSession::MsiRecordGetString:
		{
			ret = MsiFunc_II_S((PMsiFunc_II_S) ::MsiRecordGetString, pReq, pResp, szBuf1, cchBuf1);
		}
		break;
		case RemoteMsiSession::MsiRecordSetString:
		{
			MSIHANDLE hRecord = (MSIHANDLE) pReq->fields[0].iValue;
			UINT iField = pReq->fields[1].uiValue;
			const wchar_t* szValue = pReq->fields[2].szValue;
			ret = ::MsiRecordSetString(hRecord, iField, szValue);
		}
		break;
        case RemoteMsiSession::MsiRecordClearData:
		{
			MSIHANDLE hRecord = (MSIHANDLE) pReq->fields[0].iValue;
			ret = ::MsiRecordClearData(hRecord);
		}
		break;
		case RemoteMsiSession::MsiRecordIsNull:
		{
			MSIHANDLE hRecord = (MSIHANDLE) pReq->fields[0].iValue;
			UINT iField = pReq->fields[1].uiValue;
			ret = ::MsiRecordIsNull(hRecord, iField);
		}
		break;
		case RemoteMsiSession::MsiFormatRecord:
		{
			ret = MsiFunc_II_S((PMsiFunc_II_S) ::MsiFormatRecord, pReq, pResp, szBuf1, cchBuf1);
		}
		break;
		case RemoteMsiSession::MsiGetActiveDatabase:
		{
			MSIHANDLE hInstall = (MSIHANDLE) pReq->fields[0].iValue;
			ret = (UINT) ::MsiGetActiveDatabase(hInstall);
		}
		break;
		case RemoteMsiSession::MsiDatabaseOpenView:
		{
			ret = MsiFunc_IS_I((PMsiFunc_IS_I) ::MsiDatabaseOpenView, pReq, pResp);
		}
		break;
		case RemoteMsiSession::MsiViewExecute:
		{
			MSIHANDLE hView = (MSIHANDLE) pReq->fields[0].iValue;
			MSIHANDLE hRecord = (MSIHANDLE) pReq->fields[1].iValue;
			ret = ::MsiViewExecute(hView, hRecord);
		}
		break;
		case RemoteMsiSession::MsiViewFetch:
		{
			ret = MsiFunc_I_I((PMsiFunc_I_I) ::MsiViewFetch, pReq, pResp);
		}
		break;
		case RemoteMsiSession::MsiViewModify:
		{
			MSIHANDLE hView = (MSIHANDLE) pReq->fields[0].iValue;
			MSIMODIFY eModifyMode = (MSIMODIFY) pReq->fields[1].iValue;
			MSIHANDLE hRecord = (MSIHANDLE) pReq->fields[2].iValue;
			ret = ::MsiViewModify(hView, eModifyMode, hRecord);
		}
		break;
		case RemoteMsiSession::MsiViewGetError:
		{
			ret = MsiFunc_I_S((PMsiFunc_I_S) ::MsiViewGetError, pReq, pResp, szBuf1, cchBuf1);
		}
		break;
		case RemoteMsiSession::MsiViewGetColumnInfo:
		{
			ret = MsiFunc_II_I((PMsiFunc_II_I) ::MsiViewGetColumnInfo, pReq, pResp);
		}
		break;
		case RemoteMsiSession::MsiDatabaseGetPrimaryKeys:
		{
			ret = MsiFunc_IS_I((PMsiFunc_IS_I) ::MsiDatabaseGetPrimaryKeys, pReq, pResp);
		}
		break;
		case RemoteMsiSession::MsiDatabaseIsTablePersistent:
		{
			MSIHANDLE hDb = (MSIHANDLE) pReq->fields[0].iValue;
			const wchar_t* szTable = pReq->fields[1].szValue;
			ret = ::MsiDatabaseIsTablePersistent(hDb, szTable);
		}
		break;
		case RemoteMsiSession::MsiDoAction:
		{
			MSIHANDLE hInstall = (MSIHANDLE) pReq->fields[0].iValue;
			const wchar_t* szAction = pReq->fields[1].szValue;
			ret = ::MsiDoAction(hInstall, szAction);
		}
		break;
		case RemoteMsiSession::MsiEnumComponentCosts:
		{
			ret = MsiFunc_ISII_SII((PMsiFunc_ISII_SII) ::MsiEnumComponentCosts, pReq, pResp, szBuf1, cchBuf1);
		}
		break;
		case RemoteMsiSession::MsiEvaluateCondition:
		{
			MSIHANDLE hInstall = (MSIHANDLE) pReq->fields[0].iValue;
			const wchar_t* szCondition = pReq->fields[1].szValue;
			ret = ::MsiEvaluateCondition(hInstall, szCondition);
		}
		break;
		case RemoteMsiSession::MsiGetComponentState:
		{
			ret = MsiFunc_IS_II((PMsiFunc_IS_II) ::MsiGetComponentState, pReq, pResp);
		}
		break;
		case RemoteMsiSession::MsiGetFeatureCost:
		{
			ret = MsiFunc_ISII_I((PMsiFunc_ISII_I) ::MsiGetFeatureCost, pReq, pResp);
		}
		break;
		case RemoteMsiSession::MsiGetFeatureState:
		{
			ret = MsiFunc_IS_II((PMsiFunc_IS_II) ::MsiGetFeatureState, pReq, pResp);
		}
		break;
		case RemoteMsiSession::MsiGetFeatureValidStates:
		{
			ret = MsiFunc_IS_I((PMsiFunc_IS_I) ::MsiGetFeatureValidStates, pReq, pResp);
		}
		break;
		case RemoteMsiSession::MsiGetLanguage:
		{
			MSIHANDLE hInstall = (MSIHANDLE) pReq->fields[0].iValue;
			ret = ::MsiGetLanguage(hInstall);
		}
		break;
		case RemoteMsiSession::MsiGetLastErrorRecord:
		{
			ret = ::MsiGetLastErrorRecord();
		}
		break;
		case RemoteMsiSession::MsiGetMode:
		{
			MSIHANDLE hInstall = (MSIHANDLE) pReq->fields[0].iValue;
			MSIRUNMODE iRunMode = (MSIRUNMODE) pReq->fields[1].iValue;
			ret = ::MsiGetMode(hInstall, iRunMode);
		}
		break;
		case RemoteMsiSession::MsiGetSourcePath:
		{
			ret = MsiFunc_IS_S((PMsiFunc_IS_S) ::MsiGetSourcePath, pReq, pResp, szBuf1, cchBuf1);
		}
		break;
		case RemoteMsiSession::MsiGetSummaryInformation:
		{
			ret = MsiFunc_ISI_I((PMsiFunc_ISI_I) ::MsiGetSummaryInformation, pReq, pResp);
		}
		break;
		case RemoteMsiSession::MsiGetTargetPath:
		{
			ret = MsiFunc_IS_S((PMsiFunc_IS_S) ::MsiGetTargetPath, pReq, pResp, szBuf1, cchBuf1);
		}
		break;
		case RemoteMsiSession::MsiRecordDataSize:
		{
			MSIHANDLE hRecord = (MSIHANDLE) pReq->fields[0].iValue;
			UINT iField = pReq->fields[1].uiValue;
			ret = ::MsiRecordDataSize(hRecord, iField);
		}
		break;
		case RemoteMsiSession::MsiRecordReadStream:
		{
			MSIHANDLE hRecord = (MSIHANDLE) pReq->fields[0].iValue;
			UINT iField = pReq->fields[1].uiValue;
			DWORD cbRead = (DWORD) pReq->fields[2].uiValue;
			ret = EnsureBufSize(&szBuf1, &cchBuf1, (cbRead + 1) / 2);
			if (ret == 0)
			{
				ret = ::MsiRecordReadStream(hRecord, iField, (char*) szBuf1, &cbRead);
				if (ret == 0)
				{
					pResp->fields[1].vt = VT_STREAM;
					pResp->fields[1].szValue = szBuf1;
					pResp->fields[2].vt = VT_I4;
					pResp->fields[2].uiValue = (UINT) cbRead;
				}
			}
		}
		break;
		case RemoteMsiSession::MsiRecordSetStream:
		{
			MSIHANDLE hRecord = (MSIHANDLE) pReq->fields[0].iValue;
			UINT iField = pReq->fields[1].uiValue;
			const wchar_t* szFilePath = pReq->fields[2].szValue;
			ret = ::MsiRecordSetStream(hRecord, iField, szFilePath);
		}
		break;
		case RemoteMsiSession::MsiSequence:
		{
			MSIHANDLE hRecord = (MSIHANDLE) pReq->fields[0].iValue;
			const wchar_t* szTable = pReq->fields[1].szValue;
			UINT iSequenceMode = pReq->fields[2].uiValue;
			ret = ::MsiSequence(hRecord, szTable, iSequenceMode);
		}
		break;
		case RemoteMsiSession::MsiSetComponentState:
		{
			MSIHANDLE hInstall = (MSIHANDLE) pReq->fields[0].iValue;
			const wchar_t* szComponent = pReq->fields[1].szValue;
			INSTALLSTATE iState = (INSTALLSTATE) pReq->fields[2].iValue;
			ret = ::MsiSetComponentState(hInstall, szComponent, iState);
		}
		break;
		case RemoteMsiSession::MsiSetFeatureAttributes:
		{
			MSIHANDLE hInstall = (MSIHANDLE) pReq->fields[0].iValue;
			const wchar_t* szFeature = pReq->fields[1].szValue;
			DWORD dwAttrs = (DWORD) pReq->fields[2].uiValue;
			ret = ::MsiSetFeatureAttributes(hInstall, szFeature, dwAttrs);
		}
		break;
		case RemoteMsiSession::MsiSetFeatureState:
		{
			MSIHANDLE hInstall = (MSIHANDLE) pReq->fields[0].iValue;
			const wchar_t* szFeature = pReq->fields[1].szValue;
			INSTALLSTATE iState = (INSTALLSTATE) pReq->fields[2].iValue;
			ret = ::MsiSetFeatureState(hInstall, szFeature, iState);
		}
		break;
		case RemoteMsiSession::MsiSetInstallLevel:
		{
			MSIHANDLE hInstall = (MSIHANDLE) pReq->fields[0].iValue;
			int iInstallLevel = pReq->fields[1].iValue;
			ret = ::MsiSetInstallLevel(hInstall, iInstallLevel);
		}
		break;
		case RemoteMsiSession::MsiSetMode:
		{
			MSIHANDLE hInstall = (MSIHANDLE) pReq->fields[0].iValue;
			MSIRUNMODE iRunMode = (MSIRUNMODE) pReq->fields[1].uiValue;
			BOOL fState = (BOOL) pReq->fields[2].iValue;
			ret = ::MsiSetMode(hInstall, iRunMode, fState);
		}
		break;
		case RemoteMsiSession::MsiSetTargetPath:
		{
			MSIHANDLE hInstall = (MSIHANDLE) pReq->fields[0].iValue;
			const wchar_t* szFolder = pReq->fields[1].szValue;
			const wchar_t* szFolderPath = pReq->fields[2].szValue;
			ret = ::MsiSetTargetPath(hInstall, szFolder, szFolderPath);
		}
		break;
		case RemoteMsiSession::MsiSummaryInfoGetProperty:
		{
			MSIHANDLE hSummaryInfo = (MSIHANDLE) pReq->fields[0].iValue;
			UINT uiProperty = pReq->fields[1].uiValue;
			UINT uiDataType;
			int iValue;
			FILETIME ftValue;
			szBuf1[0] = L'\0';
			DWORD cchValue = cchBuf1;
			ret = ::MsiSummaryInfoGetProperty(hSummaryInfo, uiProperty, &uiDataType, &iValue, &ftValue, szBuf1, &cchValue);
			if (ret == ERROR_MORE_DATA)
			{
				ret = EnsureBufSize(&szBuf1, &cchBuf1, ++cchValue);
				if (ret == 0)
				{
					ret = ::MsiSummaryInfoGetProperty(hSummaryInfo, uiProperty, &uiDataType, &iValue, &ftValue, szBuf1, &cchValue);
				}
			}
			if (ret == 0)
			{
				pResp->fields[1].vt = VT_UI4;
				pResp->fields[1].uiValue = uiDataType;

				switch (uiDataType)
				{
				case VT_I2:
				case VT_I4:
					pResp->fields[2].vt = VT_I4;
					pResp->fields[2].iValue = iValue;
					break;
				case VT_FILETIME:
					pResp->fields[2].vt = VT_UI4;
					pResp->fields[2].iValue = ftValue.dwHighDateTime;
					pResp->fields[3].vt = VT_UI4;
					pResp->fields[3].iValue = ftValue.dwLowDateTime;
					break;
				case VT_LPSTR:
					pResp->fields[2].vt = VT_LPWSTR;
					pResp->fields[2].szValue = szBuf1;
					break;
				}
			}
		}
		break;
		case RemoteMsiSession::MsiVerifyDiskSpace:
		{
			MSIHANDLE hInstall = (MSIHANDLE) pReq->fields[0].iValue;
			ret = ::MsiVerifyDiskSpace(hInstall);
		}
		break;
		
		default:
		{
			ret = ERROR_INVALID_FUNCTION;
		}
		break;
    }

    pResp->fields[0].vt = VT_UI4;
    pResp->fields[0].uiValue = ret;
}
