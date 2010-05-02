//-------------------------------------------------------------------------------------------------
// <copyright file="CoordinatorPimplsPriv.h" company="Microsoft">
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

#include "CoordinatorPimpls.h"
#include "schema\EngineData.h"
#include "Coordinator.h"
//#include "..\CompositePerformer.h"
#include "..\TextLogger.h"
////#include "interfaces\ILogger.h"  //For NullLogger
//#include "..\Ux\Ux.h"

namespace CoordinatorPImplsPrivate
{
//------------------------------------------------------------------------------
// Class: CCoordinatorPImpl
//
// This class wraps the actual implementation of Coordinator class
//------------------------------------------------------------------------------
    class CCoordinatorPImpl : public CoordinatorPImpls::CoordinatorPImpl
    {
        IronMan::Coordinator* m_pImpl;
        IronMan::EngineData * m_EngineData;
        IronMan::ILogger* m_log;   
        EnginePImpls::ItemsPImpl* m_items;
    public:
        //LPCWSTR szFileName:  file name of IronManEngine data file
        //IronMan::IFilesInUse& fileInUse:  implementation of IFileInUse; used when file is in use
        CCoordinatorPImpl(LPCWSTR szFileName, const IronMan::IProvideDataToOperand& dataToOperand)			
        {
            m_log = new IronMan::TextLogger();

            //create EngineData object based on input XML
            m_EngineData = new IronMan::EngineData(IronMan::EngineData::CreateEngineData(szFileName));
            
            //get list of items to be performed
            m_pImpl = new IronMan::Coordinator(m_EngineData->GetItems()
                , IronMan::Operation::uioInstalling // TODO:  REVIEW:  QA guys, do you want to pass this in?
                , dataToOperand
                , *m_log);
        }

        //static class factory that returns an instance of the Coordinator Pimpl
        static CoordinatorPImpls::CoordinatorPImpl* MakePImpl(LPCWSTR szFileName)
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

            return new CCoordinatorPImpl(szFileName, dataToOperand);
        }
 
        virtual IronMan::IDownloadItems& GetDownloadItems()
        {
            return m_pImpl->GetDownloadItems();
        }

        virtual IronMan::IInstallItems& GetInstallItems()
        {
            return m_pImpl->GetInstallItems();
        }
        
        virtual ~CCoordinatorPImpl()
        {
            delete m_pImpl;
            delete m_EngineData;
            //delete m_nullMetrics;
            //delete m_UxLogger;
            delete m_log;
        }
    };
}
