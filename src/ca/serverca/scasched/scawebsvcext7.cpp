//-------------------------------------------------------------------------------------------------
// <copyright file="scawebsvcext7.cpp" company="Microsoft">
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
//    IIS Web Service Extension Table functions for CustomActions
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"

// sql queries
LPCWSTR vcsWebSvcExtQuery7 = L"SELECT `Component_`, `File`, `Description`, `Group`, `Attributes` FROM `IIsWebServiceExtension`";
enum eWebSvcExtQuery { ldqComponent=1 , ldqFile, ldqDescription, ldqGroup, ldqAttributes, ldqInstalled, ldqAction };

// prototypes for private helper functions
static HRESULT AddWebSvcExtToList(
    __in SCA_WEBSVCEXT** ppsWseList
    );

static HRESULT ScaWebSvcExtInstall(
    const SCA_WEBSVCEXT* psWseList
    );

static HRESULT ScaWebSvcExtUninstall(
    const SCA_WEBSVCEXT* psWseList
    );

// functions

HRESULT __stdcall ScaWebSvcExtRead7(
    __in SCA_WEBSVCEXT** ppsWseList
    )
{
    Assert(ppsWseList);

    HRESULT hr = S_OK;
    UINT er = ERROR_SUCCESS;
    PMSIHANDLE hView, hRec;
    LPWSTR pwzData = NULL;
    INSTALLSTATE isInstalled = INSTALLSTATE_UNKNOWN;
    INSTALLSTATE isAction = INSTALLSTATE_UNKNOWN;
    SCA_WEBSVCEXT* psWebSvcExt = NULL;

    // check to see if necessary tables are specified
    if (S_OK != WcaTableExists(L"IIsWebServiceExtension"))
    {
        WcaLog(LOGMSG_VERBOSE, "Skipping ScaWebSvcExtRead7() - because IIsWebServiceExtension table not present");
        ExitFunction1(hr = S_FALSE);
    }

    // loop through all the web service extensions
    hr = WcaOpenExecuteView(vcsWebSvcExtQuery7, &hView);
    ExitOnFailure(hr, "Failed to open view on IIsWebServiceExtension table");
    while (S_OK == (hr = WcaFetchRecord(hView, &hRec)))
    {
        // Get the Component first.  If the Component is not being modified during
        // this transaction, skip processing this whole record.
        hr = WcaGetRecordString(hRec, ldqComponent, &pwzData);
        ExitOnFailure(hr, "Failed to get Component for WebSvcExt");

        er = ::MsiGetComponentStateW(WcaGetInstallHandle(), pwzData, &isInstalled, &isAction);
        hr = HRESULT_FROM_WIN32(er);
        ExitOnFailure(hr, "Failed to get Component state for WebSvcExt");

        if (!WcaIsInstalling(isInstalled, isAction) &&
            !WcaIsReInstalling(isInstalled, isAction) &&
            !WcaIsUninstalling(isInstalled, isAction))
        {
            continue; // skip this record.
        }

        hr = AddWebSvcExtToList(ppsWseList);
        ExitOnFailure(hr, "failed to add element to web svc ext list");

        psWebSvcExt = *ppsWseList;
        Assert(psWebSvcExt);

        psWebSvcExt->isInstalled = isInstalled;
        psWebSvcExt->isAction = isAction;

        hr = WcaGetRecordFormattedString(hRec, ldqFile, &pwzData);
        ExitOnFailure(hr, "Failed to get File for WebSvcExt");
        hr = ::StringCchCopyW(psWebSvcExt->wzFile, countof(psWebSvcExt->wzFile), pwzData);
        ExitOnFailure(hr, "Failed to copy File for WebSvcExt");

        hr = WcaGetRecordFormattedString(hRec, ldqDescription, &pwzData);
        ExitOnFailure(hr, "Failed to get Description for WebSvcExt");
        hr = ::StringCchCopyW(psWebSvcExt->wzDescription, countof(psWebSvcExt->wzDescription), pwzData);
        ExitOnFailure(hr, "Failed to copy Description for WebSvcExt");

        hr = WcaGetRecordFormattedString(hRec, ldqGroup, &pwzData);
        ExitOnFailure(hr, "Failed to get Group for WebSvcExt");
        hr = ::StringCchCopyW(psWebSvcExt->wzGroup, countof(psWebSvcExt->wzGroup), pwzData);
        ExitOnFailure(hr, "Failed to copy Group for WebSvcExt");

        hr = WcaGetRecordInteger(hRec, ldqAttributes, &psWebSvcExt->iAttributes);
        ExitOnFailure(hr, "Failed to get Attributes for WebSvcExt");
    }

    if (E_NOMOREITEMS == hr)
    {
        hr = S_OK;
    }
    ExitOnFailure(hr, "Failure while processing WebSvcExt");

LExit:
    ReleaseStr(pwzData);

    return hr;
}


// Commit does both install and uninstall
HRESULT __stdcall ScaWebSvcExtCommit7(
    __in SCA_WEBSVCEXT* psWseList
    )
{
    HRESULT hr = S_OK;

    if(!psWseList)
    {
        WcaLog(LOGMSG_VERBOSE, "Skipping ScaWebSvcExtCommit() because there are no web service extensions in the list");
        ExitFunction();
    }

    // Make changes to local copy of metabase
    while (psWseList)
    {
        if (WcaIsInstalling(psWseList->isInstalled, psWseList->isAction))
        {
            hr = ScaWebSvcExtInstall(psWseList);
            ExitOnFailure(hr, "Failed to install Web Service extension");
        }
        else if (WcaIsUninstalling(psWseList->isInstalled, psWseList->isAction))
        {
            hr = ScaWebSvcExtUninstall(psWseList);
            ExitOnFailure(hr, "Failed to uninstall Web Service extension");
        }

        psWseList = psWseList->psWseNext;
    }


LExit:

    return hr;
}


HRESULT __stdcall ScaWebSvcExtInstall(
    const SCA_WEBSVCEXT* psWseList
    )
{
    HRESULT hr = S_OK;
    int iAllow;

    //Write CAData actions
    hr = ScaWriteConfigID(IIS_WEB_SVC_EXT);
    ExitOnFailure(hr, "failed add web svc ext ID");
    hr = ScaWriteConfigID(IIS_CREATE);
    ExitOnFailure(hr, "failed add web svc ext action");

    // write File path
    hr = ScaWriteConfigString(psWseList->wzFile);
    ExitOnFailure(hr, "failed add web svc ext file path");

    // write allowed
    // unDeleatable n/a in IIS7
    iAllow = (psWseList->iAttributes & 1);
    hr = ScaWriteConfigInteger(iAllow);
    ExitOnFailure(hr, "failed add web svc ext Allowed");

    //write group
    hr = ScaWriteConfigString(psWseList->wzGroup);
    ExitOnFailure(hr, "failed add web svc ext group");

    //write description
    hr = ScaWriteConfigString(psWseList->wzDescription);
    ExitOnFailure(hr, "failed add web svc ext description");

LExit:

    return hr;
}


HRESULT __stdcall ScaWebSvcExtUninstall(
    const SCA_WEBSVCEXT* psWseList
    )
{
    HRESULT hr = S_OK;

    //Write CAData actions
    hr = ScaWriteConfigID(IIS_WEB_SVC_EXT);
    ExitOnFailure(hr, "failed add web svc ext ID");
    hr = ScaWriteConfigID(IIS_DELETE);
    ExitOnFailure(hr, "failed add web svc ext action");

    // write File path (Key)
    hr = ScaWriteConfigString(psWseList->wzFile);
    ExitOnFailure(hr, "failed add web svc ext file path");

LExit:
    return hr;
}


void ScaWebSvcExtFreeList7(
    __in SCA_WEBSVCEXT* psWseList
    )
{
    SCA_WEBSVCEXT* psWseDelete = psWseList;
    while (psWseList)
    {
        psWseDelete = psWseList;
        psWseList = psWseList->psWseNext;

        MemFree(psWseDelete);
    }
}


static HRESULT AddWebSvcExtToList(
    __in SCA_WEBSVCEXT** ppsWseList
    )
{
    HRESULT hr = S_OK;

    SCA_WEBSVCEXT* psWse = static_cast<SCA_WEBSVCEXT*>(MemAlloc(sizeof(SCA_WEBSVCEXT), TRUE));
    ExitOnNull(psWse, hr, E_OUTOFMEMORY, "failed to allocate element for web svc ext list");

    psWse->psWseNext = *ppsWseList;
    *ppsWseList = psWse;

LExit:
    return hr;
}
