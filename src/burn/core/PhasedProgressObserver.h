//-------------------------------------------------------------------------------------------------
// <copyright file="PhasedProgressObserver.h" company="Microsoft">
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

#define HIASSERT(a,b)

namespace IronMan
{
class PhasedProgressObserver : public IProgressObserver
{
    /*
        NOTE NOTE NOTE
        I'm commenting out the code dealing with implicit and explicit rollback,
        until such time as we need to hook it in.
        There are no unit tests for this code yet.
    */

    //bool m_bExplicitRollback;
    int m_currentPhase;
    int m_numPhases;
    int m_NextPhaseResult;
    IProgressObserver& m_Observer;
public:
    PhasedProgressObserver(IProgressObserver& observer, int numPhases)
        : m_Observer(observer)
        , m_numPhases(numPhases)
        , m_currentPhase(-1)
        , m_NextPhaseResult(IDOK)
        //, m_bExplicitRollback(false)
    {}
    virtual ~PhasedProgressObserver() {}

    int SetNextPhase()
    {
        //if (true == m_bExplicitRollback)
        //{
        //  HIASSERT(m_currentPhase >= 0, L"too many phases during rollback!");
        //  --m_currentPhase;
        //}
        //else
        {
            HIASSERT(m_currentPhase <= m_numPhases, L"too many phases!");
            ++m_currentPhase;
        }
        return OnProgress(0);
    }

    // Will be called if the installer has to restart due to the user selecting 'retry' in FilesInUse dialog
    void DecrementPhase(void) throw()
    {
        IMASSERT(m_currentPhase >= 0);
        --m_currentPhase;
    }

    //void Reset(bool bExplicitRollback=false)
    //{
    //  m_bExplicitRollback = bExplicitRollback;

    //  if (true == bExplicitRollback)
    //  {
    //      m_currentPhase = m_numPhases = 2;
    //  }
    //  else
    //  {
    //      m_currentPhase = -1;
    //  }
    //}
    //void ImplicitRollback()
    //{
    //  m_currentPhase = 0;
    //  m_numPhases = 1;
    //}
    void ExplicitRollback(int current, int total)
    {
        m_currentPhase = current;
        m_numPhases = total;
    }

public:
    // IProgressObserver methods
    virtual int OnProgress(unsigned char soFar)
    {
        int nResult = IDOK;

        //if (true == m_bExplicitRollback)
        //  soFar = 255 - soFar;

        if (m_currentPhase < 0)
            nResult = m_Observer.OnProgress(0);
        else if (m_currentPhase >= m_numPhases)
            nResult = m_Observer.OnProgress(255);
        else
            nResult = m_Observer.OnProgress((m_currentPhase*255 + soFar)/m_numPhases);

        return nResult;
    }

    virtual int OnProgressDetail(unsigned char soFar)
    {
        return IDOK;
    }

    virtual void Finished(HRESULT hr)
    {
        m_Observer.Finished(hr);
    }
    virtual void OnStateChange(IProgressObserver::State enumVal) { m_Observer.OnStateChange(enumVal); }
    virtual void OnStateChangeDetail(const State enumVal, const CString changeInfo) 
    { 
        m_Observer.OnStateChangeDetail(enumVal, changeInfo); 
    }
    virtual void OnRebootPending()
    {
        m_Observer.OnRebootPending();
    }
};

class MsiProgressObserver : public IProgressObserver
{
    /*
        NOTE NOTE NOTE
        I'm commenting out the code dealing with implicit and explicit rollback,
        until such time as we need to hook it in.
        There are no unit tests for this code yet.
    */

    //bool m_bExplicitRollback;
    IProgressObserver& m_Observer;
    CSimpleArray<UINT> m_phaseTicks; // Stores number of ticks allocated for each phase. Ticks for all phases total to 255.
    CSimpleArray<UINT> m_ticksUpToPhase; // Stores the sum of ticks upto and including the phase
    int m_currentPhase;

public:
    MsiProgressObserver(IProgressObserver& observer, const CSimpleArray<UCHAR>& phaseWeights)
        : m_Observer(observer)
        , m_currentPhase(-1)
        //, m_bExplicitRollback(false)
    {
        UINT totalWeight = 0;
        for (int i=0; i<phaseWeights.GetSize(); ++i)
        {
            totalWeight += phaseWeights[i];
        }

        UINT  currTotalWeight = 0;
        UCHAR ticksSum = 0; 
        for (int i=0; i<phaseWeights.GetSize(); ++i)
        {
            currTotalWeight += phaseWeights[i];
            UINT tempTicks = (currTotalWeight * 255) / totalWeight;
            m_ticksUpToPhase.Add(tempTicks);
        }

        m_phaseTicks.Add(m_ticksUpToPhase[0]);
        for (int i=1; i<phaseWeights.GetSize(); ++i)
        {
            m_phaseTicks.Add(m_ticksUpToPhase[i] - m_ticksUpToPhase[i-1]);
        }
    }
    virtual ~MsiProgressObserver() {}

    int SetNextPhase()
    {
        //if (true == m_bExplicitRollback)
        //{
        //  HIASSERT(m_currentPhase >= 0, L"too many phases during rollback!");
        //  --m_currentPhase;
        //}
        //else
        {
            HIASSERT(m_currentPhase <= m_ticksUpToPhase.GetSize(), L"too many phases!");
            ++m_currentPhase;
        }
        return OnProgress(0);
    }

    // Will be called if the installer has to restart due to the user selecting 'retry' in FilesInUse dialog
    void DecrementPhase(void) throw()
    {
        IMASSERT(m_currentPhase >= 0);
        --m_currentPhase;
    }

    //void Reset(bool bExplicitRollback=false)
    //{
    //  m_bExplicitRollback = bExplicitRollback;

    //  if (true == bExplicitRollback)
    //  {
    //      m_currentPhase = m_numPhases = 2;
    //  }
    //  else
    //  {
    //      m_currentPhase = -1;
    //  }
    //}
    //void ImplicitRollback()
    //{
    //  m_currentPhase = 0;
    //  m_numPhases = 1;
    //}
    //void ExplicitRollback(int current, int total)
    //{
    //    m_currentPhase = current;
    //    m_numPhases = total;
    //}

public:
    // IProgressObserver methods
    virtual int OnProgress(unsigned char soFar)
    {
        int nResult = IDOK;

        //if (true == m_bExplicitRollback)
        //  soFar = 255 - soFar;

        if (m_currentPhase < 0)
        {
           nResult = m_Observer.OnProgress(0);
        }
        else if (m_currentPhase >= m_ticksUpToPhase.GetSize())
        {
            nResult = m_Observer.OnProgress(255);
        }
        else
        {
            UINT soFarOut = 0;
            
            if (m_currentPhase == 0)
            {
                soFarOut = (m_phaseTicks[0] * soFar) / 255;
            }
            else
            {
                soFarOut = m_ticksUpToPhase[m_currentPhase-1] + (m_phaseTicks[m_currentPhase] * soFar) / 255;
            }
            
            IMASSERT(soFarOut <= 255);
            if (soFarOut > 255)
            {
                soFarOut = 255;
            }

            nResult = m_Observer.OnProgress(soFarOut);
        }

        return nResult;
    }

    virtual int OnProgressDetail(unsigned char soFar) 
    { 
        return IDOK;
    }

    virtual void Finished(HRESULT hr)
    {
        m_Observer.Finished(hr);
    }
    virtual void OnStateChange(IProgressObserver::State enumVal) { m_Observer.OnStateChange(enumVal); }
    virtual void OnStateChangeDetail(const State enumVal, const CString changeInfo) 
    { 
        m_Observer.OnStateChangeDetail(enumVal, changeInfo); 
    }
    virtual void OnRebootPending()
    {
        m_Observer.OnRebootPending();
    }
};
}
