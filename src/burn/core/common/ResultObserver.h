//-------------------------------------------------------------------------------------------------
// <copyright file="ResultObserver.h" company="Microsoft">
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

class ResultObserver : public IProgressObserver
{
    IProgressObserver& m_observer;
    HRESULT& m_hr;
public:
    ResultObserver(IProgressObserver& observer, HRESULT& hr)
        : m_observer(observer)
        , m_hr(hr)
    {}
    virtual int OnProgress(unsigned char soFar)
    {
        return m_observer.OnProgress(soFar);
    }

    virtual int OnProgressDetail(unsigned char soFar)
    {
        return m_observer.OnProgressDetail(soFar);
    }

    virtual void Finished(HRESULT hr)
    {
        m_hr = hr;
    }
    virtual void OnStateChange(IProgressObserver::State enumVal) { m_observer.OnStateChange(enumVal); }
    virtual void OnStateChangeDetail(const State enumVal, const CString changeInfo) { m_observer.OnStateChangeDetail(enumVal, changeInfo); }
    virtual void OnRebootPending()
    {
        m_observer.OnRebootPending();
    }
};

}
