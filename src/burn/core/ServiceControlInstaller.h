//-------------------------------------------------------------------------------------------------
// <copyright file="ServiceControlInstaller.h" company="Microsoft">
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

namespace IronMan
{

class ServiceControlInstaller : public AbortPerformerBase
{
private:
    const CString m_name;
    const CString m_controlString;
    const DWORD m_controlAccess;
    const DWORD m_serviceControl;
    const DWORD m_desiredStatus;
    ILogger& m_logger;
    Ux& m_uxLogger;
    DWORD m_dwRetryCount;

public:
    // Main constructor
    ServiceControlInstaller(const ServiceControl &sc, ILogger& logger, Ux& uxLogger)
        : m_name(sc.GetServiceName())
        , m_controlString(GetControlString(sc.GetServiceControl()))
        , m_controlAccess(GetControlAccessMask(sc.GetServiceControl()))
        , m_serviceControl(GetRequestedServiceControl(sc.GetServiceControl()))
        , m_desiredStatus(GetDesiredStatus(sc.GetServiceControl()))
        , m_logger(logger)
        , m_uxLogger(uxLogger)
        , m_dwRetryCount(0)
    {
    }

    // Performs control (stop, start, pause or resume) operation on the service.
    virtual void PerformAction(IProgressObserver& observer)
    {
        CString section = L" complete";
        CString localPathName = m_name;
        PUSHLOGSECTIONPOP(m_logger, L"Action", L"ServiceControl - " + m_controlString + L" " +  m_name, section);
        SYSTEMTIME actionStartTime;
        GetSystemTime(&actionStartTime);

        // UX Logging        
        m_uxLogger.StartRecordingItem(m_name
                                        , UxEnum::pInstall
                                        , UxEnum::aInstall                                        
                                        , UxEnum::tServiceControl);

        HRESULT hr = ERROR_SUCCESS;

        SC_HANDLE hSCM = NULL;
        SC_HANDLE hService = NULL;

        // Try to open the SC Manager
        // --------------------------
        hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
        if (NULL == hSCM)
        {
            hr = ProcessReturnCode(GetLastError(), L"OpenSCManager");
            goto EndOfPerformAction;
        }

        // Open Service with appropriate control access
        hService = OpenService(hSCM, m_name, m_controlAccess);
        if (!hService)
        {
            hr = ProcessReturnCode(GetLastError(), L"OpenService");
            goto EndOfPerformAction;
        }

        // Perform task
        // Query Service Status
        SERVICE_STATUS serviceStatus;
        if (!QueryServiceStatus(hService, &serviceStatus))
        {
            hr = ProcessReturnCode(GetLastError(), L"QueryServiceStatus");
            goto EndOfPerformAction;
        }

        if (serviceStatus.dwCurrentState == m_desiredStatus)
        {
            LOG(m_logger, ILogger::Result,L"ServiceControl operation succeeded!");
            goto EndOfPerformAction;
        }

        // Perform core control aciton
         SERVICE_STATUS currentServiceStatus;
         if (m_serviceControl == SERVICE_START)
         {
             if (!StartService(hService, 0, NULL))
             {
                 hr = ProcessReturnCode(GetLastError(), L"StartService");
                 goto EndOfPerformAction;
             }
         }
         else
         {
             if (!ControlService(hService, m_serviceControl, &currentServiceStatus))
             {
                 hr = ProcessReturnCode(GetLastError(), L"ControlService");
                 goto EndOfPerformAction;
             }
         }

        if (!WaitForServiceState(hService, m_desiredStatus, &currentServiceStatus, 15000))
        {
            hr = ProcessReturnCode(GetLastError(), L"WaitForServiceStatus" , L"Service Control operation timed out.");
        }
        else
        {
            LOG(m_logger, ILogger::Result,L"ServiceControl operation succeeded!");
        }

EndOfPerformAction:
        // close handles
        CloseServiceHandle(hService);
        CloseServiceHandle(hSCM);        
        m_uxLogger.StopRecordingItem(m_name                                     
                                     , HRESULT_FROM_WIN32(hr)
                                     , 0
                                     , m_controlString
                                     , m_dwRetryCount++); //Supply control string as this is an important piece of information.
        observer.Finished(hr);
    }

private:

    // Returns String value of the controlEnum
    static CString GetControlString(ServiceControl::ControlEnum control)
    {
        return control.ToString();
    }

    // Returns access mask to be used when QueryServiceStatus
    static DWORD GetControlAccessMask(ServiceControl::ControlEnum control)
    {
        DWORD controlAccess = SERVICE_QUERY_STATUS;
        if (control.IsControlStop())
            controlAccess |= SERVICE_STOP;
        if (control.IsControlStart())
            controlAccess |= SERVICE_START;
        if (control.IsControlPause()||control.IsControlResume())
            controlAccess |= SERVICE_PAUSE_CONTINUE;
        return controlAccess;
    }

    // Maps ServiceControl item's ControlEnum to service state/status value.
    static DWORD GetDesiredStatus(ServiceControl::ControlEnum control)
    {
        DWORD desiredStatus = 0;
        if (control.IsControlStart() || control.IsControlResume())
            desiredStatus = SERVICE_RUNNING;
        if (control.IsControlStop())
            desiredStatus = SERVICE_STOPPED;
        if (control.IsControlPause())
            desiredStatus = SERVICE_PAUSED;
        return desiredStatus;
    }

    // Maps ServiceControl item's ControlEum to service control
    static DWORD GetRequestedServiceControl(ServiceControl::ControlEnum control)
    {
        DWORD serviceControl = 0;
        if (control.IsControlStart())
            serviceControl = SERVICE_START;
        if (control.IsControlStop())
            serviceControl = SERVICE_CONTROL_STOP;
        if (control.IsControlPause())
            serviceControl = SERVICE_CONTROL_PAUSE;
        if (control.IsControlResume())
            serviceControl = SERVICE_CONTROL_CONTINUE;
        return serviceControl;
    }

    bool WaitForServiceState(SC_HANDLE hService, DWORD dwDesiredState, SERVICE_STATUS* pss, DWORD dwMilliseconds)
    {
        bool fServiceOK = true;
        bool fFirstTime = true;

        DWORD dwLastState = 0, dwLastCheckPoint = 0;
        DWORD dwTimeout = GetTickCount() + dwMilliseconds;

        for (;;)
        {
            fServiceOK = QueryServiceStatus(hService, pss);
            if (!fServiceOK) break;                                           // error occured
            if (pss->dwCurrentState == dwDesiredState) break;                 // desired state
            if ((dwMilliseconds != INFINITE) && (dwTimeout < GetTickCount())) // Timeout
            {
                fServiceOK = FALSE;
                SetLastError(ERROR_TIMEOUT);
                break;
            }

            if (fFirstTime)
            {
                dwLastState = pss->dwCurrentState;
                dwLastCheckPoint = pss->dwCheckPoint;
                fFirstTime = FALSE;
            }
            else
            {
                if (dwLastState != pss->dwCurrentState)
                {
                    dwLastState = pss->dwCurrentState;
                    dwLastCheckPoint = pss->dwCheckPoint;
                }
                else
                {
                    if (pss->dwCheckPoint >= dwLastCheckPoint)
                    {
                        // Good check point
                        dwLastCheckPoint = pss->dwCheckPoint;
                    }
                    else
                    {
                        // Bad check point
                        fServiceOK = FALSE;
                        break;
                    }
                }
            }

            // Wait the specified period of time
            DWORD dwWaitHint = pss->dwWaitHint / 10;
            if (dwWaitHint < 1000) dwWaitHint = 1000;
            if (dwWaitHint > 10000) dwWaitHint = 10000;
            Sleep(dwWaitHint);
        }

        return (fServiceOK);
    }

    // Logs the Win32 error value and string and returns HRESULT
    HRESULT ProcessReturnCode(DWORD error, const CString& functionName, const CString& message = L"")
    {
        CComBSTR errMsg;
        CSystemUtil::GetErrorString(error, errMsg);
        LOG(m_logger, ILogger::Error, functionName + L" returned: " + errMsg);
        if (!message.IsEmpty())
            LOG(m_logger, ILogger::Error, L"    Error calling " + functionName + L". " + message);
        return HRESULT_FROM_WIN32(error);
    }

private: 
    // test hooks
    // Opens handle to the service with specified control access.
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

typedef ServiceControlInstaller ServiceControlUnInstaller;
typedef ServiceControlInstaller ServiceControlRepairer;

//
// CreateServiceControlPerformer
//
static HRESULT CreateServiceControlPerformer(
                           __in const ActionTable::Actions action,
                           __in const ServiceControl* serviceControl,
                           __in ILogger& logger,
                           __in Ux& uxLogger,
                           __out IPerformer** ppPerformer
                           )
{
    HRESULT hr = S_OK;
    LOG(logger, ILogger::Verbose, L"Creating new Performer for ServiceControl item");

    switch(action)
    {
    case ActionTable::Install:
        *ppPerformer = new ServiceControlInstaller(*serviceControl, logger, uxLogger);
    case ActionTable::Uninstall:
        *ppPerformer = new ServiceControlUnInstaller(*serviceControl, logger, uxLogger);
    case ActionTable::Repair:
        *ppPerformer = new ServiceControlRepairer(*serviceControl, logger, uxLogger);
    case ActionTable::Noop:
        *ppPerformer = new DoNothingPerformer();
        break;
    default:
        IMASSERT2(0, L"Invalid action type; can't create performer");
        LOG(logger, ILogger::Warning, L"Invalid action type. Can't create performer.");
        hr = E_INVALIDARG;
    }

    return hr;
}

} // namespace IronMan
