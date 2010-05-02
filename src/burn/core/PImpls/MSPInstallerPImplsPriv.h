//-------------------------------------------------------------------------------------------------
// <copyright file="MSPInstallerPImplsPriv.h" company="Microsoft">
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

#include "MSPInstallerPImpls.h"
#include "..\MSPInstaller.h"
#include "..\MSPUninstaller.h"
#include "Interfaces\ILogger.h"
#include "..\Ux\Ux.h"

namespace MSPInstallerPImplsPrivate
{
    struct MockFilesInUse : public IronMan::IFilesInUse
    {
        MockFilesInUse() {}
        virtual void DestroySelf() {}
        virtual IFilesInUse::Result Run(LPCWSTR arrayOfNames[], unsigned int count) { return IronMan::IFilesInUse::Ignore; }
    };

    class CMSPInstallerPImpl : public MSPInstallerPImpls::MSPInstallerPImpl
    {
        IronMan::IPerformer* m_performer;
        IronMan::IFilesInUse& m_fileInUse;
        IronMan::ILogger& m_log;
        IronMan::Ux m_uxLogger;
        IronMan::NullMetrics nullMetrics;
        MockFilesInUse m_mockFilesInUse;
        IronMan::FailureActionEnum m_subFailureAction;
        IronMan::MSP m_msp;

    public:
        CMSPInstallerPImpl( CComPtr<IXMLDOMElement> mspElement,IronMan::ILogger& logger,IronMan::FailureActionEnum m_subFailureAct, Action action) 
            : m_msp(mspElement, logger)
            , m_fileInUse(m_mockFilesInUse)
            , m_log(logger)
            , m_uxLogger(IronMan::NullLogger::GetNullLogger(), nullMetrics)
            , m_subFailureAction(m_subFailureAct)
       {
            switch (action)
            {
            case Install:
                m_performer = new IronMan::MspInstaller(m_msp
                                                        , m_fileInUse
                                                        , m_subFailureAction
                                                        , IronMan::UxEnum::ptNone
                                                        , m_log
                                                        , m_uxLogger);
                break;
            case Uninstall:
                m_performer = new IronMan::MspUninstaller(m_msp
                                                            , m_fileInUse
                                                            , m_subFailureAction
                                                            , m_log
                                                            , m_uxLogger);
                break;
            }
        }

        static MSPInstallerPImpls::MSPInstallerPImpl* MakePImpl(CComPtr<IXMLDOMElement> mspElement,IronMan::ILogger& logger, IronMan::FailureActionEnum m_subFailureAct, Action action) 
        { 
            return new CMSPInstallerPImpl( mspElement, logger, m_subFailureAct, action);
        }

        virtual void MSPInstallerPImpls::MSPInstallerPImpl::PerformAction(IronMan::IProgressObserver& observer)
        {
            m_performer->PerformAction(observer);
        }

        virtual void MSPInstallerPImpls::MSPInstallerPImpl::Abort()
        {
            m_performer->Abort();
        }
        
        virtual ~CMSPInstallerPImpl()
        {
            delete m_performer;         
        }       
    };
}
