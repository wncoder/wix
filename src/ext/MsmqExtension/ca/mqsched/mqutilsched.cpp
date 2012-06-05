//-------------------------------------------------------------------------------------------------
// <copyright file="mqutilsched.cpp" company="Microsoft Corporation">
//   Copyright (c) 2004, Microsoft Corporation.
//   This software is released under Common Public License Version 1.0 (CPL).
//   The license and further copyright text can be found in the file LICENSE.TXT
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
// 
// <summary>
//    MSMQ Custom Action utility functions for CustomActions
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"


// function definitions

HRESULT PcaGuidToRegFormat(
    LPWSTR pwzGuid,
    LPWSTR pwzDest,
    SIZE_T cchDest
    )
{
    HRESULT hr = S_OK;

    GUID guid = GUID_NULL;
    int cch = 0;

    WCHAR wz[39];
    ::ZeroMemory(wz, sizeof(wz));

    cch = lstrlenW(pwzGuid);

    if (38 == cch && L'{' == pwzGuid[0] && L'}' == pwzGuid[37])
        StringCchCopyW(wz, countof(wz), pwzGuid);
    else if (36 == cch)
        StringCchPrintfW(wz, countof(wz), L"{%s}", pwzGuid);
    else
        ExitFunction1(hr = E_INVALIDARG);

    // convert string to guid
    hr = ::CLSIDFromString(wz, &guid);
    ExitOnFailure(hr, "Failed to parse guid string");

    // convert guid to string
    if (0 == ::StringFromGUID2(guid, pwzDest, cchDest))
        ExitOnFailure(hr = E_FAIL, "Failed to convert guid to string");

    hr = S_OK;

LExit:
    return hr;
}
