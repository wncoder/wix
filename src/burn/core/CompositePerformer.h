//-------------------------------------------------------------------------------------------------
// <copyright file="CompositePerformer.h" company="Microsoft">
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
//      The controller that handles the installation of items
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once

#include "LiveOperation.h"
#include "interfaces\IPerformer.h"
#include "schema\EngineData.h"
#include "UberCoordinator.h"
#include "InvertingProgressObserver.h"
#include "common\ResultObserver.h"
#include "Ux\Ux.h"
#include "WeightedProgressObserver.h"
#include "PerformerCustomErrorHandler.h"

#include "MsiInstaller.h"
#include "MspInstaller.h"
#include "MspUnInstaller.h"
#include "ExeSelectingPerformer.h"
#include "AgileMsiInstaller.h"
#include "ServiceControlInstaller.h"
#include "CleanupBlockInstaller.h"
#include "RelatedProducts.h"
#include "interfaces\IDataProviders.h"
#include "interfaces\IOperationData.h"
#include "UnelevatedController.h"
#include "ICacheManager.h"
#include "SourceLocation.h"

namespace IronMan
{

template
<
      typename _MsiPerformer
    , typename _MspPerformer
    , typename _ExePerformer
    , typename _AgileMsiPerformer
    , typename _ServiceControlPerformer
    , typename _CleanupBlockPerformer
    , typename _RelatedProductsPerformer
>
struct Performers
{
    typedef _MsiPerformer MsiPerformer;
    typedef _MspPerformer MspPerformer;
    typedef _ExePerformer ExePerformer;
    typedef _AgileMsiPerformer AgileMsiPerformer;
    typedef _ServiceControlPerformer ServiceControlPerformer;
    typedef _CleanupBlockPerformer CleanupBlockPerformer;
    typedef _RelatedProductsPerformer RelatedProductsPerformer;
};

template<typename InstallPerformers, typename UninstallPerformers, typename RepairPerformers>
class CompositePerformerBaseT : public AbortStopPerformerBase
{
    Operation::euiOperation m_op;
    IBurnView *m_pBurnView;
    IInstallItems& m_items;
    ILogger& m_logger;
    IPerformer* m_currentPerformer;
    const FailureActionEnum m_subFailureAction;
    bool m_bSerialDownload;
    Ux& m_uxLogger;
    FirstError m_firstError;
    UxEnum::actionEnum m_currentAction;
    bool m_rollingBack;

    UINT m_elevatedItemIndex;
    ActionTable::Actions m_elevatedItemAction;
    HRESULT m_elevatedResult;
    bool m_performStop;
    bool m_performRollback;
    bool m_performerAborted;

    const int m_iSecondToWaitForMsiMutex;

    ICacheManager& m_cacheManager;

protected:
    const IOperationData& m_operationData;

public:
    CompositePerformerBaseT(Operation::euiOperation op
                            , IInstallItems& items
                            , ICacheManager& cacheManager
                            , IBurnView *pBurnView
                            , const IOperationData& operationData
                            , const FailureActionEnum& subFailureAction
                            , bool bSerialDownload
                            , ILogger& logger
                            , Ux& uxLogger
                            , __in int iSecondToWaitForMsiMutex = 60)  //Enable calling method to determine the wait time.
        : m_op(op)
        , m_items(items)
        , m_cacheManager(cacheManager)
        , m_pBurnView(pBurnView)
        , m_operationData(operationData)
        , m_currentPerformer(&NullPerformer::GetNullPerformer())
        , m_subFailureAction(subFailureAction)
        , m_bSerialDownload(bSerialDownload)
        , m_logger(logger)
        , m_uxLogger(uxLogger)
        , m_firstError(logger)
        , m_currentAction(UxEnum::aNone)
        , m_rollingBack(false)
        , m_elevatedItemIndex(0)
        , m_elevatedItemAction(ActionTable::Install)
        , m_elevatedResult (E_FAIL)
        , m_performStop (false)
        , m_performRollback (false)
        , m_performerAborted (false)
        , m_iSecondToWaitForMsiMutex(iSecondToWaitForMsiMutex)
    {}
    virtual ~CompositePerformerBaseT() {}


private:
    HRESULT EnsurePackageIsAvailable(unsigned int i, IProgressObserver& observer, UnElevatedController *pUnElevatedController, bool& bIgnorePackage)
    {
        bIgnorePackage = false;

        // Check to see if the item is cached on the machine already and is Valid.
        // If it is cached but not valid, then it will be deleted and it will need to be cached again
        // If it is cached and valid, the state will change to Available and the LocalPath updated
        if (m_cacheManager.IsCached(m_items, i, pUnElevatedController))
        {
            LOG(m_logger, ILogger::Verbose, L"Item (" + m_items.GetItemName(i) + L") is already cached on the machine.");
        }
        else
        {
            //
            //  Item is not available & ignorable
            //
            if (m_items.IsItemNotAvailableAndIgnorable(i))
            {
                LOG(m_logger, ILogger::Result, m_items.GetItemName(i) + L" Item ignored as it is not available and is ignorable");
                bIgnorePackage = true;
                return S_OK;
            }

            //
            //  If item needs to be downloaded, wait for download to complete.
            //
            if (m_items.IsItemOnDownloadList(i))
            {
                LOG(m_logger, ILogger::Verbose, L"Wait for Item (" + m_items.GetItemName(i) + L") to be downloaded");

                // Poll, for now, may need to use an event if this proves inefficient
                while (m_items.IsItemOnDownloadList(i))
                {
                    Sleep(1000);
                    if (HasAborted() || IsStopPending())
                    {
                        observer.Finished(E_ABORT);
                        return E_ABORT;
                    }
                }

                if (m_items.IsItemAvailable(i))
                {
                    LOG(m_logger, ILogger::Verbose, m_items.GetItemName(i) + L" is now downloaded & available to install");
                    return S_OK;
                }
                else if (m_items.IsItemNotAvailableAndIgnorable(i))
                {
                    LOG(m_logger, ILogger::Result, m_items.GetItemName(i) + L" Item ignored as it is not available and is ignorable");
                    bIgnorePackage = true;;
                    return S_OK;
                }
                else
                {
                    IMASSERT(0 && L"If the item is on the download list, after download is completed the item should either become available or its state changed to ignorable");
                    LOG(m_logger, ILogger::Error, m_items.GetItemName(i) + L" Item not available & not ignorable!");
                    return E_UNEXPECTED;
                }
            }


            //
            //  Due to multi-volume scenario we need to check again if the item exists at the place it was found earlier - if not
            //  we need to ask Ux to resolve source for it.
            //
            //  Even for items marked NotAvailableAskUX we should check if the item has become available as a result of the Ux
            //  resolving source for a previously missing item.
            //
            //  So we will first check if the item is available, if not we will ask UX to resolve the source
            //
            CPath itemPath = m_items.GetItemName(i);
            if (itemPath.IsRelative())
            {
                itemPath = CPath(SourceLocation::GetPath());
                itemPath.Append(m_items.GetItemName(i));
            }

            const ItemBase* pItem = m_items.GetItem(i);
            if (itemPath.FileExists())
            {
                bool bLocalExe = true;
                if (pItem->GetType() == ItemBase::Exe)
                {
                    const IronMan::ExeBase* exe = dynamic_cast<const IronMan::ExeBase *>(pItem);
                    if (exe != NULL)
                    {
                        bLocalExe = exe->GetExeType().IsLocalExe();
                    }
                }

                if (bLocalExe == false)
                {
                    m_items.UpdateItemPath(i, itemPath);
                }
            }
            else if (m_pBurnView != NULL) // Ask UX to ResolveSource
            {
                CPath itemID = m_items.GetItemID(i);
                CPath itemOriginalName = m_items.GetItemOriginalName(i);
                CPath itemOriginalNameStripped = itemOriginalName;
                itemOriginalNameStripped.StripPath();

                if (CString::StringLength(itemID) <= 0)
                {
                    itemID = itemOriginalNameStripped;
                }

                int idRet = IDCANCEL;

                // Since the loop below can potentially go into an infinite loop we limit it to maxRetries.
                const int maxRetries = 100;
                int retries = 0;

                itemPath = CPath(SourceLocation::GetPath());
                itemPath.Append(itemOriginalName);

                while (1)
                {
                    if (!itemPath.FileExists())
                    {
                        idRet = m_pBurnView->ResolveSource(itemID, itemPath);
                        itemPath = CPath(SourceLocation::GetPath());
                        itemPath.Append(itemOriginalNameStripped);
                    }
                    else
                    {
                        idRet = IDOK;
                    }

                    if (idRet == IDOK || idRet == IDRETRY)
                    {
                        if (itemPath.FileExists() == false)
                        {
                            // Oops file still not found!!

                            // Ensure number of retries do not exceed 100.
                            ++retries;
                            if (retries >= maxRetries)
                            {
                                LOGEX(m_logger, ILogger::Error, + L"Number of retries to ResolveSource() for item %s exceeded preset limit of %d!",  m_items.GetItemName(i), maxRetries);
                                return E_FAIL;
                            }

                            // Ask again.
                            continue;
                        }
                    }

                    break;
                }

                if (idRet == IDIGNORE)
                {
                    m_items.SetItemStateToIgnorable(i);
                    LOG(m_logger, ILogger::Result, m_items.GetItemName(i) + L" item ignored per UX, continuing with the install of remaining items");
                    bIgnorePackage = true;;
                    return S_OK;
                }
                else if (idRet == IDCANCEL)
                {
                    observer.Finished(E_ABORT);
                    return E_ABORT;
                }

                // Update Item path in unelevated process
                m_items.UpdateItemPath(i, itemPath);
                if ( m_items.VerifyItem(i, m_logger) )
                {
                    m_items.SetItemStateAsAvailable(i);
                }
                else
                {
                    LOG(m_logger, ILogger::Error, m_items.GetItemName(i) + L" Item resolved to file that could not be verified.");
                    return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
                }
                // Update Item path in elevated process if needed
                if (pUnElevatedController != NULL && pItem->IsPerMachine())
                {
                    HRESULT hr = S_OK;
                    if ( FAILED(hr = pUnElevatedController->UpdatePackageLocation(i, itemPath)) )
                    {
                        LOG(m_logger, ILogger::Error, m_items.GetItemName(i) + L" Package location could not be updated in elevated process.");
                        return hr;
                    }
                }
            }
            else
            {
                LOGEX(m_logger, ILogger::Error, + L"Item %s not found!",  m_items.GetItemName(i));
                return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
            }



            //
            // Cache the file if Available.
            //
            if (m_items.IsItemAvailableUnVerified(i) || m_items.IsItemAvailable(i))
            {
                return m_cacheManager.VerifyAndCachePackage(m_items, i, pUnElevatedController);
            }
        }
        return S_OK;
    }

public:
    // IPerformer
    virtual void PerformAction(IProgressObserver& observer)
    {
        HRESULT hr = S_OK;
        int nResult = IDOK;
        UnElevatedController *pUnElevatedController = NULL;

        { // Scope for PushLogSectionPop
        CString section = L" complete";

        bool bElevationRequired = false;

        CSimpleArray<ULONGLONG> weights;
        for(unsigned int i=0; i<m_items.GetCount(); ++i)
        {
            unsigned int index = i;

            if (Operation::uioUninstalling ==  m_op)
            {
                index = m_items.GetCount()-i-1;  // if uninstalling, this reverses the "sense"
            }

            bool bApplicable = false;
            ACTION_STATE actionState = ACTION_STATE_NONE;
            m_items.GetItem(index)->GetPackageState(&bApplicable, NULL, NULL, &actionState);
            bool bUseActionState = m_items.GetItem(index)->ActionStateUpdated();

            if (!bApplicable || (bUseActionState && actionState == ACTION_STATE_NONE))
            {
                continue;
            }

            weights.Add(m_items.GetEstimatedInstallTime(index));
            // Compute requirement of additional Elevated process
            if ( !bElevationRequired && m_items.GetItem(index)->IsPerMachine())
            {
                bElevationRequired = true;
            }
        }

        CString performActionMessage;
        performActionMessage = L"Performing actions on all Items";

        PUSHLOGSECTIONPOP(m_logger, L"Action", performActionMessage, section);

        WeightedProgressObserver weightedObserver(observer, weights);

        ResultObserver resultObserver(weightedObserver, hr);

        pUnElevatedController = new UnElevatedController(resultObserver, m_logger, m_pBurnView);

        if (pUnElevatedController != NULL && bElevationRequired == true)
        {
            pUnElevatedController->SaveUnElevatedLogFile();
        }

        // Iterate over all items in the install list
        unsigned int iItemIndex=0;
        bool performerAborted = false;
        bool performStop = false;
        bool performRollback = false;

        for(; iItemIndex<m_items.GetCount() && !HasAborted() && !IsStopPending(); ++iItemIndex)
        {
            unsigned int i = 0;

            i = iItemIndex;

            if (Operation::uioUninstalling ==  m_op)
            {
                i = m_items.GetCount()-iItemIndex-1;  // if uninstalling, this reverses the "sense"
            }

            hr = PerformCache(i, pUnElevatedController, observer);
            if (FAILED(hr))
            {
                LOGEX(m_logger, ILogger::Error, L"Cache operation failed with error = %d.", hr);
                m_firstError.SetError(hr);
                Abort();
                break;
            }

            bool bApplicable = false;
            ACTION_STATE actionState = ACTION_STATE_NONE;
            m_items.GetItem(i)->GetPackageState(&bApplicable, NULL, NULL, &actionState);
            bool bUseActionState = m_items.GetItem(i)->ActionStateUpdated();

            if (!bApplicable || (bUseActionState && actionState == ACTION_STATE_NONE))
            {
                LOG(m_logger, ILogger::Result, m_items.GetItemName(i) + L" Package is not applicable or action state is none.");
                continue;
            }

            if (m_items.IsItemNotAvailableAndIgnorable(i))
            {
                LOG(m_logger, ILogger::Result, m_items.GetItemName(i) + L"Package not available and can be ignored.");
                continue;
            }

            nResult = weightedObserver.OnProgress(255); // in case, last time didn't finish up
            hr = HRESULT_FROM_VIEW(nResult);
            if (FAILED(hr))
            {
                LOG(m_logger, ILogger::Error, L"User interface commanded engine to abort");
                m_firstError.SetError(hr);
                Abort();
                break;
            }

            nResult = weightedObserver.SetNextPhase();
            hr = HRESULT_FROM_VIEW(nResult);
            if (FAILED(hr))
            {
                LOG(m_logger, ILogger::Error, L"User interface commanded engine to abort");
                m_firstError.SetError(hr);
                Abort();
                break;
            }

            // poll for Msi mutex, too
            // Under Vista, the Mutex is created in the Global namespace
            // Under Vista, if IronMan does not specify the Global namespace, the Local namespace is used
            // so for this to work under all OS we need to check the Global Mutex
            LPCTSTR szGlobalMSIMutex = _T("Global\\_MSIExecute");
            HANDLE hMutex = OpenMutex(MUTEX_MODIFY_STATE, FALSE, szGlobalMSIMutex) ;
            if (hMutex != NULL) // NULL means no other MSI installation is happening
            {
                CloseHandle(hMutex);
                // Update the status of the install bar to waiting.
                observer.OnStateChangeDetail(IProgressObserver::WaitingForAnotherInstallToComplete, L"");

                int secondsToWait = m_iSecondToWaitForMsiMutex;
                CCmdLineSwitches switches;
                if (!switches.InteractiveMode())
                {
                    secondsToWait = m_iSecondToWaitForMsiMutex*10; // wait for 10 minutes before bailing
                }

                while(true)
                {
                    bool bOk = false;
                    LOGEX(m_logger, ILogger::Information, L"Another installation is already running, waiting up to %i seconds for it to finish", secondsToWait);
                    for(int j=0; j<secondsToWait; ++j)
                    {
                        // Progress doesn't move foward but this call in the loop will update the rotating icons.
                        nResult = weightedObserver.OnProgress(0);
                        hr = HRESULT_FROM_VIEW(nResult);
                        if (FAILED(hr))
                        {
                            LOG(m_logger, ILogger::Error, L"User interface commanded engine to abort");
                            m_firstError.SetError(hr);
                            Abort();
                            break;
                        }
                        Sleep(1000);
                        if (HasAborted() || IsStopPending())
                        {
                            LOG(m_logger, ILogger::Information, L"User has aborted the install, exit from the wait.");
                            observer.Finished(E_ABORT);
                            break;
                        }

                        hMutex = OpenMutex(MUTEX_MODIFY_STATE, FALSE, szGlobalMSIMutex) ;
                        CloseHandle(hMutex);
                        if (NULL == hMutex)
                        {
                            bOk = true;
                            LOG(m_logger, ILogger::Information, L"Msi Handle released.");
                            break;
                        }
                    }
                    if (true == bOk)
                    {
                        LOG(m_logger, ILogger::Information, L"Other installation completed, continuing.");
                        break;
                    }

                    // ok, we waited a minute.  Ask the user what to do.
                    // There is no IBurnUserExperience function to do this for now. Mimic silentUIFactory behavior.
                    //if (IMsiBusy::Cancel == m_msiBusy.Run())
                    //{
                        m_uxLogger.RecordCancelPage(L"MSIBusy");
                        LOG(m_logger, ILogger::Information, L"Another installation is already running and the user has chosen to cancel rather than wait");
                        Abort();
                        observer.Finished(E_ABORT);
                        return;
                    //}
                    //LOG(m_logger, ILogger::Information, L"Another installation is already running and the user has chosen to wait for it to finish before continuing");
                    //secondsToWait = 1; // ask immediately, from now on
                }
            }

            const ItemBase* pItem = m_items.GetItem(i);

            performStop = false;
            performRollback = false;

            CString performItemAction = L" complete";
            PUSHLOGSECTIONPOP(m_logger, L"Action", L"Performing action on package " + m_items.GetItemName(i), performItemAction);

            // Execute the package
            if (!HasAborted())
            {
                PerformItem(pItem, i, pUnElevatedController, observer, resultObserver, hr, performStop, performRollback, performerAborted);
            }

            if (performerAborted || performRollback || performStop)
            {
                break;
            }

            // Update the state of the item to complete
            m_items.UpdateItemState(i);

        } // End of install items loop

        // If user aborted, in install mode, we have to rollback.
        if ((performRollback || performerAborted || HasAborted()) && iItemIndex > 0 && InInstallMode() && !m_rollingBack)
        {
            LOG(m_logger, ILogger::Warning, L"Rolling back. OnFailureBehavior for current item will be ignored.");
            InternalRollback(observer, iItemIndex);
            LOG(m_logger, ILogger::Warning, L"Removing cache of packages that were not installed.");
            for (int k = iItemIndex; k < m_items.GetCount(); ++k)
            {
                m_cacheManager.DeleteCachedPackage(m_items, k, pUnElevatedController);
            }
        }

        } // End of Scope of PushLogSectionPop so that Pop is completed before observer.Finished() below is called.

        if ( (NULL == vpEngineState) || (!vpEngineState->fReboot) )
        {
            m_cacheManager.DeleteTemporaryCacheDirectories(pUnElevatedController);
        }

        if (!m_rollingBack)
        {
            // Failure ignored when clearing cache.
            ClearCache(pUnElevatedController);
        }

        if (pUnElevatedController)
            delete pUnElevatedController;

        observer.Finished(m_firstError.GetError());
    }

    virtual void PerformItem(const ItemBase* pItem, int i, UnElevatedController *pUnElevatedController, IProgressObserver& observer, ResultObserver& resultObserver, HRESULT& hr, bool& performStop, bool &performRollback, bool &performerAborted)
    {
            BOOL fAbort = FALSE;

            PerformerCustomErrorHandler pcehErrorHandler(m_uxLogger, m_logger);
            const CustomErrorHandling* pErrorHandler = dynamic_cast<const CustomErrorHandling*>(pItem);
            pcehErrorHandler.Initialize(pErrorHandler);

            // CreateItemPerformer
            hr = CreateItemPerformer(pItem, m_currentPerformer);

            if (FAILED(hr))
            {
                performerAborted = true;
                return;
            }

            bool bRetry = true;
            FirstError localError(m_logger);

            int nResult = m_pBurnView->OnExecutePackageBegin(pItem->GetId(), !m_rollingBack);
            if (!m_rollingBack)
            {
                hr = HRESULT_FROM_VIEW(nResult);
                if (FAILED(hr))
                {
                    LOG(m_logger, ILogger::Error, L"User interface commanded engine to abort");
                    Abort();
                    performerAborted = TRUE;
                }
                ExitOnFailure(hr, "Execution flow interrupted after OnExecutePackageBegin");
            }

            if (pItem->IsPerMachine())
            {
                if (pUnElevatedController)
                {
                    LOG(m_logger, ILogger::Information, L"Begin Elevated install " + m_items.GetItemName(i));
                    UINT elevatedAction = (UINT)GetAction(pItem, m_op, m_operationData.GetDataToOperandData());
                    pUnElevatedController->ApplyPackage(i, (UINT) m_op, elevatedAction, hr, fAbort);
                    if (fAbort)
                    {
                        Abort();
                        performerAborted = TRUE;
                    }
                    LOG(m_logger, ILogger::Information, L"Completed Elevated install " + m_items.GetItemName(i));
                    localError.SetError(hr);
                }
                else
                {
                    LOG(m_logger, ILogger::Error, L"Elevated Process cannot be reached");
                    hr = E_FAIL;
                    performerAborted = true;
                }
            }
            else
            {
                while (!HasAborted() && !IsStopPending() && bRetry)
                {
                    WatsonData::WatsonDataStatic()->SetInternalErrorState(0, L"");
                    m_currentPerformer->PerformAction(resultObserver);

                    localError.SetError(hr);

                    bRetry = pcehErrorHandler.Execute(localError, resultObserver);
                    if (bRetry)
                        localError.ClearError();
                }
            }

            m_pBurnView->OnExecutePackageComplete(pItem->GetId(), hr);

            { // avoid (extremely rare) race-condition
                IPerformer* tmp = m_currentPerformer;
                m_currentPerformer = &NullPerformer::GetNullPerformer();
                delete tmp;
            }

            if ( localError.IsError())
            {
                /*
                Between the whole package and indivdual item's failure actions, the following table
                determines which action wins.
                --------------------------------------------------------------
                    OnSubFailureAction      OnFailureAction    Winner
                    (whole package)         (item)
                --------------------------------------------------------------
                    Not Specified           Not Specified       OnFailureAction (with default values)
                    Not Specified           Specified           OnFailureAction
                    Specified               Not Specified       OnSubFailureAction
                    Specified               Specified           OnFailureAction
                --------------------------------------------------------------
                */
                FailureActionEnum itemFailureAction = GetItemFailureAction(pItem);
                FailureActionEnum failureAction = itemFailureAction;
                if (m_subFailureAction.IsFailureActionSpecified() && !failureAction.IsFailureActionSpecified())
                {
                    failureAction = m_subFailureAction;
                }

                if (failureAction.IsFailureActionRollback())
                {
                    // Set error to report later
                    m_firstError.SetErrorWithAbort(localError.GetError());
                    //Logging the error here since it is a non-ignorable failure.
                    m_uxLogger.RecordPhaseAndCurrentItemNameOnError(UxEnum::pInstall, m_currentAction, m_items.GetItemName(i));
                    LOG(m_logger, ILogger::Error, L"OnFailureBehavior for this item is to Rollback.");
                    observer.OnStateChange(HasAborted() ? IProgressObserver::UserCancelled : IProgressObserver::Rollback);
                    performRollback = true;
                    performStop = true;
                    return;
                }
                else if (failureAction.IsFailureActionStop())
                {
                    // Set error to report later
                    m_firstError.SetErrorWithAbort(localError.GetError());
                    //Logging the error here since it is a non-ignorable failure.
                    m_uxLogger.RecordPhaseAndCurrentItemNameOnError(UxEnum::pInstall, m_currentAction, m_items.GetItemName(i));
                    // Log
                    LOG(m_logger, ILogger::Error, L"OnFailureBehavior for this item is to Stop.");
                    if (HasAborted())
                    {
                        performerAborted = true;
                    }
                    performStop = true;
                    return;
                }
                else if (failureAction.IsFailureActionContinue())
                {
                    // In Maintenance mode, if *item* failure action is not Continue, we will continue and
                    // process next items but will store and report error.
                    if (!InInstallMode() && !itemFailureAction.IsFailureActionSpecified())
                    {
                        //Logging the error here since Failure is not specified.
                        m_uxLogger.RecordPhaseAndCurrentItemNameOnError(UxEnum::pInstall, m_currentAction, m_items.GetItemName(i));
                        // Set error to report later
                        m_firstError.SetErrorWithAbort(localError.GetError());
                        // Log Error
                        LOG(m_logger, ILogger::Error, L"Item Failed. OnFailureBehavior for this item is not specified.");
                        LOG(m_logger, ILogger::Error, L"Default behavior for Repair and Uninstall is to continue and report this failure.");
                    }
                    else
                    {
                        // Log warning. Not Setting the error.
                        LOG(m_logger, ILogger::Warning, L"Item Failed. OnFailureBehavior for this item is to Continue.");
                    }
                    if (HasAborted())
                    {
                        performerAborted = true;
                        return;
                    }
                    observer.OnStateChange(InInstallMode() ? IProgressObserver::Installing : IProgressObserver::Uninstalling);
                }
            }
            else
            {
                // If this was a successful uninstall, then delete the package if it is cached.
                if ( ActionTable::Uninstall == GetAction(pItem, m_op, m_operationData.GetDataToOperandData()) )
                {
                    m_cacheManager.DeleteCachedPackage(m_items, i, pUnElevatedController);
                }

                if (localError.IsReboot())
                {
                    LOG(m_logger, ILogger::Verbose, L"Item Requested Reboot.");
                    // Set Reboot Required success code.
                    m_firstError.SetErrorWithAbort(localError.GetError());
                    // Tell the observer that a 3010 has been returned, so that if we are in UI mode
                    // and real error occurs, we can still pop up the RebootPending dialog box
                    observer.OnRebootPending();
                }
                else
                {
                    // Though localError is not Error or Reboot, it was updated (1641, 3011 etc). So update m_firstError
                    m_firstError.SetErrorWithAbort(localError.GetError());
                }
            }

    LExit:
            return;
    }

public:

    // Sets the Composite Performer in rollback mode.
    // This changes the performer in the following ways:
    // Return code of actions during rollback are ignored.
    // Uninstall performer will always use "uninstall" as action.
    virtual void SetRollbackFlag(bool bRollingBack)
    {
        m_rollingBack = true;
    }

private:
    HRESULT PerformCache(unsigned int nItemIndex, UnElevatedController *pUnElevatedController, IProgressObserver& observer)
    {
        HRESULT hr = S_OK;
        int nResult = IDOK;

        nResult = m_pBurnView->OnCacheBegin();
        hr = HRESULT_FROM_VIEW(nResult);
        ExitOnFailure(hr, "Cache sequence interrupted.");

        if (Operation::uioUninstalling ==  m_op)
        {
            nItemIndex = m_items.GetCount()- nItemIndex -1;  // if uninstalling, this reverses the "sense"
        }

        bool bApplicable = false;
        ACTION_STATE actionState = ACTION_STATE_NONE;
        m_items.GetItem(nItemIndex)->GetPackageState(&bApplicable, NULL, NULL, &actionState);
        bool bUseActionState = m_items.GetItem(nItemIndex)->ActionStateUpdated();

        if (!bApplicable || (bUseActionState && actionState == ACTION_STATE_NONE))
        {
            LOG(m_logger, ILogger::Result, m_items.GetItemName(nItemIndex) + L" Cache is not required. Package is not applicable or action state is none.");
            ExitFunction1(hr = S_OK);
        }

        bool bIgnorePackage = false;
        bool bSkipEnsuringPackageAvailability = false;

        // If we are uninstalling & the item we are uninstalling is not an Exe then we do not need to ensure that package is available
        if (actionState == ACTION_STATE_UNINSTALL)
        {
            bSkipEnsuringPackageAvailability = (m_items.GetItem(nItemIndex)->GetType() != ItemBase::Exe);
        }

        if (!bSkipEnsuringPackageAvailability)
        {
            nResult = m_pBurnView->OnCachePackageBegin(m_items.GetItemID(nItemIndex), m_items.GetPackageCacheSize(nItemIndex));
            LOG(m_logger, ILogger::Verbose, L"Cache begin: " + m_items.GetItemOriginalName(nItemIndex));
            hr = HRESULT_FROM_VIEW(nResult);
            ExitOnFailure(hr, "Cache sequence interrupted.");

            hr = EnsurePackageIsAvailable(nItemIndex, observer, pUnElevatedController, bIgnorePackage);
            if (FAILED(hr))
            {
                LOGEX(m_logger, ILogger::Error, L"Cache operation failed for %s. Error = %d.", m_items.GetItemOriginalName(nItemIndex), hr);
                ExitOnFailure(hr, "Cache operation failed.");
            }

            m_pBurnView->OnCachePackageComplete(m_items.GetItemID(nItemIndex), hr);
            LOG(m_logger, ILogger::Verbose, L"Cache completed: " + m_items.GetItemOriginalName(nItemIndex));
            if (bIgnorePackage)
            {
                m_items.SetItemStateToIgnorable(nItemIndex);
            }
        }

    LExit:
        m_pBurnView->OnCacheComplete(hr);
        return hr;
    }


    // ClearCache
    // When the bundle is being unisntalled, deletes cache of packages that are requested to be absent.
    HRESULT ClearCache(UnElevatedController *pUnElevatedController)
    {
        HRESULT hr = S_OK;

        if (Operation::uioUninstalling ==  m_op)
        {
            unsigned int iItemIndex=0;
            for(; iItemIndex<m_items.GetCount() && !HasAborted() && !IsStopPending(); ++iItemIndex)
            {
                unsigned int i = iItemIndex;
                bool bApplicable = false;
                ACTION_STATE actionState = ACTION_STATE_NONE;
                REQUEST_STATE requestState = REQUEST_STATE_NONE;

                m_items.GetItem(i)->GetPackageState(&bApplicable, NULL, &requestState, &actionState);
                bool bUseActionState = m_items.GetItem(i)->ActionStateUpdated();

                if (bApplicable && bUseActionState && requestState == REQUEST_STATE_ABSENT && actionState == ACTION_STATE_NONE)
                {
                    if (m_cacheManager.IsCached(m_items, i, pUnElevatedController))
                    {
                        hr = m_cacheManager.DeleteCachedPackage(m_items, i, pUnElevatedController);
                        if (FAILED(hr))
                        {
                            LOG(m_logger, ILogger::Warning, m_items.GetItemName(i) + L"Package cache was not cleared successfully.");
                        }
                    }
                }
            }
        }

    LExit:
        return hr;
    }


    //Refactor as a method to remove duplication of code.
    void InternalRollback(IProgressObserver& observer, unsigned int iItemIndex)
    {
        ProgressNoFinishObserver pnfo(observer);
        //Set the UX rollback state.
        m_uxLogger.SetInRollback();
        // Note that if the item failed, for Exe items, we rollback explicitly
        // Msi item will be rolled back automatically. So, we have to have only rollback previous items.
        if (iItemIndex > 0)
        {
            //We need to cache the Internal error as we don't want the value set during rollback.
            int cacheInternalError = WatsonData::WatsonDataStatic()->GetInternalError();
            CString cacheCurrentStep = WatsonData::WatsonDataStatic()->GetCurrentStep();
            Rollback(pnfo, iItemIndex-1, m_items, m_pBurnView, m_bSerialDownload, m_logger, m_uxLogger);
            WatsonData::WatsonDataStatic()->SetInternalErrorState(cacheInternalError, cacheCurrentStep);
        }
    }

    // Helper classs with Null Finish observer behavior
    // This is used in rollback mode.
    class ProgressNoFinishObserver : public IProgressObserver
    {
        IProgressObserver& m_observer;
    public:
        ProgressNoFinishObserver(IProgressObserver& observer) : m_observer(observer) {}
    private:
        virtual int OnProgress(unsigned char soFar) { return m_observer.OnProgress(soFar); }
        virtual int OnProgressDetail(unsigned char ) { return IDOK; }

        // HRESULT is ignored
        virtual void Finished(HRESULT) {}
        virtual void OnStateChange(IProgressObserver::State enumVal)  {}
        virtual void OnStateChangeDetail (const State enumVal, const CString changeInfo)
        {
            m_observer.OnStateChangeDetail(enumVal, changeInfo);
        }
        virtual void OnRebootPending()
        {
            m_observer.OnRebootPending();
        }
    };

public:
    virtual void Abort()
    {
        AbortPerformerBase::Abort();
        m_currentPerformer->Abort();
        m_firstError.Abort();
    }

private:
    IPerformer* CreateMsiPerformer(const MSI* msi)
    {
        LOG(m_logger, ILogger::Verbose, L"Creating new Performer for MSI item");
        switch(GetAction(msi, m_op, m_operationData.GetDataToOperandData()))
        {
        case ActionTable::Install:
            return new   InstallPerformers::MsiPerformer(*msi, m_logger, m_uxLogger, m_pBurnView);
        case ActionTable::Uninstall:
            return new UninstallPerformers::MsiPerformer(*msi, m_logger, m_uxLogger, m_pBurnView);
        case ActionTable::Repair:
            return new    RepairPerformers::MsiPerformer(*msi, m_logger, m_uxLogger, m_pBurnView);
        case ActionTable::Noop:
            return new DoNothingPerformer();
        default:
            IMASSERT2(0, L"GetAction returned an invalid action type; can't create performer");
            LOG(m_logger, ILogger::Warning, L"GetAction returned an invalid action type; creating DoNothingPerformer");
            return new DoNothingPerformer();
        }
    }

    IPerformer* CreateAgileMsiPerformer(const AgileMSI* agileMsi)
    {
        LOG(m_logger, ILogger::Verbose, L"Creating new Performer for AgileMSI item");
        switch(GetAction(agileMsi, m_op, m_operationData.GetDataToOperandData()))
        {
        case ActionTable::Install:
            return new   InstallPerformers::AgileMsiPerformer(*agileMsi, m_logger, m_uxLogger);
        case ActionTable::Uninstall:
            return new UninstallPerformers::AgileMsiPerformer(*agileMsi, m_logger, m_uxLogger);
        case ActionTable::Repair:
            return new    RepairPerformers::AgileMsiPerformer(*agileMsi, m_logger, m_uxLogger);
        case ActionTable::Noop:
            return new DoNothingPerformer();
        default:
            IMASSERT2(0, L"GetAction returned an invalid action type; can't create performer");
            LOG(m_logger, ILogger::Warning, L"GetAction returned an invalid action type; creating DoNothingPerformer");
            return new DoNothingPerformer();
        }
    }

    IPerformer* CreateServiceControlPerformer(const ServiceControl* serviceControl)
    {
        LOG(m_logger, ILogger::Verbose, L"Creating new Performer for ServiceControl item");
        switch(GetAction(serviceControl, m_op, m_operationData.GetDataToOperandData()))
        {
        case ActionTable::Install:
            return new   InstallPerformers::ServiceControlPerformer(*serviceControl, m_logger, m_uxLogger);
        case ActionTable::Uninstall:
            return new UninstallPerformers::ServiceControlPerformer(*serviceControl, m_logger, m_uxLogger);
        case ActionTable::Repair:
            return new    RepairPerformers::ServiceControlPerformer(*serviceControl, m_logger, m_uxLogger);
        case ActionTable::Noop:
            return new DoNothingPerformer();
        default:
            IMASSERT2(0, L"GetAction returned an invalid action type; can't create performer");
            LOG(m_logger, ILogger::Warning, L"GetAction returned an invalid action type; creating DoNothingPerformer");
            return new DoNothingPerformer();
        }
    }

    // CleanupBlockPerformer
    IPerformer* CreateCleanupBlockPerformer(const CleanupBlock* cleanupBlock)
    {
        LOG(m_logger, ILogger::Verbose, L"Creating new Performer for CleanupBlock item");
        switch(GetAction(cleanupBlock, m_op, m_operationData.GetDataToOperandData()))
        {
        case ActionTable::Install:
            return new   InstallPerformers::CleanupBlockPerformer(*cleanupBlock
                                                                    , m_subFailureAction
                                                                    , m_logger
                                                                    , m_uxLogger);
        case ActionTable::Uninstall:
            return new   DoNothingPerformer();
        case ActionTable::Repair:
            return new   DoNothingPerformer();
        case ActionTable::Noop:
            return new   DoNothingPerformer();
        default:
            IMASSERT2(0, L"GetAction returned an invalid action type; can't create performer");
            LOG(m_logger, ILogger::Warning, L"GetAction returned an invalid action type; creating DoNothingPerformer");
            return new DoNothingPerformer();
        }
    }

    //RelatedProductsPerformer
    IPerformer* CreateRelatedProductsPerformer(const RelatedProducts* relatedProducts)
    {
        LOG(m_logger, ILogger::Verbose, L"Creating new Performer for RelatedProducts item");
        switch(GetAction(relatedProducts, m_op, m_operationData.GetDataToOperandData()))
        {
        case ActionTable::Install:
            return new   DoNothingPerformer();
        case ActionTable::Uninstall:
            return new   UninstallPerformers::RelatedProductsPerformer(*relatedProducts, m_subFailureAction, m_logger, m_uxLogger);
        case ActionTable::Repair:
            return new   RepairPerformers::RelatedProductsPerformer(*relatedProducts, m_subFailureAction, m_logger, m_uxLogger);
        case ActionTable::Noop:
            return new   DoNothingPerformer();
        default:
            IMASSERT2(0, L"GetAction returned an invalid action type; can't create performer");
            LOG(m_logger, ILogger::Warning, L"GetAction returned an invalid action type; creating DoNothingPerformer");
            return new DoNothingPerformer();
        }
    }

    IPerformer* CreateMspPerformer(const MSP* msp)
    {
        LOG(m_logger, ILogger::Verbose, L"Creating new Performer for MSP item");
        switch(GetAction(msp, m_op, m_operationData.GetDataToOperandData()))
        {
        case ActionTable::Install:
            return new   InstallPerformers::MspPerformer(m_items
                                                        , *msp
                                                        , m_subFailureAction
                                                        , m_operationData.GetPackageData().GetServicingTrain()
                                                        , m_logger
                                                        , m_uxLogger);
        case ActionTable::Uninstall:
            return new UninstallPerformers::MspPerformer(m_items
                                                        , *msp
                                                        , m_subFailureAction
                                                        , m_logger
                                                        , m_uxLogger);
        case ActionTable::Repair:
            return new    RepairPerformers::MspPerformer(m_items
                                                        , *msp
                                                        , m_subFailureAction
                                                        , m_operationData.GetPackageData().GetServicingTrain()
                                                        , m_logger
                                                        , m_uxLogger);
        case ActionTable::Noop:
            return new DoNothingPerformer();
        default:
            IMASSERT2(0, L"GetAction returned an invalid action type; can't create performer");
            LOG(m_logger, ILogger::Warning, L"GetAction returned an invalid action type; creating DoNothingPerformer");
            return new DoNothingPerformer();
        }
    }
    IPerformer* CreatePatchesPerformer(const Patches* patches)
    {
        LOG(m_logger, ILogger::Verbose, L"Creating new Performer for Patches item");
        switch(GetAction(patches, m_op, m_operationData.GetDataToOperandData()))
        {
        case ActionTable::Install:
            return new   InstallPerformers::MspPerformer(m_items
                                                            , *patches
                                                            , m_subFailureAction
                                                            , m_operationData.GetPackageData().GetServicingTrain()
                                                            , m_logger
                                                            , m_uxLogger);
        case ActionTable::Uninstall:
            return new UninstallPerformers::MspPerformer(m_items
                                                        , *patches
                                                        , m_subFailureAction
                                                        , m_logger
                                                        , m_uxLogger);
        case ActionTable::Repair:
            return new    RepairPerformers::MspPerformer(m_items
                                                        , *patches
                                                        , m_subFailureAction
                                                        , m_operationData.GetPackageData().GetServicingTrain()
                                                        , m_logger
                                                        , m_uxLogger);
        case ActionTable::Noop:
            return new DoNothingPerformer();
        default:
            IMASSERT2(0, L"GetAction returned an invalid action type; can't create performer");
            LOG(m_logger, ILogger::Warning, L"GetAction returned an invalid action type; creating DoNothingPerformer");
            return new DoNothingPerformer();
        }
    }
    IPerformer* CreateExePerformer(const Exe* exe)
    {
        LOG(m_logger, ILogger::Verbose, L"Creating new Performer for Exe item");
        const ExeBase* pRepairExe = NULL;
        const ExeBase* pUninstallExe = NULL;
        switch(GetAction(exe, m_op, m_operationData.GetDataToOperandData()))
        {
        case ActionTable::Install:
            return new   InstallPerformers::ExePerformer(*exe, m_bSerialDownload, m_logger, m_uxLogger);
        case ActionTable::Uninstall:
            pUninstallExe = exe->GetUninstallOverride();

            // If there is an UninstallOverride Exe available, that needs to be used to uninstall
            // this 'exe' item, use it.

            if (pUninstallExe)
                return new UninstallPerformers::ExePerformer(*pUninstallExe, m_bSerialDownload, m_logger, m_uxLogger);
            else
                return new UninstallPerformers::ExePerformer(*exe, m_bSerialDownload, m_logger, m_uxLogger);
        case ActionTable::Repair:
            pRepairExe = exe->GetRepairOverride();

            // If there is an RepairOverride Exe available, that needs to be used to Repair
            // this 'exe' item, use it.

            if (pRepairExe)
                return new RepairPerformers::ExePerformer(*pRepairExe, m_bSerialDownload, m_logger, m_uxLogger);
            else
                return new RepairPerformers::ExePerformer(*exe, m_bSerialDownload, m_logger, m_uxLogger);

        case ActionTable::Noop:
            return new DoNothingPerformer();

        default:
            IMASSERT2(0, L"GetAction returned an invalid action type; can't create performer");
            LOG(m_logger, ILogger::Warning, L"GetAction returned an invalid action type; creating DoNothingPerformer");
            return new DoNothingPerformer();
        }
    }

    ActionTable::Actions GetAction(const ItemBase* pItem, Operation::euiOperation operation, const IProvideDataToOperand& dataToOperand)
    {
        HRESULT hr = S_OK;
        BOOL fResult = FALSE;

        const IsPresent* ip = dynamic_cast<const IsPresent*>(pItem);
        if (!ip)
        {
            m_currentAction = UxEnum::aInstall;
            return ActionTable::Install; // this should only happen in our test code!
        }
        bool isPresent = ip->Evaluate(dataToOperand);

        // When rolling back, "Uninstall" action needs to be performed,
        // irrespective of Authored UninstallAction
        if (isPresent && m_rollingBack)
        {
            return ActionTable::Uninstall;
        }

        if (pItem->ActionStateUpdated())
        {
            ACTION_STATE actionState = ACTION_STATE_NONE;
            pItem->GetPackageState(NULL, NULL, NULL, &actionState);
            if ((ACTION_STATE_INSTALL == actionState) || (ACTION_STATE_PATCH == actionState))
            {
                return ActionTable::Install;
            }
            else if (ACTION_STATE_UNINSTALL == actionState)
            {
                return ActionTable::Uninstall;
            }
            else if ((ACTION_STATE_RECACHE == actionState) || (ACTION_STATE_MAINTENANCE == actionState))
            {
                return ActionTable::Repair;
            }
            else if (ACTION_STATE_NONE == actionState)
            {
                return ActionTable::Noop;
            }
            else
            {
                LOGEX(m_logger, ILogger::Warning, L"Unsupported action state - %d", actionState);
            }
        }

        const ActionTable* ac = dynamic_cast<const ActionTable*>(pItem);
        switch(operation)
        {
        case Operation::uioInstalling:
            {
                m_currentAction = UxEnum::aInstall;
                return ac->GetInstallAction(isPresent);
            }
        case Operation::uioUninstalling:
            {
                m_currentAction = UxEnum::aUninstall;
                return ac->GetUninstallAction(isPresent);
            }
        case Operation::uioRepairing:
            {
                m_currentAction = UxEnum::aRepair;
                return ac->GetRepairAction(isPresent);
            }
        default:
            {
                m_currentAction = UxEnum::aNone;
                return ActionTable::Noop;
            }
        }

    LExit:
        return ActionTable::Noop;
    }

    // For current ui operation, m_op, this method returns Failure Action
    // (Rollback, Stop or Continue)behavior for this item.
    FailureActionEnum GetItemFailureAction(const ItemBase* pItem)
    {
        const ActionTable* ac = dynamic_cast<const ActionTable*>(pItem);
        switch(m_op)
        {
        case Operation::uioInstalling:      return ac->GetInstallFailureAction();
        case Operation::uioUninstalling:    return ac->GetUninstallFailureAction();
        case Operation::uioRepairing:       return ac->GetRepairFailureAction();
        }
        // If we get here, there is a new Actiontable row implemented for which OnFailureBehavior was not defined.
        IMASSERT(0 && L"OnFailureBehavior is not implemented for this Action table row!");
        return ac->GetInstallFailureAction();
    }



    virtual HRESULT CreateItemPerformer(
        __in const ItemBase* pItem,
        __out IPerformer* pPerformer
        )
    {
        HRESULT hr = S_OK;

        switch(pItem->GetType())
        {
        case ItemBase::Patches:
            {
                m_currentPerformer = CreatePatchesPerformer(static_cast<const Patches*>(pItem));
                break;
            }
        case ItemBase::Msi:
            {
                m_currentPerformer = CreateMsiPerformer(static_cast<const MSI*>(pItem));
                break;
            }
        case ItemBase::AgileMsi:
            {
                m_currentPerformer = CreateAgileMsiPerformer(static_cast<const AgileMSI*>(pItem));
                break;
            }
        case ItemBase::Msp:
            {
                m_currentPerformer = CreateMspPerformer(static_cast<const MSP*>(pItem));
                break;
            }
        case ItemBase::Exe:
            {
                m_currentPerformer = CreateExePerformer(static_cast<const Exe*>(pItem));
                break;
            }
        case ItemBase::ServiceControl:
            {
                m_currentPerformer = CreateServiceControlPerformer(static_cast<const ServiceControl*>(pItem));
                break;
            }
        case ItemBase::CleanupBlockType:
            {
                m_currentPerformer = CreateCleanupBlockPerformer(static_cast<const CleanupBlock*>(pItem));
                break;
            }
        case ItemBase::RelatedProductsType:
            {
                m_currentPerformer = CreateRelatedProductsPerformer(static_cast<const RelatedProducts*>(pItem));
                break;
            }
        case ItemBase::File:
            {
                m_currentPerformer = new DoNothingPerformer();
                LOG(m_logger, ILogger::Verbose, L"Created new DoNothingPerformer for File item");
                break;
            }
        default:
            HIASSERT(false, L"Unsupported Item Type!");
            hr = E_INVALIDARG;
            break;
        }

        return hr;
    }


private: // "template method" refactorings
    virtual void Rollback(IProgressObserver& observer, unsigned int lastSuccessful, IInstallItems& items,  IBurnView *pBurnView, bool bSerialDownload, ILogger& logger, Ux& uxLogger) = 0;
    virtual unsigned int MapIndex(unsigned int i, IInstallItems&) = 0;

private:
    virtual void Sleep(__in DWORD dwMilliseconds) { ::Sleep(dwMilliseconds); }
    virtual HANDLE OpenMutex(__in DWORD dwDesiredAccess, __in BOOL bInheritHandle, __in LPCWSTR lpName) { return ::OpenMutex(dwDesiredAccess, bInheritHandle, lpName); }
    virtual BOOL CloseHandle(__in HANDLE hMutex) { return ::CloseHandle(hMutex); }

public:
    virtual bool InInstallMode(void) { return false; }

};

template<typename InstallPerformers, typename UninstallPerformers, typename RepairPerformers>
class CompositeUninstallerT : public CompositePerformerBaseT<InstallPerformers, UninstallPerformers, RepairPerformers>
{
public:
    CompositeUninstallerT(IInstallItems& items
                            , ICacheManager& cacheManager
                            , IBurnView *pBurnView
                            , const IOperationData& operationData
                            , bool bSerialDownload
                            , ILogger& logger
                            , Ux& uxLogger)
        : CompositePerformerBaseT<InstallPerformers, UninstallPerformers, RepairPerformers>
                (   Operation::uioUninstalling
                    , items
                    , cacheManager
                    , pBurnView
                    , operationData
                    , FailureActionEnum::GetContinueAction()
                    , bSerialDownload
                    , logger
                    , uxLogger)
    {}
    virtual ~CompositeUninstallerT() {}

private:
    virtual void Rollback(IProgressObserver& observer, unsigned int lastSuccessful, IInstallItems& items, IBurnView *pBurnView, bool bSerialDownload, ILogger& logger, Ux& uxLogger) {} // nothing to do

private: // reverse order of loop via "template method"
    virtual unsigned int MapIndex(unsigned int i, IInstallItems& items) { return items.GetCount()-i-1; }
};

typedef CompositeUninstallerT<Performers<MsiInstaller,   MspInstaller,   ExeSelectingInstaller,     AgileMsiInstaller,   ServiceControlInstaller,   CleanupBlockInstaller, RelatedProductsRepairer>,
                              Performers<MsiUnInstaller, MspUninstaller, ExeSelectingUnInstaller,   AgileMsiUnInstaller, ServiceControlUnInstaller, CleanupBlockInstaller, RelatedProductsUninstaller>,
                              Performers<MsiRepairer,    MspRepairer,    ExeSelectingRepairer,      AgileMsiRepairer,    ServiceControlRepairer,    CleanupBlockInstaller, RelatedProductsRepairer>
                              > CompositeUninstaller;

template<typename InstallPerformers, typename UninstallPerformers, typename RepairPerformers, typename Uninstaller = CompositeUninstaller>
class CompositeInstallerT : public CompositePerformerBaseT<InstallPerformers, UninstallPerformers, RepairPerformers>
{
    ICacheManager& m_cacheManager;

public:
    CompositeInstallerT(IInstallItems& items
                        , ICacheManager& cacheManager
                        , IBurnView *pBurnView
                        , const IOperationData& operationData
                        , const FailureActionEnum& subFailureAction
                        , bool bSerialDownload
                        , ILogger& logger
                        , Ux& uxLogger)
        : CompositePerformerBaseT<InstallPerformers, UninstallPerformers, RepairPerformers>
            (Operation::uioInstalling
                    , items
                    , cacheManager
                    , pBurnView
                    , operationData
                    , subFailureAction
                    , bSerialDownload
                    , logger
                    , uxLogger)
        , m_cacheManager(cacheManager)
    {}
    virtual ~CompositeInstallerT() {}
    virtual bool InInstallMode(void) { return true; }

private:
    virtual void Rollback(IProgressObserver& observer
                        , unsigned int lastSuccessful
                        , IInstallItems& items
                        , IBurnView *pBurnView
                        , bool bSerialDownload
                        , ILogger& logger
                        , Ux& uxLogger)
    {
        // create an IInstallItems list of successfully installed items
        class UninstallItems : public InstallItemsBase
        {
            CSimpleArray<const ItemBase*> m_items;
        public:
            // Uninstall (rollback) items when the following conditions are true:
            // Unless they are marked with the Rollback="false" attribute. Default is true
            UninstallItems(unsigned int lastSuccessful
                            , const IInstallItems& items
                            , const IOperationData& operationData)
            {
                for(unsigned int i=0; i<=lastSuccessful; ++i)
                {
                    bool bInstalled = false;
                    const IsPresent* ip = dynamic_cast<const IsPresent*>(items.GetItem(i));
                    if (!ip)
                        bInstalled = true; // this should only happen in our test code!
                    else
                        bInstalled = ip->Evaluate(operationData.GetDataToOperandData());

                    const RollbackOnPackageInstallFailure* rollbackOnPackageInstallFailure = dynamic_cast<const RollbackOnPackageInstallFailure*>(items.GetItem(i));
                    if (   bInstalled
                        && rollbackOnPackageInstallFailure != NULL
                        && rollbackOnPackageInstallFailure->ShouldRollBack())
                    {
                        m_items.Add(items.GetItem(i));
                    }
                }
            }

        private:
            virtual unsigned int GetCount() const { return m_items.GetSize(); }
            virtual const ItemBase* GetItem(unsigned int nIndex) const { return m_items[nIndex]; }
            virtual unsigned int GetChildItemCount(unsigned int nParentIndex) const { return 0; }
            virtual const ItemBase* GetChildItem(unsigned int nParentIndex, unsigned int nChildIndex) const { return NULL; }
            virtual CString GetItemID(unsigned int nIndex) const { return L""; }
            virtual bool IsItemAvailable(unsigned int nIndex) const { return true; }
            virtual void UpdateItemState(unsigned int nIndex) {}
            virtual bool IsItemComplete(unsigned int nIndex) const { return false;}
            virtual bool IsItemNotAvailableAndIgnorable(unsigned int nIndex) const { return false; }
            virtual bool IsItemAvailableUnVerified(unsigned int nIndex) const { return false; }
            virtual void SetItemStateAsAvailable(unsigned int nIndex) {}
            virtual void UpdateItemPath(unsigned int nIndex, const CPath& itemPath) {}
            virtual bool IsItemOnDownloadList(unsigned int nIndex) const { return false;}
            virtual bool DoesItemNeedToBeCached(unsigned int nIndex, CPath& pathCachedFileName, bool& bPerMachine) const { return false; }
            virtual void SetItemStateToIgnorable(unsigned int nIndex) const {}
            virtual bool VerifyItem(unsigned int nIndex, ILogger& logger) { return false; }
        } uninstallItems(lastSuccessful, items, m_operationData);

        Uninstaller uninstaller(uninstallItems
                                , m_cacheManager
                                , pBurnView
                                , m_operationData
                                , bSerialDownload
                                , logger
                                , uxLogger);

        // Set the rollback flag on at uninstaller.
        uninstaller.SetRollbackFlag(true);

        InvertingProgressObserver invertingObserver(items.GetCount(), lastSuccessful+1, observer);
        uninstaller.PerformAction(invertingObserver);
    }

private:
    virtual unsigned int MapIndex(unsigned int i, IInstallItems&) { return i; }
};

typedef CompositeInstallerT  <Performers<MsiInstaller,   MspInstaller,   ExeSelectingInstaller,     AgileMsiInstaller,   ServiceControlInstaller,   CleanupBlockInstaller, RelatedProductsRepairer>,
                              Performers<MsiUnInstaller, MspUninstaller, ExeSelectingUnInstaller,   AgileMsiUnInstaller, ServiceControlUnInstaller, CleanupBlockInstaller, RelatedProductsUninstaller>,
                              Performers<MsiRepairer,    MspRepairer,    ExeSelectingRepairer,      AgileMsiRepairer,    ServiceControlRepairer,    CleanupBlockInstaller, RelatedProductsRepairer>
                              > CompositeInstaller;

template<typename InstallPerformers, typename UninstallPerformers, typename RepairPerformers>
class CompositeRepairerT : public CompositePerformerBaseT<InstallPerformers, UninstallPerformers, RepairPerformers>
{
public:
    CompositeRepairerT(IInstallItems& items
                        , ICacheManager& cacheManager
                        , IBurnView *pBurnView
                        , const IOperationData& operationData
                        , bool bSerialDownload
                        , ILogger& logger
                        , Ux& uxLogger)
        : CompositePerformerBaseT<InstallPerformers, UninstallPerformers, RepairPerformers>
                    (   Operation::uioRepairing
                        , items
                        , cacheManager
                        , pBurnView
                        , operationData
                        , FailureActionEnum::GetContinueAction()
                        , bSerialDownload
                        , logger
                        , uxLogger)
    {}
    virtual ~CompositeRepairerT() {}

private:
    virtual void Rollback(IProgressObserver& observer, unsigned int lastSuccessful, IInstallItems& items, IBurnView *pBurnView, bool bSerialDownload, ILogger& logger, Ux& uxLogger) {} // nothing to do

private: // forward order loop via "template method"
    virtual unsigned int MapIndex(unsigned int i, IInstallItems&) { return i; }
};

typedef CompositeRepairerT   <Performers<MsiInstaller,   MspInstaller,   ExeSelectingInstaller,     AgileMsiInstaller,   ServiceControlInstaller,   CleanupBlockInstaller, RelatedProductsRepairer>,
                              Performers<MsiUnInstaller, MspUninstaller, ExeSelectingUnInstaller,   AgileMsiUnInstaller, ServiceControlUnInstaller, CleanupBlockInstaller, RelatedProductsUninstaller>,
                              Performers<MsiRepairer,    MspRepairer,    ExeSelectingRepairer,      AgileMsiRepairer,    ServiceControlRepairer,    CleanupBlockInstaller, RelatedProductsRepairer>
                              > CompositeRepairer;

class CompositePerformer : public ICompositePerformer
{
    IPerformer* m_performer;
    LiveOperation m_liveOperation;
    const IOperationData& m_operationData;

    bool m_performingInElevatedProcess;
    UINT m_elevatedItemIndex;
    ILogger& m_logger;
    Operation::euiOperation m_operation;
    ActionTable::Actions m_action;
    ICacheManager& m_cacheManager;

    struct ConstructionCommandPattern
    {
        ICoordinator& m_coordinator;
        const FailureActionEnum m_subFailureAction;
        ILogger& m_logger;
        Ux& m_uxLogger;
        const IProvideDataToUi& m_provideDataToUi;
        bool m_bSerialDownload;

        ConstructionCommandPattern( ICoordinator& coordinator
                                    , const IProvideDataToUi& dataToUi
                                    , const FailureActionEnum& subFailureAction
                                    , ILogger& logger
                                    , Ux& uxLogger)
            : m_coordinator(coordinator)
            , m_subFailureAction(subFailureAction)
            , m_logger(logger)
            , m_uxLogger(uxLogger)
            , m_provideDataToUi(dataToUi)
            , m_bSerialDownload(dataToUi.IsSimultaneousDownloadAndInstallDisabled())
        {
        }
    } m_ccp;

public:
    CompositePerformer( Operation::euiOperation op
                        , ICoordinator& coordinator
                        , ICacheManager& cacheManager
                        , const IOperationData& operationData
                        , const IProvideDataToUi& dataToUi
                        , const FailureActionEnum& subFailureAction
                        , ILogger& logger
                        , Ux& uxLogger)
        : m_liveOperation(op)
        , m_performer(&NullPerformer::GetNullPerformer())
        , m_operationData(operationData)
        , m_ccp(coordinator, dataToUi, subFailureAction, logger, uxLogger)
        , m_cacheManager(cacheManager)
        , m_performingInElevatedProcess(false)
        , m_elevatedItemIndex(0)
        , m_logger(logger)
    {}
    virtual ~CompositePerformer() {}
    void SetEngineDataProvider(IProvideDataToEngine* provider)
    {
        m_liveOperation.SetEngineDataProvider(provider);
    }

public: // IPerformer
    virtual void PerformAction(IProgressObserver& observer)
    {
        CString log(L"calling PerformAction on ");
        bool bSerialDownload = m_ccp.m_provideDataToUi.IsSimultaneousDownloadAndInstallDisabled();

        IBurnView *pBurnView = NULL;

        HRESULT hr =  m_ccp.m_provideDataToUi.GetBurnView(&pBurnView);
        ExitOnFailure(hr, "Failed to get IBurnView.");


        Operation::euiOperation operation = m_liveOperation.GetOperationFromUi();

        if (m_performingInElevatedProcess)
        {
            operation = m_operation;
        }
        else
        {
            switch (pBurnView->GetCurrentAction())
            {
            case BURN_ACTION_INSTALL:
                operation = Operation::uioInstalling;
                break;
            case BURN_ACTION_REPAIR:
                operation = Operation::uioRepairing;
                break;
            case BURN_ACTION_UNINSTALL:
                operation = Operation::uioUninstalling;
                break;
            case BURN_ACTION_MODIFY:
                operation = Operation::uioRepairing;
                break;
            default:
                hr = E_INVALIDARG;
                ExitOnFailure(hr, "Invalid action state.");
            }
        }

        switch(operation)
        {
        case Operation::uioInstalling:
            log += L"an installing performer";
            m_performer = new CompositeInstaller  (m_ccp.m_coordinator.GetInstallItems()
                                                    , m_cacheManager
                                                    , pBurnView
                                                    , m_operationData
                                                    , m_ccp.m_subFailureAction
                                                    , m_ccp.m_bSerialDownload
                                                    , m_ccp.m_logger
                                                    , m_ccp.m_uxLogger);
            break;
        case Operation::uioUninstalling:
            log += L"an uninstalling performer";
            m_performer = new CompositeUninstaller(m_ccp.m_coordinator.GetInstallItems()
                                                    , m_cacheManager
                                                    , pBurnView
                                                    , m_operationData
                                                    , m_ccp.m_bSerialDownload
                                                    , m_ccp.m_logger
                                                    , m_ccp.m_uxLogger);
            break;
        case Operation::uioRepairing:
            log += L"a repairing performer";
            m_performer = new CompositeRepairer   (m_ccp.m_coordinator.GetInstallItems()
                                                    , m_cacheManager
                                                    , pBurnView
                                                    , m_operationData
                                                    , m_ccp.m_bSerialDownload
                                                    , m_ccp.m_logger
                                                    , m_ccp.m_uxLogger);
            break;
        default:
            IMASSERT(0 && L"unsupported performer - doing nothing!");
            LOG(m_ccp.m_logger, ILogger::Verbose, L"unsupported performer - doing nothing!");
            return;
        }
        LOG(m_ccp.m_logger, ILogger::Verbose, log);

        m_performer->PerformAction(observer);
        IPerformer* tmp = m_performer;
        m_performer = &NullPerformer::GetNullPerformer();
        delete tmp;

        return;

    LExit:
        observer.Finished(hr);

    }

    virtual void Abort()
    {
        m_performer->Abort();
    }

    //
    // GetPackage - returns item/package based on the index.
    //
    virtual HRESULT GetPackage(
        __in UINT index,
        __out const ItemBase** ppItem)
    {
        HRESULT hr = S_OK;
        IInstallItems *pPackages = &m_ccp.m_coordinator.GetInstallItems();
        if (index < pPackages->GetCount())
        {
            *ppItem = pPackages->GetItem(index);
        }
        else
        {
            LOGEX(m_ccp.m_logger, ILogger::Error, L"GetPackage - Item index %d is invalid.", index);
            hr = E_INVALIDARG;
        }

        return hr;
    }

    //
    // CreatePackagePerformer - Creates a performer based on the package and action
    //
    virtual HRESULT CreatePackagePerformer(
        __in const ItemBase* pItem,
        __in const ActionTable::Actions action,
        __in IBurnView *pBurnView,
        __out IPerformer** ppPerformer)
    {
        HRESULT hr = S_OK;


        switch(pItem->GetType())
        {
        case ItemBase::Patches:
            {
                const Patches* patches = static_cast<const Patches*>(pItem);
                hr = CreatePatchesPerformer(
                    action,
                    m_ccp.m_coordinator.GetInstallItems(),
                    *patches,
                    m_ccp.m_subFailureAction,
                    m_operationData,
                    m_ccp.m_logger,
                    m_ccp.m_uxLogger,
                    ppPerformer,
                    pBurnView
                    );
                break;
            }

        case ItemBase::Msi:
            {
                const MSI* msi = static_cast<const MSI*>(pItem);
                hr = CreateMsiPerformer(
                    action,
                    *msi,
                    m_ccp.m_logger,
                    m_ccp.m_uxLogger,
                    pBurnView,
                    ppPerformer
                    );
                break;
            }

        case ItemBase::AgileMsi:
            {
                const AgileMSI* agileMsi = static_cast<const AgileMSI*>(pItem);
                hr = CreateAgileMsiPerformer(
                    action,
                    *agileMsi,
                    m_ccp.m_logger,
                    m_ccp.m_uxLogger,
                    ppPerformer
                    );
                break;
            }

        case ItemBase::Msp:
            {
                const MSP* msp = static_cast<const MSP*>(pItem);
                hr = CreateMspPerformer(
                    action,
                    m_ccp.m_coordinator.GetInstallItems(),
                    *msp,
                    m_ccp.m_subFailureAction,
                    m_operationData,
                    m_ccp.m_logger,
                    m_ccp.m_uxLogger,
                    ppPerformer,
                    pBurnView
                    );
                break;
            }

        case ItemBase::Exe:
            {
                const Exe* exe = static_cast<const Exe*>(pItem);
                hr = CreateExePerformer(
                    action,
                    *exe,
                    m_ccp.m_bSerialDownload,
                    m_ccp.m_logger,
                    m_ccp.m_uxLogger,
                    ppPerformer
                    );
                break;

            }
        case ItemBase::ServiceControl:
            {
                const ServiceControl* serviceControl = static_cast<const ServiceControl*>(pItem);
                hr =  CreateServiceControlPerformer(
                    action,
                    serviceControl,
                    m_ccp.m_logger,
                    m_ccp.m_uxLogger,
                    ppPerformer
                    );
                break;
            }

        case ItemBase::CleanupBlockType:
            {
                const CleanupBlock* cleanupBlock = static_cast<const CleanupBlock*>(pItem);
                CreateCleanupBlockPerformer(
                    action,
                    *cleanupBlock,
                    m_ccp.m_subFailureAction,
                    m_ccp.m_logger,
                    m_ccp.m_uxLogger,
                    ppPerformer
                    );
                break;
            }

        case ItemBase::RelatedProductsType:
            {
                const RelatedProducts* relatedProducts = static_cast<const RelatedProducts*>(pItem);
                hr = CreateRelatedProductsPerformer(
                    action,
                    *relatedProducts,
                    m_ccp.m_subFailureAction,
                    m_ccp.m_logger,
                    m_ccp.m_uxLogger,
                    ppPerformer
                    );
                break;
            }

        case ItemBase::File:
            {
                *ppPerformer = new DoNothingPerformer();
                break;
            }

        default:
            HIASSERT(false, L"Unsupported Item Type!");
            hr = E_INVALIDARG;
            break;
        }

        return hr;
    }

    virtual void StopPerformer()
    {
        CompositeInstaller* pInstallPerformer = dynamic_cast<CompositeInstaller*> (m_performer);
        if (pInstallPerformer)
        {
            pInstallPerformer->StopPerformer();
        }

    }

    //------------------------------------------------------------------------------
    // IsCached
    // Determines if a package is already cached
    //------------------------------------------------------------------------------
    virtual bool IsCached(UINT nIndex)
    {
        return m_cacheManager.IsCached(m_ccp.m_coordinator.GetInstallItems(), nIndex);
    }

    //------------------------------------------------------------------------------
    // VerifyAndCachePackage
    // verify a package and cache it
    //------------------------------------------------------------------------------
    virtual HRESULT VerifyAndCachePackage(UINT nIndex)
    {
        return m_cacheManager.VerifyAndCachePackage(m_ccp.m_coordinator.GetInstallItems(), nIndex);
    }

    //
    // UpdatePackageLocation - updates a packages location
    //
    virtual HRESULT UpdatePackageLocation(UINT nIndex, const CPath& path, bool fVerify)
    {
        if (!fVerify)
        {
            m_ccp.m_coordinator.GetInstallItems().UpdateItemPath(nIndex, path);
            return S_OK;
        }
        else
        {
            CPath pathOldName(m_ccp.m_coordinator.GetInstallItems().GetItemName(nIndex));
            m_ccp.m_coordinator.GetInstallItems().UpdateItemPath(nIndex, path);
            if ( !m_ccp.m_coordinator.GetInstallItems().VerifyItem(nIndex, m_logger) )
            {
                m_ccp.m_coordinator.GetInstallItems().UpdateItemPath(nIndex, pathOldName);
                return E_FAIL;
            }

            return S_OK;
        }
    }
};
};
