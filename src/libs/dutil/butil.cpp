//-------------------------------------------------------------------------------------------------
// <copyright file="butil.cpp" company="Outercurve Foundation">
//   Copyright (c) 2004, Outercurve Foundation.
//   This software is released under Microsoft Reciprocal License (MS-RL).
//   The license and further copyright text can be found in the file
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
// 
// <summary>
//    bundle helper functions
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"
#include "butil.h"

// constants
// From engine/registration.h
const LPCWSTR BUNDLE_REGISTRATION_REGISTRY_UNINSTALL_KEY = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall";
const LPCWSTR BUNDLE_REGISTRATION_REGISTRY_BUNDLE_UPGRADE_CODE = L"BundleUpgradeCode";
const LPCWSTR BUNDLE_REGISTRATION_REGISTRY_BUNDLE_PROVIDER_KEY = L"BundleProviderKey";

// Forward declarations.
HRESULT OpenBundleKey(
    __in LPCWSTR szBundleId,
    __in BUNDLE_INSTALL_CONTEXT context, 
    __inout HKEY *key);

/********************************************************************
BundleGetBundleInfo - Queries the bundle installation metadata for a given property

RETURNS:
    ERROR_INVALID_PARAMETER
        An invalid parameter was passed to the function.
    ERROR_UNKNOWN_PRODUCT   
        The bundle is not installed
    ERROR_UNKNOWN_PROPERTY 
        The property is unrecognized
    ERROR_MORE_DATA
        A buffer is too small to hold the requested data.
    E_NOTIMPL:
        Tried to read a bundle attribute for a type which has not been implemented

    All other returns are unexpected returns from other dutil methods.
********************************************************************/
extern "C" HRESULT DAPI BundleGetBundleInfo(
  __in LPCWSTR   szBundleId,                             // Bundle code
  __in LPCWSTR   szAttribute,                            // attribute name
  __out_ecount_opt(*pcchValueBuf) LPWSTR lpValueBuf,     // returned value, NULL if not desired
  __inout_opt                     LPDWORD pcchValueBuf   // in/out buffer character count
  )
{
    Assert(szBundleId && szAttribute);

    HRESULT hr = ERROR_UNKNOWN_PROPERTY;
    BUNDLE_INSTALL_CONTEXT context = BUNDLE_INSTALL_CONTEXT_MACHINE;
    LPWSTR szValue = NULL;
    HKEY bundleKey = NULL;
    DWORD cchSource = 0;
    DWORD dwType = 0;
    DWORD dwValue = 0;

    if ((lpValueBuf && !pcchValueBuf) || !szBundleId || !szAttribute)
    {
        ExitOnFailure(hr = ERROR_INVALID_PARAMETER, "An invalid parameter was passed to the function.");
    }

    if ((hr = OpenBundleKey(szBundleId, 
                                   context = BUNDLE_INSTALL_CONTEXT_MACHINE,
                                   &bundleKey)) != ERROR_SUCCESS &&
        (hr = OpenBundleKey(szBundleId, 
                                   context = BUNDLE_INSTALL_CONTEXT_USER,
                                   &bundleKey)) != ERROR_SUCCESS)
    {
        ExitOnFailure(hr = ERROR_UNKNOWN_PRODUCT, "Failed to locate bundle uninstall key path.");
    }

    // If the bundle doesn't have the property defined, return ERROR_UNKNOWN_PROPERTY
    hr = RegGetType(bundleKey, szAttribute, &dwType);
    if (FAILED(hr))
    {
        ExitOnFailure(hr = ERROR_UNKNOWN_PROPERTY, "Failed to locate bundle property.");
    }

    switch(dwType)
    {
        case REG_SZ:
            hr = RegReadString(bundleKey, szAttribute, &szValue);
            ExitOnFailure(hr, "Failed to read string property.");
            break;
        case REG_DWORD:
            hr = RegReadNumber(bundleKey, szAttribute, &dwValue);
            ExitOnFailure(hr, "Failed to read dword property.");

            hr = StrAllocFormatted(&szValue, L"%d", dwValue);
            ExitOnFailure(hr, "Failed to format dword property as string.");
            break;

        // TODO: read REG_MULTI_SZ, but as it's a multi value we would either need to concat it as a delimited string or provide an additional indexed input.
        //case REG_MULTI_SZ:

        default:
            ExitOnFailure1(hr = E_NOTIMPL, "Reading bundle info of type 0x%x not implemented.", dwType);

    }

    hr = ::StringCchLengthW(szValue, STRSAFE_MAX_CCH, reinterpret_cast<UINT_PTR*>(&cchSource));
    ExitOnFailure(hr, "Failed to calculate length of string");

    if(lpValueBuf)
    {
        // cchSource is the length of the string not including the terminating null character
        if (*pcchValueBuf <= cchSource)
        {
            *pcchValueBuf = cchSource++;
            ExitOnFailure(hr = ERROR_MORE_DATA, "A buffer is too small to hold the requested data.");
        }

        hr = ::StringCchCatNExW(lpValueBuf, *pcchValueBuf, szValue, cchSource, NULL, NULL, STRSAFE_FILL_BEHIND_NULL);
        ExitOnFailure(hr, "Failed to copy the property value to the output buffer.");
        
        *pcchValueBuf = cchSource++;        
    }
    
LExit:
    ReleaseRegKey(bundleKey);
    ReleaseStr(szValue);

    return hr;
}

/********************************************************************
OpenBundleKey - Opens the bundle uninstallation key for a given bundle

NOTE: caller is responsible for closing key
********************************************************************/
HRESULT OpenBundleKey(
    __in LPCWSTR szBundleId,
    __in BUNDLE_INSTALL_CONTEXT context, 
    __inout HKEY *key)
{
    Assert(key && szBundleId);
    AssertSz(NULL == *key, "*key should be null");

    HRESULT hr = S_OK;
    HKEY root = HKEY_LOCAL_MACHINE;
    REGSAM access = KEY_READ;
    LPWSTR keypath = NULL;

    if (context == BUNDLE_INSTALL_CONTEXT_USER)
    {
        root = HKEY_CURRENT_USER;
    }

    hr = StrAllocFormatted(&keypath, L"%ls\\%ls", BUNDLE_REGISTRATION_REGISTRY_UNINSTALL_KEY, szBundleId);
    ExitOnFailure(hr, "Failed to allocate bundle uninstall key path.");
    
    hr = RegOpen(root, keypath, access, key);
    ExitOnFailure(hr, "Failed to open bundle uninstall key path.");
LExit:
    ReleaseStr(keypath);

    return hr;
}

/********************************************************************
BundleEnumRelatedBundle - Queries the bundle installation metadata for installs with the given upgrade code

NOTE: lpBundleIdBuff is a buffer to receive the bundle GUID. This buffer must be 39 characters long. 
        The first 38 characters are for the GUID, and the last character is for the terminating null character.
RETURNS:
    ERROR_INVALID_PARAMETER
        An invalid parameter was passed to the function.

    All other returns are unexpected returns from other dutil methods.
********************************************************************/
HRESULT DAPI BundleEnumRelatedBundle(
  __in     LPCWSTR lpUpgradeCode,
  __in     BUNDLE_INSTALL_CONTEXT context,
  __inout  PDWORD pdwStartIndex,
  __out_ecount(MAX_GUID_CHARS+1)  LPWSTR lpBundleIdBuf
    )
{
    HRESULT hr = S_OK;
    HKEY root = HKEY_LOCAL_MACHINE;
    HKEY uninstallKey = NULL;
    HKEY bundleKey = NULL;
    REGSAM access = KEY_READ;
    LPWSTR uninstallSubKey = NULL;
    LPWSTR uninstallSubKeyPath = NULL;
    LPWSTR szValue = NULL;
    DWORD dwType = 0;

    LPWSTR* rgsczBundleUpgradeCodes = NULL;
    DWORD cBundleUpgradeCodes = 0;
    BOOL fUpgradeCodeFound = FALSE;

    LPWSTR szProviderKey = NULL;
    DWORD cchProviderKey = 0;

    if ((NULL == lpUpgradeCode)||(NULL == lpBundleIdBuf)||(NULL == pdwStartIndex))
    {
        ExitOnFailure(hr = ERROR_INVALID_PARAMETER, "An invalid parameter was passed to the function.");
    }

    if (context == BUNDLE_INSTALL_CONTEXT_USER)
    {
        root = HKEY_CURRENT_USER;
    }

    hr = RegOpen(root, BUNDLE_REGISTRATION_REGISTRY_UNINSTALL_KEY,access, &uninstallKey);
    ExitOnFailure(hr, "Failed to open bundle uninstall key path.");

    for(DWORD dwIndex = *pdwStartIndex; !fUpgradeCodeFound; dwIndex++)
    {
        hr = RegKeyEnum(uninstallKey, dwIndex, &uninstallSubKey);
        ExitOnFailure(hr, "Failed to enumerate bundle uninstall key path.");

        hr = StrAllocFormatted(&uninstallSubKeyPath, L"%ls\\%ls", BUNDLE_REGISTRATION_REGISTRY_UNINSTALL_KEY, uninstallSubKey);
        ExitOnFailure(hr, "Failed to allocate bundle uninstall key path.");
        
        hr = RegOpen(root, uninstallSubKeyPath, access, &bundleKey);
        ExitOnFailure(hr, "Failed to open uninstall key path.");

        // If it's a bundle, it should have a BundleUpgradeCode value of type REG_SZ (old) or REG_MULTI_SZ
        hr = RegGetType(bundleKey, BUNDLE_REGISTRATION_REGISTRY_BUNDLE_UPGRADE_CODE, &dwType);
        if (FAILED(hr))
        {
            ReleaseRegKey(bundleKey);
            ReleaseNullStr(uninstallSubKey);
            ReleaseNullStr(uninstallSubKeyPath);
            // Not a bundle
            continue;
        }

        switch(dwType)
        {
            case REG_SZ:
                hr = RegReadString(bundleKey, BUNDLE_REGISTRATION_REGISTRY_BUNDLE_UPGRADE_CODE, &szValue);
                ExitOnFailure(hr, "Failed to read BundleUpgradeCode string property.");
                if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, szValue, -1, lpUpgradeCode, -1))
                {
                    *pdwStartIndex = dwIndex;
                    fUpgradeCodeFound = TRUE;
                    break;
                }

                ReleaseNullStr(szValue);

                break;
            case REG_MULTI_SZ:
                hr = RegReadStringArray(bundleKey, BUNDLE_REGISTRATION_REGISTRY_BUNDLE_UPGRADE_CODE, &rgsczBundleUpgradeCodes, &cBundleUpgradeCodes);
                ExitOnFailure(hr, "Failed to read BundleUpgradeCode  multi-string property.");

                for (DWORD i = 0; i < cBundleUpgradeCodes; i++)
                {
                    LPWSTR wzUpgradeCode = rgsczBundleUpgradeCodes[i];
                    if (wzUpgradeCode && *wzUpgradeCode)
                    {
                        if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, wzUpgradeCode, -1, lpUpgradeCode, -1))
                        {
                            *pdwStartIndex = dwIndex;
                            fUpgradeCodeFound = TRUE;
                            break;
                        }
                    }
                }
                ReleaseNullStrArray(rgsczBundleUpgradeCodes, cBundleUpgradeCodes);

                break;

            default:
                ExitOnFailure1(hr = E_NOTIMPL, "BundleUpgradeCode of type 0x%x not implemented.", dwType);

        }

        if(fUpgradeCodeFound)
        {
            hr = RegReadString(bundleKey, BUNDLE_REGISTRATION_REGISTRY_BUNDLE_PROVIDER_KEY, &szProviderKey );
            ExitOnFailure(hr, "Failed to read the bundle provider key.");

            hr = ::StringCchLengthW(szProviderKey, STRSAFE_MAX_CCH, reinterpret_cast<UINT_PTR*>(&cchProviderKey));
            ExitOnFailure(hr, "Failed to calculate length of string");

            if(lpBundleIdBuf)
            {
                // cchSource is the lenght of the string not including the terminating null character
                //if (*pcchValueBuf <= cchProviderKey)
                //{
                //	*pcchValueBuf = cchSource++;
                //	ExitOnFailure(hr = ERROR_MORE_DATA, "A buffer is too small to hold the requested data.");
                //}


                hr = ::StringCchCatNExW(lpBundleIdBuf, 39, szProviderKey, cchProviderKey, NULL, NULL, STRSAFE_FILL_BEHIND_NULL);
                ExitOnFailure(hr, "Failed to copy the property value to the output buffer.");
        
                //*pcchValueBuf = cchSource++;        
            }

            break;
        }

        // Cleanup before next iteration
        ReleaseRegKey(bundleKey);
        ReleaseNullStr(uninstallSubKey);
        ReleaseNullStr(uninstallSubKeyPath);
    }


LExit:
    ReleaseStr(szProviderKey);
    ReleaseStr(szValue);
    ReleaseStr(uninstallSubKey);
    ReleaseStr(uninstallSubKeyPath);
    ReleaseRegKey(bundleKey);
    ReleaseRegKey(uninstallKey);
    ReleaseStrArray(rgsczBundleUpgradeCodes, cBundleUpgradeCodes);


    return hr;
}
