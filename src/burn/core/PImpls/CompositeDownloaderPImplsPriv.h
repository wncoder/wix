//-------------------------------------------------------------------------------------------------
// <copyright file="CompositeDownloaderPImplsPriv.h" company="Microsoft">
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

#include "CompositeDownloaderPImpls.h"
#include "CompositeDownloader.h"
#include "..\TextLogger.h"


namespace CompositeDownloaderPImplsPrivate
{
    //------------------------------------------------------------------------------
    // Class: NotifyControllerPImpl
    //
    // This class wraps the actual implementation of NotifyController
    //------------------------------------------------------------------------------
    class CCompositeDownloaderPImpl : public CompositeDownloaderPImpls::CompositeDownloaderPImpl
    {
        IronMan::EngineData * m_EngineData;
        IronMan::Coordinator* m_coordinator;
        IronMan::CompositeDownloader* m_pImpl;
        IronMan::ILogger* m_log;     
        IronMan::NullMetrics* m_nullMetrics; 
        IronMan::Ux* m_UxLogger;     


    public:
        CCompositeDownloaderPImpl(LPCWSTR szFileName, const IronMan::IProvideDataToOperand& dataToOperand, int retries, int secondsToWait) 		
        {
            m_log = new IronMan::TextLogger();

            //create EngineData object based on input XML
            m_EngineData = new IronMan::EngineData(IronMan::EngineData::CreateEngineData(szFileName));

            //get list of items to be performed
            m_coordinator = new IronMan::Coordinator(m_EngineData->GetItems()
                , IronMan::Operation::uioInstalling // TODO:  REVIEW:  QA guys, do you want to pass this in?
                , dataToOperand
                , *m_log);

            m_nullMetrics = new IronMan::NullMetrics();
            m_UxLogger = new IronMan::Ux(*m_log, *m_nullMetrics);

            //create real instance of CompositePerformer
            m_pImpl = new IronMan::CompositeDownloader(
                *m_coordinator, // ICoordinator
                retries, 
                secondsToWait,
                true,
                *m_log,//ILogger
                *m_UxLogger
                );
        }

        //static class factory that returns an instance of the NotifyController Pimpl
        static CompositeDownloaderPImpls::CompositeDownloaderPImpl* MakePImpl(LPCWSTR szFileName, int retries, int secondsToWait)
        {
            class DummyDataToOperand : public IronMan::IProvideDataToOperand
            {                
                virtual const CString GetLcid() const
                {
                    return L"1033";
                }

                virtual const CString GetChainerMode() const
                {
                    return L"Installing";
                }
            };  
            static DummyDataToOperand dataToOperand;

            return new CCompositeDownloaderPImpl(szFileName, dataToOperand, retries, secondsToWait);  //, fileInUse
        }

        //delegate Abort to the real NotifyController
        virtual void Abort()
        {
            m_pImpl->Abort();
        }

        //delegate MayBegin to the real NotifyController
        virtual void PerformAction(IronMan::IProgressObserver& observer)
        {
            m_pImpl->PerformAction(observer);
        }

        virtual ~CCompositeDownloaderPImpl()
        {
            delete m_pImpl;
            delete m_EngineData;
            delete m_coordinator;
            delete m_nullMetrics;
            delete m_UxLogger;
            delete m_log;
        }
    };
}
