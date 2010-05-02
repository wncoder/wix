//-------------------------------------------------------------------------------------------------
// <copyright file="IronManExeInstaller.h" company="Microsoft">
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
#include "MmioChainer.h"
//------------------------------------------------------------------------------
// Set of classes that implement IronMan chaining an IronMan exe functionality
//------------------------------------------------------------------------------
namespace IronMan
{
// Base class that Intstaller, Repairer and Uninstaller use
class IronManExeInstallerBase : public ExeInstallerBase
{
    MmioChainer* m_pMmioChainer;
    bool m_bSerialDownload;
    IProgressObserver* m_pChainerObserver;
    HANDLE m_handleChainerRun;
    HRESULT m_hrInternalCode;
    CString m_strCurrentItemStep;
    
public:
    IronManExeInstallerBase(const ExeBase& exe, IProgressObserver::State action,  bool bSerialDownload, ILogger& logger, Ux& uxLogger)
        : ExeInstallerBase(exe, action, logger, uxLogger)
        , m_pMmioChainer(NULL)
        , m_bSerialDownload(bSerialDownload)
        , m_pChainerObserver(&NullProgressObserver::GetNullProgressObserver())
        , m_handleChainerRun(NULL)
        , m_hrInternalCode(S_OK)
    {
        LOG(GetLogger(), ILogger::Verbose, L"Created new IronManExePerformer for Exe item");                  
    }

    ~IronManExeInstallerBase()
    {
        if (m_pMmioChainer != NULL)
        {
            delete m_pMmioChainer;
            m_pMmioChainer = NULL;
        }
    }
    // This gets called when the Controller is aborted, via Performer.Abort()
    // On Abort, communicate that information to the Chainee via MmioChainer Abort()
    virtual void Abort()
    {
        if (m_pMmioChainer)
            m_pMmioChainer->Abort();
    }
private:
    // Launches TreadProc() member.
    static DWORD WINAPI ThreadProc(LPVOID pThis) 
    { 
        return reinterpret_cast<IronManExeInstallerBase*>(pThis)->ThreadProc(); 
    }

    // Runs the blocking MmioChainer::Run() method.
    DWORD ThreadProc()
    {
        // Run method returns when chainee process exits or finished.
        m_pMmioChainer->Run(GetProcess(), *m_pChainerObserver);        
        m_pChainerObserver = &NullProgressObserver::GetNullProgressObserver();
        return 0;
    }

    // This Exe perfomer reports true progress
    virtual bool ExeReportsProgress(void) const
    {
        return true;
    }


private:
    virtual HRESULT PreCreateProcess(CString& commandLine)
    {
        CString log;
        LOG(GetLogger(), ILogger::Verbose, L"In PreCreateProcess");

        // Generate unique section & event names
        WCHAR szSectionName[50] = {0}; 
        WCHAR szEventName[50] = {0}; 
        UINT uRand = 0;
        rand_s(&uRand);
        swprintf_s(szSectionName, _countof(szSectionName), L"SectionName_%u", uRand);
        swprintf_s(szEventName, _countof(szEventName), L"EventName_%u", uRand);


        // Create a MmioChainer instance 
        m_pMmioChainer = new MmioChainer(szSectionName, szEventName, m_logger);
        if (!m_pMmioChainer)
        {
            return E_OUTOFMEMORY;
        }

        commandLine += _T(" /pipe ");
        commandLine += szSectionName;

        // Add /CEIPconsent switch if Ux is sending report.
        if (m_uxLogger.GetUxApproval())        
        {
            commandLine += _T(" /CEIPconsent");
        }

        // Add /log switch if logger is not the NullLogger
        // this will place the child IronMan log files in the same directory
        // as the parent
        if ( !GetLogger().GetFilePath().m_strPath.IsEmpty() )
        {
            // if there is already a log switch authored in the command line, then don't add another
            CString lowerCommandLine(commandLine);
            lowerCommandLine.MakeLower();
            if ( -1 == lowerCommandLine.Find(L"/log") )
            {
                CPath logFilePath(GetLogger().GetFilePath());
                logFilePath.RemoveFileSpec();
                commandLine += L" /log \"" + logFilePath + L"\"";
            }
        }

        // This will be true if simultaneous download is disabled and we are performing
        // serial download followed by install. So, pass this to the Ironman exe.
        if (m_bSerialDownload)
        {
            commandLine += _T(" /serialdownload");
        }

        return S_OK;
    }

    virtual HRESULT PostCreateProcess(IProgressObserver& observer)
    {
        // Remember the passed in observer
        m_pChainerObserver = &observer;
        // Create thread to run blocking MmioChainer::Run() method
        m_handleChainerRun = ::CreateThread( NULL,	// LPSECURITY_ATTRIBUTES
                                            0,		// SIZE_T dwStackSize,
                                            IronManExeInstallerBase::ThreadProc,
                                            reinterpret_cast<LPVOID>(this),	// LPVOID lpParameter,
                                            0,		// DWORD dwCreationFlags,
                                            NULL);	// LPDWORD lpThreadId
        return S_OK;
    }

    virtual HRESULT OnProcessExit(DWORD dwExitCode)
    {
        ::WaitForSingleObject(m_handleChainerRun, INFINITE);
        ::CloseHandle(m_handleChainerRun);

        HRESULT hr = HRESULT_FROM_WIN32(dwExitCode);
        if (!MSIUtils::IsSuccess(hr))
        {
            m_hrInternalCode = m_pMmioChainer->GetInternalErrorCode();
            m_strCurrentItemStep = m_pMmioChainer->GetCurrentItemStep();
            //Need to set the Internal error here so that it gets captured by user experience data.
            WatsonData::WatsonDataStatic()->SetInternalErrorState(m_hrInternalCode, m_strCurrentItemStep);
            LOG(GetLogger(), ILogger::Verbose, L"Internal error from callee is" + WatsonData::WatsonDataStatic()->GetInternalErrorString());
        }

        delete m_pMmioChainer;
        m_pMmioChainer = NULL;        
        return S_OK;
    }

    virtual HRESULT GetInternalError(int exitCode)
    {
        return  m_hrInternalCode;
    }

    virtual CString GetCurrentItemStep() const
    {
        return m_strCurrentItemStep;
    }
};

class IronManExeInstaller : public IronManExeInstallerBase
{
public:
    IronManExeInstaller(const ExeBase& exe,  bool bSerialDownload, ILogger& logger, Ux& uxLogger) 
        : IronManExeInstallerBase(exe, IProgressObserver::Installing, bSerialDownload, logger, uxLogger)
    {
        LOG(GetLogger(), ILogger::Verbose, L"In IronManExeInstaller::IronManExeInstaller");
    }

private:    
    virtual CString GetCommandLine() const
    {
        return GetInstallCommandLine();
    }

    virtual void Rollback(IProgressObserver& observer)
    {
    }
};

class IronManExeUnInstaller : public IronManExeInstallerBase
{
public:
    IronManExeUnInstaller(const ExeBase& exe,  bool bSerialDownload, ILogger& logger, Ux& uxLogger) 
        : IronManExeInstallerBase(exe, IProgressObserver::Uninstalling, bSerialDownload, logger, uxLogger)
    {
    }

private:    
    virtual CString GetCommandLine() const
    {
        return GetUnInstallCommandLine();
    }

    virtual void Rollback(IProgressObserver& observer)
    {
    }
};

class IronManExeRepairer : public IronManExeInstallerBase
{
public:
    IronManExeRepairer(const ExeBase& exe,  bool bSerialDownload, ILogger& logger, Ux& uxLogger) 
        : IronManExeInstallerBase(exe, IProgressObserver::Repairing, bSerialDownload, logger, uxLogger)
    {
    }

private:    
    virtual CString GetCommandLine() const
    {
        return GetRepairCommandLine();
    }

    virtual void Rollback(IProgressObserver& observer)
    {
    }
};



} // namespace IronMan
