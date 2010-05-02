//-------------------------------------------------------------------------------------------------
// <copyright file="AgileMSIInstallerPImplsPriv.h" company="Microsoft">
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

#include "AgileMsiInstallerPImpls.h"
#include "..\AgileMsiInstaller.h"
#include "Interfaces\ILogger.h"
#include "..\Ux\Ux.h"

namespace AgileMsiInstallerPImplsPrivate
{
    struct MockFilesInUse : public IronMan::IFilesInUse
    {
        MockFilesInUse() {}
        virtual void DestroySelf() {}
        virtual IFilesInUse::Result Run(LPCWSTR arrayOfNames[], unsigned int count) { return IronMan::IFilesInUse::Ignore; }
    };

    class CAgileMsiInstallerPImpl : public AgileMsiInstallerPImpls::AgileMsiInstallerPImpl
    {
        IronMan::IPerformer* m_performer;        
        IronMan::AgileMSI m_agilemsi;
        IronMan::IFilesInUse& m_fileInUse;
        IronMan::ILogger& m_log;
        IronMan::Ux m_uxLogger;
        IronMan::NullMetrics nullMetrics;
        MockFilesInUse m_mockFilesInUse;

    public:
        CAgileMsiInstallerPImpl(CComPtr<IXMLDOMElement> agileMsiElement,IronMan::ILogger& logger, Action action):
          m_agilemsi(agileMsiElement, logger)
          ,m_fileInUse(m_mockFilesInUse)
          ,m_log(logger)
          , m_uxLogger(IronMan::NullLogger::GetNullLogger(), nullMetrics)
        {
            switch (action)
            {
            case Install:
                m_performer = new IronMan::AgileMsiInstaller(m_agilemsi, m_fileInUse, m_log, m_uxLogger);
                break;
            case Uninstall:
                m_performer = new IronMan::AgileMsiUnInstaller(m_agilemsi, m_fileInUse, m_log, m_uxLogger);
                break;
            }         
        }

        static AgileMsiInstallerPImpls::AgileMsiInstallerPImpl* MakePImpl(CComPtr<IXMLDOMElement> agileMsiElement,IronMan::ILogger& logger, Action action) 
        { 
            return new CAgileMsiInstallerPImpl(agileMsiElement, logger, action);       
        }

        virtual void AgileMsiInstallerPImpls::AgileMsiInstallerPImpl::PerformAction(IronMan::IProgressObserver& observer)
        {
            m_performer->PerformAction(observer);
        }

        virtual void AgileMsiInstallerPImpls::AgileMsiInstallerPImpl::Abort()
        {
            m_performer->Abort();
        }
        
        virtual ~CAgileMsiInstallerPImpl()
        {
            delete m_performer;         
        }       
    };
}
