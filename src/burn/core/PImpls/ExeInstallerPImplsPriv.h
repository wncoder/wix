//-------------------------------------------------------------------------------------------------
// <copyright file="ExeInstallerPImplsPriv.h" company="Microsoft">
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

#include "ExeInstallerPImpls.h"
#include "..\ExeInstaller.h"
#include "Interfaces\ILogger.h"
#include "Interfaces\IProvideDataToEngine.h"
#include "..\Watson\WatsonDefinition.h"

namespace ExeInstallerPImplsPrivate
{	

    class CExeInstallerPImpl : public ExeInstallerPImpls::ExeInstallerPImpl
    {
        IronMan::IPerformer* m_performer;		
        IronMan::Exe m_Exe;		
        IronMan::ILogger& m_log;		
        
    public:
        CExeInstallerPImpl(CComPtr<IXMLDOMElement> exeElement, IronMan::IProvideDataToEngine* pProvideDataToEngine,IronMan::ILogger& logger, Action action):
          m_Exe(exeElement, logger)		
          ,m_log(IronMan::NullLogger::GetNullLogger())
        {
            IronMan::NullMetrics* nullMetrics = new IronMan::NullMetrics();
            IronMan::ILogger* log = new IronMan::TextLogger();

            IronMan::WatsonData::WatsonDataStatic() = new IronMan::WatsonData();

            IronMan::Ux* uxLogger = new IronMan::Ux(*log, *nullMetrics);
            switch (action)
            {
            case Install:
                m_performer = new IronMan::ExeInstaller(m_Exe, pProvideDataToEngine, m_log, *uxLogger);
                break;
            case Uninstall:
                m_performer = new IronMan::ExeUnInstaller(m_Exe, pProvideDataToEngine, m_log, *uxLogger);
                break;
            default:
                HIASSERT(false, L"Unknown Action!");
                break;
            }

            delete nullMetrics;
            delete log;
        }

        static ExeInstallerPImpls::ExeInstallerPImpl* MakePImpl(CComPtr<IXMLDOMElement> exeElement, IronMan::IProvideDataToEngine* pProvideDataToEngine, IronMan::ILogger& logger, Action action) 
        { 
            return new CExeInstallerPImpl(exeElement, pProvideDataToEngine, logger, action);		
        }

        virtual void ExeInstallerPImpls::ExeInstallerPImpl::PerformAction(IronMan::IProgressObserver& observer)
        {
            m_performer->PerformAction(observer);
        }

        virtual void ExeInstallerPImpls::ExeInstallerPImpl::Abort()
        {
            m_performer->Abort();
        }
        
        virtual ~CExeInstallerPImpl()
        {
            delete m_performer;			
        }		
    };
}
