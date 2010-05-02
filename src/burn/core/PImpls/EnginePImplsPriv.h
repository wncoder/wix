//-------------------------------------------------------------------------------------------------
// <copyright file="EnginePImplsPriv.h" company="Microsoft">
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

#include "EnginePImpls.h"
#include "..\schema\EngineData.h"

namespace EnginePImplsPrivate
{

class CItemsPImpl : public EnginePImpls::ItemsPImpl
{
    IronMan::Items * m_pImpl;
public:
    CItemsPImpl(const IronMan::Items& rhs) : m_pImpl(new IronMan::Items(rhs)) {}
    virtual ~CItemsPImpl() { delete m_pImpl; }

//	const IronMan::Items& ItemsPImpl::GetImpl() const { return *m_pImpl; }
//	ItemsPImpl::operator const IronMan::Items&() const { return *m_pImpl; }
};

class CUiPImpl : public EnginePImpls::UiPImpl
{
    IronMan::Ui * m_pImpl;
public:
    CUiPImpl(const IronMan::Ui& rhs) : m_pImpl(new IronMan::Ui(rhs)) {}
    virtual ~CUiPImpl() { delete m_pImpl; }
    virtual CPath GetDll() { return m_pImpl->GetDll(); }
};

class CEngineDataPImpl : public EnginePImpls::EngineDataPImpl
{
    IronMan::EngineData * m_pImpl;
    EnginePImpls::ItemsPImpl* m_items;
    EnginePImpls::UiPImpl* m_ui;

public:
    CEngineDataPImpl(LPCWSTR szFileName)
        : m_pImpl(new IronMan::EngineData(IronMan::EngineData::CreateEngineData(szFileName))) // logger?
        , m_items(NULL)
        , m_ui(NULL)
    {}
    static EnginePImpls::EngineDataPImpl* MakePImpl(LPCWSTR szFileName) { return new CEngineDataPImpl(szFileName); }
    virtual ~CEngineDataPImpl()
    {
        delete m_pImpl;
        delete m_items;
        delete m_ui;
    }
    virtual EnginePImpls::ItemsPImpl& GetItems()
    {
        if (m_items == NULL)
            m_items = new CItemsPImpl(m_pImpl->GetItems());
        return *m_items;
    }
    virtual EnginePImpls::UiPImpl& GetUi()
    {
        if (m_ui == NULL)
            m_ui = new CUiPImpl(m_pImpl->GetUi());
        return *m_ui;
    }
};

}
