//-------------------------------------------------------------------------------------------------
// <copyright file="pathutil.cpp" company="Microsoft">
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
//    Path helper funtions.
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"

#define PATH_GOOD_ENOUGH 64

/*******************************************************************
 PathFile -  returns a pointer to the file part of the path

********************************************************************/
extern "C" LPWSTR DAPI PathFile(
    __in_z LPCWSTR wzPath
    )
{
    if (!wzPath)
    {
        return NULL;
    }

    LPWSTR wzFile = const_cast<LPWSTR>(wzPath);
    for (LPWSTR wz = wzFile; *wz; wz++)
    {
        // valid delineators 
        //     \ => Windows path
        //     / => unix and URL path
        //     : => relative path from mapped root
        if (L'\\' == *wz || L'/' == *wz || (L':' == *wz && wz == wzPath + 1))
        {
            wzFile = wz + 1;
        }
    }

    return wzFile;
}


/*******************************************************************
 PathGetDirectory - extracts the directory from a path.

********************************************************************/
extern "C" HRESULT DAPI PathGetDirectory(
    __in_z LPCWSTR wzPath,
    __out LPWSTR *psczDirectory
    )
{
    HRESULT hr = S_OK;
    DWORD cchDirectory = DWORD_MAX;

    for (LPCWSTR wz = wzPath; *wz; wz++)
    {
        // valid delineators:
        //     \ => Windows path
        //     / => unix and URL path
        //     : => relative path from mapped root
        if (L'\\' == *wz || L'/' == *wz || (L':' == *wz && wz == wzPath + 1))
        {
            cchDirectory = static_cast<DWORD>(wz - wzPath) + 1;
        }
    }

    if (DWORD_MAX == cchDirectory)
    {
        // we were given just a file name, so there's no directory available
        return S_FALSE;
    }

    hr = StrAllocString(psczDirectory, wzPath, cchDirectory);
    ExitOnFailure(hr, "Failed to copy directory.");

LExit:
    return hr;
}


/*******************************************************************
 PathExpand - gets the full path to a file resolving environment
              variables along the way.

********************************************************************/
extern "C" HRESULT DAPI PathExpand(
    __out LPWSTR *psczFullPath,
    __in_z LPCWSTR wzRelativePath,
    __in DWORD dwResolveFlags
    )
{
    Assert(wzRelativePath && *wzRelativePath);

    HRESULT hr = S_OK;
    DWORD cch = 0;
    LPWSTR sczExpandedPath = NULL;
    DWORD cchExpandedPath = 0;

    LPWSTR sczFullPath = NULL;

    //
    // First, expand any environment variables.
    //
    if (dwResolveFlags & PATH_EXPAND_ENVIRONMENT)
    {
        cchExpandedPath = PATH_GOOD_ENOUGH;

        hr = StrAlloc(&sczExpandedPath, cchExpandedPath);
        ExitOnFailure(hr, "Failed to allocate space for expanded path.");

        cch = ::ExpandEnvironmentStringsW(wzRelativePath, sczExpandedPath, cchExpandedPath);
        if (0 == cch)
        {
            ExitWithLastError1(hr, "Failed to expand environment variables in string: %S", wzRelativePath);
        }
        else if (cchExpandedPath < cch)
        {
            cchExpandedPath = cch;
            hr = StrAlloc(&sczExpandedPath, cchExpandedPath);
            ExitOnFailure(hr, "Failed to re-allocate more space for expanded path.");

            cch = ::ExpandEnvironmentStringsW(wzRelativePath, sczExpandedPath, cchExpandedPath);
            if (0 == cch)
            {
                ExitWithLastError1(hr, "Failed to expand environment variables in string: %S", wzRelativePath);
            }
            else if (cchExpandedPath < cch)
            {
                hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
                ExitOnFailure(hr, "Failed to allocate buffer for expanded path.");
            }
        }

        if (MAX_PATH < cch)
        {
            hr = PathPrefix(&sczExpandedPath); // ignore invald arg from path prefix because this may not be a complete path yet
            if (E_INVALIDARG == hr)
            {
                hr = S_OK;
            }
            ExitOnFailure(hr, "Failed to prefix long path after expanding environment variables.");

            hr = StrMaxLength(sczExpandedPath, reinterpret_cast<DWORD_PTR *>(&cchExpandedPath));
            ExitOnFailure(hr, "Failed to get max length of expanded path.");
        }
    }

    //
    // Second, get the full path.
    //
    if (dwResolveFlags & PATH_EXPAND_FULLPATH)
    {
        LPWSTR wzFileName = NULL;
        LPCWSTR wzPath = sczExpandedPath ? sczExpandedPath : wzRelativePath;
        DWORD cchFullPath = PATH_GOOD_ENOUGH < cchExpandedPath ? cchExpandedPath : PATH_GOOD_ENOUGH;

        hr = StrAlloc(&sczFullPath, cchFullPath);
        ExitOnFailure(hr, "Failed to allocate space for full path.");

        cch = ::GetFullPathNameW(wzPath, cchFullPath, sczFullPath, &wzFileName);
        if (0 == cch)
        {
            ExitWithLastError1(hr, "Failed to get full path for string: %S", wzPath);
        }
        else if (cchFullPath < cch)
        {
            cchFullPath = cch < MAX_PATH ? cch : cch + 7; // ensure space for "\\?\UNC" prefix if needed
            hr = StrAlloc(&sczFullPath, cchFullPath);
            ExitOnFailure(hr, "Failed to re-allocate more space for full path.");

            cch = ::GetFullPathNameW(wzPath, cchFullPath, sczFullPath, &wzFileName);
            if (0 == cch)
            {
                ExitWithLastError1(hr, "Failed to get full path for string: %S", wzPath);
            }
            else if (cchFullPath < cch)
            {
                hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
                ExitOnFailure(hr, "Failed to allocate buffer for full path.");
            }
        }

        if (MAX_PATH < cch)
        {
            hr = PathPrefix(&sczFullPath);
            ExitOnFailure(hr, "Failed to prefix long path after expanding.");
        }
    }
    else
    {
        sczFullPath = sczExpandedPath;
        sczExpandedPath = NULL;
    }

    hr = StrAllocString(psczFullPath, sczFullPath? sczFullPath : wzRelativePath, 0);
    ExitOnFailure(hr, "Failed to copy relative path into full path.");

LExit:
    ReleaseStr(sczFullPath);
    ReleaseStr(sczExpandedPath);

    return hr;
}


/*******************************************************************
 PathPrefix - prefixes a full path with \\?\ or \\?\UNC as 
              appropriate.

********************************************************************/
extern "C" HRESULT DAPI PathPrefix(
    __inout LPWSTR *psczFullPath
    )
{
    Assert(psczFullPath && *psczFullPath);

    HRESULT hr = S_OK;
    LPWSTR wzFullPath = *psczFullPath;
    DWORD_PTR cbFullPath = 0;

    if (((L'a' <= wzFullPath[0] && L'z' >= wzFullPath[0]) ||
         (L'A' <= wzFullPath[0] && L'Z' >= wzFullPath[0])) &&
        L':' == wzFullPath[1] &&
        L'\\' == wzFullPath[2]) // normal path
    {
        hr = StrAllocPrefix(psczFullPath, L"\\\\?\\", 4);
        ExitOnFailure(hr, "Failed to add prefix to file path.");
    }
    else if (L'\\' == wzFullPath[0] && L'\\' == wzFullPath[1]) // UNC
    {
        // ensure that we're not already prefixed
        if (!(L'?' == wzFullPath[2] && L'\\' == wzFullPath[3]))
        {
            hr = StrSize(*psczFullPath, &cbFullPath);
            ExitOnFailure(hr, "Failed to get size of full path.");

            memmove_s(wzFullPath, cbFullPath, wzFullPath + 1, cbFullPath - sizeof(WCHAR));

            hr = StrAllocPrefix(psczFullPath, L"\\\\?\\UNC", 7);
            ExitOnFailure(hr, "Failed to add prefix to UNC path.");
        }
    }
    else
    {
        hr = E_INVALIDARG;
        ExitOnFailure(hr, "Invalid path provided to prefix.");
    }

LExit:
    return hr;
}


/*******************************************************************
 PathFixedBackslashTerminate - appends a \ if path does not have it
                                 already, but fails if the buffer is
                                 insufficient.
********************************************************************/
extern "C" HRESULT DAPI PathFixedBackslashTerminate(
    __inout_ecount_z(cchPath) LPWSTR wzPath,
    __in DWORD_PTR cchPath
    )
{
    HRESULT hr = S_OK;
    size_t cchLength = 0;

    hr = ::StringCchLengthW(wzPath, cchPath, &cchLength);
    ExitOnFailure(hr, "Failed to get length of path.");

    if (cchLength >= cchPath)
    {
        hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
    }
    else if (L'\\' != wzPath[cchLength - 1])
    {
        wzPath[cchLength] = L'\\';
        wzPath[cchLength + 1] = L'\0';
    }

LExit:
    return hr;
}


/*******************************************************************
 PathBackslashTerminate - appends a \ if path does not have it
                                 already.
********************************************************************/
extern "C" HRESULT DAPI PathBackslashTerminate(
    __inout LPWSTR* psczPath
    )
{
    Assert(psczPath && *psczPath);

    HRESULT hr = S_OK;
    DWORD_PTR cchPath = 0;
    size_t cchLength = 0;

    hr = StrMaxLength(*psczPath, &cchPath);
    ExitOnFailure(hr, "Failed to get size of path string.");

    hr = ::StringCchLengthW(*psczPath, cchPath, &cchLength);
    ExitOnFailure(hr, "Failed to get length of path.");

    if (L'\\' != (*psczPath)[cchLength - 1])
    {
        hr = StrAllocConcat(psczPath, L"\\", 1);
        ExitOnFailure(hr, "Failed to concat backslash onto string.");
    }

LExit:
    return hr;
}


/*******************************************************************
 PathForCurrentProcess - gets the full path to the currently executing
                         process or (optionally) a module inside the process.
********************************************************************/
extern "C" HRESULT DAPI PathForCurrentProcess(
    __inout LPWSTR *psczFullPath,
    __in_opt HMODULE hModule
    )
{
    HRESULT hr = S_OK;
    DWORD cch = MAX_PATH;

    do
    {
        hr = StrAlloc(psczFullPath, cch);
        ExitOnFailure(hr, "Failed to allocate string for module path.");

        DWORD cchRequired = ::GetModuleFileNameW(hModule, *psczFullPath, cch);
        if (0 == cchRequired)
        {
            ExitWithLastError(hr, "Failed to get path for executing process.");
        }
        else if (cchRequired == cch)
        {
            cch = cchRequired + 1;
            hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
        }
        else
        {
            hr = S_OK;
        }
    } while (HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER) == hr);

LExit:
    return hr;
}


/*******************************************************************
 PathRelativeToModule - gets the name of a file in the same 
    directory as the current process or (optionally) a module inside 
    the process
********************************************************************/
HRESULT DAPI PathRelativeToModule(
    __inout LPWSTR *psczFullPath,
    __in_opt LPCWSTR wzFileName,
    __in_opt HMODULE hModule
    )
{
    HRESULT hr = PathForCurrentProcess(psczFullPath, hModule);
    ExitOnFailure(hr, "Failed to get current module path.");

    hr = PathGetDirectory(*psczFullPath, psczFullPath);
    ExitOnFailure(hr, "Failed to get current module directory.");

    if (wzFileName)
    {
        hr = PathConcat(*psczFullPath, wzFileName, psczFullPath);
        ExitOnFailure(hr, "Failed to append filename.");
    }

LExit:
    return hr;
}


/*******************************************************************
 PathCreateTempFile

 Note: if wzDirectory is null, ::GetTempPath() will be used instead.
       if wzFileNameTemplate is null, GetTempFileName() will be used instead.
*******************************************************************/
extern "C" HRESULT DAPI PathCreateTempFile(
    __in_opt LPCWSTR wzDirectory,
    __in_opt __format_string LPCWSTR wzFileNameTemplate,
    __in DWORD dwUniqueCount,
    __in DWORD dwFileAttributes,
    __out_opt LPWSTR* psczTempFile,
    __out_opt HANDLE* phTempFile
    )
{
    AssertSz(0 < dwUniqueCount, "Must specify a non-zero unique count.");

    HRESULT hr = S_OK;

    LPWSTR sczTempPath = NULL;
    DWORD cchTempPath = MAX_PATH;

    HANDLE hTempFile = INVALID_HANDLE_VALUE;
    LPWSTR scz = NULL;
    LPWSTR sczTempFile = NULL;

    if (wzDirectory && *wzDirectory)
    {
        hr = StrAllocString(&sczTempPath, wzDirectory, 0);
        ExitOnFailure(hr, "Failed to copy temp path.");
    }
    else
    {
        hr = StrAlloc(&sczTempPath, cchTempPath);
        ExitOnFailure(hr, "Failed to allocate memory for the temp path.");

        if (!::GetTempPathW(cchTempPath, sczTempPath))
        {
            ExitWithLastError(hr, "Failed to get temp path.");
        }
    }

    if (wzFileNameTemplate && *wzFileNameTemplate)
    {
        for (DWORD i = 1; i <= dwUniqueCount && INVALID_HANDLE_VALUE == hTempFile; ++i)
        {
            hr = StrAllocFormatted(&scz, wzFileNameTemplate, i);
            ExitOnFailure(hr, "Failed to allocate memory for file template.");

            hr = StrAllocFormatted(&sczTempFile, L"%s%s", sczTempPath, scz);
            ExitOnFailure(hr, "Failed to allocate temp file name.");

            hTempFile = ::CreateFileW(sczTempFile, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_DELETE, NULL, CREATE_NEW, dwFileAttributes, NULL);
            if (INVALID_HANDLE_VALUE == hTempFile)
            {
                // if the file already exists, just try again
                hr = HRESULT_FROM_WIN32(::GetLastError());
                if (HRESULT_FROM_WIN32(ERROR_FILE_EXISTS) == hr)
                {
                    hr = S_OK;
                }
                ExitOnFailure1(hr, "Failed to create file: %S", sczTempFile);
            }
        }
    }

    // If we were not able to or we did not try to create a temp file, ask
    // the system to provide us a temp file using its built-in mechanism.
    if (INVALID_HANDLE_VALUE == hTempFile)
    {
        hr = StrAlloc(&sczTempFile, MAX_PATH);
        ExitOnFailure(hr, "failed to allocate memory for the temp path");

        if (!::GetTempFileNameW(sczTempPath, L"TMP", 0, sczTempFile))
        {
            ExitWithLastError(hr, "Failed to create new temp file name.");
        }

        hTempFile = ::CreateFileW(sczTempFile, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, dwFileAttributes, NULL);
        if (INVALID_HANDLE_VALUE == hTempFile)
        {
            ExitWithLastError1(hr, "Failed to open new temp file: %S", sczTempFile);
        }
    }

    // If the caller wanted the temp file name or handle, return them here.
    if (psczTempFile)
    {
        hr = StrAllocString(psczTempFile, sczTempFile, 0);
        ExitOnFailure(hr, "Failed to copy temp file string.");
    }

    if (phTempFile)
    {
        *phTempFile = hTempFile;
        hTempFile = INVALID_HANDLE_VALUE;
    }

LExit:
    if (INVALID_HANDLE_VALUE != hTempFile)
    {
        ::CloseHandle(hTempFile);
    }

    ReleaseStr(scz);
    ReleaseStr(sczTempFile);
    ReleaseStr(sczTempPath);

    return hr;
}


/*******************************************************************
 PathCreateTempDirectory

 Note: if wzDirectory is null, ::GetTempPath() will be used instead.
*******************************************************************/
extern "C" HRESULT DAPI PathCreateTempDirectory(
    __in_opt LPCWSTR wzDirectory,
    __in __format_string LPCWSTR wzDirectoryNameTemplate,
    __in DWORD dwUniqueCount,
    __out LPWSTR* psczTempDirectory
    )
{
    AssertSz(wzDirectoryNameTemplate && *wzDirectoryNameTemplate, "DirectoryNameTemplate must be specified.");
    AssertSz(0 < dwUniqueCount, "Must specify a non-zero unique count.");

    HRESULT hr = S_OK;

    LPWSTR sczTempPath = NULL;
    DWORD cchTempPath = MAX_PATH;

    LPWSTR scz = NULL;

    if (wzDirectory && *wzDirectory)
    {
        hr = StrAllocString(&sczTempPath, wzDirectory, 0);
        ExitOnFailure(hr, "Failed to copy temp path.");

        hr = PathBackslashTerminate(&sczTempPath);
        ExitOnFailure1(hr, "Failed to ensure path ends in backslash: %S", wzDirectory);
    }
    else
    {
        hr = StrAlloc(&sczTempPath, cchTempPath);
        ExitOnFailure(hr, "Failed to allocate memory for the temp path.");

        if (!::GetTempPathW(cchTempPath, sczTempPath))
        {
            ExitWithLastError(hr, "Failed to get temp path.");
        }
    }

    for (DWORD i = 1; i <= dwUniqueCount; ++i)
    {
        hr = StrAllocFormatted(&scz, wzDirectoryNameTemplate, i);
        ExitOnFailure(hr, "Failed to allocate memory for directory name template.");

        hr = StrAllocFormatted(psczTempDirectory, L"%s%s", sczTempPath, scz);
        ExitOnFailure(hr, "Failed to allocate temp directory name.");

        if (!::CreateDirectoryW(*psczTempDirectory, NULL))
        {
            DWORD er = ::GetLastError();
            if (ERROR_ALREADY_EXISTS == er)
            {
                hr = HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS);
                continue;
            }
            else if (ERROR_PATH_NOT_FOUND == er)
            {
                hr = DirEnsureExists(*psczTempDirectory, NULL);
                break;
            }
            else
            {
                hr = HRESULT_FROM_WIN32(er);
                break;
            }
        }
        else
        {
            hr = S_OK;
            break;
        }
    }
    ExitOnFailure(hr, "Failed to create temp directory.");

    hr = PathBackslashTerminate(psczTempDirectory);
    ExitOnFailure(hr, "Failed to ensure temp directory is backslash terminated.");

LExit:
    ReleaseStr(scz);
    ReleaseStr(sczTempPath);

    return hr;
}

/*******************************************************************
 PathGetKnownFolder - returns the path to a well-known shell folder

*******************************************************************/
extern "C" HRESULT DAPI PathGetKnownFolder(
    __in int csidl,
    __out LPWSTR* psczKnownFolder
    )
{
    HRESULT hr = S_OK;

    hr = StrAlloc(psczKnownFolder, MAX_PATH);
    ExitOnFailure(hr, "Failed to allocate memory for known folder.");

    hr = ::SHGetFolderPathW(NULL, csidl, NULL, SHGFP_TYPE_CURRENT, *psczKnownFolder);
    ExitOnFailure(hr, "Failed to get known folder path.");

    hr = PathBackslashTerminate(psczKnownFolder);
    ExitOnFailure(hr, "Failed to ensure known folder path is backslash terminated.");

LExit:
    return hr;
}


/*******************************************************************
 PathIsAbsolute - returns true if the path is absolute; false 
    otherwise.
*******************************************************************/
extern "C" BOOL DAPI PathIsAbsolute(
    __in_z LPCWSTR wzPath
    )
{
    DWORD dwLength = lstrlenW(wzPath);
    return (1 < dwLength) && (wzPath[0] == L'\\') || (wzPath[1] == L':');
}


/*******************************************************************
 PathConcat - like .NET's Path.Combine, lets you build up a path
    one piece -- file or directory -- at a time.

*******************************************************************/
extern "C" HRESULT DAPI PathConcat(
    __in_opt LPCWSTR wzPath1,
    __in_opt LPCWSTR wzPath2,
    __deref_out_z LPWSTR* psczCombined
    )
{
    HRESULT hr = S_OK;

    if (!wzPath2 || !*wzPath2)
    {
        hr = StrAllocString(psczCombined, wzPath1, 0);
        ExitOnFailure(hr, "Failed to copy just path1 to output.");
    }
    else if (!wzPath1 || !*wzPath1 || PathIsAbsolute(wzPath2))
    {
        hr = StrAllocString(psczCombined, wzPath2, 0);
        ExitOnFailure(hr, "Failed to copy just path2 to output.");
    }
    else
    {
        hr = StrAllocString(psczCombined, wzPath1, 0);
        ExitOnFailure(hr, "Failed to copy path1 to output.");

        hr = PathBackslashTerminate(psczCombined);
        ExitOnFailure(hr, "Failed to backslashify.");

        hr = StrAllocConcat(psczCombined, wzPath2, 0);
        ExitOnFailure(hr, "Failed to append path2 to output.");
    }

LExit:
    return hr;
}
