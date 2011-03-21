//-------------------------------------------------------------------------------------------------
// <copyright file="WixDepCA.cpp" company="Microsoft">
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
//    Custom action to check product dependencies.
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"

#define ARRAY_GROWTH_SIZE 5
#define INITIAL_STRINGDICT_SIZE 10

LPCWSTR vcsDependencyProviderQuery =
    L"SELECT `WixDependencyProvider`.`WixDependencyProvider`, `WixDependencyProvider`.`Component_`, `WixDependencyProvider`.`ProviderKey`, `WixDependencyProvider`.`DisplayKey`, `WixDependencyProvider`.`Attributes` "
    L"FROM `WixDependencyProvider`";
enum eDependencyProviderQuery { dpqId = 1, dpqComponent, dpqProviderKey, dpqDisplayKey, dpqAttributes };

LPCWSTR vcsDependencyQuery =
    L"SELECT `WixDependency`.`WixDependency`, `WixDependencyProvider`.`Component_`, `WixDependency`.`ProviderKey`, `WixDependency`.`MinVersion`, `WixDependency`.`MaxVersion`, `WixDependency`.`Attributes` "
    L"FROM `WixDependencyProvider`, `WixDependency`, `WixDependencyRef` "
    L"WHERE `WixDependency`.`WixDependency` = `WixDependencyRef`.`WixDependency_` AND `WixDependencyProvider`.`WixDependencyProvider` = `WixDependencyRef`.`WixDependencyProvider_`";
enum eDependencyComponentQuery { dqId = 1, dqComponent, dqProviderKey, dqMinVersion, dqMaxVersion, dqAttributes };

enum eRequiresAttributes { RequiresAttributesMinVersionInclusive = 256, RequiresAttributesMaxVersionInclusive = 512 };

LPCWSTR vsczRegistryRoot = L"Software\\Dependencies\\";
LPCWSTR vsczRegistryDependents = L"Dependents";
LPCWSTR vsczRegistryMachineARP = L"Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\";

typedef struct _DEPENDENCY
{
    LPWSTR sczKey;
    LPWSTR sczName;

} DEPENDENCY;

static HRESULT ValidateExistentDependencies(
    __in MSIHANDLE hInstall,
    __in BOOL fMachineContext
    );

static HRESULT ValidateNonexistentDependents(
    __in MSIHANDLE hInstall,
    __in BOOL fMachineContext
    );

static HRESULT SplitIgnoredDependents(
    __deref_inout_opt STRINGDICT_HANDLE *psdIgnoredDependents
    );

static HRESULT ProcessDependencyMessage(
    __in int iMessageId,
    __in int iReturnValue,
    __in_ecount(cDependencies) const DEPENDENCY *rgDependencies,
    __in UINT cDependencies
    );

// Following APIs should have no dependency on MSI.
//
static HRESULT CheckDependencies(
    __in HKEY hkHive,
    __in_z LPCWSTR wzProviderKey,
    __in_z_opt LPCWSTR wzMinVersion,
    __in_z_opt LPCWSTR wzMaxVersion,
    __in int iAttributes,
    __in STRINGDICT_HANDLE sdDependencies,
    __deref_inout_ecount_opt(*pcDependencies) DEPENDENCY **prgDependencies,
    __inout LPUINT pcDependencies
    );

static HRESULT CheckDependents(
    __in HKEY hkHive,
    __in_z LPCWSTR wzProviderKey,
    __in int iAttributes,
    __in C_STRINGDICT_HANDLE sdIgnoredDependents,
    __deref_inout_ecount_opt(*pcDependents) DEPENDENCY **prgDependents,
    __inout LPUINT pcDependents
    );

static HRESULT GetDependencyName(
    __in HKEY hkKey,
    __deref_out_z LPWSTR *psczName
    );

static HRESULT GetDependencyNameFromKey(
    __in HKEY hkKey,
    __in LPCWSTR wzKey,
    __deref_out_z LPWSTR *psczName
    );

static HRESULT DependencyArrayAlloc(
    __deref_inout_ecount_opt(*pcDependencies) DEPENDENCY **prgDependencies,
    __inout LPUINT pcDependencies,
    __in_z LPCWSTR wzKey,
    __in_z_opt LPCWSTR wzName
    );

static void DependencyArrayFree(
    __in_ecount(cDependencies) DEPENDENCY *rgDependencies,
    __in UINT cDependencies
    );

#define ReleaseDependencyArray(rg, c) { if (rg) { DependencyArrayFree(rg, c);  } }

/***************************************************************************
 WixDependencyCheck - Check dependencies based on component state.

 Note: may return ERROR_NO_MORE_ITEMS to terminate the session early.
***************************************************************************/
extern "C" UINT __stdcall WixDependencyCheck(
    __in MSIHANDLE hInstall
    )
{
    HRESULT hr = S_OK;
    UINT er = ERROR_SUCCESS;
    BOOL fMachineContext = FALSE;

    hr = WcaInitialize(hInstall, "WixDependencyCheck");
    ExitOnFailure(hr, "Failed to initialize.");

    hr = RegInitialize();
    ExitOnFailure(hr, "Failed to initialize the registry functions.");

    // Determine whether we're installing per-user or per-machine.
    fMachineContext = WcaIsPropertySet("ALLUSERS");

    // Check for any provider components being (re)installed that their requirements are already installed.
    hr = ValidateExistentDependencies(hInstall, fMachineContext);
    ExitOnFailure(hr, "Failed to validate existent dependencies for (re)installing components.");

    // Check for any dependencies for provider components being uninstalled.
    hr = ValidateNonexistentDependents(hInstall, fMachineContext);
    ExitOnFailure(hr, "Failed to validate nonexistent dependents for uninstalling components.");

LExit:
    RegUninitialize();

    er = FAILED(hr) ? ERROR_INSTALL_FAILURE : ERROR_SUCCESS;
    return WcaFinalize(er);
}

/***************************************************************************
 ValidateExistentDependencies - Check that dependencies are installed for
  any provider component that is being installed or reinstalled.

 Note: Skipped if DISABLEDEPENDENCYCHECK is set.
***************************************************************************/
static HRESULT ValidateExistentDependencies(
    __in MSIHANDLE hInstall,
    __in BOOL fMachineContext
    )
{
    HRESULT hr = S_OK;
    STRINGDICT_HANDLE sdDependencies = NULL;
    HKEY hkHive = NULL;
    PMSIHANDLE hView = NULL;
    PMSIHANDLE hRec = NULL;
    LPWSTR sczId = NULL;
    LPWSTR sczComponent = NULL;
    LPWSTR sczProviderKey = NULL;
    LPWSTR sczMinVersion = NULL;
    LPWSTR sczMaxVersion = NULL;
    int iAttributes = 0;
    WCA_TODO tComponentAction = WCA_TODO_UNKNOWN;
    DEPENDENCY *rgDependencies = NULL;
    UINT cDependencies = 0;

    // Skip the dependency check if requested.
    if (WcaIsPropertySet("DISABLEDEPENDENCYCHECK"))
    {
        WcaLog(LOGMSG_STANDARD, "Skipping the dependency check since DISABLEDEPENDENCYCHECK is set.");
        ExitFunction();
    }

    // Skip the dependency check if the WixDependency table is missing (no dependencies to check for).
    hr = WcaTableExists(L"WixDependency");
    if (S_FALSE == hr)
    {
        WcaLog(LOGMSG_STANDARD, "Skipping the dependency check since no dependencies are authored.");
        ExitFunction();
    }
    // If the table exists but not the others, the database was not authored correctly.
    ExitOnFailure(hr, "Failed to check if the WixDependency table exists.");

    // Initialize the dictionary to keep track of unique dependency keys.
    hr = DictCreateStringList(&sdDependencies, INITIAL_STRINGDICT_SIZE, DICT_FLAG_CASEINSENSITIVE);
    ExitOnFailure(hr, "Failed to initialize the unique dependency string list.");

    // Set the registry hive to use depending on install context.
    hkHive = fMachineContext ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER;

    // Loop over the provider components.
    hr = WcaOpenExecuteView(vcsDependencyQuery, &hView);
    ExitOnFailure(hr, "Failed to open the query view for dependencies.");

    while (S_OK == (hr = WcaFetchRecord(hView, &hRec)))
    {
        hr = WcaGetRecordString(hRec, dqId, &sczId);
        ExitOnFailure(hr, "Failed to get WixDependency.WixDependency.");

        hr = WcaGetRecordString(hRec, dqComponent, &sczComponent);
        ExitOnFailure(hr, "Failed to get WixDependencyProvider.Component_.");

        // Skip the current component if its not being installed or reinstalled.
        tComponentAction = WcaGetComponentToDo(sczComponent);
        if (WCA_TODO_INSTALL != tComponentAction && WCA_TODO_REINSTALL != tComponentAction)
        {
            WcaLog(LOGMSG_STANDARD, "Skipping dependency check for %ls because the component %ls is not being (re)installed.", sczId, sczComponent);
            continue;
        }

        hr = WcaGetRecordString(hRec, dqProviderKey, &sczProviderKey);
        ExitOnFailure(hr, "Failed to get WixDependency.ProviderKey.");

        hr = WcaGetRecordString(hRec, dqMinVersion, &sczMinVersion);
        ExitOnFailure(hr, "Failed to get WixDependency.MinVersion.");

        hr = WcaGetRecordString(hRec, dqMaxVersion, &sczMaxVersion);
        ExitOnFailure(hr, "Failed to get WixDependency.MaxVersion.");

        hr = WcaGetRecordInteger(hRec, dqAttributes, &iAttributes);
        ExitOnFailure(hr, "Failed to get WixDependency.Attributes.");

        // Check the registry to see if the required providers (dependencies) exist.
        hr = CheckDependencies(hkHive, sczProviderKey, sczMinVersion, sczMaxVersion, iAttributes, sdDependencies, &rgDependencies, &cDependencies);
        ExitOnFailure1(hr, "Failed dependency check for %ls.", sczId);
    }

    if (E_NOMOREITEMS == hr)
    {
        hr = S_OK;
    }
    ExitOnFailure(hr, "Failed to enumerate all of the rows in the dependency query view.");

    // If we collected any dependencies in the previous check, pump a message and prompt the user.
    if (0 < cDependencies)
    {
        hr = ProcessDependencyMessage(msierrDependencyMissingDependencies, ERROR_INSTALL_USEREXIT, rgDependencies, cDependencies);
        ExitOnFailure1(hr, "Failed to process the dependency message %d.", msierrDependencyMissingDependencies);
    }

LExit:
    ReleaseDependencyArray(rgDependencies, cDependencies);
    ReleaseStr(sczId);
    ReleaseStr(sczComponent);
    ReleaseStr(sczProviderKey);
    ReleaseStr(sczMinVersion);
    ReleaseStr(sczMaxVersion);
    ReleaseDict(sdDependencies);

    return hr;
}

/***************************************************************************
 ValidateNonexistentDependents - Checks that there are no dependents
  registered for providers that are being uninstalled.

 Note: Skipped if UPGRADINGPRODUCTCODE is set.
***************************************************************************/
static HRESULT ValidateNonexistentDependents(
    __in MSIHANDLE hInstall,
    __in BOOL fMachineContext
    )
{
    HRESULT hr = S_OK;
    STRINGDICT_HANDLE sdIgnoredDependents = NULL;
    HKEY hkHive = NULL;
    PMSIHANDLE hView = NULL;
    PMSIHANDLE hRec = NULL;
    LPWSTR sczId = NULL;
    LPWSTR sczComponent = NULL;
    LPWSTR sczProviderKey = NULL;
    int iAttributes = 0;
    WCA_TODO tComponentAction = WCA_TODO_UNKNOWN;
    DEPENDENCY *rgDependents = NULL;
    UINT cDependents = 0;

    // Skip the dependency check if requested nested as part of a major ugprade.
    if (WcaIsPropertySet("UPGRADINGPRODUCTCODE"))
    {
        WcaLog(LOGMSG_STANDARD, "Skipping the dependents check since UPGRADINGPRODUCTCODE is set.");
        ExitFunction();
    }

    // Split the IGNOREDEPENDENCIES property for use below if set. If it is "ALL", then quit now.
    hr = SplitIgnoredDependents(&sdIgnoredDependents);
    ExitOnFailure(hr, "Failed to get the ignored dependents.");

    hr = DictKeyExists(sdIgnoredDependents, L"ALL");
    if (S_OK == hr)
    {
        WcaLog(LOGMSG_STANDARD, "Skipping the dependencies check since IGNOREDEPENDENCIES contains \"ALL\".");

        hr = S_FALSE;
        ExitFunction();
    }
    else if (E_NOTFOUND == hr)
    {
        // Key was not found, so proceed.
        hr = S_OK;
    }
    ExitOnFailure(hr, "Failed to check if \"ALL\" was set in IGNOREDEPENDENCIES.");

    // Skip the dependent check if the WixDependencyProvider table is missing (no dependency providers).
    hr = WcaTableExists(L"WixDependencyProvider");
    if (S_FALSE == hr)
    {
        WcaLog(LOGMSG_STANDARD, "Skipping the dependents check since no dependency providers are authored.");
        ExitFunction();
    }
    ExitOnFailure(hr, "Failed to check if the WixDependencyProvider table exists.");

    // Set the registry hive to use depending on install context.
    hkHive = fMachineContext ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER;

    // Loop over the provider components.
    hr = WcaOpenExecuteView(vcsDependencyProviderQuery, &hView);
    ExitOnFailure(hr, "Failed to open the query view for dependency providers.");

    while (S_OK == (hr = WcaFetchRecord(hView, &hRec)))
    {
        hr = WcaGetRecordString(hRec, dpqId, &sczId);
        ExitOnFailure(hr, "Failed to get WixDependencyProvider.WixDependencyProvider.");

        hr = WcaGetRecordString(hRec, dpqComponent, &sczComponent);
        ExitOnFailure(hr, "Failed to get WixDependencyProvider.Component.");

        // Skip the current component if its not being uninstalled.
        tComponentAction = WcaGetComponentToDo(sczComponent);
        if (WCA_TODO_UNINSTALL != tComponentAction)
        {
            WcaLog(LOGMSG_STANDARD, "Skipping dependents check for %ls because the component %ls is not being uninstalled.", sczId, sczComponent);
            continue;
        }

        hr = WcaGetRecordString(hRec, dpqProviderKey, &sczProviderKey);
        ExitOnFailure(hr, "Failed to get WixDependencyProvider.ProviderKey.");

        hr = WcaGetRecordInteger(hRec, dpqAttributes, &iAttributes);
        ExitOnFailure(hr, "Failed to get WixDependencyProvider.Attributes.");

        // Check the registry to see if the provider has any dependents registered.
        hr = CheckDependents(hkHive, sczProviderKey, iAttributes, sdIgnoredDependents, &rgDependents, &cDependents);
        ExitOnFailure1(hr, "Failed dependents check for %ls.", sczId);
    }

    if (E_NOMOREITEMS == hr)
    {
        hr = S_OK;
    }
    ExitOnFailure(hr, "Failed to enumerate all of the rows in the dependency provider query view.");

    // If we collected any providers with dependents in the previous check, pump a message and prompt the user.
    if (0 < cDependents)
    {
        hr = ProcessDependencyMessage(msierrDependencyHasDependents, ERROR_NO_MORE_ITEMS, rgDependents, cDependents);
        ExitOnFailure1(hr, "Failed to process the dependency message %d.", msierrDependencyHasDependents);
    }

LExit:
    ReleaseDependencyArray(rgDependents, cDependents);
    ReleaseStr(sczId);
    ReleaseStr(sczComponent);
    ReleaseStr(sczProviderKey);

    return hr;
}

/***************************************************************************
 SplitIgnoredDependents - Splits the IGNOREDEPENDENCIES property into a map.

 Notes: Returns S_FALSE if the property is not set.
***************************************************************************/
static HRESULT SplitIgnoredDependents(
    __deref_inout_opt STRINGDICT_HANDLE *psdIgnoredDependents
    )
{
    HRESULT hr = S_OK;
    LPWSTR sczIgnoreDependencies = NULL;
    LPCWSTR wzDelim = L";";
    LPWSTR wzContext = NULL;

    hr = WcaGetProperty(L"IGNOREDEPENDENCIES", &sczIgnoreDependencies);
    ExitOnFailure(hr, "Failed to get the string value of the IGNOREDEPENDENCIES property.");

    hr = DictCreateStringList(psdIgnoredDependents, INITIAL_STRINGDICT_SIZE, DICT_FLAG_CASEINSENSITIVE);
    ExitOnFailure(hr, "Failed to create the string dictionary.");

    // Parse through the semicolon-delimited tokens and add to the string dictionary.
    for (LPCWSTR wzToken = ::wcstok_s(sczIgnoreDependencies, wzDelim, &wzContext); wzToken; wzToken = ::wcstok_s(NULL, wzDelim, &wzContext))
    {
        hr = DictAddKey(*psdIgnoredDependents, wzToken);
        ExitOnFailure1(hr, "Failed to ignored dependency \"%ls\" to the string dictionary.", wzToken);
    }

LExit:
    ReleaseStr(sczIgnoreDependencies);

    return hr;
}

/***************************************************************************
 ProcessDependencyMessage - Sends a message to the user or bootstrapper application
  prompting to continue with the operation or abort.

 Notes: If IDOK is returend, there was no direct user interaction.
***************************************************************************/
static HRESULT ProcessDependencyMessage(
    __in int iMessageId,
    __in int iReturnValue,
    __in_ecount(cDependencies) const DEPENDENCY *rgDependencies,
    __in UINT cDependencies
    )
{
    HRESULT hr = S_OK;
    UINT er = ERROR_SUCCESS;
    UINT cParams = 0;
    PMSIHANDLE hRec = NULL;
    UINT iParam = 0;

    // Calculate the number of parameters based on the format:
    // msgId, count, key1, name1, key2, name2, etc.
    cParams = 2 + 2 * cDependencies;

    hRec = ::MsiCreateRecord(cParams);
    ExitOnNull(hRec, hr, E_OUTOFMEMORY, "Not enough memory to create the message record.");

    er = ::MsiRecordSetInteger(hRec, ++iParam, iMessageId);
    ExitOnFailure(hr = HRESULT_FROM_WIN32(er), "Failed to set the message identifier into the message record.");

    er = ::MsiRecordSetInteger(hRec, ++iParam, cDependencies);
    ExitOnFailure(hr = HRESULT_FROM_WIN32(er), "Failed to set the number of dependencies into the message record.");

    // Now loop through each dependency and add the key and name to the record.
    for (UINT i = 0; i < cDependencies; i++)
    {
        // Log message type-specific information.
        switch (iMessageId)
        {
        case msierrDependencyMissingDependencies:
            WcaLog(LOGMSG_VERBOSE, "The dependency \"%ls\" is missing or is not the required version.", rgDependencies[i].sczKey);
            break;

        case msierrDependencyHasDependents:
            WcaLog(LOGMSG_VERBOSE, "Found existing dependent \"%ls\".", rgDependencies[i].sczKey);
            break;
        }

        er = ::MsiRecordSetStringW(hRec, ++iParam, rgDependencies[i].sczKey);
        ExitOnFailure1(hr = HRESULT_FROM_WIN32(er), "Failed to set the dependency key \"%ls\" into the message record.", rgDependencies[i].sczKey);

        er = ::MsiRecordSetStringW(hRec, ++iParam, rgDependencies[i].sczName);
        ExitOnFailure1(hr = HRESULT_FROM_WIN32(er), "Failed to set the dependency name \"%ls\" into the message record.", rgDependencies[i].sczName);
    }

    // Send a yes/no message with a warning icon since continuing could be detrimental.
    // This is sent as a USER message to better detect whether a user or dependency-aware bootstrapper is responding
    // or if Windows Installer or a dependency-unaware boostrapper is returning a typical default response.
    er = WcaProcessMessage(static_cast<INSTALLMESSAGE>(INSTALLMESSAGE_USER | MB_ICONWARNING | MB_YESNO | MB_DEFBUTTON2), hRec);
    switch (er)
    {
    // Only a user or dependency-aware bootstrapper can return IDYES to continue anyway.
    case IDYES:
        hr = S_OK;
        ExitFunction();

    // Only a user or dependency-aware bootstrapper would typically return IDNO.
    case IDNO:
        __fallthrough;

    // Bootstrappers which are not dependency-aware will likely return IDOK for unhandled messages.
    case IDOK:
        __fallthrough;

    // During quiet or passive installs, Windows Installer returns ERROR_SUCCESS for USER messages.
    case ERROR_SUCCESS:
        WcaSetReturnValue(iReturnValue);

        hr = S_FALSE;
        ExitFunction();

    default:
        hr = E_UNEXPECTED;
        ExitOnFailure1(hr, "Unexpected message response %d from user or bootstrapper application.", er);
    }

LExit:
    return hr;
}

/***************************************************************************
 CheckDependencies - Checks that all dependencies are registered
  and within the proper version range.

 Note: Returns S_FALSE if the authored dependencies were not found.
***************************************************************************/
static HRESULT CheckDependencies(
    __in HKEY hkHive,
    __in_z LPCWSTR wzProviderKey,
    __in_z_opt LPCWSTR wzMinVersion,
    __in_z_opt LPCWSTR wzMaxVersion,
    __in int iAttributes,
    __in STRINGDICT_HANDLE sdDependencies,
    __deref_inout_ecount_opt(*pcDependencies) DEPENDENCY **prgDependencies,
    __inout LPUINT pcDependencies
    )
{
    HRESULT hr = S_OK;
    LPWSTR sczKey = NULL;
    HKEY hkKey = NULL;
    DWORD64 dw64Version = 0;
    int cchMinVersion = 0;
    DWORD64 dw64MinVersion = 0;
    int cchMaxVersion = 0;
    DWORD64 dw64MaxVersion = 0;
    BOOL fAllowEqual = FALSE;
    LPWSTR sczName = NULL;

    // Format the provider dependency registry key.
    hr = StrAllocString(&sczKey, vsczRegistryRoot, 0);
    ExitOnFailure1(hr, "Failed to allocate the root registry key for the dependency check on \"%ls\".", wzProviderKey);

    hr = StrAllocConcat(&sczKey, wzProviderKey, 0);
    ExitOnFailure1(hr, "Failed to concatenate the dependency key \"%ls\" to the root registry key.", wzProviderKey);

    // Try to open the key. If that fails, add the missing dependency key to the dependency array if it doesn't already exist.
    hr = RegOpen(hkHive, sczKey, KEY_READ, &hkKey);
    if (SUCCEEDED(hr))
    {
        hr = RegReadVersion(hkKey, L"Version", &dw64Version);
        if (E_FILENOTFOUND == hr)
        {
            // The identifying Version value was not found.
            hr = S_FALSE;
        }
    }
    else if (E_FILENOTFOUND == hr)
    {
        // The dependency provider key was not found.
        hr = S_FALSE;
    }
    ExitOnFailure1(hr, "Failed to read the Version registry value from the registry key \"%ls\".", sczKey);

    // If the key was not found or the Version value was not found, add the missing dependency key to the dependency array.
    if (S_FALSE == hr)
    {
        hr = DictKeyExists(sdDependencies, wzProviderKey);
        if (E_NOTFOUND == hr)
        {
            hr = DependencyArrayAlloc(prgDependencies, pcDependencies, wzProviderKey, NULL);
            ExitOnFailure1(hr, "Failed to add the missing dependency \"%ls\" to the dependencies array.", wzProviderKey);

            hr = DictAddKey(sdDependencies, wzProviderKey);
            ExitOnFailure1(hr, "Failed to add the missing dependency \"%ls\" to the unique dependency string list.", wzProviderKey);
        }
        ExitOnFailure1(hr, "Failed to check the unique dependency string list for missing dependency \"%ls\".", wzProviderKey);

        // Exit since the check already failed.
        ExitFunction();
    }
    ExitOnFailure1(hr, "Failed to open the registry key \"%ls\".", sczKey);

    // Check MinVersion if provided.
    if (wzMinVersion)
    {
        cchMinVersion = lstrlenW(wzMinVersion);
        if (0 < cchMinVersion)
        {
            hr = FileVersionFromStringEx(wzMinVersion, cchMinVersion, &dw64MinVersion);
            ExitOnFailure1(hr, "Failed to get the 64-bit version number from \"%ls\".", wzMinVersion);

            fAllowEqual = iAttributes & RequiresAttributesMinVersionInclusive;
            if (!(fAllowEqual && dw64MinVersion <= dw64Version || dw64MinVersion < dw64Version))
            {
                hr = DictKeyExists(sdDependencies, wzProviderKey);
                if (E_NOTFOUND == hr)
                {
                    hr = GetDependencyName(hkKey, &sczName);
                    ExitOnFailure1(hr, "Failed to get the name of the older dependency \"%ls\".", wzProviderKey);

                    hr = DependencyArrayAlloc(prgDependencies, pcDependencies, wzProviderKey, sczName);
                    ExitOnFailure1(hr, "Failed to add the older dependency \"%ls\" to the dependencies array.", wzProviderKey);

                    hr = DictAddKey(sdDependencies, wzProviderKey);
                    ExitOnFailure1(hr, "Failed to add the older dependency \"%ls\" to the unique dependency string list.", wzProviderKey);
                }
                ExitOnFailure1(hr, "Failed to check the unique dependency string list for the older dependency \"%ls\".", wzProviderKey);

                // Exit since the check already failed.
                ExitFunction();
            }
        }
    }

    // Check MaxVersion if provided.
    if (wzMaxVersion)
    {
        cchMaxVersion = lstrlenW(wzMaxVersion);
        if (0 < cchMaxVersion)
        {
            hr = FileVersionFromStringEx(wzMaxVersion, cchMaxVersion, &dw64MaxVersion);
            ExitOnFailure1(hr, "Failed to get the 64-bit version number from \"%ls\".", wzMaxVersion);

            fAllowEqual = iAttributes & RequiresAttributesMaxVersionInclusive;
            if (!(fAllowEqual && dw64Version <= dw64MaxVersion || dw64Version < dw64MaxVersion))
            {
                hr = DictKeyExists(sdDependencies, wzProviderKey);
                if (E_NOTFOUND == hr)
                {
                    hr = GetDependencyName(hkKey, &sczName);
                    ExitOnFailure1(hr, "Failed to get the name of the newer dependency \"%ls\".", wzProviderKey);

                    hr = DependencyArrayAlloc(prgDependencies, pcDependencies, wzProviderKey, sczName);
                    ExitOnFailure1(hr, "Failed to add the newer dependency \"%ls\" to the dependencies array.", wzProviderKey);

                    hr = DictAddKey(sdDependencies, wzProviderKey);
                    ExitOnFailure1(hr, "Failed to add the newer dependency \"%ls\" to the unique dependency string list.", wzProviderKey);
                }
                ExitOnFailure1(hr, "Failed to check the unique dependency string list for the newer dependency \"%ls\".", wzProviderKey);

                // Exit since the check already failed.
                ExitFunction();
            }
        }
    }

LExit:
    ReleaseStr(sczName);
    ReleaseRegKey(hkKey);
    ReleaseStr(sczKey);

    return hr;
}

/***************************************************************************
 CheckDependents - Checks if any dependents are still installed for the
  given provider key.

 Notes: Returns S_FALSE if no authored dependents were found.
***************************************************************************/
static HRESULT CheckDependents(
    __in HKEY hkHive,
    __in_z LPCWSTR wzProviderKey,
    __in int iAttributes,
    __in C_STRINGDICT_HANDLE sdIgnoredDependents,
    __deref_inout_ecount_opt(*pcDependents) DEPENDENCY **prgDependents,
    __inout LPUINT pcDependents
    )
{
    HRESULT hr = S_OK;
    LPWSTR sczKey = NULL;
    HKEY hkProviderKey = NULL;
    HKEY hkDependentsKey = NULL;
    LPWSTR sczDependentKey = NULL;
    HKEY hkDependentKey = NULL;
    LPWSTR sczDependentName = NULL;

    // Format the provider dependency registry key.
    hr = StrAllocString(&sczKey, vsczRegistryRoot, 0);
    ExitOnFailure1(hr, "Failed to allocate the root registry key for the dependency check on \"%ls\".", wzProviderKey);

    hr = StrAllocConcat(&sczKey, wzProviderKey, 0);
    ExitOnFailure1(hr, "Failed to concatenate the dependency key \"%ls\" to the root registry key.", wzProviderKey);

    // Try to open the key. If that fails, the dependency information is corrupt.
    hr = RegOpen(hkHive, sczKey, KEY_READ, &hkProviderKey);
    ExitOnFailure1(hr, "Failed to open the registry key \"%ls\". The dependency store is corrupt.", sczKey);

    // Try to open the dependencies key. If that does not exist, there are no dependents so return S_FALSE;
    hr = RegOpen(hkProviderKey, vsczRegistryDependents, KEY_READ, &hkDependentsKey);
    if (E_FILENOTFOUND == hr)
    {
        hr = S_FALSE;
        ExitFunction();
    }
    ExitOnFailure1(hr, "Failed to open the registry key for dependents of \"%ls\".", wzProviderKey);

    // Now enumerate the dependent keys. If they are not defined in the ignored list, add them to the array.
    for (DWORD dwIndex = 0; ; ++dwIndex)
    {
        hr = RegKeyEnum(hkDependentsKey, dwIndex, &sczDependentKey);
        if (E_NOMOREITEMS == hr)
        {
            hr = S_OK;
            ExitFunction();
        }
        ExitOnFailure1(hr, "Failed to enumerate the dependents key of \"%ls\".", wzProviderKey);

        // If the key isn't ignored, add it to the dependent array.
        hr = DictKeyExists(sdIgnoredDependents, sczDependentKey);
        if (E_NOTFOUND == hr)
        {
            // Get the name of the dependent from the key.
            hr = GetDependencyNameFromKey(hkHive, sczDependentKey, &sczDependentName);
            ExitOnFailure1(hr, "Failed to get the name of the dependent from the key \"%ls\".", sczDependentKey);

            hr = DependencyArrayAlloc(prgDependents, pcDependents, sczDependentKey, sczDependentName);
            ExitOnFailure1(hr, "Failed to add the dependent key \"%ls\" to the string array.", sczDependentKey);
        }
        ExitOnFailure(hr, "Failed to check if the key exists in the ignored dependents property.");
    }

LExit:
    ReleaseStr(sczDependentName);
    ReleaseStr(sczDependentKey);
    ReleaseRegKey(hkDependentsKey);
    ReleaseRegKey(hkProviderKey);
    ReleaseStr(sczKey);

    return hr;
}

/***************************************************************************
 GetDependencyName - Attempts to name of the dependency.

 Notes: Returns S_FALSE if the dependency name is not registered.
***************************************************************************/
static HRESULT GetDependencyName(
    __in HKEY hkKey,
    __deref_out_z LPWSTR *psczName
    )
{
    HRESULT hr = S_OK;
    LPWSTR sczDisplayKey = NULL;
    HKEY hkDisplayKey = NULL;

    hr = RegReadString(hkKey, L"DisplayKey", &sczDisplayKey);
    if (E_FILENOTFOUND == hr)
    {
        // Attempt to get the default value which may also indicate the ARP key.
        hr = RegReadString(hkKey, NULL, &sczDisplayKey);
        if (E_FILENOTFOUND == hr)
        {
            // We can proceed without a dependency name.
            hr = S_FALSE;
            ExitFunction();
        }
        ExitOnFailure(hr, "Failed to get the default registry value.");
    }
    ExitOnFailure(hr, "Failed to get the DisplayKey registry value.");

    // TODO: Determine if there are per-user ARP keys; for now, check only HKLM.
    hr = StrAllocPrefix(&sczDisplayKey, vsczRegistryMachineARP, 0);
    ExitOnFailure(hr, "Failed to prepend the ARP root registry key path to the ARP registry key.");

    // Now open the ARP key and get the DisplayName.
    hr = RegOpen(HKEY_LOCAL_MACHINE, sczDisplayKey, KEY_READ, &hkDisplayKey);
    if (E_FILENOTFOUND == hr)
    {
        // We can proceed without a dependency name.
        hr = S_FALSE;
        ExitFunction();
    }
    ExitOnFailure1(hr, "Failed to open the ARP registry key \"%ls\".", sczDisplayKey);

    hr = RegReadString(hkDisplayKey, L"DisplayName", psczName);
    if (E_FILENOTFOUND == hr)
    {
        // We can proceed without a dependency name.
        hr = S_FALSE;
        ExitFunction();
    }
    ExitOnFailure1(hr, "Failed to get the DisplayName registry value from registry key \"%ls\".", sczDisplayKey);

LExit:
    ReleaseRegKey(hkDisplayKey);
    ReleaseStr(sczDisplayKey);

    return hr;
}

/***************************************************************************
 GetDependencyNameFromKey - Attempts to name of the dependency from the key.

 Notes: Returns S_FALSE if the dependency name is not registered.
***************************************************************************/
static HRESULT GetDependencyNameFromKey(
    __in HKEY hkHive,
    __in LPCWSTR wzKey,
    __deref_out_z LPWSTR *psczName
    )
{
    HRESULT hr = S_OK;
    LPWSTR sczKey = NULL;
    HKEY hkKey = NULL;

    // Format the provider dependency registry key.
    hr = StrAllocString(&sczKey, vsczRegistryRoot, 0);
    ExitOnFailure1(hr, "Failed to allocate the root registry key for the dependency check on \"%ls\".", wzKey);

    hr = StrAllocConcat(&sczKey, wzKey, 0);
    ExitOnFailure1(hr, "Failed to concatenate the dependency key \"%ls\" to the root registry key.", wzKey);

    // Try to open the key. If that fails, add the missing dependency key to the dependency array if it doesn't already exist.
    hr = RegOpen(hkHive, sczKey, KEY_READ, &hkKey);
    if (E_FILENOTFOUND == hr)
    {
        hr = S_FALSE;
        ExitFunction();
    }
    ExitOnFailure1(hr, "Failed to open the registry key for the dependency key \"%ls\".", wzKey);

    // Now get the display name from ARP for the key.
    hr = GetDependencyName(hkKey, psczName);
    ExitOnFailure1(hr, "Failed to get the dependency name for the dependency key \"%ls\".", wzKey);

LExit:
    ReleaseRegKey(hkKey);
    ReleaseStr(sczKey);

    return hr;
}

/***************************************************************************
 DependencyArrayAlloc - Allocates or expands an array of DEPENDENCY structs.

***************************************************************************/
static HRESULT DependencyArrayAlloc(
    __deref_inout_ecount_opt(*pcDependencies) DEPENDENCY **prgDependencies,
    __inout LPUINT pcDependencies,
    __in_z LPCWSTR wzKey,
    __in_z_opt LPCWSTR wzName
    )
{
    HRESULT hr = S_OK;
    UINT cRequired = 0;
    DEPENDENCY *pDependency = NULL;

    hr = ::UIntAdd(*pcDependencies, 1, &cRequired);
    ExitOnFailure(hr, "Failed to increment the number of elements required in the dependency array.");

    hr = MemEnsureArraySize(reinterpret_cast<LPVOID*>(prgDependencies), cRequired, sizeof(DEPENDENCY), ARRAY_GROWTH_SIZE);
    ExitOnFailure(hr, "Failed to allocate memory for the dependency array.");

    pDependency = static_cast<DEPENDENCY*>(&(*prgDependencies)[*pcDependencies]);
    ExitOnNull(pDependency, hr, E_POINTER, "The dependency element in the array is invalid.");

    hr = StrAllocString(&(pDependency->sczKey), wzKey, 0);
    ExitOnFailure(hr, "Failed to allocate the string key in the dependency array.");

    if (NULL != wzName)
    {
        hr = StrAllocString(&(pDependency->sczName), wzName, 0);
        ExitOnFailure(hr, "Failed to allocate the string name in the dependency array.");
    }

    // Update the number of current elements in the dependency array.
    *pcDependencies = cRequired;

LExit:
    return hr;
}

/***************************************************************************
 DependencyArrayFree - Frees an array of DEPENDENCY structs.

***************************************************************************/
static void DependencyArrayFree(
    __in_ecount(cDependencies) DEPENDENCY *rgDependencies,
    __in UINT cDependencies
    )
{
    for (UINT i = 0; i < cDependencies; ++i)
    {
        ReleaseStr(rgDependencies[i].sczKey);
        ReleaseStr(rgDependencies[i].sczName);
    }

    ReleaseMem(rgDependencies);
}
