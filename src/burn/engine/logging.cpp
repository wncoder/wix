//-------------------------------------------------------------------------------------------------
// <copyright file="logging.cpp" company="Microsoft">
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
//    Module: Core
//
//    Setup chainer/bootstrapper logging for WiX toolset.
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"

// structs



// internal function declarations



// function definitions

extern "C" HRESULT LoggingOpen(
    __in BURN_LOGGING* pLog,
    __in_z LPCWSTR wzCommandLine,
    __in BURN_VARIABLES* pVariables
    )
{
    HRESULT hr = S_OK;
    LPWSTR sczExePath = NULL;
    LPWSTR sczExeDir = NULL;

    PathForCurrentProcess(&sczExePath, NULL); // Ignore failure.

    hr = PathGetDirectory(sczExePath, &sczExeDir);
    ExitOnFailure(hr, "Failed to get directory bundle executable is in");

    if (pLog->sczPath && *pLog->sczPath)
    {
        hr = LogOpen(sczExeDir, pLog->sczPath, NULL, NULL, pLog->fAppend, FALSE, &pLog->sczPath);
        ExitOnFailure1(hr, "Failed to open log: %ls", pLog->sczPath);

        pLog->state = BURN_LOGGING_STATE_OPEN;
    }
    else if (pLog->sczPrefix && *pLog->sczPrefix)
    {
        hr = LogOpen(NULL, pLog->sczPrefix, NULL, pLog->sczExtension, FALSE, FALSE, &pLog->sczPath);
        ExitOnFailure1(hr, "Failed to open log with prefix: %ls", pLog->sczPrefix);

        pLog->state = BURN_LOGGING_STATE_OPEN;
    }
    else // no logging enabled.
    {
        LogDisable();
        pLog->state = BURN_LOGGING_STATE_DISABLED;
    }

    // If the log was opened, write the header info and update the prefix and extension to match
    // the log name so future logs are opened with the same pattern.
    if (BURN_LOGGING_STATE_OPEN == pLog->state)
    {
        LogId(REPORT_STANDARD, MSG_BURN_INFO, szVerMajorMinorBuild, sczExePath, wzCommandLine);

        LPCWSTR wzExtension = PathExtension(pLog->sczPath);
        ExitOnNull1(wzExtension, hr, E_UNEXPECTED, "Failed to find extension on log path: %ls", pLog->sczPath);

        hr = StrAllocString(&pLog->sczPrefix, pLog->sczPath, wzExtension - pLog->sczPath);
        ExitOnFailure(hr, "Failed to copy log path to prefix.");

        hr = StrAllocString(&pLog->sczExtension, wzExtension + 1, 0);
        ExitOnFailure(hr, "Failed to copy log extension to extension.");

        if (pLog->sczPathVariable && *pLog->sczPathVariable)
        {
            VariableSetString(pVariables, pLog->sczPathVariable, pLog->sczPath); // Ignore failure.
        }
    }

LExit:
    ReleaseStr(sczExePath);
    ReleaseStr(sczExeDir);

    return hr;
}

extern "C" HRESULT LoggingSetPackageVariable(
    __in DWORD dwPackageSequence,
    __in BURN_PACKAGE* pPackage,
    __in BOOL fRollback,
    __in BURN_LOGGING* pLog,
    __in BURN_VARIABLES* pVariables,
    __out_opt LPWSTR* psczLogPath
    )
{
    HRESULT hr = S_OK;
    LPWSTR sczLogPath = NULL;

    if ((!fRollback && pPackage->sczLogPathVariable && *pPackage->sczLogPathVariable) ||
        (fRollback && pPackage->sczRollbackLogPathVariable && *pPackage->sczRollbackLogPathVariable))
    {
        hr = StrAllocFormatted(&sczLogPath, L"%ls_%u_%ls%ls.%ls", pLog->sczPrefix, dwPackageSequence, pPackage->sczId, fRollback ? L"_rollback" : L"", pLog->sczExtension);
        ExitOnFailure(hr, "Failed to allocate path for package log.");

        hr = VariableSetString(pVariables, fRollback ? pPackage->sczRollbackLogPathVariable : pPackage->sczLogPathVariable, sczLogPath);
        ExitOnFailure(hr, "Failed to set log path into variable.");

        if (psczLogPath)
        {
            hr = StrAllocString(psczLogPath, sczLogPath, 0);
            ExitOnFailure(hr, "Failed to copy package log path.");
        }
    }

LExit:
    ReleaseStr(sczLogPath);
    return hr;
}

extern "C" LPCSTR LoggingBurnActionToString(
    __in BOOTSTRAPPER_ACTION action
    )
{
    switch (action)
    {
    case BOOTSTRAPPER_ACTION_UNKNOWN:
        return "Unknown";
    case BOOTSTRAPPER_ACTION_HELP:
        return "Help";
    case BOOTSTRAPPER_ACTION_UNINSTALL:
        return "Uninstall";
    case BOOTSTRAPPER_ACTION_INSTALL:
        return "Install";
    case BOOTSTRAPPER_ACTION_MODIFY:
        return "Modify";
    case BOOTSTRAPPER_ACTION_REPAIR:
        return "Repair";
    default:
        return "Invalid";
    }
}

extern "C" LPCSTR LoggingActionStateToString(
    __in BOOTSTRAPPER_ACTION_STATE actionState
    )
{
    switch (actionState)
    {
    case BOOTSTRAPPER_ACTION_STATE_NONE:
        return "None";
    case BOOTSTRAPPER_ACTION_STATE_UNINSTALL:
        return "Uninstall";
    case BOOTSTRAPPER_ACTION_STATE_INSTALL:
        return "Install";
    case BOOTSTRAPPER_ACTION_STATE_ADMIN_INSTALL:
        return "AdminInstall";
    case BOOTSTRAPPER_ACTION_STATE_MAINTENANCE:
        return "MaintenanceMode";
    case BOOTSTRAPPER_ACTION_STATE_RECACHE:
        return "Recache";
    case BOOTSTRAPPER_ACTION_STATE_MINOR_UPGRADE:
        return "MinorUpgrade";
    case BOOTSTRAPPER_ACTION_STATE_MAJOR_UPGRADE:
        return "MajorUpgrade";
    case BOOTSTRAPPER_ACTION_STATE_PATCH:
        return "Patch";
    default:
        return "Invalid";
    }
}

extern "C" LPCSTR LoggingBoolToString(
    __in BOOL f
    )
{
    if (f)
    {
        return "Yes";
    }

    return "No";
}

extern "C" LPCSTR LoggingTrueFalseToString(
    __in BOOL f
    )
{
    if (f)
    {
        return "true";
    }

    return "false";
}

extern "C" LPCSTR LoggingPackageStateToString(
    __in BOOTSTRAPPER_PACKAGE_STATE packageState
    )
{
    switch (packageState)
    {
    case BOOTSTRAPPER_PACKAGE_STATE_UNKNOWN:
        return "Unknown";
    case BOOTSTRAPPER_PACKAGE_STATE_ABSENT:
        return "Absent";
    case BOOTSTRAPPER_PACKAGE_STATE_CACHED:
        return "Cached";
    case BOOTSTRAPPER_PACKAGE_STATE_PRESENT:
        return "Present";
    case BOOTSTRAPPER_PACKAGE_STATE_SUPERSEDED:
        return "Superseded";
    default:
        return "Invalid";
    }
}

extern "C" LPCSTR LoggingPerMachineToString(
    __in BOOL fPerMachine
    )
{
    if (fPerMachine)
    {
        return "PerMachine";
    }

    return "PerUser";
}

extern "C" LPCSTR LoggingRelatedOperationToString(
    __in BOOTSTRAPPER_RELATED_OPERATION operation
    )
{
    switch (operation)
    {
    case BOOTSTRAPPER_RELATED_OPERATION_NONE:
        return "None";
    case BOOTSTRAPPER_RELATED_OPERATION_DOWNGRADE:
        return "Downgrade";
    case BOOTSTRAPPER_RELATED_OPERATION_MINOR_UPDATE:
        return "MinorUpdate";
    case BOOTSTRAPPER_RELATED_OPERATION_MAJOR_UPGRADE:
        return "MajorUpgrade";
    default:
        return "Invalid";
    }
}

extern "C" LPCSTR LoggingRequestStateToString(
    __in BOOTSTRAPPER_REQUEST_STATE requestState
    )
{
    switch (requestState)
    {
    case BOOTSTRAPPER_REQUEST_STATE_NONE:
        return "None";
    case BOOTSTRAPPER_REQUEST_STATE_ABSENT:
        return "Absent";
    case BOOTSTRAPPER_REQUEST_STATE_CACHE:
        return "Cache";
    case BOOTSTRAPPER_REQUEST_STATE_PRESENT:
        return "Present";
    case BOOTSTRAPPER_REQUEST_STATE_REPAIR:
        return "Repair";
    default:
        return "Invalid";
    }
}

// Note: this function is not thread safe.
extern "C" LPCSTR LoggingVersionToString(
    __in DWORD64 dw64Version
    )
{
    static CHAR szVersion[40] = { 0 };
    HRESULT hr = S_OK;

    hr = ::StringCchPrintfA(szVersion, countof(szVersion), "%I64u.%I64u.%I64u.%I64u", dw64Version >> 48 & 0xFFFF, dw64Version >> 32 & 0xFFFF, dw64Version >> 16 & 0xFFFF, dw64Version  & 0xFFFF);
    if (FAILED(hr))
    {
        memset(szVersion, 0, sizeof(szVersion));
    }

    return szVersion;
}
