//-------------------------------------------------------------------------------------------------
// <copyright file="MSUInstaller.h" company="Microsoft">
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

#include "schema\EngineData.h"
#include "Interfaces\IPerformer.h"
#include "Interfaces\ILogger.h"
#include "Ux\Ux.h"
#include "ExeInstaller.h"

namespace IronMan
{
// Helper class to start and, if necessary, stop the WU Service around calls to WUSA.exe
class MsuWUServiceHelper
{
protected:
    bool m_bServiceShouldBeDisabledInDestructor;
    DWORD m_dwPreviousState;
    ILogger& m_logger;
    const WCHAR *m_pszSvcName;
    enum eActionRequired {None, Stop, Pause} m_actionRequired;
public:
    // Constructor
    MsuWUServiceHelper(ILogger& logger)
        :
         m_bServiceShouldBeDisabledInDestructor(false)
        ,m_dwPreviousState(SERVICE_RUNNING)
        ,m_logger(logger)
        ,m_pszSvcName(L"wuauserv")
        ,m_actionRequired(None)
    {
    }

    // Ensure WU service is not disabled. The destructor will restore the previous state
    // Save the state it had before we start if, if necessary
    virtual void EnsureWUServiceIsNotDisabled(void)
    {
        SC_HANDLE schSCManager = {0};
        SC_HANDLE schService = {0};
        LPQUERY_SERVICE_CONFIG lpsc = NULL; 
        DWORD dwBytesNeeded = 0;
        DWORD cbBufSize = 0;
        DWORD dwError = 0;
        CString strFailingOperation;

        // Get a handle to the SCM database. 
        schSCManager = OpenSCManager( 
            NULL,                    // local computer
            NULL,                    // ServicesActive database 
            SC_MANAGER_ALL_ACCESS);  // full access rights 

        if (schSCManager != NULL) 
        {
            // Get a handle to the service.
            schService = OpenService( 
                schSCManager,            // SCM database 
                m_pszSvcName,               // name of service 
                SERVICE_QUERY_CONFIG | SERVICE_CHANGE_CONFIG | SERVICE_QUERY_STATUS );   // need query config access 

            if (schService != NULL)
            {
                SERVICE_STATUS serviceStatus = {0};
                if ( QueryServiceStatus(schService, &serviceStatus) )
                {
                    m_dwPreviousState = serviceStatus.dwCurrentState;
                }
                else
                {
                    strFailingOperation = CString(L"QueryServiceStatus");
                }

                // Query service config
                if( !QueryServiceConfig(schService,NULL,0,&dwBytesNeeded) )
                {
                    dwError = GetLastError();
                    if( ERROR_INSUFFICIENT_BUFFER == dwError )
                    {
                        cbBufSize = dwBytesNeeded;
                        lpsc = (LPQUERY_SERVICE_CONFIG) LocalAlloc(LMEM_FIXED, cbBufSize);

                        if( QueryServiceConfig(schService,lpsc,cbBufSize,&dwBytesNeeded) ) 
                        {
                            // If the service is already running do nothing, even if it is marked Disabled
                            if (lpsc->dwStartType == SERVICE_DISABLED  &&  m_dwPreviousState != SERVICE_RUNNING)
                            {
                                LOG(m_logger, ILogger::Verbose, L"WU Service was disabled");

                                // Enable WU service
                                if ( ChangeServiceConfig( 
                                    schService,            // handle of service 
                                    SERVICE_NO_CHANGE,     // service type: no change 
                                    SERVICE_DEMAND_START,  // service start type 
                                    SERVICE_NO_CHANGE,     // error control: no change 
                                    NULL,                  // binary path: no change 
                                    NULL,                  // load order group: no change 
                                    NULL,                  // tag ID: no change 
                                    NULL,                  // dependencies: no change 
                                    NULL,                  // account name: no change 
                                    NULL,                  // password: no change 
                                    NULL) )                // display name: no change
                                {
                                    m_bServiceShouldBeDisabledInDestructor = true;
                                    LOG(m_logger, ILogger::Verbose, L"WU Service enabled successfully prior to invoking MSU performer"); 
                                }
                                else
                                {
                                    strFailingOperation = CString(L"ChangeServiceConfig");
                                }
                            }
                            else
                            {
                                LOG(m_logger, ILogger::Verbose, L"WU Service was already enabled or was running"); 
                            }
                        }
                        else
                        {
                            strFailingOperation = CString(L"QueryServiceConfig");
                        }

                        // Free the memory when finished with it
                        LocalFree(lpsc);
                    }
                }
            }
            else
            { 
                strFailingOperation = CString(L"OpenService");
            }

        }
        else
        {
            strFailingOperation = CString(L"OpenSCManager");
        }

        // Log the result
        LogMessageAndCloseHandles(L"EnsureWUServiceIsNotDisabled",strFailingOperation,schSCManager,schService);
    }

    // Log a success or failure message at Verbose level and close any open handles
    void LogMessageAndCloseHandles(const CString   &strTask,
                                   const CString   &strFailingOperation,
                                   const SC_HANDLE &schSCManager,
                                   const SC_HANDLE &schService)
    {
        CString strMessage;

        if (!strFailingOperation.IsEmpty())
        {
            // Get failure details
            DWORD dwError = GetLastError();
            CComBSTR bstrSystemError;
            CSystemUtil::GetErrorString(dwError,bstrSystemError);
            strMessage.Format( L"WU Service: Trying to %s function %s failed with error: %u", strTask, strFailingOperation, GetLastError(),bstrSystemError);
        }
        else
        {
            // Success
            strMessage.Format( L"WU Service: %s succeeded", strTask);
        }

        // Write message to log
        LOG(m_logger, ILogger::Verbose, strMessage); 

        // Close any open handles
        if (schService)
        {
            CloseServiceHandle(schService);
        }

        if (schSCManager)
        {
            CloseServiceHandle(schSCManager);
        }
    }

    // Destructor which will put the service back in the state it had prior to invocation of the constructor
    virtual ~MsuWUServiceHelper()
    {
        if (m_bServiceShouldBeDisabledInDestructor)
        {
            DisableWUService();

            if (m_dwPreviousState == SERVICE_STOPPED  ||  m_dwPreviousState == SERVICE_STOP_PENDING)
            {
                StopWUService();
            }
            else if (m_dwPreviousState == SERVICE_PAUSED  ||  m_dwPreviousState == SERVICE_PAUSE_PENDING)
            {
                PauseWUService();
            }
        }
    }

protected:

    // Stops the WU service
    virtual void StopWUService()
    {
        ControlWUService(Stop);
    }

    // Pauses the WU service
    virtual void PauseWUService()
    {
        ControlWUService(Pause);
    }


    // Disable the WU service
    virtual void DisableWUService()
    {
        SC_HANDLE schSCManager = {0};
        SC_HANDLE schService = {0};
        CString strFailingOperation;

        // Get a handle to the SCM database. 
        schSCManager = OpenSCManager( 
            NULL,                    // local computer
            NULL,                    // ServicesActive database 
            SC_MANAGER_ALL_ACCESS);  // full access rights 

        if (NULL == schSCManager) 
        {
            strFailingOperation = CString(L"OpenSCManager");
        }
        else
        {
            // Get a handle to the service.
            schService = OpenService( 
                schSCManager,            // SCM database 
                m_pszSvcName,               // name of service 
                SERVICE_CHANGE_CONFIG);  // need change config access 

            if (schService == NULL)
            { 
                strFailingOperation = CString(L"OpenService");
            }
            else
            {
                // Change the service start type.

                if (ChangeServiceConfig( 
                    schService,        // handle of service 
                    SERVICE_NO_CHANGE, // service type: no change 
                    SERVICE_DISABLED,  // service start type 
                    SERVICE_NO_CHANGE, // error control: no change 
                    NULL,              // binary path: no change 
                    NULL,              // load order group: no change 
                    NULL,              // tag ID: no change 
                    NULL,              // dependencies: no change 
                    NULL,              // account name: no change 
                    NULL,              // password: no change 
                    NULL) )            // display name: no change
                {
                    LOG(m_logger, ILogger::Verbose, L"WU Service successfully returned to disabled state after invoking MSU performer"); 
                }
                else
                {
                    strFailingOperation = CString(L"ChangeServiceConfig");
                }
            }
        }

        // Log the result
        LogMessageAndCloseHandles(L"DisableWUService",strFailingOperation,schSCManager,schService);
    }

    // Pauses the WU service
    virtual void ControlWUService(eActionRequired action)
    {
        CString strFailingOperation;
        CString strTask;
        DWORD dwDesiredServiceStatus = 0;

        if (action == Stop)
        {
            dwDesiredServiceStatus = SERVICE_CONTROL_STOP;
            strTask = CString(L"StopWUService");
        }
        else if (action == Pause)
        {
            dwDesiredServiceStatus = SERVICE_CONTROL_PAUSE;
            strTask = CString(L"PauseWUService");
        }
        else
        {
            IMASSERT2((action == Stop  ||  action == Pause), L"Invalid action requested in call to ControlWUService");
            return;
        }

        SC_HANDLE schSCManager = {0};
        SC_HANDLE schService = {0};

        // Get a handle to the SCM database. 
        schSCManager = OpenSCManager( 
            NULL,                    // local computer
            NULL,                    // ServicesActive database 
            SC_MANAGER_ALL_ACCESS);  // full access rights 

        if (NULL == schSCManager) 
        {
            strFailingOperation = CString(L"OpenSCManager");
        }
        else
        {
            // Get a handle to the service.
            schService = OpenService( 
                schSCManager,                            // SCM database 
                m_pszSvcName,                            // name of service 
                SERVICE_PAUSE_CONTINUE | SERVICE_STOP);  // appropriate access 
            if (schService == NULL)
            { 
                strFailingOperation = CString(L"OpenService");
            }
            else
            {
                // Pause the service
                SERVICE_STATUS svsResultantServiceStatus = {0};
                if (!ControlService(schService, dwDesiredServiceStatus, &svsResultantServiceStatus))
                {
                    strFailingOperation = CString(L"ControlService");
                }
            }
        }

        // Log the result
        LogMessageAndCloseHandles(strTask,strFailingOperation,schSCManager,schService);
    }

    // For testing purpose - to be overridden in unit tests.
private:
    virtual BOOL WINAPI ChangeServiceConfig(__in SC_HANDLE hService,__in DWORD dwServiceType,__in DWORD dwStartType,__in DWORD dwErrorControl,__in_opt LPCWSTR lpBinaryPathName,__in_opt LPCWSTR lpLoadOrderGroup,__out_opt LPDWORD lpdwTagId,__in_opt LPCWSTR lpDependencies,__in_opt LPCWSTR lpServiceStartName,__in_opt LPCWSTR lpPassword,__in_opt LPCWSTR lpDisplayName)
    {
        return ::ChangeServiceConfig(hService,dwServiceType,dwStartType,dwErrorControl,lpBinaryPathName,lpLoadOrderGroup,lpdwTagId,lpDependencies,lpServiceStartName,lpPassword,lpDisplayName);
    }

    virtual BOOL WINAPI QueryServiceConfig(__in SC_HANDLE hService,__out_bcount_opt(cbBufSize) LPQUERY_SERVICE_CONFIGW lpServiceConfig,__in DWORD cbBufSize,__out LPDWORD pcbBytesNeeded)
    {
        return ::QueryServiceConfig(hService,lpServiceConfig,cbBufSize,pcbBytesNeeded);
    }

    virtual SC_HANDLE OpenService(__in SC_HANDLE hSCManager, __in LPCWSTR lpServiceName, __in DWORD dwDesiredAccess)
    {
        return ::OpenService(hSCManager, lpServiceName, dwDesiredAccess);
    }

    virtual bool QueryServiceStatus(__in SC_HANDLE hService, __out LPSERVICE_STATUS lpServiceStatus)
    {
        return !!::QueryServiceStatus(hService, lpServiceStatus);
    }

    virtual bool CloseServiceHandle(__in SC_HANDLE   hSCObject)
    {
        if (hSCObject)
            return ::CloseServiceHandle(hSCObject);
        return true;
    }

    virtual bool ControlService(__in SC_HANDLE hService,__in DWORD dwControl,__out LPSERVICE_STATUS lpServiceStatus)
    {
        return !!::ControlService(hService, dwControl, lpServiceStatus);
    }

    virtual bool StartService(__in SC_HANDLE hService,__in DWORD dwNumServiceArgs,__in_ecount_opt(dwNumServiceArgs) LPCWSTR *lpServiceArgVectors)
    {
        return !!::StartService(hService, dwNumServiceArgs, lpServiceArgVectors);
    }

};

class MsuInstaller : public ExeInstallerBase
{
private:
    CString m_commandLine;
public:
    // Constructor
    MsuInstaller(const ExeBase& exe,  ILogger& logger, Ux& uxLogger) 
        : ExeInstallerBase(exe, IProgressObserver::Installing, logger, uxLogger)
    {
        m_commandLine.Format(L"\"%s\" /quiet /norestart",GetExe().GetName());
    }

    // Destructor
    virtual ~MsuInstaller(void)
    {
    }

private:
    // Process exit code and get appropriate HRESULT
    // For MSUs, no translation of the exit code for MSUs. 
    // Also, exitCode for WUSA.Exe is already an HRESULT. So, no need to convert.
    // This is only true for Vista + Update, KB 949545. Before that, WUSA.exe returns S_FALSE
    // when .msu doesn't apply/already installed.
    // In this case also, we are just returning the truncated return code.
    virtual HRESULT ProcessReturnCode(DWORD& exitCode)
    {
        return exitCode;
    }

private:
    // Invoke the base class performer directly here to simplify ensuring that the WU Service is in the correct state
    virtual void PerformAction(IProgressObserver& observer)
    {
        // This class ensures the service is started, and turns it off again in the destructor if it was off to start with
        MsuWUServiceHelper setCorrectServiceStateBeforeAndAfter(GetLogger());
        setCorrectServiceStateBeforeAndAfter.EnsureWUServiceIsNotDisabled();

        // Perform the action, which is to invoke WUSA.exe
        ExeInstallerBase::PerformAction(observer);
    }

    // Make the GetCommandLine virtual so we can override it for testing
    virtual CString GetCommandLine() const
    {        
        return m_commandLine;
    }

    // Rollback
    virtual void Rollback(IProgressObserver& observer) 
    {   
    }

    // Make the GetExecutable virtual so we can override it for testing
    virtual const CPath GetExecutable() const
    {
        const CString wusaLocation = L"%windir%\\system32\\wusa.exe";
        CString wusaLocationExpanded; 
        CSystemUtil::ExpandEnvironmentVariables(wusaLocation, wusaLocationExpanded);
        return CPath(wusaLocationExpanded);
    }
};

class MsuUninstaller : public ExeInstallerBase
{
    ILogger& m_logger;
public:
    MsuUninstaller(const ExeBase& exe,  ILogger& logger, Ux& uxLogger) 
        : ExeInstallerBase(exe, IProgressObserver::Uninstalling, logger, uxLogger)
        , m_logger(logger)
    {}
    virtual ~MsuUninstaller(void) {}

protected:
    // No translation of the exit code for MSUs
    virtual void EnsureExitCodeIsAnMSIErrorCode(DWORD& exitCode)
    {
    }
private:    
    virtual void PerformAction(IProgressObserver& observer)
    {
        CPath msu(GetExe().GetName());
        msu.StripPath();
        CString warning;
        warning.Format(L"%s is an MSU package. MSU packages do not support the Uninstall operation", msu.m_strPath);
            LOG(m_logger, ILogger::Warning, warning);
        IMASSERT(warning);
    }

    virtual CString GetCommandLine() const
    {
        return GetUnInstallCommandLine();
    }

    virtual void Rollback(IProgressObserver& observer) 
    {
        CPath msu(GetExe().GetName());
        msu.StripPath();
        CString warning;
        warning.Format(L"%s is an MSU package. MSU packages do not support rollback", msu.m_strPath);
            LOG(m_logger, ILogger::Warning, warning);
    }	

    virtual const CPath GetExecutable() const
    {
        return GetExe().GetName();
    }
};


class MsuRepairer : public ExeInstallerBase
{
    ILogger& m_logger;
public:
    MsuRepairer(const ExeBase& exe,  ILogger& logger, Ux& uxLogger) 
        : ExeInstallerBase(exe, IProgressObserver::Repairing, logger, uxLogger)
        , m_logger(logger)
    {}
    virtual ~MsuRepairer(void) {}

protected:
    // No translation of the exit code for MSUs
    virtual void EnsureExitCodeIsAnMSIErrorCode(DWORD& exitCode)
    {
    }

private:    
    virtual void PerformAction(IProgressObserver& observer)
    {
        CPath msu(GetExe().GetName());
        msu.StripPath();
        CString warning;
        warning.Format(L"%s is an MSU package. MSU packages do not support the Repair operation", msu.m_strPath);
            LOG(m_logger, ILogger::Warning, warning);
        IMASSERT(warning);
    }

    virtual CString GetCommandLine() const
    {
        return GetUnInstallCommandLine();
    }

    virtual void Rollback(IProgressObserver& observer) 
    {}	

    virtual const CPath GetExecutable() const
    {
        return GetExe().GetName();
    }
};
} // namespace IronMan
