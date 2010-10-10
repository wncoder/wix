//-------------------------------------------------------------------------------------------------
// <copyright file="exeengine.cpp" company="Microsoft">
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
//    Module: EXE Engine
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"


// internal function declarations

static HRESULT HandleExitCode(
    __in BURN_PACKAGE* pPackage,
    __in DWORD dwExitCode,
    __out BOOTSTRAPPER_APPLY_RESTART* pRestart
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

    // @Repairable
    hr = XmlGetYesNoAttribute(pixnExePackage, L"Repairable", &pPackage->Exe.fRepairable);
    if (E_NOTFOUND != hr)
    {
        ExitOnFailure(hr, "Failed to get @Repairable.");
    }

    // @Protocol
    hr = XmlGetAttributeEx(pixnExePackage, L"Protocol", &scz);
    if (SUCCEEDED(hr))
    {
        if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, 0, scz, -1, L"burn", -1))
        {
            pPackage->Exe.protocol = BURN_EXE_PROTOCOL_TYPE_BURN;
        }
        else if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, 0, scz, -1, L"netfx4", -1))
        {
            pPackage->Exe.protocol = BURN_EXE_PROTOCOL_TYPE_NETFX4;
        }
        else if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, 0, scz, -1, L"none", -1))
        {
            pPackage->Exe.protocol = BURN_EXE_PROTOCOL_TYPE_NONE;
        }
        else
        {
            hr = E_UNEXPECTED;
            ExitOnFailure1(hr, "Invalid exit code type: %ls", scz);
        }
    }
    else if (E_NOTFOUND != hr)
    {
        ExitOnFailure(hr, "Failed to get @Protocol.");
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
    if (pPackage->Exe.sczDetectCondition && *pPackage->Exe.sczDetectCondition)
    {
        hr = ConditionEvaluate(pVariables, pPackage->Exe.sczDetectCondition, &fDetected);
        ExitOnFailure(hr, "Failed to evaluate executable package detect condition.");
    }

    // update detect state
    pPackage->currentState = fDetected ? BOOTSTRAPPER_PACKAGE_STATE_PRESENT : BOOTSTRAPPER_PACKAGE_STATE_ABSENT;

LExit:
    return hr;
}

//
// Plan - calculates the execute and rollback state for the requested package state.
//
extern "C" HRESULT ExeEnginePlanPackage(
    __in DWORD dwPackageSequence,
    __in BURN_PACKAGE* pPackage,
    __in BURN_PLAN* pPlan,
    __in BURN_LOGGING* pLog,
    __in BURN_VARIABLES* pVariables,
    __in_opt HANDLE hCacheEvent,
    __out BOOTSTRAPPER_ACTION_STATE* pExecuteAction,
    __out BOOTSTRAPPER_ACTION_STATE* pRollbackAction
    )
{
    HRESULT hr = S_OK;
    //BOOL fCondition = FALSE;
    //BOOTSTRAPPER_PACKAGE_STATE expected = BOOTSTRAPPER_PACKAGE_STATE_UNKNOWN;
    BOOTSTRAPPER_ACTION_STATE execute = BOOTSTRAPPER_ACTION_STATE_NONE;
    BOOTSTRAPPER_ACTION_STATE rollback = BOOTSTRAPPER_ACTION_STATE_NONE;
    BURN_EXECUTE_ACTION* pAction = NULL;

    //// evaluate rollback install condition
    //if (pPackage->sczRollbackInstallCondition)
    //{
    //    hr = ConditionEvaluate(pVariables, pPackage->sczRollbackInstallCondition, &fCondition);
    //    ExitOnFailure(hr, "Failed to evaluate rollback install condition.");

    //    expected = fCondition ? BOOTSTRAPPER_PACKAGE_STATE_PRESENT : BOOTSTRAPPER_PACKAGE_STATE_ABSENT;
    //}

    // execute action
    switch (pPackage->currentState)
    {
    case BOOTSTRAPPER_PACKAGE_STATE_PRESENT:
        switch (pPackage->requested)
        {
        case BOOTSTRAPPER_REQUEST_STATE_PRESENT:
            execute = BOOTSTRAPPER_ACTION_STATE_NONE;
            break;
        case BOOTSTRAPPER_REQUEST_STATE_REPAIR:
            execute = pPackage->Exe.fRepairable ? BOOTSTRAPPER_ACTION_STATE_RECACHE : BOOTSTRAPPER_ACTION_STATE_NONE;
            break;
        case BOOTSTRAPPER_REQUEST_STATE_ABSENT:
            execute = pPackage->fUninstallable ? BOOTSTRAPPER_ACTION_STATE_UNINSTALL : BOOTSTRAPPER_ACTION_STATE_NONE;
            break;
        default:
            execute = BOOTSTRAPPER_ACTION_STATE_NONE;
        }
        break;

    case BOOTSTRAPPER_PACKAGE_STATE_ABSENT:
        switch (pPackage->requested)
        {
        case BOOTSTRAPPER_REQUEST_STATE_PRESENT: __fallthrough;
        case BOOTSTRAPPER_REQUEST_STATE_REPAIR:
            execute = BOOTSTRAPPER_ACTION_STATE_INSTALL;
            break;
        case BOOTSTRAPPER_REQUEST_STATE_ABSENT:
            execute = BOOTSTRAPPER_ACTION_STATE_NONE;
            break;
        default:
            execute = BOOTSTRAPPER_ACTION_STATE_NONE;
        }
        break;

    default:
        hr = E_INVALIDARG;
        ExitOnRootFailure(hr, "Invalid package request state.");
    }

    // rollback action
    switch (BOOTSTRAPPER_PACKAGE_STATE_UNKNOWN != pPackage->expected ? pPackage->expected : pPackage->currentState)
    {
    case BOOTSTRAPPER_PACKAGE_STATE_PRESENT:
        switch (pPackage->requested)
        {
        case BOOTSTRAPPER_REQUEST_STATE_PRESENT: __fallthrough;
        case BOOTSTRAPPER_REQUEST_STATE_REPAIR:
            rollback = BOOTSTRAPPER_ACTION_STATE_NONE;
            break;
        case BOOTSTRAPPER_REQUEST_STATE_ABSENT:
            rollback = BOOTSTRAPPER_ACTION_STATE_INSTALL;
            break;
        default:
            rollback = BOOTSTRAPPER_ACTION_STATE_NONE;
            break;
        }
        break;

    case BOOTSTRAPPER_PACKAGE_STATE_ABSENT:
        switch (pPackage->requested)
        {
        case BOOTSTRAPPER_REQUEST_STATE_PRESENT: __fallthrough;
        case BOOTSTRAPPER_REQUEST_STATE_REPAIR:
            rollback = pPackage->fUninstallable ? BOOTSTRAPPER_ACTION_STATE_UNINSTALL : BOOTSTRAPPER_ACTION_STATE_NONE;
            break;
        case BOOTSTRAPPER_REQUEST_STATE_ABSENT:
            rollback = BOOTSTRAPPER_ACTION_STATE_NONE;
            break;
        default:
            rollback = BOOTSTRAPPER_ACTION_STATE_NONE;
            break;
        }
        break;

    default:
        hr = E_INVALIDARG;
        ExitOnRootFailure(hr, "Invalid package expected state.");
    }

    // add wait for cache
    if (hCacheEvent)
    {
        hr = PlanAppendExecuteAction(pPlan, &pAction);
        ExitOnFailure(hr, "Failed to append wait action.");

        pAction->type = BURN_EXECUTE_ACTION_TYPE_SYNCPOINT;
        pAction->syncpoint.hEvent = hCacheEvent;
    }

    // add execute action
    if (BOOTSTRAPPER_ACTION_STATE_NONE != execute)
    {
        hr = PlanAppendExecuteAction(pPlan, &pAction);
        ExitOnFailure(hr, "Failed to append execute action.");

        pAction->type = BURN_EXECUTE_ACTION_TYPE_EXE_PACKAGE;
        pAction->exePackage.pPackage = pPackage;
        pAction->exePackage.action = execute;

        LoggingSetPackageVariable(dwPackageSequence, pPackage, FALSE, pLog, pVariables, NULL); // ignore errors.
    }

    // add rollback action
    if (BOOTSTRAPPER_ACTION_STATE_NONE != rollback)
    {
        hr = PlanAppendRollbackAction(pPlan, &pAction);
        ExitOnFailure(hr, "Failed to append rollback action.");

        pAction->type = BURN_EXECUTE_ACTION_TYPE_EXE_PACKAGE;
        pAction->exePackage.pPackage = pPackage;
        pAction->exePackage.action = rollback;

        LoggingSetPackageVariable(dwPackageSequence, pPackage, TRUE, pLog, pVariables, NULL); // ignore errors.
    }

    // add checkpoints
    if (BOOTSTRAPPER_ACTION_STATE_NONE != execute || BOOTSTRAPPER_ACTION_STATE_NONE != rollback)
    {
        DWORD dwCheckpointId = PlanGetNextCheckpointId();

        // execute checkpoint
        hr = PlanAppendExecuteAction(pPlan, &pAction);
        ExitOnFailure(hr, "Failed to append execute action.");

        pAction->type = BURN_EXECUTE_ACTION_TYPE_CHECKPOINT;
        pAction->checkpoint.dwId = dwCheckpointId;

        // rollback checkpoint
        hr = PlanAppendRollbackAction(pPlan, &pAction);
        ExitOnFailure(hr, "Failed to append rollback action.");

        pAction->type = BURN_EXECUTE_ACTION_TYPE_CHECKPOINT;
        pAction->checkpoint.dwId = dwCheckpointId;
    }

    // return values
    *pExecuteAction = execute;
    *pRollbackAction = rollback;

LExit:
    return hr;
}

extern "C" HRESULT ExeEngineExecutePackage(
    __in BURN_USER_EXPERIENCE* pUX,
    __in HANDLE hElevatedPipe,
    __in BURN_EXECUTE_ACTION* pExecuteAction,
    __in BURN_VARIABLES* pVariables,
    __out BOOTSTRAPPER_APPLY_RESTART* pRestart
    )
{
    HRESULT hr = S_OK;
    LPCWSTR wzArguments = NULL;
    LPWSTR sczArgumentsFormatted = NULL;
    LPWSTR sczCachedDirectory = NULL;
    LPWSTR sczExecutablePath = NULL;
    LPWSTR sczCommand = NULL;
    STARTUPINFOW si = { };
    PROCESS_INFORMATION pi = { };
    DWORD dwExitCode = 0;

    // get cached executable path
    hr = CacheGetCompletedPath(pExecuteAction->exePackage.pPackage->fPerMachine, pExecuteAction->exePackage.pPackage->sczCacheId, &sczCachedDirectory);
    ExitOnFailure1(hr, "Failed to get cached path for package: %ls", pExecuteAction->exePackage.pPackage->sczId);

    hr = PathConcat(sczCachedDirectory, pExecuteAction->exePackage.pPackage->rgPayloads[0].pPayload->sczFilePath, &sczExecutablePath);
    ExitOnFailure(hr, "Failed to build executable path.");

    // pick arguments
    switch (pExecuteAction->exePackage.action)
    {
    case BOOTSTRAPPER_ACTION_STATE_INSTALL:
        wzArguments = pExecuteAction->exePackage.pPackage->Exe.sczInstallArguments;
        break;

    case BOOTSTRAPPER_ACTION_STATE_UNINSTALL:
        wzArguments = pExecuteAction->exePackage.pPackage->Exe.sczUninstallArguments;
        break;

    case BOOTSTRAPPER_ACTION_STATE_RECACHE:
        wzArguments = pExecuteAction->exePackage.pPackage->Exe.sczRepairArguments;
        break;

    default:
        hr = E_UNEXPECTED;
        ExitOnFailure(hr, "Failed to get action arguments for executable package.");
    }

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

    // Log before we add the secret pipe name and client token for embedded processes.
    LogId(REPORT_STANDARD, MSG_APPLYING_PACKAGE, pExecuteAction->exePackage.pPackage->sczId, LoggingActionStateToString(pExecuteAction->exePackage.action), sczExecutablePath, sczCommand);

    if (BURN_EXE_PROTOCOL_TYPE_BURN == pExecuteAction->exePackage.pPackage->Exe.protocol)
    {
        hr = EmbeddedLaunchChildProcess(pExecuteAction->exePackage.pPackage, pUX, hElevatedPipe, sczExecutablePath, sczCommand, &pi.hProcess);
        ExitOnFailure1(hr, "Failed to launch executable as embedded from path: %ls", sczExecutablePath);
    }
    else // create and wait for the executable process like normal.
    {
        si.cb = sizeof(si); // TODO: hookup the stdin/stdout/stderr pipes for logging purposes?
        if (!::CreateProcessW(sczExecutablePath, sczCommand, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi))
        {
            ExitWithLastError1(hr, "Failed to CreateProcess on path: %ls", sczExecutablePath);
        }
    }

    hr = ProcWaitForCompletion(pi.hProcess, INFINITE, &dwExitCode);
    ExitOnFailure1(hr, "Failed to wait for executable to complete: %ls", sczExecutablePath);

    hr = HandleExitCode(pExecuteAction->exePackage.pPackage, dwExitCode, pRestart);
    ExitOnRootFailure1(hr, "Process returned error: %u", dwExitCode);

LExit:
    ReleaseStr(sczArgumentsFormatted);
    ReleaseStr(sczCachedDirectory);
    ReleaseStr(sczExecutablePath);
    ReleaseStr(sczCommand);

    ReleaseHandle(pi.hThread);
    ReleaseHandle(pi.hProcess);

    return hr;
}


// internal helper functions

static HRESULT HandleExitCode(
    __in BURN_PACKAGE* pPackage,
    __in DWORD dwExitCode,
    __out BOOTSTRAPPER_APPLY_RESTART* pRestart
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
        *pRestart = BOOTSTRAPPER_APPLY_RESTART_NONE;
        hr = S_OK;
        break;

    case BURN_EXE_EXIT_CODE_TYPE_ERROR:
        *pRestart = BOOTSTRAPPER_APPLY_RESTART_NONE;
        hr = HRESULT_FROM_WIN32(dwExitCode);
        if (SUCCEEDED(hr))
        {
            hr = E_FAIL;
        }
        break;

    case BURN_EXE_EXIT_CODE_TYPE_SCHEDULE_REBOOT:
        *pRestart = BOOTSTRAPPER_APPLY_RESTART_REQUIRED;
        hr = S_OK;
        break;

    case BURN_EXE_EXIT_CODE_TYPE_FORCE_REBOOT:
        *pRestart = BOOTSTRAPPER_APPLY_RESTART_INITIATED;
        hr = S_OK;
        break;

    default:
        hr = E_UNEXPECTED;
        break;
    }

//LExit:
    return hr;
}
