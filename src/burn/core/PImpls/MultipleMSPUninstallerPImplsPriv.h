//-------------------------------------------------------------------------------------------------
// <copyright file="MultipleMSPUninstallerPImplsPriv.h" company="Microsoft">
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

#include "MultipleMSPUninstallerPImpls.h"
#include "..\MSPUninstaller.h"
#include "Interfaces\ILogger.h"
#include "..\Ux\Ux.h"

namespace MultipleMSPUninstallerPImplsPrivate
{
    struct MockFilesInUse : public IronMan::IFilesInUse
    {
        MockFilesInUse() {}
        virtual void DestroySelf() {}
        virtual IFilesInUse::Result Run(LPCWSTR arrayOfNames[], unsigned int count) { return IronMan::IFilesInUse::Ignore; }
    };

    class CMultipleMSPUninstallerPImpl : public MultipleMSPUninstallerPImpls::MultipleMSPUninstallerPImpl
    {
        IronMan::IPerformer* m_performer;        
        IronMan::IFilesInUse& m_fileInUse;
        IronMan::ILogger& m_log;
        IronMan::Ux m_uxLogger;
        IronMan::NullMetrics nullMetrics;
        MockFilesInUse m_mockFilesInUse;		
        IronMan::FailureActionEnum m_subFailureAction;
        IronMan::Patches m_patches;

        public:
        CMultipleMSPUninstallerPImpl( CComPtr<IXMLDOMElement> patchesElement, IronMan::FailureActionEnum m_subFailureAct
          , IronMan::ILogger& logger):
           m_patches(patchesElement, logger)
          ,m_fileInUse(m_mockFilesInUse)
          ,m_log(logger)
          ,m_uxLogger(IronMan::NullLogger::GetNullLogger(), nullMetrics)
          ,m_subFailureAction(m_subFailureAct)
        {
            m_performer = new IronMan::MspUninstaller(m_patches, m_fileInUse, m_subFailureAction, m_log, m_uxLogger);
        }

        static MultipleMSPUninstallerPImpls::MultipleMSPUninstallerPImpl* MakePImpl( CComPtr<IXMLDOMElement> patchesElement, IronMan::FailureActionEnum m_subFailureAct
            ,IronMan::ILogger& logger) 
        { 
            return new CMultipleMSPUninstallerPImpl( patchesElement, m_subFailureAct, logger);       
        }

        virtual void MultipleMSPUninstallerPImpls::MultipleMSPUninstallerPImpl::PerformAction(IronMan::IProgressObserver& observer)
        {
            m_performer->PerformAction(observer);
        }

        virtual void MultipleMSPUninstallerPImpls::MultipleMSPUninstallerPImpl::Abort()
        {
            m_performer->Abort();
        }
        
        virtual ~CMultipleMSPUninstallerPImpl()
        {
            delete m_performer;         
        }       
    };
}
