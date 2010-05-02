//-------------------------------------------------------------------------------------------------
// <copyright file="WeightedProgressObserver.h" company="Microsoft">
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

class WeightedProgressObserver : public IProgressObserver
{
    IProgressObserver& m_observer;
    CSimpleArray<ULONGLONG> m_weights;
    int m_currentIndex;
    ULONGLONG m_weightSoFar, m_totalWeight;
public:
    WeightedProgressObserver(IProgressObserver& observer, const CSimpleArray<ULONGLONG>& weights)
        : m_observer(observer)
        , m_weights(weights)
        , m_currentIndex(-1)
        , m_weightSoFar(0)
        , m_totalWeight(0)
    {
        for(int i=0; i<m_weights.GetSize(); ++i)
        {
            m_totalWeight += m_weights[i];
        }
    }
    virtual ~WeightedProgressObserver() {}

    int SetNextPhase()
    {
        ++m_currentIndex;
        IMASSERT(m_currentIndex < m_weights.GetSize());
        if (m_currentIndex < m_weights.GetSize())
        {
            if (m_currentIndex > 0)
                m_weightSoFar += m_weights[m_currentIndex-1];
        }
        return OnProgress(0);
    }

public:
    // IProgressObserver methods
    virtual int OnProgress(unsigned char soFar) // 0 through 255
    {
        int nResult = IDOK;

        if (m_currentIndex < 0)
            nResult = m_observer.OnProgress(0);
        else if (m_currentIndex >= m_weights.GetSize())
            nResult = m_observer.OnProgress(255);
        else
        {
            m_observer.OnProgressDetail(soFar);
            ULONGLONG weightThisPhase = m_weights[m_currentIndex]*soFar/255;
            // Don't allow divide by zero error
            ULONGLONG totalFractionSoFar = (m_totalWeight > 0 )? (m_weightSoFar + weightThisPhase)*255/m_totalWeight : 128;
            nResult = m_observer.OnProgress(0xFF&totalFractionSoFar); // convert to ticks (ANDing 0xFF to avoid compiler warning)
        }

        return nResult;
    }

    virtual int OnProgressDetail(unsigned char soFar) // 0 through 255
    {
        return IDOK;
    }

    virtual void Finished(HRESULT hr)
    {
        m_observer.Finished(hr);
    }
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
