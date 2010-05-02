//-------------------------------------------------------------------------------------------------
// <copyright file="InvertingProgressObserver.h" company="Microsoft">
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

#include "interfaces\IProgressObserver.h"

namespace IronMan
{

class InvertingProgressObserver : public IProgressObserver
{
    IProgressObserver& m_observer;
    unsigned int m_installTotalItems, m_rollbackItemCount;
public:
    InvertingProgressObserver(unsigned int totalInstallItems, unsigned int itemsToRollBack, IProgressObserver& observer)
        : m_installTotalItems(totalInstallItems)
        , m_rollbackItemCount(itemsToRollBack)
        , m_observer(observer)
    {}
    virtual ~InvertingProgressObserver() {}
private: // IProgressObserver
    virtual int OnProgress(unsigned char soFar)
    {
        return m_observer.OnProgress((255 - soFar)*m_rollbackItemCount/m_installTotalItems);
    }

    virtual int OnProgressDetail(unsigned char soFar)
    {
        return IDOK;
    }

    virtual void Finished(HRESULT hr) { m_observer.Finished(hr); }
    virtual void OnStateChange(IProgressObserver::State enumVal) { m_observer.OnStateChange(enumVal); }
    virtual void OnStateChangeDetail(const State enumVal, const CString changeInfo) 
    { 
        m_observer.OnStateChangeDetail(enumVal, changeInfo); 
    }
    virtual void OnRebootPending()
    {
        m_observer.OnRebootPending();
    }

};
}
