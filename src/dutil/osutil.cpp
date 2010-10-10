//-------------------------------------------------------------------------------------------------
// <copyright file="aclutil.cpp" company="Microsoft">
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
//    Access Control List helper funtions.
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"

OS_VERSION vOsVersion = OS_VERSION_UNKNOWN;
DWORD vdwOsServicePack = 0;

/********************************************************************
 OsGetVersion

********************************************************************/
extern "C" void DAPI OsGetVersion(
    __out OS_VERSION* pVersion,
    __out DWORD* pdwServicePack
    )
{
    OSVERSIONINFOEXW ovi = { };

    if (OS_VERSION_UNKNOWN == vOsVersion)
    {
        ovi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXW);
        ::GetVersionExW(reinterpret_cast<OSVERSIONINFOW*>(&ovi)); // only fails if version info size is set incorrectly.

        vdwOsServicePack = static_cast<DWORD>(ovi.wServicePackMajor) << 16 | ovi.wServicePackMinor;
        if (4 == ovi.dwMajorVersion)
        {
            vOsVersion = OS_VERSION_WINNT;
        }
        else if (5 == ovi.dwMajorVersion)
        {
            if (0 == ovi.dwMinorVersion)
            {
                vOsVersion = OS_VERSION_WIN2000;
            }
            else if (1 == ovi.dwMinorVersion)
            {
                vOsVersion = OS_VERSION_WINXP;
            }
            else if (2 == ovi.dwMinorVersion)
            {
                vOsVersion = OS_VERSION_WIN2003;
            }
            else
            {
                vOsVersion = OS_VERSION_FUTURE;
            }
        }
        else if (6 == ovi.dwMajorVersion)
        {
            if (0 == ovi.dwMinorVersion)
            {
                vOsVersion = (VER_NT_WORKSTATION == ovi.wProductType) ? OS_VERSION_VISTA : OS_VERSION_WIN2008;
            }
            else if (1 == ovi.dwMinorVersion)
            {
                vOsVersion = (VER_NT_WORKSTATION == ovi.wProductType) ? OS_VERSION_WIN7 : OS_VERSION_WIN2008_R2;
            }
            else
            {
                vOsVersion = OS_VERSION_FUTURE;
            }
        }
        else
        {
            vOsVersion = OS_VERSION_FUTURE;
        }
    }

    *pVersion = vOsVersion;
    *pdwServicePack = vdwOsServicePack;
}

extern "C" HRESULT OsIsRunningPrivileged(
    __out BOOL* pfPrivileged
    )
{
    HRESULT hr = S_OK;
    UINT er = ERROR_SUCCESS;
    HANDLE hToken = NULL;
    TOKEN_ELEVATION_TYPE elevationType = TokenElevationTypeDefault;
    DWORD dwSize = 0; 

    if (!::OpenProcessToken(::GetCurrentProcess(), TOKEN_QUERY, &hToken))
    {
        ExitOnLastError(hr, "Failed to open process token.");
    }

    if (::GetTokenInformation(hToken, TokenElevationType, &elevationType, sizeof(TOKEN_ELEVATION_TYPE), &dwSize))
    {
        *pfPrivileged = (TokenElevationTypeFull == elevationType);
        ExitFunction1(hr = S_OK);
    }

    er = ::GetLastError();
    hr = HRESULT_FROM_WIN32(er);
    // If it's invalid argument, this means they don't support TokenElevationType, and we should fallback to another check
    if (E_INVALIDARG == hr)
    {
        hr = S_OK;
    }
    ExitOnFailure(hr, "Failed to get process token information.");

    // Fallback to this check for some OS's (like XP)
    *pfPrivileged = ::IsUserAnAdmin();

LExit:
    if (hToken)
    {
        ::CloseHandle(hToken);
    }

    return hr; 
}
