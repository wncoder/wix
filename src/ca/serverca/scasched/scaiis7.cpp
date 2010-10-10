//-------------------------------------------------------------------------------------------------
// <copyright file="scaiis.h" company="Microsoft">
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
//    IIS functions for CustomActions
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"

// globals
LPWSTR vpwzCustomActionDataIIS7 = NULL;
DWORD vdwCustomActionCostIIS7 = 0;

#define COST_IIS_WRITEKEY 10


// prototypes
static HRESULT ScaAddToIIS7Configuration(LPCWSTR pwzData, DWORD dwCost);


HRESULT ScaScheduleIIS7Configuration()
{
    HRESULT hr = S_OK;

    if (vpwzCustomActionDataIIS7 && *vpwzCustomActionDataIIS7)
    {
        hr = WcaDoDeferredAction(L"WriteIIS7ConfigChanges", vpwzCustomActionDataIIS7, vdwCustomActionCostIIS7);
        ExitOnFailure(hr, "Failed to schedule IIS7 Config Changes custom action");

        ReleaseNullStr(vpwzCustomActionDataIIS7);
    }
    else
    {
        hr = S_FALSE;
    }

LExit:
    return hr;
}



HRESULT ScaIIS7ConfigTransaction(LPCWSTR wzBackup)
{
    HRESULT hr = S_OK;

    hr = WcaDoDeferredAction(L"StartIIS7ConfigTransaction", wzBackup, COST_IIS_TRANSACTIONS);
    ExitOnFailure(hr, "Failed to schedule StartIIS7ConfigTransaction");

    hr = WcaDoDeferredAction(L"RollbackIIS7ConfigTransaction", wzBackup, 0);   // rollback cost is irrelevant
    ExitOnFailure(hr, "Failed to schedule RollbackIIS7ConfigTransaction");

    hr = WcaDoDeferredAction(L"CommitIIS7ConfigTransaction", wzBackup, 0);  // commit is free
    ExitOnFailure(hr, "Failed to schedule StartIIS7ConfigTransaction");

LExit:
    return hr;
}


HRESULT ScaWriteConfigString(const LPCWSTR wzValue)
{
    HRESULT hr = S_OK;
    WCHAR* pwzCustomActionData = NULL;

    hr = WcaWriteStringToCaData(wzValue, &pwzCustomActionData);
    ExitOnFailure(hr, "Failed to add metabase delete key directive to CustomActionData");

    hr = ScaAddToIIS7Configuration(pwzCustomActionData, COST_IIS_WRITEKEY);
    ExitOnFailure2(hr, "Failed to add ScaWriteMetabaseValue action data: %S, cost: %d", pwzCustomActionData, COST_IIS_WRITEKEY);

LExit:
    ReleaseStr(pwzCustomActionData);

    return hr;
}

HRESULT ScaWriteConfigInteger(DWORD dwValue)
{
    HRESULT hr = S_OK;
    WCHAR* pwzCustomActionData = NULL;

    hr = WcaWriteIntegerToCaData(dwValue, &pwzCustomActionData);
    ExitOnFailure(hr, "Failed to add metabase delete key directive to CustomActionData");

    hr = ScaAddToIIS7Configuration(pwzCustomActionData, COST_IIS_WRITEKEY);
    ExitOnFailure2(hr, "Failed to add ScaWriteMetabaseValue action data: %S, cost: %d", pwzCustomActionData, COST_IIS_WRITEKEY);

LExit:
    ReleaseStr(pwzCustomActionData);

    return hr;
}

HRESULT ScaWriteConfigID(IIS_CONFIG_ACTION emID)
{
    HRESULT hr = S_OK;
    WCHAR* pwzCustomActionData = NULL;

    hr = WcaWriteIntegerToCaData(emID, &pwzCustomActionData);
    ExitOnFailure(hr, "Failed to add metabase delete key directive to CustomActionData");

    hr = ScaAddToIIS7Configuration(pwzCustomActionData, COST_IIS_WRITEKEY);
    ExitOnFailure2(hr, "Failed to add ScaWriteMetabaseValue action data: %S, cost: %d", pwzCustomActionData, COST_IIS_WRITEKEY);

LExit:
    ReleaseStr(pwzCustomActionData);

    return hr;
}

static HRESULT ScaAddToIIS7Configuration(LPCWSTR pwzData, DWORD dwCost)
{
    HRESULT hr = S_OK;

    hr = WcaWriteStringToCaData(pwzData, &vpwzCustomActionDataIIS7);
    ExitOnFailure1(hr, "failed to add to metabase configuration data string: %S", pwzData);

    vdwCustomActionCostIIS7 += dwCost;

LExit:
    return hr;
}

