//-------------------------------------------------------------------------------------------------
// <copyright file="scafilter7.cpp" company="Microsoft">
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
//    IIS Filter functions for CustomActions
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"

// sql queries
LPCWSTR vcsFilterQuery7 = L"SELECT `Web_`, `Name`, `Component_`, `Path`, `Description`, `Flags`, `LoadOrder` FROM `IIsFilter` ORDER BY `Web_`";

enum eFilterQuery { fqWeb = 1, fqFilter, fqComponent , fqPath, fqDescription, fqFlags, fqLoadOrder, fqInstalled, fqAction };

// prototypes
static HRESULT AddFilterToList(
    __in SCA_FILTER** ppsfList
    );

static HRESULT WriteFilter(const SCA_FILTER* psf);


UINT __stdcall ScaFiltersRead7(
    __in SCA_WEB7* pswList,
    __inout SCA_FILTER** ppsfList
    )
{
    HRESULT hr = S_OK;
    UINT er = ERROR_SUCCESS;
    PMSIHANDLE hView, hRec;
    INSTALLSTATE isInstalled = INSTALLSTATE_UNKNOWN;
    INSTALLSTATE isAction = INSTALLSTATE_UNKNOWN;
    SCA_WEB7 *pswWeb;
    SCA_FILTER* psf;

    LPWSTR pwzData = NULL;

    // check for required table
    if (S_OK != WcaTableExists(L"IIsFilter"))
    {
        WcaLog(LOGMSG_VERBOSE, "Skipping ScaFiltersRead7() - because IIsFilter table not present");
        ExitFunction1(hr = S_FALSE);
    }

    // loop through all the filters
    hr = WcaOpenExecuteView(vcsFilterQuery7, &hView);
    ExitOnFailure(hr, "Failed to open view on IIsFilter table");
    while (S_OK == (hr = WcaFetchRecord(hView, &hRec)))
    {
        // Get the Component first.  If the component is not being modified during
        // this transaction, skip processing this whole record.
        // get the darwin information
        hr = WcaGetRecordString(hRec, fqComponent, &pwzData);
        ExitOnFailure(hr, "failed to get WebFilter.Component");

        er = ::MsiGetComponentStateW(WcaGetInstallHandle(), pwzData, &isInstalled, &isAction);
        hr = HRESULT_FROM_WIN32(er);
        ExitOnFailure(hr, "Failed to get WebFilter Component state");

        if (!WcaIsInstalling(isInstalled, isAction) &&
            !WcaIsReInstalling(isInstalled, isAction) &&
            !WcaIsUninstalling(isInstalled, isAction))
        {
            continue; // skip this record.
        }

        hr = AddFilterToList(ppsfList);
        ExitOnFailure(hr, "failed to add filter to list");

        psf = *ppsfList;

        psf->isInstalled = isInstalled;
        psf->isAction = isAction;
        hr = ::StringCchCopyW(psf->wzComponent, countof(psf->wzComponent), pwzData);
        ExitOnFailure1(hr, "failed to copy component name: %S", pwzData);


        hr = WcaGetRecordString(hRec, fqWeb, &pwzData);
        ExitOnFailure(hr, "Failed to get Web for VirtualDir");

        if (*pwzData)
        {
            hr = ScaWebsGetBase7(pswList, pwzData, &pswWeb);
            if (FAILED(hr) && WcaIsUninstalling(isInstalled, isAction))
            {
                // If we're uninstalling, don't bother finding the existing web, just leave the filter root empty
                hr = S_OK;
            }
            ExitOnFailure(hr, "Failed to get base of web for Filter");
#pragma prefast(suppress:26037, "Source string is null terminated - it is populated as target of ::StringCchCopyW")
            if (0 != wcslen(pswWeb->wzDescription))
            {
                hr = ::StringCchCopyW(psf->wzFilterRoot, countof(psf->wzFilterRoot), pswWeb->wzDescription);
                ExitOnFailure(hr, "Failed to set WebName for Filter");
            }
        }
        else
        {
            hr = ::StringCchCopyW(psf->wzFilterRoot, countof(psf->wzFilterRoot), L"/");
            ExitOnFailure(hr, "Failed to allocate global filter base string");
        }

        // filter Name key
        hr = WcaGetRecordString(hRec, fqFilter, &pwzData);
        ExitOnFailure(hr, "Failed to get Filter.Filter");
        hr = ::StringCchCopyW(psf->wzKey, countof(psf->wzKey), pwzData);
        ExitOnFailure(hr, "Failed to copy key string to filter object");

        // filter path
        hr = WcaGetRecordFormattedString(hRec, fqPath, &pwzData);
        ExitOnFailure(hr, "Failed to get Filter.Path");
        hr = ::StringCchCopyW(psf->wzPath, countof(psf->wzPath), pwzData);
        ExitOnFailure(hr, "Failed to copy path string to filter object");

        // filter description -- not supported in iis 7
        hr = WcaGetRecordFormattedString(hRec, fqDescription, &pwzData);
        ExitOnFailure(hr, "Failed to get Filter.Description");
        hr = ::StringCchCopyW(psf->wzDescription, countof(psf->wzDescription), pwzData);
        ExitOnFailure(hr, "Failed to copy description string to filter object");

        // filter flags
        //What are these
        hr = WcaGetRecordInteger(hRec, fqFlags, &psf->iFlags);
        ExitOnFailure(hr, "Failed to get Filter.Flags");

        // filter load order
        hr = WcaGetRecordInteger(hRec, fqLoadOrder, &psf->iLoadOrder);
        ExitOnFailure(hr, "Failed to get Filter.LoadOrder");
    }

    if (E_NOMOREITEMS == hr)
    {
        hr = S_OK;
    }
    ExitOnFailure(hr, "Failure while processing filters");

LExit:
    ReleaseStr(pwzData);

    return hr;
}


HRESULT ScaFiltersInstall7(
    __in SCA_FILTER* psfList
    )
{
    HRESULT hr = S_OK;
    SCA_FILTER* psf = psfList;

    if(!psf)
    {
        ExitFunction();
    }
    //write global filters
    hr = ScaWriteConfigID(IIS_FILTER_GLOBAL_BEGIN);
    ExitOnFailure(hr, "Failed to write filter begin ID");
    while (psf)
    {
        if (WcaIsInstalling(psf->isInstalled, psf->isAction))
        {
            if (0 == wcscmp(psf->wzFilterRoot, L"/"))
            {
                hr = WriteFilter(psf);
            }
        }
        psf = psf->psfNext;
    }
    hr = ScaWriteConfigID(IIS_FILTER_END);
    ExitOnFailure(hr, "Failed to write filter ID");

    psf = psfList;

    //Write Web Site Filters
    hr = ScaWriteConfigID(IIS_FILTER_BEGIN);
    ExitOnFailure(hr, "Failed to write filter begin ID");
    while (psf)
    {
        if (WcaIsInstalling(psf->isInstalled, psf->isAction))
        {
            if (0 != wcscmp(psf->wzFilterRoot, L"/"))
            {
                hr = WriteFilter(psf);
            }
        }
        psf = psf->psfNext;
    }
    hr = ScaWriteConfigID(IIS_FILTER_END);
    ExitOnFailure(hr, "Failed to write filter ID");

LExit:

    return hr;
}
static HRESULT WriteFilter(const SCA_FILTER* psf)
{
    HRESULT hr = S_OK;

    hr = ScaWriteConfigID(IIS_FILTER);
    ExitOnFailure(hr, "Failed to write filter begin ID");

    hr = ScaWriteConfigID(IIS_CREATE);
    ExitOnFailure(hr, "Failed to write filter create ID");

    //filter Name key
    hr = ScaWriteConfigString(psf->wzKey);
    ExitOnFailure1(hr, "Failed to write key name for filter '%S'", psf->wzKey);

    //web site name
    hr = ScaWriteConfigString(psf->wzFilterRoot);
    ExitOnFailure(hr, "Failed to write filter web root ");

    // filter path
    hr = ScaWriteConfigString(psf->wzPath);
    ExitOnFailure1(hr, "Failed to write Path for filter '%S'", psf->wzKey);

    //filter load order
    hr = ScaWriteConfigInteger(psf->iLoadOrder);
    ExitOnFailure1(hr, "Failed to write load order for filter '%S'", psf->wzKey);

LExit:
    return hr;
}


HRESULT ScaFiltersUninstall7(
    __in SCA_FILTER* psfList
    )
{
    HRESULT hr = S_OK;
    SCA_FILTER* psf = psfList;

    if(!psf)
    {
        ExitFunction1(hr = S_OK);
    }

    //Uninstall global filters
    hr = ScaWriteConfigID(IIS_FILTER_GLOBAL_BEGIN);
    ExitOnFailure(hr, "Failed to write filter begin ID");

    while (psf)
    {
        if (WcaIsUninstalling(psf->isInstalled, psf->isAction))
        {
            if (0 == wcscmp(psf->wzFilterRoot, L"/"))
            {
                hr = ScaWriteConfigID(IIS_FILTER);
                ExitOnFailure(hr, "Failed to write filter begin ID");

                hr = ScaWriteConfigID(IIS_DELETE);
                ExitOnFailure(hr, "Failed to write filter create ID");

                //filter Name key
                hr = ScaWriteConfigString(psf->wzKey);
                ExitOnFailure1(hr, "Failed to write key name for filter '%S'", psf->wzKey);

                //web site name
                hr = ScaWriteConfigString(psf->wzFilterRoot);
                ExitOnFailure(hr, "Failed to write filter web root ");

            }
        }
        psf = psf->psfNext;
    }

    hr = ScaWriteConfigID(IIS_FILTER_END);
    ExitOnFailure(hr, "Failed to write filter ID");

    psf = psfList;

    //Uninstall website filters
    hr = ScaWriteConfigID(IIS_FILTER_GLOBAL_BEGIN);
    ExitOnFailure(hr, "Failed to write filter begin ID");
    while (psf)
    {
        if (WcaIsUninstalling(psf->isInstalled, psf->isAction))
        {
            if (0 != wcscmp(psf->wzFilterRoot, L"/"))
            {
                hr = ScaWriteConfigID(IIS_FILTER);
                ExitOnFailure(hr, "Failed to write filter begin ID");

                hr = ScaWriteConfigID(IIS_DELETE);
                ExitOnFailure(hr, "Failed to write filter create ID");

                //filter Name key
                hr = ScaWriteConfigString(psf->wzKey);
                ExitOnFailure1(hr, "Failed to write key name for filter '%S'", psf->wzKey);

                //web site name
                hr = ScaWriteConfigString(psf->wzFilterRoot);
                ExitOnFailure(hr, "Failed to write filter web root ");
            }
        }
        psf = psf->psfNext;
    }
    hr = ScaWriteConfigID(IIS_FILTER_END);
    ExitOnFailure(hr, "Failed to write filter ID");

LExit:
    return hr;
}


void ScaFiltersFreeList7(
    __in SCA_FILTER* psfList
    )
{
    SCA_FILTER* psfDelete = psfList;
    while (psfList)
    {
        psfDelete = psfList;
        psfList = psfList->psfNext;

        MemFree(psfDelete);
    }
}


// private helper functions
static HRESULT AddFilterToList(
    __inout SCA_FILTER** ppsfList)
{
    HRESULT hr = S_OK;

    SCA_FILTER* psf = static_cast<SCA_FILTER*>(MemAlloc(sizeof(SCA_FILTER), TRUE));
    ExitOnNull(psf, hr, E_OUTOFMEMORY, "failed to add filter to filter list");

    psf->psfNext = *ppsfList;
    *ppsfList = psf;

LExit:
    return hr;
}


