//-------------------------------------------------------------------------------------------------
// <copyright file="exeengine.cpp" company="Microsoft">
//    Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//
// <summary>
//    Module: EXE Engine
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"


// internal function declarations

static HRESULT HandleExitCode(
    __in BURN_PACKAGE* pPackage,
    __in DWORD dwExitCode,
    __out HRESULT* phrExitCode
    );


// function definitions

extern "C" HRESULT ExeEngineParsePackageFromXml(
    __in IXMLDOMNode* pixnExePackage,
    __in BURN_PACKAGE* pPackage
    )
{
    HRESULT hr = S_OK;
    IXMLDOMNodeList* pixnNodes = NULL;
    IXMLDOMNode* pixnNode = NULL;
    DWORD cNodes = 0;
    LPWSTR scz = NULL;

    // @DetectCondition
    hr = XmlGetAttributeEx(pixnExePackage, L"DetectCondition", &pPackage->Exe.sczDetectCondition);
    ExitOnFailure(hr, "Failed to get @DetectCondition.");

    // @InstallArguments
    hr = XmlGetAttributeEx(pixnExePackage, L"InstallArguments", &pPackage->Exe.sczInstallArguments);
    ExitOnFailure(hr, "Failed to get @InstallArguments.");

    // @UninstallArguments
    hr = XmlGetAttributeEx(pixnExePackage, L"UninstallArguments", &pPackage->Exe.sczUninstallArguments);
    ExitOnFailure(hr, "Failed to get @UninstallArguments.");

    // @RepairArguments
    hr = XmlGetAttributeEx(pixnExePackage, L"RepairArguments", &pPackage->Exe.sczRepairArguments);
    ExitOnFailure(hr, "Failed to get @RepairArguments.");

    // @AllowRepair
    hr = XmlGetYesNoAttribute(pixnNode, L"AllowRepair", &pPackage->Exe.fAllowRepair);
    if (E_NOTFOUND != hr)
    {
        ExitOnFailure(hr, "Failed to get @AllowRepair.");
    }

    // select exit code nodes
    hr = XmlSelectNodes(pixnExePackage, L"ExitCode", &pixnNodes);
    ExitOnFailure(hr, "Failed to select exit code nodes.");

    // get exit code node count
    hr = pixnNodes->get_length((long*)&cNodes);
    ExitOnFailure(hr, "Failed to get exit code node count.");

    if (cNodes)
    {
        // allocate memory for exit codes
        pPackage->Exe.rgExitCodes = (BURN_EXE_EXIT_CODE*)MemAlloc(sizeof(BURN_EXE_EXIT_CODE) * cNodes, TRUE);
        ExitOnNull(pPackage->Exe.rgExitCodes, hr, E_OUTOFMEMORY, "Failed to allocate memory for exit code structs.");

        pPackage->Exe.cExitCodes = cNodes;

        // parse package elements
        for (DWORD i = 0; i < cNodes; ++i)
        {
            BURN_EXE_EXIT_CODE* pExitCode = &pPackage->Exe.rgExitCodes[i];

            hr = XmlNextElement(pixnNodes, &pixnNode, NULL);
            ExitOnFailure(hr, "Failed to get next node.");

            // @Type
            hr = XmlGetAttributeEx(pixnNode, L"Type", &scz);
            ExitOnFailure(hr, "Failed to get @Type.");

            if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, 0, scz, -1, L"success", -1))
            {
                pExitCode->type = BURN_EXE_EXIT_CODE_TYPE_SUCCESS;
            }
            else if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, 0, scz, -1, L"error", -1))
            {
                pExitCode->type = BURN_EXE_EXIT_CODE_TYPE_ERROR;
            }
            else if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, 0, scz, -1, L"scheduleReboot", -1))
            {
                pExitCode->type = BURN_EXE_EXIT_CODE_TYPE_SCHEDULE_REBOOT;
            }
            else if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, 0, scz, -1, L"forceReboot", -1))
            {
                pExitCode->type = BURN_EXE_EXIT_CODE_TYPE_FORCE_REBOOT;
            }
            else
            {
                hr = E_UNEXPECTED;
                ExitOnFailure1(hr, "Invalid exit code type: %ls", scz);
            }

            // @Code
            hr = XmlGetAttributeEx(pixnNode, L"Code", &scz);
            ExitOnFailure(hr, "Failed to get @Code.");

            if (L'*' == scz[0])
            {
                pExitCode->fWildcard = TRUE;
            }
            else
            {
                hr = StrStringToUInt32(scz, 0, (UINT*)&pExitCode->dwCode);
                ExitOnFailure1(hr, "Failed to parse @Code value: %ls", scz);
            }

            // prepare next iteration
            ReleaseNullObject(pixnNode);
        }
    }

    hr = S_OK;

LExit:
    ReleaseObject(pixnNodes);
    ReleaseObject(pixnNode);
    ReleaseStr(scz);

    return hr;
}

extern "C" void ExeEnginePackageUninitialize(
    __in BURN_PACKAGE* pPackage
    )
{
    ReleaseStr(pPackage->Exe.sczDetectCondition);
    ReleaseStr(pPackage->Exe.sczInstallArguments);
    ReleaseStr(pPackage->Exe.sczRepairArguments);
    ReleaseStr(pPackage->Exe.sczUninstallArguments);
    //ReleaseStr(pPackage->Exe.sczProgressSwitch);
    ReleaseMem(pPackage->Exe.rgExitCodes);

    // clear struct
    memset(&pPackage->Exe, 0, sizeof(pPackage->Exe));
}

extern "C" HRESULT ExeEngineDetectPackage(
    __in BURN_PACKAGE* pPackage,
    __in BURN_VARIABLES* pVariables
    )
{
    HRESULT hr = S_OK;
    BOOL fDetected = FALSE;

    // evaluate detect condition
    if (pPackage->Exe.sczDetectCondition)
    {
        hr = ConditionEvaluate(pVariables, pPackage->Exe.sczDetectCondition, &fDetected);
        ExitOnFailure(hr, "Failed to evaluate executable package detect condition.");
    }

    // update detect state
    pPackage->currentState = fDetected ? PACKAGE_STATE_PRESENT : PACKAGE_STATE_ABSENT;

LExit:
    return hr;
}

//
// Plan - calculates the execute and rollback state for the requested package state.
//
extern "C" HRESULT ExeEnginePlanPackage(
    __in BURN_PACKAGE* pPackage,
    __in BURN_VARIABLES* pVariables
    )
{
    HRESULT hr = S_OK;
    PACKAGE_STATE expected = PACKAGE_STATE_UNKNOWN;
    REQUEST_STATE requested = REQUEST_STATE_NONE;
    ACTION_STATE execute = ACTION_STATE_NONE;
    ACTION_STATE rollback = ACTION_STATE_NONE;
    BOOL fCondition = FALSE;

    // TODO: add code to allow the UX to override expected and requested state.

    // evaluate install condition
    if (pPackage->sczInstallCondition)
    {
        hr = ConditionEvaluate(pVariables, pPackage->sczInstallCondition, &fCondition);
        ExitOnFailure(hr, "Failed to evaluate install condition.");

        requested = fCondition ? REQUEST_STATE_PRESENT : REQUEST_STATE_ABSENT;
    }

    // evaluate rollback install condition
    if (pPackage->sczRollbackInstallCondition)
    {
        hr = ConditionEvaluate(pVariables, pPackage->sczRollbackInstallCondition, &fCondition);
        ExitOnFailure(hr, "Failed to evaluate rollback install condition.");

        expected = fCondition ? PACKAGE_STATE_PRESENT : PACKAGE_STATE_ABSENT;
    }

    // execute action
    switch (requested)
    {
    case PACKAGE_STATE_PRESENT:
        switch (requested)
        {
        case REQUEST_STATE_PRESENT:
            execute = ACTION_STATE_NONE;
            break;
        case REQUEST_STATE_REPAIR:
            execute = pPackage->Exe.fAllowRepair ? ACTION_STATE_RECACHE : ACTION_STATE_NONE;
            break;
        case REQUEST_STATE_ABSENT:
            execute = pPackage->fUninstallable ? ACTION_STATE_UNINSTALL : ACTION_STATE_NONE;
            break;
        default:
            execute = ACTION_STATE_NONE;
        }
        break;
    case PACKAGE_STATE_ABSENT:
        switch (requested)
        {
        case REQUEST_STATE_PRESENT: __fallthrough;
        case REQUEST_STATE_REPAIR:
            execute = ACTION_STATE_INSTALL;
            break;
        case REQUEST_STATE_ABSENT:
            execute = ACTION_STATE_NONE;
            break;
        default:
            execute = ACTION_STATE_NONE;
        }
        break;
    default:
        hr = E_INVALIDARG;
        ExitOnRootFailure(hr, "Invalid package request state.");
    }

    // rollback action
    switch (PACKAGE_STATE_UNKNOWN != expected ? expected : requested)
    {
    case PACKAGE_STATE_PRESENT:
        switch (requested)
        {
        case REQUEST_STATE_PRESENT: __fallthrough;
        case REQUEST_STATE_REPAIR:
            rollback = ACTION_STATE_NONE;
            break;
        case REQUEST_STATE_ABSENT:
            rollback = ACTION_STATE_INSTALL;
            break;
        default:
            rollback = ACTION_STATE_NONE;
            break;
        }
        break;
    case PACKAGE_STATE_ABSENT:
        switch (requested)
        {
        case REQUEST_STATE_PRESENT: __fallthrough;
        case REQUEST_STATE_REPAIR:
            rollback = pPackage->fUninstallable ? ACTION_STATE_UNINSTALL : ACTION_STATE_NONE;
            break;
        case REQUEST_STATE_ABSENT:
            rollback = ACTION_STATE_NONE;
            break;
        default:
            rollback = ACTION_STATE_NONE;
            break;
        }
        break;
    default:
        hr = E_INVALIDARG;
        ExitOnRootFailure(hr, "Invalid package expected state.");
    }

    // update package actions
    pPackage->executeAction = execute;
    pPackage->rollbackAction = rollback;

LExit:
    return hr;
}

extern "C" HRESULT ExeEngineConfigurePackage(
    __in BURN_PACKAGE* pPackage,
    __in BURN_VARIABLES* pVariables,
    __in BOOL fExecutingRollback
    )
{
    HRESULT hr = S_OK;
    ACTION_STATE action = ACTION_STATE_NONE;
    LPCWSTR wzArguments = NULL;
    LPWSTR sczArgumentsFormatted = NULL;
    LPWSTR sczExecutablePath = NULL;
    LPWSTR sczCommand = NULL;
    STARTUPINFOW si = { };
    PROCESS_INFORMATION pi = { };
    DWORD dwExitCode = 0;
    HRESULT hrExitCode = S_OK;

    // get action
    action = fExecutingRollback ? pPackage->rollbackAction : pPackage->executeAction;

    switch (action)
    {
    case ACTION_STATE_INSTALL:
        wzArguments = pPackage->Exe.sczInstallArguments;
        break;

    case ACTION_STATE_UNINSTALL:
        wzArguments = pPackage->Exe.sczUninstallArguments;
        break;

    case ACTION_STATE_RECACHE:
        wzArguments = pPackage->Exe.sczRepairArguments;
        break;

    default:
        hr = E_UNEXPECTED;
        ExitOnFailure(hr, "Failed to get action arguments for executable package.");
    }

    // Always use the first payload as the installation package.
    // TODO: pick engine payload

    // build command
    if (wzArguments && *wzArguments)
    {
        hr = VariableFormatString(pVariables, wzArguments, &sczArgumentsFormatted, NULL);
        ExitOnFailure(hr, "Failed to format argument string.");

        hr = StrAllocFormatted(&sczCommand, L"\"%s\" %s", sczExecutablePath, sczArgumentsFormatted);
    }
    else
    {
        hr = StrAllocFormatted(&sczCommand, L"\"%s\"", sczExecutablePath);
    }
    ExitOnFailure(hr, "Failed to create executable command.");

    // create process
    si.cb = sizeof(si); // TODO: hookup the stdin/stdout/stderr pipes for logging purposes?
    if (!::CreateProcessW(sczExecutablePath, sczCommand, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi))
    {
        ExitWithLastError1(hr, "Failed to CreateProcess on path: %ls", sczExecutablePath);
    }

    // wait for process to terminate
    if (WAIT_OBJECT_0 != ::WaitForSingleObject(pi.hProcess, INFINITE))
    {
        hr = E_UNEXPECTED;
        ExitOnFailure1(hr, "Unexpected terminated process: %ls", sczExecutablePath);
    }

    // get process exit code
    if (!::GetExitCodeProcess(pi.hProcess, &dwExitCode))
    {
        ExitWithLastError(hr, "Failed to get process exit code.");
    }

    hr = HandleExitCode(pPackage, dwExitCode, &hrExitCode);
    ExitOnFailure(hr, "Failed to process exit code.");

    hr = hrExitCode;

LExit:
    ReleaseStr(sczArgumentsFormatted);
    ReleaseStr(sczExecutablePath);
    ReleaseStr(sczCommand);

    if (pi.hProcess)
    {
        ::CloseHandle(pi.hProcess);
    }
    if (pi.hThread)
    {
        ::CloseHandle(pi.hThread);
    }

    return hr;
}


// internal helper functions

static HRESULT HandleExitCode(
    __in BURN_PACKAGE* pPackage,
    __in DWORD dwExitCode,
    __out HRESULT* phrExitCode
    )
{
    HRESULT hr = S_OK;
    BURN_EXE_EXIT_CODE_TYPE typeCode = BURN_EXE_EXIT_CODE_TYPE_NONE;

    for (DWORD i = 0; i < pPackage->Exe.cExitCodes; ++i)
    {
        BURN_EXE_EXIT_CODE* pExitCode = &pPackage->Exe.rgExitCodes[i];

        // If this is a wildcard, use the last one we come across.
        if (pExitCode->fWildcard)
        {
            typeCode = pExitCode->type;
        }
        else if (dwExitCode == pExitCode->dwCode) // If we have an exact match on the error code use that and stop looking.
        {
            typeCode = pExitCode->type;
            break;
        }
    }

    // If we didn't find a matching code then say that 0 == success and
    // everything else is an error.
    if (BURN_EXE_EXIT_CODE_TYPE_NONE == typeCode)
    {
        if (0 == dwExitCode)
        {
            typeCode = BURN_EXE_EXIT_CODE_TYPE_SUCCESS;
        }
        else
        {
            typeCode = BURN_EXE_EXIT_CODE_TYPE_ERROR;
        }
    }

    switch (typeCode)
    {
    case BURN_EXE_EXIT_CODE_TYPE_SUCCESS:
        *phrExitCode = S_OK;
        break;

    case BURN_EXE_EXIT_CODE_TYPE_ERROR:
        *phrExitCode = HRESULT_FROM_WIN32(ERROR_INSTALL_FAILURE);
        break;

    case BURN_EXE_EXIT_CODE_TYPE_SCHEDULE_REBOOT:
        *phrExitCode = HRESULT_FROM_WIN32(ERROR_SUCCESS_REBOOT_REQUIRED);
        break;

    case BURN_EXE_EXIT_CODE_TYPE_FORCE_REBOOT:
        *phrExitCode = HRESULT_FROM_WIN32(ERROR_INSTALL_SUSPEND);
        break;

    default:
        hr = E_UNEXPECTED;
        break;
    }

LExit:
    return hr;
}
