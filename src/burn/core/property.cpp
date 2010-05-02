//-------------------------------------------------------------------------------------------------
// <copyright file="property.cpp" company="Microsoft">
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
//    Property managing functions for Burn.
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"


// structs

typedef const struct _BUILT_IN_PROPERTY_DECLARATION {
    LPCWSTR wzProperty;
    PFN_INITIALIZEPROPERTY pfnInitialize;
    DWORD_PTR dwpInitializeData;
} BUILT_IN_PROPERTY_DECLARATION;


// constants

const DWORD GROW_PROPERTY_ARRAY = 3;

enum OS_INFO_PROPERTY
{
    OS_INFO_PROPERTY_NONE,
    OS_INFO_PROPERTY_VersionNT,
    OS_INFO_PROPERTY_VersionNT64,
    OS_INFO_PROPERTY_NTProductType,
    OS_INFO_PROPERTY_NTSuiteBackOffice,
    OS_INFO_PROPERTY_NTSuiteDataCenter,
    OS_INFO_PROPERTY_NTSuiteEnterprise,
    OS_INFO_PROPERTY_NTSuitePersonal,
    OS_INFO_PROPERTY_NTSuiteSmallBusiness,
    OS_INFO_PROPERTY_NTSuiteSmallBusinessRestricted,
    OS_INFO_PROPERTY_NTSuiteWebServer,
};


// internal function declarations

static HRESULT AddBuiltInProperty(
    __in BURN_PROPERTIES* pProperties,
    __in LPCWSTR wzProperty,
    __in PFN_INITIALIZEPROPERTY pfnInitialize,
    __in DWORD_PTR dwpInitializeData
    );
static HRESULT GetPropertyValue(
    __in BURN_PROPERTIES* pProperties,
    __in_z LPCWSTR wzProperty,
    __out BURN_VARIANT** ppValue
    );
static HRESULT FindPropertyIndexByName(
    __in BURN_PROPERTIES* pProperties,
    __in_z LPCWSTR wzProperty,
    __out DWORD* piProperty
    );
static HRESULT InsertProperty(
    __in BURN_PROPERTIES* pProperties,
    __in_z LPCWSTR wzProperty,
    __in DWORD iPosition
    );

static HRESULT AddBuiltInProperty(
    __in BURN_PROPERTIES* pProperties,
    __in LPCWSTR wzProperty,
    __in PFN_INITIALIZEPROPERTY pfnInitializer
    );
static HRESULT InitializePropertyOsInfo(
    __in DWORD_PTR dwpData,
    __inout BURN_VARIANT* pValue
    );
static HRESULT InitializePropertyVersionMsi(
    __in DWORD_PTR dwpData,
    __inout BURN_VARIANT* pValue
    );
static HRESULT InitializePropertyCsidlFolder(
    __in DWORD_PTR dwpData,
    __inout BURN_VARIANT* pValue
    );
//static HRESULT InitializePropertyKnownFolder(
//    __in DWORD_PTR dwpData,
//    __inout BURN_VARIANT* pValue
//    );
static HRESULT InitializePropertyWindowsVolumeFolder(
    __in DWORD_PTR dwpData,
    __inout BURN_VARIANT* pValue
    );
static HRESULT InitializePropertyTempFolder(
    __in DWORD_PTR dwpData,
    __inout BURN_VARIANT* pValue
    );


// function definitions

extern "C" HRESULT PropertyInitializeBuiltIn(
    __in BURN_PROPERTIES* pProperties
    )
{
    HRESULT hr = S_OK;

    const BUILT_IN_PROPERTY_DECLARATION vrgBuiltInProperties[] = {
        {L"AdminToolsFolder", InitializePropertyCsidlFolder, CSIDL_ADMINTOOLS},
        {L"AppDataFolder", InitializePropertyCsidlFolder, CSIDL_APPDATA},
        {L"CommonAppDataFolder", InitializePropertyCsidlFolder, CSIDL_COMMON_APPDATA},
        //{L"CommonFiles64Folder", InitializePropertyKnownFolder, (DWORD_PTR)&FOLDERID_ProgramFilesCommonX64},
        {L"CommonFilesFolder", InitializePropertyCsidlFolder, CSIDL_PROGRAM_FILES_COMMONX86},
        {L"DesktopFolder", InitializePropertyCsidlFolder, CSIDL_DESKTOP},
        {L"FavoritesFolder", InitializePropertyCsidlFolder, CSIDL_FAVORITES},
        {L"FontsFolder", InitializePropertyCsidlFolder, CSIDL_FONTS},
        {L"LocalAppDataFolder", InitializePropertyCsidlFolder, CSIDL_LOCAL_APPDATA},
        {L"MyPicturesFolder", InitializePropertyCsidlFolder, CSIDL_MYPICTURES},
        {L"NTProductType", InitializePropertyOsInfo, OS_INFO_PROPERTY_NTProductType},
        {L"NTSuiteBackOffice", InitializePropertyOsInfo, OS_INFO_PROPERTY_NTSuiteBackOffice},
        {L"NTSuiteDataCenter", InitializePropertyOsInfo, OS_INFO_PROPERTY_NTSuiteDataCenter},
        {L"NTSuiteEnterprise", InitializePropertyOsInfo, OS_INFO_PROPERTY_NTSuiteEnterprise},
        {L"NTSuitePersonal", InitializePropertyOsInfo, OS_INFO_PROPERTY_NTSuitePersonal},
        {L"NTSuiteSmallBusiness", InitializePropertyOsInfo, OS_INFO_PROPERTY_NTSuiteSmallBusiness},
        {L"NTSuiteSmallBusinessRestricted", InitializePropertyOsInfo, OS_INFO_PROPERTY_NTSuiteSmallBusinessRestricted},
        {L"NTSuiteWebServer", InitializePropertyOsInfo, OS_INFO_PROPERTY_NTSuiteWebServer},
        {L"PersonalFolder", InitializePropertyCsidlFolder, CSIDL_PERSONAL},
        //{L"ProgramFiles64Folder", InitializePropertyKnownFolder, (DWORD_PTR)&FOLDERID_ProgramFilesX64},
        {L"ProgramFilesFolder", InitializePropertyCsidlFolder, CSIDL_PROGRAM_FILESX86},
        {L"ProgramMenuFolder", InitializePropertyCsidlFolder, CSIDL_PROGRAMS},
        {L"SendToFolder", InitializePropertyCsidlFolder, CSIDL_SENDTO},
        {L"StartMenuFolder", InitializePropertyCsidlFolder, CSIDL_STARTMENU},
        {L"StartupFolder", InitializePropertyCsidlFolder, CSIDL_STARTUP},
        {L"SystemFolder", InitializePropertyCsidlFolder, CSIDL_SYSTEMX86},
        {L"TempFolder", InitializePropertyTempFolder, CSIDL_TEMPLATES},
        {L"TemplateFolder", InitializePropertyCsidlFolder, CSIDL_TEMPLATES},
        {L"VersionMsi", InitializePropertyVersionMsi, 0},
        {L"VersionNT", InitializePropertyOsInfo, OS_INFO_PROPERTY_VersionNT},
        {L"VersionNT64", InitializePropertyOsInfo, OS_INFO_PROPERTY_VersionNT64},
        {L"WindowsFolder", InitializePropertyCsidlFolder, CSIDL_WINDOWS},
        {L"WindowsVolume", InitializePropertyWindowsVolumeFolder, 0},
    };

    for (DWORD i = 0; i < countof(vrgBuiltInProperties); ++i)
    {
        BUILT_IN_PROPERTY_DECLARATION* pBuiltInProperty = &vrgBuiltInProperties[i];

        hr = AddBuiltInProperty(pProperties, pBuiltInProperty->wzProperty, pBuiltInProperty->pfnInitialize, pBuiltInProperty->dwpInitializeData);
        ExitOnFailure1(hr, "Failed to add built-in property: %S.", pBuiltInProperty->wzProperty);
    }

LExit:
    return hr;
}

extern "C" void PropertiesUninitialize(
    __in BURN_PROPERTIES* pProperties
    )
{
    if (pProperties->rgProperties)
    {
        for (DWORD i = 0; i < pProperties->cProperties; ++i)
        {
            BURN_PROPERTY* pProperty = &pProperties->rgProperties[i];
            if (pProperty)
            {
                ReleaseStr(pProperty->sczName);
                BVariantUninitialize(&pProperty->Value);
            }
        }
        MemFree(pProperties->rgProperties);
    }
}

extern "C" HRESULT PropertyGetNumeric(
    __in BURN_PROPERTIES* pProperties,
    __in_z LPCWSTR wzProperty,
    __out LONGLONG* pllValue
    )
{
    HRESULT hr = S_OK;
    BURN_VARIANT* pPropertyValue = NULL;

    hr = GetPropertyValue(pProperties, wzProperty, &pPropertyValue);
    ExitOnFailure1(hr, "Failed to get value of property: %S", wzProperty);

    hr = BVariantGetNumeric(pPropertyValue, pllValue);
    ExitOnFailure1(hr, "Failed to get value as numeric for property: %S", wzProperty);

LExit:
    return hr;
}

extern "C" HRESULT PropertyGetString(
    __in BURN_PROPERTIES* pProperties,
    __in_z LPCWSTR wzProperty,
    __out_z LPWSTR* psczValue
    )
{
    HRESULT hr = S_OK;
    BURN_VARIANT* pPropertyValue = NULL;

    hr = GetPropertyValue(pProperties, wzProperty, &pPropertyValue);
    ExitOnFailure1(hr, "Failed to get value of property: %S", wzProperty);

    hr = BVariantGetString(pPropertyValue, psczValue);
    ExitOnFailure1(hr, "Failed to get value as string for property: %S", wzProperty);

LExit:
    return hr;
}

extern "C" HRESULT PropertyGetVersion(
    __in BURN_PROPERTIES* pProperties,
    __in_z LPCWSTR wzProperty,
    __in DWORD64* pqwValue
    )
{
    HRESULT hr = S_OK;
    BURN_VARIANT* pPropertyValue = NULL;

    hr = GetPropertyValue(pProperties, wzProperty, &pPropertyValue);
    ExitOnFailure1(hr, "Failed to get value of property: %S", wzProperty);

    hr = BVariantGetVersion(pPropertyValue, pqwValue);
    ExitOnFailure1(hr, "Failed to get value as version for property: %S", wzProperty);

LExit:
    return hr;
}

extern "C" HRESULT PropertyGetVariant(
    __in BURN_PROPERTIES* pProperties,
    __in_z LPCWSTR wzProperty,
    __in BURN_VARIANT* pValue
    )
{
    HRESULT hr = S_OK;
    BURN_VARIANT* pPropertyValue = NULL;

    hr = GetPropertyValue(pProperties, wzProperty, &pPropertyValue);
    ExitOnFailure1(hr, "Failed to get value of property: %S", wzProperty);

    hr = BVariantCopy(pPropertyValue, pValue);
    ExitOnFailure1(hr, "Failed to copy value of property: %S", wzProperty);

LExit:
    return hr;
}

extern "C" HRESULT PropertyGetFormatted(
    __in BURN_PROPERTIES* pProperties,
    __in_z LPCWSTR wzProperty,
    __out_z LPWSTR* psczValue
    )
{
    HRESULT hr = S_OK;
    DWORD iProperty = 0;
    BURN_PROPERTY* pProperty = NULL;

    hr = FindPropertyIndexByName(pProperties, wzProperty, &iProperty);
    ExitOnFailure1(hr, "Failed to find property: %S", wzProperty);

    if (S_FALSE == hr)
    {
        ExitFunction1(hr = E_INVALIDARG);
    }

    pProperty = &pProperties->rgProperties[iProperty];
    if (BURN_VARIANT_TYPE_STRING == pProperty->Value.Type)
    {
        hr = PropertyFormatString(pProperties, pProperty->Value.sczValue, psczValue);
        ExitOnFailure2(hr, "Failed to format value '%S' of property: %S", pProperty->Value.sczValue, wzProperty);
    }
    else
    {
        hr = BVariantGetString(&pProperty->Value, psczValue);
        ExitOnFailure1(hr, "Failed to get value as string for property: %S", wzProperty);
    }

LExit:
    return hr;
}

extern "C" HRESULT PropertySetNumeric(
    __in BURN_PROPERTIES* pProperties,
    __in_z LPCWSTR wzProperty,
    __in LONGLONG llValue
    )
{
    BURN_VARIANT variant = { };

    variant.llValue = llValue;
    variant.Type = BURN_VARIANT_TYPE_NUMERIC;

    return PropertySetVariant(pProperties, wzProperty, &variant);
}

extern "C" HRESULT PropertySetString(
    __in BURN_PROPERTIES* pProperties,
    __in_z LPCWSTR wzProperty,
    __in_z_opt LPCWSTR wzValue
    )
{
    BURN_VARIANT variant = { };

    variant.sczValue = (LPWSTR)wzValue;
    variant.Type = BURN_VARIANT_TYPE_STRING;

    return PropertySetVariant(pProperties, wzProperty, &variant);
}

extern "C" HRESULT PropertySetVersion(
    __in BURN_PROPERTIES* pProperties,
    __in_z LPCWSTR wzProperty,
    __in DWORD64 qwValue
    )
{
    BURN_VARIANT variant = { };

    variant.qwValue = qwValue;
    variant.Type = BURN_VARIANT_TYPE_VERSION;

    return PropertySetVariant(pProperties, wzProperty, &variant);
}

extern "C" HRESULT PropertySetVariant(
    __in BURN_PROPERTIES* pProperties,
    __in_z LPCWSTR wzProperty,
    __in BURN_VARIANT* pVariant
    )
{
    HRESULT hr = S_OK;
    DWORD iProperty = 0;

    hr = FindPropertyIndexByName(pProperties, wzProperty, &iProperty);
    ExitOnFailure1(hr, "Failed to find property value '%S'.", wzProperty);

    // insert element if not found
    if (S_FALSE == hr)
    {
        hr = InsertProperty(pProperties, wzProperty, iProperty);
        ExitOnFailure1(hr, "Failed to insert property '%S'.", wzProperty);
    }
    else if (pProperties->rgProperties[iProperty].fBuiltIn)
    {
        hr = E_INVALIDARG;
        ExitOnRootFailure1(hr, "Attempt to set built-in property value: %S", wzProperty);
    }

    // update property value
    hr = BVariantCopy(pVariant, &pProperties->rgProperties[iProperty].Value);
    ExitOnFailure1(hr, "Failed to set value of property: %S", wzProperty);

LExit:
    return hr;
}

extern "C" HRESULT PropertyFormatString(
    __in BURN_PROPERTIES* pProperties,
    __in_z LPCWSTR wzIn,
    __out_z LPWSTR* ppwzOut
    )
{
    HRESULT hr = S_OK;
    DWORD er = ERROR_SUCCESS;
    LPWSTR pwzUnformatted = NULL;
    LPWSTR pwzFormat = NULL;
    LPCWSTR wzRead = NULL;
    LPCWSTR wzOpen = NULL;
    LPCWSTR wzClose = NULL;
    LPWSTR pwz = NULL;
    LPWSTR* rgProperties = NULL;
    DWORD cProperties = 0;
    DWORD cch = 0;
    MSIHANDLE hRecord = NULL;

    // allocate buffer for format string
    hr = StrAlloc(&pwzFormat, lstrlenW(wzIn) + 1);
    ExitOnFailure(hr, "Failed to allocate buffer for format string.");

    // read out properties from the unformatted string and build a format string
    wzRead = wzIn;
    for (;;)
    {
        // scan for opening '['
        wzOpen = wcschr(wzRead, L'[');
        if (!wzOpen)
        {
            // end reached, append the remainder of the string and end loop
            hr = StrAllocConcat(&pwzFormat, wzRead, 0);
            ExitOnFailure(hr, "Failed to append string.");
            break;
        }

        // scan for closing ']'
        wzClose = wcschr(wzOpen + 1, L']');
        if (!wzClose)
        {
            // end reached, treat unterminated expander as literal
            hr = StrAllocConcat(&pwzFormat, wzRead, 0);
            ExitOnFailure(hr, "Failed to append string.");
            break;
        }
        cch = wzClose - wzOpen - 1;

        if (0 == cch)
        {
            // blank, copy all text including the terminator
            hr = StrAllocConcat(&pwzFormat, wzRead, (DWORD_PTR)(wzClose - wzRead) + 1);
            ExitOnFailure(hr, "Failed to append string.");
        }
        else
        {
            // append text preceding expander
            if (wzOpen > wzRead)
            {
                hr = StrAllocConcat(&pwzFormat, wzRead, (DWORD_PTR)(wzOpen - wzRead));
                ExitOnFailure(hr, "Failed to append string.");
            }

            // get property name
            hr = StrAllocString(&pwz, wzOpen + 1, cch);
            ExitOnFailure(hr, "Failed to get property name.");

            // allocate space in property array
            if (rgProperties)
            {
                LPVOID pv = MemReAlloc(rgProperties, sizeof(LPWSTR) * (cProperties + 1), TRUE);
                ExitOnNull(pv, hr, E_OUTOFMEMORY, "Failed to reallocate property array.");
                rgProperties = (LPWSTR*)pv;
            }
            else
            {
                rgProperties = (LPWSTR*)MemAlloc(sizeof(LPWSTR) * (cProperties + 1), TRUE);
                ExitOnNull(rgProperties, hr, E_OUTOFMEMORY, "Failed to allocate property array.");
            }

            // set property value
            if (2 <= cch && L'\\' == wzOpen[1])
            {
                // escape sequence, copy character
                hr = StrAllocString(&rgProperties[cProperties], &wzOpen[2], 1);
            }
            else
            {
                // get formatted property value
                hr = PropertyGetFormatted(pProperties, pwz, &rgProperties[cProperties]);
                if (E_INVALIDARG == hr) // property not found
                {
                    hr = StrAllocString(&rgProperties[cProperties], L"", 0);
                }
            }
            ExitOnFailure(hr, "Failed to set property value.");
            ++cProperties;

            // append placeholder to format string
            hr = StrAllocFormatted(&pwz, L"[%d]", cProperties);
            ExitOnFailure(hr, "Failed to format placeholder string.");

            hr = StrAllocConcat(&pwzFormat, pwz, 0);
            ExitOnFailure(hr, "Failed to append placeholder.");
        }

        // update read pointer
        wzRead = wzClose + 1;
    }

    // create record
    hRecord = ::MsiCreateRecord(cProperties);
    ExitOnNull(hRecord, hr, E_OUTOFMEMORY, "Failed to allocate record.");

    // set format string
    er = ::MsiRecordSetStringW(hRecord, 0, pwzFormat);
    ExitOnWin32Error(er, hr, "Failed to set record format string.");

    // copy record fields
    for (DWORD i = 0; i < cProperties; ++i)
    {
        if (*rgProperties[i]) // not setting if blank
        {
            er = ::MsiRecordSetStringW(hRecord, i + 1, rgProperties[i]);
            ExitOnWin32Error(er, hr, "Failed to set record string.");
        }
    }

    // format record
    cch = 0;
    er = ::MsiFormatRecordW(NULL, hRecord, L"", &cch);
    if (ERROR_MORE_DATA == er || ERROR_SUCCESS == er)
    {
        hr = StrAlloc(&pwz, ++cch);
    }
    else
    {
        hr = HRESULT_FROM_WIN32(er);
    }
    ExitOnFailure(hr, "Failed to allocate string.");

    er = ::MsiFormatRecordW(NULL, hRecord, pwz, &cch);
    ExitOnWin32Error(er, hr, "Failed to format record.");

    // return value
    hr = StrAllocString(ppwzOut, pwz, 0);
    ExitOnFailure(hr, "Failed to copy string.");

LExit:
    if (rgProperties)
    {
        for (DWORD i = 0; i < cProperties; ++i)
        {
            ReleaseStr(rgProperties[i]);
        }
        MemFree(rgProperties);
    }

    if (hRecord)
    {
        ::MsiCloseHandle(hRecord);
    }

    ReleaseStr(pwzUnformatted);
    ReleaseStr(pwzFormat);
    ReleaseStr(pwz);

    return hr;
}

extern "C" HRESULT PropertyEscapeString(
    __in_z LPCWSTR wzIn,
    __out_z LPWSTR* ppwzOut
    )
{
    HRESULT hr = S_OK;
    LPCWSTR wzRead = NULL;
    LPWSTR pwzEscaped = NULL;
    LPWSTR pwz = NULL;
    SIZE_T i = 0;

    // allocate buffer for escaped string
    hr = StrAlloc(&pwzEscaped, lstrlenW(wzIn) + 1);
    ExitOnFailure(hr, "Failed to allocate buffer for escaped string.");

    // read trough string and move characters, inserting escapes as needed
    wzRead = wzIn;
    for (;;)
    {
        // find next character needing escaping
        i = wcscspn(wzRead, L"[]{}");

        // copy skipped characters
        if (0 < i)
        {
            hr = StrAllocConcat(&pwzEscaped, wzRead, i);
            ExitOnFailure(hr, "Failed to append characters.");
        }

        if (L'\0' == wzRead[i])
        {
            break; // end reached
        }

        // escape character
        hr = StrAllocFormatted(&pwz, L"[\\%c]", wzRead[i]);
        ExitOnFailure(hr, "Failed to format escape sequence.");

        hr = StrAllocConcat(&pwzEscaped, pwz, 0);
        ExitOnFailure(hr, "Failed to append escape sequence.");

        // update read pointer
        wzRead += i + 1;
    }

    // return value
    hr = StrAllocString(ppwzOut, pwzEscaped, 0);
    ExitOnFailure(hr, "Failed to copy string.");

LExit:
    ReleaseStr(pwzEscaped);
    ReleaseStr(pwz);
    return hr;
}


extern "C" HRESULT PropertySerialize(
    __in BURN_PROPERTIES* pProperties,
    __inout BYTE** ppbBuffer,
    __inout SIZE_T* piBuffer
    )
{
    HRESULT hr = S_OK;

    // write property count
    hr = BuffWriteNumber(ppbBuffer, piBuffer, pProperties->cProperties);
    ExitOnFailure(hr, "Failed to write property count.");

    // write properties
    for (DWORD i = 0; i < pProperties->cProperties; ++i)
    {
        BURN_PROPERTY* pProperty = &pProperties->rgProperties[i];

        // write property built-in flag
        hr = BuffWriteNumber(ppbBuffer, piBuffer, (DWORD)pProperty->fBuiltIn);
        ExitOnFailure(hr, "Failed to write property built-in flag.");

        if (pProperty->fBuiltIn)
        {
            continue;// if propety is built-in, don't serialized
        }

        // write property name
        hr = BuffWriteString(ppbBuffer, piBuffer, pProperty->sczName);
        ExitOnFailure(hr, "Failed to write property name.");

        // write property value
        hr = BuffWriteNumber(ppbBuffer, piBuffer, (DWORD)pProperty->Value.Type);
        ExitOnFailure(hr, "Failed to write property value type.");

        switch (pProperty->Value.Type)
        {
        case BURN_VARIANT_TYPE_NUMERIC: __fallthrough;
        case BURN_VARIANT_TYPE_VERSION:
            hr = BuffWriteNumber64(ppbBuffer, piBuffer, pProperty->Value.qwValue);
            ExitOnFailure(hr, "Failed to write property value as number.");
            break;
        case BURN_VARIANT_TYPE_STRING:
            hr = BuffWriteString(ppbBuffer, piBuffer, pProperty->Value.sczValue);
            ExitOnFailure(hr, "Failed to write property value as string.");
            break;
        default:
            hr = E_INVALIDARG;
            ExitOnFailure(hr, "Unsupported property type.");
        }
    }

LExit:
    return hr;
}


extern "C" HRESULT PropertySaveToFile(
    __in BURN_PROPERTIES* pProperties,
    __in_z LPCWSTR wzPersistPath
    )
{
    HRESULT hr = S_OK;
    BYTE* pbProperties = NULL;
    SIZE_T cbProperties = 0;

    hr = PropertySerialize(pProperties, &pbProperties, &cbProperties);
    ExitOnFailure(hr, "Failed to serialize properties.");

    hr = FileWrite(wzPersistPath, FILE_ATTRIBUTE_NORMAL, pbProperties, cbProperties, NULL);
    ExitOnFailure1(hr, "Failed to write properties to file: %S", wzPersistPath);

LExit:
    ReleaseBuffer(pbProperties);
    return hr;
}


extern "C" HRESULT PropertyDeserialize(
    __in BURN_PROPERTIES* pProperties,
    __in_bcount(cbBuffer) BYTE* pbBuffer,
    __in SIZE_T cbBuffer,
    __inout SIZE_T* piBuffer
    )
{
    HRESULT hr = S_OK;
    DWORD cProperties = 0;
    LPWSTR sczName = NULL;
    DWORD fBuiltIn = 0;
    BURN_VARIANT value = { };

    // read property count
    hr = BuffReadNumber(pbBuffer, cbBuffer, piBuffer, &cProperties);
    ExitOnFailure(hr, "Failed to read property count.");

    // read properties
    for (DWORD i = 0; i < cProperties; ++i)
    {
        // read property built-in flag
        hr = BuffReadNumber(pbBuffer, cbBuffer, piBuffer, &fBuiltIn);
        ExitOnFailure(hr, "Failed to read property built-in flag.");

        if (fBuiltIn)
        {
            continue; // if propety is built-in, it is not serialized
        }

        // read property name
        hr = BuffReadString(pbBuffer, cbBuffer, piBuffer, &sczName);
        ExitOnFailure(hr, "Failed to read property name.");

        // read property value type
        hr = BuffReadNumber(pbBuffer, cbBuffer, piBuffer, (DWORD*)&value.Type);
        ExitOnFailure(hr, "Failed to read property value type.");

        // read property value
        switch (value.Type)
        {
        case BURN_VARIANT_TYPE_NUMERIC: __fallthrough;
        case BURN_VARIANT_TYPE_VERSION:
            hr = BuffReadNumber64(pbBuffer, cbBuffer, piBuffer, &value.qwValue);
            ExitOnFailure(hr, "Failed to read property value as number.");
            break;
        case BURN_VARIANT_TYPE_STRING:
            hr = BuffReadString(pbBuffer, cbBuffer, piBuffer, &value.sczValue);
            ExitOnFailure(hr, "Failed to read property value as string.");
            break;
        default:
            hr = E_INVALIDARG;
            ExitOnFailure(hr, "Unsupported property type.");
        }

        // set property
        hr = PropertySetVariant(pProperties, sczName, &value);
        ExitOnFailure(hr, "Failed to set property.");

        // clean up
        BVariantUninitialize(&value);
    }

LExit:
    ReleaseStr(sczName);
    BVariantUninitialize(&value);
    return hr;
}


extern "C" HRESULT PropertyLoadFromFile(
    __in BURN_PROPERTIES* pProperties,
    __in_z LPCWSTR wzPersistPath
    )
{
    HRESULT hr = S_OK;
    BYTE* pbProperties = NULL;
    DWORD cbProperties = 0;
    SIZE_T iProperties = 0;

    hr = FileRead(&pbProperties, &cbProperties, wzPersistPath);
    ExitOnFailure1(hr, "Failed to read properties from file: %S", wzPersistPath);

    hr = PropertyDeserialize(pProperties, pbProperties, cbProperties, &iProperties);
    ExitOnFailure(hr, "Failed to serialize properties.");

LExit:
    ReleaseMem(pbProperties);
    return hr;
}


// internal function definitions

static HRESULT AddBuiltInProperty(
    __in BURN_PROPERTIES* pProperties,
    __in LPCWSTR wzProperty,
    __in PFN_INITIALIZEPROPERTY pfnInitialize,
    __in DWORD_PTR dwpInitializeData
    )
{
    HRESULT hr = S_OK;
    DWORD iProperty = 0;
    BURN_PROPERTY* pProperty = NULL;

    hr = FindPropertyIndexByName(pProperties, wzProperty, &iProperty);
    ExitOnFailure(hr, "Failed to find property value.");

    // insert element if not found
    if (S_FALSE == hr)
    {
        hr = InsertProperty(pProperties, wzProperty, iProperty);
        ExitOnFailure(hr, "Failed to insert property.");
    }

    // set property values
    pProperty = &pProperties->rgProperties[iProperty];
    pProperty->fBuiltIn = TRUE;
    pProperty->pfnInitialize = pfnInitialize;
    pProperty->dwpInitializeData = dwpInitializeData;

LExit:
    return hr;
}

static HRESULT GetPropertyValue(
    __in BURN_PROPERTIES* pProperties,
    __in_z LPCWSTR wzProperty,
    __out BURN_VARIANT** ppValue
    )
{
    HRESULT hr = S_OK;
    DWORD iProperty = 0;
    BURN_PROPERTY* pProperty = NULL;

    hr = FindPropertyIndexByName(pProperties, wzProperty, &iProperty);
    ExitOnFailure(hr, "Failed to find property value.");

    if (S_FALSE == hr)
    {
        ExitFunction1(hr = E_INVALIDARG);
    }

    pProperty = &pProperties->rgProperties[iProperty];

    // initialize built-in property
    if (BURN_VARIANT_TYPE_NONE == pProperty->Value.Type && pProperty->fBuiltIn)
    {
        hr = pProperty->pfnInitialize(pProperty->dwpInitializeData, &pProperty->Value);
        ExitOnFailure(hr, "Failed to initialize built-in property value.");
    }

    *ppValue = &pProperty->Value;

LExit:
    return hr;
}

static HRESULT FindPropertyIndexByName(
    __in BURN_PROPERTIES* pProperties,
    __in_z LPCWSTR wzProperty,
    __out DWORD* piProperty
    )
{
    HRESULT hr = S_OK;
    DWORD iRangeFirst = 0;
    DWORD cRangeLength = pProperties->cProperties;
    int cchProperty = lstrlenW(wzProperty);

    while (cRangeLength)
    {
        // get property in middle of range
        DWORD iPosition = (cRangeLength / 2);
        BURN_PROPERTY* pProperty = &pProperties->rgProperties[iRangeFirst + iPosition];

        switch (::CompareStringW(LOCALE_INVARIANT, SORT_STRINGSORT, wzProperty, cchProperty, pProperty->sczName, -1))
        {
        case CSTR_LESS_THAN:
            // restrict range to elements before the current
            cRangeLength = iPosition;
            break;
        case CSTR_EQUAL:
            // property found
            *piProperty = iRangeFirst + iPosition;
            ExitFunction1(hr = S_OK);
        case CSTR_GREATER_THAN:
            // restrict range to elements after the current
            iRangeFirst += iPosition + 1;
            cRangeLength -= iPosition + 1;
            break;
        default:
            ExitWithLastError(hr, "Failed to compare strings.");
        }
    }

    *piProperty = iRangeFirst;
    hr = S_FALSE; // property not found

LExit:
    return hr;
}

static HRESULT InsertProperty(
    __in BURN_PROPERTIES* pProperties,
    __in_z LPCWSTR wzProperty,
    __in DWORD iPosition
    )
{
    HRESULT hr = S_OK;

    // ensure there is room in the property array
    if (pProperties->cProperties == pProperties->dwMaxProperties)
    {
        pProperties->dwMaxProperties += GROW_PROPERTY_ARRAY;

        if (pProperties->rgProperties)
        {
            LPVOID pv = MemReAlloc(pProperties->rgProperties, sizeof(BURN_PROPERTY) * pProperties->dwMaxProperties, FALSE);
            ExitOnNull(pv, hr, E_OUTOFMEMORY, "Failed to allocate room for more properties.");

            pProperties->rgProperties = (BURN_PROPERTY*)pv;
            memset(&pProperties->rgProperties[pProperties->cProperties], 0, sizeof(BURN_PROPERTY) * (pProperties->dwMaxProperties - pProperties->cProperties));
        }
        else
        {
            pProperties->rgProperties = (BURN_PROPERTY*)MemAlloc(sizeof(BURN_PROPERTY) * pProperties->dwMaxProperties, TRUE);
            ExitOnNull(pProperties->rgProperties, hr, E_OUTOFMEMORY, "Failed to allocate room for properties.");
        }
    }

    // move properties
    if (0 < pProperties->cProperties - iPosition)
    {
        memmove(&pProperties->rgProperties[iPosition + 1], &pProperties->rgProperties[iPosition], sizeof(BURN_PROPERTY) * (pProperties->cProperties - iPosition));
        memset(&pProperties->rgProperties[iPosition], 0, sizeof(BURN_PROPERTY));
    }

    ++pProperties->cProperties;

    // allocate name
    hr = StrAllocString(&pProperties->rgProperties[iPosition].sczName, wzProperty, 0);
    ExitOnFailure(hr, "Failed to copy property name.");

LExit:
    return hr;
}


static HRESULT InitializePropertyOsInfo(
    __in DWORD_PTR dwpData,
    __inout BURN_VARIANT* pValue
    )
{
    HRESULT hr = S_OK;
    OSVERSIONINFOEXW ovix = { };
    BURN_VARIANT value = { };

    ovix.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXW);
    if (!::GetVersionExW((LPOSVERSIONINFOW)&ovix))
    {
        ExitWithLastError(hr, "Failed to get OS info.");
    }

    switch ((OS_INFO_PROPERTY)dwpData)
    {
    case OS_INFO_PROPERTY_VersionNT:
        value.qwValue = MAKEQWORDVERSION(ovix.dwMajorVersion, ovix.dwMinorVersion, 0, 0);
        value.Type = BURN_VARIANT_TYPE_VERSION;
        break;
    case OS_INFO_PROPERTY_VersionNT64:
        {
#if !defined(_WIN64)
            BOOL fIsWow64 = FALSE;
            typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS)(HANDLE, PBOOL);
            LPFN_ISWOW64PROCESS pfnIsWow64Process = (LPFN_ISWOW64PROCESS)::GetProcAddress(::GetModuleHandleW(L"kernel32"), "IsWow64Process");

            if (pfnIsWow64Process)
            {
                if (!pfnIsWow64Process(::GetCurrentProcess(), &fIsWow64))
                {
                    ExitWithLastError(hr, "Failed to check WOW64 process.");
                }
            }
            if (fIsWow64)
#endif
            {
                value.qwValue = MAKEQWORDVERSION(ovix.dwMajorVersion, ovix.dwMinorVersion, 0, 0);
                value.Type = BURN_VARIANT_TYPE_VERSION;
            }
        }
        break;
    case OS_INFO_PROPERTY_NTProductType:
        value.llValue = ovix.wProductType;
        value.Type = BURN_VARIANT_TYPE_NUMERIC;
        break;
    case OS_INFO_PROPERTY_NTSuiteBackOffice:
        value.llValue = VER_SUITE_BACKOFFICE == ovix.wSuiteMask ? 1 : 0;
        value.Type = BURN_VARIANT_TYPE_NUMERIC;
        break;
    case OS_INFO_PROPERTY_NTSuiteDataCenter:
        value.llValue = VER_SUITE_DATACENTER == ovix.wSuiteMask ? 1 : 0;
        value.Type = BURN_VARIANT_TYPE_NUMERIC;
        break;
    case OS_INFO_PROPERTY_NTSuiteEnterprise:
        value.llValue = VER_SUITE_ENTERPRISE == ovix.wSuiteMask ? 1 : 0;
        value.Type = BURN_VARIANT_TYPE_NUMERIC;
        break;
    case OS_INFO_PROPERTY_NTSuitePersonal:
        value.llValue = VER_SUITE_PERSONAL == ovix.wSuiteMask ? 1 : 0;
        value.Type = BURN_VARIANT_TYPE_NUMERIC;
        break;
    case OS_INFO_PROPERTY_NTSuiteSmallBusiness:
        value.llValue = VER_SUITE_SMALLBUSINESS == ovix.wSuiteMask ? 1 : 0;
        value.Type = BURN_VARIANT_TYPE_NUMERIC;
        break;
    case OS_INFO_PROPERTY_NTSuiteSmallBusinessRestricted:
        value.llValue = VER_SUITE_SMALLBUSINESS_RESTRICTED == ovix.wSuiteMask ? 1 : 0;
        value.Type = BURN_VARIANT_TYPE_NUMERIC;
        break;
    case OS_INFO_PROPERTY_NTSuiteWebServer:
        value.llValue = VER_SUITE_BLADE == ovix.wSuiteMask ? 1 : 0;
        value.Type = BURN_VARIANT_TYPE_NUMERIC;
        break;
    }

    hr = BVariantCopy(&value, pValue);
    ExitOnFailure(hr, "Failed to set variant value.");

LExit:
    return hr;
}

static HRESULT InitializePropertyVersionMsi(
    __in DWORD_PTR dwpData,
    __inout BURN_VARIANT* pValue
    )
{
    UNREFERENCED_PARAMETER(dwpData);

    HRESULT hr = S_OK;
    DLLGETVERSIONPROC pfnMsiDllGetVersion = NULL;
    DLLVERSIONINFO msiVersionInfo = { };

    // get DllGetVersion proc address
    pfnMsiDllGetVersion = (DLLGETVERSIONPROC)::GetProcAddress(::GetModuleHandleW(L"msi"), "DllGetVersion");
    ExitOnNullWithLastError(pfnMsiDllGetVersion, hr, "Failed to find DllGetVersion entry point in msi.dll.");

    // get msi.dll version info
    msiVersionInfo.cbSize = sizeof(DLLVERSIONINFO);
    hr = pfnMsiDllGetVersion(&msiVersionInfo);
    ExitOnFailure(hr, "Failed to get msi.dll version info.");

    hr = BVariantSetVersion(pValue, MAKEQWORDVERSION(msiVersionInfo.dwMajorVersion, msiVersionInfo.dwMinorVersion, 0, 0));
    ExitOnFailure(hr, "Failed to set variant value.");

LExit:
    return hr;
}

static HRESULT InitializePropertyCsidlFolder(
    __in DWORD_PTR dwpData,
    __inout BURN_VARIANT* pValue
    )
{
    HRESULT hr = S_OK;
    int nFolder = (int)dwpData;
    WCHAR wzPath[MAX_PATH] = { };

    // get folder path
    hr = ::SHGetFolderPathW(NULL, nFolder, NULL, 0, wzPath);
    ExitOnRootFailure(hr, "Failed to get known folder.");

    // set value
    hr = BVariantSetString(pValue, wzPath, 0);
    ExitOnFailure(hr, "Failed to set variant value.");

LExit:
    return hr;
}

//static HRESULT InitializePropertyKnownFolder(
//    __in DWORD_PTR dwpData,
//    __inout BURN_VARIANT* pValue
//    )
//{
//    HRESULT hr = S_OK;
//    REFKNOWNFOLDERID rfid = *(KNOWNFOLDERID*)dwpData;
//    typedef HRESULT (*PFN_SHGetKnownFolderPath)(REFKNOWNFOLDERID rfid, DWORD dwFlags, HANDLE hToken, PWSTR *ppszPath);
//    PFN_SHGetKnownFolderPath pfnSHGetKnownFolderPath = NULL;
//    LPWSTR sczPath = NULL;
//
//    // get DllGetVersion proc address
//    pfnSHGetKnownFolderPath = (PFN_SHGetKnownFolderPath)::GetProcAddress(::GetModuleHandleW(L"shell32"), "SHGetKnownFolderPath");
//    if (pfnSHGetKnownFolderPath)
//    {
//        hr = pfnSHGetKnownFolderPath(rfid, 0, NULL, &sczPath);
//        ExitOnRootFailure(hr, "Failed to get known folder.");
//    }
//
//    // set value
//    hr = BVariantSetString(pValue, sczPath ? sczPath : L"", 0);
//    ExitOnFailure(hr, "Failed to set variant value.");
//
//LExit:
//    if (sczPath)
//    {
//        ::CoTaskMemFree(sczPath);
//    }
//
//    return hr;
//}

static HRESULT InitializePropertyTempFolder(
    __in DWORD_PTR dwpData,
    __inout BURN_VARIANT* pValue
    )
{
    UNREFERENCED_PARAMETER(dwpData);

    HRESULT hr = S_OK;
    WCHAR wzPath[MAX_PATH] = { };

    // get volume path name
    if (!::GetTempPathW(MAX_PATH, wzPath))
    {
        ExitWithLastError(hr, "Failed to get temp path.");
    }

    // set value
    hr = BVariantSetString(pValue, wzPath, 0);
    ExitOnFailure(hr, "Failed to set variant value.");

LExit:
    return hr;
}

static HRESULT InitializePropertyWindowsVolumeFolder(
    __in DWORD_PTR dwpData,
    __inout BURN_VARIANT* pValue
    )
{
    UNREFERENCED_PARAMETER(dwpData);

    HRESULT hr = S_OK;
    WCHAR wzWindowsPath[MAX_PATH] = { };
    WCHAR wzVolumePath[MAX_PATH] = { };

    // get windows directory
    hr = ::GetWindowsDirectoryW(wzWindowsPath, countof(wzWindowsPath));
    ExitOnRootFailure(hr, "Failed to get windows directory.");

    // get volume path name
    if (!::GetVolumePathNameW(wzWindowsPath, wzVolumePath, MAX_PATH))
    {
        ExitWithLastError(hr, "Failed to get volume path name.");
    }

    // set value
    hr = BVariantSetString(pValue, wzVolumePath, 0);
    ExitOnFailure(hr, "Failed to set variant value.");

LExit:
    return hr;
}
