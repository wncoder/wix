//-------------------------------------------------------------------------------------------------
// <copyright file="FileCompression.h" company="Microsoft">
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
//     For uncompressing a zip file during downloading
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once

#include "Interfaces\ILogger.h"
#include "common\MsgWaitForObject.h"
#include "common\ProcessUtils.h"
#include "LogSignatureDecorator.h"

namespace IronMan
{

// Manages compressed file's Decompression action. After Decompression, 
// FileExists() can be used to find specific files in the decompressed folder.
class FileCompression
{
private:
    ILogger& m_logger;
    CString m_sourceFile;
    CString m_destinationFolder;

public:

    // Constructor: 
    // Input: 'file' to Decompress
    FileCompression(const CString& file, ILogger& logger) 
        : m_sourceFile(file)
        , m_logger(logger)
        , m_destinationFolder(L"")
    {
    }

    // Destrcutor:
    // If exists, the compressed file is deleted.
    // If exists, the folder to which the file was decompressed, is cleanedup and deleted.
    virtual ~FileCompression(void)
    {
        // Delete compressed file
        CPath sourceFile(m_sourceFile);
        if (sourceFile.FileExists())
        {
            DeleteFile(m_sourceFile);
            m_sourceFile = L"";
        }

        // Empty and Delete extracted folder.
        CPath extractedFolder(m_destinationFolder);
        if (extractedFolder.IsDirectory() && extractedFolder.FileExists())
        {
            DWORD dwFolderLength = m_destinationFolder.GetLength();
            LPWSTR pszBuffer = m_destinationFolder.GetBuffer(dwFolderLength + 1);
            pszBuffer[dwFolderLength + 1] = 0;
            // Build the details about the operation to perform
            SHFILEOPSTRUCT fileOperation = {0};
            fileOperation.hwnd = NULL;
            fileOperation.wFunc = FO_DELETE;
            fileOperation.pFrom = pszBuffer;
            fileOperation.pTo = NULL;
            fileOperation.fFlags = FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT;

            // Execute the operation to delete the folder.
            DWORD result = SHFileOperation(&fileOperation);
            m_destinationFolder._ReleaseBuffer();
            if (result != 0)
            {
                CString resultString;
                resultString.Format(L"Cleanup of temporary folder %s returned: %d", m_destinationFolder, result);
                LOG(m_logger, ILogger::Warning, resultString);
            }
            m_destinationFolder = L"";
        }
    }

    // Decompresses the file into given 'destinationFolder'
    // This is a blocking call, till decompress process is finished.
    // Returns S_OK on success.
    virtual HRESULT Decompress(const CString& destinationFolder)
    {
        // Note - if need be, this method should be made reentrant
        m_destinationFolder = destinationFolder;
        return DecompressWithWait();
    }

    // Takes the filename part from file and checks to see if it exists in the decompressed folder. 
    // Returns false if file doesn't exist. 
    // 'decompressedFile' contains full path if it exists and empty string if it does not.
    virtual bool FileExists(const CString& file, CString& decompressedFile)
    {
        CPath filePath(m_destinationFolder);
        CPath fileName(file);
        fileName.StripPath();
        filePath.Append(fileName);
        if (filePath.FileExists())
        {
            decompressedFile = CString(filePath);
            return true;
        }
        decompressedFile.Empty();
        return false;
    }

private:
    
    // Returns the path to the source file to decompress
    virtual const CPath GetExecutable() const
    {
        return CPath(m_sourceFile);
    }

    // Returns commandline to decomopress. Works for SFX and Box packages.
    virtual CString GetCommandLine() const
    {
        CString commandLine;

        CPath destinationFolderPath(m_destinationFolder);
        destinationFolderPath.QuoteSpaces();

        commandLine.Format(L"/Q /X:%s", destinationFolderPath);
        return commandLine;
    }

    // Decompresses the file into given 'destinationFolder'
    // This is a blocking call, till decompress process is finished.
    // Returns S_OK on success.
    virtual HRESULT DecompressWithWait()
    {
        STARTUPINFO startupInfo;
        ::ZeroMemory(&startupInfo, sizeof(STARTUPINFO));
        startupInfo.cb = sizeof( STARTUPINFO );
        ProcessUtils::CProcessInformation processInformation;

        const CPath& exeFullName = GetExecutable();

        CPath workingDir(exeFullName);
        workingDir.RemoveFileSpec();

        CPath exeName(exeFullName);
        exeName.StripPath();
        exeName.QuoteSpaces();

        CString commandLine;
        commandLine.Format(L"%s %s", exeName, GetCommandLine());

        LOG(m_logger, ILogger::Verbose, L"Launching CreateProcess to Decompress: " + commandLine);

        bool bCreateProcessSucceded = CreateProcess(exeFullName,
                                                      commandLine.GetBuffer(),
                                                      NULL, // lpProcessAttributes
                                                      NULL, // lpThreadAttributes
                                                      TRUE, // bInheritHandles
                                                      CREATE_NO_WINDOW, // dwCreationFlags
                                                      NULL, // lpEnvironment
                                                      workingDir,
                                                      &startupInfo,
                                                      &processInformation);
        if (bCreateProcessSucceded == false)
        {   
            DWORD err = GetLastError();
            CComBSTR errMsg;
            CSystemUtil::GetErrorString(err, errMsg);
            LOG(m_logger, ILogger::Error, L"Error launching CreateProcess with command line = " + commandLine + errMsg);
            LOG(m_logger, ILogger::Error, L"      CreateProcess returned error = " + CString(errMsg));

            return HRESULT_FROM_WIN32(err);
        }

        // This is a blocking call and gets signalled at regular interval (100 ms).
        // This Waits on the process, while allowing sigmoidal progress to continue.
        MsgWaitForObject::Wait(processInformation.hProcess); 

        DWORD exitCode = 0;
        GetExitCodeProcess(processInformation.hProcess, &exitCode);
        
        CString resultString;
        if (0 != exitCode)
        {
            resultString.Format(L"%s %d", L"Decompression completed with code:",  exitCode);
            LOG(m_logger, ILogger::Warning, resultString);
            return (HRESULT_FROM_WIN32(exitCode));
        }
        else
        {
            resultString.Format(L"%s %d", L"Decompression successfully completed with code:",  exitCode);
            LOG(m_logger, ILogger::Verbose, resultString);
        }

        return S_OK;
    }


private: // test sub-class test hook
    virtual BOOL CreateProcess(
            __in_opt    LPCTSTR lpApplicationName,
            __inout_opt LPTSTR lpCommandLine,
            __in_opt    LPSECURITY_ATTRIBUTES lpProcessAttributes,
            __in_opt    LPSECURITY_ATTRIBUTES lpThreadAttributes,
            __in        BOOL bInheritHandles,
            __in        DWORD dwCreationFlags,
            __in_opt    LPVOID lpEnvironment,
            __in_opt    LPCTSTR lpCurrentDirectory,
            __in        LPSTARTUPINFOW lpStartupInfo,
            __out       LPPROCESS_INFORMATION lpProcessInformation)
    {
        // Process handles closed in ~CProcessInformation()
#pragma warning (push)
#pragma warning( disable:25028 )
        return ::CreateProcess(
            lpApplicationName,
            lpCommandLine,
            lpProcessAttributes,
            lpThreadAttributes,
            bInheritHandles,
            dwCreationFlags,
            lpEnvironment,
            lpCurrentDirectory,
            lpStartupInfo,
            lpProcessInformation);
#pragma warning (pop)
    }

    virtual BOOL DeleteFile(
        __in LPCWSTR lpFileName
        )
    {
        return ::DeleteFile(lpFileName);
    }

    virtual BOOL GetExitCodeProcess(
        __in  HANDLE hProcess,
        __out LPDWORD lpExitCode)
    {
        return ::GetExitCodeProcess(hProcess, lpExitCode);
    }
};

} // IronMan
