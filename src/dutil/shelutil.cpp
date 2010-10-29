//-------------------------------------------------------------------------------------------------
// <copyright file="shelutil.cpp" company="Microsoft">
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
//    Shell helper funtions.
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"


/********************************************************************
 ShelExec() - executes a target.

*******************************************************************/
extern "C" HRESULT DAPI ShelExec(
    __in_z LPCWSTR wzTargetPath,
    __in_opt LPCWSTR wzParameters,
    __in_opt LPCWSTR wzVerb,
    __in_opt LPCWSTR wzWorkingDirectory,
    __in int nShowCmd,
    __out_opt HINSTANCE* phInstance
    )
{
    HRESULT hr = S_OK;

    HINSTANCE hinst = ::ShellExecuteW(NULL, wzVerb, wzTargetPath, wzParameters, wzWorkingDirectory, nShowCmd);
    if (hinst <= HINSTANCE(32))
    {
        switch (reinterpret_cast<DWORD_PTR>(hinst))
        {
        case ERROR_FILE_NOT_FOUND:
            hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
            break;
        case ERROR_PATH_NOT_FOUND:
            hr = HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND);
            break;
        case ERROR_BAD_FORMAT:
            hr = HRESULT_FROM_WIN32(ERROR_BAD_FORMAT);
            break;
        case SE_ERR_ASSOCINCOMPLETE: __fallthrough;
        case SE_ERR_NOASSOC:
            hr = HRESULT_FROM_WIN32(ERROR_NO_ASSOCIATION);
            break;
        case SE_ERR_DDEBUSY: __fallthrough;
        case SE_ERR_DDEFAIL: __fallthrough;
        case SE_ERR_DDETIMEOUT:
            hr = HRESULT_FROM_WIN32(ERROR_DDE_FAIL);
            break;
        case SE_ERR_DLLNOTFOUND:
            hr = HRESULT_FROM_WIN32(ERROR_DLL_NOT_FOUND);
            break;
        case SE_ERR_OOM:
            hr = E_OUTOFMEMORY;
            break;
        case SE_ERR_ACCESSDENIED:
            hr = E_ACCESSDENIED;
            break;
        default:
            hr = E_FAIL;
        }
        ExitOnFailure1(hr, "ShellExec failed with return code %d", reinterpret_cast<DWORD_PTR>(hinst));
    }

    if (phInstance)
    {
        *phInstance = hinst;
    }

LExit:
    return hr;
}


/********************************************************************
 ShelExecEx() - executes a target and waits for the target to finish

*******************************************************************/
extern "C" HRESULT DAPI ShelExecEx(
    __in_z LPCWSTR wzTargetPath,
    __in_z_opt LPCWSTR wzParameters,
    __in_z_opt LPCWSTR wzVerb,
    __in_z_opt LPCWSTR wzWorkingDirectory,
    __in int nShowCmd,
    __out_opt HINSTANCE* phInstance
    )
{
    HRESULT hr = S_OK;

    SHELLEXECUTEINFOW shExecInfo = {};

    shExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
    shExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
    shExecInfo.hwnd = NULL;
    shExecInfo.lpVerb = wzVerb;
    shExecInfo.lpFile = wzTargetPath;
    shExecInfo.lpParameters = wzParameters;
    shExecInfo.lpDirectory = wzWorkingDirectory;
    shExecInfo.nShow = nShowCmd;
    shExecInfo.hInstApp = NULL;

    if ( ::ShellExecuteExW(&shExecInfo) )
    {
        WaitForSingleObject(shExecInfo.hProcess,INFINITE);
    }
    else
    {
        switch (reinterpret_cast<DWORD_PTR>(shExecInfo.hInstApp))
        {
        case SE_ERR_FNF:
            hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
            break;
        case SE_ERR_PNF:
            hr = HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND);
            break;
        case ERROR_BAD_FORMAT:
            hr = HRESULT_FROM_WIN32(ERROR_BAD_FORMAT);
            break;
        case SE_ERR_ASSOCINCOMPLETE:
        case SE_ERR_NOASSOC:
            hr = HRESULT_FROM_WIN32(ERROR_NO_ASSOCIATION);
            break;
        case SE_ERR_DDEBUSY: __fallthrough;
        case SE_ERR_DDEFAIL: __fallthrough;
        case SE_ERR_DDETIMEOUT:
            hr = HRESULT_FROM_WIN32(ERROR_DDE_FAIL);
            break;
        case SE_ERR_DLLNOTFOUND:
            hr = HRESULT_FROM_WIN32(ERROR_DLL_NOT_FOUND);
            break;
        case SE_ERR_OOM:
            hr = E_OUTOFMEMORY;
            break;
        case SE_ERR_ACCESSDENIED:
            hr = E_ACCESSDENIED;
            break;
        default:
            hr = E_FAIL;
        }
        ExitOnFailure1(hr, "ShellExecEx failed with return code %d", reinterpret_cast<DWORD_PTR>(shExecInfo.hInstApp));
    }

    if (phInstance)
    {
        *phInstance = shExecInfo.hInstApp;
    }

LExit:
    return hr;
}


/********************************************************************
 ShelGetFolder() - gets a folder by CSIDL.

*******************************************************************/
extern "C" HRESULT DAPI ShelGetFolder(
    __out_z LPWSTR* psczFolderPath,
    __in int csidlFolder
    )
{
    HRESULT hr = S_OK;
    WCHAR wzPath[MAX_PATH];

    hr = ::SHGetFolderPathW(NULL, csidlFolder | CSIDL_FLAG_CREATE, NULL, SHGFP_TYPE_CURRENT, wzPath);
    ExitOnFailure1(hr, "Failed to get folder path for CSIDL: %d", csidlFolder);

    hr = StrAllocString(psczFolderPath, wzPath, 0);
    ExitOnFailure1(hr, "Failed to copy shell folder path: %S", wzPath);

    hr = PathBackslashTerminate(psczFolderPath);
    ExitOnFailure1(hr, "Failed to backslash terminate shell folder path: %S", *psczFolderPath);

LExit:
    return hr;
}


/********************************************************************
 ShelGetKnownFolder() - gets a folder by KNOWNFOLDERID.

 Note: return E_NOTIMPL if called on pre-Vista operating systems.
*******************************************************************/
#ifndef REFKNOWNFOLDERID
#define REFKNOWNFOLDERID REFGUID
#endif

#ifndef KF_FLAG_CREATE
#define KF_FLAG_CREATE              0x00008000  // Make sure that the folder already exists or create it and apply security specified in folder definition
#endif

EXTERN_C typedef HRESULT (STDAPICALLTYPE *PFN_SHGetKnownFolderPath)(
    REFKNOWNFOLDERID rfid,
    DWORD dwFlags,
    HANDLE hToken,
    PWSTR *ppszPath
    );

extern "C" HRESULT DAPI ShelGetKnownFolder(
    __out_z LPWSTR* psczFolderPath,
    __in REFKNOWNFOLDERID rfidFolder
    )
{
    HRESULT hr = S_OK;
    HMODULE hShell32Dll = NULL;
    PFN_SHGetKnownFolderPath pfn = NULL;
    LPWSTR pwzPath = NULL;

    hr = LoadSystemLibrary(L"shell32.dll", &hShell32Dll);
    if (E_MODNOTFOUND == hr)
    {
        TraceError(hr, "Failed to load shell32.dll");
        ExitFunction1(hr = E_NOTIMPL);
    }
    ExitOnFailure(hr, "Failed to load shell32.dll.");

    pfn = reinterpret_cast<PFN_SHGetKnownFolderPath>(::GetProcAddress(hShell32Dll, "SHGetKnownFolderPath"));
    ExitOnNull(pfn, hr, E_NOTIMPL, "Failed to find SHGetKnownFolderPath entry point.");

    hr = pfn(rfidFolder, KF_FLAG_CREATE, NULL, &pwzPath);
    ExitOnFailure(hr, "Failed to get known folder path.");

    hr = StrAllocString(psczFolderPath, pwzPath, 0);
    ExitOnFailure1(hr, "Failed to copy shell folder path: %S", pwzPath);

    hr = PathBackslashTerminate(psczFolderPath);
    ExitOnFailure1(hr, "Failed to backslash terminate shell folder path: %S", *psczFolderPath);

LExit:
    if (pwzPath)
    {
        ::CoTaskMemFree(pwzPath);
    }

    if (hShell32Dll)
    {
        ::FreeLibrary(hShell32Dll);
    }

    return hr;
}
