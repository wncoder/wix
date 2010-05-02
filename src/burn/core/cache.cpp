//-------------------------------------------------------------------------------------------------
// <copyright file="cache.cpp" company="Microsoft">
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
//    Burn cache functions.
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"


extern "C" HRESULT CacheGetCompletedPath(
    __in BOOL fPerMachine,
    __in_z LPCWSTR wzCacheId,
    __inout_z LPWSTR* psczCompletedPath
    )
{
    HRESULT hr = S_OK;
    LPWSTR sczLocalAppData = NULL;
    LPWSTR sczLocalAppDataAppsCache = NULL;

    hr = PathGetKnownFolder(fPerMachine ? CSIDL_COMMON_APPDATA : CSIDL_LOCAL_APPDATA, &sczLocalAppData);
    ExitOnFailure1(hr, "Failed to find local %s appdata directory.", fPerMachine ? "per-machine" : "per-user");

    hr = PathConcat(sczLocalAppData, L"Apps\\Cache", &sczLocalAppDataAppsCache);
    ExitOnFailure(hr, "Failed to format cache path Apps\\Cache.");

    hr = PathConcat(sczLocalAppDataAppsCache, wzCacheId, psczCompletedPath);
    ExitOnFailure(hr, "Failed to format cache path.");

LExit:
    ReleaseStr(sczLocalAppDataAppsCache);
    ReleaseStr(sczLocalAppData);
    return hr;
}

extern "C" HRESULT CacheDeleteDirectory(
    __in_z LPCWSTR wzDirectory
    )
{
    HRESULT hr = S_OK;
    DWORD er = ERROR_SUCCESS;
    //LPWSTR sczDirectory = NULL;
    LPWSTR sczFiles = NULL;
    HANDLE hFind = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATAW wfd = { };

    LPWSTR sczFile = NULL;
    WCHAR wzTempDirectory[MAX_PATH] = { };
    WCHAR wzTempPath[MAX_PATH] = { };

    hr = PathConcat(wzDirectory, L"*.*", &sczFiles);
    ExitOnFailure(hr, "Failed to allocate path to all files in bundle directory.");

    if (!::GetTempPathW(countof(wzTempDirectory), wzTempDirectory))
    {
        ExitWithLastError(hr, "Failed to get temp directory.");
    }

    hFind = ::FindFirstFileW(sczFiles, &wfd);
    if (INVALID_HANDLE_VALUE == hFind)
    {
        ExitOnLastError1(hr, "Failed to get first file in bundle directory: %S", wzDirectory);
    }

    do
    {
        // Skip the dot directories.
        if (L'.' == wfd.cFileName[0] && (L'\0' == wfd.cFileName[1] || (L'.' == wfd.cFileName[1] && L'\0' == wfd.cFileName[2])))
        {
            continue;
        }

        hr = PathConcat(wzDirectory, wfd.cFileName, &sczFile);
        ExitOnFailure(hr, "Failed to allocate file name to move.");

        if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            hr = CacheDeleteDirectory(sczFile);
            ExitOnFailure1(hr, "Failed to delete: %S", sczFile);
        }
        else
        {
            // Clear any attributes that might get in our way.
            if (wfd.dwFileAttributes & FILE_ATTRIBUTE_READONLY || wfd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN || wfd.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM)
            {
                if (!::SetFileAttributesW(sczFile, FILE_ATTRIBUTE_NORMAL))
                {
                    ExitWithLastError1(hr, "Failed to remove attributes from file: %S", sczFile);
                }
            }

            // We'll ignore failures to remove files for now. The directory delete below may
            // be more successful.
            if (!::DeleteFileW(sczFile))
            {
                if (!::GetTempFileNameW(wzTempDirectory, L"BRN", 0, wzTempPath))
                {
                    ExitWithLastError(hr, "Failed to get temp file to move to.");
                }

                if (!::MoveFileExW(sczFile, wzTempPath, MOVEFILE_REPLACE_EXISTING))
                {
                    ::MoveFileExW(sczFile, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
                }
            }
        }
    } while (::FindNextFileW(hFind, &wfd));

    // delete directory
    ::RemoveDirectoryW(wzDirectory);

LExit:
    ReleaseFileFindHandle(hFind);
    ReleaseStr(sczFile);
    ReleaseStr(sczFiles);

    return hr;
}

extern "C" HRESULT CacheEnsureWorkingDirectory(
    __in_z LPCWSTR wzWorkingPath,
    __out_z_opt LPWSTR* psczWorkingDir
    )
{
    HRESULT hr = S_OK;
    LPWSTR sczWorkingDir = NULL;

    hr = StrAllocString(&sczWorkingDir, wzWorkingPath, FileFromPath(wzWorkingPath) - wzWorkingPath);
    ExitOnFailure(hr, "Failed to get working directory for payload.");

    hr = DirEnsureExists(sczWorkingDir, NULL);
    ExitOnFailure(hr, "Failed create working directory for payload.");

    if (psczWorkingDir)
    {
        hr = StrAllocString(psczWorkingDir, sczWorkingDir, 0);
        ExitOnFailure(hr, "Failed to copy working directory.");
    }

LExit:
    ReleaseStr(sczWorkingDir);
    return hr;
}

    
extern "C" HRESULT CacheCompletePayload(
    __in IBurnPayload* pPayload
    )
{
    HRESULT hr = S_OK;
    BOOL fIsCached = FALSE;
    LPWSTR sczWorkingPath = NULL;
    LPWSTR sczCompletedPath = NULL;
    LPWSTR sczCompletedDir = NULL;

    hr = pPayload->IsCached(&fIsCached);
    ExitOnFailure(hr, "Failed to find whether payload is already cached.");

    if (!fIsCached)
    {
        hr = pPayload->GetWorkingPath(&sczWorkingPath);
        ExitOnFailure(hr, "Failed to get working path for payload.");

        hr = pPayload->GetCompletedPath(&sczCompletedPath);
        ExitOnFailure(hr, "Failed to get completed path for payload.");

        hr = StrAllocString(&sczCompletedDir, sczCompletedPath, FileFromPath(sczCompletedPath) - sczCompletedPath);
        ExitOnFailure(hr, "Failed to get completed directory for payload.");

        hr = DirEnsureExists(sczCompletedDir, NULL);
        ExitOnFailure(hr, "Failed create completed directory for payload.");

        LogStringLine(REPORT_STANDARD, "Moving payload from working path '%ls' to completed path '%ls'", sczWorkingPath, sczCompletedPath);
        hr = FileEnsureMove(sczWorkingPath, sczCompletedPath, TRUE, TRUE);
        ExitOnFailure2(hr, "Failed to move %ls to %ls", sczWorkingPath, sczCompletedPath);
    }

LExit:
    ReleaseStr(sczWorkingPath);
    ReleaseStr(sczCompletedPath);
    ReleaseStr(sczCompletedDir);

    return hr;
}


extern "C" HRESULT CacheRemovePackage(
    __in BOOL fPerMachine,
    __in LPCWSTR wzPackageId
    )
{
    HRESULT hr = S_OK;
    LPWSTR sczDirectory = NULL;
    LPWSTR sczFiles = NULL;
    HANDLE hFind = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATAW wfd;

    LPWSTR sczFile = NULL;
    WCHAR wzTempDirectory[MAX_PATH];
    WCHAR wzTempPath[MAX_PATH];

    hr = CacheGetCompletedPath(fPerMachine, wzPackageId, &sczDirectory);
    ExitOnFailure(hr, "Failed to get bundle cache path.");

    hr = PathConcat(sczDirectory, L"*.*", &sczFiles);
    ExitOnFailure(hr, "Failed to allocate path to all files in bundle directory.");

    if (!::GetTempPathW(countof(wzTempDirectory), wzTempDirectory))
    {
        ExitWithLastError(hr, "Failed to get temp directory.");
    }

    hFind = ::FindFirstFileW(sczFiles, &wfd);
    if (INVALID_HANDLE_VALUE == hFind)
    {
        ExitOnLastError1(hr, "Failed to get first file in bundle directory: %S", sczDirectory);
    }

    do
    {
        // Skip the dot directories.
        if (L'.' == wfd.cFileName[0] && (L'\0' == wfd.cFileName[1] || (L'.' == wfd.cFileName[1] && L'\0' == wfd.cFileName[2])))
        {
            continue;
        }

        hr = PathConcat(sczDirectory, wfd.cFileName, &sczFile);
        ExitOnFailure(hr, "Failed to allocate file name to move.");

        if (!::GetTempFileNameW(wzTempDirectory, L"BRN", 0, wzTempPath))
        {
            ExitWithLastError(hr, "Failed to get temp file to move to.");
        }

        // Clear any attributes that might get in our way.
        if (wfd.dwFileAttributes & FILE_ATTRIBUTE_READONLY || wfd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN || wfd.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM)
        {
            if (!::SetFileAttributesW(sczFile, FILE_ATTRIBUTE_NORMAL))
            {
                ExitWithLastError1(hr, "Failed to remove attributes from file: %S", sczFile);
            }
        }

        // We'll ignore failures to remove files for now. The directory delete below may
        // be more successful.
        ::MoveFileExW(sczFile, wzTempPath, MOVEFILE_REPLACE_EXISTING);
    } while (::FindNextFileW(hFind, &wfd));

    hr = DirEnsureDelete(sczDirectory, TRUE, TRUE);
    ExitOnFailure1(hr, "Failed to remove cached directory: %S", sczDirectory);

LExit:
    ReleaseFileFindHandle(hFind);
    ReleaseStr(sczFile);
    ReleaseStr(sczFiles);
    ReleaseStr(sczDirectory);

    return hr;
}
