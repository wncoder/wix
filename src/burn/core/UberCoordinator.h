//-------------------------------------------------------------------------------------------------
// <copyright file="UberCoordinator.h" company="Microsoft">
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

#include "LiveOperation.h"
#include "common\Operation.h"
#include "Coordinator.h"
#include "LayoutList.h"
#include "TargetPackages.h"
#include "Ux\Ux.h"
#include "interfaces\IDataProviders.h"
#include "BlockChecker.h"
#include "inc\IBurnView.h"
#include "BurnController.h"

namespace IronMan
{
template<typename FileAuthenticity>
class UberCoordinatorT : public ICoordinator, private IProvideDataToUi, private INotifyEngine
{
    bool m_bInitialized;
    CRITICAL_SECTION m_cs;
    const CString m_packageName;
    mutable CSimpleArray<CString> m_targetPackageNames;
    const CSimpleArray<CString> m_imageBlocks;
    const CSimpleArray<CString> m_serviceBlocks;
    const WCHAR m_productDriveLetter;
    const UserExperienceDataCollection::Policy  m_uxPolicy;
    const bool m_bForceSerialDownload;

    const LayoutList m_nonLayoutList;
    const LayoutList m_layoutList;
    CoordinatorT<FileAuthenticity> m_install;
    CoordinatorT<FileAuthenticity> m_uninstall;
    CoordinatorT<FileAuthenticity> m_repair;
    CoordinatorT<FileAuthenticity> m_layout;
    LiveOperation m_liveOperation;
    ILogger& m_logger;
    Ux& m_uxLogger;
    const IProvideDataToOperand& m_dataToOperand;
    const BlockersElement& m_blocker;
    IBlockChecker& m_blockChecker;
    unsigned int m_authoredItemCount;
    IBurnView* m_pBurnView;
    CBurnController* m_pBurnCore;
    bool m_bRunningElevated;

#ifdef FeaturesAreImplented
    mutable Features m_features;
#endif //FeaturesAreImplented

    static CPath MungeCreateLayoutFolder(const CPath& layoutFolder)
    {
        /*
        Problem:  this class exists because we don't necessarily know up front what mode we're in.
        Originally, in earlier versions we knew whether we were installing or uninstalling, but now that we have Repair,
        we offer the user the choice to repair /or/ uninstall, and we need to be reactive to that.  In the future, we'll need to add
        the capability to add or remove featuers, too, so we could even get into an 'install' mode.

        So, this class creates, upfront, all the possible lists that we need to do any of the operations.

        Real problem:  unfortunately, this only sort of makes sense for /CreateLayout.  If we're explicitly NOT in CreateLayout mode,
        I can't create the layout item list correctly.  Why?  Because I need to look in two places for local or already
        downloaded items:  in the current directory and in the user-specified layout directory.  But the latter doesn't 
        exist (it's an empty string, in fact).

        This method exists to overcome this problem.
        */

        if (CString(layoutFolder).IsEmpty())
            return ModuleUtils::GetDllPath(); // look only in local folder
        else
            return layoutFolder;
    }

public:
    UberCoordinatorT(
        const EngineData& engineData
        , IBlockChecker& blockChecker
        , const IProvideDataToOperand& dataToOperand
        , const CPath& nonLayoutFolder
        , const CPath& layoutFolder
        , Operation::euiOperation initialOperation
        , ILogger& logger
        , Ux& uxLogger
        )
        : m_bInitialized(false)
        , m_blockChecker(blockChecker)
        , m_liveOperation(initialOperation)
        , m_dataToOperand(dataToOperand)
        , m_nonLayoutList(engineData.GetItems(),                       nonLayoutFolder,  true, logger)
        , m_layoutList   (engineData.GetItems(), MungeCreateLayoutFolder(layoutFolder), false, logger)
        , m_install      (m_nonLayoutList.GetItems(), Operation::uioInstalling, dataToOperand, logger)
        , m_uninstall    (m_nonLayoutList.GetItems(), Operation::uioUninstalling, dataToOperand, logger)
        , m_repair       (m_nonLayoutList.GetItems(), Operation::uioRepairing, dataToOperand, logger)
        , m_layout       (   m_layoutList.GetItems(), Operation::uioCreateLayout, dataToOperand, logger)
        , m_logger       (logger)
        , m_uxLogger     (uxLogger)
#ifdef FeaturesAreImplented
        , m_features     (logger)
#endif //FeaturesAreImplented
        , m_packageName  (engineData.GetUi().GetName())
        , m_imageBlocks  (engineData.GetSystemCheck().GetProcessBlocks().GetImageNamesToBlockOn())
        , m_serviceBlocks(engineData.GetSystemCheck().GetServiceBlocks().GetServiceNamesToBlockOn())
        , m_productDriveLetter(engineData.GetSystemCheck().GetProductDriveHints().EvaluateHints())
        , m_uxPolicy(engineData.GetConfiguration().GetUserExperienceDataCollectionData().GetPolicy())
        , m_bForceSerialDownload(engineData.GetConfiguration().IsSimultaneousDownloadAndInstallDisabled())
        , m_blocker(engineData.GetBlocks())
        , m_authoredItemCount(engineData.GetItems().GetCount())
        , m_pBurnView(NULL)
        , m_pBurnCore(NULL)
        , m_bRunningElevated(false)
    {
        InitializeCriticalSection(&m_cs);
    }

    virtual ~UberCoordinatorT() 
    {
        DeleteCriticalSection(&m_cs);
    }

    // This method needs to be called before using any of the items are used.
    virtual bool  InitializeItems()
    {
#ifdef ON_DEVELOPER_MC_ONLY_2
        {
            UINT ret = ::MessageBox(0, L"Click Cancel to return false, OK to continue", L"UberCoordinator::InitializeItems", MB_OKCANCEL);
            if (ret == IDCANCEL)
                return false;
        }
#endif 
        bool fLogApplicableCount = false;
        unsigned int uiItemCount = 0;
        EnterCriticalSection(&m_cs);
        if (!m_bInitialized)
        {
            m_uxLogger.StartApplicableIfTime();
            // We need to calculate state and Get install/download items for only
            // live operation - this operation remains constant for each install session.
            switch(m_liveOperation.GetOperationFromUi())
            {
            case Operation::uioInstalling  : 
                uiItemCount = m_install.GetInstallItems().GetCount();
                m_install.GetDownloadItems();
                break;
            case Operation::uioUninstalling: 
                uiItemCount = m_uninstall.GetInstallItems().GetCount();
                m_uninstall.GetDownloadItems();
                break;
            case Operation::uioRepairing   :
                m_repair.GetDownloadItems();
                uiItemCount = m_repair.GetInstallItems().GetCount();
                break;
            case Operation::uioCreateLayout:
                uiItemCount = m_layout.GetInstallItems().GetCount();
                m_layout.GetDownloadItems();
                break;
            default:
                IMASSERT(false && L"incompatible operation");
                throw E_FAIL;
            }

            m_bInitialized = true;
            fLogApplicableCount = true;
            m_uxLogger.StopApplicableIfTime();
        }

        LeaveCriticalSection(&m_cs);

        // Make this a final result summary message too
        if (fLogApplicableCount)
        {
            CString section = L" ";
            PUSHLOGSECTIONPOP(m_logger, L"Applicability Result Count", L" ", section);
            CString strMessage;
            strMessage.Format(L"Number of applicable items: %u",uiItemCount);
            LOG(m_logger, ILogger::Result, strMessage);
        }

        return m_bInitialized;
    }

    // UX calls this method to detect all the applicable items (packages).
    virtual HRESULT Detect()
    {
        IMASSERT(m_pBurnView != NULL);

        HRESULT hr = m_install.Detect(m_pBurnView);
        ExitOnFailure(hr, "Failed to Detect");

        if (m_bRunningElevated)
        {
            // Detect is enough in elevated process. Plan should not be performed.
            m_bInitialized = true;
        }

    LExit:
        return hr;
    }

    // UX calls this method to calculate Action State for each applicable package
    // UX can call Plan multiple times, and may change BURN_ACTION.
    virtual HRESULT Plan(__in BURN_ACTION action)
    {
        IMASSERT(m_pBurnView != NULL);

        HRESULT hr = S_OK;

        Operation::euiOperation newOperation = m_liveOperation.GetOperation();

        switch (action)
        {
        case BURN_ACTION_INSTALL:
            newOperation = Operation::uioInstalling;
            break;
        case BURN_ACTION_REPAIR:
            newOperation = Operation::uioRepairing;
            break;
        case BURN_ACTION_UNINSTALL:
            newOperation = Operation::uioUninstalling;
            break;
        case BURN_ACTION_MODIFY:
            newOperation = Operation::uioRepairing;
            break;
        default:
            hr = E_INVALIDARG;
            ExitOnFailure(hr, "Invalid action state.");
        }

        UpdateLiveOperation(newOperation);

        hr = m_install.Plan(action, m_pBurnView);
        ExitOnFailure(hr, "Failed to Plan");

        m_bInitialized = true;

    LExit:
        return hr;
    }

    virtual HRESULT SetSource(__in LPCWSTR pathSource)
    {
        SourceLocation::SetPath(pathSource);
        return S_OK;
    }

    // Called from IronMan.h with UiMode's Data provider. This is the 
    // provider that engine can use to get Ui Data
    void SetEngineDataProvider(IProvideDataToEngine* provider) 
    {
        m_liveOperation.SetEngineDataProvider(provider); 
    }

    void SetBurnView(__in IBurnView *pBurnView)
    {
        m_pBurnView = pBurnView;
        m_install.SetBurnView(pBurnView);
        m_uninstall.SetBurnView(pBurnView);
        m_repair.SetBurnView(pBurnView);
        m_layout.SetBurnView(pBurnView);
    }

    void SetBurnCore(__in CBurnController *pBurnCore)
    {
        m_pBurnCore = pBurnCore;
    }

    void SetRunningElevated()
    {
        m_bRunningElevated = true;
        m_install.SetRunningElevated();
    }

    // This object provides data to Ui
    IProvideDataToUi& GetDataProviderToUi() 
    { 
        return *this; 
    }

    // List of download items for the current (live) operation
    IDownloadItems& GetDownloadItems() 
    {
        if (!m_bInitialized)
            throw CObjectNotInitializedException(L"UberCoordinator");

        return GetDownloadItems(m_liveOperation.GetOperationFromUi()); 
    }

    IDownloadItems& GetDownloadItems(Operation::euiOperation op)
    {
        if (!m_bInitialized)
            throw CObjectNotInitializedException(L"UberCoordinator");

        if (m_pBurnView)
        {
            return m_install.GetDownloadItems();
        }

        switch(op)
        {
        case Operation::uioInstalling  : return   m_install.GetDownloadItems();
        case Operation::uioUninstalling: return m_uninstall.GetDownloadItems();
        case Operation::uioRepairing   : return    m_repair.GetDownloadItems();
        case Operation::uioCreateLayout: return    m_layout.GetDownloadItems();
        default:
            IMASSERT(false && L"incompatible operation");
            throw E_FAIL;
        }
    }

    // Called to validate that state of items is not "Error"
    bool ValidateStatefulItems() const
    {
        if (!m_bInitialized)
            throw CObjectNotInitializedException(L"UberCoordinator");

        // Validate list that current (live) operation requires.
        switch(m_liveOperation.GetOperationFromUi())
        {
        case Operation::uioInstalling  : return   m_install.ValidateStatefulItems();
        case Operation::uioUninstalling: return m_uninstall.ValidateStatefulItems();
        case Operation::uioRepairing   : return    m_repair.ValidateStatefulItems();
        case Operation::uioCreateLayout: return    m_layout.ValidateStatefulItems();
        default:
            IMASSERT(false && L"incompatible operation");
            throw E_FAIL;
        }
    }

        // Returns count of items authored
    virtual unsigned int GetAuthoredItemCount() const
    {
        return m_authoredItemCount;
    }

    // Returns count of items authored of the given type.
    virtual unsigned int GetAuthoredItemCount(ItemBase::ItemType itemType) const
    {
        return m_install.GetAuthoredItemCount(itemType);
    }

     virtual HRESULT GetBurnView(__in IBurnView** ppBurnView) const
     {
         if (!m_pBurnView)
             return E_INVALIDARG;

         *ppBurnView = m_pBurnView;

         return S_OK;
     }


private: // IProvideDataToUi
    virtual Operation::euiOperation GetOperation() { return m_liveOperation.GetOperationFromUi(); }

    virtual bool  GetItemValidationState(const bool &bStopProcessing)
    {
        return ValidateStatefulItems();
    }

    virtual const CSimpleArray<CString>& GetAffectedProducts(const bool &bStopProcessing)
    {
        if (!m_bInitialized)
            throw CObjectNotInitializedException(L"UberCoordinator");

        TargetPackages tp(GetInstallItems());
        CSimpleArray<CString>& names = tp.GetTargetPackagesNames(bStopProcessing);

        m_targetPackageNames.RemoveAll();
        for(int i=0; i<names.GetSize(); ++i)
        {
            m_targetPackageNames.Add(names[i]);
        }
        return m_targetPackageNames;
    }
    virtual ILogger& GetEngineLogger() { return m_logger; }
    virtual Ux& GetUxLogger() { return m_uxLogger; }
    virtual const CString& GetPackageName() { return m_packageName; }
    virtual const CSimpleArray<CString>& GetImageNamesToBlockOn() { return m_imageBlocks; }
    virtual const CSimpleArray<CString>& GetServiceNamesToBlockOn() { return m_serviceBlocks; }
    virtual const ULONGLONG GetSpaceRequiredForDownload()
    {
        if (!m_bInitialized)
            throw CObjectNotInitializedException(L"UberCoordinator");

        return GetDownloadItems().GetDownloadDiskSpaceRequirement();
    }
    virtual const ULONGLONG GetSpaceRequiredOnProductInstalledDrive()
    {
        if (!m_bInitialized)
            throw CObjectNotInitializedException(L"UberCoordinator");

        ULONGLONG total = 0;
        IInstallItems& installItems = GetInstallItems();
        for (unsigned int i=0; i<installItems.GetCount(); ++i)
        {
            total += installItems.GetInstalledProductSize(i);
        }
        return total;
    }
    virtual const ULONGLONG GetSpaceRequiredOnSystemDrive()
    {
        if (!m_bInitialized)
            throw CObjectNotInitializedException(L"UberCoordinator");

        ULONGLONG total = 0;
        IInstallItems& installItems = GetInstallItems();
        for (unsigned int i=0; i<installItems.GetCount(); ++i)
        {
            total += installItems.GetSystemDriveSize(i);
        }
        return total;
    }
    virtual const WCHAR GetProductDriveLetter() const
    {
        return m_productDriveLetter;
    }
#ifdef FeaturesAreImplented
    virtual IFeatureTreeRoot& GetFeatureTreeRoot()
    {
        if (!m_bInitialized)
            throw CObjectNotInitializedException(L"UberCoordinator");

        m_features.RemoveAll();
        const IInstallItems& installItems = GetInstallItems();
        for (unsigned int i=0; i<installItems.GetCount(); ++i)
        {
            const Features* cfeatures = dynamic_cast<const Features*>(installItems.GetItem(i));
            if (cfeatures)
            {
                Features* features = const_cast<Features*>(cfeatures);
                for(unsigned int j=0; j<features->GetNumberOfChildren(); ++j)
                {
                    m_features.Add(features->GetChild(j));
                }
            }
        }
        return m_features;
    }
#endif //FeaturesAreImplented
    virtual INotifyEngine& GetEngineNotificationInterface()
    {
        return *this;
    }

    virtual const UserExperienceDataCollection::Policy UxCollectionPolicy() const
    {
        return m_uxPolicy;
    }

    // Updates the live operation with the one passed in.
    // Called from UI's MaintenanceMode page with operation selected by the user.
    virtual void UpdateLiveOperation(Operation::euiOperation newLiveOperation)
    {
        Operation::euiOperation currentUIOperation = m_liveOperation.GetOperation();
        if (newLiveOperation != currentUIOperation)
        {
            m_liveOperation.ResetInitialOperation(newLiveOperation);

            CString section = L" ";
            PUSHLOGSECTIONPOP(m_logger, L"Operation Type", L" ", section);
            CString strMessage;
            strMessage.Format(L"Operation updated to: %s", Operation::GetOperationCanonicalString(newLiveOperation));
            LOG(m_logger, ILogger::Result, strMessage);
        }
    }
    
    virtual IInstallItems & GetInstallItems() 
    {
        if (!m_bInitialized)
            throw CObjectNotInitializedException(L"UberCoordinator");

        return GetInstallItems(m_liveOperation.GetOperationFromUi()); 
    }

    IInstallItems & GetInstallItems(Operation::euiOperation op)
    {
        if (!m_bInitialized)
            throw CObjectNotInitializedException(L"UberCoordinator");

        if (m_pBurnView)
        {
            // Detect() and Plan() compute m_install items. 
            // Return those install items when BurnUx is loaded.
            return m_install.GetInstallItems();
        }

        switch(op)
        {
        case Operation::uioInstalling  : return   m_install.GetInstallItems();
        case Operation::uioUninstalling: return m_uninstall.GetInstallItems();
        case Operation::uioRepairing   : return    m_repair.GetInstallItems();
        case Operation::uioCreateLayout: return    m_layout.GetInstallItems();
        default:
            IMASSERT(false && L"incompatible operation");
            throw E_FAIL;
        }
    }

    // Is True when /serialdownload is passed at the command line or
    // SerialDownload is authored to be true in Configuration section.
    virtual bool IsSimultaneousDownloadAndInstallDisabled() const
    {
        return m_bForceSerialDownload;
    }

    virtual const IProvideDataToOperand& GetDataToOperand() const
    {
        return m_dataToOperand;
    }

    virtual const BlockersElement& GetBlocks() const
    {
        return m_blocker;
    }

    virtual IBlockChecker& GetBlockChecker()
    {
        return m_blockChecker;
    }

private: // INotifyEngine
#ifdef FeaturesAreImplented
    // Features feature not implemented
    virtual void SetFeatureTreeRoot(IFeatureTreeRoot& featureTreeRoot)
    {
        if (!m_bInitialized)
            throw CObjectNotInitializedException(L"UberCoordinator");

        Features features(m_logger);
        for(unsigned int i=0; i<featureTreeRoot.GetNumberOfChildren(); ++i)
        {
            features.Add(featureTreeRoot.GetChild(i));
        }

        m_features.RemoveAll();
        for(unsigned int i=0; i<features.GetNumberOfChildren(); ++i)
        {
            m_features.Add(features.GetChild(i));
        }

        // apply to install coordinator only (for now.  REVIEW:  What do I do with repair?)
        m_install.Refresh(m_nonLayoutList.GetItems(), m_features, Operation::uioInstalling, m_logger);
    }
#endif //FeaturesAreImplented
};

typedef UberCoordinatorT<FileAuthenticity> UberCoordinator;


}

