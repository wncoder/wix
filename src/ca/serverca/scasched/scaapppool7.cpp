//-------------------------------------------------------------------------------------------------
// <copyright file="scaapppool7.cpp" company="Microsoft">
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
//    Application pool functions for CustomActions
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"

/*------------------------------------------------------------------
AppPool table:

Column           Type   Nullable     Example Value
AppPool          s72    No           TestPool
Name             s72    No           "TestPool"
Component_       s72    No           ComponentName
Attributes       i2     No           8 (APATTR_OTHERUSER)
User_            s72    Yes          UserKey
RecycleMinutes   i2     Yes          500
RecycleRequests  i2     Yes          5000
RecycleTimes     s72    Yes          "1:45,13:30,22:00"
IdleTimeout      i2     Yes          15
QueueLimit       i2     Yes          500
CPUMon           s72    Yes          "65,500,1" (65% CPU usage, 500 minutes, Shutdown Action)
MaxProc          i2     Yes          5

Notes:
RecycleTimes is a comma delimeted list of times.  CPUMon is a
comma delimeted list of the following format:
<percent CPU usage>,<refress minutes>,<Action>.  The values for
Action are 1 (Shutdown) and 0 (No Action).

------------------------------------------------------------------*/
// sql queries
LPCWSTR vcsAppPoolQuery7 = L"SELECT `AppPool`, `Name`, `Component_`, `Attributes`, `User_`, `RecycleMinutes`, `RecycleRequests`, `RecycleTimes`, `VirtualMemory`, `PrivateMemory`, `IdleTimeout`, `QueueLimit`, `CPUMon`, `MaxProc` FROM `IIsAppPool`";
enum eAppPoolQuery { apqAppPool = 1, apqName, apqComponent, apqAttributes, apqUser, apqRecycleMinutes, apqRecycleRequests, apqRecycleTimes, apqVirtualMemory, apqPrivateMemory, apqIdleTimeout, apqQueueLimit, apqCpuMon, apqMaxProc};

LPCWSTR vcsComponentAttrQuery = L"SELECT `Attributes` FROM `Component` WHERE `Component`=?";
enum eComponentAttrQuery { caqAttributes = 1 };

// prototypes
static HRESULT AppPoolExists(
    __in LPCWSTR wzAppPool
    );

static HRESULT AddAppPoolToList(
    __in SCA_APPPOOL** ppsapList
    );

// functions

void ScaAppPoolFreeList7(
    __in SCA_APPPOOL* psapList
    )
{
    SCA_APPPOOL* psapDelete = psapList;
    while (psapList)
    {
        psapDelete = psapList;
        psapList = psapList->psapNext;

        MemFree(psapDelete);
    }
}


HRESULT ScaAppPoolRead7(
    __inout SCA_APPPOOL** ppsapList
    )
{
    Assert(ppsapList);

    HRESULT hr = S_OK;
    UINT er = ERROR_SUCCESS;

    PMSIHANDLE hView, hRec, hViewComp, hRecComp, hRecAttr;
    LPWSTR pwzData = NULL;
    INSTALLSTATE isInstalled = INSTALLSTATE_UNKNOWN;
    INSTALLSTATE isAction = INSTALLSTATE_UNKNOWN;
    SCA_APPPOOL* psap = NULL;


    if (S_OK != WcaTableExists(L"IIsAppPool"))
    {
        WcaLog(LOGMSG_VERBOSE, "Skipping ScaAppPoolRead7() - because IIsAppPool table not present");
        ExitFunction1(hr = S_FALSE);
    }

    // open a view on the component table
    hr = WcaOpenView(vcsComponentAttrQuery, &hViewComp);
    ExitOnFailure(hr, "Failed to open view on Component table for ScaAppPoolRead7");

    // loop through all the AppPools
    hr = WcaOpenExecuteView(vcsAppPoolQuery7, &hView);
    ExitOnFailure(hr, "failed to open view on IIsAppPool table");
    // loop through all the AppPools
    while (S_OK == (hr = WcaFetchRecord(hView, &hRec)))
    {
        BOOL fHasComponent = FALSE;
        // Add this record's information into the list of things to process.
        hr = AddAppPoolToList(ppsapList);
        ExitOnFailure(hr, "failed to add app pool to app pool list");

        psap = *ppsapList;

        // get the darwin information
        hr = WcaGetRecordString(hRec, apqComponent, &pwzData);
        ExitOnFailure(hr, "failed to get AppPool.Component");
        hr = ::StringCchCopyW(psap->wzComponent, countof(psap->wzComponent), pwzData);
        ExitOnFailure(hr, "Failed StringCchCopyW of apppool component");

        if (*(psap->wzComponent))
        {
            psap->fHasComponent = TRUE;

            er = ::MsiGetComponentStateW(WcaGetInstallHandle(), psap->wzComponent, &psap->isInstalled, &psap->isAction);
            hr = HRESULT_FROM_WIN32(er);
            ExitOnFailure(hr, "Failed to get appPool Component state");

            // Determine component attributes, this needs to be a seperate query since not all app pools have components
            hRecComp = ::MsiCreateRecord(1);
            hr = WcaSetRecordString(hRecComp, 1, psap->wzComponent);
            ExitOnFailure(hr, "Failed to look up component attributes");

            hr = WcaExecuteView(hViewComp, hRecComp);
            ExitOnFailure1(hr, "Failed to open Component.Attributes for Component '%S'", psap->wzComponent);
            hr = WcaFetchSingleRecord(hViewComp, &hRecAttr);
            ExitOnFailure1(hr, "Failed to fetch Component.Attributes for Component '%S'", psap->wzComponent);

            hr = WcaGetRecordInteger(hRecAttr, caqAttributes, &psap->iCompAttributes);
            ExitOnFailure(hr, "failed to get Component.Attributes");
        }
        //
        //get apppool properties
        //
        hr = WcaGetRecordString(hRec, apqAppPool, &pwzData);
        ExitOnFailure(hr, "failed to get AppPool.AppPool");
        hr = ::StringCchCopyW(psap->wzAppPool, countof(psap->wzAppPool), pwzData);
        ExitOnFailure1(hr, "failed to copy AppPool name: %S", pwzData);

        hr = WcaGetRecordFormattedString(hRec, apqName, &pwzData);
        ExitOnFailure(hr, "failed to get AppPool.Name");
        hr = ::StringCchCopyW(psap->wzName, countof(psap->wzName), pwzData);
        ExitOnFailure1(hr, "failed to copy app pool name: %S", pwzData);

        hr = WcaGetRecordInteger(hRec, apqAttributes, &psap->iAttributes);
        ExitOnFailure(hr, "failed to get AppPool.Attributes");

        hr = WcaGetRecordString(hRec, apqUser, &pwzData);
        ExitOnFailure(hr, "failed to get AppPool.User");

        hr = ScaGetUser(pwzData, &psap->suUser);
        ExitOnFailure1(hr, "failed to get user: %S", pwzData);

        hr = WcaGetRecordInteger(hRec, apqRecycleRequests, &psap->iRecycleRequests);
        ExitOnFailure(hr, "failed to get AppPool.RecycleRequests");

        hr = WcaGetRecordInteger(hRec, apqRecycleMinutes, &psap->iRecycleMinutes);
        ExitOnFailure(hr, "failed to get AppPool.Minutes");

        hr = WcaGetRecordString(hRec, apqRecycleTimes, &pwzData);
        ExitOnFailure(hr, "failed to get AppPool.RecycleTimes");
        hr = ::StringCchCopyW(psap->wzRecycleTimes, countof(psap->wzRecycleTimes), pwzData);
        ExitOnFailure1(hr, "failed to copy recycle value: %S", pwzData);

        hr = WcaGetRecordInteger(hRec, apqVirtualMemory, &psap->iVirtualMemory);
        ExitOnFailure(hr, "failed to get AppPool.VirtualMemory");

        hr = WcaGetRecordInteger(hRec, apqPrivateMemory, &psap->iPrivateMemory);
        ExitOnFailure(hr, "failed to get AppPool.PrivateMemory");

        hr = WcaGetRecordInteger(hRec, apqIdleTimeout, &psap->iIdleTimeout);
        ExitOnFailure(hr, "failed to get AppPool.IdleTimeout");

        hr = WcaGetRecordInteger(hRec, apqQueueLimit, &psap->iQueueLimit);
        ExitOnFailure(hr, "failed to get AppPool.QueueLimit");

        hr = WcaGetRecordString(hRec, apqCpuMon, &pwzData);
        ExitOnFailure(hr, "failed to get AppPool.CPUMon");
        hr = ::StringCchCopyW(psap->wzCpuMon, countof(psap->wzCpuMon), pwzData);
        ExitOnFailure1(hr, "failed to copy cpu monitor value: %S", pwzData);

        hr = WcaGetRecordInteger(hRec, apqMaxProc, &psap->iMaxProcesses);
        ExitOnFailure(hr, "failed to get AppPool.MaxProc");
    }

    if (E_NOMOREITEMS == hr)
    {
        hr = S_OK;
    }
    ExitOnFailure(hr, "failure while processing AppPools");

LExit:
    ReleaseStr(pwzData);

    return hr;
}


HRESULT ScaFindAppPool7(
    __in LPCWSTR wzAppPool,
    __out_ecount(cchName) LPWSTR wzName,
    __in DWORD cchName,
    __in SCA_APPPOOL *psapList
    )
{
    Assert(wzAppPool && *wzAppPool && wzName && *wzName);

    HRESULT hr = S_OK;

    // check memory first
    SCA_APPPOOL* psap = psapList;
    for (; psap; psap = psap->psapNext)
    {
        if (0 == wcscmp(psap->wzAppPool, wzAppPool))
        {
            break;
        }
    }
    ExitOnNull1(psap, hr, HRESULT_FROM_WIN32(ERROR_NOT_FOUND), "Could not find the app pool: %S", wzAppPool);

    // copy the web app pool name
#pragma prefast(suppress:26037, "Source string is null terminated - it is populated as target of ::StringCchCopyW")
    hr = ::StringCchCopyW(wzName, cchName, psap->wzName);
    ExitOnFailure1(hr, "failed to copy app pool name while finding app pool: %S", psap->wzName);

    // if it's not being installed now, check if it exists already
    if (!psap->fHasComponent)
    {
        hr = AppPoolExists(psap->wzName);
        ExitOnFailure1(hr, "failed to check for existence of app pool: %S", psap->wzName);
    }

LExit:
    return hr;
}


static HRESULT AppPoolExists(
    __in LPCWSTR wzAppPool
    )
{
    Assert(wzAppPool && *wzAppPool);

    HRESULT hr = S_OK;

    //this function checks for existance of app pool in IIS7 config
    //at schedule time, we will defer this to execute time.

    return hr;
}


HRESULT ScaAppPoolInstall7(
    __in SCA_APPPOOL* psapList
    )
{
    HRESULT hr = S_OK;

    for (SCA_APPPOOL* psap = psapList; psap; psap = psap->psapNext)
    {
        // if we are installing the app pool
        if (psap->fHasComponent && WcaIsInstalling(psap->isInstalled, psap->isAction))
        {
            hr = ScaWriteAppPool7(psap);
            ExitOnFailure1(hr, "failed to write AppPool '%S' to metabase", psap->wzAppPool);
        }
    }

LExit:
    return hr;
}


HRESULT ScaAppPoolUninstall7(
    __in SCA_APPPOOL* psapList
    )
{

    HRESULT hr = S_OK;

    for (SCA_APPPOOL* psap = psapList; psap; psap = psap->psapNext)
    {
        // if we are uninstalling the app pool
        if (psap->fHasComponent && WcaIsUninstalling(psap->isInstalled, psap->isAction))
        {
            hr = ScaRemoveAppPool7(psap);
            ExitOnFailure1(hr, "Failed to remove AppPool '%S' from metabase", psap->wzAppPool);
        }
    }

LExit:
    return hr;
}


HRESULT ScaWriteAppPool7(
    __in const SCA_APPPOOL* psap
    )
{
    Assert(psap);

    HRESULT hr = S_OK;
    DWORD dwIdentity = -1;
    BOOL fExists = FALSE;
    LPWSTR pwzValue = NULL;
    LPWSTR wz = NULL;

    //create the app pool
    hr = ScaWriteConfigID(IIS_APPPOOL);
    ExitOnFailure(hr, "failed to write AppPool key.");

    hr = ScaWriteConfigID(IIS_CREATE);
    ExitOnFailure(hr, "failed to write AppPool create action.");

    hr = ScaWriteConfigString(psap->wzName);
    ExitOnFailure1(hr, "failed to write AppPool name: %S", psap->wzName);

    // Now do all the optional stuff

    // Set the AppPool Recycling Tab
    if (MSI_NULL_INTEGER != psap->iRecycleMinutes)
    {
        hr = ScaWriteConfigID(IIS_APPPOOL_RECYCLE_MIN);
        ExitOnFailure(hr, "failed to set periodic restart time id");
        hr = ScaWriteConfigInteger(psap->iRecycleMinutes);
        ExitOnFailure(hr, "failed to set periodic restart time");
    }

    if (MSI_NULL_INTEGER != psap->iRecycleRequests)
    {
        hr = ScaWriteConfigID(IIS_APPPOOL_RECYCLE_REQ);
        ExitOnFailure(hr, "failed to set periodic restart request count id");
        hr = ScaWriteConfigInteger(psap->iRecycleRequests);
        ExitOnFailure(hr, "failed to set periodic restart request count");
    }

    if (*psap->wzRecycleTimes)
    {
        hr = ScaWriteConfigID(IIS_APPPOOL_RECYCLE_TIMES);
        ExitOnFailure(hr, "failed to set periodic restart schedule id");
        hr = ScaWriteConfigString(psap->wzRecycleTimes);
        ExitOnFailure(hr, "failed to set periodic restart schedule");
    }

    if (MSI_NULL_INTEGER != psap->iVirtualMemory)
    {
        hr = ScaWriteConfigID(IIS_APPPOOL_RECYCLE_VIRMEM);
        ExitOnFailure(hr, "failed to set periodic restart memory count id");
        hr = ScaWriteConfigInteger(psap->iVirtualMemory);
        ExitOnFailure(hr, "failed to set periodic restart memory count");
    }

    if (MSI_NULL_INTEGER != psap->iPrivateMemory)
    {
        hr = ScaWriteConfigID(IIS_APPPOOL_RECYCLE_PRIVMEM);
        ExitOnFailure(hr, "failed to set periodic restart private memory count id");
        hr = ScaWriteConfigInteger(psap->iPrivateMemory);
        ExitOnFailure(hr, "failed to set periodic restart private memory count");
    }

    // Set AppPool Performance Tab
    if (MSI_NULL_INTEGER != psap->iIdleTimeout)
    {
        hr = ScaWriteConfigID(IIS_APPPOOL_RECYCLE_IDLTIMEOUT);
        ExitOnFailure(hr, "failed to set idle timeout value id");
        hr = ScaWriteConfigInteger(psap->iIdleTimeout);
        ExitOnFailure(hr, "failed to set idle timeout value");
    }

    if (MSI_NULL_INTEGER != psap->iQueueLimit)
    {
        hr = ScaWriteConfigID(IIS_APPPOOL_RECYCLE_QUEUELIMIT);
        ExitOnFailure(hr, "failed to set request queue limit value id");
        hr = ScaWriteConfigInteger(psap->iQueueLimit);
        ExitOnFailure(hr, "failed to set request queue limit value");
    }
    if(*psap->wzCpuMon)
    {
#pragma prefast(suppress:26037, "Source string is null terminated - it is populated as target of ::StringCchCopyW")
        hr = ::StrAllocString(&pwzValue, psap->wzCpuMon, 0);
        ExitOnFailure(hr, "failed to allocate CPUMonitor string");

        DWORD dwPercent = 0;
        DWORD dwRefreshMinutes = 0;
        DWORD dwAction = 0;

        dwPercent = wcstoul(pwzValue, &wz, 10);
        if (100  < dwPercent)
        {
            ExitOnFailure1(hr = E_INVALIDARG, "invalid maximum cpu percentage value: %d", dwPercent);
        }
        if (wz && L',' == *wz)
        {
            wz++;
            dwRefreshMinutes = wcstoul(wz, &wz, 10);
            if (wz && L',' == *wz)
            {
                wz++;
                dwAction = wcstoul(wz, &wz, 10);
            }
        }
        if (dwPercent)
        {
            hr = ScaWriteConfigID(IIS_APPPOOL_RECYCLE_CPU_PCT);
            ExitOnFailure(hr, "failed to set recycle pct id");
            hr = ScaWriteConfigInteger(dwPercent);
            ExitOnFailure(hr, "failed to set CPU percentage max");
        }
        if (dwRefreshMinutes)
        {
            hr = ScaWriteConfigID(IIS_APPPOOL_RECYCLE_CPU_REFRESH);
            ExitOnFailure(hr, "failed to set recycle refresh id");
            hr = ScaWriteConfigInteger(dwRefreshMinutes);
            ExitOnFailure(hr, "failed to set refresh CPU minutes");
        }
        if (dwAction)
        {
            // 0 = No Action
            // 1 = Shutdown
            hr = ScaWriteConfigID(IIS_APPPOOL_RECYCLE_CPU_ACTION);
            ExitOnFailure(hr, "failed to set recycle refresh id");
            hr = ScaWriteConfigInteger(dwAction);
            ExitOnFailure(hr, "failed to set CPU action");
        }
    }

    if (MSI_NULL_INTEGER != psap->iMaxProcesses)
    {
        hr = ScaWriteConfigID(IIS_APPPOOL_MAXPROCESS);
        ExitOnFailure(hr, "Failed to write max processes config ID");

        hr = ScaWriteConfigInteger(psap->iMaxProcesses);
        ExitOnFailure(hr, "failed to set web garden maximum worker processes");
    }

    if (!(psap->iCompAttributes & msidbComponentAttributes64bit))
    {
        hr = ScaWriteConfigID(IIS_APPPOOL_32BIT);
        ExitOnFailure(hr, "Failed to write 32 bit app pool config ID");
    }

    //
    // Set the AppPool Identity tab
    //
    if (psap->iAttributes & APATTR_NETSERVICE)
    {
        dwIdentity = 2;
    }
    else if (psap->iAttributes & APATTR_LOCSERVICE)
    {
        dwIdentity = 1;
    }
    else if (psap->iAttributes & APATTR_LOCSYSTEM)
    {
        dwIdentity = 0;
    }
    else if (psap->iAttributes & APATTR_OTHERUSER)
    {
        if (!*psap->suUser.wzDomain || 0 == _wcsicmp(psap->suUser.wzDomain, L"."))
        {
            if (0 == _wcsicmp(psap->suUser.wzName, L"NetworkService"))
            {
                dwIdentity = 2;
            }
            else if (0 == _wcsicmp(psap->suUser.wzName, L"LocalService"))
            {
                dwIdentity = 1;
            }
            else if (0 == _wcsicmp(psap->suUser.wzName, L"LocalSystem"))
            {
                dwIdentity = 0;
            }
            else
            {
                dwIdentity = 3;
            }
        }
        else if (0 == _wcsicmp(psap->suUser.wzDomain, L"NT AUTHORITY"))
        {
            if (0 == _wcsicmp(psap->suUser.wzName, L"NETWORK SERVICE"))
            {
                dwIdentity = 2;
            }
            else if (0 == _wcsicmp(psap->suUser.wzName, L"SERVICE"))
            {
                dwIdentity = 1;
            }
            else if (0 == _wcsicmp(psap->suUser.wzName, L"SYSTEM"))
            {
                dwIdentity = 0;
            }
            else
            {
                dwIdentity = 3;
            }
        }
        else
        {
            dwIdentity = 3;
        }
    }

    if (-1 != dwIdentity)
    {
        hr = ScaWriteConfigID(IIS_APPPOOL_IDENTITY);
        ExitOnFailure(hr, "failed to set app pool identity id");
        hr = ScaWriteConfigInteger(dwIdentity);
        ExitOnFailure(hr, "failed to set app pool identity");

        if (3 == dwIdentity)
        {
            if (*psap->suUser.wzDomain)
            {
                hr = StrAllocFormatted(&pwzValue, L"%s\\%s", psap->suUser.wzDomain, psap->suUser.wzName);
                ExitOnFailure2(hr, "failed to format user name: %S domain: %S", psap->suUser.wzName, psap->suUser.wzDomain);
            }
            else
            {
                hr = StrAllocFormatted(&pwzValue, L"%s", psap->suUser.wzName);
                ExitOnFailure1(hr, "failed to format user name: %S", psap->suUser.wzName);
            }

            hr = ScaWriteConfigID(IIS_APPPOOL_USER);
            ExitOnFailure(hr, "failed to set app pool identity name id");
            hr = ScaWriteConfigString(pwzValue);
            ExitOnFailure(hr, "failed to set app pool identity name");

            hr = ScaWriteConfigID(IIS_APPPOOL_PWD);
            ExitOnFailure(hr, "failed to set app pool identity password id");
            hr = ScaWriteConfigString(psap->suUser.wzPassword);
            ExitOnFailure(hr, "failed to set app pool identity password");
        }
    }
    //
    //The number of properties above is variable so we put an end tag in so the
    //execute CA will know when to stop looking for AppPool properties
    //
    hr = ScaWriteConfigID(IIS_APPPOOL_END);
    ExitOnFailure(hr, "failed to set app pool end of properties id");

LExit:
    ReleaseStr(pwzValue);

    return hr;
}


HRESULT ScaRemoveAppPool7(
    __in const SCA_APPPOOL* psap
    )
{
    Assert(psap);

    HRESULT hr = S_OK;

    //do not delete the default App Pool
    if (0 != _wcsicmp(psap->wzAppPool, L"DefaultAppPool"))
    {
        //delete the app pool
        hr = ScaWriteConfigID(IIS_APPPOOL);
        ExitOnFailure(hr, "failed to write AppPool key.");

        hr = ScaWriteConfigID(IIS_DELETE);
        ExitOnFailure(hr, "failed to write AppPool delete action.");

        hr = ScaWriteConfigString(psap->wzName);
        ExitOnFailure1(hr, "failed to delete AppPool: %S", psap->wzName);
    }

LExit:
    return hr;
}


static HRESULT AddAppPoolToList(
    __in SCA_APPPOOL** ppsapList
    )
{
    HRESULT hr = S_OK;
    SCA_APPPOOL* psap = static_cast<SCA_APPPOOL*>(MemAlloc(sizeof(**ppsapList), TRUE));
    ExitOnNull(psap, hr, E_OUTOFMEMORY, "failed to allocate memory for new element in app pool list");

    psap->psapNext = *ppsapList;
    *ppsapList = psap;

LExit:
    return hr;
}