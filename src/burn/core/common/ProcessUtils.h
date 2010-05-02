//-------------------------------------------------------------------------------------------------
// <copyright file="ProcessUtils.h" company="Microsoft">
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
//
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once

#include "SmartLibrary.h"
#include "LogSignatureDecorator.h"
#include "LogUtils.h"
#include "Interfaces\IExceptions.h"

namespace IronMan
{

struct ProcessUtils
{
    struct CProcessInformation : public PROCESS_INFORMATION
    {
        CProcessInformation(void)
        {
            ZeroMemory(this, sizeof(PROCESS_INFORMATION));
        }
        ~CProcessInformation(void)
        {
            if (hProcess)
                ::CloseHandle(hProcess);

            if (hThread)
                ::CloseHandle(hThread);
        }
    };

    static DWORD GetParentOfProcess(DWORD dwProcess = ::GetCurrentProcessId())
    {
        return GetParentOfProcessT(dwProcess, CreateToolhelp32Snapshot, Process32First, Process32Next, CloseHandle);
    }
    template<typename F1, typename F2, typename F3, typename F4>

    static DWORD GetParentOfProcessT(DWORD dwProcess, F1 CreateToolhelp32Snapshot, F2 Process32First, F3 Process32Next, F4 CloseHandle)
    {
        DWORD dwParentProcess = 0;

        HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (hSnap != INVALID_HANDLE_VALUE)
        {
            PROCESSENTRY32 pe = {0};
            pe.dwSize = sizeof(PROCESSENTRY32);

            if (TRUE == Process32First(hSnap, &pe))
            {
                do
                {
                    if (pe.th32ProcessID == dwProcess)
                    {
                        dwParentProcess = pe.th32ParentProcessID;
                        break;
                    }
                } while (TRUE == Process32Next(hSnap, &pe));
            }
            CloseHandle(hSnap);
        }
        return dwParentProcess;
    }

    static CPath GetTempFolderOfUserOfProcess(DWORD dwProcessId)
    {
        return GetTempFolderOfUserOfProcessT(dwProcessId, OpenProcess, OpenProcessToken, ExpandEnvironmentStringsForUser, CloseHandle);
    }

    template<typename F1, typename F2, typename F3, typename F4>
    static CPath GetTempFolderOfUserOfProcessT(DWORD dwProcessId, F1 OpenProcess, F2 OpenProcessToken, F3 ExpandEnvironmentStringsForUser, F4 CloseHandle)
    {
        CString tempFolder;
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, dwProcessId);
        if (hProcess != NULL)
        {
            HANDLE hToken;
            
            if (OpenProcessToken(hProcess, TOKEN_QUERY, &hToken))
            {
                BOOL b = ExpandEnvironmentStringsForUser(hToken, L"%TEMP%", tempFolder.GetBuffer(MAX_PATH), MAX_PATH);
                tempFolder._ReleaseBuffer();
                if (!b)
                {
                    tempFolder = L"";
                }

                CloseHandle(hToken);
            }
            CloseHandle(hProcess);
        }
        return CPath(tempFolder);
    }

    static UINT GetTempFileName(
        __in LPCWSTR lpPathName,
        __in LPCWSTR lpPrefixString,
        __in UINT uUnique,
        __out_ecount(MAX_PATH) LPWSTR lpTempFileName
        )
    {
        return ::GetTempFileName(lpPathName, lpPrefixString, uUnique, lpTempFileName);
    }

    //------------------------------------------------------------------------------
    // CanFileBeCreatedAndDeletedInFolder
    //
    // Determines if a file can be created and deleted in the folder that is passed in
    //------------------------------------------------------------------------------
    static bool CanFileBeCreatedAndDeletedInFolder(CString tempFolder, CString& tempFile)
    {
        return CanFileBeCreatedAndDeletedInFolderT(tempFolder, tempFile, GetTempFileName, DeleteFile);
    }

    //------------------------------------------------------------------------------
    // CanFileBeCreatedAndDeletedInFolder
    //
    // Used only for Unit Tests
    //------------------------------------------------------------------------------
    template <typename F1, typename F2>
    static bool CanFileBeCreatedAndDeletedInFolderT(const CString tempFolder
                                                , CString& tempFile
                                                , F1 GetTempFileName
                                                , F2 DeleteFile)
    {
        tempFile = L"";
        if (0 == GetTempFileName(tempFolder, L"HFI", 0, tempFile.GetBuffer(MAX_PATH*2)))
        {
            //Note:  DevDiv Bug 152756: One of the known reason is because %TEMP% is invalid. 
            //       DevDiv Bug 152752: Another reason is because the %TEMP% is readonly
            return false;
        }
        tempFile._ReleaseBuffer();

        if (0 == DeleteFile(tempFile))
        {
            //Note:  DevDiv Bug 152752: When the temp file exists but is readonly.
            return false;
        }
        return true;
    }


    //------------------------------------------------------------------------------
    // GetTempFolderOfDelevatedUser
    //
    // Finds a temp folder where a file could be created and deleted
    //------------------------------------------------------------------------------
    static CString GetTempFolderOfDelevatedUser()
    {
        return GetTempFolderOfDelevatedUserT(
            GetUserOfProcess
            , GetParentOfProcess
            , GetTempFolderOfUserOfProcess
            , CanFileBeCreatedAndDeletedInFolder);
    }

    //------------------------------------------------------------------------------
    // GetTempFolderOfDelevatedUserT
    //
    // Used only for Unit Tests
    //------------------------------------------------------------------------------
    template <typename F1, typename F2, typename F3, typename F4>
    static CString GetTempFolderOfDelevatedUserT(
        F1 GetUserOfProcess
        , F2 GetParentOfProcess
        , F3 GetTempFolderOfUserOfProcess
        , F4 CanFileBeCreatedAndDeletedInFolder)
    {
        DWORD dwCurrentProcessId = ::GetCurrentProcessId();
        CString currentUser = GetUserOfProcess(dwCurrentProcessId);
        CString tempFolder;
        CString tempFile;

        DWORD dwParentProcessId = GetParentOfProcess(dwCurrentProcessId);
        CString parentUser = GetUserOfProcess(dwParentProcessId);

        if (currentUser.CompareNoCase(parentUser) == 0)
        {
            // In Cabbed scenario, cab itslef will be elevated. Get grand parent's temp folder.
            tempFolder = CString(GetTempFolderOfUserOfProcess(GetParentOfProcess(dwParentProcessId)));
        }
        if (tempFolder.IsEmpty() || !CanFileBeCreatedAndDeletedInFolder(tempFolder, tempFile))
        {
            // We will get into this situation when:

            // 1. In default scenario where non Adnara, min user launches current process directly,
            //    getting temp folder of parent process is enough.
            // 2. When Grandparent Process is dead (Admin launches current process directly,
            //    resulting in Explorer as Parent and some recycled system process as Grandparent.
            //    In this case, we just return Parent Process's temp folder.
            tempFolder = CString(GetTempFolderOfUserOfProcess(dwParentProcessId));
            if (tempFolder.IsEmpty() || !CanFileBeCreatedAndDeletedInFolder(tempFolder, tempFile))
            {
                // In pathological case where Parent Process temp can't be obtained, return current process temp.
                tempFolder = CString(GetTempFolderOfUserOfProcess(dwCurrentProcessId));
                if (tempFolder.IsEmpty() || !CanFileBeCreatedAndDeletedInFolder(tempFolder, tempFile))
                {
                    // Fallback to GetTempPath when nothing above works
                    GetTempPath(MAX_PATH, tempFolder.GetBuffer(MAX_PATH));
                    tempFolder._ReleaseBuffer();
                    if (!tempFolder.IsEmpty() && !CanFileBeCreatedAndDeletedInFolder(tempFolder, tempFile))
                    {
                        tempFolder.Truncate(0);
                    }
                }
            }
        }
        return tempFolder;
    }

    static CString GetUserOfProcess(DWORD dwProcessId)
    {
        return GetUserOfProcessT(dwProcessId, OpenProcess, OpenProcessToken, GetTokenInformation, LookupAccountSid, CloseHandle);
    }

    template <typename F1, typename F2, typename F3, typename F4, typename F5>
    static CString GetUserOfProcessT(DWORD dwProcessId, F1 OpenProcess, F2 OpenProcessToken, F3 GetTokenInformation, F4 LookupAccountSid, F5 CloseHandle)
    {
        CString userName, userDomain;

        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, dwProcessId);
        if (hProcess != NULL)
        {
            HANDLE hToken;
            if (OpenProcessToken(hProcess, TOKEN_QUERY, &hToken))
            {
                DWORD dwSize = 0;

                //Condition updated to fix prefast warning (26017)
                if (!GetTokenInformation(hToken, TokenUser, NULL, dwSize, &dwSize) && dwSize >= sizeof(TOKEN_USER))
                {
                    CStringA buffer;
                    PTOKEN_USER pUserInfo = reinterpret_cast<PTOKEN_USER>(buffer.GetBuffer(dwSize));

                    if (GetTokenInformation(hToken, TokenUser, pUserInfo, dwSize, &dwSize))
                    {
                        SID_NAME_USE sidType;
                        dwSize = MAX_PATH;

                        LookupAccountSid(NULL, pUserInfo->User.Sid, userName.GetBuffer(dwSize), &dwSize, userDomain.GetBuffer(dwSize), &dwSize, &sidType);
                        userName._ReleaseBuffer();
                        userDomain._ReleaseBuffer();
                    }
                    buffer._ReleaseBuffer();
                }
                CloseHandle(hToken);
            }
            CloseHandle(hProcess);
        }

        CString user = userDomain + L"\\"+ userName;

        return (user.GetLength() > 1) ? user : L"";
    }

    static const CString GetImageName( DWORD processID, ILogger& logger )
    {
        return GetImageNameT(processID, logger, OpenProcess
            , EnumProcessModules, GetModuleBaseName);
    }
    template <typename F1, typename F2, typename F3>
    static const CString GetImageNameT( DWORD processID, ILogger& logger, F1 OpenProcess
        , F2 EnumProcessModules, F3 GetModuleBaseName)
    {
        CString imageName;

        // Get a handle to the process.
        HANDLE hProcess = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
                                       FALSE, processID );
        // Get the process name.
        if (NULL != hProcess )
        {
            HMODULE hMod;
            DWORD cbNeeded;

            // The first module returned by EnumProcessModules is the executable file
            // Need to call this because GetProcessImageFileName is not available on Win2k
            // but EnumProcessModules doese not work if calling app is 32 bit and process is 64-bit
            if ( EnumProcessModules( hProcess, &hMod, sizeof(hMod), &cbNeeded) )
            {
                if ( 0 == GetModuleBaseName( hProcess, hMod, imageName.GetBuffer(MAX_PATH), MAX_PATH) )
                {
                    LOGGETLASTERROR(logger, ILogger::Debug, L"GetModuleBaseName");
                }
                imageName._ReleaseBuffer();
            }
            else
            {
                CString message;
                message.Format(L"EnumProcessModules failed with error %u, will try GetProcessImageFileName", GetLastError() );
                LOG(logger, ILogger::Debug, message);               
                try
                {
                    SmartLibrary psapi(L"psapi.dll");
                    // NOT AVAILABLE ON WIN2K GetProcessImageFileName
                    try
                    {
                        typedef DWORD (WINAPI *GETPROCESSIMAGEFILENAME)(HANDLE, LPTSTR, DWORD);                 
                        GETPROCESSIMAGEFILENAME GetProcessImageFileName = 
                            psapi.GetProcAddress<GETPROCESSIMAGEFILENAME>("GetProcessImageFileNameW");
                        if ( 0 == (GetProcessImageFileName)(hProcess, imageName.GetBuffer(MAX_PATH), MAX_PATH) )
                        {
                            LOGGETLASTERROR(logger, ILogger::Debug, L"GetProcessImageFileName");
                        }
                        else
                        {
                            imageName._ReleaseBuffer();
                            PathStripPath(imageName.GetBuffer());
                        }
                        imageName._ReleaseBuffer();
                    }
                    catch(const IronMan::CWinAPIException& e)
                    {
                        LOG(logger, ILogger::Debug, e.GetMessage() );
                    }
                }
                catch(const IronMan::CWinAPIException& e)
                {
                    LOG(logger, ILogger::Debug, e.GetMessage() );
                }
            }
            CloseHandle( hProcess );
        }
        else
        {
            LOGGETLASTERROR(logger, ILogger::Debug, L"OpenProcess");
        }
        return imageName;
    }

    // Returns the owner process ID of the given thread ID
    static DWORD GetProcessIdOfThread(DWORD threadID, ILogger& logger)
    {
        HANDLE hThreadSnap = ::CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
        if (hThreadSnap == INVALID_HANDLE_VALUE)
            return -1;

        THREADENTRY32 te32 = {0};
        te32.dwSize = sizeof(THREADENTRY32);

        if (Thread32First(hThreadSnap, &te32))
        {
            do
            {
                if (te32.th32ThreadID == threadID)
                {
                    return te32.th32OwnerProcessID; 
                }

                te32.dwSize = sizeof(THREADENTRY32);
            } while(Thread32Next(hThreadSnap, &te32));
        }

        return -1;
    }

#ifdef DEBUG
    // Returns the process tree in the form
    //      WmiPrvSE.exe <-- svchost.exe <-- services.exe <-- wininit.exe
    static void GetProcessTree(DWORD pid, CString& processTree, ILogger& logger)
    {
        struct GetProcessInfo
        {
            void operator() (HANDLE hProcessSnap, DWORD pid, DWORD& pidParent, CString& strCurrProcessExeFile, ILogger& logger)
            {
                pidParent = 0;
                PROCESSENTRY32 pe32 = {0};
                pe32.dwSize = sizeof(PROCESSENTRY32);

                BOOL bSucceded = Process32First(hProcessSnap, &pe32);
                if (!bSucceded && ::GetLastError() != ERROR_NO_MORE_FILES)
                {
                    LOGEX(logger, ILogger::Warning, L"Process32First failed with error %d", ::GetLastError());
                }
                else
                {
                    do
                    {
                        if (pe32.th32ProcessID == pid)
                        {
                            strCurrProcessExeFile = pe32.szExeFile;
                            pidParent = pe32.th32ParentProcessID;
                            break;
                        }

                        bSucceded = Process32Next(hProcessSnap, &pe32);
                        if (!bSucceded && ::GetLastError() != ERROR_NO_MORE_FILES)
                        {
                            LOGEX(logger, ILogger::Warning, L"Process32First failed with error %d", ::GetLastError());
                        }

                    } while(bSucceded);
                }
            }
        } getProcessInfo;

        processTree.Format(L"Process ID: %d", pid);

        HANDLE hProcessSnap = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (hProcessSnap == INVALID_HANDLE_VALUE)
        {
            LOGEX(logger, ILogger::Warning, L"CreateToolhelp32Snapshot failed with error %d", ::GetLastError());
            return;
        }         
        const DWORD maxAncestorLevel = 6;
        for (DWORD pidCurr=pid, pidParent=-1, ancestorLevel = 0; 
             pidParent != 0 && ancestorLevel < maxAncestorLevel; 
             pidCurr = pidParent, ++ancestorLevel)
        {
            CString strCurrProcessExeFile;
            getProcessInfo(hProcessSnap, pidCurr, pidParent, strCurrProcessExeFile, logger);

            if (CString::StringLength(strCurrProcessExeFile) > 0)
            {
                if (pidCurr == pid)
                    processTree = strCurrProcessExeFile;
                else if (CString::StringLength(strCurrProcessExeFile) > 0)
                    processTree += L" <-- " + strCurrProcessExeFile;

                CString strPIDCurr;
                strPIDCurr.Format(L"(%d)", pidCurr);
                processTree += strPIDCurr;
            }
        }
    }
#endif // DEBUG

};
}
