//-------------------------------------------------------------------------------------------------
// <copyright file="scaweb7.h" company="Microsoft">
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
//    IIS Web Table functions for CustomActions
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"

//Adding this because delivery doesn't have the updated specstrings.h that windows build does
#ifndef __in_xcount
#define __in_xcount(size)
#endif

// sql queries
LPCWSTR vcsWebQueryIIS7 = L"SELECT `Web`, `Component_`, `Id`, `Description`, `ConnectionTimeout`, `Directory_`, `State`, `Attributes`, `DirProperties_`, `Application_`, "
                      L"`Address`, `IP`, `Port`, `Header`, `Secure`, `Log_` FROM `IIsWebSite`, `IIsWebAddress` "
                      L"WHERE `KeyAddress_`=`Address` ORDER BY `Sequence`";

enum eWebQueryIIS7 { wqWeb = 1, wqComponent , wqId, wqDescription, wqConnectionTimeout, wqDirectory,
                 wqState, wqAttributes, wqProperties, wqApplication, wqAddress, wqIP, wqPort, wqHeader, wqSecure, wqLog};

LPCWSTR vcsWebAddressQueryIIS7 = L"SELECT `Address`, `IP`, `Port`, `Header`, `Secure` "
                             L"FROM `IIsWebAddress` WHERE `Web_`=?";

enum eWebAddressQueryIIS7 { waqAddress = 1, waqIP, waqPort, waqHeader, waqSecure };


LPCWSTR vcsWebBaseQueryIIS7 = L"SELECT `Web`, `IP`, `Port`, `Header`, `Secure` "
                          L"FROM `IIsWebSite`, `IIsWebAddress` "
                          L"WHERE `KeyAddress_`=`Address` AND `Web`=?";
enum eWebBaseQueryIIS7 { wbqWeb = 1, wbqIP, wbqPort, wbqHeader, wbqSecure};



// prototypes for private helper functions
static SCA_WEB7* NewWeb7();
static SCA_WEB7* AddWebToList7(
    __in SCA_WEB7* pswList,
    __in SCA_WEB7* psw
    );

static HRESULT ScaWebFindBase7(
    __in SCA_WEB7* pswList,
    LPCWSTR wzDescription
    );

static HRESULT ScaWebWrite7(
    __in SCA_WEB7* psw,
    __in SCA_APPPOOL * psapList
    );

static HRESULT ScaWebRemove7(__in const SCA_WEB7* psw);


HRESULT ScaWebsRead7(
    __in SCA_WEB7** ppswList,
    __in SCA_HTTP_HEADER** ppshhList,
    __in SCA_WEB_ERROR** ppsweList
    )
{
    Assert(ppswList);
    WcaLog(LOGMSG_VERBOSE, "Entering ScaWebsRead7()");

    HRESULT hr = S_OK;
    UINT er = ERROR_SUCCESS;

    BOOL fIIsWebSiteTable = FALSE;
    BOOL fIIsWebAddressTable = FALSE;
    BOOL fIIsWebApplicationTable = FALSE;

    PMSIHANDLE hView, hRec;
    PMSIHANDLE hViewAddresses, hRecAddresses;
    PMSIHANDLE hViewApplications, hRecApplications;

    SCA_WEB7* psw = NULL;
    LPWSTR pwzData = NULL;

    DWORD dwLen = 0;
    errno_t error = EINVAL;

    // check to see what tables are available
    fIIsWebSiteTable = (S_OK == WcaTableExists(L"IIsWebSite"));
    fIIsWebAddressTable = (S_OK == WcaTableExists(L"IIsWebAddress"));
    fIIsWebApplicationTable = (S_OK == WcaTableExists(L"IIsWebApplication"));

    if (!fIIsWebSiteTable || !fIIsWebAddressTable)
    {
        WcaLog(LOGMSG_VERBOSE, "Skipping ScaWebsRead7() - because IIsWebSite/IIsWebAddress table not present.");
        ExitFunction1(hr = S_FALSE);
    }
    else
    {
        WcaLog(LOGMSG_VERBOSE, "Executing ScaWebsRead7() - Confirmed minimum required tables exist");
    }

    // open the view on webs' addresses
    hr = WcaOpenView(vcsWebAddressQueryIIS7, &hViewAddresses);
    ExitOnFailure(hr, "Failed to open view on IIsWebAddress table");

    // open the view on webs' applications
    if (fIIsWebApplicationTable)
    {
        hr = WcaOpenView(vcsWebApplicationQuery, &hViewApplications);
        ExitOnFailure(hr, "Failed to open view on IIsWebApplication table");
    }

    WcaLog(LOGMSG_VERBOSE, "Executing ScaWebsRead7() - Opened views for IIsWebAddress table & IIsWebApplicationTable");

    // loop through all the webs
    hr = WcaOpenExecuteView(vcsWebQueryIIS7, &hView);
    ExitOnFailure(hr, "Failed to execute view on IIsWebSite table");
    WcaLog(LOGMSG_VERBOSE, "Executing ScaWebsRead7() - Opened and executed view for IIsWebSite");
    
    while (S_OK == (hr = WcaFetchRecord(hView, &hRec)))
    {
        psw = NewWeb7();
        if (!psw)
        {
            hr = E_OUTOFMEMORY;
            break;
        }
        WcaLog(LOGMSG_VERBOSE, "Executing ScaWebsRead7() - Fetched record from IIsWebSite table");

        // get the darwin information
        hr = WcaGetRecordString(hRec, wqWeb, &pwzData);
        ExitOnFailure(hr, "Failed to get Web");
        hr = ::StringCchCopyW(psw->wzKey, countof(psw->wzKey), pwzData);
        ExitOnFailure(hr, "Failed to copy web key");

        // get component install state
        hr = WcaGetRecordString(hRec, wqComponent, &pwzData);
        ExitOnFailure(hr, "Failed to get Component for Web");
        hr = ::StringCchCopyW(psw->wzComponent, countof(psw->wzComponent), pwzData);
        ExitOnFailure(hr, "Failed to copy web component");
        if (*(psw->wzComponent))
        {
            psw->fHasComponent = TRUE;

            er = ::MsiGetComponentStateW(WcaGetInstallHandle(), psw->wzComponent, &psw->isInstalled, &psw->isAction);
            hr = HRESULT_FROM_WIN32(er);
            ExitOnFailure(hr, "Failed to get web Component state");
        }
        // get the web's description (Site Name)
        hr = WcaGetRecordFormattedString(hRec, wqDescription, &pwzData);
        ExitOnFailure(hr, "Failed to get Description for Web");
        hr = ::StringCchCopyW(psw->wzDescription, countof(psw->wzDescription), pwzData);
        ExitOnFailure(hr, "Failed to copy web description");

        //get web's site Id
        hr = WcaGetRecordInteger(hRec, wqId, &psw->iSiteId);
        ExitOnFailure(hr, "Failed to get SiteId for Web");

        // get the web's key address (Bindings)
        hr = WcaGetRecordString(hRec, wqAddress, &pwzData);
        ExitOnFailure(hr, "Failed to get Address for Web");
        hr = ::StringCchCopyW(psw->swaBinding.wzKey, countof(psw->swaBinding.wzKey), pwzData);
        ExitOnFailure(hr, "Failed to copy web binding key");

        hr = WcaGetRecordFormattedString(hRec, wqIP, &pwzData);
        ExitOnFailure(hr, "Failed to get IP for Web");
        hr = ::StringCchCopyW(psw->swaBinding.wzIP, countof(psw->swaBinding.wzIP), pwzData);
        ExitOnFailure(hr, "Failed to copy web IP");

        hr = WcaGetRecordFormattedString(hRec, wqPort, &pwzData);
        ExitOnFailure(hr, "Failed to get Web Address port");
        psw->swaBinding.iPort = wcstol(pwzData, NULL, 10);

        hr = WcaGetRecordFormattedString(hRec, wqHeader, &pwzData);
        ExitOnFailure(hr, "Failed to get Header for Web");
        hr = ::StringCchCopyW(psw->swaBinding.wzHeader, countof(psw->swaBinding.wzHeader), pwzData);
        ExitOnFailure(hr, "Failed to copy web header");

        hr = WcaGetRecordInteger(hRec, wqSecure, &psw->swaBinding.fSecure);
        ExitOnFailure(hr, "Failed to get if Web is secure");
        if (S_FALSE == hr)
        {
            psw->swaBinding.fSecure = FALSE;
        }

        WcaLog(LOGMSG_VERBOSE, "Entering ScaWebFindBase7()");
        // look to see if site exists
        dwLen = METADATA_MAX_NAME_LEN;
        hr = ScaWebFindBase7(*ppswList, psw->wzDescription);
        WcaLog(LOGMSG_VERBOSE, "Exiting ScaWebFindBase7()");

        // If we didn't find a web in memory, ignore it - during execute CA
        // if the site truly does not exist then there will be an error.
        if (S_OK == hr)
        {
            // site exists in config
            psw->fBaseExists = TRUE;
        }
        else if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr)
        {
            hr = S_OK;

            // site does not exists in config
            psw->fBaseExists = FALSE;
        }
        ExitOnFailure(hr, "Failed to find web site");

        // get any extra web addresses
        hr = WcaExecuteView(hViewAddresses, hRec);
        ExitOnFailure(hr, "Failed to execute view on extra IIsWebAddress table");
        WcaLog(LOGMSG_VERBOSE, "Executing ScaWebsRead7() - Executing view on Address table");
        while (S_OK == (hr = WcaFetchRecord(hViewAddresses, &hRecAddresses)))
        {
            WcaLog(LOGMSG_VERBOSE, "Executing ScaWebsRead7() - Fetched record from IIsWebAddress table");
            if (MAX_ADDRESSES_PER_WEB <= psw->cExtraAddresses)
            {
                hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
                ExitOnFailure(hr, "Failure to get more extra web addresses, max exceeded.");
            }

            hr = WcaGetRecordString(hRecAddresses, waqAddress, &pwzData);
            ExitOnFailure(hr, "Failed to get extra web Address");
            hr = ::StringCchCopyW(psw->swaExtraAddresses[psw->cExtraAddresses].wzKey,
                countof(psw->swaExtraAddresses[psw->cExtraAddresses].wzKey), pwzData);
            ExitOnFailure(hr, "Failed to copy web binding key");

            hr = WcaGetRecordFormattedString(hRecAddresses, waqIP, &pwzData);
            ExitOnFailure(hr, "Failed to get extra web IP");
            hr = ::StringCchCopyW(psw->swaExtraAddresses[psw->cExtraAddresses].wzIP, countof(psw->swaExtraAddresses[psw->cExtraAddresses].wzIP), pwzData);
            ExitOnFailure(hr, "Failed to copy web binding IP");

            hr = WcaGetRecordFormattedString(hRecAddresses, waqPort, &pwzData);
            ExitOnFailure(hr, "Failed to get port for extra web IP");
            psw->swaExtraAddresses[psw->cExtraAddresses].iPort= wcstol(pwzData, NULL, 10);

            // errno is set to ERANGE if overflow or underflow occurs
            _get_errno(&error);

            if (ERANGE == error)
            {
                hr = E_INVALIDARG;
                ExitOnFailure(hr, "Failed to convert web Port address");
            }

            hr = WcaGetRecordFormattedString(hRecAddresses, waqHeader, &pwzData);
            ExitOnFailure(hr, "Failed to get header for extra web IP");
            hr = ::StringCchCopyW(psw->swaExtraAddresses[psw->cExtraAddresses].wzHeader, countof(psw->swaExtraAddresses[psw->cExtraAddresses].wzHeader), pwzData);
            ExitOnFailure(hr, "Failed to copy web binding header");

            hr = WcaGetRecordInteger(hRecAddresses, waqSecure, &psw->swaExtraAddresses[psw->cExtraAddresses].fSecure);
            ExitOnFailure(hr, "Failed to get if secure extra web IP");
            if (S_FALSE == hr)
            {
                psw->swaExtraAddresses[psw->cExtraAddresses].fSecure = FALSE;
            }

            psw->cExtraAddresses++;
        }

        if (E_NOMOREITEMS == hr)
        {
            hr = S_OK;
        }
        ExitOnFailure(hr, "Failure occured while getting extra web addresses");

        //
        // Connection time out
        //
        hr = WcaGetRecordInteger(hRec, wqConnectionTimeout, &psw->iConnectionTimeout);
        ExitOnFailure(hr, "Failed to get connection timeout for Web");

        if (psw->fHasComponent) // If we're installing it, it needs a dir
        {
            // get the web's directory
            hr = WcaGetRecordString(hRec, wqDirectory, &pwzData);
            ExitOnFailure(hr, "Failed to get Directory for Web");

            WCHAR wzPath[MAX_PATH];
            dwLen = countof(wzPath);
            if (INSTALLSTATE_SOURCE == psw->isAction)
            {
                er = ::MsiGetSourcePathW(WcaGetInstallHandle(), pwzData, wzPath, &dwLen);
            }
            else
            {
                er = ::MsiGetTargetPathW(WcaGetInstallHandle(), pwzData, wzPath, &dwLen);
            }
            hr = HRESULT_FROM_WIN32(er);
            ExitOnFailure(hr, "Failed to get Source/TargetPath for Directory");

            if (dwLen > countof(wzPath))
            {
                hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
                ExitOnFailure(hr, "Failed because Source/TargetPath for Directory was greater than MAX_PATH.");
            }

            // remove trailing backslash
            if (dwLen > 0 && wzPath[dwLen-1] == L'\\')
            {
                wzPath[dwLen-1] = 0;
            }
            hr = ::StringCchCopyW(psw->wzDirectory, countof(psw->wzDirectory), wzPath);
            ExitOnFailure1(hr, "Failed to copy web dir: '%S'", wzPath);

        }

        hr = WcaGetRecordInteger(hRec, wqState, &psw->iState);
        ExitOnFailure(hr, "Failed to get state for Web");

        hr = WcaGetRecordInteger(hRec, wqAttributes, &psw->iAttributes);
        ExitOnFailure(hr, "Failed to get attributes for Web");

        // get the dir properties for this web
        hr = WcaGetRecordString(hRec, wqProperties, &pwzData);
        ExitOnFailure(hr, "Failed to get directory properties for Web");
        if (*pwzData)
        {
            WcaLog(LOGMSG_VERBOSE, "Executing ScaWebsRead7() - Entering ScaGetWebDirProperties7");
            hr = ScaGetWebDirProperties7(pwzData, &psw->swp);
            ExitOnFailure(hr, "Failed to get directory properties for Web");
            WcaLog(LOGMSG_VERBOSE, "Executing ScaWebsRead7() - Entering ScaGetWebDirProperties7");

            psw->fHasProperties = TRUE;
        }

        // get the application information for this web
        hr = WcaGetRecordString(hRec, wqApplication, &pwzData);
        ExitOnFailure(hr, "Failed to get application identifier for Web");
        if (*pwzData)
        {
            WcaLog(LOGMSG_VERBOSE, "Executing ScaWebsRead7() - Entering ScaGetWebApplication7");
            hr = ScaGetWebApplication7(NULL, pwzData, &psw->swapp);
            ExitOnFailure(hr, "Failed to get application for Web");
            WcaLog(LOGMSG_VERBOSE, "Executing ScaWebsRead7() - Exiting ScaGetWebApplication7");

            psw->fHasApplication = TRUE;
        }

        // get the SSL certificates
        WcaLog(LOGMSG_VERBOSE, "Executing ScaWebsRead7() - Entering ScaSslCertificateRead7");
        hr = ScaSslCertificateRead7(psw->wzKey, &(psw->pswscList));
        ExitOnFailure(hr, "Failed to get SSL Certificates.");
        WcaLog(LOGMSG_VERBOSE, "Executing ScaWebsRead7() - Exiting ScaSslCertificateRead7");

        // get the custom headers
        if (*ppshhList)
        {
            WcaLog(LOGMSG_VERBOSE, "Executing ScaWebsRead7() - Entering ScaGetHttpHeader7");
            hr = ScaGetHttpHeader7(hhptWeb, psw->wzKey, ppshhList, &(psw->pshhList));
            ExitOnFailure(hr, "Failed to get Custom HTTP Headers");
            WcaLog(LOGMSG_VERBOSE, "Executing ScaWebsRead7() - Exiting ScaGetHttpHeader7");
        }

        // get the errors
        if (*ppsweList)
        {
            WcaLog(LOGMSG_VERBOSE, "Executing ScaWebsRead7() - Entering ScaGetWebError7");
            hr = ScaGetWebError7(weptWeb, psw->wzKey, ppsweList, &(psw->psweList));
            ExitOnFailure(hr, "Failed to get Custom Errors");
            WcaLog(LOGMSG_VERBOSE, "Executing ScaWebsRead7() - Exiting ScaGetWebError7");
        }

        // get the log information for this web
        hr = WcaGetRecordString(hRec, wqLog, &pwzData);
        ExitOnFailure(hr, "Failed to get log identifier for Web");
        if (*pwzData)
        {
            hr = ScaGetWebLog7(pwzData, &psw->swl);
            ExitOnFailure(hr, "Failed to get Log for Web.");
            psw->fHasLog = TRUE;
        }

        *ppswList = AddWebToList7(*ppswList, psw);
        psw = NULL; // set the web NULL so it doesn't accidentally get freed below
    }

    if (E_NOMOREITEMS == hr)
    {
        hr = S_OK;
    }

LExit:
    // if anything was left over after an error clean it all up
    ScaWebsFreeList7(psw);

    ReleaseStr(pwzData);
    WcaLog(LOGMSG_VERBOSE, "Exiting ScaWebsRead7()");

    return hr;
}


HRESULT ScaWebsGetBase7(
    __in SCA_WEB7* pswList,
    __in LPCWSTR pswWebKey,
    __in SCA_WEB7** pswWeb
    )
{
    HRESULT hr = S_OK;
    BOOL fFound = FALSE;

    //looking for psw->wzKey == pswWebKey
    for (SCA_WEB7* psw = pswList; psw; psw = psw->pswNext)
    {
        if (0 == wcscmp(pswWebKey, psw->wzKey))
        {
            *pswWeb = psw;
            fFound = TRUE;
            break;
        }
    }

    if (!fFound && SUCCEEDED(hr))
    {
        hr = S_FALSE;
    }

    return hr;
}


HRESULT ScaWebsInstall7(
    __in SCA_WEB7* pswList,
    __in SCA_APPPOOL * psapList
    )
{
    HRESULT hr = S_OK;
    SCA_WEB7* psw = pswList;

    while (psw)
    {
        // if we are installing the web site
        if (psw->fHasComponent && WcaIsInstalling(psw->isInstalled, psw->isAction))
        {
            hr = ScaWebWrite7(psw, psapList);
            ExitOnFailure1(hr, "failed to write web '%S' to metabase", psw->wzKey);
        }

        psw = psw->pswNext;
    }

LExit:
    return hr;
}


HRESULT ScaWebsUninstall7(
    __in SCA_WEB7* pswList
    )
{
    HRESULT hr = S_OK;
    SCA_WEB7* psw = pswList;

    while (psw)
    {
        // if we are uninstalling the web site
        if (psw->fHasComponent && WcaIsUninstalling(psw->isInstalled, psw->isAction))
        {
            hr = ScaWebRemove7(psw);
            ExitOnFailure1(hr, "Failed to remove web '%S' ", psw->wzKey);
        }

        psw = psw->pswNext;
    }

LExit:
    return hr;
}


void ScaWebsFreeList7(__in SCA_WEB7* pswList)
{
    SCA_WEB7* pswDelete = pswList;
    while (pswList)
    {
        pswDelete = pswList;
        pswList = pswList->pswNext;

        // Free the SSL, headers and errors list first
        ScaSslCertificateFreeList7(pswDelete->pswscList);
        ScaHttpHeaderFreeList7(pswDelete->pshhList);
        ScaWebErrorFreeList(pswDelete->psweList);

        MemFree(pswDelete);
    }
}


// private helper functions

static SCA_WEB7* NewWeb7()
{
    SCA_WEB7* psw = (SCA_WEB7*)MemAlloc(sizeof(SCA_WEB7), TRUE);
    Assert(psw);
    return psw;
}


static SCA_WEB7* AddWebToList7(
    __in SCA_WEB7* pswList,
    __in SCA_WEB7* psw
    )
{
    if (pswList)
    {
        SCA_WEB7* pswTemp = pswList;
        while (pswTemp->pswNext)
        {
            pswTemp = pswTemp->pswNext;
        }

        pswTemp->pswNext = psw;
    }
    else
    {
        pswList = psw;
    }

    return pswList;
}


static HRESULT ScaWebFindBase7(
    __in SCA_WEB7* pswList,
    __in_z LPCWSTR wzDescription
    )
{
    HRESULT hr = S_OK;
    BOOL fFound = FALSE;

    // try to find the web in memory first
    for (SCA_WEB7* psw = pswList; psw; psw = psw->pswNext)
    {
        if (0 == wcscmp(wzDescription, psw->wzDescription))
        {
            fFound = TRUE;
            break;
        }
    }

    if (!fFound)
    {
        hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
    }

    return hr;
}


static HRESULT ScaWebWrite7(
    __in SCA_WEB7* psw,
    __in SCA_APPPOOL * psapList
    )
{
    HRESULT hr = S_OK;

    UINT ui = 0;
    WCHAR wzIP[64];
    WCHAR wzBinding[1024];
    WCHAR wzAppPoolName[MAX_PATH];

    //create a site
    hr = ScaWriteConfigID(IIS_SITE);
    ExitOnFailure(hr, "Failed write site ID");
    //
    // determine if site must be new
    //
    if (psw->iAttributes & SWATTRIB_NOCONFIGUREIFEXISTS)
        // if we're not supposed to configure existing webs
        // then create only if new
    {
        hr = ScaWriteConfigID(IIS_CREATE_NEW);
        ExitOnFailure(hr, "Failed write site ID create new action");
    }
    else
    {
        hr = ScaWriteConfigID(IIS_CREATE);
        ExitOnFailure(hr, "Failed write site ID create action");
    }
    //Site Name
    hr = ScaWriteConfigString(psw->wzDescription);                  //Site Name
    ExitOnFailure(hr, "Failed write site desc");

    //Site Id -- value is MSI_NULL_INTEGER if not set in WIX
    hr = ScaWriteConfigInteger(psw->iSiteId);                        //SiteID
    ExitOnFailure(hr, "Failed write site id value");

    //Site Auto Start -- value is MSI_NULL_INTEGER if not set in WIX
    hr = ScaWriteConfigInteger(psw->iState);                        // serverAutoStart
    ExitOnFailure(hr, "Failed write site autostart");

    hr = ScaWriteConfigInteger(psw->iConnectionTimeout);            //limits/connectionTimeout
    ExitOnFailure(hr, "Failed write site timeout");

    //create default application
    hr = ScaWriteConfigID(IIS_APPLICATION);
    ExitOnFailure(hr, "Failed write app ID");
    hr = ScaWriteConfigID(IIS_CREATE);
    ExitOnFailure(hr, "Failed write app action ID");
    hr = ScaWriteConfigString(psw->wzDescription);      //site name key
    ExitOnFailure(hr, "Failed write app desc");
    hr = ScaWriteConfigString(L"/");                    //  App Path (default)
    ExitOnFailure(hr, "Failed write app def path /");

    if (psw->fHasApplication)
    {
        hr = ScaFindAppPool7(psw->swapp.wzAppPool, wzAppPoolName, countof(wzAppPoolName), psapList);
        ExitOnFailure(hr, "Failed to read app pool from application");
    }

    hr = ScaWriteConfigString(psw->fHasApplication ? wzAppPoolName : L"");
    ExitOnFailure(hr, "Failed write app appPool");

    //create vdir for default application
    hr = ScaWriteConfigID(IIS_VDIR);
    ExitOnFailure(hr, "Failed write vdir ID");
    hr = ScaWriteConfigID(IIS_CREATE);
    ExitOnFailure(hr, "Failed write vdir action");
    hr = ScaWriteConfigString(psw->wzDescription);      //site name key
    ExitOnFailure(hr, "Failed write vdir desc");
    hr = ScaWriteConfigString(L"/");                    //vdir path (default)
    ExitOnFailure(hr, "Failed write vdir app");
    hr = ScaWriteConfigString(psw->wzDirectory);         //physical dir
    ExitOnFailure(hr, "Failed write vdir dir");

    //create bindings for site
    hr = ScaWriteConfigID(IIS_BINDING);
    ExitOnFailure(hr, "Failed write binding ID");
    hr = ScaWriteConfigID(IIS_CREATE);
    ExitOnFailure(hr, "Failed write binding action ID");
    hr = ScaWriteConfigString(psw->wzDescription);      //site name key
    ExitOnFailure(hr, "Failed write binding site key");

    if (psw->swaBinding.fSecure)
    {
        hr = ScaWriteConfigString(L"https");            // binding protocol
        ExitOnFailure(hr, "Failed write binding https");
    }
    else
    {
        hr = ScaWriteConfigString(L"http");             // binding protocol
        ExitOnFailure(hr, "Failed write binding http");
    }

    // set the IP address appropriately
    if (0 == wcscmp(psw->swaBinding.wzIP, L"*"))
    {
        ::ZeroMemory(wzIP, sizeof(wzIP));
    }
    else
    {
#pragma prefast(suppress:26037, "Source string is null terminated - it is populated as target of ::StringCchCopyW")
        hr = ::StringCchCopyW(wzIP, countof(wzIP), psw->swaBinding.wzIP);
        ExitOnFailure(hr, "Failed to copy IP string");
    }

    hr = ::StringCchPrintfW(wzBinding, countof(wzBinding), L"%s:%d:%s", wzIP, psw->swaBinding.iPort, psw->swaBinding.wzHeader);
    ExitOnFailure(hr, "Failed to format IP:Port:Header binding string");

    // write bindings CAData
    hr = ScaWriteConfigString(wzBinding) ;            //binding info
    ExitOnFailure(hr, "Failed to create web bindings");

    for (ui = 0;(ui < MAX_ADDRESSES_PER_WEB) && (ui < psw->cExtraAddresses); ui++)
    {
        // set the IP address appropriately
        if (0 == wcscmp(psw->swaExtraAddresses[ui].wzIP, L"*"))
        {
            ::ZeroMemory(wzIP, sizeof(wzIP));
        }
        else
        {
#pragma prefast(suppress:26037, "Source string is null terminated - it is populated as target of ::StringCchCopyW")
            hr = ::StringCchCopyW(wzIP, countof(wzIP), psw->swaExtraAddresses[ui].wzIP);
            ExitOnFailure(hr, "Failed to copy web IP");
        }
        hr = ::StringCchPrintfW(wzBinding, countof(wzBinding), L"%s:%d:%s", wzIP, psw->swaExtraAddresses[ui].iPort, psw->swaExtraAddresses[ui].wzHeader);
        ExitOnFailure(hr, "Failed to copy web IP");

        //create bindings for site
        hr = ScaWriteConfigID(IIS_BINDING);
        ExitOnFailure(hr, "Failed write binding ID");
        hr = ScaWriteConfigID(IIS_CREATE);
        ExitOnFailure(hr, "Failed write binding action");
        hr = ScaWriteConfigString(psw->wzDescription);      //site name key
        ExitOnFailure(hr, "Failed write binding web name");

        if (psw->swaExtraAddresses[ui].fSecure)
        {
            hr = ScaWriteConfigString(L"https");            // binding protocol
        }
        else
        {
            hr = ScaWriteConfigString(L"http");             // binding protocol
        }
        ExitOnFailure(hr, "Failed write binding http(s)");

        // write bindings CAData
        hr = ScaWriteConfigString(wzBinding) ;              //binding info
        ExitOnFailure(hr, "Failed write binding info");
    }

    // write the web dirproperties information
    if (psw->fHasProperties)
    {
        // dir properties are for the default application of the web
        // with location '/'
        hr = ScaWriteWebDirProperties7(psw->wzDescription, L"/", &psw->swp);
        ExitOnFailure(hr, "Failed to write web security information to metabase");
    }

    //// write the application information
    if (psw->fHasApplication)
    {
        hr = ScaWriteWebApplication7(psw->wzDescription, L"/", &psw->swapp, psapList);
        ExitOnFailure(hr, "Failed to write web application information to metabase");
    }

    // write the SSL certificate information
    if (psw->pswscList)
    {
        hr = ScaSslCertificateWrite7(psw->wzDescription, psw->pswscList);
        ExitOnFailure1(hr, "Failed to write SSL certificates for Web site: %S", psw->wzKey);
    }

    // write the headers
    if (psw->pshhList)
    {
        hr = ScaWriteHttpHeader7(psw->wzDescription, L"/", psw->pshhList);
        ExitOnFailure1(hr, "Failed to write custom HTTP headers for Web site: %S", psw->wzKey);
    }

    // write the errors
    if (psw->psweList)
    {
        hr = ScaWriteWebError7(psw->wzDescription, L"/", psw->psweList);
        ExitOnFailure1(hr, "Failed to write custom web errors for Web site: %S", psw->wzKey);
    }

    // write the log information to the metabase
    if (psw->fHasLog)
    {
        hr = ScaWriteWebLog7(psw->wzDescription, &psw->swl);
        ExitOnFailure(hr, "Failed to write web log information to metabase");
    }

LExit:
    return hr;
}


static HRESULT ScaWebRemove7(
    __in const SCA_WEB7* psw
    )
{
    HRESULT hr = S_OK;

    hr = ScaWriteConfigID(IIS_SITE);
    ExitOnFailure(hr, "Failed write site ID");
    hr = ScaWriteConfigID(IIS_DELETE);
    ExitOnFailure(hr, "Failed write site action");
    hr = ScaWriteConfigString(psw->wzDescription);  //Site Name
    ExitOnFailure(hr, "Failed write site name");

LExit:
    return hr;
}
