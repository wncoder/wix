//-------------------------------------------------------------------------------------------------
// <copyright file="scawebdir7.cpp" company="Microsoft">
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
//    IIS Web Directory functions for CustomActions
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"

// sql queries
static LPCWSTR vcsWebDirQuery7 = L"SELECT `Web_`, `WebDir`, `Component_`, `Path`, `DirProperties_`, `Application_`"
                                       L"FROM `IIsWebDir`";

enum eWebDirQuery { wdqWeb = 1, wdqWebDir, wdqComponent , wdqPath, wdqProperties, wdqApplication, wdqInstalled, wdqAction };

// prototypes
static HRESULT AddWebDirToList(SCA_WEBDIR7** ppswdList);


UINT __stdcall ScaWebDirsRead7(
    SCA_WEB7* pswList,
    SCA_WEBDIR7** ppswdList
    )
{
    HRESULT hr = S_OK;
    UINT er = ERROR_SUCCESS;
    PMSIHANDLE hView, hRec;

    LPWSTR pwzData = NULL;
    SCA_WEBDIR7* pswd;
    SCA_WEB7 *pswWeb;

    // check to see if necessary tables are specified
    if (S_OK != WcaTableExists(L"IIsWebDir"))
    {
        WcaLog(LOGMSG_VERBOSE, "Skipping ScaWebDirsRead7() - because IIsWebDir table not present");
        ExitFunction1(hr = S_FALSE);
    }

    // loop through all the web directories
    hr = WcaOpenExecuteView(vcsWebDirQuery7, &hView);
    ExitOnFailure(hr, "Failed to open view on IIsWebDir table");
    while (S_OK == (hr = WcaFetchRecord(hView, &hRec)))
    {
        hr = AddWebDirToList(ppswdList);
        ExitOnFailure(hr, "failed to add web dir to list");

        pswd = *ppswdList;
        ExitOnNull(pswd, hr, E_INVALIDARG, "No web dir provided");

        // get component install state
        hr = WcaGetRecordString(hRec, wdqComponent, &pwzData);
        ExitOnFailure(hr, "Failed to get Component for WebDirs");
        hr = ::StringCchCopyW(pswd->wzComponent, countof(pswd->wzComponent), pwzData);
        ExitOnFailure(hr, "Failed to copy component string to webdir object");

        er = ::MsiGetComponentStateW(WcaGetInstallHandle(), pwzData, &pswd->isInstalled, &pswd->isAction);
        hr = HRESULT_FROM_WIN32(er);
        ExitOnFailure(hr, "Failed to get Component state for WebDirs");

        hr = WcaGetRecordString(hRec, wdqWeb, &pwzData);
        ExitOnFailure(hr, "Failed to get Web for WebDir");

        // get the web key
        hr = ScaWebsGetBase7(pswList, pwzData, &pswWeb);
        if(S_FALSE == hr)
        {
            hr = HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
            ExitOnFailure(hr, "Failed to get base of web for WebDir");
        }
#pragma prefast(suppress:26037, "Source string is null terminated - it is populated as target of ::StringCchCopyW")
        hr = ::StringCchCopyW(pswd->wzWebSite, countof(pswd->wzWebSite), pswWeb->wzDescription);
        ExitOnFailure(hr, "Failed to format webdir root string");

        hr = WcaGetRecordString(hRec, wdqPath, &pwzData);
        ExitOnFailure(hr, "Failed to get Path for WebDir");

        hr = ::StringCchCopyW(pswd->wzPath, countof(pswd->wzPath), pwzData);
        ExitOnFailure(hr, "Failed to copy path for WebDir");

        // get the directory properties for this web
        hr = WcaGetRecordString(hRec, wdqProperties, &pwzData);
        ExitOnFailure(hr, "Failed to get security identifier for WebDir");
        if (*pwzData)
        {
            hr = ScaGetWebDirProperties7(pwzData, &pswd->swp);
            ExitOnFailure(hr, "Failed to get properties for WebDir");

            pswd->fHasProperties = TRUE;
        }

        // get the application information for this web directory
        hr = WcaGetRecordString(hRec, wdqApplication, &pwzData);
        ExitOnFailure(hr, "Failed to get application identifier for WebDir");
        if (*pwzData)
        {
            hr = ScaGetWebApplication7(NULL, pwzData, &pswd->swapp);
            ExitOnFailure(hr, "Failed to get application for WebDir");

            pswd->fHasApplication = TRUE;
        }
    }

    if (E_NOMOREITEMS == hr)
    {
        hr = S_OK;
    }
    ExitOnFailure(hr, "Failure while processing WebDirs");

LExit:
    ReleaseStr(pwzData);

    return hr;
}


HRESULT ScaWebDirsInstall7(SCA_WEBDIR7* pswdList, SCA_APPPOOL * psapList)
{
    HRESULT hr = S_OK;
    SCA_WEBDIR7* pswd = pswdList;

    while (pswd)
    {
        // if we are installing the web site
        if (WcaIsInstalling(pswd->isInstalled, pswd->isAction))
        {
            hr = ScaWriteConfigID(IIS_WEBDIR);
            ExitOnFailure(hr, "Failed to write WebDir ID");

            hr = ScaWriteConfigID(IIS_CREATE);
            ExitOnFailure(hr, "Failed to write WebDir action ID");

            hr = ScaWriteConfigString(pswd->wzWebSite);
            ExitOnFailure(hr, "Failed to write WebDir site");

            hr = ScaWriteConfigString(pswd->wzPath);
            ExitOnFailure(hr, "Failed to write WebDir path");

            // get the security information for this web
            if (pswd->fHasProperties)
            {
                ScaWriteWebDirProperties7(pswd->wzWebSite, pswd->wzPath, &pswd->swp);
                ExitOnFailure(hr, "Failed to write properties for WebDir");
            }

            // get the application information for this web directory
            if (pswd->fHasApplication)
            {
                hr = ScaWriteWebApplication7(pswd->wzWebSite, pswd->wzPath, &pswd->swapp, psapList);
                ExitOnFailure(hr, "Failed to write application for WebDir");
            }
        }

        pswd = pswd->pswdNext;
    }

LExit:
    return hr;
}


HRESULT ScaWebDirsUninstall7(SCA_WEBDIR7* pswdList)
{
    HRESULT hr = S_OK;
    SCA_WEBDIR7* pswd = pswdList;

    while (pswd)
    {
        if (WcaIsUninstalling(pswd->isInstalled, pswd->isAction))
        {
            hr = ScaWriteConfigID(IIS_WEBDIR);
            ExitOnFailure(hr, "Failed to write WebDir ID");

            hr = ScaWriteConfigID(IIS_DELETE);
            ExitOnFailure(hr, "Failed to write WebDir action ID");

            hr = ScaWriteConfigString(pswd->wzWebSite);
            ExitOnFailure(hr, "Failed to write WebDir site");

            hr = ScaWriteConfigString(pswd->wzPath);
            ExitOnFailure(hr, "Failed to write WebDir path");
        }

        pswd = pswd->pswdNext;
    }

LExit:
    return hr;
}


void ScaWebDirsFreeList7(SCA_WEBDIR7* pswdList)
{
    SCA_WEBDIR7* pswdDelete = pswdList;
    while (pswdList)
    {
        pswdDelete = pswdList;
        pswdList = pswdList->pswdNext;

        MemFree(pswdDelete);
    }
}


static HRESULT AddWebDirToList(SCA_WEBDIR7** ppswdList)
{
    HRESULT hr = S_OK;

    SCA_WEBDIR7* pswd = static_cast<SCA_WEBDIR7*>(MemAlloc(sizeof(SCA_WEBDIR7), TRUE));
    ExitOnNull(pswd, hr, E_OUTOFMEMORY, "failed to allocate element for web dir list");

    pswd->pswdNext = *ppswdList;
    *ppswdList = pswd;

LExit:
    return hr;
}
