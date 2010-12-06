//-------------------------------------------------------------------------------------------------
// <copyright file="regutil.cpp" company="Microsoft">
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
//    Registry helper functions.
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"

static PFN_REGCREATEKEYEXW vpfnRegCreateKeyExW = ::RegCreateKeyExW;
static PFN_REGOPENKEYEXW vpfnRegOpenKeyExW = ::RegOpenKeyExW;
static PFN_REGDELETEKEYEXW vpfnRegDeleteKeyExW = NULL;
static PFN_REGDELETEKEYEXW vpfnRegDeleteKeyExWFromLibrary = NULL;
static PFN_REGDELETEKEYW vpfnRegDeleteKeyW = ::RegDeleteKeyW;
static PFN_REGENUMKEYEXW vpfnRegEnumKeyExW = ::RegEnumKeyExW;
static PFN_REGENUMVALUEW vpfnRegEnumValueW = ::RegEnumValueW;
static PFN_REGQUERYINFOKEYW vpfnRegQueryInfoKeyW = ::RegQueryInfoKeyW;
static PFN_REGQUERYVALUEEXW vpfnRegQueryValueExW = ::RegQueryValueExW;
static PFN_REGSETVALUEEXW vpfnRegSetValueExW = ::RegSetValueExW;
static PFN_REGDELETEVALUEW vpfnRegDeleteValueW = ::RegDeleteValueW;

static HMODULE vhAdvApi32Dll = NULL;
static BOOL vfRegInitialized = FALSE;

/********************************************************************
 RegInitialize - initializes regutil

*********************************************************************/
extern "C" HRESULT DAPI RegInitialize(
    )
{
    HRESULT hr = S_OK;

    vhAdvApi32Dll = ::LoadLibraryW(L"AdvApi32.dll");
    ExitOnNullWithLastError(vhAdvApi32Dll, hr, "Failed to load AdvApi32.DLL");

    // ignore failures - if this doesn't exist, we'll fall back to RegDeleteKeyW
    vpfnRegDeleteKeyExWFromLibrary = reinterpret_cast<PFN_REGDELETEKEYEXW>(::GetProcAddress(vhAdvApi32Dll, "RegDeleteKeyExW"));

    if (NULL == vpfnRegDeleteKeyExW)
    {
        vpfnRegDeleteKeyExW = vpfnRegDeleteKeyExWFromLibrary;
    }

    vfRegInitialized = TRUE;

LExit:
    return hr;
}


/********************************************************************
 RegUninitialize - uninitializes regutil

*********************************************************************/
extern "C" void DAPI RegUninitialize(
    )
{
    if (vhAdvApi32Dll)
    {
        ::FreeLibrary(vhAdvApi32Dll);
        vhAdvApi32Dll = NULL;
        vpfnRegDeleteKeyExWFromLibrary = NULL;
        vpfnRegDeleteKeyExW = NULL;
    }

    vfRegInitialized = FALSE;
}


/********************************************************************
 RegFunctionOverride - overrides the registry functions. Typically used
                       for unit testing.

*********************************************************************/
extern "C" void DAPI RegFunctionOverride(
    __in_opt PFN_REGCREATEKEYEXW pfnRegCreateKeyExW,
    __in_opt PFN_REGOPENKEYEXW pfnRegOpenKeyExW,
    __in_opt PFN_REGDELETEKEYEXW pfnRegDeleteKeyExW,
    __in_opt PFN_REGENUMKEYEXW pfnRegEnumKeyExW,
    __in_opt PFN_REGENUMVALUEW pfnRegEnumValueW,
    __in_opt PFN_REGQUERYINFOKEYW pfnRegQueryInfoKeyW,
    __in_opt PFN_REGQUERYVALUEEXW pfnRegQueryValueExW,
    __in_opt PFN_REGSETVALUEEXW pfnRegSetValueExW,
    __in_opt PFN_REGDELETEVALUEW pfnRegDeleteValueW
    )
{
    vpfnRegCreateKeyExW = pfnRegCreateKeyExW ? pfnRegCreateKeyExW : ::RegCreateKeyExW;
    vpfnRegOpenKeyExW = pfnRegOpenKeyExW ? pfnRegOpenKeyExW : ::RegOpenKeyExW;
    vpfnRegDeleteKeyExW = pfnRegDeleteKeyExW ? pfnRegDeleteKeyExW : vpfnRegDeleteKeyExWFromLibrary;
    vpfnRegEnumKeyExW = pfnRegEnumKeyExW ? pfnRegEnumKeyExW : ::RegEnumKeyExW;
    vpfnRegEnumValueW = pfnRegEnumValueW ? pfnRegEnumValueW : ::RegEnumValueW;
    vpfnRegQueryInfoKeyW = pfnRegQueryInfoKeyW ? pfnRegQueryInfoKeyW : ::RegQueryInfoKeyW;
    vpfnRegQueryValueExW = pfnRegQueryValueExW ? pfnRegQueryValueExW : ::RegQueryValueExW;
    vpfnRegSetValueExW = pfnRegSetValueExW ? pfnRegSetValueExW : ::RegSetValueExW;
    vpfnRegDeleteValueW = pfnRegDeleteValueW ? pfnRegDeleteValueW : ::RegDeleteValueW;
}


/********************************************************************
 RegCreate - creates a registry key.

*********************************************************************/
extern "C" HRESULT DAPI RegCreate(
    __in HKEY hkRoot,
    __in_z LPCWSTR wzSubKey,
    __in DWORD dwAccess,
    __out HKEY* phk
    )
{
    HRESULT hr = S_OK;
    DWORD er = ERROR_SUCCESS;

    er = vpfnRegCreateKeyExW(hkRoot, wzSubKey, 0, NULL, REG_OPTION_NON_VOLATILE, dwAccess, NULL, phk, NULL);
    ExitOnWin32Error(er, hr, "Failed to create registry key.");

LExit:
    return hr;
}


/********************************************************************
 RegCreate - creates a registry key with extra options.

*********************************************************************/
HRESULT DAPI RegCreateEx(
    __in HKEY hkRoot,
    __in_z LPCWSTR wzSubKey,
    __in DWORD dwAccess,
    __in BOOL fVolatile,
    __in_opt SECURITY_ATTRIBUTES* pSecurityAttributes,
    __out HKEY* phk,
    __out_opt BOOL* pfCreated
    )
{
    HRESULT hr = S_OK;
    DWORD er = ERROR_SUCCESS;
    DWORD dwDisposition;

    er = vpfnRegCreateKeyExW(hkRoot, wzSubKey, 0, NULL, fVolatile ? REG_OPTION_VOLATILE : REG_OPTION_NON_VOLATILE, dwAccess, pSecurityAttributes, phk, &dwDisposition);
    ExitOnWin32Error(er, hr, "Failed to create registry key.");

    if (pfCreated)
    {
        *pfCreated = (REG_CREATED_NEW_KEY == dwDisposition);
    }

LExit:
    return hr;
}


/********************************************************************
 RegOpen - opens a registry key.

*********************************************************************/
extern "C" HRESULT DAPI RegOpen(
    __in HKEY hkRoot,
    __in_z LPCWSTR wzSubKey,
    __in DWORD dwAccess,
    __out HKEY* phk
    )
{
    HRESULT hr = S_OK;
    DWORD er = ERROR_SUCCESS;

    er = vpfnRegOpenKeyExW(hkRoot, wzSubKey, 0, dwAccess, phk);
    ExitOnWin32Error(er, hr, "Failed to open registry key.");

LExit:
    return hr;
}


/********************************************************************
 RegDelete - deletes a registry key (and optionally it's whole tree).

*********************************************************************/
extern "C" HRESULT DAPI RegDelete(
    __in HKEY hkRoot,
    __in_z LPCWSTR wzSubKey,
    __in REG_KEY_BITNESS kbKeyBitness,
    __in BOOL fDeleteTree
    )
{
    HRESULT hr = S_OK;
    DWORD er = ERROR_SUCCESS;
    LPWSTR pszEnumeratedSubKey = NULL;
    LPWSTR pszRecursiveSubKey = NULL;
    HKEY hkKey = NULL;
    REGSAM samDesired = 0;

    if (fDeleteTree)
    {
        hr = RegOpen(hkRoot, wzSubKey, KEY_READ, &hkKey);
        ExitOnFailure1(hr, "Failed to open this key for enumerating subkeys", wzSubKey);

        // Yes, keep enumerating the 0th item, because we're deleting it every time
        while (E_NOMOREITEMS != (hr = RegKeyEnum(hkKey, 0, &pszEnumeratedSubKey)))
        {
            ExitOnFailure(hr, "Failed to enumerate key 0");
            
            hr = PathConcat(wzSubKey, pszEnumeratedSubKey, &pszRecursiveSubKey);
            ExitOnFailure2(hr, "Failed to concatenate paths while recursively deleting subkeys. Path1: %ls, Path2: %ls", wzSubKey, pszEnumeratedSubKey);

            hr = RegDelete(hkRoot, pszRecursiveSubKey, kbKeyBitness, fDeleteTree);
            ExitOnFailure1(hr, "Failed to recursively delete subkey: %ls", pszRecursiveSubKey);
        }

        hr = S_OK;
    }

    if (!vfRegInitialized && REG_KEY_DEFAULT != kbKeyBitness)
    {
        hr = E_INVALIDARG;
        ExitOnFailure(hr, "RegInitialize must be called first in order to RegDelete() a key with non-default bit attributes!");
    }

    switch (kbKeyBitness)
    {
    case REG_KEY_32BIT:
        samDesired = KEY_WOW64_32KEY;
        break;
    case REG_KEY_64BIT:
        samDesired = KEY_WOW64_64KEY;
        break;
    case REG_KEY_DEFAULT:
        // Nothing to do
        break;
    }

    if (NULL != vpfnRegDeleteKeyExW)
    {
        er = vpfnRegDeleteKeyExW(hkRoot, wzSubKey, samDesired, 0);
        ExitOnWin32Error(er, hr, "Failed to delete registry key (ex).");
    }
    else
    {
        er = vpfnRegDeleteKeyW(hkRoot, wzSubKey);
        ExitOnWin32Error(er, hr, "Failed to delete registry key.");
    }

LExit:
    ReleaseRegKey(hkKey);
    ReleaseStr(pszEnumeratedSubKey);
    ReleaseStr(pszRecursiveSubKey);

    return hr;
}


/********************************************************************
 RegKeyEnum - enumerates a registry key.

*********************************************************************/
extern "C" HRESULT DAPI RegKeyEnum(
    __in HKEY hk,
    __in DWORD dwIndex,
    __deref_out_z LPWSTR* psczKey
    )
{
    HRESULT hr = S_OK;
    DWORD er = ERROR_SUCCESS;
    DWORD cch = 0;

    if (psczKey && *psczKey)
    {
        hr = StrMaxLength(*psczKey, reinterpret_cast<DWORD_PTR*>(&cch));
        ExitOnFailure(hr, "Failed to determine length of string.");
    }

    if (2 > cch)
    {
        cch = 2;

        hr = StrAlloc(psczKey, cch);
        ExitOnFailure(hr, "Failed to allocate string to minimum size.");
    }

    er = vpfnRegEnumKeyExW(hk, dwIndex, *psczKey, &cch, NULL, NULL, NULL, NULL);
    if (ERROR_MORE_DATA == er)
    {
        er = vpfnRegQueryInfoKeyW(hk, NULL, NULL, NULL, NULL, &cch, NULL, NULL, NULL, NULL, NULL, NULL);
        ExitOnWin32Error(er, hr, "Failed to get max size of value name under registry key.");

        ++cch; // add one because RegQueryInfoKeyW() returns the length of the subkeys without the null terminator.
        hr = StrAlloc(psczKey, cch);
        ExitOnFailure(hr, "Failed to allocate string bigger for enum registry value.");

        er = vpfnRegEnumKeyExW(hk, dwIndex, *psczKey, &cch, NULL, NULL, NULL, NULL);
    }
    else if (ERROR_NO_MORE_ITEMS == er)
    {
        ExitFunction1(hr = E_NOMOREITEMS);
    }
    ExitOnWin32Error(er, hr, "Failed to enum registry key.");

    // Always ensure the registry key name is null terminated.
    (*psczKey)[cch] = L'\0'; // note that cch will always be one less than the size of the buffer because that's how RegEnumKeyExW() works.

LExit:
    return hr;
}


/********************************************************************
 RegValueEnum - enumerates a registry value.

*********************************************************************/
HRESULT DAPI RegValueEnum(
    __in HKEY hk,
    __in DWORD dwIndex,
    __deref_out_z LPWSTR* psczName,
    __out_opt DWORD *pdwType
    )
{
    HRESULT hr = S_OK;
    DWORD er = ERROR_SUCCESS;
    DWORD cbValueName = 0;

    er = vpfnRegQueryInfoKeyW(hk, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &cbValueName, NULL, NULL, NULL);
    ExitOnWin32Error(er, hr, "Failed to get max size of value name under registry key.");

    // Add one for null terminator
    ++cbValueName;

    hr = StrAlloc(psczName, cbValueName);
    ExitOnFailure(hr, "Failed to allocate array for registry value name");

    er = vpfnRegEnumValueW(hk, dwIndex, *psczName, &cbValueName, NULL, pdwType, NULL, NULL);
    ExitOnWin32Error(er, hr, "Failed to enumerate registry value");

LExit:
    return hr;
}


/********************************************************************
 RegReadBinary - reads a registry key binary value.
 NOTE: caller is responsible for freeing *ppbBuffer
*********************************************************************/
HRESULT DAPI RegReadBinary(
    __in HKEY hk,
    __in_z_opt LPCWSTR wzName,
    __deref_out_bcount(*pcbBuffer) BYTE** ppbBuffer,
    __out DWORD *pcbBuffer
     )
{
    HRESULT hr = S_OK;
    LPBYTE pbBuffer = NULL;
    DWORD er = ERROR_SUCCESS;
    DWORD cb = 0;
    DWORD dwType = 0;

    vpfnRegQueryValueExW(hk, wzName, 0, &dwType, NULL, &cb);

    pbBuffer = static_cast<LPBYTE>(MemAlloc(cb, FALSE));
    ExitOnNull(pbBuffer, hr, E_OUTOFMEMORY, "Failed to allocate buffer for binary registry value.");

    er = vpfnRegQueryValueExW(hk, wzName, 0, &dwType, pbBuffer, &cb);
    ExitOnWin32Error(er, hr, "Failed to read registry key.");

    if (REG_BINARY == dwType)
    {
        *ppbBuffer = pbBuffer;
        pbBuffer = NULL;
        *pcbBuffer = cb;
    }
    else
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATATYPE);
        ExitOnRootFailure1(hr, "Error reading binary registry value due to unexpected data type: %u", dwType);
    }

LExit:
    ReleaseMem(pbBuffer);

    return hr;
}


/********************************************************************
 RegReadString - reads a registry key value as a string.

*********************************************************************/
extern "C" HRESULT DAPI RegReadString(
    __in HKEY hk,
    __in_z_opt LPCWSTR wzName,
    __deref_out_z LPWSTR* psczValue
    )
{
    HRESULT hr = S_OK;
    DWORD er = ERROR_SUCCESS;
    DWORD cch = 0;
    DWORD cb = 0;
    DWORD dwType = 0;
    LPWSTR sczExpand = NULL;

    if (psczValue && *psczValue)
    {
        hr = StrMaxLength(*psczValue, reinterpret_cast<DWORD_PTR*>(&cch));
        ExitOnFailure(hr, "Failed to determine length of string.");
    }

    if (2 > cch)
    {
        cch = 2;

        hr = StrAlloc(psczValue, cch);
        ExitOnFailure(hr, "Failed to allocate string to minimum size.");
    }

    cb = sizeof(WCHAR) * (cch - 1); // subtract one to ensure there will be a space at the end of the string for the null terminator.
    er = vpfnRegQueryValueExW(hk, wzName, 0, &dwType, reinterpret_cast<LPBYTE>(*psczValue), &cb);
    if (ERROR_MORE_DATA == er)
    {
        cch = cb / sizeof(WCHAR) + 1; // add one to ensure there will be space at the end for the null terminator
        hr = StrAlloc(psczValue, cch);
        ExitOnFailure(hr, "Failed to allocate string bigger for registry value.");

        er = vpfnRegQueryValueExW(hk, wzName, 0, &dwType, reinterpret_cast<LPBYTE>(*psczValue), &cb);
    }
    ExitOnWin32Error(er, hr, "Failed to read registry key.");

    if (REG_SZ == dwType || REG_EXPAND_SZ == dwType)
    {
        // Always ensure the registry value is null terminated.
        (*psczValue)[cch - 1] = L'\0';

        if (REG_EXPAND_SZ == dwType)
        {
            hr = StrAllocString(&sczExpand, *psczValue, 0);
            ExitOnFailure(hr, "Failed to copy registry value to expand.");

            hr = PathExpand(psczValue, sczExpand, PATH_EXPAND_ENVIRONMENT);
            ExitOnFailure1(hr, "Failed to expand registry value: %ls", *psczValue);
        }
    }
    else
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATATYPE);
        ExitOnRootFailure1(hr, "Error reading string registry value due to unexpected data type: %u", dwType);
    }

LExit:
    ReleaseStr(sczExpand);
    return hr;
}


/********************************************************************
 RegReadVersion - reads a registry key value as a version.

*********************************************************************/
extern "C" HRESULT DAPI RegReadVersion(
    __in HKEY hk,
    __in_z_opt LPCWSTR wzName,
    __out DWORD64* pdw64Version
    )
{
    HRESULT hr = S_OK;
    DWORD er = ERROR_SUCCESS;
    DWORD dwType = 0;
    DWORD cb = 0;
    LPWSTR sczVersion = NULL;

    cb = sizeof(DWORD64);
    er = vpfnRegQueryValueExW(hk, wzName, 0, &dwType, reinterpret_cast<LPBYTE>(*pdw64Version), &cb);
    if (REG_SZ == dwType || REG_EXPAND_SZ == dwType)
    {
        hr = RegReadString(hk, wzName, &sczVersion);
        ExitOnFailure(hr, "Failed to read registry version as string.");

        hr = FileVersionFromStringEx(sczVersion, 0, pdw64Version);
        ExitOnFailure(hr, "Failed to convert registry string to version.");
    }
    else if (REG_QWORD == dwType)
    {
        ExitOnWin32Error(er, hr, "Failed to read registry key.");
    }
    else // unexpected data type
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATATYPE);
        ExitOnRootFailure1(hr, "Error reading version registry value due to unexpected data type: %u", dwType);
    }

LExit:
    ReleaseStr(sczVersion);
    return hr;
}


/********************************************************************
 RegReadNumber - reads a registry key value as a number.

*********************************************************************/
extern "C" HRESULT DAPI RegReadNumber(
    __in HKEY hk,
    __in_z_opt LPCWSTR wzName,
    __out DWORD* pdwValue
    )
{
    HRESULT hr = S_OK;
    DWORD er = ERROR_SUCCESS;
    DWORD dwType = 0;
    DWORD cb = sizeof(DWORD);

    er = vpfnRegQueryValueExW(hk, wzName, NULL, &dwType, reinterpret_cast<LPBYTE>(pdwValue), &cb);
    ExitOnWin32Error(er, hr, "Failed to query registry key value.");

    if (REG_DWORD != dwType)
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATATYPE);
        ExitOnRootFailure1(hr, "Error reading version registry value due to unexpected data type: %u", dwType);
    }

LExit:
    return hr;
}


/********************************************************************
 RegWriteBinary - writes a registry key value as a binary.

*********************************************************************/
HRESULT DAPI RegWriteBinary(
    __in HKEY hk,
    __in_z_opt LPCWSTR wzName,
    __in_bcount(cBuffer) BYTE *pbBuffer,
    __in DWORD cbBuffer
    )
{
    HRESULT hr = S_OK;
    DWORD er = ERROR_SUCCESS;

    er = vpfnRegSetValueExW(hk, wzName, 0, REG_BINARY, pbBuffer, cbBuffer);
    ExitOnWin32Error1(er, hr, "Failed to write binary registry value with name: %ls", wzName);

LExit:
    return hr;
}


/********************************************************************
 RegWriteString - writes a registry key value as a string.

 Note: if wzValue is NULL the value will be removed.
*********************************************************************/
extern "C" HRESULT DAPI RegWriteString(
    __in HKEY hk,
    __in_z_opt LPCWSTR wzName,
    __in_z_opt LPWSTR wzValue
    )
{
    HRESULT hr = S_OK;
    DWORD er = ERROR_SUCCESS;
    DWORD cbValue = 0;

    if (wzValue)
    {
        hr = ::StringCbLengthW(wzValue, DWORD_MAX, reinterpret_cast<size_t*>(&cbValue));
        ExitOnFailure1(hr, "Failed to determine length of registry value: %ls", wzName);

        er = vpfnRegSetValueExW(hk, wzName, 0, REG_SZ, reinterpret_cast<PBYTE>(wzValue), cbValue);
        ExitOnWin32Error1(er, hr, "Failed to set registry value: %ls", wzName);
    }
    else
    {
        er = vpfnRegDeleteValueW(hk, wzName);
        if (ERROR_FILE_NOT_FOUND == er || ERROR_PATH_NOT_FOUND == er)
        {
            er = ERROR_SUCCESS;
        }
        ExitOnWin32Error1(er, hr, "Failed to delete registry value: %ls", wzName);
    }

LExit:
    return hr;
}


/********************************************************************
 RegWriteStringFormatted - writes a registry key value as a formatted string.

*********************************************************************/
extern "C" HRESULT DAPI RegWriteStringFormatted(
    __in HKEY hk,
    __in_z_opt LPCWSTR wzName,
    __in __format_string LPCWSTR szFormat,
    ...
    )
{
    HRESULT hr = S_OK;
    LPWSTR sczValue = NULL;
    va_list args;

    va_start(args, szFormat);
    hr = StrAllocFormattedArgs(&sczValue, szFormat, args);
    va_end(args);
    ExitOnFailure1(hr, "Failed to allocate %ls value.", wzName);

    hr = RegWriteString(hk, wzName, sczValue);

LExit:
    ReleaseStr(sczValue);
    return hr;
}


/********************************************************************
 RegWriteNumber - writes a registry key value as a number.

*********************************************************************/
extern "C" HRESULT DAPI RegWriteNumber(
    __in HKEY hk,
    __in_z_opt LPCWSTR wzName,
    __in DWORD dwValue
    )
{
    HRESULT hr = S_OK;
    DWORD er = ERROR_SUCCESS;

    er = vpfnRegSetValueExW(hk, wzName, 0, REG_DWORD, reinterpret_cast<PBYTE>(&dwValue), sizeof(dwValue));
    ExitOnWin32Error1(er, hr, "Failed to set %ls value.", wzName);

LExit:
    return hr;
}
