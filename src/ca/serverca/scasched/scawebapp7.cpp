//-------------------------------------------------------------------------------------------------
// <copyright file="scawebapp7.cpp" company="Microsoft">
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
//    IIS Web Application functions for CustomActions
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"

// sql queries
LPCWSTR vcsWebApplicationQuery7 = L"SELECT `Name`, `Isolation`, `AllowSessions`, `SessionTimeout`, "
                                  L"`Buffer`, `ParentPaths`, `DefaultScript`, `ScriptTimeout`, "
                                  L"`ServerDebugging`, `ClientDebugging`, `AppPool_` "
                                  L"FROM `IIsWebApplication` WHERE `Application`=?";
enum eWebApplicationQuery { wappqName = 1, wappqIsolation, wappqAllowSession,
                            wappqSessionTimeout, wappqBuffer, wappqParentPaths,
                            wappqDefaultScript, wappqScriptTimeout,
                            wappqServerDebugging, wappqClientDebugging, wappqAppPool};


HRESULT ScaGetWebApplication7(
    __in MSIHANDLE hViewApplications,
    __in_z LPCWSTR pwzApplication,
    SCA_WEB_APPLICATION* pswapp
    )
{
    HRESULT hr = S_OK;

    PMSIHANDLE hView, hRec;
    LPWSTR pwzData = NULL;

    hr = WcaTableExists(L"IIsWebApplication");
    if (S_FALSE == hr)
    {
        hr = E_ABORT;
    }
    ExitOnFailure(hr, "IIsWebApplication table does not exists or error");

    hRec = ::MsiCreateRecord(1);
    hr = WcaSetRecordString(hRec, 1, pwzApplication);
    ExitOnFailure(hr, "Failed to set record to look up Web Application");

    // if the view wasn't provided open one
    if (!hViewApplications)
    {
        hr = WcaOpenView(vcsWebApplicationQuery7, &hView);
        ExitOnFailure(hr, "Failed to open view on IIsWebApplication table");
    }
    else
    {
        hView = hViewApplications;
    }

    hr = WcaExecuteView(hView, hRec);
    ExitOnFailure1(hr, "Failed to execute view on IIsWebApplication table looking Application: %S", pwzApplication);

    // get the application information
    hr = WcaFetchSingleRecord(hView, &hRec);
    if (S_OK == hr)
    {
        // application name
        hr = WcaGetRecordFormattedString(hRec, wappqName, &pwzData);
        ExitOnFailure(hr, "Failed to get Name of App");
        hr = ::StringCchCopyW(pswapp->wzName, countof(pswapp->wzName), pwzData);
        ExitOnFailure1(hr, "Failed to copy name of app: '%S'", pwzData);

        hr = WcaGetRecordInteger(hRec, wappqIsolation, &pswapp->iIsolation);
        ExitOnFailure1(hr, "Failed to get App isolation: '%S'", pswapp->wzName);

        hr = WcaGetRecordInteger(hRec, wappqAllowSession, &pswapp->fAllowSessionState);
        ExitOnFailure1(hr, "Failed to get allow session state for App: '%S'", pswapp->wzName);

        hr = WcaGetRecordInteger(hRec, wappqSessionTimeout, &pswapp->iSessionTimeout);
        ExitOnFailure1(hr, "Failed to get session timeout for App: '%S'", pswapp->wzName);

        hr = WcaGetRecordInteger(hRec, wappqBuffer, &pswapp->fBuffer);
        ExitOnFailure1(hr, "Failed to get buffer for App: '%S'", pswapp->wzName);

        hr = WcaGetRecordInteger(hRec, wappqParentPaths, &pswapp->fParentPaths);
        ExitOnFailure1(hr, "Failed to get parent paths for App: '%S'", pswapp->wzName);

        hr = WcaGetRecordString(hRec, wappqDefaultScript, &pwzData);
        ExitOnFailure1(hr, "Failed to get default scripting language for App: '%S'", pswapp->wzName);
        hr = ::StringCchCopyW(pswapp->wzDefaultScript, countof(pswapp->wzDefaultScript), pwzData);
        ExitOnFailure1(hr, "Failed to copy default scripting language of App: '%S'", pwzData);

        // asp script timeout
        hr = WcaGetRecordInteger(hRec, wappqScriptTimeout, &pswapp->iScriptTimeout);
        ExitOnFailure1(hr, "Failed to get scripting timeout for App: '%S'", pswapp->wzName);

        // asp server-side script debugging
        hr = WcaGetRecordInteger(hRec, wappqServerDebugging, &pswapp->fServerDebugging);
        ExitOnFailure1(hr, "Failed to server debugging value for App: '%S'", pswapp->wzName);

        // asp client-side script debugging
        hr = WcaGetRecordInteger(hRec, wappqClientDebugging, &pswapp->fClientDebugging);
        ExitOnFailure1(hr, "Failed to client debugging value for App: '%S'", pswapp->wzName);

        hr = WcaGetRecordString(hRec, wappqAppPool, &pwzData);
        ExitOnFailure1(hr, "Failed to get AppPool for App: '%S'", pswapp->wzName);
        hr = ::StringCchCopyW(pswapp->wzAppPool, countof(pswapp->wzAppPool), pwzData);
        ExitOnFailure2(hr, "failed to copy AppPool: '%S' for App: '%S'", pwzData, pswapp->wzName);

        // app extensions
        hr = ScaWebAppExtensionsRead7(pwzApplication, &pswapp->pswappextList);
        ExitOnFailure1(hr, "Failed to read AppExtensions for App: '%S'", pswapp->wzName);

        hr = S_OK;
    }
    else if (E_NOMOREITEMS == hr)
    {
        WcaLog(LOGMSG_STANDARD, "Error: Cannot locate IIsWebApplication.Application='%S'", pwzApplication);
        hr = E_FAIL;
    }
    else
    {
        ExitOnFailure(hr, "Error or found multiple matching Application rows");
    }

LExit:
    ReleaseStr(pwzData);

    return hr;
}


HRESULT ScaWriteWebApplication7(
    __in_z LPCWSTR wzWebName,
    __in_z LPCWSTR wzRootOfWeb,
    SCA_WEB_APPLICATION* pswapp,
    SCA_APPPOOL * psapList
    )
{
    HRESULT hr = S_OK;

    //all go to same web/root location tag
    hr = ScaWriteConfigID(IIS_ASP_BEGIN);
    ExitOnFailure(hr, "Failed to write WebApp ASP begin id");
    hr = ScaWriteConfigString(wzWebName);                //site name key
    ExitOnFailure(hr, "Failed to write app web key");
    hr = ScaWriteConfigString(wzRootOfWeb);               //app path key
    ExitOnFailure(hr, "Failed to write app web root");

    // IIS7 Not Supported: Isolation
    if (MSI_NULL_INTEGER != pswapp->iIsolation)
    {
        WcaLog(LOGMSG_TRACEONLY, "Not supported by IIS7: Isolation Mode, ignoring");
    }

    // allow session state
    if (MSI_NULL_INTEGER != pswapp->fAllowSessionState)
    {
        //system.webServer/asp /session | allowSessionState
        hr = ScaWriteConfigID(IIS_ASP_SESSIONSTATE);
        ExitOnFailure(hr, "Failed to write WebApp ASP sessionstate id");
        hr = ScaWriteConfigInteger(pswapp->fAllowSessionState);
        ExitOnFailure1(hr, "Failed to write allow session information for App: '%S'", pswapp->wzName);
    }

    // session timeout
    if (MSI_NULL_INTEGER != pswapp->iSessionTimeout)
    {
        //system.webServer/asp /session | timeout
        hr = ScaWriteConfigID(IIS_ASP_SESSIONTIMEOUT);
        ExitOnFailure(hr, "Failed to write WebApp ASP sessiontimepot id");
        hr = ScaWriteConfigInteger(pswapp->iSessionTimeout);
        ExitOnFailure1(hr, "Failed to write session timeout for App: '%S'", pswapp->wzName);
    }

    // asp buffering
    if (MSI_NULL_INTEGER != pswapp->fBuffer)
    {
        //system.webServer/asp | bufferingOn
        hr = ScaWriteConfigID(IIS_ASP_BUFFER);
        ExitOnFailure(hr, "Failed to write WebApp ASP buffer id");
        hr = ScaWriteConfigInteger(pswapp->fBuffer);
        ExitOnFailure1(hr, "Failed to write buffering flag for App: '%S'", pswapp->wzName);
    }

    // asp parent paths
    if (MSI_NULL_INTEGER != pswapp->fParentPaths)
    {
        //system.webServer/asp | enableParentPaths
        hr = ScaWriteConfigID(IIS_ASP_PARENTPATHS);
        ExitOnFailure(hr, "Failed to write WebApp ASP parentpaths id");
        hr = ScaWriteConfigInteger(pswapp->fParentPaths);
        ExitOnFailure1(hr, "Failed to write parent paths flag for App: '%S'", pswapp->wzName);
    }

    // default scripting language
    if (*pswapp->wzDefaultScript)
    {
        //system.webServer/asp | scriptLanguage
        hr = ScaWriteConfigID(IIS_ASP_SCRIPTLANG);
        ExitOnFailure(hr, "Failed to write WebApp ASP script lang id");
        hr = ScaWriteConfigString(pswapp->wzDefaultScript);
        ExitOnFailure1(hr, "Failed to write default scripting language for App: '%S'", pswapp->wzName);
    }

    // asp script timeout
    if (MSI_NULL_INTEGER != pswapp->iScriptTimeout)
    {
        //system.webServer/asp /limits | scriptTimeout
        hr = ScaWriteConfigID(IIS_ASP_SCRIPTTIMEOUT);
        ExitOnFailure(hr, "Failed to write WebApp ASP script timeout id");
        hr = ScaWriteConfigInteger(pswapp->iScriptTimeout);
        ExitOnFailure1(hr, "Failed to write script timeout for App: '%S'", pswapp->wzName);
    }

    // asp server-side script debugging
    if (MSI_NULL_INTEGER != pswapp->fServerDebugging)
    {
        //system.webServer/asp | appAllowDebugging
        hr = ScaWriteConfigID(IIS_ASP_SCRIPTSERVERDEBUG);
        ExitOnFailure(hr, "Failed to write WebApp ASP script debug id");
        hr = ScaWriteConfigInteger(pswapp->fServerDebugging);
        ExitOnFailure1(hr, "Failed to write ASP server-side script debugging flag for App: '%S'", pswapp->wzName);
    }

    // asp client-side script debugging
    if (MSI_NULL_INTEGER != pswapp->fClientDebugging)
    {
        //system.webServer/asp | appAllowClientDebug
        hr = ScaWriteConfigID(IIS_ASP_SCRIPTCLIENTDEBUG);
        ExitOnFailure(hr, "Failed to write WebApp ASP script debug id");
        hr = ScaWriteConfigInteger(pswapp->fClientDebugging);
        ExitOnFailure1(hr, "Failed to write ASP client-side script debugging flag for App: '%S'", pswapp->wzName);
    }

    //done with ASP application properties
    hr = ScaWriteConfigID(IIS_ASP_END);
    ExitOnFailure(hr, "Failed to write WebApp ASP begin id");

    //write out app estensions
    if (pswapp->pswappextList)
    {
        hr = ScaWebAppExtensionsWrite7(wzWebName, wzRootOfWeb, pswapp->pswappextList);
        ExitOnFailure1(hr, "Failed to write AppExtensions for App: '%S'", pswapp->wzName);
    }

LExit:
    return hr;
}
