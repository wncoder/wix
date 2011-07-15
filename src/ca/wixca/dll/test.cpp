//-------------------------------------------------------------------------------------------------
// <copyright file="test.cpp" company="Microsoft">
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
//    Code useful in testing custom actions.
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"


/******************************************************************
WixFailWhenDeferred - entry point for WixFailWhenDeferred
    custom action which always fails when running as a deferred
    custom action (otherwise it blindly succeeds). It's useful when
    testing the rollback of deferred custom actions: Schedule it
    immediately after the rollback/deferred CA pair you're testing
    and it will fail, causing your rollback CA to get invoked.
********************************************************************/
extern "C" UINT __stdcall WixFailWhenDeferred(
    __in MSIHANDLE hInstall
    )
{
    return ::MsiGetMode(hInstall, MSIRUNMODE_SCHEDULED) ? ERROR_INSTALL_FAILURE : ERROR_SUCCESS;
}

/******************************************************************
WixWaitForEvent - entry point for WixWaitForEvent custom action
    which waits for either the WixWaitForEventFail or
    WixWaitForEventSucceed named auto reset events. Signaling the
    WixWaitForEventFail event will return ERROR_INSTALL_FAILURE or
    signaling the WixWaitForEventSucceed event will return
    ERROR_SUCCESS. Both events are declared in the Global\ namespace.
********************************************************************/
extern "C" UINT __stdcall WixWaitForEvent(
    __in MSIHANDLE hInstall
    )
{
    HRESULT hr = S_OK;
    UINT er = ERROR_SUCCESS;
    LPCWSTR wzSDDL = L"D:(A;;GA;;;WD)";
    OS_VERSION version = OS_VERSION_UNKNOWN;
    DWORD dwServicePack = 0;
    PSECURITY_DESCRIPTOR pSD = NULL;
    SECURITY_ATTRIBUTES sa = { };
    HANDLE rghEvents[2];

    hr = WcaInitialize(hInstall, "WixWaitForEvent");
    ExitOnFailure(hr, "Failed to initialize.");

    // If running on Vista/2008 or newer use integrity enhancements.
    OsGetVersion(&version, &dwServicePack);
    if (OS_VERSION_VISTA <= version)
    {
        // Add SACL to allow Everyone to signal from a medium integrity level.
        wzSDDL = L"D:(A;;GA;;;WD)S:(ML;;NW;;;ME)";
    }

    // Create the security descriptor and attributes for the events.
    if (!::ConvertStringSecurityDescriptorToSecurityDescriptorW(wzSDDL, SDDL_REVISION_1, &pSD, NULL))
    {
        ExitWithLastError(hr, "Failed to create the security descriptor for the events.");
    }

    sa.nLength = sizeof(sa);
    sa.lpSecurityDescriptor = pSD;
    sa.bInheritHandle = FALSE;

    rghEvents[0] = ::CreateEventW(&sa, FALSE, FALSE, L"Global\\WixWaitForEventFail");
    ExitOnNullWithLastError(rghEvents[0], hr, "Failed to create the Global\\WixWaitForEventFail event.");

    rghEvents[1] = ::CreateEventW(&sa, FALSE, FALSE, L"Global\\WixWaitForEventSucceed");
    ExitOnNullWithLastError(rghEvents[1], hr, "Failed to create the Global\\WixWaitForEventSucceed event.");

    // Wait for either of the events to be signaled and handle accordingly.
    er = ::WaitForMultipleObjects(countof(rghEvents), rghEvents, FALSE, INFINITE);
    switch (er)
    {
    case WAIT_OBJECT_0 + 0:
        er = ERROR_INSTALL_FAILURE;
        break;
    case WAIT_OBJECT_0 + 1:
        er = ERROR_SUCCESS;
        break;
    default:
        ExitOnWin32Error(er, hr, "Unexpected failure.");
    }

LExit:
    ReleaseHandle(rghEvents[1]);
    ReleaseHandle(rghEvents[0]);

    if (pSD)
    {
        ::LocalFree(pSD);
    }

    if (FAILED(hr))
    {
        er = ERROR_INSTALL_FAILURE;
    }

    return WcaFinalize(er);
}
