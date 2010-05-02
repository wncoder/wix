//-------------------------------------------------------------------------------------------------
// <copyright file="MmioController.h" company="Microsoft">
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

#include "NotifyController.h"
#include "Watson\WatsonDefinition.h"

namespace IronMan
{
// IronMan creates the controller using the name passed in on the command line for the memory mapped section
class MmioController : public NotifyController, private MmioChainee, private IProgressObserver
{
    IProgressObserver* m_pObserver;
    bool m_downloader;
public:
    // Constructor
    MmioController(const NotifyController& controller, const CString& sectionName, bool downloader, ILogger& logger)
        : NotifyController(controller)
        , MmioChainee(sectionName, logger) // sectionName is passed in via command line.
        , m_pObserver(&NullProgressObserver::GetNullProgressObserver())
        , m_downloader(downloader) // true if this is a downloader
    {
    }

    // Destructor
    virtual ~MmioController()
    {
    }

private:
    // INotifyController
    virtual bool MayBegin(IProgressObserver& observer)
    {
        m_pObserver = &observer;
        return NotifyController::MayBegin(*this);
    }
    virtual void Abort()
    {
        MmioChainee::Abort();
        NotifyController::Abort();
    }

private:
    // delegating IProgressObserver
    virtual int OnProgress(unsigned char soFar)
    {
        MmioChainee::SoFar(soFar, m_downloader); // Updated the chainee progress
        int nResult = m_pObserver->OnProgress(soFar);

        if (MmioChainee::IsAborted())
            NotifyController::Abort();

        return nResult;
    }

    virtual int OnProgressDetail(unsigned char soFar) 
    {
        return IDOK;
    }

    virtual void Finished(HRESULT hr)
    {
        // This is called when either the install or download phases finish, with m_downloader indicating which one
        MmioChainee::Finished(  hr, WatsonData::WatsonDataStatic()->GetInternalError(), WatsonData::WatsonDataStatic()->GetCurrentStep() , m_downloader);

        // Tell the progrss obsrver and pass the final RESULT, the install result taking priority if both fail
        m_pObserver->Finished(hr);
        // Replace the observer with th null observer since no more calls should be made to it
        m_pObserver = &NullProgressObserver::GetNullProgressObserver();
    }
    
    // The send the state change and detailed state change (item infomation) to the observer
    virtual void OnStateChange(IProgressObserver::State enumVal)
    {
        MmioChainee::OnStateChange(enumVal);
        m_pObserver->OnStateChange(enumVal);
    }

    virtual void OnStateChangeDetail(const State enumVal, const CString changeInfo)
    {
        MmioChainee::OnStateChangeDetail(enumVal, changeInfo);
        m_pObserver->OnStateChangeDetail(enumVal, changeInfo);
    }

    // Called when there is a reboot pending
    virtual void OnRebootPending()
    {
        MmioChainee::OnRebootPending();
        m_pObserver->OnRebootPending();
    }
};
}
