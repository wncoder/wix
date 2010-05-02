//-------------------------------------------------------------------------------------------------
// <copyright file="IPerformer.h" company="Microsoft">
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

#include "IProgressObserver.h"

namespace IronMan
{

// IPerformer
class IPerformer
{
public:
    virtual ~IPerformer() {};
    virtual void PerformAction(IProgressObserver&) = 0;
    virtual void Abort() = 0;
};


// ICompositePerformer
class ICompositePerformer : public IPerformer
{
public:
    virtual HRESULT GetPackage(
        __in UINT index,
        __out const ItemBase** pItem) = 0;

    virtual HRESULT CreatePackagePerformer(
        __in const ItemBase* pItem,
        __in const ActionTable::Actions action,
        __in IBurnView *pBurnView,
        __out IPerformer** ppPerformer) = 0;
    
    virtual void StopPerformer() = 0;

    virtual HRESULT UpdatePackageLocation(
        __in UINT index,
        __out const CPath& path,
        __in bool fVerify) = 0;

    virtual bool IsCached(UINT nIndex) = 0;

    virtual HRESULT VerifyAndCachePackage(UINT nIndex) = 0;
};


class NullPerformer : public IPerformer
{
public:
    static IPerformer& GetNullPerformer()
    {
        static NullPerformer nullPerformer;
        return nullPerformer;
    }
private:
    virtual void PerformAction(IProgressObserver&) {}
    virtual void Abort() {}
};


class NullCompositePerformer : public ICompositePerformer
{
public:
    static NullCompositePerformer& GetNullPerformer()
    { 
        static NullCompositePerformer nullPerformer;
        return nullPerformer;
    }
private:
    virtual void PerformAction(IProgressObserver&) {}

    virtual void Abort() {}

    virtual HRESULT GetPackage(
        __in UINT index,
        __out const ItemBase** pItem)
    {
    }

    virtual HRESULT CreatePackagePerformer(
        __in const ItemBase* pItem,
        __in const ActionTable::Actions action,
        __in IBurnView *pBurnView,
        __out IPerformer** ppPerformer)
    {
    }

    virtual void StopPerformer() 
    {
    }
    virtual HRESULT UpdatePackageLocation(
        __in UINT index, 
        __out const CPath& path,
        __in bool fVerify)
    {
    }
    virtual bool IsCached(UINT nIndex)
    {
        return false;
    }

    virtual HRESULT VerifyAndCachePackage(UINT nIndex)
    {
        return true;
    }

};

template<typename B>
class AbortBaseT : public B
{
    bool m_abort;
public:
    AbortBaseT() : m_abort(false) {}
    virtual ~AbortBaseT() {}
    virtual void Abort() { m_abort = true; }
protected:
    bool HasAborted() const { return m_abort; }
    const bool& AbortFlagRef() const { return m_abort; }
};

typedef AbortBaseT<IPerformer> AbortPerformerBase;

class AbortStopPerformerBase : public AbortPerformerBase
{
    bool m_stopExecution;

public:
    AbortStopPerformerBase() 
        : m_stopExecution(false)
    {
    }

    virtual void StopPerformer() 
    {
        m_stopExecution = true; 
    }

    virtual bool IsStopPending() const
    { 
        return m_stopExecution; 
    }

};

class DoNothingPerformer : public AbortPerformerBase
{
    virtual void PerformAction(IProgressObserver& observer) 
    { 
        observer.Finished(S_OK); 
    }
};

}

