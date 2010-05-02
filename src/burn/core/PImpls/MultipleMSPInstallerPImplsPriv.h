//-------------------------------------------------------------------------------------------------
// <copyright file="MultipleMSPInstallerPImplsPriv.h" company="Microsoft">
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

#include "MultipleMSPInstallerPImpls.h"
#include "..\MSPInstaller.h"
#include "Interfaces\ILogger.h"
#include "..\Ux\Ux.h"

namespace MultipleMSPInstallerPImplsPrivate
{
    struct MockFilesInUse : public IronMan::IFilesInUse
    {
        MockFilesInUse() {}
        virtual void DestroySelf() {}
        virtual IFilesInUse::Result Run(LPCWSTR arrayOfNames[], unsigned int count) { return IronMan::IFilesInUse::Ignore; }
    };

    class CMultipleMSPInstallerPImpl : public MultipleMSPInstallerPImpls::MultipleMSPInstallerPImpl
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
        CMultipleMSPInstallerPImpl( CComPtr<IXMLDOMElement> patchesElement
                                    , IronMan::FailureActionEnum m_subFailureAct
                                    , IronMan::ILogger& logger)
                : m_patches(patchesElement, logger)
                , m_fileInUse(m_mockFilesInUse)
                , m_log(logger)
                , m_uxLogger(IronMan::NullLogger::GetNullLogger(), nullMetrics)
                , m_subFailureAction(m_subFailureAct)
        {
            m_performer = new IronMan::MspInstaller(IronMan::PatchesFiltered(m_patches, m_log)
                                                    , m_fileInUse
                                                    , m_subFailureAction
                                                    , IronMan::UxEnum::ptNone
                                                    , m_log
                                                    , m_uxLogger); 
        }

        static MultipleMSPInstallerPImpls::MultipleMSPInstallerPImpl* MakePImpl( CComPtr<IXMLDOMElement> patchesElement
                                                                                , IronMan::FailureActionEnum m_subFailureAct
                                                                                ,IronMan::ILogger& logger) 
        {
            return new CMultipleMSPInstallerPImpl( patchesElement, m_subFailureAct, logger);
        }

        virtual void MultipleMSPInstallerPImpls::MultipleMSPInstallerPImpl::PerformAction(IronMan::IProgressObserver& observer)
        {
            m_performer->PerformAction(observer);
        }

        virtual void MultipleMSPInstallerPImpls::MultipleMSPInstallerPImpl::Abort()
        {
            m_performer->Abort();
        }
        
        virtual ~CMultipleMSPInstallerPImpl()
        {
            delete m_performer;
        }
    };
}
