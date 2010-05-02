//-------------------------------------------------------------------------------------------------
// <copyright file="NotifyController.h" company="Microsoft">
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

#include "Interfaces\IController.h"
#include "Interfaces\IProgressObserver.h"
#include "Interfaces\IPerformer.h"
#include "CompositePerformer.h"
#include "CompositeDownloader.h"

namespace IronMan
{

class NotifyController : public  INotifyController // so controller can be notified that it MayBegin
{
    IPerformer& m_performer;
    IProgressObserver* m_pObserver;
    HANDLE m_handle;

protected: // for PipeController, for instance
    NotifyController(const NotifyController& rhs)
        : m_performer(rhs.m_performer)
        , m_pObserver(rhs.m_pObserver)
        , m_handle(NULL)
    {}

public:
    NotifyController(IPerformer& performer)
        : m_performer(performer)
        , m_pObserver(&NullProgressObserver::GetNullProgressObserver())
        , m_handle(NULL)
    {}
    virtual ~NotifyController()
    {
        if (m_handle != NULL)
        {
            ::WaitForSingleObject(m_handle, INFINITE);
            ::CloseHandle(m_handle);
        }
    }

private:
    static DWORD WINAPI ThreadProc(LPVOID pThis) { return reinterpret_cast<NotifyController*>(pThis)->ThreadProc(); }
    DWORD ThreadProc()
    {
        m_performer.PerformAction(*m_pObserver);

        m_pObserver = &NullProgressObserver::GetNullProgressObserver();
        ::CloseHandle(m_handle);
        m_handle = NULL;
        return 0;
    }

protected: // INotifyController (protected so that PipeController can call base methods)
    virtual bool MayBegin(IProgressObserver& observer)
    {
        // only once
        _ASSERT(m_handle == NULL);

        m_pObserver = &observer;
        m_handle = ::CreateThread ( NULL,	// LPSECURITY_ATTRIBUTES
                                    0,		// SIZE_T dwStackSize,
                                    NotifyController::ThreadProc,
                                    reinterpret_cast<LPVOID>(this),	// LPVOID lpParameter,
                                    0,		// DWORD dwCreationFlags,
                                    NULL);	// LPDWORD lpThreadId
        return true;		
    }
    virtual void Abort()
    {
        m_performer.Abort();
    }

    virtual void Stop()
    {
        CompositeDownloader* pDownloader = dynamic_cast<CompositeDownloader*> (&m_performer);
        if (pDownloader)
        {
            pDownloader->StopPerformer();
        }
        CompositePerformer* pInstaller = dynamic_cast<CompositePerformer*>(&m_performer);
        if (pInstaller)
        {
            pInstaller->StopPerformer();
        }
    }
};

}
