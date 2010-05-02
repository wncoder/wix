//-------------------------------------------------------------------------------------------------
// <copyright file="RebootManager.h" company="Microsoft">
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

namespace IronMan 
{

class RebootManager
{
public:
    virtual ~RebootManager() {}

    void Reboot()
    {
        // copy 'n' pasted from setup\vssetup\setupexe\cartman\setupexe\main.cpp,
        // and then fixed.  Sheesh.
        OSVERSIONINFO ov = {0};
        ov.dwOSVersionInfoSize = sizeof(ov);

        if (GetVersionEx(&ov) && (ov.dwPlatformId == VER_PLATFORM_WIN32_NT))
        {
            HANDLE htoken;
            if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &htoken))
            {
                LUID luid;
                LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &luid);
                TOKEN_PRIVILEGES privs;
                privs.PrivilegeCount = 1;
                privs.Privileges[0].Luid = luid;
                privs.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
                AdjustTokenPrivileges(htoken, FALSE, &privs, 0, NULL, 0);
                CloseHandle(htoken);
            } 
            //on Whistler and future NT use InitiateSystemShutdownEx to avoid unexpected reboot message box
            CString csEmpty(_T(""));
            if ( (ov.dwMajorVersion > 5) || ( (ov.dwMajorVersion == 5) && (ov.dwMinorVersion  > 0) ))
            {
                InitiateSystemShutdownEx(0, csEmpty.GetBuffer(), 0, false, true, REASON_PLANNED_FLAG);
            }
            else
            {
                InitiateSystemShutdown(0, csEmpty.GetBuffer(), 0, false, true);
            }
        }
        else
        {
            ExitWindowsEx(EWX_REBOOT, 0);
        }
    }

private: // "subclass 'n' override" testhooks
    virtual BOOL GetVersionEx(__inout LPOSVERSIONINFOW lpVersionInformation) { return ::GetVersionEx(lpVersionInformation); }
    virtual BOOL OpenProcessToken(__in HANDLE ProcessHandle, __in DWORD DesiredAccess, __deref_out PHANDLE TokenHandle) { return ::OpenProcessToken(ProcessHandle, DesiredAccess, TokenHandle); }
    virtual BOOL LookupPrivilegeValue(__in_opt LPCWSTR lpSystemName, __in  LPCTSTR lpName, __out PLUID lpLuid) { return ::LookupPrivilegeValue(lpSystemName, lpName, lpLuid); }
    virtual BOOL AdjustTokenPrivileges(__in HANDLE TokenHandle, __in BOOL DisableAllPrivileges, __in_opt PTOKEN_PRIVILEGES NewState, __in DWORD BufferLength, __out_bcount_part_opt(BufferLength, *ReturnLength) PTOKEN_PRIVILEGES PreviousState, __out_opt PDWORD ReturnLength) { return ::AdjustTokenPrivileges(TokenHandle, DisableAllPrivileges, NewState, BufferLength, PreviousState, ReturnLength); }
    virtual BOOL CloseHandle(__in HANDLE hObject) { return ::CloseHandle(hObject); }
    __pragma ( prefast(suppress: 25027, "unsafe API, supposedly, but not in .xls") )
    virtual BOOL InitiateSystemShutdown(__in_opt LPTSTR lpMachineName, __in_opt LPTSTR lpMessage, __in DWORD dwTimeout, __in BOOL bForceAppsClosed, __in BOOL bRebootAfterShutdown) { return ::InitiateSystemShutdown(lpMachineName, lpMessage, dwTimeout, bForceAppsClosed, bRebootAfterShutdown); }
    __pragma ( prefast(suppress: 25027, "unsafe API, supposedly, but not in .xls") )
    virtual BOOL InitiateSystemShutdownEx(__in_opt LPTSTR lpMachineName, __in_opt LPTSTR lpMessage, __in DWORD dwTimeout, __in BOOL bForceAppsClosed, __in BOOL bRebootAfterShutdown, __in DWORD dwReason) { return ::InitiateSystemShutdownEx(lpMachineName, lpMessage, dwTimeout, bForceAppsClosed, bRebootAfterShutdown, dwReason); }
    __pragma ( prefast(suppress: 25027, "unsafe API, supposedly, but not in .xls") )
    virtual BOOL ExitWindowsEx(__in UINT uFlags, __in DWORD dwReason) { return ::ExitWindowsEx(uFlags, dwReason); }
};

} // namespace IronMan 
