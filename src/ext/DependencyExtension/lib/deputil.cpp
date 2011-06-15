//-------------------------------------------------------------------------------------------------
// <copyright file="deputil.cpp" company="Microsoft">
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
//    Common function definitions for the dependency/ref-counting feature.
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"

#define ARRAY_GROWTH_SIZE 5

enum eRequiresAttributes { RequiresAttributesMinVersionInclusive = 256, RequiresAttributesMaxVersionInclusive = 512 };

// We write to Software\Classes explicitly based on the current security context instead of HKCR.
// See http://msdn.microsoft.com/en-us/library/ms724475(VS.85).aspx for more information.
LPCWSTR vsczRegistryRoot = L"Software\\Classes\\Installer\\Dependencies\\";
LPCWSTR vsczRegistryDependents = L"Dependents";
LPCWSTR vsczRegistryMachineARP = L"Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\";

static HRESULT AllocDependencyKeyName(
    __in_z LPCWSTR wzName,
    __deref_out_z LPWSTR *psczKeyName
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
    __deref_inout_ecount_opt(*pcDependencies) DEPENDENCY** prgDependencies,
    __inout LPUINT pcDependencies,
    __in_z LPCWSTR wzKey,
    __in_z_opt LPCWSTR wzName
    );

DAPI_(HRESULT) DepCheckDependencies(
    __in HKEY hkHive,
    __in_z LPCWSTR wzProviderKey,
    __in_z_opt LPCWSTR wzMinVersion,
    __in_z_opt LPCWSTR wzMaxVersion,
    __in int iAttributes,
    __in STRINGDICT_HANDLE sdDependencies,
    __deref_inout_ecount_opt(*pcDependencies) DEPENDENCY** prgDependencies,
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
    hr = AllocDependencyKeyName(wzProviderKey, &sczKey);
    ExitOnFailure1(hr, "Failed to allocate the registry key for dependency \"%ls\".", wzProviderKey);

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

DAPI_(HRESULT) DepCheckDependents(
    __in HKEY hkHive,
    __in_z LPCWSTR wzProviderKey,
    __reserved int /*iAttributes*/,
    __in C_STRINGDICT_HANDLE sdIgnoredDependents,
    __deref_inout_ecount_opt(*pcDependents) DEPENDENCY** prgDependents,
    __inout LPUINT pcDependents
    )
{
    HRESULT hr = S_OK;
    LPWSTR sczKey = NULL;
    HKEY hkProviderKey = NULL;
    HKEY hkDependentsKey = NULL;
    LPWSTR sczDependentKey = NULL;
    LPWSTR sczDependentName = NULL;

    // Format the provider dependency registry key.
    hr = AllocDependencyKeyName(wzProviderKey, &sczKey);
    ExitOnFailure1(hr, "Failed to allocate the registry key for dependency \"%ls\".", wzProviderKey);

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

DAPI_(HRESULT) DepRegisterDependency(
    __in HKEY hkHive,
    __in_z LPCWSTR wzProviderKey,
    __in_z LPCWSTR wzDisplayKey,
    __in_z LPCWSTR wzVersion,
    __in int iAttributes
    )
{
    HRESULT hr = S_OK;
    LPWSTR sczKey = NULL;
    HKEY hkKey = NULL;
    BOOL fCreated = FALSE;

    // Format the provider dependency registry key.
    hr = AllocDependencyKeyName(wzProviderKey, &sczKey);
    ExitOnFailure1(hr, "Failed to allocate the registry key for dependency \"%ls\".", wzProviderKey);

    // Create the dependency key (or open it if it already exists).
    hr = RegCreateEx(hkHive, sczKey, KEY_WRITE, FALSE, NULL, &hkKey, &fCreated);
    ExitOnFailure1(hr, "Failed to create the dependency registry key \"%ls\".", sczKey);

    // Set the display key name as the default value.
    hr = RegWriteString(hkKey, NULL, wzDisplayKey);
    ExitOnFailure1(hr, "Failed to set the default registry value to \"%ls\".", wzDisplayKey);

    // Set the version.
    hr = RegWriteString(hkKey, L"Version", wzVersion);
    ExitOnFailure1(hr, "Failed to set the Version registry value to \"%ls\".", wzVersion);

    // Set the attributes if non-zero.
    if (0 != iAttributes)
    {
        hr = RegWriteNumber(hkKey, L"Attributes", static_cast<DWORD>(iAttributes));
        ExitOnFailure1(hr, "Failed to set the Attributes registry value to %d.", iAttributes);
    }

    // If no failures, return whether the key was created or simply opened.
    hr = fCreated ? S_OK : S_FALSE;

LExit:
    ReleaseRegKey(hkKey);
    ReleaseStr(sczKey);

    return hr;
}

DAPI_(HRESULT) DepRegisterDependent(
    __in HKEY hkHive,
    __in_z LPCWSTR wzDependencyProviderKey,
    __in_z LPCWSTR wzProviderKey,
    __in_z_opt LPCWSTR wzMinVersion,
    __in_z_opt LPCWSTR wzMaxVersion,
    __in int iAttributes
    )
{
    HRESULT hr = S_OK;
    LPWSTR sczDependencyKey = NULL;
    HKEY hkDependencyKey = NULL;
    LPWSTR sczKey = NULL;
    HKEY hkKey = NULL;
    BOOL fCreated = FALSE;

    // Format the provider dependency registry key.
    hr = AllocDependencyKeyName(wzDependencyProviderKey, &sczDependencyKey);
    ExitOnFailure1(hr, "Failed to allocate the registry key for dependency \"%ls\".", wzDependencyProviderKey);

    // Try to open the dependency key. If that does not exist, simply return S_FALSE.
    hr = RegOpen(hkHive, sczDependencyKey, KEY_READ, &hkDependencyKey);
    if (E_FILENOTFOUND == hr)
    {
        hr = S_FALSE;
        ExitFunction();
    }
    ExitOnFailure1(hr, "Failed to open the registry key for the dependency \"%ls\".", wzDependencyProviderKey);

    // Create the sub-key to register the dependent.
    hr = StrAllocFormatted(&sczKey, L"%ls\\%ls", vsczRegistryDependents, wzProviderKey);
    ExitOnFailure2(hr, "Failed to allocate dependent sub-key \"%ls\" under dependency \"%ls\".", wzProviderKey, wzDependencyProviderKey);

    hr = RegCreateEx(hkDependencyKey, sczKey, KEY_WRITE, FALSE, NULL, &hkKey, &fCreated);
    ExitOnFailure1(hr, "Failed to create the dependency sub-key \"%ls\".", sczKey);

    // Set the minimum version if not NULL.
    hr = RegWriteString(hkKey, L"MinVersion", wzMinVersion);
    ExitOnFailure1(hr, "Failed to set the MinVersion registry value to \"%ls\".", wzMinVersion);

    // Set the maximum version if not NULL.
    hr = RegWriteString(hkKey, L"MaxVersion", wzMaxVersion);
    ExitOnFailure1(hr, "Failed to set the MaxVersion registry value to \"%ls\".", wzMaxVersion);

    // Set the attributes if non-zero.
    if (0 != iAttributes)
    {
        hr = RegWriteNumber(hkKey, L"Attributes", static_cast<DWORD>(iAttributes));
        ExitOnFailure1(hr, "Failed to set the Attributes registry value to %d.", iAttributes);
    }

    // If no failures, return whether the key was created or simply opened.
    hr = fCreated ? S_OK : S_FALSE;

LExit:
    ReleaseRegKey(hkKey);
    ReleaseStr(sczKey);
    ReleaseRegKey(hkDependencyKey);
    ReleaseStr(sczDependencyKey);

    return hr;
}

DAPI_(HRESULT) DepUnregisterDependency(
    __in HKEY hkHive,
    __in_z LPCWSTR wzProviderKey
    )
{
    HRESULT hr = S_OK;
    LPWSTR sczKey = NULL;
    HKEY hkKey = NULL;

    // Format the provider dependency registry key.
    hr = AllocDependencyKeyName(wzProviderKey, &sczKey);
    ExitOnFailure1(hr, "Failed to allocate the registry key for dependency \"%ls\".", wzProviderKey);

    // Delete the entire key including all sub-keys.
    hr = RegDelete(hkHive, sczKey, REG_KEY_DEFAULT, TRUE);
    if (E_FILENOTFOUND == hr)
    {
        hr = S_FALSE;
        ExitFunction();
    }
    ExitOnFailure1(hr, "Failed to delete the key \"%ls\".", sczKey);

LExit:
    ReleaseRegKey(hkKey);
    ReleaseStr(sczKey);

    return hr;
}

DAPI_(HRESULT) DepUnregisterDependent(
    __in HKEY hkHive,
    __in_z LPCWSTR wzDependencyProviderKey,
    __in_z LPCWSTR wzProviderKey
    )
{
    HRESULT hr = S_OK;
    HKEY hkRegistryRoot = NULL;
    HKEY hkDependencyProviderKey = NULL;
    HKEY hkRegistryDependents = NULL;
    LPWSTR sczSubkey = NULL;
    DWORD64 qwVersion = 0;

    // Open the root key. We may delete the wzDependencyProviderKey during clean up.
    hr = RegOpen(hkHive, vsczRegistryRoot, KEY_READ, &hkRegistryRoot);
    if (E_FILENOTFOUND == hr)
    {
        hr = S_FALSE;
        ExitFunction();
    }
    ExitOnFailure1(hr, "Failed to open root registry key \"%ls\".", vsczRegistryRoot);

    // Try to open the dependency key. If that does not exist, simply return S_FALSE.
    hr = RegOpen(hkRegistryRoot, wzDependencyProviderKey, KEY_READ, &hkDependencyProviderKey);
    if (E_FILENOTFOUND == hr)
    {
        hr = S_FALSE;
        ExitFunction();
    }
    ExitOnFailure1(hr, "Failed to open the registry key for the dependency \"%ls\".", wzDependencyProviderKey);

    // Try to open the dependents sub-key to enumerate.
    hr = RegOpen(hkDependencyProviderKey, vsczRegistryDependents, KEY_READ, &hkRegistryDependents);
    if (E_FILENOTFOUND == hr)
    {
        hr = S_FALSE;
        ExitFunction();
    }

    // Delete the wzProviderKey dependent sub-key.
    hr = RegDelete(hkRegistryDependents, wzProviderKey, REG_KEY_DEFAULT, TRUE);
    ExitOnFailure1(hr, "Failed to delete the dependency sub-key \"%ls\".", wzProviderKey);

    // If there are no remaining dependents, delete the Dependents sub-key.
    hr = RegKeyEnum(hkRegistryDependents, 0, &sczSubkey);
    if (E_NOMOREITEMS != hr)
    {
        ExitOnFailure1(hr, "Failed to enumerate sub-keys under the dependency \"%ls\".", wzDependencyProviderKey);

        hr = S_OK;
        ExitFunction();
    }

    // Release the handle to make sure it's deleted immediately.
    ReleaseRegKey(hkRegistryDependents);

    // Fail if there are any sub-keys since we just checked.
    hr = RegDelete(hkDependencyProviderKey, vsczRegistryDependents, REG_KEY_DEFAULT, FALSE);
    ExitOnFailure1(hr, "Failed to delete the dependents sub-key for the dependency \"%ls\".", wzDependencyProviderKey);

    // If the "Version" registry value is not found, delete the provider dependency key.
    hr = RegReadVersion(hkDependencyProviderKey, L"Version", &qwVersion);
    if (E_FILENOTFOUND == hr)
    {
        // Release the handle to make sure it's deleted immediately.
        ReleaseRegKey(hkDependencyProviderKey);

        // Fail if there are any sub-keys since we just checked.
        hr = RegDelete(hkRegistryRoot, wzDependencyProviderKey, REG_KEY_DEFAULT, FALSE);
        ExitOnFailure1(hr, "Failed to delete the dependency \"%ls\".", wzDependencyProviderKey);
    }
    else
    {
        ExitOnFailure1(hr, "Failed to read the Version registry value for the dependency \"%ls\".", wzDependencyProviderKey);
    }

LExit:
    ReleaseStr(sczSubkey);
    ReleaseRegKey(hkRegistryDependents);
    ReleaseRegKey(hkDependencyProviderKey);
    ReleaseRegKey(hkRegistryRoot);

    return hr;
}

DAPI_(void) DepDependencyArrayFree(
    __in_ecount(cDependencies) DEPENDENCY* rgDependencies,
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

/***************************************************************************
 AllocDependencyKeyName - Allocates and formats the root registry key name.

***************************************************************************/
static HRESULT AllocDependencyKeyName(
    __in_z LPCWSTR wzName,
    __deref_out_z LPWSTR *psczKeyName
    )
{
    HRESULT hr = S_OK;
    size_t cchName = 0;
    size_t cchKeyName = 0;

    // Get the length of the static registry root once.
    static size_t cchRegistryRoot = ::lstrlenW(vsczRegistryRoot);

    // Get the length of the dependency, and add to the length of the root.
    hr = ::StringCchLengthW(wzName, STRSAFE_MAX_CCH, &cchName);
    ExitOnFailure(hr, "Failed to get string length of dependency name.");

    // Add the sizes together to allocate memory once (callee will add space for nul).
    hr = ::SizeTAdd(cchRegistryRoot, cchName, &cchKeyName);
    ExitOnFailure(hr, "Failed to add the string lengths together.");

    // Allocate and concat the strings together.
    hr = StrAllocString(psczKeyName, vsczRegistryRoot, cchKeyName);
    ExitOnFailure(hr, "Failed to allocate string for dependency registry root.");

    hr = StrAllocConcat(psczKeyName, wzName, cchName);
    ExitOnFailure(hr, "Failed to concatenate the dependency key name.");

LExit:
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
    __in LPCWSTR wzProviderKey,
    __deref_out_z LPWSTR *psczName
    )
{
    HRESULT hr = S_OK;
    LPWSTR sczKey = NULL;
    HKEY hkKey = NULL;

    // Format the provider dependency registry key.
    hr = AllocDependencyKeyName(wzProviderKey, &sczKey);
    ExitOnFailure1(hr, "Failed to allocate the registry key for dependency \"%ls\".", wzProviderKey);

    // Try to open the key. If that fails, add the missing dependency key to the dependency array if it doesn't already exist.
    hr = RegOpen(hkHive, sczKey, KEY_READ, &hkKey);
    if (E_FILENOTFOUND == hr)
    {
        hr = S_FALSE;
        ExitFunction();
    }
    ExitOnFailure1(hr, "Failed to open the registry key for the dependency key \"%ls\".", wzProviderKey);

    // Now get the display name from ARP for the key.
    hr = GetDependencyName(hkKey, psczName);
    ExitOnFailure1(hr, "Failed to get the dependency name for the dependency key \"%ls\".", wzProviderKey);

LExit:
    ReleaseRegKey(hkKey);
    ReleaseStr(sczKey);

    return hr;
}

/***************************************************************************
 DependencyArrayAlloc - Allocates or expands an array of DEPENDENCY structs.

***************************************************************************/
static HRESULT DependencyArrayAlloc(
    __deref_inout_ecount_opt(*pcDependencies) DEPENDENCY** prgDependencies,
    __inout LPUINT pcDependencies,
    __in_z LPCWSTR wzKey,
    __in_z_opt LPCWSTR wzName
    )
{
    HRESULT hr = S_OK;
    UINT cRequired = 0;
    DEPENDENCY* pDependency = NULL;

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
