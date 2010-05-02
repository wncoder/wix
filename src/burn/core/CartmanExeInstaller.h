//-------------------------------------------------------------------------------------------------
// <copyright file="CartmanExeInstaller.h" company="Microsoft">
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
//    The set of classes for handling the installation of a Cartman package.
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once

#include "schema\EngineData.h"
#include "Interfaces\IPerformer.h"
#include "Interfaces\ILogger.h"
#include "common\SystemUtil.h"
#include "Ux\Ux.h"
#include "ExeInstaller.h"

#pragma warning (push)
#pragma warning( disable:22116 )
#pragma warning( disable:25007 )
#pragma warning( disable:25021 )
#pragma warning( disable:25032 )
#pragma warning( disable:25057 )
#include "SetupWatcher.h"
#pragma warning (pop)

namespace IronMan
{

class ProgressManager
{
    ILogger& m_logger;
    IProgressObserver& m_observer;

    //  ProgressObserver takes progress ticks from 0 to 255 max.   
    //
    //  SetupWatcher reports progress in 3 phases - initiallizing, downloading 
    //  and installing. Download phase might not be present if the required file
    //  is present locally.
    //
    //  I will assign
    //      25 ticks to the initializing phase,
    //      75 ticks to the downloading phase (if any) and the  
    //      rest to install phase.
    //
    static const UINT c_maxTicksForInitializingPhase = 25;
    static const UINT c_maxTicksForDownloadingPhase = 75;
    bool m_downloadPhaseWasPresent;

    UINT m_maxTicksForInstallPhase;
    UINT m_ticksReportedBeforeInstallPhase;
    
    UINT m_ticksReportedOnPreviousUpdate;

public:
    //Constructor
    ProgressManager(IProgressObserver& observer, ILogger& logger)
        : m_logger(logger)
        , m_observer(observer)
        , m_maxTicksForInstallPhase(0)
        , m_downloadPhaseWasPresent(false)
        , m_ticksReportedOnPreviousUpdate(0)
        , m_ticksReportedBeforeInstallPhase(0)
    {
    }

    class ProgressRecord
    {
        MSIHANDLE m_hRecord;
    public:
        ProgressRecord(MSIHANDLE hRecord) : m_hRecord(hRecord) {}
        int GetIntFromField(int field) const { return ::MsiRecordGetInteger(m_hRecord, field); }
        int GetPhase(void) const { return GetIntFromField(1); }
    };

    int Update(ProgressRecord record)
    {
        int nResult = IDOK;
        UINT ticksToReport = 0;
        
        int phase = record.GetPhase();


        /*
           Table showing the contents of the fields in MSI record
        __________________________________________________________________________________________________________________
        |             |         |                              |                                 |                       |
        |Phase        | Field-1 |         Field-2              |        Field-3                  |     Field-4           |
        |             | (int)   |          (int)               |        (int)                    |     (int)             |
        |_____________|_________|______________________________|_________________________________|_______________________|
        |             |         |                              |                                 |                       |
        |Initializing |    1    | Percent completed so far     |      - not used -               |   - not used -        |
        |             |         | An int from 0 - 100.         |                                 |                       |
        |             |         |                              |                                 |                       |
        |Downloading  |    2    | # of KBs downloaded so far   | Total # of KBs to be downloaded | Transfer rate in KB/s |
        |             |         |                              |                                 |                       |
        |             |         |                              |                                 |                       |
        |Installing   |    3    | # of steps completed so far  | Total # steps to be completed   |   - not used -        |
        |             |         |                              |                                 |                       |
        |             |         |                              |                                 |                       |
        |Rollback     |   -1    |     - not used -             |      - not used -               |   - not used -        |
        |_____________|_________|______________________________|_________________________________|_______________________|

        */

        int intInFieldtwo = record.GetIntFromField(2);
        int intInFieldthree = 0;

        switch (phase)
        {
        case 1: // Phase 1: Initializing
            if (intInFieldtwo < 100)
            {
                ticksToReport = c_maxTicksForInitializingPhase * intInFieldtwo / 100;
            }
            break;

        case 2: // Phase 2: Downloading
            {
                if (m_downloadPhaseWasPresent == false)
                {
                    IMASSERT(m_ticksReportedOnPreviousUpdate > 0 && m_ticksReportedOnPreviousUpdate <=25);
                    
                    m_downloadPhaseWasPresent = true;
                }

                intInFieldthree = record.GetIntFromField(3);

                if (intInFieldthree > 0 && (intInFieldtwo < intInFieldthree))
                {
                    ticksToReport = c_maxTicksForInitializingPhase 
                                  + c_maxTicksForDownloadingPhase * intInFieldtwo / intInFieldthree;
                }
            }
            break;

        case 3: // Phase 3: Installing
            {
                if (m_maxTicksForInstallPhase == 0)
                {
                    IMASSERT(m_ticksReportedOnPreviousUpdate > 0 && m_ticksReportedOnPreviousUpdate <= 100);

                    m_ticksReportedBeforeInstallPhase = c_maxTicksForInitializingPhase;
                    if (m_downloadPhaseWasPresent)
                    {
                        m_ticksReportedBeforeInstallPhase += c_maxTicksForDownloadingPhase;
                    }

                    IMASSERT(m_ticksReportedBeforeInstallPhase <= 100);

                    m_maxTicksForInstallPhase = 255 - m_ticksReportedBeforeInstallPhase;
                }

                intInFieldthree = record.GetIntFromField(3);

                if (intInFieldthree > 0 && (intInFieldtwo < intInFieldthree))
                {
                    ticksToReport = m_ticksReportedBeforeInstallPhase
                                  + m_maxTicksForInstallPhase * intInFieldtwo / intInFieldthree;
                }
            }
            break;

        case -1: // Rollback (after error or cancellation)
            // no progress reported.
            break;
        }

        IMASSERT(ticksToReport <= 255);

        if (ticksToReport >= m_ticksReportedOnPreviousUpdate)
        {
            nResult = m_observer.OnProgress(ticksToReport);
            m_ticksReportedOnPreviousUpdate = ticksToReport;
        }

        return nResult;
    }

};

class CartmanExeInstallerBase : public ExeInstallerBase
{
    CSetupWatcher*  m_pSetupWatcher;
    ProgressManager* m_pProgressManager;

public:
    //Constructor
    CartmanExeInstallerBase(const ExeBase& exe, IProgressObserver::State action,  ILogger& logger, Ux& uxLogger)
        : ExeInstallerBase(exe, action, logger, uxLogger)
        , m_pProgressManager(NULL)
    {
        LOG(GetLogger(), ILogger::Verbose, L"Created new CartmanExePerformer for Exe item");
    }

    //Destructor
    ~CartmanExeInstallerBase()
    {
        if (m_pProgressManager != NULL)
        {
            delete m_pProgressManager;
            m_pProgressManager = NULL;
        }
        if (m_pSetupWatcher != NULL)
        {
            delete m_pSetupWatcher;
            m_pSetupWatcher = NULL;
        }
    }

private:
    virtual bool ExeReportsProgress(void) const
    {
        return true;
    }

private:
    virtual HRESULT PreCreateProcess(CString& commandLine)
    {
        CString log;
        LOG(GetLogger(), ILogger::Verbose, L"In PreCreateProcess");

        // Generate an unique name 
        WCHAR szWatcherPipeName[50] = {0};
        UINT uRand = 0;
        rand_s(&uRand);
        swprintf_s(szWatcherPipeName, _countof(szWatcherPipeName), L"CartmanSetupExeWatcher%u", uRand);

        //
        // Create the SetupWatcher instance and begin the connection process.
        //
        m_pSetupWatcher = new CSetupWatcher(szWatcherPipeName);

        if (!m_pSetupWatcher)
        {
            return E_OUTOFMEMORY;
        }

        DWORD dwRet = m_pSetupWatcher->Connect();
        if (dwRet != 0)
        {
            log.Format(L"Failed to connect setup watcher. Error code = %d", dwRet);
            LOG(GetLogger(), ILogger::Warning, log);
            return HRESULT_FROM_WIN32(dwRet);
        }

        LOG(GetLogger(), ILogger::Verbose, L"m_pSetupWatcher->Connect succeeded");

        commandLine += _T(" /progress ");
        commandLine += szWatcherPipeName;

        return S_OK;
    }

    virtual HRESULT PostCreateProcess(IProgressObserver& observer)
    {
        m_pProgressManager = new ProgressManager(observer, GetLogger());

        //
        // Begin watching by receiving progress messages from the setup process.
        // Handle messages with the HandleProgressMessages callback.
        //
        DWORD dwRet = m_pSetupWatcher->ReceiveMessages(HandleProgressMessages, reinterpret_cast<void*>(this));
        if (dwRet != 0)
        {
            LOG(GetLogger(), ILogger::Warning, L"Failed to receive messages from setup watcher.");
        }

        LOG(GetLogger(), ILogger::Verbose, L"PostCreateProcess succeeded");
        return S_OK;
    }

    virtual HRESULT OnProcessExit(DWORD dwExitCode)
    {
        delete m_pSetupWatcher;
        m_pSetupWatcher = NULL;
        return S_OK;
    }

    //
    // Handle progress messages received from the setup process. 
    //
    static int CALLBACK HandleProgressMessages(void* pvContext, UINT iMessageType, MSIHANDLE hRecord)
    {
        CartmanExeInstallerBase *pThis = reinterpret_cast<CartmanExeInstallerBase*>(pvContext);
        int nResult = pThis->HasAborted() ? IDCANCEL : IDOK;
        int nNewResult = IDOK;
    
        if (iMessageType == INSTALLMESSAGE_SUITEPROGRESS)
        {
            nNewResult = pThis->UpdateProgress(hRecord);
            if (IDOK == nResult)
            {
                nResult = nNewResult;
                if (FAILED(HRESULT_FROM_VIEW(nResult)))
                {
                    LOG(pThis->GetLogger(), ILogger::Error, L"User interface commanded engine to abort");
                }
            }
        }
    
        return nResult;
    }


    int UpdateProgress(MSIHANDLE hRecord)
    {
        return m_pProgressManager->Update(hRecord);
    }
};

class CartmanExeInstaller : public CartmanExeInstallerBase
{
public:
    CartmanExeInstaller(const ExeBase& exe,  ILogger& logger, Ux& uxLogger) 
        : CartmanExeInstallerBase(exe, IProgressObserver::Installing, logger, uxLogger)
    {
        LOG(GetLogger(), ILogger::Verbose, L"In CartmanExeInstaller::CartmanExeInstaller");
    }

private:    
    virtual CString GetCommandLine() const
    {
        return GetInstallCommandLine();
    }

    virtual void Rollback(IProgressObserver& observer)
    {}
};

class CartmanExeUnInstaller : public CartmanExeInstallerBase
{
public:
    CartmanExeUnInstaller(const ExeBase& exe,  ILogger& logger, Ux& uxLogger) 
        : CartmanExeInstallerBase(exe, IProgressObserver::Uninstalling, logger, uxLogger)
    {
    }

private:    
    virtual CString GetCommandLine() const
    {
        return GetUnInstallCommandLine();
    }

    virtual void Rollback(IProgressObserver& observer)
    {}
};

class CartmanExeRepairer : public CartmanExeInstallerBase
{
public:
    CartmanExeRepairer(const ExeBase& exe,  ILogger& logger, Ux& uxLogger) 
        : CartmanExeInstallerBase(exe, IProgressObserver::Repairing, logger, uxLogger)
    {
    }

private:    
    virtual CString GetCommandLine() const
    {
        return GetRepairCommandLine();
    }

    virtual void Rollback(IProgressObserver& observer)
    {}
};

} // namespace IronMan
