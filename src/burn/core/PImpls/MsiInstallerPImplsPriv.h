//-------------------------------------------------------------------------------------------------
// <copyright file="MsiInstallerPImplsPriv.h" company="Microsoft">
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

#include "MsiInstallerPImpls.h"
#include "..\MsiInstaller.h"
#include "Interfaces\ILogger.h"
#include "..\Ux\Ux.h"

namespace MsiInstallerPImplsPrivate
{
    struct MockFilesInUse : public IronMan::IFilesInUse
    {
        MockFilesInUse() {}
        virtual void DestroySelf() {}
        virtual IFilesInUse::Result Run(LPCWSTR arrayOfNames[], unsigned int count) { return IronMan::IFilesInUse::Ignore; }
    };

    class CMsiInstallerPImpl : public MsiInstallerPImpls::MsiInstallerPImpl
    {
        IronMan::IPerformer* m_performer;        
        IronMan::MSI m_msi;
        IronMan::IFilesInUse& m_fileInUse;
        IronMan::ILogger& m_log;
        IronMan::Ux m_uxLogger;
        IronMan::NullMetrics nullMetrics;
        MockFilesInUse m_mockFilesInUse;

    public:
        CMsiInstallerPImpl(CComPtr<IXMLDOMElement> msiElement,IronMan::ILogger& logger, Action action):
          m_msi(msiElement, logger)
          ,m_fileInUse(m_mockFilesInUse)
          ,m_log(logger)
          , m_uxLogger(IronMan::NullLogger::GetNullLogger(), nullMetrics)
        {
            switch (action)
            {
            case Install:
                m_performer = new IronMan::MsiInstaller(m_msi, m_fileInUse, m_log, m_uxLogger);
                break;
            case Uninstall:
                m_performer = new IronMan::MsiUnInstaller(m_msi, m_fileInUse, m_log, m_uxLogger);
                break;
            }         
        }

        static MsiInstallerPImpls::MsiInstallerPImpl* MakePImpl(CComPtr<IXMLDOMElement> msiElement,IronMan::ILogger& logger, Action action) 
        { 
            return new CMsiInstallerPImpl(msiElement, logger, action);       
        }

        virtual void MsiInstallerPImpls::MsiInstallerPImpl::PerformAction(IronMan::IProgressObserver& observer)
        {
            m_performer->PerformAction(observer);
        }

        virtual void MsiInstallerPImpls::MsiInstallerPImpl::Abort()
        {
            m_performer->Abort();
        }
        
        virtual ~CMsiInstallerPImpl()
        {
            delete m_performer;         
        }       
    };
}
