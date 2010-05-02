//-------------------------------------------------------------------------------------------------
// <copyright file="CompositeController.h" company="Microsoft">
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
//      The controller for installing and downloading.
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once

#include "interfaces\IController.h"
#include "interfaces\IProgressObserver.h"
#include "interfaces\IPerformer.h"
#include "interfaces\ILogger.h"

namespace IronMan
{

class CompositeController : public AbortBaseT<INotifyController>
{
    CSimpleArray<INotifyController*> m_controllers;
    int m_started;
    ILogger& m_logger;
    ILogger* m_forked;
    BOOL m_fStopPending;

    class LogMergingObserver : public IProgressObserver
    {
        IProgressObserver& m_observer;
        ILogger& m_logger;
        ILogger* m_forked;
    public:
        LogMergingObserver(IProgressObserver& observer, ILogger& logger, ILogger* forked)
            : m_observer(observer)
            , m_logger(logger)
            , m_forked(forked)
        {}

        virtual int OnProgress(unsigned char uc) { return m_observer.OnProgress(uc); }
        virtual int OnProgressDetail(unsigned char uc) { return m_observer.OnProgressDetail(uc); }

        virtual void Finished(HRESULT hr)
        {
            m_logger.Merge(m_forked);
            m_observer.Finished(hr);
        }
        virtual void OnStateChange(IProgressObserver::State enumVal)
        {
            m_observer.OnStateChange(enumVal);
        }
        virtual void OnStateChangeDetail (const State enumVal, const CString changeInfo)
        {
            m_observer.OnStateChangeDetail(enumVal, changeInfo);
        }
        virtual void OnRebootPending()
        {
            m_observer.OnRebootPending();
        }
    };
    LogMergingObserver* m_lmo;

    class ControllingObserver : public IProgressObserver
    {
        // idea:  if this observer gets an error in its finished method
        // abort the other guy
        IProgressObserver* m_observer;
        INotifyController& m_controllee;
    public:
        ControllingObserver(INotifyController& controllee)
            : m_controllee(controllee)
            , m_observer(&NullProgressObserver::GetNullProgressObserver())
        {}
        void SetObserver(IProgressObserver* observer) { m_observer = observer; }
    public: // IProgressObserver
        virtual int OnProgress(unsigned char uc)
        {
            return m_observer->OnProgress(uc);
        }
        virtual int OnProgressDetail(unsigned char uc)
        {
            return m_observer->OnProgressDetail(uc);
        }

        virtual void Finished(HRESULT hr)
        {
            if (FAILED(hr))
                m_controllee.Abort();
            m_observer->Finished(hr);
        }
        virtual void OnStateChange(IProgressObserver::State enumVal)
        {
            m_observer->OnStateChange(enumVal);
        }
        virtual void OnStateChangeDetail (const State enumVal, const CString changeInfo)
        {
            m_observer->OnStateChangeDetail(enumVal, changeInfo);
        }
        virtual void OnRebootPending()
        {
            m_observer->OnRebootPending();
        }
    };
    ControllingObserver m_download, m_install;

public:
    CompositeController(INotifyController& controller, ILogger& logger, ILogger* forked)
        : m_started(-1)
        , m_lmo(NULL)
        , m_logger(logger)
        , m_forked(forked)
        , m_download(controller)
        , m_install (controller)
        , m_fStopPending(FALSE)
    {
        m_controllers.Add(&controller);
    }
    CompositeController(INotifyController* controllers[2], ILogger& logger, ILogger* forked)
        : m_started(-1)
        , m_lmo(NULL)
        , m_logger(logger)
        , m_forked(forked)
        , m_download(*controllers[1]) // download observer aborts the installer
        , m_install (*controllers[0]) //  install observer aborts the downloader
        , m_fStopPending(FALSE)
    {
        for(unsigned i=0; i<2; ++i)
        {
            m_controllers.Add(controllers[i]);
        }
    }
    virtual ~CompositeController()
    {
        delete m_lmo;
    }

public:
    virtual bool MayBegin(IProgressObserver& observer)
    {
        ++m_started;
        if (m_started >= m_controllers.GetSize())
            return false;

        if (m_started != 1)
        {
            m_download.SetObserver(&observer);
            m_controllers[m_started]->MayBegin(m_download);
        }
        else
        {
            m_lmo = new LogMergingObserver(observer, m_logger, m_forked);
            m_install.SetObserver(m_lmo);
            m_controllers[m_started]->MayBegin(m_install);
        }

        if (m_fStopPending)
        {
            m_controllers[m_started]->Stop();
            LOG(m_logger, ILogger::Information, L"Performer is stopped");
        }

        return !HasAborted();
    }
    virtual void Abort()
    {
        AbortBaseT<INotifyController>::Abort();

        for(int i=0; i<m_controllers.GetSize(); ++i)
            m_controllers[i]->Abort();
    }

    virtual void Stop()
    {
        m_fStopPending = TRUE;
        for(int i=0; i<m_controllers.GetSize(); ++i)
            m_controllers[i]->Stop();
    }
};

}
