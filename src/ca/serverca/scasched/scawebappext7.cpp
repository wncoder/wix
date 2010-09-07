//-------------------------------------------------------------------------------------------------
// <copyright file="scawebappext7.cpp" company="Microsoft">
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
//    IIS Web Application Extension functions for CustomActions
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"
// sql queries
LPCWSTR vcsWebAppExtensionQuery7 = L"SELECT `Extension`, `Verbs`, `Executable`, `Attributes` FROM `IIsWebApplicationExtension` WHERE `Application_`=?";
enum eWebAppExtensionQuery { wappextqExtension = 1, wappextqVerbs, wappextqExecutable, wappextqAttributes };

// prototypes for private helper functions
static HRESULT NewAppExt7(
    __out SCA_WEB_APPLICATION_EXTENSION** ppswappext
    );
static SCA_WEB_APPLICATION_EXTENSION* AddAppExtToList7(
    __in SCA_WEB_APPLICATION_EXTENSION* pswappextList,
    __in SCA_WEB_APPLICATION_EXTENSION* pswappext
    );


HRESULT ScaWebAppExtensionsRead7(
    __in_z LPCWSTR wzApplication,
    __inout SCA_WEB_APPLICATION_EXTENSION** ppswappextList
    )
{
    HRESULT hr = S_OK;
    PMSIHANDLE hView, hRec;
    SCA_WEB_APPLICATION_EXTENSION* pswappext = NULL;
    LPWSTR pwzData = NULL;

    // check pre-requisites
    hr = WcaTableExists(L"IIsWebApplicationExtension");
    if (S_FALSE == hr)
    {
        WcaLog(LOGMSG_VERBOSE, "Skipping ScaWebAppExtensionsRead7() - because IIsWebApplicationExtension table not present.");
        ExitFunction();
    }

    // convert the string into a msi record
    hRec = ::MsiCreateRecord(1);
    hr = WcaSetRecordString(hRec, 1, wzApplication);
    ExitOnFailure(hr, "Failed to set record to look up Web Application");

    // open and execute the view on the applicatoin extension table
    hr = WcaOpenView(vcsWebAppExtensionQuery7, &hView);
    ExitOnFailure(hr, "Failed to open view on IIsWebApplicationExtension table");

    hr = WcaExecuteView(hView, hRec);
    ExitOnFailure1(hr, "Failed to execute view on IIsWebApplicationExtension table looking Application: %S", wzApplication);

    // get the application extention information
    while (S_OK == (hr = WcaFetchRecord(hView, &hRec)))
    {
        hr = NewAppExt7(&pswappext);
        ExitOnFailure(hr, "failed to create new web app extension");

        // get the extension
        hr = WcaGetRecordString(hRec, wappextqExtension, &pwzData);
        ExitOnFailure(hr, "Failed to get Web Application Extension");
        hr = ::StringCchCopyW(pswappext->wzExtension, countof(pswappext->wzExtension), pwzData);
        ExitOnFailure1(hr, "Failed to copy extension string: '%S'", pwzData);

        // application extension verbs
        hr = WcaGetRecordFormattedString(hRec, wappextqVerbs, &pwzData);
        ExitOnFailure1(hr, "Failed to get Verbs for Application: '%S'", wzApplication);
        hr = ::StringCchCopyW(pswappext->wzVerbs, countof(pswappext->wzVerbs), pwzData);
        ExitOnFailure1(hr, "Failed to copy verb string: '%S'", pwzData);

        // extension executeable
        hr = WcaGetRecordFormattedString(hRec, wappextqExecutable, &pwzData);
        ExitOnFailure1(hr, "Failed to get Executable for Application: '%S'", wzApplication);
        hr = ::StringCchCopyW(pswappext->wzExecutable, countof(pswappext->wzExecutable), pwzData);
        ExitOnFailure1(hr, "Failed to copy executable string: '%S'", pwzData);

        hr = WcaGetRecordInteger(hRec, wappextqAttributes, &pswappext->iAttributes);
        if (S_FALSE == hr)
        {
            pswappext->iAttributes = 0;
            hr = S_OK;
        }
        ExitOnFailure(hr, "Failed to get App isolation");

        *ppswappextList = AddAppExtToList7(*ppswappextList, pswappext);
        pswappext = NULL; // set the appext NULL so it doesn't accidentally get freed below
    }

    if (E_NOMOREITEMS == hr)
    {
        hr = S_OK;
    }

LExit:
    // if anything was left over after an error clean it all up
    if (pswappext)
    {
        ScaWebAppExtensionsFreeList(pswappext);
    }

    ReleaseStr(pwzData);

    return hr;
}



HRESULT ScaWebAppExtensionsWrite7(
    __in_z LPCWSTR wzWebName,
    __in_z LPCWSTR wzRootOfWeb,
    __in SCA_WEB_APPLICATION_EXTENSION* pswappextList
    )
{
    HRESULT hr = S_OK;
    SCA_WEB_APPLICATION_EXTENSION* pswappext = NULL;

    if (!pswappextList)
    {
        ExitFunction1(hr = S_OK);
    }

    //create the Extension for this vdir application
    //all go to same web/root location tag
    hr = ScaWriteConfigID(IIS_APPEXT_BEGIN);
    ExitOnFailure(hr, "Failed to write webappext begin id");
    hr = ScaWriteConfigString(wzWebName);                //site name key
    ExitOnFailure(hr, "Failed to write app web key");
    hr = ScaWriteConfigString(wzRootOfWeb);               //app path key
    ExitOnFailure(hr, "Failed to write app web key");

    pswappext = pswappextList;

    while (pswappext)
    {
        //create the Extension for this vdir application
        hr = ScaWriteConfigID(IIS_APPEXT);
        ExitOnFailure(hr, "Failed to write webappext begin id");

        if (*pswappext->wzExtension)
        {
            hr = ScaWriteConfigString(pswappext->wzExtension);
        }
        else   // blank means "*" (all)
        {
            hr = ScaWriteConfigString(L"*");
        }
        ExitOnFailure(hr, "Failed to write extension");

        hr = ScaWriteConfigString(pswappext->wzExecutable);
        ExitOnFailure(hr, "Failed to write extension executable");

        hr = ScaWriteConfigString(pswappext->wzVerbs);
        ExitOnFailure(hr, "Failed to write extension verbs");

        pswappext = pswappext->pswappextNext;
    }

    hr = ScaWriteConfigID(IIS_APPEXT_END);
    ExitOnFailure(hr, "Failed to write webappext begin id");

LExit:
    return hr;
}


void ScaWebAppExtensionsFreeList7(
    __in SCA_WEB_APPLICATION_EXTENSION* pswappextList
    )
{
    SCA_WEB_APPLICATION_EXTENSION* pswappextDelete = pswappextList;
    while (pswappextList)
    {
        pswappextDelete = pswappextList;
        pswappextList = pswappextList->pswappextNext;

        MemFree(pswappextDelete);
    }
}



// private helper functions

static HRESULT NewAppExt7(
    __out SCA_WEB_APPLICATION_EXTENSION** ppswappext
    )
{
    HRESULT hr = S_OK;

    SCA_WEB_APPLICATION_EXTENSION* pswappext = (SCA_WEB_APPLICATION_EXTENSION*)MemAlloc(sizeof(SCA_WEB_APPLICATION_EXTENSION), TRUE);
    ExitOnNull(pswappext, hr, E_OUTOFMEMORY, "failed to allocate memory for new web app ext element");

    *ppswappext = pswappext;

LExit:
    return hr;
}


static SCA_WEB_APPLICATION_EXTENSION* AddAppExtToList7(
    __in SCA_WEB_APPLICATION_EXTENSION* pswappextList,
    __in SCA_WEB_APPLICATION_EXTENSION* pswappext
    )
{
    if (pswappextList)
    {
        SCA_WEB_APPLICATION_EXTENSION* pswappextT = pswappextList;
        while (pswappextT->pswappextNext)
        {
            pswappextT = pswappextT->pswappextNext;
        }

        pswappextT->pswappextNext = pswappext;
    }
    else
    {
        pswappextList = pswappext;
    }

    return pswappextList;
}
