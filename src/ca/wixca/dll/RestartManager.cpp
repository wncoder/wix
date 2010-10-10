//-------------------------------------------------------------------------------------------------
// <copyright file="RestartManager.cpp" company="Microsoft">
//    Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// 
// <summary>
//    Restart Manager custom actions.
// </summary>
// 
// <remarks>
//    MSDN (http://msdn.microsoft.com/library/aa373680.aspx) implies we need separate
//    custom actions for different processor architectures / component bitness.
// </remarks>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"
#include <restartmanager.h>

// Include space for the terminating null.
#define CCH_SESSION_KEY CCH_RM_SESSION_KEY + 1

enum eRmuResourceType
{
    etInvalid,
    etFilename,
    etApplication,
    etServiceName,

    // Mask types from Attributes.
    etTypeMask = 0xf,
};

LPCWSTR vcsRestartResourceQuery =
    L"SELECT `WixRestartResource`.`WixRestartResource`, `WixRestartResource`.`Component_`, `WixRestartResource`.`Resource`, `WixRestartResource`.`Attributes` "
    L"FROM `WixRestartResource`";
enum eRestartResourceQuery { rrqRestartResource = 1, rrqComponent, rrqResource, rrqAttributes };

/********************************************************************
WixRegisterRestartResources - Immediate CA to register resources with RM.

Enumerates components before InstallValidate and registers resources
to be restarted by Restart Manager if the component action
is anything other than None.

Do not disable file system redirection.

********************************************************************/
extern "C" UINT __stdcall WixRegisterRestartResources(
    __in MSIHANDLE hInstall
    )
{
    HRESULT hr = S_OK;
    UINT er = ERROR_SUCCESS;

    PMSIHANDLE hView = NULL;
    PMSIHANDLE hRec = NULL;

    LPWSTR wzSessionKey = NULL;
    size_t cchSessionKey = 0;
    PRMU_SESSION pSession = NULL;

    LPWSTR wzRestartResource = NULL;
    LPWSTR wzComponent = NULL;
    LPWSTR wzResource = NULL;
    int iAttributes = NULL;
    WCA_TODO todo = WCA_TODO_UNKNOWN;
    int iType = etInvalid;

    hr = WcaInitialize(hInstall, "WixRegisterRestartResources");
    ExitOnFailure(hr, "Failed to initialize.");

    // Skip if the table doesn't exist.
    if (S_OK != WcaTableExists(L"WixRestartResource"))
    {
        WcaLog(LOGMSG_STANDARD, "The RestartResource table does not exist; there are no resources to register with Restart Manager.");
        ExitFunction();
    }

    // Get the existing Restart Manager session if available.
    hr = WcaGetProperty(L"MsiRestartManagerSessionKey", &wzSessionKey);
    ExitOnFailure(hr, "Failed to get the MsiRestartManagerSessionKey property.");

    hr = ::StringCchLengthW(wzSessionKey, CCH_SESSION_KEY, &cchSessionKey);
    ExitOnFailure(hr, "Failed to get the MsiRestartManagerSessionKey string length.");

    // Skip if the property doesn't exist.
    if (0 == cchSessionKey)
    {
        WcaLog(LOGMSG_STANDARD, "The MsiRestartManagerSessionKey property is not available to join.");
        ExitFunction();
    }

    // Join the existing Restart Manager session.
    hr = RmuJoinSession(&pSession, wzSessionKey);
    MessageExitOnFailure1(hr, msierrRmuJoinFailed, "Could not join the existing Restart Manager session %ls.", wzSessionKey);

    // Loop through each record in the table.
    hr = WcaOpenExecuteView(vcsRestartResourceQuery, &hView);
    ExitOnFailure(hr, "Failed to open a view on the RestartResource table.");

    while (S_OK == (hr = WcaFetchRecord(hView, &hRec)))
    {
        hr = WcaGetRecordString(hRec, rrqRestartResource, &wzRestartResource);
        ExitOnFailure(hr, "Failed to get the RestartResource field value.");

        hr = WcaGetRecordString(hRec, rrqComponent, &wzComponent);
        ExitOnFailure(hr, "Failed to get the Component_ field value.");

        hr = WcaGetRecordFormattedString(hRec, rrqResource, &wzResource);
        ExitOnFailure(hr, "Failed to get the Resource formatted field value.");

        hr = WcaGetRecordInteger(hRec, rrqAttributes, &iAttributes);
        ExitOnFailure(hr, "Failed to get the Attributes field value.");

        // Only register resources for components that are being installed, reinstalled, or uninstalled.
        todo = WcaGetComponentToDo(wzComponent);
        if (WCA_TODO_UNKNOWN == todo)
        {
            WcaLog(LOGMSG_VERBOSE, "Skipping resource %ls for null action component %ls.", wzRestartResource, wzComponent);
            continue;
        }

        // Get the type from Attributes and add to the Restart Manager.
        iType = iAttributes & etTypeMask;
        switch (iType)
        {
        case etFilename:
            WcaLog(LOGMSG_VERBOSE, "Adding file %ls for component %ls to register with the Restart Manager.", wzResource, wzComponent);
            hr = RmuAddFile(pSession, wzResource);
            ExitOnFailure(hr, "Failed to add the filename to the Restart Manager session.");
            break;

        case etServiceName:
            WcaLog(LOGMSG_VERBOSE, "Adding service name %ls for component %ls to register with the Restart Manager.", wzResource, wzComponent);
            hr = RmuAddService(pSession, wzResource);
            ExitOnFailure(hr, "Failed to add the service name to the Restart Manager session.");
            break;

        default:
            WcaLog(LOGMSG_VERBOSE, "The resource type %d for %ls is not supported and will not be registered.", iType, wzRestartResource);
            break;
        }
    }

    if (E_NOMOREITEMS == hr)
    {
        hr = S_OK;
    }
    ExitOnFailure(hr, "Failed while looping through all rows to register resources.");

    // Register the resources and unjoin the session.
    hr = RmuEndSession(pSession);
    MessageExitOnFailure(hr, msierrRmuRegisterFailed, "Could not register the resources with the Restart Manager.");

LExit:
    ReleaseStr(wzRestartResource);
    ReleaseStr(wzComponent);
    ReleaseStr(wzResource);

    if (FAILED(hr))
    {
        er = ERROR_INSTALL_FAILURE;
    }

    return WcaFinalize(er);
}
