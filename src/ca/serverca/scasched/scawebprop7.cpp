//-------------------------------------------------------------------------------------------------
// <copyright file="scawebprop7.cpp" company="Microsoft">
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
//    Web directory property functions for CustomActions
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"

// sql queries
LPCWSTR vcsWebDirPropertiesQuery7 = L"SELECT `DirProperties`, `Access`, `Authorization`, `AnonymousUser_`, `IIsControlledPassword`, `LogVisits`, `Index`, `DefaultDoc`, `AspDetailedError`, `HttpExpires`, `CacheControlMaxAge`, `CacheControlCustom`, `NoCustomError`, `AccessSSLFlags`, `AuthenticationProviders`"
                                   L"FROM `IIsWebDirProperties` WHERE `DirProperties`=?";

enum eWebDirPropertiesQuery { wpqProperties = 1, wpqAccess, wpqAuthorization, wpqUser, wpqControlledPassword, wpqLogVisits, wpqIndex, wpqDefaultDoc,  wpqAspDetailedError, wpqHttpExp, wpqCCMaxAge, wpqCCCustom, wpqNoCustomError, wpqAccessSSLFlags, wpqAuthenticationProviders };

HRESULT ScaGetWebDirProperties7(
    __in_z LPCWSTR wzProperties,
    __inout SCA_WEB_PROPERTIES* pswp
    )
{
    Assert(*wzProperties && pswp);

    HRESULT hr = S_OK;
    PMSIHANDLE hView, hRec;
    LPWSTR pwzData = NULL;

    hr = WcaTableExists(L"IIsWebDirProperties");
    if (S_FALSE == hr)
    {
        hr = E_ABORT;
    }
    ExitOnFailure(hr, "IIsWebDirProperties table does not exists or error");

    hRec = ::MsiCreateRecord(1);
    hr = WcaSetRecordString(hRec, 1, wzProperties);
    ExitOnFailure(hr, "Failed to look up Web DirProperties");

    hr = WcaOpenView(vcsWebDirPropertiesQuery7, &hView);
    ExitOnFailure(hr, "Failed to open view on WebDirProperties");
    hr = WcaExecuteView(hView, hRec);
    ExitOnFailure(hr, "Failed to exectue view on WebDirProperties");

    hr = WcaFetchSingleRecord(hView, &hRec);
    if (S_OK == hr)
    {
        hr = WcaGetRecordString(hRec, wpqProperties, &pwzData);
        ExitOnFailure(hr, "Failed to get IIsWebDirProperties.DirProperties");
        hr = ::StringCchCopyW(pswp->wzKey, countof(pswp->wzKey), pwzData);
        ExitOnFailure(hr, "Failed to copy key string to webdirproperties object");

        Assert(0 == lstrcmpW(pswp->wzKey, wzProperties));

        hr = WcaGetRecordInteger(hRec, wpqAccess, &pswp->iAccess);
        ExitOnFailure(hr, "Failed to get access value");

        hr = WcaGetRecordInteger(hRec, wpqAuthorization, &pswp->iAuthorization);
        ExitOnFailure(hr, "Failed to get authorization value");

        // if allow anonymous users
        if (S_OK == hr && pswp->iAuthorization & 1)
        {
            // if there is an anonymous user specified
            hr = WcaGetRecordString(hRec, wpqUser, &pwzData);
            ExitOnFailure(hr, "Failed to get AnonymousUser_");
            if (pwzData && *pwzData)
            {
                hr = WcaGetRecordInteger(hRec, wpqControlledPassword, &pswp->fIIsControlledPassword);
                ExitOnFailure(hr, "Failed to get IIsControlledPassword");
                if (S_FALSE == hr)
                {
                    pswp->fIIsControlledPassword = FALSE;
                    hr = S_OK;
                }

                hr = ScaGetUser(pwzData, &pswp->scau);
                ExitOnFailure(hr, "Failed to get User information for Web");

                pswp->fHasUser = TRUE;
            }
            else
                pswp->fHasUser = FALSE;
        }

        hr = WcaGetRecordInteger(hRec, wpqLogVisits, &pswp->fLogVisits);
        ExitOnFailure(hr, "Failed to get IIsWebDirProperties.LogVisits");

        hr = WcaGetRecordInteger(hRec, wpqIndex, &pswp->fIndex);
        ExitOnFailure(hr, "Failed to get IIsWebDirProperties.Index");

        hr = WcaGetRecordFormattedString(hRec, wpqDefaultDoc, &pwzData);
        ExitOnFailure(hr, "Failed to get IIsWebDirProperties.DefaultDoc");
        if (pwzData && *pwzData)
        {
            pswp->fHasDefaultDoc = TRUE;
            if (0 == lstrcmpW(L"-", pwzData))   // remove any existing default documents by setting them blank
            {
                pswp->wzDefaultDoc[0] = L'\0';
            }
            else   // set the default documents
            {
                hr = ::StringCchCopyW(pswp->wzDefaultDoc, countof(pswp->wzDefaultDoc), pwzData);
                ExitOnFailure(hr, "Failed to copy default document string to webdirproperties object");
            }
        }
        else
        {
            pswp->fHasDefaultDoc = FALSE;
        }

        hr = WcaGetRecordInteger(hRec, wpqAspDetailedError, &pswp->fAspDetailedError);
        ExitOnFailure(hr, "Failed to get IIsWebDirProperties.AspDetailedError");

        hr = WcaGetRecordFormattedString(hRec, wpqHttpExp, &pwzData);
        ExitOnFailure(hr, "Failed to get IIsWebDirProperties.HttpExp");
        if (pwzData && *pwzData)
        {
            pswp->fHasHttpExp = TRUE;
            if (0 == lstrcmpW(L"-", pwzData))   // remove any existing default expiration settings by setting them blank
            {
                pswp->wzHttpExp[0] = L'\0';
            }
            else   // set the expiration setting
            {
                hr = ::StringCchCopyW(pswp->wzHttpExp, countof(pswp->wzHttpExp), pwzData);
                ExitOnFailure(hr, "Failed to copy http expiration string to webdirproperties object");
            }
        }
        else
        {
            pswp->fHasHttpExp = FALSE;
        }

        hr = WcaGetRecordInteger(hRec, wpqCCMaxAge, &pswp->iCacheControlMaxAge);
        ExitOnFailure(hr, "failed to get IIsWebDirProperties.CacheControlMaxAge");

        hr = WcaGetRecordFormattedString(hRec, wpqCCCustom, &pwzData);
        ExitOnFailure(hr, "Failed to get IIsWebDirProperties.CacheControlCustom");
        if (pwzData && *pwzData)
        {
            pswp->fHasCacheControlCustom = TRUE;
            if (0 == lstrcmpW(L"-", pwzData))   // remove any existing default cache control custom settings by setting them blank
            {
                pswp->wzCacheControlCustom[0] = L'\0';
            }
            else   // set the custom cache control setting
            {
                hr = ::StringCchCopyW(pswp->wzCacheControlCustom, countof(pswp->wzCacheControlCustom), pwzData);
                ExitOnFailure(hr, "Failed to copy cache control custom settings to webdirproperites object");
            }
        }
        else
        {
            pswp->fHasCacheControlCustom = FALSE;
        }

        hr = WcaGetRecordInteger(hRec, wpqNoCustomError, &pswp->fNoCustomError);
        ExitOnFailure(hr, "failed to get IIsWebDirProperties.NoCustomError");
        if (MSI_NULL_INTEGER == pswp->fNoCustomError)
            pswp->fNoCustomError = FALSE;

        hr = WcaGetRecordInteger(hRec, wpqAccessSSLFlags, &pswp->iAccessSSLFlags);
        ExitOnFailure(hr, "failed to get IIsWebDirProperties.AccessSSLFlags");

        hr = WcaGetRecordFormattedString(hRec, wpqAuthenticationProviders, &pwzData);
        ExitOnFailure(hr, "Failed to get IIsWebDirProperties.AuthenticationProviders");
        if(pwzData && *pwzData)
        {
            hr = ::StringCchCopyW(pswp->wzAuthenticationProviders, countof(pswp->wzAuthenticationProviders), pwzData);
            ExitOnFailure(hr, "Failed to copy authentication providers string to webdirproperties object");
        }
        else
        {
            pswp->wzAuthenticationProviders[0] = L'\0';
        }
    }
    else if (E_NOMOREITEMS == hr)
    {
        WcaLog(LOGMSG_STANDARD, "Error: Cannot locate IIsWebDirProperties.DirProperties='%S'", wzProperties);
        hr = E_FAIL;
    }
    else
    {
        ExitOnFailure(hr, "Error getting appropriate webdirproperty");
    }

    //// Let's check that there isn't more than one record found - if there is, throw an assert like WcaFetchSingleRecord() would
    //HRESULT hrTemp = WcaFetchSingleRecord(hView, &hRec);
    //if (SUCCEEDED(hrTemp))
    //{
    //    AssertSz(E_NOMOREITEMS == hrTemp, "ScaGetWebDirProperties found more than one record");
    //}

LExit:
    ReleaseStr(pwzData);

    return hr;
}


HRESULT ScaWriteWebDirProperties7(
    __in_z LPCWSTR wzWebName,
    __in_z LPCWSTR wzRootOfWeb,
    __in const SCA_WEB_PROPERTIES* pswp
    )
{
    HRESULT hr = S_OK;
    DWORD dw = 0;
    WCHAR wz[METADATA_MAX_NAME_LEN + 1];

    //all go to same web/root location tag
    hr = ScaWriteConfigID(IIS_DIRPROP_BEGIN);
    ExitOnFailure(hr, "Failed to write DirProp begin id");
    hr = ScaWriteConfigString(wzWebName);                //site name key
    ExitOnFailure(hr, "Failed to write DirProp web key");
    hr = ScaWriteConfigString(wzRootOfWeb);               //app path key
    ExitOnFailure(hr, "Failed to write DirProp app key");

    // write the access permissions to the metabase
    if (MSI_NULL_INTEGER != pswp->iAccess)
    {
        hr = ScaWriteConfigID(IIS_DIRPROP_ACCESS);
        ExitOnFailure(hr, "Failed to write DirProp access id");
        hr = ScaWriteConfigInteger(pswp->iAccess);
        ExitOnFailure(hr, "Failed to write access permissions for Web");
    }

    if (MSI_NULL_INTEGER != pswp->iAuthorization)
    {
        hr = ScaWriteConfigID(IIS_DIRPROP_AUTH);
        ExitOnFailure(hr, "Failed to write DirProp auth id");
        hr = ScaWriteConfigInteger(pswp->iAuthorization);
        ExitOnFailure(hr, "Failed to write authorization for Web");
    }

    if (pswp->fHasUser)
    {
        Assert(pswp->scau.wzName);
        // write the user name
        if (*pswp->scau.wzDomain)
        {
            hr = ::StringCchPrintfW(wz, countof(wz), L"%s\\%s", pswp->scau.wzDomain, pswp->scau.wzName);
            ExitOnFailure(hr, "Failed to format domain\\username string");
        }
        else
        {
#pragma prefast(suppress:26037, "Source string is null terminated - it is populated as target of ::StringCchCopyW")
            hr = ::StringCchCopyW(wz, countof(wz), pswp->scau.wzName);
            ExitOnFailure(hr, "Failed to copy user name");
        }
        hr = ScaWriteConfigID(IIS_DIRPROP_USER);
        ExitOnFailure(hr, "Failed to write DirProp user id");
        hr = ScaWriteConfigString(wz);
        ExitOnFailure(hr, "Failed to write anonymous user name for Web");

        // write the password
        hr = ScaWriteConfigID(IIS_DIRPROP_PWD);
        ExitOnFailure(hr, "Failed to write DirProp pwd id");
        hr = ScaWriteConfigString(pswp->scau.wzPassword);
        ExitOnFailure(hr, "Failed to write anonymous user password for Web");

        if(pswp->fIIsControlledPassword)
        {
            //Not Supported by IIS7 : pswp->fIIsControlledPassword
            WcaLog(LOGMSG_VERBOSE, "Not supported by IIS7: WebDirProperties.IIsControlledPassword, ignoring");
        }
    }

    if (MSI_NULL_INTEGER != pswp->fLogVisits)
    {
        //Not Supported by IIS7 : pswp->fIIsControlledPassword
        WcaLog(LOGMSG_VERBOSE, "Not supported by IIS7: WebDirProperties.LogVisits, ignoring");
    }

    if (MSI_NULL_INTEGER != pswp->fIndex)
    {
        //Not Supported by IIS7 : pswp->fIndex
        WcaLog(LOGMSG_VERBOSE, "Not supported by IIS7: WebDirProperties.Index, ignoring");
    }

    if (pswp->fHasDefaultDoc)
    {
        hr = ScaWriteConfigID(IIS_DIRPROP_DEFDOCS);
        ExitOnFailure(hr, "Failed to write DirProp defdocs id");
        hr = ScaWriteConfigString(pswp->wzDefaultDoc);
        ExitOnFailure(hr, "Failed to write default documents for Web");
    }

    if (MSI_NULL_INTEGER != pswp->fAspDetailedError)
    {
        hr = ScaWriteConfigID(IIS_DIRPROP_ASPERROR);
        ExitOnFailure(hr, "Failed to write ASP script error id");
        hr = ScaWriteConfigInteger(pswp->fAspDetailedError);
        ExitOnFailure(hr, "Failed to write ASP script error for Web");
    }

    if (pswp->fHasHttpExp)
    {
        hr = ScaWriteConfigID(IIS_DIRPROP_HTTPEXPIRES);
        ExitOnFailure(hr, "Failed to write DirProp HttpExpires id");
        hr = ScaWriteConfigString(pswp->wzHttpExp);
        ExitOnFailure(hr, "Failed to write DirProp HttpExpires value");
    }

    if (MSI_NULL_INTEGER != pswp->iCacheControlMaxAge)
    {
        hr = ScaWriteConfigID(IIS_DIRPROP_MAXAGE);
        ExitOnFailure(hr, "Failed to write DirProp MaxAge id");
        hr = ScaWriteConfigInteger(pswp->iCacheControlMaxAge);
        ExitOnFailure(hr, "Failed to write DirProp MaxAge value");
    }

    if (pswp->fHasCacheControlCustom)
    {
        hr = ScaWriteConfigID(IIS_DIRPROP_CACHECUST);
        ExitOnFailure(hr, "Failed to write DirProp Cache Control Custom id");
        hr = ScaWriteConfigString(pswp->wzCacheControlCustom);
        ExitOnFailure(hr, "Failed to write Cache Control Custom for Web");
    }

    if (pswp->fNoCustomError)
    {
        hr = ScaWriteConfigID(IIS_DIRPROP_NOCUSTERROR);
        ExitOnFailure(hr, "Failed to write DirProp clear Cust Errors id");
    }

    if (MSI_NULL_INTEGER != pswp->iAccessSSLFlags)
    {
        hr = ScaWriteConfigID(IIS_DIRPROP_SSLFLAGS);
        ExitOnFailure(hr, "Failed to write DirProp sslFlags id");
        hr = ScaWriteConfigInteger(pswp->iAccessSSLFlags);
        ExitOnFailure(hr, "Failed to write AccessSSLFlags for Web");
    }

    if (*pswp->wzAuthenticationProviders)
    {
        hr = ::StringCchCopyW(wz, countof(wz), pswp->wzAuthenticationProviders);
        ExitOnFailure(hr, "Failed to copy authentication providers string");
        hr = ScaWriteConfigID(IIS_DIRPROP_AUTHPROVID);
        ExitOnFailure(hr, "Failed to write DirProp AuthProvid id");
        hr = ScaWriteConfigString(wz);
        ExitOnFailure(hr, "Failed to write AuthenticationProviders for Web");
    }
    //End of Dir Properties
    hr = ScaWriteConfigID(IIS_DIRPROP_END);
    ExitOnFailure(hr, "Failed to write DirProp end id");

LExit:
    return hr;
}
