//-------------------------------------------------------------------------------------------------
// <copyright file="scaweberr7.cpp" company="Microsoft">
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

#include "precomp.h"
LPCWSTR vcsWebErrorQuery7 =
    L"SELECT `ErrorCode`, `SubCode`, `ParentType`, `ParentValue`, `File`, `URL` "
    L"FROM `IIsWebError` ORDER BY `ErrorCode`, `SubCode`";

enum eWebErrorQuery { weqErrorCode = 1, weqSubCode, weqParentType, weqParentValue, weqFile, weqURL };

static HRESULT AddWebErrorToList(SCA_WEB_ERROR** ppsweList);

void ScaWebErrorFreeList7(SCA_WEB_ERROR *psweList)
{
    SCA_WEB_ERROR *psweDelete = psweList;
    while (psweList)
    {
        psweDelete = psweList;
        psweList = psweList->psweNext;

        MemFree(psweDelete);
    }
}

HRESULT ScaWebErrorRead7(
    SCA_WEB_ERROR **ppsweList
    )
{
    HRESULT hr = S_OK;
    UINT er = 0;
    PMSIHANDLE hView, hRec;
    LPWSTR pwzData = NULL;
    SCA_WEB_ERROR* pswe;

    ExitOnNull(ppsweList, hr, E_INVALIDARG, "Failed to read web error, because no web error was provided to read");

    // bail quickly if the IIsWebError table isn't around
    if (S_OK != WcaTableExists(L"IIsWebError"))
    {
        WcaLog(LOGMSG_VERBOSE, "Skipping ScaWebErrorRead7() - because IIsWebError table not present.");
        ExitFunction1(hr = S_FALSE);
    }

    // loop through all web errors
    hr = WcaOpenExecuteView(vcsWebErrorQuery7, &hView);
    ExitOnFailure(hr, "Failed to open view on IIsWebError table");
    while (S_OK == (hr = WcaFetchRecord(hView, &hRec)))
    {
        hr = AddWebErrorToList(ppsweList);
        ExitOnFailure(hr, "failed to add web error to list");

        pswe = *ppsweList;

        hr = WcaGetRecordInteger(hRec, weqErrorCode, &(pswe->iErrorCode));
        ExitOnFailure(hr, "failed to get IIsWebError.ErrorCode");

        if((pswe->iErrorCode < 400) || (pswe->iErrorCode > 999))
        {
            ExitOnFailure(hr = HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER), "Error: IIsWebError.ErrorCode out of range (400-999)");
        }

        hr = WcaGetRecordInteger(hRec, weqSubCode, &(pswe->iSubCode));
        ExitOnFailure(hr, "failed to get IIsWebError.SubCode");
        if((pswe->iSubCode < -1) || (pswe->iSubCode > 999))
        {
            ExitOnFailure(hr = HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER), "Error: IIsWebError.ErrorSubCode out of range (-1-999)");
        }
        hr = WcaGetRecordInteger(hRec, weqParentType, &(pswe->iParentType));
        ExitOnFailure(hr, "failed to get IIsWebError.ParentType");

        hr = WcaGetRecordString(hRec, weqParentValue, &pwzData);
        ExitOnFailure(hr, "Failed to get IIsWebError.ParentValue");
        hr = ::StringCchCopyW(pswe->wzParentValue, countof(pswe->wzParentValue), pwzData);
        ExitOnFailure(hr, "Failed to copy IIsWebError.ParentValue");

        hr = WcaGetRecordFormattedString(hRec, weqFile, &pwzData);
        ExitOnFailure(hr, "Failed to get IIsWebError.File");
        hr = ::StringCchCopyW(pswe->wzFile, countof(pswe->wzFile), pwzData);
        ExitOnFailure(hr, "Failed to copy IIsWebError.File");

        hr = WcaGetRecordFormattedString(hRec, weqURL, &pwzData);
        ExitOnFailure(hr, "Failed to get IIsWebError.URL");
        hr = ::StringCchCopyW(pswe->wzURL, countof(pswe->wzURL), pwzData);
        ExitOnFailure(hr, "Failed to copy IIsWebError.URL");

        // If they've specified both a file and a URL, that's invalid
        if (*(pswe->wzFile) && *(pswe->wzURL))
        {
            ExitOnFailure2(hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA), "Both File and URL specified for web error.  File: %S, URL: %S", pswe->wzFile, pswe->wzURL);
        }
    }

    if (E_NOMOREITEMS == hr)
    {
        hr = S_OK;
    }
    ExitOnFailure(hr, "Failure while processing web errors");

LExit:
    ReleaseStr(pwzData);

    return hr;
}

HRESULT ScaGetWebError7(int iParentType, LPCWSTR wzParentValue, SCA_WEB_ERROR **ppsweList, SCA_WEB_ERROR **ppsweOut)
{
    HRESULT hr = S_OK;
    SCA_WEB_ERROR* psweAdd = NULL;
    SCA_WEB_ERROR* psweLast = NULL;

    *ppsweOut = NULL;

    if (!*ppsweList)
    {
        return hr;
    }

    SCA_WEB_ERROR* pswe = *ppsweList;
    while (pswe)
    {
        if (iParentType == pswe->iParentType && 0 == wcscmp(wzParentValue, pswe->wzParentValue))
        {
            // Found a match, take this one out of the list and add it to the matched out list
            psweAdd = pswe;

            if (psweLast)
            {
                // If we're not at the beginning of the list tell the last node about it's new next (since we're taking away it's current next)
                psweLast->psweNext = psweAdd->psweNext;
            }
            else
            {
                // If we are at the beginning (no psweLast) update the beginning (since we're taking it)
                *ppsweList = pswe->psweNext;
            }
            pswe = pswe->psweNext; // move on

            // Add the one we've removed to the beginning of the out list
            psweAdd->psweNext = *ppsweOut;
            *ppsweOut = psweAdd;
        }
        else
        {
            psweLast = pswe; // remember the last we that didn't match
            pswe = pswe->psweNext; // move on
        }
    }

    return hr;
}

HRESULT ScaWriteWebError7(
    __in_z LPCWSTR wzWebName,
    __in_z LPCWSTR wzRoot,
    SCA_WEB_ERROR* psweList
    )
{
    HRESULT hr = S_OK;

    hr = ScaWriteConfigID(IIS_WEBERROR_BEGIN);
    ExitOnFailure(hr, "Fail to write webError begin ID");

    hr = ScaWriteConfigString(wzWebName);
    ExitOnFailure(hr, "Fail to write webError Web Key");

    hr = ScaWriteConfigString(wzRoot);
    ExitOnFailure(hr, "Fail to write webError Vdir key");

    // Loop through the HTTP headers
    for (SCA_WEB_ERROR* pswe = psweList; pswe; pswe = pswe->psweNext)
    {
        hr = ScaWriteConfigID(IIS_WEBERROR);
        ExitOnFailure(hr, "Fail to write webError ID");

        hr = ScaWriteConfigInteger(pswe->iErrorCode);
        ExitOnFailure(hr, "Fail to write webError code");

        hr = ScaWriteConfigInteger(pswe->iSubCode);
        ExitOnFailure(hr, "Fail to write webError subcode");

        //just write one
        if (*(pswe->wzFile))
        {
            hr = ScaWriteConfigString(pswe->wzFile);
            ExitOnFailure(hr, "Fail to write webError file");
            hr = ScaWriteConfigInteger(0);
            ExitOnFailure(hr, "Fail to write webError file code");
        }
        else if (*(pswe->wzURL))
        {
            hr = ScaWriteConfigString(pswe->wzURL);
            ExitOnFailure(hr, "Fail to write webError URL");
            hr = ScaWriteConfigInteger(1);
            ExitOnFailure(hr, "Fail to write webError URL code");
        }
    }

    hr = ScaWriteConfigID(IIS_WEBERROR_END);
    ExitOnFailure(hr, "Fail to write httpHeader end ID");

LExit:
    return hr;

}

static HRESULT AddWebErrorToList(SCA_WEB_ERROR** ppsweList)
{
    HRESULT hr = S_OK;

    SCA_WEB_ERROR* pswe = static_cast<SCA_WEB_ERROR*>(MemAlloc(sizeof(SCA_WEB_ERROR), TRUE));
    ExitOnNull(pswe, hr, E_OUTOFMEMORY, "failed to allocate memory for new web error list element");

    pswe->psweNext = *ppsweList;
    *ppsweList = pswe;

LExit:
    return hr;
}

HRESULT ScaWebErrorCheckList7(SCA_WEB_ERROR* psweList)
{
    if (!psweList)
    {
        return S_OK;
    }

    while (psweList)
    {
        WcaLog(LOGMSG_STANDARD, "WebError code: %d subcode: %d for parent: %S not used!", psweList->iErrorCode, psweList->iSubCode, psweList->wzParentValue);
        psweList = psweList->psweNext;
    }

    return E_FAIL;
}

