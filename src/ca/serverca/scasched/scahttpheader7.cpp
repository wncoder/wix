//-------------------------------------------------------------------------------------------------
// <copyright file="scahttpheader7.cpp" company="Microsoft">
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
//    IIS HTTP Header functions for CustomActions
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"

LPCWSTR vcsHttpHeaderQuery7 = L"SELECT `Name`, `ParentType`, `ParentValue`, `Value`, `Attributes` FROM `IIsHttpHeader` ORDER BY `Sequence`";
enum eHttpHeaderQuery { hhqName = 1, hhqParentType, hhqParentValue, hhqValue, hhqAttributes};

static HRESULT AddHttpHeaderToList(
    __in SCA_HTTP_HEADER** ppshhList
    );


void ScaHttpHeaderFreeList7(
    __in SCA_HTTP_HEADER* pshhList
    )
{
    SCA_HTTP_HEADER* pshhDelete = pshhList;
    while (pshhList)
    {
        pshhDelete = pshhList;
        pshhList = pshhList->pshhNext;

        MemFree(pshhDelete);
    }
}


HRESULT ScaHttpHeaderRead7(
    __in SCA_HTTP_HEADER** ppshhList
    )
{
    Assert(ppshhList);

    HRESULT hr = S_OK;
    UINT er = 0;
    PMSIHANDLE hView, hRec;
    LPWSTR pwzData = NULL;
    SCA_HTTP_HEADER* pshh = NULL;

    // bail quickly if the IIsHttpHeader table isn't around
    if (S_OK != WcaTableExists(L"IIsHttpHeader"))
    {
        WcaLog(LOGMSG_VERBOSE, "Skipping ScaHttpHeaderRead7() - because IIsHttpHeader table not present.");
        ExitFunction1(hr = S_FALSE);
    }
    // loop through all the HTTP headers
    hr = WcaOpenExecuteView(vcsHttpHeaderQuery7, &hView);
    ExitOnFailure(hr, "Failed to open view on IIsHttpHeader table");

    while (S_OK == (hr = WcaFetchRecord(hView, &hRec)))
    {
        hr = AddHttpHeaderToList(ppshhList);
        ExitOnFailure(hr, "failed to add http header to list");

        pshh = *ppshhList;

        hr = WcaGetRecordInteger(hRec, hhqParentType, &(pshh->iParentType));
        ExitOnFailure(hr, "failed to get IIsHttpHeader.ParentType");

        hr = WcaGetRecordString(hRec, hhqParentValue, &pwzData);
        ExitOnFailure(hr, "Failed to get IIsHttpHeader.ParentValue");
        hr = ::StringCchCopyW(pshh->wzParentValue, countof(pshh->wzParentValue), pwzData);
        ExitOnFailure(hr, "Failed to copy IIsHttpHeader.ParentValue");

        hr = WcaGetRecordFormattedString(hRec, hhqName, &pwzData);
        ExitOnFailure(hr, "Failed to get IIsHttpHeader.Name");
        hr = ::StringCchCopyW(pshh->wzName, countof(pshh->wzName), pwzData);
        ExitOnFailure(hr, "Failed to copy IIsHttpHeader.Name");

        hr = WcaGetRecordFormattedString(hRec, hhqValue, &pwzData);
        ExitOnFailure(hr, "Failed to get IIsHttpHeader.Value");
        hr = ::StringCchCopyW(pshh->wzValue, countof(pshh->wzValue), pwzData);
        ExitOnFailure(hr, "Failed to copy IIsHttpHeader.Value");

        hr = WcaGetRecordInteger(hRec, hhqAttributes, &(pshh->iAttributes));
        ExitOnFailure(hr, "failed to get IIsHttpHeader.Attributes");
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


HRESULT ScaGetHttpHeader7(
    __in int iParentType,
    __in_z LPCWSTR wzParentValue,
    __in SCA_HTTP_HEADER** ppshhList,
    __out SCA_HTTP_HEADER** ppshhOut
    )
{
    HRESULT hr = S_OK;
    SCA_HTTP_HEADER* pshhAdd = NULL;
    SCA_HTTP_HEADER* pshhLast = NULL;

    *ppshhOut = NULL;

    if (!*ppshhList)
    {
        return hr;
    }

    SCA_HTTP_HEADER* pshh = *ppshhList;
    while (pshh)
    {
        if (iParentType == pshh->iParentType && 0 == wcscmp(wzParentValue, pshh->wzParentValue))
        {
            // Found a match, take this one out of the list and add it to the matched out list
            pshhAdd = pshh;

            if (pshhLast)
            {
                // If we're not at the beginning of the list tell the last node about it's new next (since we're taking away it's current next)
                pshhLast->pshhNext = pshhAdd->pshhNext;
            }
            else
            {
                // If we are at the beginning (no pshhLast) update the beginning (since we're taking it)
                *ppshhList = pshh->pshhNext;
            }
            pshh = pshh->pshhNext; // move on

            // Add the one we've removed to the beginning of the out list
            pshhAdd->pshhNext = *ppshhOut;
            *ppshhOut = pshhAdd;
        }
        else
        {
            pshhLast = pshh; // remember the last we that didn't match
            pshh = pshh->pshhNext; // move on
        }
    }

    return hr;
}


HRESULT ScaWriteHttpHeader7(
     __in_z LPCWSTR wzWebName,
     __in_z LPCWSTR wzRoot,
     SCA_HTTP_HEADER* pshhList
    )
{

    HRESULT hr = S_OK;
    DWORD cchData = 0;

    hr = ScaWriteConfigID(IIS_HTTP_HEADER_BEGIN);
    ExitOnFailure(hr, "Fail to write httpHeader begin ID");

    hr = ScaWriteConfigString(wzWebName);
    ExitOnFailure(hr, "Fail to write httpHeader Web Key");

    hr = ScaWriteConfigString(wzRoot);
    ExitOnFailure(hr, "Fail to write httpHeader Vdir key");

    // Loop through the HTTP headers
    for (SCA_HTTP_HEADER* pshh = pshhList; pshh; pshh = pshh->pshhNext)
    {
        hr = ScaWriteConfigID(IIS_HTTP_HEADER);
        ExitOnFailure(hr, "Fail to write httpHeader ID");

        hr = ScaWriteConfigString(pshh->wzName);
        ExitOnFailure(hr, "Fail to write httpHeader name");

        hr = ScaWriteConfigString(pshh->wzValue);
        ExitOnFailure(hr, "Fail to write httpHeader value");
    }

    hr = ScaWriteConfigID(IIS_HTTP_HEADER_END);
    ExitOnFailure(hr, "Fail to write httpHeader end ID");

LExit:
    return hr;
}


HRESULT ScaHttpHeaderCheckList7(
    __in SCA_HTTP_HEADER* pshhList
    )
{
    if (!pshhList)
    {
        return S_OK;
    }

    while (pshhList)
    {
        WcaLog(LOGMSG_STANDARD, "Http Header: %S for parent: %S not used!", pshhList->wzName, pshhList->wzParentValue);
        pshhList = pshhList->pshhNext;
    }

    return E_FAIL;
}


static HRESULT AddHttpHeaderToList(
    __in SCA_HTTP_HEADER** ppshhList
    )
{
    HRESULT hr = S_OK;

    SCA_HTTP_HEADER* pshh = static_cast<SCA_HTTP_HEADER*>(MemAlloc(sizeof(SCA_HTTP_HEADER), TRUE));
    ExitOnNull(pshh, hr, E_OUTOFMEMORY, "failed to allocate memory for new http header list element");

    pshh->pshhNext = *ppshhList;
    *ppshhList = pshh;

LExit:
    return hr;
}
