//-------------------------------------------------------------------------------------------------
// <copyright file="scawebdir.h" company="Microsoft">
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
enum eWebDirQuery { wdqWeb = 1, wdqWebDir, wdqComponent , wdqPath, wdqProperties, wdqApplication, wdqInstalled, wdqAction };

// prototypes
HRESULT AddWebDirToList(SCA_WEBDIR** ppswdList);


UINT __stdcall ScaWebDirsRead(
    __in IMSAdminBase* piMetabase,
    __in SCA_WEB* pswList,
    __in WCA_WRAPQUERY_HANDLE hUserQuery,
    __in WCA_WRAPQUERY_HANDLE hWebBaseQuery,
    __in WCA_WRAPQUERY_HANDLE hWebDirPropQuery,
    __in WCA_WRAPQUERY_HANDLE hWebAppQuery,
    __in WCA_WRAPQUERY_HANDLE hWebAppExtQuery,
    __inout LPWSTR *ppwzCustomActionData,
    __out SCA_WEBDIR** ppswdList
    )
{
    Assert(piMetabase && ppswdList);

    HRESULT hr = S_OK;
    UINT er = ERROR_SUCCESS;
    MSIHANDLE hRec;

    LPWSTR pwzData = NULL;
    SCA_WEBDIR* pswd;
    WCA_WRAPQUERY_HANDLE hWrapQuery = NULL;

    hr = WcaBeginUnwrapQuery(&hWrapQuery, ppwzCustomActionData);
    ExitOnFailure(hr, "Failed to unwrap query for ScaAppPoolRead");

    if (0 == WcaGetQueryRecords(hWrapQuery))
    {
        WcaLog(LOGMSG_VERBOSE, "Skipping ScaInstallWebDirs() because IIsWebDir table not present");
        hr = S_FALSE;
        goto LExit;
    }

    // loop through all the web directories
    while (S_OK == (hr = WcaFetchWrappedRecord(hWrapQuery, &hRec)))
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

        hr = WcaGetRecordInteger(hRec, wdqInstalled, (int *)&pswd->isInstalled);
        ExitOnFailure(hr, "Failed to get Component installed state for webdir");

        hr = WcaGetRecordInteger(hRec, wdqAction, (int *)&pswd->isAction);
        ExitOnFailure(hr, "Failed to get Component action state for webdir");

        hr = WcaGetRecordString(hRec, wdqWeb, &pwzData);
        ExitOnFailure(hr, "Failed to get Web for WebDir");

        hr = ScaWebsGetBase(piMetabase, pswList, pwzData, pswd->wzWebBase, countof(pswd->wzWebBase), hWebBaseQuery);
        ExitOnFailure(hr, "Failed to get base of web for WebDir");

        hr = WcaGetRecordString(hRec, wdqPath, &pwzData);
        ExitOnFailure(hr, "Failed to get Path for WebDir");

        hr = ::StringCchPrintfW(pswd->wzWebDirRoot, countof(pswd->wzWebDirRoot), L"%s/Root/%s", pswd->wzWebBase, pwzData);
        ExitOnFailure(hr, "Failed to format webdir root string");

        // get the directory properties for this web
        hr = WcaGetRecordString(hRec, wdqProperties, &pwzData);
        ExitOnFailure(hr, "Failed to get security identifier for WebDir");
        if (*pwzData)
        {
            hr = ScaGetWebDirProperties(pwzData, hUserQuery, hWebDirPropQuery, &pswd->swp);
            ExitOnFailure(hr, "Failed to get properties for WebDir");

            pswd->fHasProperties = TRUE;
        }

        // get the application information for this web directory
        hr = WcaGetRecordString(hRec, wdqApplication, &pwzData);
        ExitOnFailure(hr, "Failed to get application identifier for WebDir");
        if (*pwzData)
        {
            hr = ScaGetWebApplication(NULL, pwzData, hWebAppQuery, hWebAppExtQuery, &pswd->swapp);
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
    WcaFinishUnwrapQuery(hWrapQuery);

    ReleaseStr(pwzData);

    return hr;
}


HRESULT ScaWebDirsInstall(IMSAdminBase* piMetabase, SCA_WEBDIR* pswdList, SCA_APPPOOL * psapList)
{
    HRESULT hr = S_OK;
    SCA_WEBDIR* pswd = pswdList;
    int i;

    while (pswd)
    {
        // On reinstall, we have to uninstall the old application, otherwise a duplicate will be created
        if (WcaIsReInstalling(pswd->isInstalled, pswd->isAction))
        {
            if (pswd->fHasApplication)
            {
                hr = ScaDeleteApp(piMetabase, pswd->wzWebDirRoot);
                ExitOnFailure(hr, "Failed to remove application for WebDir as part of a reinstall");
            }
        }

        // if we are installing the web site
        if (WcaIsInstalling(pswd->isInstalled, pswd->isAction))
        {
            hr = ScaCreateMetabaseKey(piMetabase, pswd->wzWebDirRoot, L"");
            ExitOnFailure(hr, "Failed to create key for WebDir");
            hr = ScaWriteMetabaseValue(piMetabase, pswd->wzWebDirRoot, L"", MD_KEY_TYPE, METADATA_NO_ATTRIBUTES, IIS_MD_UT_SERVER, STRING_METADATA, (LPVOID)L"IIsWebDirectory");
            ExitOnFailure(hr, "Failed to write key type for for WebDir");
            i = 0x4000003e; // 1073741886; // default directory browsing rights
            hr = ScaWriteMetabaseValue(piMetabase, pswd->wzWebDirRoot, L"", MD_DIRECTORY_BROWSING, METADATA_INHERIT, IIS_MD_UT_FILE, DWORD_METADATA, (LPVOID)((DWORD_PTR)i));
            ExitOnFailure(hr, "Failed to set directory browsing for WebDir");

            // get the security information for this web
            if (pswd->fHasProperties)
            {
                ScaWriteWebDirProperties(piMetabase, pswd->wzWebDirRoot, &pswd->swp);
                ExitOnFailure(hr, "Failed to write properties for WebDir");
            }

            // get the application information for this web directory
            if (pswd->fHasApplication)
            {
                hr = ScaWriteWebApplication(piMetabase, pswd->wzWebDirRoot, &pswd->swapp, psapList);
                ExitOnFailure(hr, "Failed to write application for WebDir");
            }
        }

        pswd = pswd->pswdNext;
    }

LExit:
    return hr;
}


HRESULT ScaWebDirsUninstall(IMSAdminBase* piMetabase, SCA_WEBDIR* pswdList)
{
    Assert(piMetabase);

    HRESULT hr = S_OK;
    SCA_WEBDIR* pswd = pswdList;

    while (pswd)
    {
        if (WcaIsUninstalling(pswd->isInstalled, pswd->isAction))
        {
            // remove the application from this web directory
            if (pswd->fHasApplication)
            {
                hr = ScaDeleteApp(piMetabase, pswd->wzWebDirRoot);
                ExitOnFailure(hr, "Failed to remove application for WebDir");
            }

            hr = ScaDeleteMetabaseKey(piMetabase, pswd->wzWebDirRoot, L"");
            ExitOnFailure1(hr, "Failed to remove WebDir '%S' from metabase", pswd->wzKey);
        }

        pswd = pswd->pswdNext;
    }

LExit:
    return hr;
}


void ScaWebDirsFreeList(SCA_WEBDIR* pswdList)
{
    SCA_WEBDIR* pswdDelete = pswdList;
    while (pswdList)
    {
        pswdDelete = pswdList;
        pswdList = pswdList->pswdNext;

        MemFree(pswdDelete);
    }
}


HRESULT AddWebDirToList(SCA_WEBDIR** ppswdList)
{
    HRESULT hr = S_OK;
    SCA_WEBDIR* pswd = static_cast<SCA_WEBDIR*>(MemAlloc(sizeof(SCA_WEBDIR), TRUE));
    ExitOnNull(pswd, hr, E_OUTOFMEMORY, "failed to allocate element for web dir list");

    pswd->pswdNext = *ppswdList;
    *ppswdList = pswd;

LExit:
    return hr;
}
