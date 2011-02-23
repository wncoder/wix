//-------------------------------------------------------------------------------------------------
// <copyright file="balretry.cpp" company="Microsoft">
//    Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//
// <summary>
// Bootstrapper Application Layer retry utility.
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"

// prototypes

static DWORD vdwMaxRetries = 0;
static DWORD vdwTimeout = 0;

static LPWSTR vsczCurrentPackageId = NULL;
static DWORD vdwRetryCount = 0;
static DWORD vdwLastError = ERROR_SUCCESS;


extern "C" void BalRetryInitialize(
    __in DWORD dwMaxRetries,
    __in DWORD dwTimeout
    )
{
    vdwMaxRetries = dwMaxRetries;
    vdwTimeout = dwTimeout;

    ReleaseNullStr(vsczCurrentPackageId);
    vdwRetryCount = 0;
    vdwLastError = ERROR_SUCCESS;
}


extern "C" void BalRetryUninitialize()
{
    vdwLastError = ERROR_SUCCESS;
    vdwRetryCount = 0;
    ReleaseNullStr(vsczCurrentPackageId);

    vdwMaxRetries = 0;
    vdwTimeout = 0;
}


extern "C" void BalRetryStartPackage(
    __in_z_opt LPCWSTR wzPackageId
    )
{
    if (!wzPackageId || !*wzPackageId)
    {
        ReleaseNullStr(vsczCurrentPackageId);
    }
    else if (vsczCurrentPackageId && CSTR_EQUAL == ::CompareStringW(LOCALE_NEUTRAL, 0, wzPackageId, -1, vsczCurrentPackageId, -1))
    {
        ++vdwRetryCount;
        ::Sleep(vdwTimeout);
    }
    else
    {
        StrAllocString(&vsczCurrentPackageId, wzPackageId, 0);
        vdwRetryCount = 0;
    }

    vdwLastError = ERROR_SUCCESS;
}


extern "C" void BalRetryOnError(
    __in_z_opt LPCWSTR wzPackageId,
    __in DWORD dwError
    )
{
    if (!wzPackageId || !*wzPackageId)
    {
        ReleaseNullStr(vsczCurrentPackageId);
    }
    else if (vsczCurrentPackageId && CSTR_EQUAL == ::CompareStringW(LOCALE_NEUTRAL, 0, wzPackageId, -1, vsczCurrentPackageId, -1))
    {
        vdwLastError = dwError;
    }
}


extern "C" int BalRetryEndPackage(
    __in_z_opt LPCWSTR wzPackageId,
    __in HRESULT hrError
    )
{
    int nResult = IDNOACTION;

    if (!wzPackageId || !*wzPackageId)
    {
        ReleaseNullStr(vsczCurrentPackageId);
    }
    else if (vsczCurrentPackageId && CSTR_EQUAL == ::CompareStringW(LOCALE_NEUTRAL, 0, wzPackageId, -1, vsczCurrentPackageId, -1) &&
             vdwRetryCount < vdwMaxRetries)
    {
        // If the service is out of whack, just try again.
        if (HRESULT_FROM_WIN32(ERROR_INSTALL_SERVICE_FAILURE) == hrError)
        {
            nResult = IDRETRY;
        }
        else if (HRESULT_FROM_WIN32(ERROR_INSTALL_FAILURE) == hrError)
        {
            // If we failed with one of these specific error codes, then retry since
            // we've seen these have a high success of succeeding on retry.
            if (1303 == vdwLastError ||
                1304 == vdwLastError ||
                1306 == vdwLastError ||
                1307 == vdwLastError ||
                1309 == vdwLastError ||
                1310 == vdwLastError ||
                1311 == vdwLastError ||
                1312 == vdwLastError ||
                1316 == vdwLastError ||
                1317 == vdwLastError ||
                1321 == vdwLastError ||
                1335 == vdwLastError ||
                1402 == vdwLastError ||
                1406 == vdwLastError ||
                1606 == vdwLastError ||
                1706 == vdwLastError ||
                1719 == vdwLastError ||
                1923 == vdwLastError ||
                1931 == vdwLastError)
            {
                nResult = IDRETRY;
            }
        }
        else if (HRESULT_FROM_WIN32(ERROR_INSTALL_ALREADY_RUNNING) == hrError)
        {
            nResult = IDRETRY;
        }
    }

    return nResult;
}
