#pragma once
//-------------------------------------------------------------------------------------------------
// <copyright file="scaexecIIS7.cpp" company="Microsoft">
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
//    execution CustomActions for IIS7
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"

// IIS Config schema names
#define IIS_CONFIG_ADD                      L"add"
#define IIS_CONFIG_ALLOWED                  L"allowed"
#define IIS_CONFIG_APPHOST_ROOT             L"MACHINE/WEBROOT/APPHOST"
#define IIS_CONFIG_APPLICATION              L"application"
#define IIS_CONFIG_APPPOOL                  L"applicationPool"
#define IIS_CONFIG_APPPOOL_AUTO             L"autoStart"
#define IIS_CONFIG_APPPOOL_SECTION          L"system.applicationHost/applicationPools"
#define IIS_CONFIG_AUTOSTART                L"serverAutoStart"
#define IIS_CONFIG_BINDING                  L"binding"
#define IIS_CONFIG_BINDINGINFO              L"bindingInformation"
#define IIS_CONFIG_BINDINGS                 L"bindings"
#define IIS_CONFIG_DESC                     L"description"
#define IIS_CONFIG_EXECUTABLE               L"scriptProcessor"
#define IIS_CONFIG_ENABLED                  L"enabled"
#define IIS_CONFIG_FILEEXT                  L"fileExtension"
#define IIS_CONFIG_FILTER                   L"filter"
#define IIS_CONFIG_GROUPID                  L"groupId"
#define IIS_CONFIG_HEADERS                  L"customHeaders"
#define IIS_CONFIG_HTTPERRORS_SECTION       L"system.webServer/httpErrors"
#define IIS_CONFIG_ID                       L"id"
#define IIS_CONFIG_ISAPI_SECTION            L"system.webServer/isapiFilters"
#define IIS_CONFIG_HTTPPROTO_SECTION        L"system.webServer/httpProtocol"
#define IIS_CONFIG_LOG_SECTION              L"system.applicationHost/log"
#define IIS_CONFIG_LOG_UTF8                 L"logInUTF8"
#define IIS_CONFIG_LIMITS                   L"limits"
#define IIS_CONFIG_PIPELINEMODE             L"managedPipelineMode"
#define IIS_CONFIG_WEBLOG                   L"logFile"
#define IIS_CONFIG_LOGFORMAT                L"logFormat"
#define IIS_CONFIG_MIMEMAP                  L"mimeMap"
#define IIS_CONFIG_MIMETYPE                 L"mimeType"
#define IIS_CONFIG_MODULES                  L"modules"
#define IIS_CONFIG_NAME                     L"name"
#define IIS_CONFIG_PATH                     L"path"
#define IIS_CONFIG_PHYSPATH                 L"physicalPath"
#define IIS_CONFIG_PROTOCOL                 L"protocol"
#define IIS_CONFIG_RESTRICTION_SECTION      L"system.webServer/security/isapiCgiRestriction"
#define IIS_CONFIG_SITE                     L"site"
#define IIS_CONFIG_SITE_ID                  L"id"
#define IIS_CONFIG_SITES_SECTION            L"system.applicationHost/sites"
#define IIS_CONFIG_CONNECTTIMEOUT           L"connectionTimeout"
#define IIS_CONFIG_VDIR                     L"virtualDirectory"
#define IIS_CONFIG_VALUE                    L"value"
#define IIS_CONFIG_VERBS                    L"verb"
#define IIS_CONFIG_WEBLIMITS_SECTION        L"system.applicationHost/webLimits"
#define IIS_CONFIG_WEBLIMITS_MAXBAND        L"maxGlobalBandwidth"
#define IIS_CONFIG_TRUE                     L"true"
#define IIS_CONFIG_FALSE                    L"false"
#define IIS_CONFIG_ERROR                    L"error"
#define IIS_CONFIG_STATUSCODE               L"statusCode"
#define IIS_CONFIG_SUBSTATUS                L"subStatusCode"
#define IIS_CONFIG_LANGPATH                 L"prefixLanguageFilePath"
#define IIS_CONFIG_RESPMODE                 L"responseMode"
#define IIS_CONFIG_CLEAR                    L"clear"
#define IIS_CONFIG_RECYCLING                L"recycling"
#define IIS_CONFIG_PEROIDRESTART            L"periodicRestart"
#define IIS_CONFIG_TIME                     L"time"
#define IIS_CONFIG_REQUESTS                 L"requests"
#define IIS_CONFIG_SCHEDULE                 L"schedule"
#define IIS_CONFIG_MEMORY                   L"memory"
#define IIS_CONFIG_PRIVMEMORY               L"privateMemory"
#define IIS_CONFIG_PROCESSMODEL             L"processModel"
#define IIS_CONFIG_IDLETIMEOUT              L"idleTimeout"
#define IIS_CONFIG_QUEUELENGTH              L"queueLength"
#define IIS_CONFIG_IDENITITYTYPE            L"identityType"
#define IIS_CONFIG_LOCALSYSTEM              L"LocalSystem"
#define IIS_CONFIG_LOCALSERVICE             L"LocalService"
#define IIS_CONFIG_NETWORKSERVICE           L"NetworkService"
#define IIS_CONFIG_SPECIFICUSER             L"SpecificUser"
#define IIS_CONFIG_USERNAME                 L"userName"
#define IIS_CONFIG_PASSWORD                 L"password"
#define IIS_CONFIG_CPU                      L"cpu"
#define IIS_CONFIG_LIMIT                    L"limit"
#define IIS_CONFIG_ACTION                   L"action"
#define IIS_CONFIG_KILLW3WP                 L"KillW3wp"
#define IIS_CONFIG_NOACTION                 L"NoAction"
#define IIS_CONFIG_RESETINTERVAL            L"resetInterval"
#define IIS_CONFIG_MAXWRKPROCESSES          L"maxProcesses"
#define IIS_CONFIG_HANDLERS_SECTION         L"system.webServer/handlers"
#define IIS_CONFIG_DEFAULTDOC_SECTION       L"system.webServer/defaultDocument"
#define IIS_CONFIG_ASP_SECTION              L"system.webServer/asp"
#define IIS_CONFIG_SCRIPTERROR              L"scriptErrorSentToBrowser"
#define IIS_CONFIG_STATICCONTENT_SECTION    L"system.webServer/staticContent"
#define IIS_CONFIG_HTTPEXPIRES              L"httpExpires"
#define IIS_CONFIG_MAXAGE                   L"cacheControlMaxAge"
#define IIS_CONFIG_CLIENTCACHE              L"clientCache"
#define IIS_CONFIG_CACHECUST                L"cacheControlCustom"
#define IIS_CONFIG_ASP_SECTION              L"system.webServer/asp"
#define IIS_CONFIG_SESSION                  L"session"
#define IIS_CONFIG_ALLOWSTATE               L"allowSessionState"
#define IIS_CONFIG_TIMEOUT                  L"timeout"
#define IIS_CONFIG_BUFFERING                L"bufferingOn"
#define IIS_CONFIG_PARENTPATHS              L"enableParentPaths"
#define IIS_CONFIG_SCRIPTLANG               L"scriptLanguage"
#define IIS_CONFIG_SCRIPTTIMEOUT            L"scriptTimeout"
#define IIS_CONFIG_LIMITS                   L"limits"
#define IIS_CONFIG_ALLOWDEBUG               L"appAllowDebugging"
#define IIS_CONFIG_ALLOWCLIENTDEBUG         L"appAllowClientDebug"
#define IIS_CONFIG_CERTIFICATEHASH          L"certificateHash"
#define IIS_CONFIG_CERTIFICATESTORENAME     L"certificateStoreName"


//local CAData action functions
HRESULT IIS7Site(
    __inout  LPWSTR *ppwzCustomActionData,
    __in     IAppHostWritableAdminManager *pAdminMgr
    );

HRESULT IIS7Application(
    __inout  LPWSTR *ppwzCustomActionData,
    __in     IAppHostWritableAdminManager *pAdminMgr
    );
HRESULT IIS7VDir(
    __inout  LPWSTR *ppwzCustomActionData,
    __in     IAppHostWritableAdminManager *pAdminMgr
    );
HRESULT IIS7Binding(
    __inout  LPWSTR *ppwzCustomActionData,
    __in     IAppHostWritableAdminManager *pAdminMgr
    );
HRESULT IIS7AppPool(
    __inout  LPWSTR *ppwzCustomActionData,
    __in     IAppHostWritableAdminManager *pAdminMgr
    );
HRESULT IIS7AppExtension(
    __inout  LPWSTR *ppwzCustomActionData,
    __in     IAppHostWritableAdminManager *pAdminMgr
    );
HRESULT IIS7MimeMap(
    __inout  LPWSTR *ppwzCustomActionData,
    __in     IAppHostWritableAdminManager *pAdminMgr
    );
HRESULT IIS7DirProperties(
    __inout  LPWSTR *ppwzCustomActionData,
    __in     IAppHostWritableAdminManager *pAdminMgr
    );
HRESULT IIS7WebLog(
    __inout  LPWSTR *ppwzCustomActionData,
    __in     IAppHostWritableAdminManager *pAdminMgr
    );
HRESULT IIS7FilterGlobal(
    __inout  LPWSTR *ppwzCustomActionData,
    __in     IAppHostWritableAdminManager *pAdminMgr
    );
HRESULT IIS7FilterSite(
    __inout  LPWSTR *ppwzCustomActionData,
    __in     IAppHostWritableAdminManager *pAdminMgr
    );
HRESULT IIS7HttpHeader(
    __inout  LPWSTR *ppwzCustomActionData,
    __in     IAppHostWritableAdminManager *pAdminMgr
    );
HRESULT IIS7WebError(
    __inout  LPWSTR *ppwzCustomActionData,
    __in     IAppHostWritableAdminManager *pAdminMgr
    );
HRESULT IIS7WebSvcExt(
    __inout  LPWSTR *ppwzCustomActionData,
    __in     IAppHostWritableAdminManager *pAdminMgr
    );
HRESULT IIS7WebProperty(
    __inout  LPWSTR *ppwzCustomActionData,
    __in     IAppHostWritableAdminManager *pAdminMgr
    );
HRESULT IIS7WebDir(
    __inout  LPWSTR *ppwzCustomActionData,
    __in     IAppHostWritableAdminManager *pAdminMgr
    );
HRESULT IIS7AspProperty(
    __inout  LPWSTR *ppwzCustomActionData,
    __in     IAppHostWritableAdminManager *pAdminMgr
    );
HRESULT IIS7SslBinding(
    __inout  LPWSTR *ppwzCustomActionData,
    __in     IAppHostWritableAdminManager *pAdminMgr
    );
//local helper functions
static HRESULT PutPropertyValue(IAppHostElement *pElement,LPCWSTR wszPropName, VARIANT vtPut);
static HRESULT PutPropertyValue(IAppHostElement *pElement,LPCWSTR wszPropName, DWORD dWord);
static HRESULT PutPropertyValue(IAppHostElement *pElement,LPCWSTR wszPropName, LPCWSTR wszString);
static HRESULT PutPropertyBoolValue(IAppHostElement *pElement, LPCWSTR wszPropName, BOOL fValue);

static HRESULT GetNextAvailableSiteId( IAppHostWritableAdminManager *pAdminMgr, DWORD *plSiteId);
static HRESULT GetSiteElement( IAppHostWritableAdminManager *pAdminMgr, LPCWSTR swSiteName, IAppHostElement **pSiteElement, BOOL* fFound);
static HRESULT GetApplicationElement( IAppHostElement *pSiteElement, LPCWSTR swAppPath,  IAppHostElement **pAppElement, BOOL* fFound);

static HRESULT CreateApplication( IAppHostElement *pSiteElement, LPCWSTR swAppPath, IAppHostElement **pAppElement);
static HRESULT DeleteApplication( IAppHostElement *pSiteElement, LPCWSTR swAppPath);

static HRESULT SetAppPool( IAppHostElement *pAppElementpAppElement, LPCWSTR pwzAppPool );
static HRESULT CreateVdir( IAppHostElement *pAppElement, LPCWSTR pwzVDirPath, LPCWSTR pwzVDirPhyDir );
static HRESULT DeleteVdir( IAppHostElement *pAppElement, LPCWSTR pwzVDirPath );

static HRESULT CreateBinding( IAppHostElement *pSiteElem, LPCWSTR pwzProtocol, LPCWSTR pwzInfo );
static HRESULT DeleteBinding( IAppHostElement *pSiteElem, LPCWSTR pwzProtocol, LPCWSTR pwzInfo );

static HRESULT CreateSslBinding( IAppHostElement *pSiteElem, LPCWSTR pwzStoreName, LPCWSTR pwzEncodedCertificateHash );
static HRESULT DeleteSslBinding( IAppHostElement *pSiteElem, LPCWSTR pwzStoreName, LPCWSTR pwzEncodedCertificateHash );

static HRESULT CreateSite( IAppHostElementCollection *pAdminMgr, LPCWSTR swSiteName, IAppHostElement **pSiteElement);

static HRESULT DeleteAppPool( IAppHostWritableAdminManager *pAdminMgr, LPCWSTR swAppPoolName);
static HRESULT CreateAppPool( __inout LPWSTR *ppwzCustomActionData, IAppHostWritableAdminManager *pAdminMgr, LPCWSTR swAppPoolName);

static HRESULT SetDirPropAuthentications(IAppHostWritableAdminManager *pAdminMgr, LPCWSTR wcConfigPath, DWORD dwData);
static HRESULT SetDirPropAuthProvider(IAppHostWritableAdminManager *pAdminMgr, LPCWSTR wszConfigPath, __in LPWSTR wszData);
static HRESULT SetDirPropDefDoc(IAppHostWritableAdminManager *pAdminMgr, LPCWSTR wszConfigPath, __in LPWSTR wszData);

static HRESULT ClearLocationTag( IAppHostWritableAdminManager *pAdminMgr, LPCWSTR swLocationPath);

static HRESULT CreateWebLog( IAppHostElement *pSiteElem, LPCWSTR pwzFormat);

static HRESULT CreateGlobalFilter(__inout LPWSTR *ppwzCustomActionData, IAppHostElement *pSection);
static HRESULT DeleteGlobalFilter(__inout LPWSTR *ppwzCustomActionData, IAppHostElement *pSection);

static HRESULT CreateSiteFilter(__inout LPWSTR *ppwzCustomActionData, IAppHostWritableAdminManager *pAdminMgr);
static HRESULT DeleteSiteFilter(__inout LPWSTR *ppwzCustomActionData, IAppHostWritableAdminManager *pAdminMgr);

static HRESULT DeleteCollectionElement(__in IAppHostElementCollection *pCollection,
                                       __in LPCWSTR pwzElementName,
                                       __in LPCWSTR pwzAttributeName,
                                       __in LPCWSTR pwzAttributeValue
                                       );

struct SCA_WEB_ERROR
{
    int iErrorCode;
    int iSubCode;
    WCHAR wzFile[MAX_PATH];
    WCHAR wzLangPath[MAX_PATH];
    int iResponseMode;
    SCA_WEB_ERROR *psweNext;
};
static HRESULT AddWebErrorToList(SCA_WEB_ERROR** ppsweList);
static void ScaWebErrorFreeList7(SCA_WEB_ERROR *psweList);
static HRESULT PopulateHttpErrors(IAppHostElement *pSection, SCA_WEB_ERROR **psweList);
static HRESULT GetErrorFromList( const SCA_WEB_ERROR & we, SCA_WEB_ERROR **ppsweList, SCA_WEB_ERROR **pswe, BOOL *fFound);

static void ConvSecToHMS( int Sec,  __out_ecount(cchDest) LPWSTR wcTime, size_t cchDest);
static void ConvSecToDHMS( unsigned int Sec,  __out_ecount(cchDest) LPWSTR wcTime, size_t cchDest);

////////////////////////////////////////////////////////////////////
// ScopeBSTR: Local helper class to construct & free BSTR from LPWSTR
//
/////////////////////////////////////////////////////////////////////
class ScopeBSTR
{
public:
    BSTR m_str;

    ScopeBSTR()
    {
        m_str = NULL;
    }

    ScopeBSTR( __in LPCWSTR pSrc)
    {
        if (pSrc == NULL)
        {
            m_str = NULL;
        }
        else
        {
            m_str = ::SysAllocString(pSrc);

        }
    }

    ~ScopeBSTR()
    {
        ::SysFreeString(m_str);
    }

    operator BSTR()
    {
        return m_str;
    }
};

static BOOL IsMatchingAppHostMethod( __in IAppHostMethod *pMethod,
                                      __in LPCWSTR pwzMethodName)
{
    HRESULT hr = S_OK;
    BOOL fResult = FALSE;
    BSTR bstrName = NULL;
    
    hr = pMethod->get_Name(&bstrName);
    ExitOnFailure(hr, "Failed to get name of element");
    if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, pwzMethodName, -1, bstrName, -1))
    {
        fResult = TRUE;
    }
LExit:
    if (bstrName)
    {
        ::SysFreeString(bstrName);
    }

    return fResult;
}
static BOOL IsMatchingAppHostElement( __in IAppHostElement *pElement,
                                      __in LPCWSTR pwzElementName,
                                      __in LPCWSTR pwzAttributeName,
                                      __in LPCWSTR pwzAttributeValue)
{
    HRESULT hr = S_OK;
    BOOL fResult = FALSE;
    IAppHostProperty *pProperty = NULL;
    BSTR bstrName = NULL;

    VARIANT vPropValue;
    VariantInit(&vPropValue);
    
    hr = pElement->get_Name(&bstrName);
    ExitOnFailure(hr, "Failed to get name of element");
    if (CSTR_EQUAL != ::CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, pwzElementName, -1, bstrName, -1))
    {
        ExitFunction();
    }

    hr = pElement->GetPropertyByName(ScopeBSTR(pwzAttributeName), &pProperty);
    if (ERROR_INVALID_INDEX == hr)
    {
        hr = S_FALSE;
        ExitFunction();
    }
    ExitOnFailure(hr, "Failed to get path of virtualDirectory element");

    hr = pProperty->get_Value(&vPropValue);
    ExitOnFailure(hr, "Failed to get value of path of virtualDirectory element");
    
    if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, pwzAttributeValue, -1, vPropValue.bstrVal, -1))
    {
        fResult = TRUE;
    }
LExit:
    if (bstrName)
    {
        ::SysFreeString(bstrName);
    }
    VariantClear(&vPropValue);

    ReleaseNullObject(pProperty);
    return fResult;
}

static HRESULT FindAppHostMethod( __in IAppHostMethodCollection *pCollection,
                                   __in LPCWSTR pwzMethodName,
                                   __out IAppHostMethod** ppMethod,
                                   __out DWORD* pdwIndex)
{  
    HRESULT hr = S_OK;
    IAppHostMethod *pMethod = NULL;
    DWORD dwMethods = 0;

    VARIANT vtIndex;
    VariantInit(&vtIndex);
    
    if (NULL != ppMethod)
    {
        *ppMethod = NULL;
    }
    if (NULL != pdwIndex)
    {
        *pdwIndex = MAXDWORD;
    }

    hr = pCollection->get_Count(&dwMethods);
    ExitOnFailure(hr, "Failed get application IAppHostMethodCollection count");
    
    vtIndex.vt = VT_UI4;
    for( DWORD i = 0; i < dwMethods; i++ )
    {
        vtIndex.ulVal = i;
        hr = pCollection->get_Item(vtIndex , &pMethod);
        ExitOnFailure(hr, "Failed get IAppHostMethod element");
        
        if (IsMatchingAppHostMethod(pMethod, pwzMethodName))
        {
            if (NULL != ppMethod)
            {
                *ppMethod = pMethod;
                pMethod = NULL;
            }
            if (NULL != pdwIndex)
            {
                *pdwIndex = i;
            }
            break;
        }

        ReleaseNullObject(pMethod);
    }
LExit:
    ReleaseNullObject(pMethod);
    VariantClear(&vtIndex);

    return hr;
}

static HRESULT FindAppHostElement( __in IAppHostElementCollection *pCollection,
                                   __in LPCWSTR pwzElementName,
                                   __in LPCWSTR pwzAttributeName,
                                   __in LPCWSTR pwzAttributeValue,
                                   __out IAppHostElement** ppElement,
                                   __out DWORD* pdwIndex)
{  
    HRESULT hr = S_OK;
    IAppHostElement *pElement = NULL;
    DWORD dwElements = 0;

    VARIANT vtIndex;
    VariantInit(&vtIndex);
    
    if (NULL != ppElement)
    {
        *ppElement = NULL;
    }
    if (NULL != pdwIndex)
    {
        *pdwIndex = MAXDWORD;
    }

    hr = pCollection->get_Count(&dwElements);
    ExitOnFailure(hr, "Failed get application IAppHostElementCollection count");
    
    vtIndex.vt = VT_UI4;
    for( DWORD i = 0; i < dwElements; i++ )
    {
        vtIndex.ulVal = i;
        hr = pCollection->get_Item(vtIndex , &pElement);
        ExitOnFailure(hr, "Failed get IAppHostElement element");
        
        if (IsMatchingAppHostElement(pElement, pwzElementName, pwzAttributeName, pwzAttributeValue))
        {
            if (NULL != ppElement)
            {
                *ppElement = pElement;
                pElement = NULL;
            }
            if (NULL != pdwIndex)
            {
                *pdwIndex = i;
            }
            break;
        }

        ReleaseNullObject(pElement);
    }
LExit:
    ReleaseNullObject(pElement);
    VariantClear(&vtIndex);

    return hr;
}

/********************************************************************
 IIS7ConfigChanges - Start of IIS7 config changes

 *******************************************************************/
HRESULT IIS7ConfigChanges(MSIHANDLE hInstall, __inout LPWSTR pwzData)
{
    HRESULT hr = S_OK;
    BOOL fInitializedCom = FALSE;

    IAppHostWritableAdminManager *pAdminMgr = NULL;

    LPWSTR pwz = NULL;
    int iConfigElement = -1;
    int iAction = -1;

    hr = ::CoInitialize(NULL);
    ExitOnFailure(hr, "Failed to initialize COM");
    fInitializedCom = TRUE;

    hr = CoCreateInstance( __uuidof(AppHostWritableAdminManager),
                            NULL,
                            CLSCTX_INPROC_SERVER,
                            __uuidof(IAppHostWritableAdminManager),
                            reinterpret_cast<void**> (&pAdminMgr));
    if (FAILED(hr))
    {
        ExitFunction();   // bail with the error result from the CoCreate to kick off a rollback
    }

    pwz = pwzData;

    while (S_OK == (hr = WcaReadIntegerFromCaData(&pwz, &iAction)))
    {
        switch (iAction)
        {
        case IIS_SITE:
            {
#pragma prefast(suppress:26010, "This is a prefast issue - pAdminMgr is correctly allocated")
                hr = IIS7Site(&pwz, pAdminMgr);
                ExitOnFailure(hr, "Failed to configure IIS site.");
                break;
            }
        case IIS_APPLICATION:
            {
#pragma prefast(suppress:26010, "This is a prefast issue - pAdminMgr is correctly allocated")
                hr = IIS7Application(&pwz, pAdminMgr);
                ExitOnFailure(hr, "Failed to configure IIS application.");
                break;
            }
        case IIS_VDIR:
            {
#pragma prefast(suppress:26010, "This is a prefast issue - pAdminMgr is correctly allocated")
                hr = IIS7VDir(&pwz, pAdminMgr);
                ExitOnFailure(hr, "Failed to configure IIS VDir.");
                break;
            }
        case IIS_BINDING:
            {
#pragma prefast(suppress:26010, "This is a prefast issue - pAdminMgr is correctly allocated")
                hr = IIS7Binding(&pwz, pAdminMgr);
                ExitOnFailure(hr, "Failed to configure IIS site binding.");
                break;
            }
        case IIS_APPPOOL:
            {
#pragma prefast(suppress:26010, "This is a prefast issue - pAdminMgr is correctly allocated")
                hr = IIS7AppPool(&pwz, pAdminMgr);
                ExitOnFailure(hr, "Failed to configure IIS appPool.");
                break;
            }
        case IIS_APPEXT_BEGIN:
            {
#pragma prefast(suppress:26010, "This is a prefast issue - pAdminMgr is correctly allocated")
                hr = IIS7AppExtension(&pwz, pAdminMgr);
                ExitOnFailure(hr, "Failed to configure IIS AppExtension.");
                break;
            }
        case IIS_MIMEMAP_BEGIN:
            {
#pragma prefast(suppress:26010, "This is a prefast issue - pAdminMgr is correctly allocated")
                hr = IIS7MimeMap(&pwz, pAdminMgr);
                ExitOnFailure(hr, "Failed to configure IIS MimeMap.");
                break;
            }
        case IIS_DIRPROP_BEGIN:
            {
#pragma prefast(suppress:26010, "This is a prefast issue - pAdminMgr is correctly allocated")
                hr = IIS7DirProperties(&pwz, pAdminMgr);
                ExitOnFailure(hr, "Failed to configure IIS DirProperties.");
                break;
            }
        case IIS_WEBLOG:
            {
#pragma prefast(suppress:26010, "This is a prefast issue - pAdminMgr is correctly allocated")
                hr = IIS7WebLog(&pwz, pAdminMgr);
                ExitOnFailure(hr, "Failed to configure IIS WebLog.");
                break;
            }
        case IIS_FILTER_GLOBAL_BEGIN:
            {
#pragma prefast(suppress:26010, "This is a prefast issue - pAdminMgr is correctly allocated")
                hr = IIS7FilterGlobal(&pwz, pAdminMgr);
                ExitOnFailure(hr, "Failed to configure IIS WebLog.");
                break;
            }
        case IIS_FILTER_BEGIN:
            {
#pragma prefast(suppress:26010, "This is a prefast issue - pAdminMgr is correctly allocated")
                hr = IIS7FilterSite(&pwz, pAdminMgr);
                ExitOnFailure(hr, "Failed to configure IIS Filter.");
                break;
            }
        case IIS_HTTP_HEADER_BEGIN:
            {
#pragma prefast(suppress:26010, "This is a prefast issue - pAdminMgr is correctly allocated")
                hr = IIS7HttpHeader(&pwz, pAdminMgr);
                ExitOnFailure(hr, "Failed to configure IIS http Header.");
                break;
            }
        case IIS_WEBERROR_BEGIN:
            {
#pragma prefast(suppress:26010, "This is a prefast issue - pAdminMgr is correctly allocated")
                hr = IIS7WebError(&pwz, pAdminMgr);
                ExitOnFailure(hr, "Failed to configure IIS http Errors.");
                break;
            }

        case IIS_WEB_SVC_EXT:
            {
#pragma prefast(suppress:26010, "This is a prefast issue - pAdminMgr is correctly allocated")
                hr = IIS7WebSvcExt(&pwz, pAdminMgr);
                ExitOnFailure(hr, "Failed to configure IIS web svc ext.");
                break;
            }
        case IIS_PROPERTY:
            {
#pragma prefast(suppress:26010, "This is a prefast issue - pAdminMgr is correctly allocated")
                hr = IIS7WebProperty(&pwz, pAdminMgr);
                ExitOnFailure(hr, "Failed to configure IIS web svc ext.");
                break;
            }
        case IIS_WEBDIR:
            {
#pragma prefast(suppress:26010, "This is a prefast issue - pAdminMgr is correctly allocated")
                hr = IIS7WebDir(&pwz, pAdminMgr);
                ExitOnFailure(hr, "Failed to configure IIS web svc ext.");
                break;
            }
        case IIS_ASP_BEGIN:
            {
#pragma prefast(suppress:26010, "This is a prefast issue - pAdminMgr is correctly allocated")
                hr = IIS7AspProperty(&pwz, pAdminMgr);
                ExitOnFailure(hr, "Failed to configure IIS web svc ext.");
                break;
            }
        case IIS_SSL_BINDING:
#pragma prefast(suppress:26010, "This is a prefast issue - pAdminMgr is correctly allocated")
                hr = IIS7SslBinding(&pwz, pAdminMgr);
                ExitOnFailure(hr, "Failed to configure IIS web svc ext.");
                break;

        default:
            ExitOnFailure1(hr = E_UNEXPECTED, "IIS7ConfigChanges: Unexpected IIS Config action specified: %d", iAction);
            break;
        }
    }

    if (E_NOMOREITEMS == hr) // If there are no more items, all is well
    {
        hr = S_OK;
    }
    if( hr == S_OK )
    {
        // commit config changes now to close out IIS Admin changes,
        // the Rollback or Commit defered CAs will determine final commit status.
        hr = pAdminMgr->CommitChanges();
        ExitOnFailure(hr , "Failed to Commit IIS Config Changes");
    }

LExit:
    ReleaseStr(pwzData);
    ReleaseNullObject(pAdminMgr);

    if (fInitializedCom)
    {
        ::CoUninitialize();
    }

    return hr;
}
//-------------------------------------------------------------------------------------------------
// IIS7AspProperty
// Called by WriteIIS7ConfigChanges
// Processes asp properties for WebApplication
//
//-------------------------------------------------------------------------------------------------

HRESULT IIS7AspProperty(
    __inout  LPWSTR *ppwzCustomActionData,
    __in     IAppHostWritableAdminManager *pAdminMgr
    )
{
    HRESULT hr = S_OK;

    int iAction = -1;
    int iData   =  0;

    LPWSTR pwzData = NULL;
    LPWSTR pwzSiteName = NULL;
    LPWSTR pwzPathName = NULL;
    LPWSTR pwzLocationPath = NULL;
    WCHAR wcTime[60];

    IAppHostElement *pSection = NULL;
    IAppHostElement *pElement = NULL;

    //read web site key
    hr = WcaReadStringFromCaData(ppwzCustomActionData, &pwzSiteName);
    ExitOnFailure(hr, "Failed read webDir webkey");

    //read path key
    hr = WcaReadStringFromCaData(ppwzCustomActionData, &pwzPathName);
    ExitOnFailure(hr, "Failed read webDir path");

    //Construct Location path
    hr = StrAllocFormatted(&pwzLocationPath, L"%s/%s", IIS_CONFIG_APPHOST_ROOT, pwzSiteName);
    ExitOnFailure(hr, "failed to format webDir location");
    //
    //Do not append trailing '/' for default vDir
    //
    if(CSTR_EQUAL != ::CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, pwzPathName, -1, L"/", -1))
    {   
        hr = StrAllocConcat(&pwzLocationPath, L"/", 0);
        ExitOnFailure(hr, "failed to copy location WebDir '/'");
        hr = StrAllocConcat(&pwzLocationPath, pwzPathName, 0);
        ExitOnFailure(hr, "failed to copy location WebDir path");
    }

    //get asp section at config path location tag
    hr = pAdminMgr->GetAdminSection( ScopeBSTR(IIS_CONFIG_ASP_SECTION), pwzLocationPath, &pSection);
    ExitOnFailure(hr, "Failed get httpErrors section");

    // Get  action
    hr = WcaReadIntegerFromCaData(ppwzCustomActionData, &iAction);
    ExitOnFailure(hr, "Failed to read property action");

    while (IIS_ASP_END != iAction)
    {
        switch (iAction)
        {
            case IIS_ASP_SESSIONSTATE:
            {
                //system.webServer/asp /session | allowSessionState
                hr = WcaReadIntegerFromCaData(ppwzCustomActionData, &iData);
                ExitOnFailure(hr, "Failed to read asp session state");
                hr = pSection->GetElementByName(ScopeBSTR(IIS_CONFIG_SESSION), &pElement);
                ExitOnFailure(hr, "Failed to get asp session element");
                hr = PutPropertyBoolValue( pElement, IIS_CONFIG_ALLOWSTATE, iData);
                ExitOnFailure(hr, "Failed to put asp session value");
                ReleaseNullObject(pElement);
                break;
            }
            case IIS_ASP_SESSIONTIMEOUT:
            {
                //system.webServer/asp /session | timeout
                hr = WcaReadIntegerFromCaData(ppwzCustomActionData, &iData);
                ExitOnFailure(hr, "Failed to read asp session timeout");
                hr = pSection->GetElementByName(ScopeBSTR(IIS_CONFIG_SESSION), &pElement);
                ExitOnFailure(hr, "Failed to get asp session timeout");
                *wcTime = '\0';
                ConvSecToHMS(iData, wcTime, countof( wcTime));
                hr = PutPropertyValue( pElement, IIS_CONFIG_TIMEOUT, wcTime);
                ExitOnFailure(hr, "Failed to put asp timeout value");
                ReleaseNullObject(pElement);
                break;
            }
            case IIS_ASP_BUFFER:
            {
                //system.webServer/asp | bufferingOn
                hr = WcaReadIntegerFromCaData(ppwzCustomActionData, &iData);
                ExitOnFailure(hr, "Failed to read asp bufferingOn");
                hr = PutPropertyBoolValue( pSection, IIS_CONFIG_BUFFERING, iData);
                ExitOnFailure(hr, "Failed to put asp bufferingOn value");
                break;
            }
            case IIS_ASP_PARENTPATHS:
            {
                //system.webServer/asp | enableParentPaths
                hr = WcaReadIntegerFromCaData(ppwzCustomActionData, &iData);
                ExitOnFailure(hr, "Failed to read asp ParentPaths");
                hr = PutPropertyBoolValue( pSection, IIS_CONFIG_PARENTPATHS, iData);
                ExitOnFailure(hr, "Failed to put asp ParentPaths value");
                break;
            }
            case IIS_ASP_SCRIPTLANG:
            {
                //system.webServer/asp | scriptLanguage
                hr = WcaReadStringFromCaData(ppwzCustomActionData, &pwzData);
                ExitOnFailure(hr, "Failed to read asp scriptLanguage");
                hr = PutPropertyValue( pSection, IIS_CONFIG_SCRIPTLANG, pwzData);
                ExitOnFailure(hr, "Failed to put asp scriptLanguage value");
                break;
            }
            case IIS_ASP_SCRIPTTIMEOUT:
            {
                //system.webServer/asp /limits | scriptTimeout
                hr = WcaReadIntegerFromCaData(ppwzCustomActionData, &iData);
                ExitOnFailure(hr, "Failed to read asp scriptTimeout");
                hr = pSection->GetElementByName(ScopeBSTR(IIS_CONFIG_LIMITS), &pElement);
                ExitOnFailure(hr, "Failed to get asp session element");
                *wcTime = '\0';
                ConvSecToHMS(iData, wcTime, countof( wcTime));
                hr = PutPropertyValue( pElement, IIS_CONFIG_SCRIPTTIMEOUT, wcTime);
                ExitOnFailure(hr, "Failed to put asp scriptTimeout value");
                ReleaseNullObject(pElement);
                break;

            }
            case IIS_ASP_SCRIPTSERVERDEBUG:
            {
                //system.webServer/asp | appAllowDebugging
                hr = WcaReadIntegerFromCaData(ppwzCustomActionData, &iData);
                ExitOnFailure(hr, "Failed to read asp appAllowDebugging");
                hr = PutPropertyBoolValue( pSection, IIS_CONFIG_ALLOWDEBUG, iData);
                ExitOnFailure(hr, "Failed to put asp appAllowDebugging value");
                break;
            }
            case IIS_ASP_SCRIPTCLIENTDEBUG:
            {
                //system.webServer/asp | appAllowClientDebug
                hr = WcaReadIntegerFromCaData(ppwzCustomActionData, &iData);
                ExitOnFailure(hr, "Failed to read asp appAllowClientDebug");
                hr = PutPropertyBoolValue( pSection, IIS_CONFIG_ALLOWCLIENTDEBUG, iData);
                ExitOnFailure(hr, "Failed to put asp appAllowClientDebug value");
                break;
            }
            default:
            {
                ExitOnFailure(hr = E_UNEXPECTED, "Unexpected IIS Config action specified for asp properties");
                break;
            }
        }
        // Get next action
        hr = WcaReadIntegerFromCaData(ppwzCustomActionData, &iAction);
        ExitOnFailure(hr, "Failed to read asp prop action");
    }

LExit:
    ReleaseStr(pwzData);
    ReleaseStr(pwzSiteName);
    ReleaseStr(pwzPathName);
    ReleaseStr(pwzLocationPath);
    ReleaseNullObject(pSection);
    ReleaseNullObject(pElement);

    return hr;
}

//-------------------------------------------------------------------------------------------------
// IIS7WebDir
// Called by WriteIIS7ConfigChanges
// Processes WebDir
//
//-------------------------------------------------------------------------------------------------
HRESULT IIS7WebDir(
    __inout  LPWSTR *ppwzCustomActionData,
    __in     IAppHostWritableAdminManager *pAdminMgr
    )
{
    HRESULT hr = S_OK;

    int iAction = -1;
    int iData   =  0;

    LPWSTR pwzSiteName = NULL;
    LPWSTR pwzPathName = NULL;
    LPWSTR pwzLocationPath = NULL;

    // Get  action
    hr = WcaReadIntegerFromCaData(ppwzCustomActionData, &iAction);
    ExitOnFailure(hr, "Failed to read property action");

    //read web site key
    hr = WcaReadStringFromCaData(ppwzCustomActionData, &pwzSiteName);
    ExitOnFailure(hr, "Failed read webDir webkey");

    //read path key
    hr = WcaReadStringFromCaData(ppwzCustomActionData, &pwzPathName);
    ExitOnFailure(hr, "Failed read webDir path");

    switch (iAction)
    {
        case IIS_CREATE:
        {
            //no action needed for create since WebDir has a
            //WebDirProperties element that will create and populate
            //location tag
            break;
        }
        case IIS_DELETE:
        {
            //Construct Location path
            hr = StrAllocString(&pwzLocationPath, pwzSiteName, 0);
            ExitOnFailure(hr, "failed to copy location WebDir web name");
            //
            //Do not append trailing '/' for default vDir
            //
            if(CSTR_EQUAL != ::CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, pwzPathName, -1, L"/", -1))
            {          
                hr = StrAllocConcat(&pwzLocationPath, L"/", 0);
                ExitOnFailure(hr, "failed to copy location WebDir '/'");
                hr = StrAllocConcat(&pwzLocationPath, pwzPathName, 0);
                ExitOnFailure(hr, "failed to copy location WebDir path");
            }
            // and delete location tag for this application
            hr = ClearLocationTag(pAdminMgr, pwzLocationPath);
            ExitOnFailure1(hr, "failed to clear location tag for %S", pwzLocationPath)
            break;
        }
        default:
        {
            ExitOnFailure(hr = E_UNEXPECTED, "Unexpected IIS Config action specified for WebDir");
            break;
        }
    }
LExit:
    ReleaseStr(pwzSiteName);
    ReleaseStr(pwzPathName);
    ReleaseStr(pwzLocationPath);

    return hr;
}

//-------------------------------------------------------------------------------------------------
// IIS7WebProperty
// Called by WriteIIS7ConfigChanges
// Processes isapiCgiRestriction
//
//-------------------------------------------------------------------------------------------------
HRESULT IIS7WebProperty(
    __inout  LPWSTR *ppwzCustomActionData,
    __in     IAppHostWritableAdminManager *pAdminMgr
    )
{
    HRESULT hr = S_OK;

    int iAction = -1;
    int iData   =  0;

    IAppHostElement *pSection = NULL;

    // Get  action
    hr = WcaReadIntegerFromCaData(ppwzCustomActionData, &iAction);
    ExitOnFailure(hr, "Failed to read property action");

    switch (iAction)
    {
        case IIS_PROPERTY_MAXBAND:
        {
            hr = WcaReadIntegerFromCaData(ppwzCustomActionData, &iData);
            ExitOnFailure(hr, "Failed to read property max band");
            //set value at system.applicationHost/webLimits | maxGlobalBandwidth
            //Get IIS config section
            hr = pAdminMgr->GetAdminSection(ScopeBSTR(IIS_CONFIG_WEBLIMITS_SECTION), ScopeBSTR(IIS_CONFIG_APPHOST_ROOT), &pSection);
            ExitOnFailure(hr, "Failed get isapiCgiRestriction section");
            if(!pSection)
            {
                hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
                ExitOnFailure(hr, "Failed get isapiCgiRestriction section object");
            }
            hr = PutPropertyValue(pSection, IIS_CONFIG_WEBLIMITS_MAXBAND, iData);

            break;
        }
        case IIS_PROPERTY_LOGUTF8:
        {
            hr = WcaReadIntegerFromCaData(ppwzCustomActionData, &iData);
            ExitOnFailure(hr, "Failed to read property log");
            //set value at system.applicationHost/log | logInUTF8
            //Get IIS config section
            hr = pAdminMgr->GetAdminSection(ScopeBSTR(IIS_CONFIG_LOG_SECTION), ScopeBSTR(IIS_CONFIG_APPHOST_ROOT), &pSection);
            ExitOnFailure(hr, "Failed get isapiCgiRestriction section");
            if(!pSection)
            {
                hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
                ExitOnFailure(hr, "Failed get isapiCgiRestriction section object");
            }
            hr = PutPropertyBoolValue(pSection, IIS_CONFIG_LOG_UTF8, iData);

            break;
        }
        default:
        {
            ExitOnFailure(hr = E_UNEXPECTED, "Unexpected IIS Config action specified for Web Property");
            break;
        }
    }

LExit:
    ReleaseNullObject(pSection);

    return hr;
}

//-------------------------------------------------------------------------------------------------
// IIS7WebSvcExt
// Called by WriteIIS7ConfigChanges
// Processes isapiCgiRestriction
//
//-------------------------------------------------------------------------------------------------
HRESULT IIS7WebSvcExt(
    __inout  LPWSTR *ppwzCustomActionData,
    __in     IAppHostWritableAdminManager *pAdminMgr
    )
{
    HRESULT hr = S_OK;

    int iAction = -1;
    int iData   =  0;
    BOOL fFound = FALSE;
    LPWSTR pwzData = NULL;
    LPWSTR pwzPath = NULL;

    IAppHostElement *pSection = NULL;
    IAppHostElement *pElement = NULL;
    IAppHostElementCollection *pCollection = NULL;

    DWORD cElements = 0;

    // Get  action
    hr = WcaReadIntegerFromCaData(ppwzCustomActionData, &iAction);
    ExitOnFailure(hr, "Failed to read WebSvcExt action");

    //get path
    hr = WcaReadStringFromCaData(ppwzCustomActionData, &pwzPath);
    ExitOnFailure(hr, "Failed to read WebSvcExt key");

    //Get IIS config section
    hr = pAdminMgr->GetAdminSection(ScopeBSTR(IIS_CONFIG_RESTRICTION_SECTION), ScopeBSTR(IIS_CONFIG_APPHOST_ROOT), &pSection);
    ExitOnFailure(hr, "Failed get isapiCgiRestriction section");
    if(!pSection)
    {
        hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
        ExitOnFailure(hr, "Failed get isapiCgiRestriction section object");
    }
    //get collection
    hr = pSection->get_Collection(&pCollection);
    ExitOnFailure(hr, "Failed get isapiCgiRestriction collection");

    //find element
    hr = FindAppHostElement(pCollection, IIS_CONFIG_ADD, IIS_CONFIG_PATH, pwzPath, &pElement, NULL);
    ExitOnFailure(hr, "Failed get isapiCgiRestriction element");
    fFound = (NULL != pElement);

    switch (iAction)
    {
        case IIS_CREATE:
        {
            if(!fFound)
            {
                //create a restriction element
                hr = pCollection->CreateNewElement(ScopeBSTR(IIS_CONFIG_ADD), &pElement);
                ExitOnFailure(hr, "Failed create isapiCgiRestriction element");
                //put path
                hr = PutPropertyValue(pElement, IIS_CONFIG_PATH, pwzPath);
                ExitOnFailure(hr, "Failed set isapiCgiRestriction path property");
            }
            //update common properties

            //update allowed
            hr = WcaReadIntegerFromCaData(ppwzCustomActionData, &iData);
            ExitOnFailure(hr, "Failed to read WebSvcExt allowed");
            hr = PutPropertyBoolValue(pElement, IIS_CONFIG_ALLOWED, iData);
            ExitOnFailure(hr, "Failed set isapiCgiRestriction allowed property");

            //update groupId
            hr = WcaReadStringFromCaData(ppwzCustomActionData, &pwzData);
            ExitOnFailure(hr, "Failed to read WebSvcExt group ID");
            if(*pwzData)
            {
                hr = PutPropertyValue(pElement, IIS_CONFIG_GROUPID, pwzData);
                ExitOnFailure(hr, "Failed set isapiCgiRestriction groupId property");
            }
            //update description
            hr = WcaReadStringFromCaData(ppwzCustomActionData, &pwzData);
            ExitOnFailure(hr, "Failed to read WebSvcExt description");
            if(*pwzData)
            {
                hr = PutPropertyValue(pElement, IIS_CONFIG_DESC, pwzData);
                ExitOnFailure(hr, "Failed set isapiCgiRestriction description property");
            }
            // add element if new
            if(!fFound)
            {
                hr = pCollection->AddElement(pElement);
                ExitOnFailure(hr, "Failed add isapiCgiRestriction element");
            }

            break;
        }
        case IIS_DELETE:
        {
            hr = DeleteCollectionElement(pCollection, IIS_CONFIG_ADD, IIS_CONFIG_PATH, pwzPath);
            ExitOnFailure(hr, "Failed delete isapiCgiRestriction element");
            break;
        }
        default:
        {
            ExitOnFailure(hr = E_UNEXPECTED, "Unexpected IIS Config action specified for WebSvcExt");
            break;
        }
    }

LExit:
    ReleaseStr(pwzPath);
    ReleaseStr(pwzData);
    ReleaseNullObject(pSection);
    ReleaseNullObject(pElement);
    ReleaseNullObject(pCollection);

    return hr;

}

//-------------------------------------------------------------------------------------------------
// IIS7WebError
// Called by WriteIIS7ConfigChanges
// Processes http header CA Data
//
//-------------------------------------------------------------------------------------------------

HRESULT IIS7WebError(
    __inout  LPWSTR *ppwzCustomActionData,
    __in     IAppHostWritableAdminManager *pAdminMgr
    )
{
    HRESULT hr = S_OK;
    LPWSTR pwzConfigPath = NULL;
    LPWSTR pwzSiteName = NULL;
    LPWSTR pwzAppName = NULL;

    IAppHostElement *pElement = NULL;
    IAppHostElement *pSection = NULL;
    IAppHostElementCollection *pCollection = NULL;

    SCA_WEB_ERROR *psweList = NULL;
    SCA_WEB_ERROR* pswe = NULL;
    SCA_WEB_ERROR we;
    BOOL fFound = FALSE;

    int iAction = -1;
    int iData = -1;
    LPWSTR pwzData = NULL;

    //read web site key
    hr = WcaReadStringFromCaData(ppwzCustomActionData, &pwzSiteName);
    ExitOnFailure(hr, "Failed read web error site name");

    //read app key
    hr = WcaReadStringFromCaData(ppwzCustomActionData, &pwzAppName);
    ExitOnFailure(hr, "Failed read web error app name");

    //Construct config root path
    hr = StrAllocFormatted(&pwzConfigPath, L"%s/%s", IIS_CONFIG_APPHOST_ROOT, pwzSiteName);
    ExitOnFailure(hr, "failed to format web error config path");
    
    if(CSTR_EQUAL != ::CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, pwzAppName, -1, L"/", -1))
    {
        hr = StrAllocConcat(&pwzConfigPath, L"/", 0);
        ExitOnFailure(hr, "failed to copy web error config path delim");
        hr = StrAllocConcat(&pwzConfigPath, pwzAppName, 0);
        ExitOnFailure(hr, "failed to app name to web error config path");
    }

    //get httpErrors section at config path location tag
    hr = pAdminMgr->GetAdminSection(ScopeBSTR(IIS_CONFIG_HTTPERRORS_SECTION), pwzConfigPath, &pSection);
    ExitOnFailure(hr, "Failed get httpErrors section");

    //get existing httpErrors list & clear collection
    hr = PopulateHttpErrors(pSection, &psweList);
    ExitOnFailure(hr, "Failed to read httpErrors list");

    //get collection
    hr = pSection->get_Collection(&pCollection);
    ExitOnFailure(hr, "Failed get httpErrors collection");

    DWORD cErrors = 0;
    hr = pCollection->get_Count(&cErrors);

   // Get web error action
    hr = WcaReadIntegerFromCaData(ppwzCustomActionData, &iAction);
    ExitOnFailure(hr, "Failed to read filter action");
    while (IIS_WEBERROR_END != iAction)
    {
        //Process property action
        if(IIS_WEBERROR == iAction)
        {
            hr = WcaReadIntegerFromCaData(ppwzCustomActionData, &(we.iErrorCode));
            ExitOnFailure(hr, "failed to get httpErrors ErrorCode");

            hr = WcaReadIntegerFromCaData(ppwzCustomActionData, &(we.iSubCode));
            ExitOnFailure(hr, "failed to get httpErrors SubCode");
            //0 is the sub error code wild card for IIS6, change to -1 for IIS7
            if(we.iSubCode == 0)
            {
                we.iSubCode = -1;
            }
            hr = WcaReadStringFromCaData(ppwzCustomActionData, &pwzData);
            ExitOnFailure(hr, "Failed to get httpErrors File");
            hr = ::StringCchCopyW(we.wzFile, countof(we.wzFile), pwzData);
            ExitOnFailure(hr, "Failed to copy httpErrors File");

            hr = WcaReadIntegerFromCaData(ppwzCustomActionData, &(we.iResponseMode));
            ExitOnFailure(hr, "Failed to get httpErrors File code");

            fFound = FALSE;
            hr = GetErrorFromList( we, &psweList, &pswe, &fFound);
            if( !fFound )
            {
                hr = AddWebErrorToList(&psweList);
                ExitOnFailure(hr, "failed to add web error to list");
                pswe = psweList;
            }
            else
            {
                //if overwriting existing http errors element then clear lang path
                hr = ::StringCchCopyW(pswe->wzLangPath, countof(pswe->wzLangPath), L"");
                ExitOnFailure(hr, "Failed to copy httpErrors lang path value");
            }
            pswe->iErrorCode = we.iErrorCode;
            pswe->iSubCode   = we.iSubCode;
            hr = ::StringCchCopyW(pswe->wzFile, countof(pswe->wzFile), we.wzFile);
            ExitOnFailure(hr, "Failed to copy httpErrors File value");
            pswe->iResponseMode = we.iResponseMode;

        }
        else
        {
            ExitOnFailure(hr = E_UNEXPECTED, "Unexpected IIS Config action specified for http header");
        }

        // Get AppExt action
        hr = WcaReadIntegerFromCaData(ppwzCustomActionData, &iAction);
        ExitOnFailure(hr, "Failed to read filter action");
    }

    //No inheritance - put a clear in at this loc tag
    hr = pCollection->CreateNewElement(ScopeBSTR(IIS_CONFIG_CLEAR), &pElement);
    ExitOnFailure(hr, "Failed create httpErrors clear");
    hr = pCollection->AddElement(pElement);
    ExitOnFailure(hr, "Failed add httpErrors clear");

    //now we have merged new, from MSI, http errors with global list
    //write this back out at location tag.
    // Loop through the HTTP headers
    for ( pswe = psweList; pswe; pswe = pswe->psweNext)
    {
        hr = pCollection->CreateNewElement(ScopeBSTR(IIS_CONFIG_ERROR), &pElement);
        ExitOnFailure(hr, "Failed create httpErrors element");

        // status code
        hr = PutPropertyValue(pElement, IIS_CONFIG_STATUSCODE, pswe->iErrorCode);
        ExitOnFailure(hr, "Failed set httpErrors code value");

        //sub status
        hr = PutPropertyValue(pElement, IIS_CONFIG_SUBSTATUS, pswe->iSubCode);
        ExitOnFailure(hr, "Failed set httpErrors sub code value");

        //lang path
        hr = PutPropertyValue(pElement, IIS_CONFIG_LANGPATH, pswe->wzLangPath);
        ExitOnFailure(hr, "Failed set httpErrors lang path value");

        //path
        hr = PutPropertyValue(pElement, IIS_CONFIG_PATH, pswe->wzFile);
        ExitOnFailure(hr, "Failed set httpErrors path value");

        //response mode
        hr = PutPropertyValue(pElement, IIS_CONFIG_RESPMODE, pswe->iResponseMode);
        ExitOnFailure(hr, "Failed set httpErrors resp mode value");

        //add the element
        hr = pCollection->AddElement(pElement);
        ExitOnFailure(hr, "Failed add httpErrors element");
        ReleaseNullObject(pElement);
    }

LExit:
    ScaWebErrorFreeList7(psweList);
    
    ReleaseStr(pwzConfigPath);
    ReleaseStr(pwzSiteName);
    ReleaseStr(pwzAppName);
    ReleaseStr(pwzData);
    ReleaseNullObject(pElement);
    ReleaseNullObject(pSection);
    ReleaseNullObject(pCollection);
   return hr;

}

static HRESULT PopulateHttpErrors(IAppHostElement *pSection, SCA_WEB_ERROR **ppsweList)
{
    HRESULT hr = S_OK;

    IAppHostElement *pElement = NULL;
    IAppHostElementCollection *pCollection = NULL;
    IAppHostProperty *pProperty = NULL;

    DWORD cErrors = 0;
    SCA_WEB_ERROR *pswe = NULL;

    VARIANT vPropValue;
    VARIANT vtIndex;

    VariantInit(&vPropValue);
    VariantInit(&vtIndex);

    hr = pSection->get_Collection(&pCollection);
    ExitOnFailure(hr, "Failed get httpErrors collection");

    hr = pCollection->get_Count(&cErrors);
    ExitOnFailure(hr, "Failed get sites collection count");

    vtIndex.vt = VT_UI4;
    for( DWORD i = 0; i < cErrors; i++ )
    {
        vtIndex.ulVal = i;
        hr = pCollection->get_Item(vtIndex , &pElement);
        ExitOnFailure(hr, "Failed get httpErrors collection item");

        hr = AddWebErrorToList(ppsweList);
        ExitOnFailure(hr, "Failed add web error list item");
        pswe = *ppsweList;

        //get all properties
        //
        // statusCode UINT
        // subStatusCode INT
        // prefixLanguageFilePath type="string"
        // path type="string"
        // responseMode type="enum" defaultValue="File">
        //  <enum name="File" value="0" />
        //  <enum name="ExecuteURL" value="1" />
        //  <enum name="Redirect" value="2" />

        // status code
        hr = pElement->GetPropertyByName(ScopeBSTR(IIS_CONFIG_STATUSCODE), &pProperty);
        ExitOnFailure(hr, "Failed get httpErrors code property");
        hr = pProperty->get_Value(&vPropValue);
        ExitOnFailure(hr, "Failed get httpErrors code value");
        pswe->iErrorCode = vPropValue.uintVal;
        VariantClear(&vPropValue);

        //sub status
        hr = pElement->GetPropertyByName(ScopeBSTR(IIS_CONFIG_SUBSTATUS), &pProperty);
        ExitOnFailure(hr, "Failed get httpErrors sub code property");
        hr = pProperty->get_Value(&vPropValue);
        ExitOnFailure(hr, "Failed get httpErrors sub code value");
        pswe->iSubCode = vPropValue.intVal;
        VariantClear(&vPropValue);

        //lang path
        hr = pElement->GetPropertyByName(ScopeBSTR(IIS_CONFIG_LANGPATH), &pProperty);
        ExitOnFailure(hr, "Failed get httpErrors lang path property");
        hr = pProperty->get_Value(&vPropValue);
        ExitOnFailure(hr, "Failed get httpErrors lang path value");
        hr = ::StringCchCopyW(pswe->wzLangPath, countof(pswe->wzLangPath), vPropValue.bstrVal);
        ExitOnFailure(hr, "Failed to copy httpErrors lang path");
        VariantClear(&vPropValue);

        //path
        hr = pElement->GetPropertyByName(ScopeBSTR(IIS_CONFIG_PATH), &pProperty);
        ExitOnFailure(hr, "Failed get httpErrors path property");
        hr = pProperty->get_Value(&vPropValue);
        ExitOnFailure(hr, "Failed get httpErrors path value");
        hr = ::StringCchCopyW(pswe->wzFile, countof(pswe->wzFile), vPropValue.bstrVal);
        ExitOnFailure(hr, "Failed to copy httpErrors File");
        VariantClear(&vPropValue);

        //response mode
        hr = pElement->GetPropertyByName(ScopeBSTR(IIS_CONFIG_RESPMODE), &pProperty);
        ExitOnFailure(hr, "Failed get httpErrors resp mode property");
        hr = pProperty->get_Value(&vPropValue);
        ExitOnFailure(hr, "Failed get httpErrors resp mode value");
        pswe->iResponseMode = vPropValue.intVal;
        VariantClear(&vPropValue);

        ReleaseNullObject(pElement);
        ReleaseNullObject(pProperty);
    }
    
    //remove the elements from connection so we can add back later
    hr = pCollection->Clear();
    ExitOnFailure(hr, "Failed clear httpErrors collection");

LExit:
    VariantClear(&vPropValue);
    ReleaseNullObject(pProperty);
    ReleaseNullObject(pElement);
    ReleaseNullObject(pCollection);
    return hr;
}

static void ScaWebErrorFreeList7(SCA_WEB_ERROR *psweList)
{
    SCA_WEB_ERROR *psweDelete = psweList;
    while (psweList)
    {
        psweDelete = psweList;
        psweList = psweList->psweNext;

        MemFree(psweDelete);
    }
}
static HRESULT AddWebErrorToList(SCA_WEB_ERROR **ppsweList)
{
    HRESULT hr = S_OK;

    SCA_WEB_ERROR* pswe = static_cast<SCA_WEB_ERROR*>(MemAlloc(sizeof(SCA_WEB_ERROR), TRUE));

    ExitOnNull(pswe, hr, E_OUTOFMEMORY, "failed to allocate memory for new web error list element");

    pswe->psweNext = *ppsweList;
    *ppsweList = pswe;

LExit:
    return hr;
}
static HRESULT GetErrorFromList( const SCA_WEB_ERROR& we,
                                    SCA_WEB_ERROR **ppsweList,
                                    SCA_WEB_ERROR **ppswe,
                                    BOOL *fFound)
{
    HRESULT hr = S_OK;

    *fFound = FALSE;

    SCA_WEB_ERROR *pswe;

    for ( pswe = *ppsweList; pswe; pswe = pswe->psweNext)
    {
        if((pswe->iErrorCode == we.iErrorCode) && (pswe->iSubCode == we.iSubCode) )
        {
            *fFound = TRUE;
            *ppswe = pswe;
            break;
        }
    }

    return hr;
}

//-------------------------------------------------------------------------------------------------
// IIS7HttpHeader
// Called by WriteIIS7ConfigChanges
// Processes http header CA Data
//
//-------------------------------------------------------------------------------------------------

HRESULT IIS7HttpHeader(
    __inout  LPWSTR *ppwzCustomActionData,
    __in     IAppHostWritableAdminManager *pAdminMgr
    )
{
    HRESULT hr = S_OK;
    LPWSTR pwzConfigPath = NULL;
    LPWSTR pwzSiteName = NULL;
    LPWSTR pwzAppName = NULL;

    LPWSTR pwzHeaderName = NULL;
    LPWSTR pwzHeaderValue = NULL;

    IAppHostElement *pElement = NULL;
    IAppHostElement *pSection = NULL;
    IAppHostElementCollection *pCollection = NULL;
    IAppHostElement *pElementHeaders = NULL;

    int iAction = -1;
    LPWSTR pwzData = NULL;
    BOOL fFound = FALSE;

    //read web site key
    hr = WcaReadStringFromCaData(ppwzCustomActionData, &pwzSiteName);
    ExitOnFailure(hr, "Failed read header web site name");

    //read app key
    hr = WcaReadStringFromCaData(ppwzCustomActionData, &pwzAppName);
    ExitOnFailure(hr, "Failed read header appkey");

    //Construct config root path
    hr = StrAllocFormatted(&pwzConfigPath, L"%s/%s", IIS_CONFIG_APPHOST_ROOT, pwzSiteName);
    ExitOnFailure(hr, "failed to format web error config path");
    
    if(CSTR_EQUAL != ::CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, pwzAppName, -1, L"/", -1))
    {
        hr = StrAllocConcat(&pwzConfigPath, L"/", 0);
        ExitOnFailure(hr, "failed to copy web error config path delim");
        hr = StrAllocConcat(&pwzConfigPath, pwzAppName, 0);
        ExitOnFailure(hr, "failed to app name to web error config path");
    }

    //get admin handlers section at config path location tag
    hr = pAdminMgr->GetAdminSection(ScopeBSTR(IIS_CONFIG_HTTPPROTO_SECTION), pwzConfigPath, &pSection);
    ExitOnFailure(hr, "Failed get http protocol section");

    hr = pSection->GetElementByName(ScopeBSTR(IIS_CONFIG_HEADERS), &pElementHeaders);
    ExitOnFailure(hr, "Failed get http customHeaders section");

    hr = pElementHeaders->get_Collection(&pCollection);
    ExitOnFailure(hr, "Failed get http customHeaders collection");

   // Get filter action
    hr = WcaReadIntegerFromCaData(ppwzCustomActionData, &iAction);
    ExitOnFailure(hr, "Failed to read filter action");
    while (IIS_HTTP_HEADER_END != iAction)
    {
        //Process property action
        if(IIS_HTTP_HEADER == iAction)
        {
            hr = WcaReadStringFromCaData(ppwzCustomActionData, &pwzHeaderName);
            ExitOnFailure(hr, "Fail to read httpHeader name");

            hr = WcaReadStringFromCaData(ppwzCustomActionData, &pwzHeaderValue);
            ExitOnFailure(hr, "Fail to read httpHeader value");
            
            hr = FindAppHostElement(pCollection, IIS_CONFIG_ADD, IIS_CONFIG_NAME, pwzHeaderName, &pElement, NULL);
            ExitOnFailure(hr, "Failed get isapiCgiRestriction element");
            fFound = (NULL != pElement);

            if (!fFound)
            {
                //make a new element
                hr = pCollection->CreateNewElement(ScopeBSTR(IIS_CONFIG_ADD), &pElement);
                ExitOnFailure(hr, "Failed to create filter config element");

                hr = PutPropertyValue(pElement, IIS_CONFIG_NAME, pwzHeaderName);
                ExitOnFailure(hr, "Failed to set header name");
            }

            hr = PutPropertyValue(pElement, IIS_CONFIG_VALUE, pwzHeaderValue);
            ExitOnFailure(hr, "Failed to set header Value");

            if (!fFound)
            {
                hr = pCollection->AddElement(pElement);
                ExitOnFailure(hr, "Failed add http header");
            }

        }
        else
        {
            ExitOnFailure(hr = E_UNEXPECTED, "Unexpected IIS Config action specified for http header");
        }

        // Get AppExt action
        hr = WcaReadIntegerFromCaData(ppwzCustomActionData, &iAction);
        ExitOnFailure(hr, "Failed to read filter action");
    }

LExit:
    ReleaseStr(pwzConfigPath);
    ReleaseStr(pwzSiteName);
    ReleaseStr(pwzAppName);
    ReleaseStr(pwzHeaderName);
    ReleaseStr(pwzHeaderValue);
    ReleaseNullObject(pElementHeaders);
    ReleaseNullObject(pElement);
    ReleaseNullObject(pSection);
    ReleaseNullObject(pCollection);
    return hr;
}

//-------------------------------------------------------------------------------------------------
// IIS7FilterGlobal
// Called by WriteIIS7ConfigChanges
// Processes Filter CA Data
//
//-------------------------------------------------------------------------------------------------
HRESULT IIS7FilterGlobal(
    __inout  LPWSTR *ppwzCustomActionData,
    __in     IAppHostWritableAdminManager *pAdminMgr
    )
{
    HRESULT hr = S_OK;
    int iAction = 0;

    IAppHostElement *pSection = NULL;

    hr = pAdminMgr->GetAdminSection(ScopeBSTR(IIS_CONFIG_ISAPI_SECTION), ScopeBSTR(IIS_CONFIG_APPHOST_ROOT), &pSection);
    ExitOnFailure(hr, "Failed get sites section");

    if(!pSection)
    {
        hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
        ExitOnFailure(hr, "Failed get isapiFilters section object");
    }

   // Get filter action
    hr = WcaReadIntegerFromCaData(ppwzCustomActionData, &iAction);
    ExitOnFailure(hr, "Failed to read filter action");

    while (IIS_FILTER_END != iAction)
    {
        //Process property action
        switch (iAction)
        {
            case IIS_FILTER :
            {
                hr = WcaReadIntegerFromCaData(ppwzCustomActionData, &iAction);
                ExitOnFailure(hr, "Failed to read filter action");

                if(iAction == IIS_CREATE)
                {
                    hr = CreateGlobalFilter(ppwzCustomActionData, pSection);
                }
                else
                {
                    hr = DeleteGlobalFilter(ppwzCustomActionData, pSection);
                }
                break;
            }
            default:
            {
                ExitOnFailure(hr = E_UNEXPECTED, "Unexpected IIS Config action specified for global filter");
                break;
            }
        }
        // Get AppExt action
        hr = WcaReadIntegerFromCaData(ppwzCustomActionData, &iAction);
        ExitOnFailure(hr, "Failed to read filter action");

    }

LExit:
    ReleaseNullObject(pSection);

    return hr;

}

static HRESULT CreateGlobalFilter( __inout LPWSTR *ppwzCustomActionData, IAppHostElement *pSection)
{
    HRESULT hr = S_OK;

    int iAction = 0;
    LPWSTR pwzFilterName = NULL;
    LPWSTR pwzSiteName = NULL;
    LPWSTR pwzFilterPath = NULL;
    int iLoadOrder = 0;
    DWORD cFilters = 0;

    IAppHostElement *pElement = NULL;
    IAppHostElementCollection *pCollection = NULL;

    hr = pSection->get_Collection(&pCollection);
    ExitOnFailure(hr, "Failed get filter collection");

    hr = pCollection->get_Count(&cFilters);
    ExitOnFailure(hr, "Failed get filter collection count");

    // Attempt to delete, we will we recreate with desired property values and order
    hr = DeleteCollectionElement(pCollection, IIS_CONFIG_FILTER, IIS_CONFIG_NAME, pwzFilterName);
    ExitOnFailure(hr, "Failed to delete filter");

    //make a new element
    hr = pCollection->CreateNewElement(ScopeBSTR(IIS_CONFIG_FILTER), &pElement);
    ExitOnFailure(hr, "Failed to create filter config element");

    //filter Name key
    hr = WcaReadStringFromCaData(ppwzCustomActionData, &pwzFilterName);
    ExitOnFailure(hr, "Failed to read filter name");
    hr = PutPropertyValue(pElement, IIS_CONFIG_NAME, pwzFilterName);
    ExitOnFailure(hr, "Failed to set filter name");

    //web site name
    hr = WcaReadStringFromCaData(ppwzCustomActionData, &pwzSiteName);
    ExitOnFailure(hr, "Failed to read filter site name");

    // filter path
    hr = WcaReadStringFromCaData(ppwzCustomActionData, &pwzFilterPath);
    ExitOnFailure(hr, "Failed to read filter path");
    hr = PutPropertyValue(pElement,IIS_CONFIG_PATH, pwzFilterPath);
    ExitOnFailure(hr, "Failed to set filter path");

    //filter load order
    hr = WcaReadIntegerFromCaData(ppwzCustomActionData, &iLoadOrder);
    ExitOnFailure(hr, "Failed to read filter load order");

    //  put element in order in list
    int iPosition = -1;
    int icFilters = cFilters;
    switch (iLoadOrder)
    {
        case 0 :
        {
            iPosition = -1;
            break;
        }
        case -1 :
        {
            iPosition = icFilters;
            break;
        }
        case MSI_NULL_INTEGER :
        {
             iPosition = icFilters;
             break;
        }
        default:
        {
            if( iLoadOrder > icFilters)
            {
                iPosition = icFilters;
            }
            else
            {
                iPosition = iLoadOrder;
            }
            break;
        }
    }
    hr = pCollection->AddElement(pElement, iPosition);
    ExitOnFailure(hr, "Failed to add filter element");

LExit:
    ReleaseStr(pwzFilterName);
    ReleaseStr(pwzSiteName);
    ReleaseStr(pwzFilterPath);
    ReleaseNullObject(pCollection);
    ReleaseNullObject(pElement);

    return hr;
}

static HRESULT DeleteGlobalFilter( __inout LPWSTR *ppwzCustomActionData, IAppHostElement *pSection)
{
    HRESULT hr = S_OK;

    int iAction = 0;
    LPWSTR pwzFilterName = NULL;
    LPWSTR pwzSiteName = NULL;

    IAppHostElementCollection *pCollection = NULL;

    hr = pSection->get_Collection(&pCollection);
    ExitOnFailure(hr, "Failed get filter collection");

    //filter Name key
    hr = WcaReadStringFromCaData(ppwzCustomActionData, &pwzFilterName);
    ExitOnFailure(hr, "Failed to read filter name");

    //web site name
    hr = WcaReadStringFromCaData(ppwzCustomActionData, &pwzSiteName); // TODO: unused?
    ExitOnFailure(hr, "Failed to read filter site name");

    DeleteCollectionElement(pCollection, IIS_CONFIG_FILTER, IIS_CONFIG_NAME, pwzFilterName);
    ExitOnFailure1(hr, "Failed to delete filter %S", pwzFilterName);

LExit:
    ReleaseStr(pwzFilterName);
    ReleaseStr(pwzSiteName);
    ReleaseNullObject(pCollection);

    return hr;
}

//-------------------------------------------------------------------------------------------------
// IIS7FilterSite
// Called by WriteIIS7ConfigChanges
// Processes Filter CA Data
//
//-------------------------------------------------------------------------------------------------
HRESULT IIS7FilterSite(
    __inout  LPWSTR *ppwzCustomActionData,
    __in     IAppHostWritableAdminManager *pAdminMgr
    )
{
    HRESULT hr = S_OK;
    int iAction = 0;

   // Get filter action
    hr = WcaReadIntegerFromCaData(ppwzCustomActionData, &iAction);
    ExitOnFailure(hr, "Failed to read filter action");

    while (IIS_FILTER_END != iAction)
    {
        //Process property action
        switch (iAction)
        {
            case IIS_FILTER :
            {
                hr = WcaReadIntegerFromCaData(ppwzCustomActionData, &iAction);
                ExitOnFailure(hr, "Failed to read filter action");

                if(iAction == IIS_CREATE)
                {
                    hr = CreateSiteFilter(ppwzCustomActionData, pAdminMgr);
                }
                else
                {
                    hr = DeleteSiteFilter(ppwzCustomActionData, pAdminMgr);
                }
                break;
            }
            default:
            {
                ExitOnFailure(hr = E_UNEXPECTED, "Unexpected IIS Config action specified for global filter");
                break;
            }
        }

        // Get AppExt action
        hr = WcaReadIntegerFromCaData(ppwzCustomActionData, &iAction);
        ExitOnFailure(hr, "Failed to read filter action");
    }

LExit:
    return hr;

}

static HRESULT CreateSiteFilter(__inout LPWSTR *ppwzCustomActionData, IAppHostWritableAdminManager *pAdminMgr)
{
    HRESULT hr = S_OK;
    LPWSTR pwzFilterName = NULL;
    LPWSTR pwzSiteName = NULL;
    LPWSTR pwzFilterPath = NULL;
    LPWSTR pwzConfigPath = NULL;
    int iLoadOrder = 0;
    DWORD cFilters;

    IAppHostElement *pElement = NULL;
    IAppHostElement *pSection = NULL;
    IAppHostElementCollection *pCollection = NULL;

    //filter Name key
    hr = WcaReadStringFromCaData(ppwzCustomActionData, &pwzFilterName);
    ExitOnFailure(hr, "Failed to read filter name");

    //web site name
    hr = WcaReadStringFromCaData(ppwzCustomActionData, &pwzSiteName);
    ExitOnFailure(hr, "Failed to read filter site name");

     //Construct config root
    hr = StrAllocFormatted(&pwzConfigPath, L"%s/%s", IIS_CONFIG_APPHOST_ROOT, pwzSiteName);
    ExitOnFailure(hr, "failed to format filter config path");

    //get admin isapiFilters section at config path location tag
    hr = pAdminMgr->GetAdminSection(ScopeBSTR(IIS_CONFIG_ISAPI_SECTION), pwzConfigPath, &pSection);
    ExitOnFailure(hr, "Failed get isapiFilters section");

    hr = pSection->get_Collection(&pCollection);
    ExitOnFailure(hr, "Failed get filter collection");

    hr = pCollection->get_Count(&cFilters);
    ExitOnFailure(hr, "Failed get filter collection count");
    
    // Attempt to delete, we will we recreate with desired property values and order
    hr = DeleteCollectionElement(pCollection, IIS_CONFIG_FILTER, IIS_CONFIG_NAME, pwzFilterName);
    ExitOnFailure(hr, "Failed to delete filter");

     //make a new element
    hr = pCollection->CreateNewElement(ScopeBSTR(IIS_CONFIG_FILTER), &pElement);
    ExitOnFailure(hr, "Failed to create filter config element");

    hr = PutPropertyValue(pElement,IIS_CONFIG_NAME, pwzFilterName);
    ExitOnFailure(hr, "Failed to set filter name");

    // filter path
    hr = WcaReadStringFromCaData(ppwzCustomActionData, &pwzFilterPath);
    ExitOnFailure(hr, "Failed to read filter path");

    hr = PutPropertyValue(pElement, IIS_CONFIG_PATH, pwzFilterPath);
    ExitOnFailure(hr, "Failed to set filter path");

    //filter load order
    hr = WcaReadIntegerFromCaData(ppwzCustomActionData, &iLoadOrder);
    ExitOnFailure(hr, "Failed to read filter load order");

    //  put element in order in list
    int iPosition = -1;
    int icFilters = cFilters;
    switch (iLoadOrder)
    {
        case 0 :
        {
            iPosition = -1;
            break;
        }
        case -1 :
        {
            iPosition = icFilters;
            break;
        }
        case MSI_NULL_INTEGER :
        {
            iPosition = icFilters;
            break;
        }
        default:
        {
            if( iLoadOrder > icFilters)
            {
                iPosition = icFilters;
            }
            else
            {
                iPosition = iLoadOrder;
            }
            break;
        }
    }

    hr = pCollection->AddElement(pElement, iPosition);
    ExitOnFailure(hr, "Failed to add filter element");

LExit:
    ReleaseStr(pwzFilterName);
    ReleaseStr(pwzSiteName);
    ReleaseStr(pwzFilterPath);
    ReleaseStr(pwzConfigPath);
    ReleaseNullObject(pElement);
    ReleaseNullObject(pSection);
    ReleaseNullObject(pCollection);

    return hr;
}

static HRESULT DeleteSiteFilter(__inout LPWSTR *ppwzCustomActionData, IAppHostWritableAdminManager *pAdminMgr)
{
    HRESULT hr = S_OK;
    LPWSTR pwzFilterName = NULL;
    LPWSTR pwzSiteName = NULL;
    LPWSTR pwzConfigPath = NULL;

    IAppHostElement *pSection = NULL;
    IAppHostElementCollection *pCollection = NULL;

    //filter Name key
    hr = WcaReadStringFromCaData(ppwzCustomActionData, &pwzFilterName);
    ExitOnFailure(hr, "Failed to read filter name");

    //web site name
    hr = WcaReadStringFromCaData(ppwzCustomActionData, &pwzSiteName);
    ExitOnFailure(hr, "Failed to read filter site name");

     //Construct config root
    hr = StrAllocFormatted(&pwzConfigPath, L"%s/%s", IIS_CONFIG_APPHOST_ROOT, pwzSiteName);
    ExitOnFailure(hr, "failed to format filter config path");

    //get admin isapiFilters section at config path location tag
    hr = pAdminMgr->GetAdminSection(ScopeBSTR(IIS_CONFIG_ISAPI_SECTION), pwzConfigPath, &pSection);
    ExitOnFailure(hr, "Failed get isapiFilters section");

    hr = pSection->get_Collection(&pCollection);
    ExitOnFailure(hr, "Failed get filter collection");
    
    DeleteCollectionElement(pCollection, IIS_CONFIG_FILTER, IIS_CONFIG_NAME, pwzFilterName);
    ExitOnFailure1(hr, "Failed to delete filter %S", pwzFilterName);

LExit:
    ReleaseStr(pwzFilterName);
    ReleaseStr(pwzSiteName);
    ReleaseStr(pwzConfigPath);
    ReleaseNullObject(pSection);
    ReleaseNullObject(pCollection);

    return hr;
}

//-------------------------------------------------------------------------------------------------
// IIS7Site
// Called by WriteIIS7ConfigChanges
// Processes WebSite CA Data
//
//-------------------------------------------------------------------------------------------------
HRESULT IIS7Site(
    __inout LPWSTR *ppwzCustomActionData,
    __in    IAppHostWritableAdminManager *pAdminMgr)
{
    HRESULT hr  = S_OK;
    int iAction = -1;
    int iData   =  0;
    BOOL fFound = FALSE;

    LPWSTR pwzSiteName = NULL;
    IAppHostElement *pSites = NULL;
    IAppHostElementCollection *pCollection = NULL;
    IAppHostElement *pSiteElem = NULL;
    IAppHostProperty *pProperty = NULL;
    IAppHostElement *pElement = NULL;
    DWORD cSites = 0;

    // Get site action
    hr = WcaReadIntegerFromCaData(ppwzCustomActionData, &iAction);
    ExitOnFailure(hr, "Failed to read site action");

    //get site name
    hr = WcaReadStringFromCaData(ppwzCustomActionData, &pwzSiteName);
    ExitOnFailure(hr, "Failed to read site key");

    //Get site if it exists
    hr = GetSiteElement(pAdminMgr, pwzSiteName, &pSiteElem, &fFound);
    ExitOnFailure(hr, "Failed to read sites from config");
    
    hr = pAdminMgr->GetAdminSection(ScopeBSTR(IIS_CONFIG_SITES_SECTION), ScopeBSTR(IIS_CONFIG_APPHOST_ROOT), &pSites);
    ExitOnFailure(hr, "Failed get sites section");
    ExitOnNull(pSites, hr, ERROR_FILE_NOT_FOUND, "Failed get sites section object");
    
    hr = pSites->get_Collection( &pCollection);
    ExitOnFailure(hr, "Failed get site collection");
    switch (iAction)
    {
        case IIS_DELETE :
        {
            if (fFound)
            {
                hr = DeleteCollectionElement(pCollection, IIS_CONFIG_SITE, IIS_CONFIG_NAME, pwzSiteName);
                ExitOnFailure(hr, "Failed to delete website");
            }
            ExitFunction();
            break;
        }
        case IIS_CREATE_NEW :
        {
            if (fFound)
            {
                //can not configure existing site
                //TODO: this is broken on repair
                hr = HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS);
                ExitOnFailure(hr, "Unable modify existing WebSite");
            }
            else
            {
                //Create the site
                hr = CreateSite(pCollection, pwzSiteName, &pSiteElem);
                ExitOnFailure(hr, "Failed to create site");
            }
            break;
        }
        case IIS_CREATE :
        {
            if(!fFound)
            {
                //Create the site
                hr = CreateSite(pCollection, pwzSiteName, &pSiteElem);
                ExitOnFailure(hr, "Failed to create site");

            }
        }
    }
    //
    //Set other Site properties
    //
    //set site Id
    hr = WcaReadIntegerFromCaData(ppwzCustomActionData, &iData);
    ExitOnFailure(hr, "Failed to read site Id");
    if( iData != MSI_NULL_INTEGER )
    {
        hr = PutPropertyValue(pSiteElem, IIS_CONFIG_SITE_ID, iData);
        ExitOnFailure(hr, "Failed set site Id data" );
    }
    //Set Site AutoStart
    hr = WcaReadIntegerFromCaData(ppwzCustomActionData, &iData);
    ExitOnFailure(hr, "Failed to read site autostart");
    if( iData != MSI_NULL_INTEGER )
    {
        hr = PutPropertyBoolValue(pSiteElem, IIS_CONFIG_AUTOSTART, iData);
        ExitOnFailure(hr, "Failed set site config data" );
    }

    //Set Site Connection timeout
    hr = WcaReadIntegerFromCaData(ppwzCustomActionData, &iData);
    ExitOnFailure(hr, "Failed to read site connection tomeout data");
    if( iData != MSI_NULL_INTEGER )
    {
        // get limits element, get connectionTimeout property
        hr = pSiteElem->GetElementByName(ScopeBSTR(IIS_CONFIG_LIMITS), &pElement);
        ExitOnFailure(hr, "Failed to read limits from config" );
        //convert iData in seconds to timeSpan hh:mm:ss
        WCHAR wcTime[60];
        *wcTime = '\0';
        ConvSecToHMS( iData, wcTime, countof( wcTime));

        hr = PutPropertyValue(pElement, IIS_CONFIG_CONNECTTIMEOUT, wcTime);
        ExitOnFailure(hr, "IIS: failed set connection timeout config data" );
   }

LExit:
    ReleaseStr(pwzSiteName);
    ReleaseNullObject(pSites);
    ReleaseNullObject(pCollection);
    ReleaseNullObject(pSiteElem);
    ReleaseNullObject(pElement);
    ReleaseNullObject(pProperty);

    return hr;
}
//-------------------------------------------------------------------------------------------------
// IIS7Application
//  Processes Application CA Data
//
//
//-------------------------------------------------------------------------------------------------

HRESULT IIS7Application(
    __inout  LPWSTR *ppwzCustomActionData,
    __in     IAppHostWritableAdminManager *pAdminMgr)
{
    HRESULT hr = S_OK;

    int iAction = -1;
    int iData   =  0;
    BOOL fSiteFound = FALSE;
    BOOL fAppFound = FALSE;

    LPWSTR pwzSiteName = NULL;
    LPWSTR pwzAppPath = NULL;
    LPWSTR pwzAppPool = NULL;
    LPWSTR pwzLocationPath = NULL;
    IAppHostElement *pSiteElem = NULL;
    IAppHostElement *pAppElement = NULL;
    // Get Application action
    hr = WcaReadIntegerFromCaData( ppwzCustomActionData, &iAction);
    ExitOnFailure(hr, "Failed to read application action")
    //get site key name
    hr = WcaReadStringFromCaData(ppwzCustomActionData, &pwzSiteName);
    ExitOnFailure(hr, "Failed to read app site key");
    //get application path
    hr = WcaReadStringFromCaData(ppwzCustomActionData, &pwzAppPath);
    ExitOnFailure(hr, "Failed to read app path key");
    //get application Pool
    hr = WcaReadStringFromCaData(ppwzCustomActionData, &pwzAppPool);
    ExitOnFailure(hr, "Failed to read app pool key");

    //Get site if it exists
    hr = GetSiteElement(pAdminMgr, pwzSiteName, &pSiteElem, &fSiteFound);
    ExitOnFailure(hr, "Failed to read sites from config");

    switch (iAction)
    {
        case IIS_CREATE :
        {
            if(fSiteFound)
            {
                //have site get application collection
                hr = GetApplicationElement(pSiteElem,
                                            pwzAppPath,
                                            &pAppElement,
                                            &fAppFound);
                ExitOnFailure(hr, "Error reading application from config");

                if (!fAppFound)
                {
                    //Create Application
                    hr = CreateApplication(pSiteElem, pwzAppPath, &pAppElement);
                    ExitOnFailure(hr, "Error creating application in config");
                }
                //Update application properties:
                //
                //Set appPool
                hr = SetAppPool(pAppElement, pwzAppPool);
                ExitOnFailure(hr, "Unable to set appPool for application");
            }
            else
            {
                hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
                ExitOnFailure(hr, "Site not found for create application");
            }
            break;
        }
        case IIS_DELETE :
        {
            if(fSiteFound)
            {
                //have site get application collection
                hr = GetApplicationElement( pSiteElem,
                                            pwzAppPath,
                                            &pAppElement,
                                            &fAppFound);
                ExitOnFailure(hr, "Error reading application from config")
                if ( fAppFound )
                {
                    //delete Application
                    hr = DeleteApplication(pSiteElem, pwzAppPath);
                    ExitOnFailure(hr, "Error deleating application from config")
                    //Construct Location path 
                    // TODO: it seems odd that these are just 
                    // jammed together, need to determine if this requires a '\'
                    hr = StrAllocString(&pwzLocationPath, pwzSiteName, 0);
                    ExitOnFailure(hr, "failed to copy location config path web name");
                    hr = StrAllocConcat(&pwzLocationPath, pwzAppPath, 0);
                    ExitOnFailure(hr, "failed to copy location config path appPath ");

                    // and delete location tag for this application
                    hr = ClearLocationTag(pAdminMgr, pwzLocationPath);
                    ExitOnFailure1(hr, "failed to clear location tag for %S", pwzLocationPath);
                }
            }
            break;
        }
        default:
            ExitOnFailure(hr = E_UNEXPECTED, "Unexpected IIS Config action specified for Application");
            break;
    }

LExit:
    ReleaseStr(pwzSiteName);
    ReleaseStr(pwzAppPath);
    ReleaseStr(pwzAppPool);
    ReleaseStr(pwzLocationPath);
    ReleaseNullObject(pSiteElem);
    ReleaseNullObject(pAppElement);
    return hr;
}
//-------------------------------------------------------------------------------------------------
// IIS7VDir
//  Processes VDir CA Data
//
//
//-------------------------------------------------------------------------------------------------
HRESULT IIS7VDir(
    __inout  LPWSTR *ppwzCustomActionData,
    __in     IAppHostWritableAdminManager *pAdminMgr)
{
    HRESULT hr = S_OK;

    int iAction = -1;
    int iData   =  0;
    BOOL fSiteFound = FALSE;
    BOOL fAppFound = FALSE;
    
    LPWSTR pwzSiteName = NULL;
    LPWSTR pwzAppPath = NULL;
    LPWSTR pwzVDirPath = NULL;
    LPWSTR pwzVDirPhyDir = NULL;

    IAppHostElement *pSiteElem = NULL;
    IAppHostElement *pAppElement = NULL;
    IAppHostProperty *pProperty = NULL;
    IAppHostElementCollection *pElement = NULL;

    // Get Application action
    hr = WcaReadIntegerFromCaData(ppwzCustomActionData, &iAction);
    ExitOnFailure(hr, "Failed to read VDir action");

    //get site key name
    hr = WcaReadStringFromCaData(ppwzCustomActionData, &pwzSiteName);
    ExitOnFailure(hr, "Failed to read site key");
    //get application key path
    hr = WcaReadStringFromCaData(ppwzCustomActionData, &pwzAppPath);
    ExitOnFailure(hr, "Failed to read appPath key");
    //get VDir path
    hr = WcaReadStringFromCaData(ppwzCustomActionData, &pwzVDirPath);
    ExitOnFailure(hr, "Failed to read VDir key");
    //get physical dir path
    hr = WcaReadStringFromCaData(ppwzCustomActionData, &pwzVDirPhyDir);
    ExitOnFailure(hr, "Failed to read VDirPath key");

    //Get site if it exists
    hr = GetSiteElement(pAdminMgr, pwzSiteName, &pSiteElem, &fSiteFound );
    ExitOnFailure(hr, "Failed to read sites from config");

    if (IIS_CREATE == iAction)
    {
        if(fSiteFound)
        {
            //have site get application collection
            hr = GetApplicationElement( pSiteElem,
                                        pwzAppPath,
                                        &pAppElement,
                                        &fAppFound);
            ExitOnFailure(hr, "Error reading application element from config");

            if (!fAppFound)
            {
                // need application to add vDir
                hr = HRESULT_FROM_WIN32( ERROR_FILE_NOT_FOUND );
                ExitOnFailure(hr, "Error application not found for create VDir");
            }
            //
            // create the virDir
            //
            hr = CreateVdir( pAppElement, pwzVDirPath, pwzVDirPhyDir );
            ExitOnFailure(hr, "Failed to create vdir for application");
        }
        else
        {
            hr = HRESULT_FROM_WIN32( ERROR_FILE_NOT_FOUND );
            ExitOnFailure(hr, "IIS: site not found for create VDir");
        }
    }
    else if(IIS_DELETE == iAction)
    {
        if( fSiteFound )
        {
            //have site get application collection
            hr = GetApplicationElement( pSiteElem,
                                        pwzAppPath,
                                        &pAppElement,
                                        &fAppFound);
            ExitOnFailure(hr, "Error reading application from config")
            if ( fAppFound )
            {
                //delete vdir
                hr = DeleteVdir( pAppElement, pwzVDirPath );
                ExitOnFailure(hr, "Unable to delete vdir for application");
            }
        }
    }

 LExit:
    ReleaseStr(pwzSiteName);
    ReleaseStr(pwzAppPath);
    ReleaseStr(pwzVDirPath);
    ReleaseStr(pwzVDirPhyDir);
    ReleaseNullObject(pSiteElem);
    ReleaseNullObject(pAppElement);
    ReleaseNullObject(pProperty);
    ReleaseNullObject(pElement);

    return hr;
}

//-------------------------------------------------------------------------------------------------
// IIS7Binding
//  Processes Bindings CA Data
//
//
//-------------------------------------------------------------------------------------------------
HRESULT IIS7Binding(
    __inout  LPWSTR *ppwzCustomActionData,
    __in     IAppHostWritableAdminManager *pAdminMgr)
{
    HRESULT hr = S_OK;

    int iAction = -1;
    int iData   =  0;
    BOOL fSiteFound = FALSE;
    BOOL fAppFound = FALSE;
    
    LPWSTR pwzSiteName = NULL;
    LPWSTR pwzProtocol = NULL;
    LPWSTR pwzInfo = NULL;

    IAppHostElement *pSiteElem = NULL;

    // Get Application action
    hr = WcaReadIntegerFromCaData( ppwzCustomActionData, &iAction);
    ExitOnFailure(hr, "Failed to read binding action");

    //get site key name
    hr = WcaReadStringFromCaData(ppwzCustomActionData, &pwzSiteName);
    ExitOnFailure(hr, "Failed to read binding site name key");

    //get binding protocol
    hr = WcaReadStringFromCaData(ppwzCustomActionData, &pwzProtocol);
    ExitOnFailure(hr, "Failed to read binding protocol");

    //get binding info
    hr = WcaReadStringFromCaData(ppwzCustomActionData, &pwzInfo);
    ExitOnFailure(hr, "Failed to read binding info");

    //Get site if it exists
    hr = GetSiteElement(pAdminMgr, pwzSiteName, &pSiteElem, &fSiteFound);
    ExitOnFailure(hr, "Failed to read sites from config");

    if (IIS_CREATE == iAction)
    {
        if( fSiteFound )
        {
            //add binding
            hr = CreateBinding(pSiteElem, pwzProtocol, pwzInfo);
            ExitOnFailure(hr, "Failed to create site binding");
        }
        else
        {
            hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
            ExitOnFailure(hr, "Site not found for create binding");
        }
    }
    else if(IIS_DELETE == iAction)
    {
        if( fSiteFound )
        {
            //delete binding
            hr = DeleteBinding(pSiteElem, pwzProtocol, pwzInfo);
            ExitOnFailure(hr, "Failed to delete binding");
        }
    }

 LExit:
    ReleaseStr(pwzSiteName);
    ReleaseStr(pwzProtocol);
    ReleaseStr(pwzInfo);
    ReleaseNullObject(pSiteElem);

    return hr;
}
//-------------------------------------------------------------------------------------------------
// IIS7Binding
//  Processes WebLog CA Data
//
//
//-------------------------------------------------------------------------------------------------
HRESULT IIS7WebLog(
    __inout  LPWSTR *ppwzCustomActionData,
    __in     IAppHostWritableAdminManager *pAdminMgr)
{
    HRESULT hr = S_OK;

    BOOL fSiteFound = FALSE;
    BOOL fAppFound = FALSE;
    
    LPWSTR pwzSiteName = NULL;
    LPWSTR pwzLogFormat = NULL;

    IAppHostElement *pSiteElem = NULL;

    //get site key name
    hr = WcaReadStringFromCaData(ppwzCustomActionData, &pwzSiteName);
    ExitOnFailure(hr, "Failed to read web log site name key");

    //get log format
    hr = WcaReadStringFromCaData(ppwzCustomActionData, &pwzLogFormat);
    ExitOnFailure(hr, "Failed to read web log protocol");

    //Get site if it exists
    hr = GetSiteElement(pAdminMgr, pwzSiteName, &pSiteElem, &fSiteFound);
    ExitOnFailure(hr, "Failed to read web log sites from config");

    if(fSiteFound)
    {
        //add log format
        hr = CreateWebLog(pSiteElem, pwzLogFormat);
        if ( FAILED(hr) )
        {
         ExitOnFailure(hr, "Failed to create weblog file format");
        }
    }
    else
    {
        hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
        ExitOnFailure(hr, "Site not found for create weblog file format");
    }

 LExit:
    ReleaseStr(pwzSiteName);
    ReleaseStr(pwzLogFormat);
    ReleaseNullObject(pSiteElem);

    return hr;
}
//-------------------------------------------------------------------------------------------------
// IIS7AppPool
//  Processes AppPool CA Data
//
//
//-------------------------------------------------------------------------------------------------
HRESULT IIS7AppPool(
    __inout  LPWSTR *ppwzCustomActionData,
    __in     IAppHostWritableAdminManager *pAdminMgr
    )
{
    HRESULT hr = S_OK;

    int iAction = -1;
    int iData   =  0;

    BOOL fAppPoolFound = FALSE;
    LPWSTR pwzAppPoolName = NULL;

    // Get AppPool action
    hr = WcaReadIntegerFromCaData( ppwzCustomActionData, &iAction);
    ExitOnFailure(hr, "Failed to read AppPool action");

    //get appPool name
    hr = WcaReadStringFromCaData(ppwzCustomActionData, &pwzAppPoolName);
    ExitOnFailure(hr, "Failed to read AppPool name key");

    switch (iAction)
    {
        case IIS_CREATE :
        {
            hr = CreateAppPool(ppwzCustomActionData, pAdminMgr, pwzAppPoolName);
            break;
        }
        case IIS_DELETE:
        {
            hr = DeleteAppPool(pAdminMgr, pwzAppPoolName);
            break;
        }
        default:
            ExitOnFailure(hr = E_UNEXPECTED, "Unexpected IIS Config action specified for appPool");
            break;
    }

LExit:
    ReleaseStr(pwzAppPoolName);
    return hr;
}

//-------------------------------------------------------------------------------------------------
// IIS7AppExtension
//  Processes AppExtension (config handlers) CA Data
//
//
//-------------------------------------------------------------------------------------------------
HRESULT IIS7AppExtension(
    __inout  LPWSTR *ppwzCustomActionData,
    __in     IAppHostWritableAdminManager *pAdminMgr)
{
   HRESULT hr = S_OK;
   
   LPWSTR pwzWebName = NULL;
   LPWSTR pwzWebRoot = NULL;
   LPWSTR pwzData = NULL;
   LPWSTR pwzConfigPath = NULL;
   LPWSTR pwzHandlerName = NULL;
   LPWSTR pwzPath = NULL;
   int iAction = -1;

   IAppHostElement *pSection = NULL;
   IAppHostElement *pElement = NULL;
   IAppHostProperty *pProperty = NULL;
   IAppHostElementCollection *pCollection = NULL;

   DWORD cHandlers = 1000;

    //get web name
    hr = WcaReadStringFromCaData(ppwzCustomActionData, &pwzWebName);
    ExitOnFailure(hr, "Failed to read appExt Web name key");

    //get root name
    hr = WcaReadStringFromCaData(ppwzCustomActionData, &pwzWebRoot);
    ExitOnFailure(hr, "Failed to read appExt Web name key");

    //Construct config root
    hr = StrAllocFormatted(&pwzConfigPath, L"%s/%s", IIS_CONFIG_APPHOST_ROOT, pwzWebName);
    ExitOnFailure(hr, "failed to format appext config path");
    //
    //Do not append trailing '/' for default vDir
    //    
    if(CSTR_EQUAL != ::CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, pwzWebRoot, -1, L"/", -1))
    {   
        hr = StrAllocConcat(&pwzConfigPath, L"/", 0);
        ExitOnFailure(hr, "failed to copy appext config path delim");
        hr = StrAllocConcat(&pwzConfigPath, pwzWebRoot, 0);
        ExitOnFailure(hr, "failed to copy appext config path root name");
    }
    //get admin handlers section at config path location tag
    hr = pAdminMgr->GetAdminSection(ScopeBSTR(IIS_CONFIG_HANDLERS_SECTION), pwzConfigPath, &pSection);
    ExitOnFailure(hr, "Failed get appext section");

    if(!pSection)
    {
        hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
        ExitOnFailure(hr, "Failed get appext section object");
    }
    
    // Get AppExt action
    hr = WcaReadIntegerFromCaData(ppwzCustomActionData, &iAction);
    ExitOnFailure(hr, "Failed to read appExt action");

    hr = pSection->get_Collection(&pCollection);
    ExitOnFailure(hr, "Failed get handlers collection for appext");

    while (IIS_APPEXT_END != iAction)
    {
        //Process property action
        switch (iAction)
        {
            case IIS_APPEXT :
            {
                //create new handler element
                hr = pCollection->CreateNewElement(ScopeBSTR(IIS_CONFIG_ADD), &pElement);
                ExitOnFailure(hr, "Failed get create handler element for appext");
                //create new unique name for handler
                //this set of names is unique to this location tag
                cHandlers = cHandlers + 1;

                hr = StrAllocFormatted(&pwzHandlerName, L"MsiCustom-%d", cHandlers);
                ExitOnFailure(hr, "Failed increment handler name");
                hr = PutPropertyValue(pElement, IIS_CONFIG_NAME, pwzHandlerName);
                ExitOnFailure(hr, "Failed set appext name property");

                //BUGBUG: For compat we are assuming these are all ISAPI MODULES so we are
                //setting the modules property to IsapiModule.
                //Currently can't deal with handlers of different module types.
                hr = PutPropertyValue(pElement, IIS_CONFIG_MODULES, L"IsapiModule");
                ExitOnFailure(hr, "Failed set site appExt path property");

                //get extension (path)
                hr = WcaReadStringFromCaData(ppwzCustomActionData, &pwzData);
                ExitOnFailure(hr, "Failed to read appExt extension");
                hr = StrAllocFormatted(&pwzPath, L"*.%s", pwzData);
                ExitOnFailure(hr, "Failed decorate appExt path");
                //put property
                hr = PutPropertyValue(pElement, IIS_CONFIG_PATH, pwzPath);
                ExitOnFailure(hr, "Failed set site appExt path property");

                //get executable
                hr = WcaReadStringFromCaData(ppwzCustomActionData, &pwzData);
                ExitOnFailure(hr, "Failed to read appExt executable");
                //put property
                hr = PutPropertyValue(pElement, IIS_CONFIG_EXECUTABLE, pwzData);
                ExitOnFailure(hr, "Failed set site appExt executable property");

                //get verbs
                hr = WcaReadStringFromCaData(ppwzCustomActionData, &pwzData);
                ExitOnFailure(hr, "Failed to read appExt verbs");
                //put property
                hr = PutPropertyValue(pElement, IIS_CONFIG_VERBS, pwzData);
                ExitOnFailure(hr, "Failed set site appExt verbs property");

                break;
            }
            default:
            {
                ExitOnFailure(hr = E_UNEXPECTED, "Unexpected IIS Config action specified for AppExt");
                break;
            }
        }
        //  put handler element at beginning of list
        hr = pCollection->AddElement(pElement, 0);
        ExitOnFailure(hr, "Failed add handler element for appext");

        // Get AppExt action
        hr = WcaReadIntegerFromCaData(ppwzCustomActionData, &iAction);
        ExitOnFailure(hr, "Failed to read AppPool Property action");
    }

LExit:
    ReleaseStr(pwzWebName);
    ReleaseStr(pwzWebRoot);
    ReleaseStr(pwzData);
    ReleaseStr(pwzConfigPath);
    ReleaseStr(pwzHandlerName);
    ReleaseStr(pwzPath);
    ReleaseNullObject(pSection);
    ReleaseNullObject(pElement);
    ReleaseNullObject(pProperty);
    ReleaseNullObject(pCollection);

    return hr;
}

//-------------------------------------------------------------------------------------------------
// IIS7MimeMap
//  Processes Mime Map (config handlers) CA Data
//
//
//-------------------------------------------------------------------------------------------------
 HRESULT IIS7MimeMap(
    __inout  LPWSTR *ppwzCustomActionData,
    __in     IAppHostWritableAdminManager *pAdminMgr
    )
{
    HRESULT hr = S_OK;
    LPWSTR pwzConfigPath = NULL;
    LPWSTR pwzWebName = NULL;
    LPWSTR pwzWebRoot = NULL;
    LPWSTR pwzData = NULL;
    int iAction = -1;

    IAppHostElement *pSection = NULL;
    IAppHostElement *pElement = NULL;
    IAppHostProperty *pProperty = NULL;
    IAppHostElementCollection *pCollection = NULL;

    BOOL fFound = FALSE;

    //get web name
    hr = WcaReadStringFromCaData(ppwzCustomActionData, &pwzWebName);
    ExitOnFailure(hr, "Failed to read appExt Web name key");

    //get vdir root name
    hr = WcaReadStringFromCaData(ppwzCustomActionData, &pwzWebRoot);
    ExitOnFailure(hr, "Failed to read appExt Web name key");

    //Construct config root
    hr = StrAllocFormatted(&pwzConfigPath, L"%s/%s",  IIS_CONFIG_APPHOST_ROOT, pwzWebName);
    ExitOnFailure(hr, "failed to format mime map config path web name");
    //
    //Do not append trailing '/' for default vDir
    //    
    if(CSTR_EQUAL != ::CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, pwzWebRoot, -1, L"/", -1))
    {   
        hr = StrAllocConcat(&pwzConfigPath, L"/", 0);
        ExitOnFailure(hr, "failed to copy appext config path delim");
        hr = StrAllocConcat(&pwzConfigPath, pwzWebRoot, 0);
        ExitOnFailure(hr, "failed to copy appext config path root name");
    }

    //get admin section <staticContent> at config path location tag
    hr = pAdminMgr->GetAdminSection(ScopeBSTR(IIS_CONFIG_STATICCONTENT_SECTION), pwzConfigPath, &pSection);
    ExitOnFailure(hr, "Failed get staticContent section for mimemap");

    if(!pSection)
    {
        hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
        ExitOnFailure(hr, "Failed get staticContent section object");
    }

    // Get mimemap action
    hr = WcaReadIntegerFromCaData(ppwzCustomActionData, &iAction);
    ExitOnFailure(hr, "Failed to read mimemap action");

    hr = pSection->get_Collection(&pCollection);
    ExitOnFailure(hr, "Failed get staticContent collection for mimemap");

    while (IIS_MIMEMAP_END != iAction)
    {
        //Process property action
        switch (iAction)
        {
            case IIS_MIMEMAP :
            {
                //get extension
                hr = WcaReadStringFromCaData(ppwzCustomActionData, &pwzData);
                ExitOnFailure(hr, "Failed to read mimemap extension");

                hr = FindAppHostElement(pCollection, IIS_CONFIG_MIMEMAP, IIS_CONFIG_FILEEXT, pwzData, &pElement, NULL);
                ExitOnFailure(hr, "Failed to find mimemap extension");
                fFound = (NULL != pElement);

                if (!fFound)
                {
                    //create new mimeMap element
                    hr = pCollection->CreateNewElement(ScopeBSTR(IIS_CONFIG_MIMEMAP), &pElement);
                    ExitOnFailure(hr, "Failed get create MimeMap element");
                }

                //put property
                hr = PutPropertyValue(pElement, IIS_CONFIG_FILEEXT, pwzData);
                ExitOnFailure(hr, "Failed set mimemap extension property");

                //get type
                hr = WcaReadStringFromCaData(ppwzCustomActionData, &pwzData);
                ExitOnFailure(hr, "Failed to read mimemap type");
                //put property
                hr = PutPropertyValue(pElement, IIS_CONFIG_MIMETYPE, pwzData);
                ExitOnFailure(hr, "Failed set mimemap type property");

                break;
            }
            default:
            {
                ExitOnFailure(hr = E_UNEXPECTED, "Unexpected IIS Config action specified for mimeMap");
                break;
            }
        }

        if (!fFound)
        {
            //  put mimeMap element at beginning of list
            hr = pCollection->AddElement(pElement, -1);
            ExitOnFailure(hr, "Failed add mimemap");
        }

        // Get AppExt action
        hr = WcaReadIntegerFromCaData(ppwzCustomActionData, &iAction);
        ExitOnFailure(hr, "Failed to read mimemap action");
        
        ReleaseNullObject(pElement);
    }

LExit:
    ReleaseStr(pwzConfigPath);
    ReleaseStr(pwzWebName);
    ReleaseStr(pwzWebRoot);
    ReleaseStr(pwzData);
    ReleaseNullObject(pSection);
    ReleaseNullObject(pElement);
    ReleaseNullObject(pProperty);
    ReleaseNullObject(pCollection);

    return hr;
}

//-------------------------------------------------------------------------------------------------
// IIS7DirProperties
//  ProcessesVdir Properties  CA Data
//
//
//-------------------------------------------------------------------------------------------------
HRESULT IIS7DirProperties(
    __inout  LPWSTR *ppwzCustomActionData,
    __in     IAppHostWritableAdminManager *pAdminMgr
    )
{
    HRESULT hr = S_OK;
    WCHAR wcTime[60];
    LPWSTR pwzConfigPath = NULL;
    LPWSTR pwzWebName = NULL;
    LPWSTR pwzWebRoot = NULL;
    LPWSTR pwzData = NULL;
    int iAction = -1;
    int iData   =  0;
    DWORD dwData = 0;

    IAppHostElement *pSection = NULL;
    IAppHostElement *pElement = NULL;
    IAppHostProperty *pProperty = NULL;
    IAppHostElementCollection *pCollection = NULL;

    //get web name
    hr = WcaReadStringFromCaData(ppwzCustomActionData, &pwzWebName);
    ExitOnFailure(hr, "Failed to read DirProp Web name key");

    //get vdir root name
    hr = WcaReadStringFromCaData(ppwzCustomActionData, &pwzWebRoot);
    ExitOnFailure(hr, "Failed to read DirProp Web name key");
    
    //Construct config root
    hr = StrAllocFormatted(&pwzConfigPath, L"%s/%s",  IIS_CONFIG_APPHOST_ROOT, pwzWebName);
    ExitOnFailure(hr, "failed to format mime map config path web name");
    //
    //Do not append trailing '/' for default vDir
    //    
    if(CSTR_EQUAL != ::CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, pwzWebRoot, -1, L"/", -1))
    {   
        hr = StrAllocConcat(&pwzConfigPath, L"/", 0);
        ExitOnFailure(hr, "failed to copy appext config path delim");
        hr = StrAllocConcat(&pwzConfigPath, pwzWebRoot, 0);
        ExitOnFailure(hr, "failed to copy appext config path root name");
    }
    
    // Get DirProps action
    hr = WcaReadIntegerFromCaData(ppwzCustomActionData, &iAction);
    ExitOnFailure(hr, "Failed to read DirProps action");

    while (IIS_DIRPROP_END != iAction)
    {
        //Process property action
        switch (iAction)
        {
            case IIS_DIRPROP_ACCESS :
            {
                hr = WcaReadIntegerFromCaData(ppwzCustomActionData, &iData);
                ExitOnFailure(hr, "Failed to read DirProps access");
                //iData contains bit flags for <handlers accessPolicy="">
                //no translation required
                //get admin section at config path location tag
                hr = pAdminMgr->GetAdminSection(ScopeBSTR(IIS_CONFIG_HANDLERS_SECTION), pwzConfigPath, &pSection);
                ExitOnFailure(hr, "Failed get handlers section for DirProp");
                if(!pSection)
                {
                    hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
                    ExitOnFailure(hr, "Failed get handlers section object for DirProps");
                }
                dwData = iData;
                hr = PutPropertyValue( pSection, L"accessPolicy", dwData);
                ExitOnFailure(hr, "Failed set accessPolicy for DirProps");
                ReleaseNullObject(pSection);
                break;
            }
            case IIS_DIRPROP_USER :
            {
                hr = WcaReadStringFromCaData(ppwzCustomActionData, &pwzData);
                ExitOnFailure(hr, "Failed to read DirProps user");
                hr = pAdminMgr->GetAdminSection(ScopeBSTR(L"system.webServer/security/authentication/anonymousAuthentication"), pwzConfigPath, &pSection);
                ExitOnFailure(hr, "Failed get AnonymousAuthentication section for DirProp");
                hr = PutPropertyValue( pSection, IIS_CONFIG_USERNAME, pwzData);
                ExitOnFailure(hr, "Failed set accessPolicy for DirProps");
                ReleaseNullObject(pSection);
                break;
            }
            case IIS_DIRPROP_PWD :
            {
                hr = WcaReadStringFromCaData(ppwzCustomActionData, &pwzData);
                ExitOnFailure(hr, "Failed to read DirProps pwd");
                hr = pAdminMgr->GetAdminSection(ScopeBSTR(L"system.webServer/security/authentication/anonymousAuthentication"), pwzConfigPath, &pSection);
                ExitOnFailure(hr, "Failed get AnonymousAuthentication section for DirProp");
                hr = PutPropertyValue( pSection, IIS_CONFIG_PASSWORD, pwzData);
                ExitOnFailure(hr, "Failed set accessPolicy for DirProps");
                ReleaseNullObject(pSection);
                break;
            }
            case IIS_DIRPROP_DEFDOCS :
            {
                hr = WcaReadStringFromCaData(ppwzCustomActionData, &pwzData);
                ExitOnFailure(hr, "Failed to read DirProps def doc");
                hr = SetDirPropDefDoc(pAdminMgr, pwzConfigPath, pwzData);
                ExitOnFailure(hr, "Failed to set DirProps Default Documents");
                break;
            }
            case IIS_DIRPROP_AUTH :
            {
                hr = WcaReadIntegerFromCaData(ppwzCustomActionData, &iData);
                ExitOnFailure(hr, "Failed to read DirProps auth");
                //iData contains bit flags for /security/authentication/<...>
                // Anonymous    = 1
                // Basic        = 2
                // Windows      = 4
                // Digest       =16
                // Passport     =64  *not supported
                //translation required from bit map to section
                // E.G security/authentication/windowsAuthentication [property enabled true|false]
                dwData= iData;
                hr = SetDirPropAuthentications(pAdminMgr, pwzConfigPath, dwData);
                ExitOnFailure(hr, "Failed set Authentication for DirProps");
                break;
            }
            case IIS_DIRPROP_SSLFLAGS :
            {
                hr = WcaReadIntegerFromCaData(ppwzCustomActionData, &iData);
                ExitOnFailure(hr, "Failed to read DirProps sslFlags");
                //iData contains bit flags for /security/access sslFlags
                //no translation required
                hr = pAdminMgr->GetAdminSection(ScopeBSTR(L"system.webServer/security/access"), pwzConfigPath, &pSection);
                ExitOnFailure(hr, "Failed get security/access section for DirProp");
                dwData = iData;
                hr = PutPropertyValue( pSection, L"sslFlags", dwData);
                ExitOnFailure(hr, "Failed set security/access for DirProps");
                ReleaseNullObject(pSection);
                break;
            }
            case IIS_DIRPROP_AUTHPROVID :
            {
                hr = WcaReadStringFromCaData(ppwzCustomActionData, &pwzData);
                ExitOnFailure(hr, "Failed to read DirProps auth provider");
                hr = SetDirPropAuthProvider(pAdminMgr, pwzConfigPath, pwzData);
                ExitOnFailure(hr, "Failed to set DirProps auth provider");
                break;
            }
            case IIS_DIRPROP_ASPERROR:
            {
                hr = WcaReadIntegerFromCaData(ppwzCustomActionData, &iData);
                ExitOnFailure(hr, "Failed to read DirProps aspDetailedError");
                hr = pAdminMgr->GetAdminSection(ScopeBSTR(IIS_CONFIG_ASP_SECTION), pwzConfigPath, &pSection);
                ExitOnFailure(hr, "Failed get asp section for DirProp");
                hr = PutPropertyBoolValue(pSection, IIS_CONFIG_SCRIPTERROR, iData);
                ExitOnFailure(hr, "Failed to set DirProps aspDetailedError");
                ReleaseNullObject(pSection);
                break;
            }
            case IIS_DIRPROP_HTTPEXPIRES:
            {
                hr = WcaReadStringFromCaData(ppwzCustomActionData, &pwzData);
                ExitOnFailure(hr, "Failed to read DirProps httpExpires provider");
                hr = pAdminMgr->GetAdminSection(ScopeBSTR(IIS_CONFIG_STATICCONTENT_SECTION), pwzConfigPath, &pSection);
                ExitOnFailure(hr, "Failed get staticContent section for DirProp");
                hr = pSection->GetElementByName(ScopeBSTR(IIS_CONFIG_CLIENTCACHE), &pElement);
                ExitOnFailure(hr, "Failed to get clientCache element");
                hr = PutPropertyValue(pElement, IIS_CONFIG_HTTPEXPIRES, pwzData);
                ExitOnFailure(hr, "Failed to set clientCache httpExpires value");
                ReleaseNullObject(pSection);
                ReleaseNullObject(pElement);
                break;
            }
            case IIS_DIRPROP_MAXAGE:
            {
                hr = WcaReadIntegerFromCaData(ppwzCustomActionData, &iData);
                ExitOnFailure(hr, "Failed to read DirProps httpExpires provider");
                hr = pAdminMgr->GetAdminSection(ScopeBSTR(IIS_CONFIG_STATICCONTENT_SECTION), pwzConfigPath, &pSection);
                ExitOnFailure(hr, "Failed get staticContent section for DirProp");
                hr = pSection->GetElementByName(ScopeBSTR(IIS_CONFIG_CLIENTCACHE), &pElement);
                ExitOnFailure(hr, "Failed to get clientCache element");
                *wcTime = '\0';
                ConvSecToDHMS(iData, wcTime, countof(wcTime));
                hr = PutPropertyValue(pElement, IIS_CONFIG_MAXAGE, wcTime);
                ExitOnFailure(hr, "Failed to set clientCache maxAge value");
                ReleaseNullObject(pSection);
                ReleaseNullObject(pElement);
                break;
            }
            case IIS_DIRPROP_CACHECUST:
            {
                hr = WcaReadStringFromCaData(ppwzCustomActionData, &pwzData);
                ExitOnFailure(hr, "Failed to read DirProps cacheControlCustom");
                hr = pAdminMgr->GetAdminSection(ScopeBSTR(IIS_CONFIG_STATICCONTENT_SECTION), pwzConfigPath, &pSection);
                ExitOnFailure(hr, "Failed get staticContent section for DirProp");
                hr = pSection->GetElementByName(ScopeBSTR(IIS_CONFIG_CLIENTCACHE), &pElement);
                ExitOnFailure(hr, "Failed to get clientCache element");
                hr = PutPropertyValue(pElement, IIS_CONFIG_CACHECUST, pwzData);
                ExitOnFailure(hr, "Failed to set clientCache cacheControlCustom value");
                ReleaseNullObject(pSection);
                ReleaseNullObject(pElement);
                break;
            }
            case IIS_DIRPROP_NOCUSTERROR:
            {
                //no value, if have ID tag write clear to system.webServer/httpErrors
                //error collection
                hr = pAdminMgr->GetAdminSection(ScopeBSTR(IIS_CONFIG_HTTPERRORS_SECTION), pwzConfigPath, &pSection);
                ExitOnFailure(hr, "Failed get httperrors section for DirProp");
                hr = pSection->get_Collection(&pCollection);
                ExitOnFailure(hr, "Failed get error collection for DirProp");
                hr = pCollection->CreateNewElement(ScopeBSTR(IIS_CONFIG_CLEAR), &pElement);
                ExitOnFailure(hr, "Failed to create clear element for error collection for DirProp");
                hr = pCollection->AddElement(pElement);
                ExitOnFailure(hr, "Failed to add lear element for error collection for DirProp");
                ReleaseNullObject(pSection);
                ReleaseNullObject(pElement);
                break;
            }
            default:
            {
                ExitOnFailure(hr = E_UNEXPECTED, "Unexpected IIS Config action specified for WebDirProperties");
                break;
            }
        }

        // Get AppExt action
        hr = WcaReadIntegerFromCaData(ppwzCustomActionData, &iAction);
        ExitOnFailure(hr, "Failed to read DirProps Property action");
    }
LExit:
    ReleaseStr(pwzConfigPath);
    ReleaseStr(pwzWebName);
    ReleaseStr(pwzWebRoot);
    ReleaseStr(pwzData);
    ReleaseNullObject(pSection);
    ReleaseNullObject(pElement);
    ReleaseNullObject(pProperty);
    ReleaseNullObject(pCollection);

    return hr;
}

//-------------------------------------------------------------------------------------------------
// IIS7SslBinding
//  ProcessesVdir Properties  CA Data
//
//
//-------------------------------------------------------------------------------------------------
HRESULT IIS7SslBinding(
    __inout  LPWSTR *ppwzCustomActionData,
    __in     IAppHostWritableAdminManager *pAdminMgr
    )
{
    HRESULT hr = S_OK;

    int iAction = -1;
    int iData   =  0;
    BOOL fSiteFound = FALSE;
    
    LPWSTR pwzSiteName = NULL;
    LPWSTR pwzStoreName = NULL;
    LPWSTR pwzEncodedCertificateHash = NULL;

    IAppHostElement *pSiteElem = NULL;

    // Get Application action
    hr = WcaReadIntegerFromCaData( ppwzCustomActionData, &iAction);
    ExitOnFailure(hr, "Failed to read binding action");

    //get site key name
    hr = WcaReadStringFromCaData(ppwzCustomActionData, &pwzSiteName);
    ExitOnFailure(hr, "Failed to read binding site name key");

    //get binding protocol
    hr = WcaReadStringFromCaData(ppwzCustomActionData, &pwzStoreName);
    ExitOnFailure(hr, "Failed to read binding protocol");

    //get binding info
    hr = WcaReadStringFromCaData(ppwzCustomActionData, &pwzEncodedCertificateHash);
    ExitOnFailure(hr, "Failed to read binding info");

    //Get site if it exists
    hr = GetSiteElement(pAdminMgr, pwzSiteName, &pSiteElem, &fSiteFound);
    ExitOnFailure(hr, "Failed to read sites from config");

    if (IIS_CREATE == iAction)
    {
        if( fSiteFound )
        {
            //add SSL cert to binding
            hr = CreateSslBinding(pSiteElem, pwzStoreName, pwzEncodedCertificateHash);
            ExitOnFailure(hr, "Failed to create site binding");
        }
        else
        {
            hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
            ExitOnFailure(hr, "Site not found for create binding");
        }
    }
    else if(IIS_DELETE == iAction)
    {
        if( fSiteFound )
        {
            //delete binding
            hr = DeleteSslBinding(pSiteElem, pwzStoreName, pwzEncodedCertificateHash);
            ExitOnFailure(hr, "Failed to delete binding");
        }
    }

 LExit:
    ReleaseStr(pwzSiteName);
    ReleaseStr(pwzStoreName);
    ReleaseStr(pwzEncodedCertificateHash);
    ReleaseNullObject(pSiteElem);

    return hr;
}

//-------------------------------------------------------------------------------------------------
// Helper Functions
//
//
//
//-------------------------------------------------------------------------------------------------

static HRESULT GetNextAvailableSiteId( IAppHostElementCollection *pCollection,
                                       DWORD *plSiteId
                                       )
{
   HRESULT hr = S_OK;
   IAppHostElement *pElement = NULL;
   IAppHostProperty *pProperty = NULL;

   DWORD cSites;
   DWORD plNextAvailSite  = 0;
   VARIANT vPropValue;
   VARIANT vtIndex;

   VariantInit(&vPropValue);
   VariantInit(&vtIndex);

   *plSiteId = 0;

    hr = pCollection->get_Count(&cSites);
    ExitOnFailure(hr, "Failed get sites collection count");

    vtIndex.vt = VT_UI4;
    for( DWORD i = 0; i < cSites; i++ )
    {
        vtIndex.ulVal = i;
        hr = pCollection->get_Item(vtIndex , &pElement);
        ExitOnFailure(hr, "Failed get sites collection item");

        hr = pElement->GetPropertyByName(ScopeBSTR(IIS_CONFIG_ID), &pProperty);
        ExitOnFailure(hr, "Failed get site property");

        hr = pProperty->get_Value(&vPropValue);
        ExitOnFailure(hr, "Failed get site property value");

        *plSiteId = vPropValue.lVal;
        if( *plSiteId > plNextAvailSite )
        {
            plNextAvailSite = *plSiteId;
        }
        ReleaseNullObject(pElement);
        ReleaseNullObject(pProperty);
    }
    *plSiteId = ++plNextAvailSite;

LExit:
    VariantClear(&vPropValue);
    VariantClear(&vtIndex);

    ReleaseNullObject(pElement);
    ReleaseNullObject(pProperty);

    return hr;
}

static HRESULT GetSiteElement( IAppHostWritableAdminManager *pAdminMgr,
                                LPCWSTR swSiteName,
                                IAppHostElement **ppSiteElement,
                                BOOL* fFound
                                )
{
   HRESULT hr = S_OK;
   IAppHostElement *pSites = NULL;
   IAppHostElementCollection *pCollection = NULL;

   *fFound = FALSE;

    hr = pAdminMgr->GetAdminSection(ScopeBSTR(IIS_CONFIG_SITES_SECTION), ScopeBSTR(IIS_CONFIG_APPHOST_ROOT), &pSites);
    ExitOnFailure(hr, "Failed get sites section");
    ExitOnNull(pSites, hr, ERROR_FILE_NOT_FOUND, "Failed get sites section object");

    hr = pSites->get_Collection(&pCollection);
    ExitOnFailure(hr, "Failed get sites collection");

    hr = FindAppHostElement(pCollection, IIS_CONFIG_SITE, IIS_CONFIG_NAME, swSiteName, ppSiteElement, NULL);
    ExitOnFailure1(hr, "Failed to find site %S", swSiteName);

    *fFound = ppSiteElement != NULL && *ppSiteElement != NULL;

LExit:
    ReleaseNullObject(pSites);
    ReleaseNullObject(pCollection);

    return hr;
}

static HRESULT GetApplicationElement( IAppHostElement *pSiteElement,
                                      LPCWSTR swAppPath,
                                      IAppHostElement **ppAppElement,
                                      BOOL* fFound)
{
   HRESULT hr = S_OK;
   IAppHostElement *pElement = NULL;
   IAppHostElementCollection *pCollection = NULL;

   *fFound = FALSE;

    hr = pSiteElement->get_Collection( &pCollection);
    ExitOnFailure(hr, "Failed get site app collection");
    
    hr = FindAppHostElement(pCollection, IIS_CONFIG_APPLICATION, IIS_CONFIG_PATH, swAppPath, ppAppElement, NULL);
    ExitOnFailure1(hr, "Failed to find app %S", swAppPath);
    
    *fFound = ppAppElement != NULL && *ppAppElement != NULL;

LExit:
    ReleaseNullObject(pElement);
    ReleaseNullObject(pCollection);

    return hr;
}

static HRESULT CreateSite( __in IAppHostElementCollection *pCollection,
                           __in LPCWSTR swSiteName,
                           __out IAppHostElement **pSiteElement
                           )
{
    HRESULT hr = S_OK;
    IAppHostElement *pNewElement = NULL;
    IAppHostProperty *pProperty = NULL;
    
    hr = pCollection->CreateNewElement(ScopeBSTR(IIS_CONFIG_SITE), &pNewElement);
    ExitOnFailure(hr, "Failed create site element");

    hr = PutPropertyValue(pNewElement, IIS_CONFIG_NAME, swSiteName);
    ExitOnFailure(hr, "Failed set site name property");

    DWORD lSiteId = 0;
    hr = GetNextAvailableSiteId(pCollection, &lSiteId);
    ExitOnFailure(hr, "Failed get next site id");

    PutPropertyValue(pNewElement, IIS_CONFIG_ID, lSiteId);
    ExitOnFailure(hr, "Failed set site id property");

    hr = pCollection->AddElement(pNewElement);
    ExitOnFailure(hr, "Failed add site element");

    *pSiteElement = pNewElement;
    pNewElement = NULL;

LExit:
    ReleaseNullObject(pProperty);
    ReleaseNullObject(pNewElement);

    return hr;
}

static HRESULT CreateApplication( IAppHostElement *pSiteElement,
                                  LPCWSTR swAppPath,
                                  IAppHostElement **pAppElement
                                )
{
    HRESULT hr = S_OK;
    IAppHostElement *pNewElement = NULL;
    IAppHostProperty *pProperty = NULL;
    IAppHostElementCollection *pCollection = NULL;

    hr = pSiteElement->get_Collection(&pCollection);
    ExitOnFailure(hr, "Failed get application collection");

    hr = pCollection->CreateNewElement(ScopeBSTR(IIS_CONFIG_APPLICATION), &pNewElement);
    ExitOnFailure(hr, "Failed get application element");

    hr = PutPropertyValue(pNewElement, IIS_CONFIG_PATH, swAppPath);
    ExitOnFailure(hr, "Failed set application path property");

    hr = pCollection->AddElement(pNewElement);
    ExitOnFailure(hr, "Failed add application to collection");

    *pAppElement = pNewElement;
    pNewElement = NULL;

LExit:
    ReleaseNullObject(pProperty);
    ReleaseNullObject(pCollection);
    ReleaseNullObject(pNewElement);

    return hr;
}

static HRESULT DeleteApplication(  IAppHostElement *pSiteElement,
                                   LPCWSTR swAppPath
                                  )
{
    HRESULT hr = S_OK;
    IAppHostElementCollection *pCollection = NULL;

    hr = pSiteElement->get_Collection(&pCollection);
    ExitOnFailure(hr, "Failed get application collection");
    
    hr = DeleteCollectionElement(pCollection, IIS_CONFIG_APPLICATION, IIS_CONFIG_PATH, swAppPath);
    ExitOnFailure(hr, "Failed to delete website");

LExit:
    ReleaseNullObject(pCollection);

    return hr;
}

static HRESULT SetAppPool( IAppHostElement *pAppElement,
                           LPCWSTR pwzAppPool
                           )
{
    HRESULT hr = S_OK;

    if(*pwzAppPool != 0)
    {
        hr = PutPropertyValue(pAppElement, IIS_CONFIG_APPPOOL, pwzAppPool);
        ExitOnFailure(hr, "Failed set application appPool property");
    }
LExit:
    return hr;
}

static HRESULT CreateVdir( IAppHostElement *pAppElement,
                           LPCWSTR pwzVDirPath,
                           LPCWSTR pwzVDirPhyDir
                          )
{
    HRESULT hr = S_OK;
    IAppHostElement *pElement = NULL;
    IAppHostProperty *pProperty = NULL;
    IAppHostElementCollection *pCollection = NULL;
    BOOL fFound;

    hr = pAppElement->get_Collection(&pCollection);
    ExitOnFailure(hr, "Failed get application VDir collection");
    
    hr = FindAppHostElement(pCollection, IIS_CONFIG_VDIR, IIS_CONFIG_PATH, pwzVDirPath, &pElement, NULL);
    ExitOnFailure(hr, "Failed while finding virtualDir");
    fFound = (NULL != pElement);

    if (!fFound)
    {
        hr = pCollection->CreateNewElement(ScopeBSTR(IIS_CONFIG_VDIR), &pElement);
        ExitOnFailure(hr, "Failed create application VDir collection");

        hr = PutPropertyValue(pElement, IIS_CONFIG_PATH, pwzVDirPath);
        ExitOnFailure(hr, "Failed set VDir path property");
    }
    
    hr = PutPropertyValue(pElement, IIS_CONFIG_PHYSPATH, pwzVDirPhyDir);
    ExitOnFailure(hr, "Failed set VDir phys path property");

    if (!fFound)
    {
        hr = pCollection->AddElement(pElement);
        ExitOnFailure(hr, "Failed add application VDir element");
    }

LExit:
    ReleaseNullObject(pCollection);
    ReleaseNullObject(pElement);
    ReleaseNullObject(pProperty);

    return hr;
}

static HRESULT DeleteVdir( IAppHostElement *pAppElement,
                           LPCWSTR pwzVDirPath
                          )
{
    HRESULT hr = S_OK;
    IAppHostElementCollection *pCollection = NULL;

    hr = pAppElement->get_Collection(&pCollection);
    ExitOnFailure(hr, "Failed get application VDir collection");
    
    hr = DeleteCollectionElement(pCollection, IIS_CONFIG_VDIR, IIS_CONFIG_PATH, pwzVDirPath);
    ExitOnFailure(hr, "Failed to delete vdir");

LExit:
    ReleaseNullObject(pCollection);

    return hr;
}

static HRESULT CreateBinding( IAppHostElement *pSiteElem,
                              LPCWSTR pwzProtocol,
                              LPCWSTR pwzInfo
                              )
{
    HRESULT hr = S_OK;
    IAppHostChildElementCollection *pChildElems = NULL;
    IAppHostElement *pBindings = NULL;
    IAppHostElement *pBindingElement = NULL;
    IAppHostProperty *pProperty = NULL;
    IAppHostElementCollection *pCollection = NULL;

    VARIANT vtProp;

    VariantInit(&vtProp);

    hr = pSiteElem->get_ChildElements(&pChildElems);
    ExitOnFailure(hr, "Failed get site child elements collection");

    vtProp.vt = VT_BSTR;
    vtProp.bstrVal = ::SysAllocString(IIS_CONFIG_BINDINGS);
    hr = pChildElems->get_Item(vtProp, &pBindings);
    ExitOnFailure(hr, "Failed get bindings element");
    VariantClear(&vtProp);

    hr = pBindings->get_Collection(&pCollection);
    ExitOnFailure(hr, "Failed get bindings collection");

    hr = pCollection->CreateNewElement(ScopeBSTR(IIS_CONFIG_BINDING), &pBindingElement);
    ExitOnFailure(hr, "Failed get binding element");

    hr = PutPropertyValue(pBindingElement, IIS_CONFIG_PROTOCOL, pwzProtocol);
    ExitOnFailure(hr, "Failed set binding protocol property");

    hr = PutPropertyValue(pBindingElement, IIS_CONFIG_BINDINGINFO, pwzInfo);
    ExitOnFailure(hr, "Failed set binding information property");

    hr = pCollection->AddElement(pBindingElement);
    if(hr == HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS))
    {
        //Eat this error. Binding is there and nothing to repair since 
        //identity == protocol + info so all is OK
        hr = S_OK;
    }
    else
    {
        ExitOnFailure(hr, "Failed add binding to site");
    }

LExit:
    VariantClear(&vtProp);

    ReleaseNullObject(pCollection);
    ReleaseNullObject(pChildElems);
    ReleaseNullObject(pBindingElement);
    ReleaseNullObject(pBindings);
    ReleaseNullObject(pProperty);

    return hr;
}
static HRESULT CreateWebLog( IAppHostElement *pSiteElem,
                              LPCWSTR pwzFormat
                              )
{
    HRESULT hr = S_OK;
    IAppHostChildElementCollection *pChildElems = NULL;
    IAppHostElement *pLogFile = NULL;

    VARIANT vtProp;

    VariantInit(&vtProp);

    hr = pSiteElem->get_ChildElements(&pChildElems);
    ExitOnFailure(hr, "Failed get site child elements collection");

    vtProp.vt = VT_BSTR;
    vtProp.bstrVal = ::SysAllocString(IIS_CONFIG_WEBLOG);
    hr = pChildElems->get_Item(vtProp, &pLogFile);
    ExitOnFailure(hr, "Failed get logfile element");
    VariantClear(&vtProp);
    
    if(CSTR_EQUAL != ::CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, pwzFormat, -1, L"none", -1))
    {
        hr = PutPropertyValue(pLogFile, IIS_CONFIG_LOGFORMAT, pwzFormat);
        ExitOnFailure(hr, "Failed set logfile format property");
        hr = PutPropertyValue(pLogFile, IIS_CONFIG_ENABLED, IIS_CONFIG_TRUE);
        ExitOnFailure(hr, "Failed set logfile enabled property");
    }
    else
    {
        hr = PutPropertyValue(pLogFile, IIS_CONFIG_ENABLED, IIS_CONFIG_FALSE);
        ExitOnFailure(hr, "Failed set logfile enabled property");
    }

LExit:
    VariantClear(&vtProp);

    ReleaseNullObject(pLogFile);
    ReleaseNullObject(pChildElems);

    return hr;
}

static HRESULT DeleteBinding( IAppHostElement *pSiteElem,
                              LPCWSTR pwzProtocol,
                              LPCWSTR pwzInfo
                              )
{
    HRESULT hr = S_OK;
    //
    //this isn't supported right now, we should support this for the SiteSearch scenario
    return hr;
}

static HRESULT AddSslCertificateToBinding(IAppHostElement *pBindingElement, LPCWSTR pwzStoreName, LPCWSTR pwzEncodedCertificateHash )
{
    HRESULT hr = S_OK;
    IAppHostMethodCollection *pAppHostMethodCollection = NULL;
    IAppHostMethod *pAddSslMethod = NULL;
    IAppHostMethodInstance *pAddSslMethodInstance = NULL;
    IAppHostElement *pAddSslInput = NULL;

    hr = pBindingElement->get_Methods(&pAppHostMethodCollection);
    ExitOnFailure(hr, "failed to get binding method collection");

    hr = FindAppHostMethod(pAppHostMethodCollection, L"AddSslCertificate", &pAddSslMethod, NULL);
    if (FAILED(hr))
    {
        WcaLog(LOGMSG_STANDARD, "The AddSslCertificate method is not supported by the binding element, SSL certificate will not be associated with the website");
        ExitFunction();
    }

    pAddSslMethod->CreateInstance(&pAddSslMethodInstance);
    ExitOnFailure(hr, "failed to create an instance of AddSslCertificate method");

    pAddSslMethodInstance->get_Input(&pAddSslInput);
    ExitOnFailure(hr, "failed to get input element of AddSslCertificate method");
    
    PutPropertyValue(pAddSslInput, IIS_CONFIG_CERTIFICATESTORENAME, pwzStoreName);
    ExitOnFailure(hr, "failed to set certificateStoreName input parameter of AddSslCertificate method");

    PutPropertyValue(pAddSslInput, IIS_CONFIG_CERTIFICATEHASH, pwzEncodedCertificateHash);
    ExitOnFailure(hr, "failed to set certificateHash input parameter of AddSslCertificate method");

    hr = pAddSslMethodInstance->Execute();
    ExitOnFailure(hr, "failed to execute AddSslCertificate method");
LExit:
    ReleaseNullObject(pAppHostMethodCollection);
    ReleaseNullObject(pAddSslMethod);
    ReleaseNullObject(pAddSslMethodInstance);
    ReleaseNullObject(pAddSslInput);
    return hr;
}


static HRESULT CreateSslBinding( IAppHostElement *pSiteElem, LPCWSTR pwzStoreName, LPCWSTR pwzEncodedCertificateHash )
{
    HRESULT hr = S_OK;
    DWORD dwElements = 0;
    IAppHostChildElementCollection *pChildElems = NULL;
    IAppHostElement *pBindingsElement = NULL;
    IAppHostElementCollection *pBindingsCollection = NULL;
    IAppHostElement *pBindingElement = NULL;

    VARIANT vtProp;
    VARIANT vtIndex;
    VariantInit(&vtIndex);
    VariantInit(&vtProp);

    hr = pSiteElem->get_ChildElements(&pChildElems);
    ExitOnFailure(hr, "Failed get site child elements collection");

    vtProp.vt = VT_BSTR;
    vtProp.bstrVal = ::SysAllocString(IIS_CONFIG_BINDINGS);
    hr = pChildElems->get_Item(vtProp, &pBindingsElement);
    ExitOnFailure(hr, "Failed get bindings element");
    VariantClear(&vtProp);

    hr = pBindingsElement->get_Collection(&pBindingsCollection);
    ExitOnFailure(hr, "Failed get bindings collection");

    // Our current IISWebSiteCertificates schema does not allow specification of the website binding
    // to associate the certificate with.  For now just associate it with all secure bindings.

    // Loop over all bindings    
    hr = pBindingsCollection->get_Count(&dwElements);
    ExitOnFailure(hr, "Failed get site bindings collection count");

    vtIndex.vt = VT_UI4;
    for( DWORD i = 0; i < dwElements; i++ )
    {
        vtIndex.ulVal = i;
        hr = pBindingsCollection->get_Item(vtIndex , &pBindingElement);
        ExitOnFailure(hr, "Failed get binding element");

        if (IsMatchingAppHostElement(pBindingElement, IIS_CONFIG_BINDING, IIS_CONFIG_PROTOCOL, L"https"))
        {
            //Secure binding
            hr = AddSslCertificateToBinding(pBindingElement, pwzStoreName, pwzEncodedCertificateHash );
            ExitOnFailure(hr, "Failed to associate ssl certificate with binding");
        }

        ReleaseNullObject(pBindingElement);
    }


LExit:
    VariantClear(&vtProp);
    VariantClear(&vtIndex);

    ReleaseNullObject(pChildElems);
    ReleaseNullObject(pBindingsElement);
    ReleaseNullObject(pBindingsCollection);
    ReleaseNullObject(pBindingElement);

    return hr;
}
static HRESULT DeleteSslBinding( IAppHostElement *pSiteElem, LPCWSTR pwzStoreName, LPCWSTR pwzEncodedCertificateHash )
{
    HRESULT hr = S_OK;
    //
    //this isn't supported right now, we should support this for the SiteSearch scenario
    return hr;
}
static HRESULT DeleteAppPool( IAppHostWritableAdminManager *pAdminMgr,
                            LPCWSTR swAppPoolName)
{
    HRESULT hr = S_OK;
    IAppHostElement *pAppPools = NULL;
    IAppHostElementCollection *pCollection = NULL;

    hr = pAdminMgr->GetAdminSection(ScopeBSTR(IIS_CONFIG_APPPOOL_SECTION), ScopeBSTR(IIS_CONFIG_APPHOST_ROOT), &pAppPools);
    ExitOnFailure(hr, "Failed get AppPools section");
    ExitOnNull(pAppPools, hr, E_UNEXPECTED, "Failed get appPools section object");

    hr = pAppPools->get_Collection( &pCollection);
    ExitOnFailure(hr, "Failed get AppPools collection");
    
    hr = DeleteCollectionElement(pCollection, IIS_CONFIG_ADD, IIS_CONFIG_NAME, swAppPoolName);
    ExitOnFailure1(hr, "Failed to delete app pool %S", swAppPoolName);

LExit:
    ReleaseNullObject(pAppPools);
    ReleaseNullObject(pCollection);

    return hr;
}

static HRESULT CreateAppPool( __inout LPWSTR *ppwzCustomActionData,
                              IAppHostWritableAdminManager *pAdminMgr,
                              LPCWSTR swAppPoolName
                           )
{
    HRESULT hr = S_OK;
    IAppHostElement *pAppPools = NULL;
    IAppHostElement *pAppPoolElement = NULL;
    IAppHostElement *pElement = NULL;
    IAppHostElement *pElement2 = NULL;
    IAppHostElement *pElement3 = NULL;
    IAppHostProperty *pProperty = NULL;
    IAppHostElementCollection *pCollection = NULL;
    IAppHostElementCollection *pCollection2 = NULL;
    int iAction = -1;
    int iData   =  0;
    LPWSTR pwzData = NULL;
    WCHAR wcData[512];
    WCHAR wcTime[60];
    BOOL fFound = FALSE;

    hr = pAdminMgr->GetAdminSection(ScopeBSTR(IIS_CONFIG_APPPOOL_SECTION), ScopeBSTR(IIS_CONFIG_APPHOST_ROOT), &pAppPools);
    ExitOnFailure(hr, "Failed get AppPools section");
    ExitOnNull(pAppPools, hr, ERROR_FILE_NOT_FOUND, "Failed get AppPools section object");

    hr = pAppPools->get_Collection( &pCollection);
    ExitOnFailure(hr, "Failed get AppPools collection");

    hr = FindAppHostElement(pCollection, IIS_CONFIG_ADD, IIS_CONFIG_NAME, swAppPoolName, &pAppPoolElement, NULL);
    ExitOnFailure(hr, "Failed find AppPool element");
    fFound = (NULL != pAppPoolElement);
    
    if (!fFound)
    {
        hr = pCollection->CreateNewElement(ScopeBSTR(IIS_CONFIG_ADD), &pAppPoolElement);
        ExitOnFailure(hr, "Failed create AppPool element");
    }

    hr = PutPropertyValue(pAppPoolElement, IIS_CONFIG_NAME, swAppPoolName);
    ExitOnFailure(hr, "Failed set AppPool name property");

    //For WiX II6 /ABO compat we will be hardcoding managedPipelineMode="Classic"
    hr = PutPropertyValue(pAppPoolElement, IIS_CONFIG_PIPELINEMODE, L"Classic");
    ExitOnFailure(hr, "Failed set AppPool name property");
    //For WiX II6 /ABO compat we will be hardcoding autostart="true"
    hr = PutPropertyValue(pAppPoolElement, IIS_CONFIG_APPPOOL_AUTO, L"true");
    ExitOnFailure(hr, "Failed set AppPool name property");

    if (!fFound)
    {
        hr = pCollection->AddElement(pAppPoolElement);
        ExitOnFailure(hr, "Failed to add appPool element");
    }

    // Get AppPool Property action
    hr = WcaReadIntegerFromCaData(ppwzCustomActionData, &iAction);
    ExitOnFailure(hr, "Failed to read AppPool Property action");
    while (IIS_APPPOOL_END != iAction)
    {
        //Process property action
        switch (iAction)
        {
            case IIS_APPPOOL_RECYCLE_MIN :
            {
            // /recycling / periodicRestart | time
                hr = WcaReadIntegerFromCaData(ppwzCustomActionData, &iData);
                ExitOnFailure(hr, "Failed to read AppPool recycle min");
                hr = pAppPoolElement->GetElementByName(ScopeBSTR(IIS_CONFIG_RECYCLING), &pElement);
                ExitOnFailure(hr, "Failed to get AppPool recycling element");
                hr = pElement->GetElementByName(ScopeBSTR(IIS_CONFIG_PEROIDRESTART), &pElement2);
                ExitOnFailure(hr, "Failed to get AppPool periodicRestart element");
                *wcTime = '\0';
                ConvSecToHMS(iData * 60, wcTime, countof(wcTime));
                hr = PutPropertyValue(pElement2, IIS_CONFIG_TIME, wcTime);
                ExitOnFailure(hr, "Failed to set AppPool periodicRestart time value");
                ReleaseNullObject(pElement);
                ReleaseNullObject(pElement2);
                break;
            }
            case IIS_APPPOOL_RECYCLE_REQ :
            {
            // /recycling / periodicRestart | requests
                hr = WcaReadIntegerFromCaData(ppwzCustomActionData, &iData);
                ExitOnFailure(hr, "Failed to read AppPool recycle req");
                hr = pAppPoolElement->GetElementByName(ScopeBSTR(IIS_CONFIG_RECYCLING), &pElement);
                ExitOnFailure(hr, "Failed to get AppPool recycling element");
                hr = pElement->GetElementByName(ScopeBSTR(IIS_CONFIG_PEROIDRESTART), &pElement2);
                ExitOnFailure(hr, "Failed to get AppPool periodicRestart element");
                hr = PutPropertyValue(pElement2, IIS_CONFIG_REQUESTS, iData);
                ExitOnFailure(hr, "Failed to set AppPool periodicRestart time value");
                ReleaseNullObject(pElement);
                ReleaseNullObject(pElement2);
                break;
            }
            case IIS_APPPOOL_RECYCLE_TIMES :
            {
            // /recycling / periodicRestart | schedule
                hr = WcaReadStringFromCaData(ppwzCustomActionData, &pwzData);
                ExitOnFailure(hr, "Failed to read AppPool recycle times");
                hr = pAppPoolElement->GetElementByName(ScopeBSTR(IIS_CONFIG_RECYCLING), &pElement);
                ExitOnFailure(hr, "Failed to get AppPool recycling element");
                hr = pElement->GetElementByName(ScopeBSTR(IIS_CONFIG_PEROIDRESTART), &pElement2);
                ExitOnFailure(hr, "Failed to get AppPool periodicRestart element");
                hr = pElement2->GetElementByName(ScopeBSTR(IIS_CONFIG_SCHEDULE), &pElement3);
                ExitOnFailure(hr, "Failed to get AppPool schedule element");
                hr = pElement3->get_Collection(&pCollection2);
                ExitOnFailure(hr, "Failed to get AppPool schedule collection");

                WCHAR wcDelim[] = L",";
                const WCHAR *wszToken = NULL;
                WCHAR *wszNextToken = NULL;
                wszToken = wcstok_s( pwzData, wcDelim, &wszNextToken);

                while (wszToken)
                {
                    *wcData = '\0';
                    hr = ::StringCchCopyW(wcData, countof(wcData), wszToken);
                    ExitOnFailure(hr, "failed to copy AppPool schedule");
                    hr = ::StringCchCatW(wcData, countof(wcData), L":00");
                    ExitOnFailure(hr, "failed to append AppPool schedule");

                    hr = pCollection2->CreateNewElement(ScopeBSTR(IIS_CONFIG_ADD), &pElement3);
                    ExitOnFailure(hr, "Failed to create AppPool schedule element");

                    hr = PutPropertyValue(pElement3, IIS_CONFIG_VALUE, wcData);
                    ExitOnFailure(hr, "Failed to set AppPool schedule value");

                    hr = pCollection2->AddElement(pElement3);
                    if(hr == HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS))
                    {
                        //Eat this error, recycle time already exists NBD
                        hr = S_OK;
                    }
                    ExitOnFailure(hr, "Failed to add win auth providers element");
                    ReleaseNullObject(pElement3);
                    wszToken = wcstok_s( NULL, wcDelim, &wszNextToken);
                }
                ReleaseNullObject(pElement);
                ReleaseNullObject(pElement2);
                ReleaseNullObject(pElement3);
                ReleaseNullObject(pCollection2);
                break;
            }
            case IIS_APPPOOL_RECYCLE_VIRMEM :
            {
            // /recycling / periodicRestart | memory
                hr = WcaReadIntegerFromCaData(ppwzCustomActionData, &iData);
                ExitOnFailure(hr, "Failed to read AppPool recycle vir mem");
                hr = pAppPoolElement->GetElementByName(ScopeBSTR(IIS_CONFIG_RECYCLING), &pElement);
                ExitOnFailure(hr, "Failed to get AppPool recycling element");
                hr = pElement->GetElementByName(ScopeBSTR(IIS_CONFIG_PEROIDRESTART), &pElement2);
                ExitOnFailure(hr, "Failed to get AppPool periodicRestart element");
                hr = PutPropertyValue(pElement2, IIS_CONFIG_MEMORY, iData);
                ExitOnFailure(hr, "Failed to set AppPool periodicRestart memory");
                ReleaseNullObject(pElement);
                ReleaseNullObject(pElement2);
                break;
            }
            case IIS_APPPOOL_RECYCLE_PRIVMEM :
            {
            // /recycling / periodicRestart | privateMemory
                hr = WcaReadIntegerFromCaData(ppwzCustomActionData, &iData);
                ExitOnFailure(hr, "Failed to read AppPool recycle priv mem");
                hr = pAppPoolElement->GetElementByName(ScopeBSTR(IIS_CONFIG_RECYCLING), &pElement);
                ExitOnFailure(hr, "Failed to get AppPool recycling element");
                hr = pElement->GetElementByName(ScopeBSTR(IIS_CONFIG_PEROIDRESTART), &pElement2);
                ExitOnFailure(hr, "Failed to get AppPool periodicRestart element");
                hr = PutPropertyValue(pElement2, IIS_CONFIG_PRIVMEMORY, iData);
                ExitOnFailure(hr, "Failed to set AppPool periodicRestart private memory");
                ReleaseNullObject(pElement);
                ReleaseNullObject(pElement2);
                break;
            }
            case IIS_APPPOOL_RECYCLE_IDLTIMEOUT :
            {
            //  /processModel | idleTimeout
                hr = WcaReadIntegerFromCaData(ppwzCustomActionData, &iData);
                ExitOnFailure(hr, "Failed to read AppPool recycle priv mem");
                hr = pAppPoolElement->GetElementByName(ScopeBSTR(IIS_CONFIG_PROCESSMODEL), &pElement);
                ExitOnFailure(hr, "Failed to get AppPool processModel element");
                *wcTime = '\0';
                ConvSecToHMS(iData, wcTime, countof(wcTime));
                hr = PutPropertyValue(pElement, IIS_CONFIG_IDLETIMEOUT, wcTime);
                ExitOnFailure(hr, "Failed to set AppPool processModel idle timeout value");
                ReleaseNullObject(pElement);
                break;
            }
            case IIS_APPPOOL_RECYCLE_QUEUELIMIT :
            {
            //  /applicationPools | queueLength
                hr = WcaReadIntegerFromCaData(ppwzCustomActionData, &iData);
                ExitOnFailure(hr, "Failed to read AppPool recycle query limit");
                hr = PutPropertyValue(pAppPoolElement, IIS_CONFIG_QUEUELENGTH, iData);
                ExitOnFailure(hr, "Failed to set AppPool processModel idle timeout value");
                break;
            }
            case IIS_APPPOOL_MAXPROCESS :
            {
            //  /processModel | maxProcesses
                hr = WcaReadIntegerFromCaData(ppwzCustomActionData, &iData);
                ExitOnFailure(hr, "Failed to read AppPool max processes");
                hr = pAppPoolElement->GetElementByName(ScopeBSTR(IIS_CONFIG_PROCESSMODEL), &pElement);
                ExitOnFailure(hr, "Failed to get AppPool processModel element");
                hr = PutPropertyValue(pElement, IIS_CONFIG_MAXWRKPROCESSES, iData);
                ExitOnFailure(hr, "Failed to set AppPool processModel maxProcesses value");
                ReleaseNullObject(pElement);
                break;
            }
            case IIS_APPPOOL_IDENTITY :
            {
            //"LocalSystem" 0
            //"LocalService" 1
            //"NetworkService" 2
            //"SpecificUser" 3
            //  /processModel | identityType
                hr = WcaReadIntegerFromCaData(ppwzCustomActionData, &iData);
                ExitOnFailure(hr, "Failed to read AppPool identity");
                hr = pAppPoolElement->GetElementByName(ScopeBSTR(IIS_CONFIG_PROCESSMODEL), &pElement);
                ExitOnFailure(hr, "Failed to get AppPool processModel element");
                if(iData == 0)
                {
                    hr = PutPropertyValue(pElement, IIS_CONFIG_IDENITITYTYPE, IIS_CONFIG_LOCALSYSTEM);
                }
                else if (iData == 1)
                {
                    hr = PutPropertyValue(pElement, IIS_CONFIG_IDENITITYTYPE, IIS_CONFIG_LOCALSERVICE);
                }
                else if (iData == 2)
                {
                    hr = PutPropertyValue(pElement, IIS_CONFIG_IDENITITYTYPE, IIS_CONFIG_NETWORKSERVICE);
                }
                else if (iData == 3)
                {
                    hr = PutPropertyValue(pElement, IIS_CONFIG_IDENITITYTYPE, IIS_CONFIG_SPECIFICUSER);
                }
                ExitOnFailure(hr, "Failed to set AppPool processModel identityType value");
                ReleaseNullObject(pElement);
                break;
            }
            case IIS_APPPOOL_USER :
            {
            //  /processModel | userName
                hr = WcaReadStringFromCaData(ppwzCustomActionData, &pwzData);
                ExitOnFailure(hr, "Failed to read AppPool user");
                hr = pAppPoolElement->GetElementByName(ScopeBSTR(IIS_CONFIG_PROCESSMODEL), &pElement);
                ExitOnFailure(hr, "Failed to get AppPool processModel element");
                hr = PutPropertyValue(pElement, IIS_CONFIG_USERNAME, pwzData);
                ExitOnFailure(hr, "Failed to set AppPool processModel username value");
                ReleaseNullObject(pElement);
                break;
            }
            case IIS_APPPOOL_PWD :
            {
            //  /processModel | password
                hr = WcaReadStringFromCaData(ppwzCustomActionData, &pwzData);
                ExitOnFailure(hr, "Failed to read AppPool pwd");
                hr = pAppPoolElement->GetElementByName(ScopeBSTR(IIS_CONFIG_PROCESSMODEL), &pElement);
                ExitOnFailure(hr, "Failed to get AppPool processModel element");
                hr = PutPropertyValue(pElement, IIS_CONFIG_PASSWORD, pwzData);
                ExitOnFailure(hr, "Failed to set AppPool processModel password value");
                ReleaseNullObject(pElement);
                break;
            }
            case  IIS_APPPOOL_RECYCLE_CPU_PCT:
            {
            // /cpu | limit
                hr = WcaReadIntegerFromCaData(ppwzCustomActionData, &iData);
                ExitOnFailure(hr, "Failed to read cpu pct");
                hr = pAppPoolElement->GetElementByName(ScopeBSTR(IIS_CONFIG_CPU), &pElement);
                ExitOnFailure(hr, "Failed to get AppPool cpu element");
                hr = PutPropertyValue(pElement, IIS_CONFIG_LIMIT, iData * 100);
                ExitOnFailure(hr, "Failed to set AppPool cpu limit");
                ReleaseNullObject(pElement);
                break;
            }
            case  IIS_APPPOOL_RECYCLE_CPU_REFRESH:
            {
            // /cpu | resetInterval
                hr = WcaReadIntegerFromCaData(ppwzCustomActionData, &iData);
                ExitOnFailure(hr, "Failed to read cpu refresh pwd");
                hr = pAppPoolElement->GetElementByName(ScopeBSTR(IIS_CONFIG_CPU), &pElement);
                ExitOnFailure(hr, "Failed to get AppPool cpu element");
                *wcTime = '\0';
                ConvSecToHMS(iData * 60, wcTime, countof(wcTime));
                hr = PutPropertyValue(pElement, IIS_CONFIG_RESETINTERVAL, wcTime);
                ExitOnFailure(hr, "Failed to set AppPool cpu resetInterval value");
                ReleaseNullObject(pElement);
                break;
            }
            case  IIS_APPPOOL_RECYCLE_CPU_ACTION:
            {
            // /cpu | action
            //"NoAction" 0
            //"KillW3wp" 1
                hr = WcaReadIntegerFromCaData(ppwzCustomActionData, &iData);
                ExitOnFailure(hr, "Failed to read cpu action");
                hr = pAppPoolElement->GetElementByName(ScopeBSTR(IIS_CONFIG_CPU), &pElement);
                ExitOnFailure(hr, "Failed to get AppPool cpu element");
                if(iData)
                {
                    hr = PutPropertyValue(pElement, IIS_CONFIG_ACTION, IIS_CONFIG_KILLW3WP);
                }
                else
                {
                    hr = PutPropertyValue(pElement, IIS_CONFIG_ACTION, IIS_CONFIG_NOACTION);
                }
                ExitOnFailure(hr, "Failed to set AppPool cpu action value");
                ReleaseNullObject(pElement);
                break;
            }

            default:
            ExitOnFailure(hr = E_UNEXPECTED, "Unexpected IIS Config action specified for AppPool");
            break;

        }
        // Get AppPool property action
        hr = WcaReadIntegerFromCaData(ppwzCustomActionData, &iAction);
        ExitOnFailure(hr, "Failed to read AppPool Property action");
    }

LExit:
    ReleaseNullObject(pAppPools);
    ReleaseNullObject(pProperty);
    ReleaseNullObject(pCollection);
    ReleaseNullObject(pCollection2);
    ReleaseNullObject(pAppPoolElement);
    ReleaseNullObject(pElement);
    ReleaseNullObject(pElement2);
    ReleaseNullObject(pElement3);
    return hr;
}

static HRESULT SetDirPropAuthentications(IAppHostWritableAdminManager *pAdminMgr,
                                         LPCWSTR wszConfigPath,
                                         DWORD dwData)
{
    HRESULT hr = S_OK;
    IAppHostElement *pSection = NULL;

    //dwData contains bit flags for /security/authentication/<...>
    // Anonymous    = 1
    // Basic        = 2
    // Windows      = 4
    // Digest       =16
    // Passport     =64  *not supported
    //translation required from bit map to section name
    // E.G security/authentication/windowsAuthentication [property enabled true|false]

    // AnonymousAuthentication = 1
    hr = pAdminMgr->GetAdminSection(ScopeBSTR(L"system.webServer/security/authentication/anonymousAuthentication"), ScopeBSTR(wszConfigPath), &pSection);
    ExitOnFailure(hr, "Failed get AnonymousAuthentication section for DirProp");
    if(!pSection)
    {
        hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
        ExitOnFailure(hr, "Failed get AnonymousAuthentication section object for DirProps");
    }

    hr = PutPropertyValue(pSection, L"enabled", (BOOL)(dwData & 0x1));
    ExitOnFailure(hr, "Failed set AnonymousAuthentication enabled for DirProps");
    ReleaseNullObject(pSection);

    // basicAuthentication = 2
    hr = pAdminMgr->GetAdminSection(ScopeBSTR(L"system.webServer/security/authentication/basicAuthentication"), ScopeBSTR(wszConfigPath), &pSection);
    ExitOnFailure(hr, "Failed get basicAuthentication section for DirProp");
    if(!pSection)
    {
        hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
        ExitOnFailure(hr, "Failed get basicAuthentication section object for DirProps");
    }

    hr = PutPropertyValue(pSection, L"enabled", (BOOL)(dwData & 0x2));
    ExitOnFailure(hr, "Failed set basicAuthentication enabled for DirProps");
    ReleaseNullObject(pSection);

    // WindowsAuthentication = 4
    hr = pAdminMgr->GetAdminSection(ScopeBSTR(L"system.webServer/security/authentication/windowsAuthentication"), ScopeBSTR(wszConfigPath), &pSection);
    ExitOnFailure(hr, "Failed get windowsAuthentication section for DirProp");
    if(!pSection)
    {
        hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
        ExitOnFailure(hr, "Failed get windowsAuthentication section object for DirProps");
    }

    hr = PutPropertyValue(pSection, L"enabled", (BOOL)(dwData & 0x4));
    ExitOnFailure(hr, "Failed set windowsAuthentication enabled for DirProps");
    ReleaseNullObject(pSection);

    // digestAuthentication = 16
    hr = pAdminMgr->GetAdminSection(ScopeBSTR(L"system.webServer/security/authentication/digestAuthentication"), ScopeBSTR(wszConfigPath), &pSection);
    ExitOnFailure(hr, "Failed get digestAuthentication section for DirProp");
    if(!pSection)
    {
        hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
        ExitOnFailure(hr, "Failed get digestAuthentication section object for DirProps");
    }

    hr = PutPropertyValue(pSection, L"enabled", (BOOL)(dwData & 0x10));
    ExitOnFailure(hr, "Failed set digestAuthentication enabled for DirProps");
    ReleaseNullObject(pSection);

LExit:
    ReleaseNullObject(pSection);
    return hr;
}

static HRESULT SetDirPropAuthProvider(IAppHostWritableAdminManager *pAdminMgr,
                                         LPCWSTR wszConfigPath,
                                         __in LPWSTR wszData)
{
    HRESULT hr = S_OK;
    IAppHostElement *pSection = NULL;
    IAppHostElement *pElement = NULL;
    IAppHostElement *pNewElement = NULL;
    IAppHostElementCollection *pCollection = NULL;

    WCHAR wcDelim[] = L",";
    const WCHAR *wszToken = NULL;
    WCHAR *wszNextToken = NULL;

    hr = pAdminMgr->GetAdminSection(ScopeBSTR(L"system.webServer/security/authentication/windowsAuthentication"), ScopeBSTR(wszConfigPath), &pSection);
    ExitOnFailure(hr, "Failed get windowsAuthentication section for DirProp providers");

    hr = pSection->GetElementByName(ScopeBSTR(L"providers"), &pElement);
    ExitOnFailure(hr, "Failed get win auth providers section");

    hr = pElement->get_Collection(&pCollection);
    ExitOnFailure(hr, "Failed get win auth providers collection");

    hr = pCollection->Clear();
    ExitOnFailure(hr, "Failed to clear win auth providers collection");

    //Clear out inherited items - add clear
    hr = pCollection->CreateNewElement(ScopeBSTR(IIS_CONFIG_CLEAR), &pNewElement);
    ExitOnFailure(hr, "Failed to create win auth providers clear element");
    hr = pCollection->AddElement(pNewElement);
    ExitOnFailure(hr, "Failed to add win auth providers clear element");
    
    wszToken = wcstok_s( wszData, wcDelim, &wszNextToken);
    for( int i = 0; (wszToken); i++)
    {
        hr = pCollection->CreateNewElement(ScopeBSTR(IIS_CONFIG_ADD), &pNewElement);
        ExitOnFailure(hr, "Failed to create win auth providers element");

        hr = PutPropertyValue( pNewElement, IIS_CONFIG_VALUE, wszToken);
        ExitOnFailure(hr, "Failed to set win auth providers value");

        hr = pCollection->AddElement(pNewElement, i);
        ExitOnFailure(hr, "Failed to add win auth providers element");
        ReleaseNullObject(pNewElement);

        wszToken = wcstok_s( NULL, wcDelim, &wszNextToken);
    }

LExit:
    ReleaseNullObject(pSection);
    ReleaseNullObject(pCollection);
    ReleaseNullObject(pNewElement);
    return hr;
}

static HRESULT SetDirPropDefDoc(IAppHostWritableAdminManager *pAdminMgr,
                                         LPCWSTR wszConfigPath,
                                         __in LPWSTR wszData)
{
    HRESULT hr = S_OK;
    IAppHostElement *pSection = NULL;
    IAppHostElement *pElement = NULL;
    IAppHostElement *pNewElement = NULL;
    IAppHostElementCollection *pCollection = NULL;
    
    WCHAR wcDelim[] = L",";
    const WCHAR *wszToken = NULL;
    WCHAR *wszNextToken = NULL;

    hr = pAdminMgr->GetAdminSection(ScopeBSTR(IIS_CONFIG_DEFAULTDOC_SECTION), ScopeBSTR(wszConfigPath), &pSection);
    ExitOnFailure(hr, "Failed get defaultDocument section for DirProp");

    hr = pSection->GetElementByName(ScopeBSTR(L"files"), &pElement);
    ExitOnFailure(hr, "Failed get win files section");

    hr = pElement->get_Collection(&pCollection);
    ExitOnFailure(hr, "Failed get files collection");

    hr = pCollection->Clear();
    ExitOnFailure(hr, "Failed clear files collection");

    //Clear out inherited items - add clear
    hr = pCollection->CreateNewElement(ScopeBSTR(IIS_CONFIG_CLEAR), &pNewElement);
    ExitOnFailure(hr, "Failed to create files clear element");
    hr = pCollection->AddElement(pNewElement);
    ExitOnFailure(hr, "Failed to add files clear element");
    
    wszToken = wcstok_s( wszData, wcDelim, &wszNextToken);
    for( int i = 0; (wszToken); i++)
    {
        hr = pCollection->CreateNewElement(ScopeBSTR(IIS_CONFIG_ADD), &pNewElement);
        ExitOnFailure(hr, "Failed to create win auth providers element");

        hr = PutPropertyValue( pNewElement, IIS_CONFIG_VALUE, wszToken);
        ExitOnFailure(hr, "Failed to set win auth providers value");

        hr = pCollection->AddElement(pNewElement, i);
        ExitOnFailure(hr, "Failed to add defaultDocument Files element");
        ReleaseNullObject(pNewElement);

        wszToken = wcstok_s( NULL, wcDelim, &wszNextToken);
    }

LExit:
    ReleaseNullObject(pSection);
    ReleaseNullObject(pCollection);
    ReleaseNullObject(pNewElement);
    return hr;
}

 static HRESULT PutPropertyValue(IAppHostElement *pElement, LPCWSTR wszPropName, VARIANT vtPut)
{
    HRESULT hr = S_OK;
    IAppHostProperty *pProperty = NULL;

    hr = pElement->GetPropertyByName(ScopeBSTR(wszPropName), &pProperty);
    ExitOnFailure1(hr, "Failed to get property object for %S", wszPropName);

    hr = pProperty->put_Value(vtPut);
    ExitOnFailure1(hr, "Failed to set property value for %S", wszPropName);
LExit:
    // caller responsible vor cleaning up variant vtPut
    ReleaseNullObject(pProperty);

    return hr;
}

static HRESULT PutPropertyValue(IAppHostElement *pElement, LPCWSTR wszPropName, LPCWSTR wszString)
{
    HRESULT hr = S_OK;
    VARIANT vtPut;

    VariantInit(&vtPut);
    vtPut.vt = VT_BSTR;
    vtPut.bstrVal = SysAllocString(wszString);
    hr = PutPropertyValue(pElement, wszPropName, vtPut);    
    VariantClear(&vtPut);
    return hr;
}

static HRESULT PutPropertyValue(IAppHostElement *pElement, LPCWSTR wszPropName, DWORD dValue)
{
    VARIANT vtPut;
    VariantInit(&vtPut);
    vtPut.vt = VT_I4;
    vtPut.lVal = dValue;
    return PutPropertyValue(pElement, wszPropName, vtPut);
}

// This method must have a different name since BOOL is actually an int and we don't want to treat ints as bools
static HRESULT PutPropertyBoolValue(IAppHostElement *pElement, LPCWSTR wszPropName, BOOL fValue)
{
    VARIANT vtPut;
    VariantInit(&vtPut);
    vtPut.vt = VT_BOOL;
    vtPut.boolVal = (fValue == FALSE) ? VARIANT_FALSE : VARIANT_TRUE;
    return PutPropertyValue(pElement, wszPropName, vtPut);
}

static HRESULT ClearLocationTag( IAppHostWritableAdminManager *pAdminMgr,
                                 LPCWSTR swLocationPath
                                 )
{
    HRESULT hr = S_OK;
    IAppHostConfigManager *pConfigMgr = NULL;
    IAppHostConfigFile    *pConfigFile = NULL;
    IAppHostConfigLocationCollection *pLocationCollection = NULL;
    IAppHostConfigLocation *pLocation = NULL;

    DWORD dwCount = 0;
    BSTR bstrLocationPath = NULL;

    hr = pAdminMgr->get_ConfigManager(&pConfigMgr);
    ExitOnFailure(hr, "Failed to get IIS ConfigManager interface");

    hr = pConfigMgr->GetConfigFile(ScopeBSTR(IIS_CONFIG_APPHOST_ROOT), &pConfigFile);
    ExitOnFailure(hr, "Failed to get IIS ConfigFile interface");

    hr = pConfigFile->get_Locations(&pLocationCollection);
    ExitOnFailure(hr, "Failed to get IIS location tag collection");

    hr = pLocationCollection->get_Count(&dwCount);
    ExitOnFailure(hr, "Failed to get IIS location collection count");

    VARIANT vtIndex;
    vtIndex.vt = VT_UI4;
    for( DWORD i =0; i < dwCount; i++)
    {
        vtIndex.ulVal = i;
        hr = pLocationCollection->get_Item(vtIndex, &pLocation);
        ExitOnFailure(hr, "Failed to get IIS location collection count");

        hr = pLocation->get_Path(&bstrLocationPath);
        if(CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, swLocationPath, -1, bstrLocationPath, -1))
        {
            hr = pLocationCollection->DeleteLocation(vtIndex);
            ExitOnFailure1(hr, "Failed to delete IIS location tag %S",swLocationPath);
            break;
        }

        ReleaseNullObject(pLocation);
        ::SysFreeString(bstrLocationPath);
        bstrLocationPath = NULL;
    }
LExit:
    ReleaseNullObject(pConfigMgr);
    ReleaseNullObject(pConfigFile);
    ReleaseNullObject(pLocationCollection);
    ReleaseNullObject(pLocation);
    if( bstrLocationPath)
    {
        ::SysFreeString(bstrLocationPath);
    }
    return hr;

}

static HRESULT DeleteCollectionElement(__in IAppHostElementCollection *pCollection,
                                       __in LPCWSTR pwzElementName,
                                       __in LPCWSTR pwzAttributeName,
                                       __in LPCWSTR pwzAttributeValue
                                       )
{
    HRESULT hr = S_OK;
    IAppHostElement *pElement = NULL;

    DWORD dwIndex;
    VARIANT vtIndex;
    VariantInit(&vtIndex);
    
    hr = FindAppHostElement(pCollection, pwzElementName, pwzAttributeName, pwzAttributeValue, NULL, &dwIndex);
    ExitOnFailure3(hr, "Failed while finding IAppHostElement %S/@%S=%S", pwzElementName, pwzAttributeName, pwzAttributeValue);
    
    if (MAXDWORD != dwIndex)
    {
        vtIndex.vt = VT_UI4;
        vtIndex.ulVal = dwIndex;
        hr = pCollection->DeleteElement(vtIndex);
        ExitOnFailure3(hr, "Failed to delete IAppHostElement %S/@%S=%S", pwzElementName, pwzAttributeName, pwzAttributeValue);
    }
    // else : nothing to do, already deleted
LExit:
    VariantClear(&vtIndex);
    ReleaseNullObject(pElement);

    return hr;
}
static void ConvSecToHMS( int Sec,  __out_ecount(cchDest) LPWSTR wcTime, size_t cchDest)
{
    int ZH, ZM, ZS = 0;

    ZH = (Sec / 3600);
    Sec = Sec - ZH * 3600;
    ZM = (Sec / 60) ;
    Sec = Sec - ZM * 60;
    ZS = Sec ;

    HRESULT hr = ::StringCchPrintfW(wcTime, cchDest, L"%02d:%02d:%02d", ZH, ZM, ZS);
    if( S_OK != hr)
    {
        *wcTime = '\0';
    }
}
static void ConvSecToDHMS( unsigned int Sec,  __out_ecount(cchDest) LPWSTR wcTime, size_t cchDest)
{
    int ZD, ZH, ZM, ZS = 0;

    ZD = Sec / 86400;
    Sec = Sec - ZD * 86400;
    ZH = (Sec / 3600);
    Sec = Sec - ZH * 3600;
    ZM = (Sec / 60) ;
    Sec = Sec - ZM * 60;
    ZS = Sec ;

    HRESULT hr = ::StringCchPrintfW(wcTime, cchDest, L"%d.%02d:%02d:%02d", ZD, ZH, ZM, ZS);
    if( S_OK != hr)
    {
        *wcTime = '\0';
    }
}