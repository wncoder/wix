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

static HRESULT AllocDependencyKeyName(
    __in_z LPCWSTR wzName,
    __deref_out_z LPWSTR* psczKeyName
    );

static HRESULT OpenARPKey(
    __in HKEY hkHive,
    __in_z LPCWSTR wzSubKey,
    __out HKEY* phkARPKey
    );

static HRESULT GetDependencyName(
    __in HKEY hkHive,
    __in HKEY hkKey,
    __deref_out_z LPWSTR* psczName
    );

static HRESULT GetDependencyNameFromKey(
    __in HKEY hkHive,
    __in LPCWSTR wzKey,
    __deref_out_z LPWSTR* psczName
    );

static HRESULT DependencyArrayAlloc(
    __deref_inout_ecount_opt(*pcDependencies) DEPENDENCY** prgDependencies,
    __inout LPUINT pcDependencies,
    __in_z LPCWSTR wzKey,
    __in_z_opt LPCWSTR wzName
    );

DAPI_(HRESULT) DepCheckDependency(
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
    if (E_FILENOTFOUND != hr)
    {
        ExitOnFailure1(hr, "Failed to open the registry key for dependency \"%ls\".", wzProviderKey);

        hr = RegReadVersion(hkKey, L"Version", &dw64Version);
        if (E_FILENOTFOUND != hr)
        {
            ExitOnFailure1(hr, "Failed to read the Version registry value for dependency \"%ls\".", wzProviderKey);
        }
    }

    // If the key was not found or the Version value was not found, add the missing dependency key to the dependency array.
    if (E_FILENOTFOUND == hr)
    {
        hr = DictKeyExists(sdDependencies, wzProviderKey);
        if (E_NOTFOUND != hr)
        {
            ExitOnFailure1(hr, "Failed to check the dictionary for missing dependency \"%ls\".", wzProviderKey);
        }
        else
        {
            hr = DependencyArrayAlloc(prgDependencies, pcDependencies, wzProviderKey, NULL);
            ExitOnFailure1(hr, "Failed to add the missing dependency \"%ls\" to the array.", wzProviderKey);

            hr = DictAddKey(sdDependencies, wzProviderKey);
            ExitOnFailure1(hr, "Failed to add the missing dependency \"%ls\" to the dictionary.", wzProviderKey);
        }

        // Exit since the check already failed.
        ExitFunction1(hr = E_NOTFOUND);
    }

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
                if (E_NOTFOUND != hr)
                {
                    ExitOnFailure1(hr, "Failed to check the dictionary for the older dependency \"%ls\".", wzProviderKey);
                }
                else
                {
                    hr = GetDependencyName(hkHive, hkKey, &sczName);
                    ExitOnFailure1(hr, "Failed to get the name of the older dependency \"%ls\".", wzProviderKey);

                    hr = DependencyArrayAlloc(prgDependencies, pcDependencies, wzProviderKey, sczName);
                    ExitOnFailure1(hr, "Failed to add the older dependency \"%ls\" to the dependencies array.", wzProviderKey);

                    hr = DictAddKey(sdDependencies, wzProviderKey);
                    ExitOnFailure1(hr, "Failed to add the older dependency \"%ls\" to the unique dependency string list.", wzProviderKey);
                }

                // Exit since the check already failed.
                ExitFunction1(hr = E_NOTFOUND);
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
                if (E_NOTFOUND != hr)
                {
                    ExitOnFailure1(hr, "Failed to check the dictionary for the newer dependency \"%ls\".", wzProviderKey);
                }
                else
                {
                    hr = GetDependencyName(hkHive, hkKey, &sczName);
                    ExitOnFailure1(hr, "Failed to get the name of the newer dependency \"%ls\".", wzProviderKey);

                    hr = DependencyArrayAlloc(prgDependencies, pcDependencies, wzProviderKey, sczName);
                    ExitOnFailure1(hr, "Failed to add the newer dependency \"%ls\" to the dependencies array.", wzProviderKey);

                    hr = DictAddKey(sdDependencies, wzProviderKey);
                    ExitOnFailure1(hr, "Failed to add the newer dependency \"%ls\" to the unique dependency string list.", wzProviderKey);
                }

                // Exit since the check already failed.
                ExitFunction1(hr = E_NOTFOUND);
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

    // Try to open the dependencies key. If that does not exist, there are no dependents.
    hr = RegOpen(hkProviderKey, vsczRegistryDependents, KEY_READ, &hkDependentsKey);
    if (E_FILENOTFOUND != hr)
    {
        ExitOnFailure1(hr, "Failed to open the registry key for dependents of \"%ls\".", wzProviderKey);
    }
    else
    {
        ExitFunction1(hr = S_OK);
    }

    // Now enumerate the dependent keys. If they are not defined in the ignored list, add them to the array.
    for (DWORD dwIndex = 0; ; ++dwIndex)
    {
        hr = RegKeyEnum(hkDependentsKey, dwIndex, &sczDependentKey);
        if (E_NOMOREITEMS != hr)
        {
            ExitOnFailure1(hr, "Failed to enumerate the dependents key of \"%ls\".", wzProviderKey);
        }
        else
        {
            hr = S_OK;
            break;
        }

        // If the key isn't ignored, add it to the dependent array.
        hr = DictKeyExists(sdIgnoredDependents, sczDependentKey);
        if (E_NOTFOUND != hr)
        {
            ExitOnFailure(hr, "Failed to check the dictionary of ignored dependents.");
        }
        else
        {
            // Get the name of the dependent from the key.
            hr = GetDependencyNameFromKey(hkHive, sczDependentKey, &sczDependentName);
            ExitOnFailure1(hr, "Failed to get the name of the dependent from the key \"%ls\".", sczDependentKey);

            hr = DependencyArrayAlloc(prgDependents, pcDependents, sczDependentKey, sczDependentName);
            ExitOnFailure1(hr, "Failed to add the dependent key \"%ls\" to the string array.", sczDependentKey);
        }
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

    // Try to open the dependency key.
    hr = RegOpen(hkHive, sczDependencyKey, KEY_READ, &hkDependencyKey);
    ExitOnFailure1(hr, "Failed to open the registry key for the dependency \"%ls\".", wzDependencyProviderKey);

    // Create the subkey to register the dependent.
    hr = StrAllocFormatted(&sczKey, L"%ls\\%ls", vsczRegistryDependents, wzProviderKey);
    ExitOnFailure2(hr, "Failed to allocate dependent subkey \"%ls\" under dependency \"%ls\".", wzProviderKey, wzDependencyProviderKey);

    hr = RegCreateEx(hkDependencyKey, sczKey, KEY_WRITE, FALSE, NULL, &hkKey, &fCreated);
    ExitOnFailure1(hr, "Failed to create the dependency subkey \"%ls\".", sczKey);

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
    if (E_FILENOTFOUND != hr)
    {
        ExitOnFailure1(hr, "Failed to delete the key \"%ls\".", sczKey);
    }

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
    if (E_FILENOTFOUND != hr)
    {
        ExitOnFailure1(hr, "Failed to open root registry key \"%ls\".", vsczRegistryRoot);
    }
    else
    {
        ExitFunction();
    }

    // Try to open the dependency key. If that does not exist, simply return.
    hr = RegOpen(hkRegistryRoot, wzDependencyProviderKey, KEY_READ, &hkDependencyProviderKey);
    if (E_FILENOTFOUND != hr)
    {
        ExitOnFailure1(hr, "Failed to open the registry key for the dependency \"%ls\".", wzDependencyProviderKey);
    }
    else
    {
        ExitFunction();
    }

    // Try to open the dependents subkey to enumerate.
    hr = RegOpen(hkDependencyProviderKey, vsczRegistryDependents, KEY_READ, &hkRegistryDependents);
    if (E_FILENOTFOUND != hr)
    {
        ExitOnFailure1(hr, "Failed to open the dependents subkey for \"%ls\".", wzDependencyProviderKey);
    }
    else
    {
        ExitFunction();
    }

    // Delete the wzProviderKey dependent sub-key.
    hr = RegDelete(hkRegistryDependents, wzProviderKey, REG_KEY_DEFAULT, TRUE);
    ExitOnFailure1(hr, "Failed to delete the dependency subkey \"%ls\".", wzProviderKey);

    // If there are no remaining dependents, delete the Dependents subkey.
    hr = RegKeyEnum(hkRegistryDependents, 0, &sczSubkey);
    if (E_NOMOREITEMS != hr)
    {
        ExitOnFailure1(hr, "Failed to enumerate subkeys under the dependency \"%ls\".", wzDependencyProviderKey);
        ExitFunction1(S_OK);
    }

    // Release the handle to make sure it's deleted immediately.
    ReleaseRegKey(hkRegistryDependents);

    // Fail if there are any subkeys since we just checked.
    hr = RegDelete(hkDependencyProviderKey, vsczRegistryDependents, REG_KEY_DEFAULT, FALSE);
    ExitOnFailure1(hr, "Failed to delete the dependents subkey for the dependency \"%ls\".", wzDependencyProviderKey);

    // If the "Version" registry value is not found, delete the provider dependency key.
    hr = RegReadVersion(hkDependencyProviderKey, L"Version", &qwVersion);
    if (E_FILENOTFOUND != hr)
    {
        ExitOnFailure1(hr, "Failed to read the Version registry value for the dependency \"%ls\".", wzDependencyProviderKey);
    }
    else
    {
        // Release the handle to make sure it's deleted immediately.
        ReleaseRegKey(hkDependencyProviderKey);

        // Fail if there are any subkeys since we just checked.
        hr = RegDelete(hkRegistryRoot, wzDependencyProviderKey, REG_KEY_DEFAULT, FALSE);
        ExitOnFailure1(hr, "Failed to delete the dependency \"%ls\".", wzDependencyProviderKey);
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
    __deref_out_z LPWSTR* psczKeyName
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
 OpenARPKey - Opens the ARP registry key in the 32- or 64-bit hives.

 Note: Returns E_FILENOTFOUND if the key can be opened in neither the
       32- or 64-bit ARP registry keys.
***************************************************************************/
static HRESULT OpenARPKey(
    __in HKEY hkHive,
    __in_z LPCWSTR wzSubKey,
    __out HKEY* phkARPKey
    )
{
    HRESULT hr = S_OK;
    LPWSTR sczARPKey = NULL;
    HKEY hkARPKey = NULL;
    BOOL fWow64 = FALSE;

    hr = StrAllocString(&sczARPKey, L"Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\", 0);
    ExitOnFailure(hr, "Failed to allocate the root ARP registry key.");

    hr = StrAllocConcat(&sczARPKey, wzSubKey, 0);
    ExitOnFailure(hr, "Failed to allocate the full ARP registry key.");

    // First attempt to open the native ARP key.
    hr = RegOpen(hkHive, sczARPKey, KEY_READ, &hkARPKey);
    if (E_FILENOTFOUND != hr)
    {
        ExitOnFailure1(hr, "Failed to open the native ARP registry key\"%ls\".", sczARPKey);
    }
    else
    {
        // If that failed and we're running under WOW64 open the 64-bit ARP key.
        hr = ProcWow64(::GetCurrentProcess(), &fWow64);
        ExitOnFailure(hr, "Failed to determine if process is running under WOW64.");

        if (fWow64)
        {
            hr = RegOpen(hkHive, sczARPKey, KEY_READ | KEY_WOW64_64KEY, &hkARPKey);
            ExitOnFailure1(hr, "Failed to open the 64-bit ARP registry key \"%ls\".", sczARPKey);
        }
        else
        {
            // Key was found in neither hive.
            ExitOnRootFailure(hr = E_FILENOTFOUND, "Failed to open the ARP registry key.");
        }
    }

    *phkARPKey = hkARPKey;

LExit:
    ReleaseStr(sczARPKey);

    return hr;
}

/***************************************************************************
 GetDependencyName - Attempts to get the name of the dependency.

***************************************************************************/
static HRESULT GetDependencyName(
    __in HKEY hkHive,
    __in HKEY hkKey,
    __deref_out_z LPWSTR* psczName
    )
{
    HRESULT hr = S_OK;
    LPWSTR sczDisplayKey = NULL;
    HKEY hkARPKey = NULL;

    // First attempt to get the DisplayKey value which will determine the key in ARP.
    hr = RegReadString(hkKey, L"DisplayKey", &sczDisplayKey);
    if (E_FILENOTFOUND != hr)
    {
        ExitOnFailure(hr, "Failed to get the DisplayKey value for lookup in ARP.");
    }
    else
    {
        // If that failed, attempt to get the default value which may also indicate the key in ARP.
        hr = RegReadString(hkKey, NULL, &sczDisplayKey);
        if (E_FILENOTFOUND != hr)
        {
           ExitOnFailure(hr, "Failed to get the default value for lookup in ARP.");
        }
        else
        {
            // If both failed, we can continue without a display name.
            ExitFunction1(hr = S_OK);
        }
    }

    // Open the appropriate ARP key.
    hr = OpenARPKey(hkHive, sczDisplayKey, &hkARPKey);
    if (E_FILENOTFOUND != hr)
    {
        ExitOnFailure1(hr, "Failed to open the ARP registry key for \"%ls\".", sczDisplayKey);
    }
    else
    {
        // We can proceed without a dependency name.
        ExitFunction1(hr = S_OK);
    }

    // Read the display name for this product.
    hr = RegReadString(hkARPKey, L"DisplayName", psczName);
    if (E_FILENOTFOUND != hr)
    {
        ExitOnFailure1(hr, "Failed to get the DisplayName registry value for \"%ls\".", sczDisplayKey);
    }
    else
    {
        // We can proceed without a dependency name.
        hr = S_OK;
    }

LExit:
    ReleaseRegKey(hkARPKey);
    ReleaseStr(sczDisplayKey);

    return hr;
}

/***************************************************************************
 GetDependencyNameFromKey - Attempts to name of the dependency from the key.

***************************************************************************/
static HRESULT GetDependencyNameFromKey(
    __in HKEY hkHive,
    __in LPCWSTR wzProviderKey,
    __deref_out_z LPWSTR* psczName
    )
{
    HRESULT hr = S_OK;
    LPWSTR sczKey = NULL;
    HKEY hkKey = NULL;

    // Format the provider dependency registry key.
    hr = AllocDependencyKeyName(wzProviderKey, &sczKey);
    ExitOnFailure1(hr, "Failed to allocate the registry key for dependency \"%ls\".", wzProviderKey);

    // Try to open the dependency key.
    hr = RegOpen(hkHive, sczKey, KEY_READ, &hkKey);
    if (E_FILENOTFOUND != hr)
    {
        ExitOnFailure1(hr, "Failed to open the registry key for the dependency \"%ls\".", wzProviderKey);
    }
    else
    {
        ExitFunction1(hr = S_OK);
    }

    // Now get the display name from ARP for the key.
    hr = GetDependencyName(hkHive, hkKey, psczName);
    ExitOnFailure1(hr, "Failed to get the dependency name for the dependency \"%ls\".", wzProviderKey);

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

    if (wzName)
    {
        hr = StrAllocString(&(pDependency->sczName), wzName, 0);
        ExitOnFailure(hr, "Failed to allocate the string name in the dependency array.");
    }

    // Update the number of current elements in the dependency array.
    *pcDependencies = cRequired;

LExit:
    return hr;
}
