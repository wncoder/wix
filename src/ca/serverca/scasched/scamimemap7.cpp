//-------------------------------------------------------------------------------------------------
// <copyright file="scamimemap7.cpp" company="Microsoft">
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
//    IIS Mime Map functions for CustomActions
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"
LPCWSTR vcsMimeMapQuery7 = L"SELECT `MimeMap`, `ParentType`, `ParentValue`, `MimeType`, `Extension` "
                         L"FROM `IIsMimeMap`";

enum eMimeMapQuery { mmqMimeMap = 1, mmqParentType, mmqParentValue,
                        mmqMimeType, mmqExtension};

// prototypes
static HRESULT AddMimeMapToList(SCA_MIMEMAP** ppsmmList);


void ScaMimeMapFreeList7(SCA_MIMEMAP* psmmList)
{
    SCA_MIMEMAP* psmmDelete = psmmList;
    while (psmmList)
    {
        psmmDelete = psmmList;
        psmmList = psmmList->psmmNext;

        MemFree(psmmDelete);
    }
}


HRESULT __stdcall ScaMimeMapRead7(
    SCA_MIMEMAP** ppsmmList
    )
{
    HRESULT hr = S_OK;
    PMSIHANDLE hView, hRec;
    BOOL fIIsWebMimeMapTable = FALSE;
    LPWSTR pwzData = NULL;
    SCA_MIMEMAP* psmm;

    // check to see what tables are available
    fIIsWebMimeMapTable = (S_OK == WcaTableExists(L"IIsMimeMap"));

    if (!fIIsWebMimeMapTable)
    {
        WcaLog(LOGMSG_VERBOSE, "Skipping ScaMimeMapRead7() - because IIsMimeMap table not present.");
        ExitFunction1(hr = S_FALSE);
    }

    hr = WcaOpenExecuteView(vcsMimeMapQuery7, &hView);
    ExitOnFailure(hr, "Failed to open view on IIsMimeMap table");

    // loop through all the mimemappings
    while (S_OK == (hr = WcaFetchRecord(hView, &hRec)))
    {
        hr = AddMimeMapToList(ppsmmList);
        ExitOnFailure(hr, "failed to add mime map to list");

        psmm = *ppsmmList;

        hr = WcaGetRecordString(hRec, mmqMimeMap, &pwzData);
        ExitOnFailure(hr, "Failed to get MimeMap.MimeMap");
        hr = ::StringCchCopyW(psmm->wzMimeMap, countof(psmm->wzMimeMap), pwzData);
        ExitOnFailure(hr, "Failed to copy mimemap string to mimemap object");

        hr = WcaGetRecordInteger(hRec, mmqParentType, &psmm->iParentType);
        ExitOnFailure(hr, "Failed to get MimeMap.iParentType");

        hr = WcaGetRecordString(hRec, mmqParentValue, &pwzData);
        ExitOnFailure(hr, "Failed to get MimeMap.ParentValue");
        hr = ::StringCchCopyW(psmm->wzParentValue, countof(psmm->wzParentValue), pwzData);
        ExitOnFailure(hr, "Failed to copy parent value string to mimemap object");

        hr = WcaGetRecordFormattedString(hRec, mmqExtension, &pwzData);
        ExitOnFailure(hr, "Failed to get MimeMap.Extension");
        hr = ::StringCchCopyW(psmm->wzExtension, countof(psmm->wzExtension), pwzData);
        ExitOnFailure(hr, "Failed to copy extension string to mimemap object");

        hr = WcaGetRecordFormattedString(hRec, mmqMimeType, &pwzData);
        ExitOnFailure(hr, "Failed to get MimeMap.MimeType");
        hr = ::StringCchCopyW(psmm->wzMimeType, countof(psmm->wzMimeType), pwzData);
        ExitOnFailure(hr, "Failed to copy mimetype string to mimemap object");
    }

    if (E_NOMOREITEMS == hr)
    {
        hr = S_OK;
    }
    ExitOnFailure(hr, "Failure while processing mimemappings");

LExit:
    ReleaseStr(pwzData);

    return hr;
}


HRESULT ScaGetMimeMap7(int iParentType, __in_z LPCWSTR wzParentValue, SCA_MIMEMAP **ppsmmList, SCA_MIMEMAP **ppsmmOut)
{
    HRESULT hr = S_OK;
    SCA_MIMEMAP* psmmAdd = NULL;
    SCA_MIMEMAP* psmmLast = NULL;

    *ppsmmOut = NULL;

    if (!*ppsmmList)
    {
        return hr;
    }

    SCA_MIMEMAP* psmm = *ppsmmList;
    while (psmm)
    {
        if (iParentType == psmm->iParentType && 0 == wcscmp(wzParentValue, psmm->wzParentValue))
        {
            // Found a match, take this one out of the list and add it to the matched out list
            psmmAdd = psmm;

            if (psmmLast)
            {
                // If we're not at the beginning of the list tell the last node about it's new next (since we're taking away it's current next)
                psmmLast->psmmNext = psmmAdd->psmmNext;
            }
            else
            {
                // If we are at the beginning (no psmmLast) update the beginning (since we're taking it)
                *ppsmmList = psmm->psmmNext;
            }
            psmm = psmm->psmmNext; // move on

            // Add the one we've removed to the beginning of the out list
            psmmAdd->psmmNext = *ppsmmOut;
            *ppsmmOut = psmmAdd;
        }
        else
        {
            psmmLast = psmm; // remember the last we that didn't match
            psmm = psmm->psmmNext; // move on
        }
    }

    return hr;
}

HRESULT ScaWriteMimeMap7(
    __in_z LPCWSTR wzWebName,
    __in_z LPCWSTR wzRootOfWeb,
    SCA_MIMEMAP* psmmList
    )
{
    HRESULT hr = S_OK;
    SCA_MIMEMAP* psmm;

    //create the mimemap list for this vdir application
    //all go to same web/root location tag
    hr = ScaWriteConfigID(IIS_MIMEMAP_BEGIN);
    ExitOnFailure(hr, "Failed to write mimemap begin id");
    hr = ScaWriteConfigString(wzWebName);                //site name key
    ExitOnFailure(hr, "Failed to write mimemap web key");
    hr = ScaWriteConfigString(wzRootOfWeb);               //app path key
    ExitOnFailure(hr, "Failed to write mimemap app key");

    psmm = psmmList;

    while (psmm)
    {
        //create the Extension for this vdir application
        hr = ScaWriteConfigID(IIS_MIMEMAP);
        ExitOnFailure(hr, "Failed to write mimemap id");

        if (*psmm->wzExtension)
        {
            hr = ScaWriteConfigString(psmm->wzExtension);
        }
        else   // blank means "*" (all)
        {
            hr = ScaWriteConfigString(L"*");
        }
        ExitOnFailure(hr, "Failed to write mimemap extension");

        hr = ScaWriteConfigString(psmm->wzMimeType);
        ExitOnFailure(hr, "Failed to write mimemap type");

        psmm = psmm->psmmNext;
    }

    hr = ScaWriteConfigID(IIS_MIMEMAP_END);
    ExitOnFailure(hr, "Failed to write mimemap end id");

LExit:
    return hr;
}


static HRESULT AddMimeMapToList(SCA_MIMEMAP** ppsmmList)
{
    HRESULT hr = S_OK;

    SCA_MIMEMAP* psmm = static_cast<SCA_MIMEMAP*>(MemAlloc(sizeof(SCA_MIMEMAP), TRUE));
    ExitOnNull(psmm, hr, E_OUTOFMEMORY, "failed to allocate memory for new mime map list element");

    psmm->psmmNext = *ppsmmList;
    *ppsmmList = psmm;

LExit:
    return hr;
}
