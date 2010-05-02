//-------------------------------------------------------------------------------------------------
// <copyright file="CompositePerformerPImplsPriv.h" company="Microsoft">
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

#include "CompositePerformerPImpls.h"
#include "schema\EngineData.h"
#include "Coordinator.h"
#include "..\CompositePerformer.h"
#include "..\TextLogger.h"
#include "..\Ux\Ux.h"
#include "OperationData.h"

namespace CompositePerformerPImplsPrivate
{
//------------------------------------------------------------------------------
// Class: CCompositePerformerPImpl
//
// This class wraps the actual implementation of CompositePerformer
//------------------------------------------------------------------------------
    class CCompositePerformerPImpl : public CompositePerformerPImpls::CompositePerformerPImpl
    {
        IronMan::CompositePerformer* m_pImpl;
        IronMan::EngineData * m_EngineData;
        IronMan::Coordinator* m_coordinator;
        IronMan::ILogger* m_log;
        IronMan::NullMetrics* m_nullMetrics; 
        IronMan::Ux* m_UxLogger;

    public:
        //LPCWSTR szFileName:  file name of IronManEngine data file
        //IronMan::IFilesInUse& fileInUse:  implementation of IFileInUse; used when file is in use
        CCompositePerformerPImpl(LPCWSTR szFileName
                                    , const IronMan::IOperationData& operationData
                                    , const IronMan::IProvideDataToUi& dataToUi
                                    , IronMan::IFilesInUse& fileInUse
                                    , IronMan::IMsiBusy& msiBusy)
        {
            m_log = new IronMan::TextLogger();

            //create EngineData object based on input XML
            m_EngineData = new IronMan::EngineData(IronMan::EngineData::CreateEngineData(szFileName));
            
            //get list of items to be performed
            m_coordinator = new IronMan::Coordinator(m_EngineData->GetItems()
                , IronMan::Operation::uioInstalling // TODO:  REVIEW:  QA guys, do you want to pass this in?
                , operationData.GetDataToOperandData()
                , *m_log);

            m_nullMetrics = new IronMan::NullMetrics();
            m_UxLogger = new IronMan::Ux(*m_log, *m_nullMetrics);
            //create real instance of CompositePerformer
            m_pImpl = new IronMan::CompositePerformer(
                IronMan::Operation::uioInstalling // TODO:  REVIEW:  QA guys, do you want to pass this in?
                , *m_coordinator // ICoordinator
                , operationData
                , dataToUi
                , fileInUse//IFilesInUse
                , msiBusy
                , IronMan::FailureActionEnum::GetRollbackAction()
                , *m_log//ILogger
                , *m_UxLogger
                );
        }

        //static class factory that returns an instance of the CompositePerformer Pimpl
        static CompositePerformerPImpls::CompositePerformerPImpl* MakePImpl(LPCWSTR szFileName, IronMan::IFilesInUse& fileInUse)
        {
            // TEMPORARY HACK ONLY!!!!
            struct NullObjectMsiBusy : public IronMan::IMsiBusy
            {
                virtual void DestroySelf() {}
                virtual Result Run() { return IMsiBusy::Cancel; }
            };
            static NullObjectMsiBusy nomb;

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

            class MockEngineDataProvider : public IronMan::IProvideDataToUi
            {
                const CString m_packageName;
                CSimpleArray<CString> m_products;
                CSimpleArray<CString> m_imageNames;
                CSimpleArray<CString> m_serviceNames;

            private:
                virtual IronMan::Operation::euiOperation GetOperation() { return IronMan::Operation::uioInstalling; }
                virtual bool  InitializeItems() {return true;}
                virtual HRESULT Detect() { return S_OK; }
                virtual HRESULT Plan(__in BURN_ACTION action) { return S_OK; }
                virtual bool  GetItemValidationState(const bool &bStopProcessing) {return true;}
                virtual const CSimpleArray<CString>& GetAffectedProducts(const bool &bStopProcessing) { return m_products; }
                virtual IronMan::ILogger& GetEngineLogger() { return IronMan::NullLogger::GetNullLogger(); }
                virtual IronMan::Ux& GetUxLogger() { throw(E_UNEXPECTED);  }
                virtual const CString & GetPackageName() { return m_packageName; }
                virtual const CSimpleArray<CString>& GetImageNamesToBlockOn() { return m_imageNames; }
                virtual const CSimpleArray<CString>& GetServiceNamesToBlockOn() { return m_serviceNames; }
                virtual const ULONGLONG GetSpaceRequiredForDownload() { return 0; }
                virtual const ULONGLONG GetSpaceRequiredOnProductInstalledDrive() { return 1; }
                virtual const ULONGLONG GetSpaceRequiredOnSystemDrive() { return 0; }
                virtual const WCHAR GetProductDriveLetter() const { return L'C'; }
                virtual void DismissSplashScreen(void) const {}
                virtual void UpdateLiveOperation(IronMan::Operation::euiOperation) {}
                virtual IronMan::IInstallItems& GetInstallItems() {throw(E_UNEXPECTED); }
                virtual IronMan::IDownloadItems& GetDownloadItems() {throw(E_UNEXPECTED); }
                virtual bool IsSimultaneousDownloadAndInstallDisabled() const { return false; }
                virtual const IronMan::IProvideDataToOperand& GetDataToOperand() const {throw(E_UNEXPECTED); }
                virtual const IronMan::BlockersElement& GetBlocks() const {throw(E_UNEXPECTED); }
                virtual unsigned int GetAuthoredItemCount(IronMan::ItemBase::ItemType itemType) const {throw(E_UNEXPECTED); }
                virtual unsigned int GetAuthoredItemCount() const {throw(E_UNEXPECTED); }
#ifdef FeaturesAreImplented
// Features feature not implemented
                virtual IFeatureTreeRoot& GetFeatureTreeRoot()
                {
                    struct MockFeatureTreeRoot : public IFeatureTreeRoot
                    {
                        virtual unsigned int GetNumberOfChildren() { return 0; }
                        virtual IFeature* GetChild(unsigned int) { return NULL; }
                    };
                    static MockFeatureTreeRoot s_mftr;
                    return s_mftr;
                }
#endif // FeaturesAreImplented
                virtual IronMan::INotifyEngine& GetEngineNotificationInterface()
                {
                    struct MockNotifyEngine : public IronMan::INotifyEngine
                    {
#ifdef FeaturesAreImplented
// Features feature not implemented
                        virtual void SetFeatureTreeRoot(IFeatureTreeRoot&) {}
#endif // FeaturesAreImplented
                    };
                    static MockNotifyEngine s_mne;
                    return s_mne;
                }
                virtual const IronMan::UserExperienceDataCollection::Policy UxCollectionPolicy() const
                {
                    return IronMan::UserExperienceDataCollection::UserControlled;
                }
                virtual IronMan::IBlockChecker& GetBlockChecker()
                {
                    struct MockResult : public IronMan::IBlockChecker::IResult
                    {
                        virtual bool SuccessBlockerWasHit() { return false; }
                        virtual bool StopBlockerWasHit(HRESULT& hrReturnCode) { return false; }
                        virtual bool WarnBlockerWasHitAndUserCanceled() { return false; }
                        virtual bool WarnBlockerWasHit() { return false; }
                    };
                    struct MockBlockChecker : public IronMan::IBlockChecker
                    {
                        MockResult m_result;
                        virtual IResult& ProcessBlocks(HWND hWndParent=NULL)
                        {
                            return m_result;
                        }
                    };
                    static MockBlockChecker s_mbc;
                    return s_mbc;
                }

            public:
                MockEngineDataProvider()
                    : m_packageName(L"test package")
                {
                    m_products.Add(L"product A");
                    m_products.Add(L"product B");
                    m_imageNames.Add(L"blockOnMe.exe");
                    m_serviceNames.Add(L"serviceToBlockOn");
                }
            } mockProvideDataToUi;


            class MockPackageData : public IronMan::IPackageData
            {
                const CString m_strPackageName;
                const CString m_strVersion;

            public:
                MockPackageData()
                    : m_strPackageName(L"")
                    , m_strVersion(L"")
                    {}

                virtual const CString& GetPackageName() const
                {
                    return m_strPackageName;
                }

                virtual const CString& GetVersion() const
                {
                    return m_strVersion;
                }

                virtual const IronMan::UxEnum::patchTrainEnum GetServicingTrain() const
                {
                    return IronMan::UxEnum::ptGDR;
                }

                virtual const IronMan::UserExperienceDataCollection::Policy GetPolicy() const
                {
                    return IronMan::UserExperienceDataCollection::Disabled;
                }
            };

            MockPackageData packageData;
            IronMan::OperationData operationData(dataToOperand, packageData);
            return new CCompositePerformerPImpl(szFileName, operationData, mockProvideDataToUi, fileInUse, nomb);
        }

        //delegate Abort to the real CompositePerformer
        virtual void Abort()
        {
            m_pImpl->Abort();
        }

        //delegate PerformAction to the real CompositePerformer
        virtual void PerformAction(IronMan::IProgressObserver& observer)
        {
            m_pImpl->PerformAction(observer);
        }

        virtual ~CCompositePerformerPImpl()
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
