//-------------------------------------------------------------------------------------------------
// <copyright file="scaweblog7.cpp" company="Microsoft">
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
//    Custom Actions for handling log settings for a particular IIS Website
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"
// sql queries
LPCWSTR vcsWebLogQuery7 = L"SELECT `Log`, `Format` "
                         L"FROM `IIsWebLog`  WHERE `Log`=?";

enum eWebLogQuery { wlqLog = 1, wlqFormat };

/* ****************************************************************
 * ScaGetWebLog7 -Retrieves Log table data for the specified Log key
 *
 * ****************************************************************/
HRESULT ScaGetWebLog7(
    __in_z LPCWSTR wzLog,
    __out SCA_WEB_LOG* pswl
    )
{
    HRESULT hr = S_OK;
    LPWSTR pwzData = NULL;
    PMSIHANDLE hView, hRec;

    // check to see what tables are available
    if (S_OK != WcaTableExists(L"IIsWebLog"))
    {
        WcaLog(LOGMSG_VERBOSE, "Skipping ScaGetWebLog7() - because IIsWebLog table not present.");
        ExitFunction1(hr = S_FALSE);
    }

    hRec = ::MsiCreateRecord(1);
    hr = WcaSetRecordString(hRec, 1, wzLog);
    ExitOnFailure(hr, "failed to look up Log data");

    hr = WcaOpenView(vcsWebLogQuery7, &hView);
    ExitOnFailure(hr, "failed to open view on IIsWebLog");
    hr = WcaExecuteView(hView, hRec);
    ExitOnFailure(hr, "failed to exectue view on IIsWebLog");

    hr = WcaFetchSingleRecord(hView, &hRec);
    if (E_NOMOREITEMS == hr)
    {
        ExitOnFailure1(hr, "cannot locate IIsWebLog.Log='%S'", wzLog);
    }
    else if (S_OK != hr)
    {
        ExitOnFailure(hr, "error or found multiple matching IIsWebLog rows");
    }

    ::ZeroMemory(pswl, sizeof(SCA_WEB_LOG));

    // check that log key matches
    hr = WcaGetRecordString(hRec, wlqLog, &pwzData);
    ExitOnFailure1(hr, "failed to get IIsWebLog.Log for Log: %S", wzLog);
    hr = ::StringCchCopyW(pswl->wzLog, countof(pswl->wzLog), pwzData);
    ExitOnFailure1(hr, "failed to copy log name: %S", pwzData);

    hr = WcaGetRecordFormattedString(hRec, wlqFormat, &pwzData);
    ExitOnFailure1(hr, "failed to get IIsWebLog.Format for Log:", wzLog);

    //translate WIX log format name strings to IIS7
    if (0 == lstrcmpW(pwzData, L"Microsoft IIS Log File Format"))
    {
        hr = ::StringCchCopyW(pswl->wzFormat, countof(pswl->wzFormat), L"IIS");
        ExitOnFailure1(hr, "failed to copy log format: %S", pwzData);
    }
    else if (0 == lstrcmpW(pwzData, L"NCSA Common Log File Format"))
    {
        hr = ::StringCchCopyW(pswl->wzFormat, countof(pswl->wzFormat), L"NCSA");
        ExitOnFailure1(hr, "failed to copy log format: %S", pwzData);
    }
    else if (0 == lstrcmpW(pwzData, L"none"))
    {
        hr = ::StringCchCopyW(pswl->wzFormat, countof(pswl->wzFormat), L"none");
        ExitOnFailure1(hr, "failed to copy log format: %S", pwzData);
    }
    else if (0 == lstrcmpW(pwzData, L"ODBC Logging"))
    {
        hr = ::StringCchCopyW(pswl->wzFormat, countof(pswl->wzFormat), L"W3C");
        ExitOnFailure1(hr, "failed to copy log format: %S", pwzData);
    }
    else if (0 == lstrcmpW(pwzData, L"W3C Extended Log File Format"))
    {
        hr = ::StringCchCopyW(pswl->wzFormat, countof(pswl->wzFormat), L"W3C");
        ExitOnFailure1(hr, "failed to copy log format: %S", pwzData);
    }
    else
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_INDEX);
        ExitOnFailure1(hr, "Invalid log file format: %S", pwzData);
    }

LExit:
    ReleaseStr(pwzData);

    return hr;
}


/* ****************************************************************
 * ScaWriteWebLog -Writes the IIS log values to the metabase.
 *
 * ****************************************************************/
HRESULT ScaWriteWebLog7(
    LPCWSTR wzWebBase,
    const SCA_WEB_LOG *pswl
    )
{
    HRESULT hr = S_OK;

    if (*pswl->wzFormat)
    {
        //write pswl->wzFormat
        hr = ScaWriteConfigID(IIS_WEBLOG);
        ExitOnFailure(hr, "Failed to write log format id");
        hr = ScaWriteConfigString(wzWebBase);
        ExitOnFailure(hr, "Failed to write log web key");
        hr = ScaWriteConfigString(pswl->wzFormat);
        ExitOnFailure(hr, "Failed to write log format string");
    }

LExit:
    return hr;
}


