//-------------------------------------------------------------------------------------------------
// <copyright file="scaproperty7.cpp" company="Microsoft">
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
//    IIS Property functions for CustomActions
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"

/*------------------------------------------------------------------
IIsProperty table:

Property  Component_  Attributes  Value
s72      s72         i4          s255
------------------------------------------------------------------*/

// sql queries
LPCWSTR vcsPropertyQuery7 = L"SELECT `Property`, `Component_`, `Attributes`, `Value` "
                         L"FROM `IIsProperty`";

enum ePropertyQuery { pqProperty = 1, pqComponent, pqAttributes, pqValue, pqInstalled, pqAction };


// prototypes
static HRESULT AddPropertyToList(
    SCA_PROPERTY** ppspList
    );


// functions
void ScaPropertyFreeList7(
    SCA_PROPERTY* pspList
    )
{
    SCA_PROPERTY* pspDelete = pspList;
    while (pspList)
    {
        pspDelete = pspList;
        pspList = pspList->pspNext;

        MemFree(pspDelete);
    }
}


HRESULT ScaPropertyRead7(
    SCA_PROPERTY** ppspList
    )
{
    HRESULT hr = S_OK;
    UINT er = ERROR_SUCCESS;
    PMSIHANDLE hView, hRec;
    INSTALLSTATE isInstalled = INSTALLSTATE_UNKNOWN;
    INSTALLSTATE isAction = INSTALLSTATE_UNKNOWN;    

    LPWSTR pwzData = NULL;
    SCA_PROPERTY* pss;

    ExitOnNull(ppspList, hr, E_INVALIDARG, "Failed to read property, because no property to read was provided");

    if (S_OK != WcaTableExists(L"IIsProperty"))
    {
        WcaLog(LOGMSG_VERBOSE, "Skipping ScaPropertyRead7() - because IIsProperty table not present.");
        ExitFunction1(hr = S_FALSE);
    }

    // loop through all the Settings
    hr = WcaOpenExecuteView(vcsPropertyQuery7, &hView);
    ExitOnFailure(hr, "failed to open view on IIsProperty table");
    while (S_OK == (hr = WcaFetchRecord(hView, &hRec)))
    {
        hr = WcaGetRecordString(hRec, pqComponent, &pwzData);
        ExitOnFailure(hr, "failed to get IIsProperty.Component");
        
        er = ::MsiGetComponentStateW(WcaGetInstallHandle(), pwzData, &isInstalled, &isAction);
        hr = HRESULT_FROM_WIN32(er);
        ExitOnFailure(hr, "Failed to get Component state for WebSvcExt");    

        hr = AddPropertyToList(ppspList);
        ExitOnFailure(hr, "failed to add property to list");

        pss = *ppspList;

        hr = ::StringCchCopyW(pss->wzComponent, countof(pss->wzComponent), pwzData);
        ExitOnFailure1(hr, "failed to copy component name: %S", pwzData);

        pss->isInstalled = isInstalled;
        pss->isAction = isAction;

        hr = WcaGetRecordString(hRec, pqProperty, &pwzData);
        ExitOnFailure(hr, "failed to get IIsProperty.Property");
        hr = ::StringCchCopyW(pss->wzProperty, countof(pss->wzProperty), pwzData);
        ExitOnFailure1(hr, "failed to copy Property name: %S", pwzData);

        hr = WcaGetRecordFormattedString(hRec, pqValue, &pwzData);
        ExitOnFailure(hr, "failed to get IIsProperty.Value");
        hr = ::StringCchCopyW(pss->wzValue, countof(pss->wzValue), pwzData);
        ExitOnFailure1(hr, "failed to copy Property value: %S", pwzData);

        hr = WcaGetRecordInteger(hRec, pqAttributes, &pss->iAttributes);
        ExitOnFailure(hr, "failed to get IIsProperty.Attributes");

    }

    if (E_NOMOREITEMS == hr)
    {
        hr = S_OK;
    }
    ExitOnFailure(hr, "failure while processing IIsProperty table");

LExit:
    ReleaseStr(pwzData);

    return hr;
}


HRESULT ScaPropertyInstall7(
    SCA_PROPERTY* pspList
    )
{
    HRESULT hr = S_OK;

    for (SCA_PROPERTY* psp = pspList; psp; psp = psp->pspNext)
    {
        // if we are installing the web site
        if (WcaIsInstalling(psp->isInstalled, psp->isAction))
        {
            hr = ScaWriteProperty7(psp);
            ExitOnFailure1(hr, "failed to write Property '%S' ", psp->wzProperty);
        }
    }

LExit:
    return hr;
}


HRESULT ScaPropertyUninstall7(
    SCA_PROPERTY* pspList
    )
{
    HRESULT hr = S_OK;

    for (SCA_PROPERTY* psp = pspList; psp; psp = psp->pspNext)
    {
        // if we are uninstalling the web site
        if (WcaIsUninstalling(psp->isInstalled, psp->isAction))
        {
            hr = ScaRemoveProperty7(psp);
            ExitOnFailure1(hr, "Failed to remove Property '%S'", psp->wzProperty);
        }
    }

LExit:
    return hr;
}


HRESULT ScaWriteProperty7(
    const SCA_PROPERTY* psp
    )
{
    HRESULT hr = S_OK;
    DWORD dwValue;
    LPWSTR wz = NULL;

    ExitOnNull(psp, hr, E_INVALIDARG, "Failed to write property because no property to write was given");
    //
    // Figure out what setting we're writing and write it
    //
    if (0 == wcscmp(psp->wzProperty, wzIISPROPERTY_IIS5_ISOLATION_MODE))
    {
        // IIs5IsolationMode not supported
        WcaLog(LOGMSG_VERBOSE, "Not supported by IIS7: IIs5IsolationMode, ignoring");
    }
    else if (0 == wcscmp(psp->wzProperty, wzIISPROPERTY_MAX_GLOBAL_BANDWIDTH))
    {
        dwValue = wcstoul(psp->wzValue, &wz, 10) * 1024; // remember, the value shown is in kilobytes, the value saved is in bytes
        hr = ScaWriteConfigID(IIS_PROPERTY);
        ExitOnFailure(hr, "failed to set Property ID");
        hr = ScaWriteConfigID(IIS_PROPERTY_MAXBAND);
        ExitOnFailure(hr, "failed to set Property MSXBAND ID");
        hr = ScaWriteConfigInteger(dwValue);
        ExitOnFailure(hr, "failed to set Property MSXBAND value");
    }
    else if (0 == wcscmp(psp->wzProperty, wzIISPROPERTY_LOG_IN_UTF8))
    {
        dwValue = 1;
        hr = ScaWriteConfigID(IIS_PROPERTY);
        ExitOnFailure(hr, "failed to set Property ID");
        hr = ScaWriteConfigID(IIS_PROPERTY_LOGUTF8);
        ExitOnFailure(hr, "failed to set Property LOG ID");
        hr = ScaWriteConfigInteger(dwValue);
        ExitOnFailure(hr, "failed to set Property Log value");
    }
    else if (0 == wcscmp(psp->wzProperty, wzIISPROPERTY_ETAG_CHANGENUMBER))
    {
        //EtagChangenumber not supported 
        WcaLog(LOGMSG_VERBOSE, "Not supported by IIS7: EtagChangenumber, ignoring");
    }

LExit:
    return hr;
}

HRESULT ScaRemoveProperty7(
    __in SCA_PROPERTY* psp
    )
{

    // NOP function for now
    //The two global values being set by WebProperty:
    //    <iis:WebProperty Id="MaxGlobalBandwidth" Value="1024" />
    //    <iis:WebProperty Id ="LogInUTF8" />
    // should should not be removed on uninstall.

    HRESULT hr = S_OK;

    return hr;
}

static HRESULT AddPropertyToList(
    SCA_PROPERTY** ppspList
    )
{
    HRESULT hr = S_OK;
    SCA_PROPERTY* psp = static_cast<SCA_PROPERTY*>(MemAlloc(sizeof(SCA_PROPERTY), TRUE));
    ExitOnNull(psp, hr, E_OUTOFMEMORY, "failed to allocate memory for new property list element");
    
    psp->pspNext = *ppspList;
    *ppspList = psp;
    
LExit:
    return hr;
}
