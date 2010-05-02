//-------------------------------------------------------------------------------------------------
// <copyright file="gaming.cpp" company="Microsoft">
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
//    Game Explorer custom action code.
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"

LPCWSTR vcsGameuxQuery =
    L"SELECT `WixGameExplorer`.`InstanceId`,   `WixGameExplorer`.`File_`,   `File`.`Component_` "
    L"FROM `WixGameExplorer`, `File` "
    L"WHERE `WixGameExplorer`.`File_` = `File`.`File`";
enum eGameuxQuery { egqInstanceId = 1, egqFile, egqComponent };


/******************************************************************
 WriteGameExplorerRegistry - write temporary rows to the Registry
   table that an upgrade to Windows Vista looks for to migrate the
   game registration to Game Explorer.
********************************************************************/
extern "C" HRESULT WriteGameExplorerRegistry(
    __in LPCWSTR wzInstanceId,
    __in LPCWSTR wzComponentId,
    __in LPCWSTR wzGdfPath,
    __in LPCWSTR wzInstallDir
    )
{
    LPWSTR wzRegKey = NULL;
    MSIHANDLE hView = NULL;
    MSIHANDLE hColumns = NULL;

    // both strings go under this new key
    HRESULT hr = StrAllocFormatted(&wzRegKey, L"Software\\Microsoft\\Windows\\CurrentVersion\\GameUX\\GamesToFindOnWindowsUpgrade\\%S", wzInstanceId);
    ExitOnFailure(hr, "failed to allocate GameUX registry key");

    hr = WcaAddTempRecord(&hView, &hColumns, 
        L"Registry",            // the table
        NULL,                   // we don't care about detailed error codes
        1,                      // the column number of the key we want "uniquified" (uniqued?)
        6,                      // the number of columns we're adding
        L"WixGameExplorer",     // primary key (uniquified)
        msidbRegistryRootLocalMachine,
        wzRegKey,
        L"GDFBinaryPath",
        wzGdfPath,
        wzComponentId);
    ExitOnFailure(hr, "failed to add temporary registry row for GDFBinaryPath");

    hr = WcaAddTempRecord(&hView, &hColumns, 
        L"Registry",
        NULL,
        1,
        6,
        L"WixGameExplorer",
        msidbRegistryRootLocalMachine,
        wzRegKey,
        L"GameInstallPath",
        wzInstallDir,
        wzComponentId);
    ExitOnFailure(hr, "failed to add temporary registry row for GameInstallPath");

LExit:
    ::MsiCloseHandle(hView);
    ::MsiCloseHandle(hColumns);
    ReleaseStr(wzRegKey);

    return hr;
}

/******************************************************************
 SchedGameExplorer - entry point for the Game Explorer Custom Action

********************************************************************/
extern "C" HRESULT __stdcall SchedGameExplorer(
    __in BOOL fInstall
    )
{
    HRESULT hr = S_OK;
    int cGames = 0;

    PMSIHANDLE hView = NULL;
    PMSIHANDLE hRec = NULL;

    IGameExplorer* piGameExplorer = NULL;
    LPWSTR pwzInstanceId = NULL;
    LPWSTR pwzFileId = NULL;
    LPWSTR pwzComponentId = NULL;
    LPWSTR pwzFormattedFile = NULL;
    LPWSTR pwzGamePath = NULL;
    LPWSTR pwzGameDir = NULL;
    LPWSTR pwzCustomActionData = NULL;

    // anything to do?
    if (S_OK != WcaTableExists(L"WixGameExplorer"))
    {
        WcaLog(LOGMSG_STANDARD, "WixGameExplorer table doesn't exist, so there are no games to register with Game Explorer");
        goto LExit;
    }

    // try to create an IGameExplorer; if it fails, assume we're on a pre-Vista OS
    hr = ::CoInitialize(NULL);
    ExitOnFailure(hr, "failed to initialize COM");
    hr = ::CoCreateInstance(__uuidof(GameExplorer), NULL, CLSCTX_ALL, __uuidof(IGameExplorer), (LPVOID*)&piGameExplorer); 
    BOOL fHasGameExplorer = SUCCEEDED(hr);
    WcaLog(LOGMSG_STANDARD, "Game Explorer will %S used", fHasGameExplorer ? L"be" : L"NOT be");

    // query and loop through all the games
    hr = WcaOpenExecuteView(vcsGameuxQuery, &hView);
    ExitOnFailure(hr, "failed to open view on WixGameExplorer table");

    while (S_OK == (hr = WcaFetchRecord(hView, &hRec)))
    {
        ++cGames;

        // start with the instance guid
        hr = WcaGetRecordString(hRec, egqInstanceId, &pwzInstanceId);
        ExitOnFailure(hr, "failed to get game instance id");

        // get file id 
        hr = WcaGetRecordString(hRec, egqFile, &pwzFileId);
        ExitOnFailure(hr, "failed to get game file id");

        // turn that into the path to the target file
        hr = StrAllocFormatted(&pwzFormattedFile, L"[#%s]", pwzFileId);
        ExitOnFailure1(hr, "failed to format file string for file: %S", pwzFileId);
        hr = WcaGetFormattedString(pwzFormattedFile, &pwzGamePath);
        ExitOnFailure1(hr, "failed to get formatted string for file: %S", pwzFileId);

        // and then get the directory part of the path
        hr = PathGetDirectory(pwzGamePath, &pwzGameDir);
        ExitOnFailure1(hr, "failed to get path for file: %S", pwzGamePath);

        // get component and its install/action states
        hr = WcaGetRecordString(hRec, egqComponent, &pwzComponentId);
        ExitOnFailure(hr, "failed to get game component id");

        // we need to know if the component's being installed, uninstalled, or reinstalled
        WCA_TODO todo = WcaGetComponentToDo(pwzComponentId);

        // skip this entry if this is the install CA and we are uninstalling the component
        if (fInstall && WCA_TODO_UNINSTALL == todo)
        {
            continue;
        }

        // skip this entry if this is an uninstall CA and we are not uninstalling the component
        if (!fInstall && WCA_TODO_UNINSTALL != todo)
        {
            continue;
        }

        // if we got a Game Explorer, write the CA data; otherwise,
        // just write the registry values for an XP-to-Vista upgrade
        if (fHasGameExplorer)
        {
            // write custom action data: operation, instance guid, path, directory
            hr = WcaWriteIntegerToCaData(todo, &pwzCustomActionData);
            ExitOnFailure1(hr, "failed to write Game Explorer operation to custom action data for instance id: %S", pwzInstanceId);

            hr = WcaWriteStringToCaData(pwzInstanceId, &pwzCustomActionData);
            ExitOnFailure1(hr, "failed to write custom action data for instance id: %S", pwzInstanceId);

            hr = WcaWriteStringToCaData(pwzGamePath, &pwzCustomActionData);
            ExitOnFailure1(hr, "failed to write game path to custom action data for instance id: %S", pwzInstanceId);

            hr = WcaWriteStringToCaData(pwzGameDir, &pwzCustomActionData);
            ExitOnFailure1(hr, "failed to write game install directory to custom action data for instance id: %S", pwzInstanceId);
        }
        else
        {
            hr = WriteGameExplorerRegistry(pwzInstanceId, pwzComponentId, pwzGamePath, pwzGameDir);
            ExitOnFailure1(hr, "failed to write registry rows for game id: %S", pwzInstanceId);
        }
    }

    // reaching the end of the list is actually a good thing, not an error
    if (E_NOMOREITEMS == hr)
    {
        hr = S_OK;
    }
    ExitOnFailure(hr, "Failure occured while processing WixGameExplorer table");

    // schedule ExecGameExplorer if there's anything to do
    if (pwzCustomActionData && *pwzCustomActionData)
    {
        WcaLog(LOGMSG_STANDARD, "Scheduling Game Explorer (%S)", pwzCustomActionData);
        hr = WcaDoDeferredAction(L"WixRollbackGameExplorer", pwzCustomActionData, cGames * COST_GAMEEXPLORER);
        ExitOnFailure(hr, "Failed to schedule Game Explorer rollback");
        hr = WcaDoDeferredAction(L"WixExecGameExplorer", pwzCustomActionData, cGames * COST_GAMEEXPLORER);
        ExitOnFailure(hr, "Failed to schedule Game Explorer execution");
    }

LExit:
    ReleaseStr(pwzInstanceId);
    ReleaseStr(pwzFileId);
    ReleaseStr(pwzComponentId);
    ReleaseStr(pwzFormattedFile);
    ReleaseStr(pwzGamePath);
    ReleaseStr(pwzCustomActionData);
    ReleaseObject(piGameExplorer);

    ::CoUninitialize();

    return hr;
}


/******************************************************************
 SchedGameExplorerUninstall - entry point for the Game Explorer uninstall Custom Action

********************************************************************/
extern "C" UINT __stdcall SchedGameExplorerUninstall(
    __in MSIHANDLE hInstall
    )
{
    HRESULT hr = S_OK;
    UINT er = ERROR_SUCCESS;

    // initialize
    hr = WcaInitialize(hInstall, "SchedGameExplorerUninstall");
    ExitOnFailure(hr, "failed to initialize");

    hr = SchedGameExplorer(FALSE);

LExit:
    return WcaFinalize(er = FAILED(hr) ? ERROR_INSTALL_FAILURE : er);
}

/******************************************************************
 SchedGameExplorer - entry point for the Game Explorer install Custom Action

********************************************************************/
extern "C" UINT __stdcall SchedGameExplorerInstall(
    __in MSIHANDLE hInstall
    )
{
    HRESULT hr = S_OK;
    UINT er = ERROR_SUCCESS;

    // initialize
    hr = WcaInitialize(hInstall, "SchedGameExplorerInstall");
    ExitOnFailure(hr, "failed to initialize");

    hr = SchedGameExplorer(TRUE);

LExit:
    return WcaFinalize(er = FAILED(hr) ? ERROR_INSTALL_FAILURE : er);
}

/******************************************************************
 ExecGameExplorer - entry point for Game Explorer Custom Action

*******************************************************************/
extern "C" UINT __stdcall ExecGameExplorer(
    __in MSIHANDLE hInstall
    )
{
    HRESULT hr = S_OK;
    UINT er = ERROR_SUCCESS;
    BOOL fHasAccess = FALSE;
    GUID guidInstanceId = {0};

    IGameExplorer* piGameExplorer = NULL;
    LPWSTR pwzCustomActionData = NULL;
    LPWSTR pwz = NULL;
    int iOperation = 0;
    LPWSTR pwzInstanceId = NULL;
    LPWSTR pwzGamePath = NULL;
    LPWSTR pwzGameDir = NULL;
    BSTR bstrGamePath = NULL;
    BSTR bstrGameDir = NULL;

    // initialize
    hr = WcaInitialize(hInstall, "ExecGameExplorer");
    ExitOnFailure(hr, "failed to initialize");

    hr = WcaGetProperty( L"CustomActionData", &pwzCustomActionData);
    ExitOnFailure(hr, "failed to get CustomActionData");
    WcaLog(LOGMSG_TRACEONLY, "CustomActionData: %S", pwzCustomActionData);

    // try to create an IGameExplorer
    hr = ::CoInitialize(NULL);
    ExitOnFailure(hr, "failed to initialize COM");
    hr = ::CoCreateInstance(__uuidof(GameExplorer), NULL, CLSCTX_ALL, __uuidof(IGameExplorer), (LPVOID*)&piGameExplorer);

    // nothing to do if there's no Game Explorer (though we should have been scheduled only if
    // there was a Game Explorer when we started the install).
    if (SUCCEEDED(hr))
    {
        // loop through all the passed in data
        pwz = pwzCustomActionData;
        while (pwz && *pwz)
        {
            // extract the custom action data
            hr = WcaReadIntegerFromCaData(&pwz, &iOperation);
            ExitOnFailure(hr, "failed to read operation from custom action data");
            hr = WcaReadStringFromCaData(&pwz, &pwzInstanceId);
            ExitOnFailure(hr, "failed to read instance ID from custom action data");
            hr = WcaReadStringFromCaData(&pwz, &pwzGamePath);
            ExitOnFailure(hr, "failed to read GDF path from custom action data");
            hr = WcaReadStringFromCaData(&pwz, &pwzGameDir);
            ExitOnFailure(hr, "failed to read game installation directory from custom action data");

            // convert from LPWSTRs to BSTRs and GUIDs, which are what IGameExplorer wants
            hr = ::CLSIDFromString(pwzInstanceId, &guidInstanceId);
            ExitOnFailure1(hr, "couldn't convert invalid GUID string '%S'", pwzInstanceId);

            bstrGamePath = ::SysAllocString(pwzGamePath);
            ExitOnNull(bstrGamePath, hr, E_OUTOFMEMORY, "failed SysAllocString for bstrGamePath");
            bstrGameDir = ::SysAllocString(pwzGameDir);
            ExitOnNull(bstrGameDir, hr, E_OUTOFMEMORY, "failed SysAllocString for bstrGameDir");

            // if rolling back, swap INSTALL and UNINSTALL
            if (::MsiGetMode(hInstall, MSIRUNMODE_ROLLBACK))
            {
                if (WCA_TODO_INSTALL == iOperation)
                {
                    iOperation = WCA_TODO_UNINSTALL;
                }
                else if (WCA_TODO_UNINSTALL == iOperation)
                {
                    iOperation = WCA_TODO_INSTALL;
                }
            }

            switch (iOperation)
            {
            case WCA_TODO_INSTALL:
                hr = piGameExplorer->VerifyAccess(bstrGamePath, &fHasAccess);
                ExitOnFailure1(hr, "failed to verify game access: %S", pwzInstanceId);

                if (SUCCEEDED(hr) && fHasAccess)
                {
                    WcaLog(LOGMSG_STANDARD, "Adding game: %S, %S", bstrGamePath, bstrGameDir);
                    hr = piGameExplorer->AddGame(bstrGamePath, bstrGameDir, GIS_ALL_USERS, &guidInstanceId);
                }
                ExitOnFailure1(hr, "failed to add game instance: %S", pwzInstanceId);
                break;
            case WCA_TODO_UNINSTALL:
                hr = piGameExplorer->RemoveGame(guidInstanceId);
                ExitOnFailure1(hr, "failed to remove game instance: %S", pwzInstanceId);
                break;
            case WCA_TODO_REINSTALL:
                hr = piGameExplorer->UpdateGame(guidInstanceId);
                ExitOnFailure1(hr, "failed to update game instance: %S", pwzInstanceId);
                break;
            }

            // Tick the progress bar along for this game
            hr = WcaProgressMessage(COST_GAMEEXPLORER, FALSE);
            ExitOnFailure1(hr, "failed to tick progress bar for game instance: %S", pwzInstanceId);
        }
    }

LExit:
    ReleaseStr(pwzCustomActionData);
    ReleaseStr(pwzInstanceId);
    ReleaseStr(pwzGamePath);
    ReleaseStr(pwzGameDir);
    ReleaseBSTR(bstrGamePath);
    ReleaseBSTR(bstrGameDir);
    ReleaseObject(piGameExplorer);
    ::CoUninitialize();

    return WcaFinalize(er = FAILED(hr) ? ERROR_INSTALL_FAILURE : er);
}

