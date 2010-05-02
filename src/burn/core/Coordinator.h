//-------------------------------------------------------------------------------------------------
// <copyright file="Coordinator.h" company="Microsoft">
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
//      This is the coordinator that coordinate install and download.
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once

#include "common\Operation.h"
#include "schema\CustomError.h"

#include "ICoordinator.h"
#include "InstallItemsBase.h"
#include "CmdLineParser.h"
#include "CheckTrust.h"
#include "inc\IBurnView.h"

namespace IronMan
{

template<typename FileAuthenticity>
struct ItemStateGeneratorT
{

    static bool VerifyItem(const ItemBase* pItem, ILogger& logger)
    {
        if (!pItem) return false; // silence prefast

        const LocalPath* localPath = dynamic_cast<const LocalPath*>(pItem);
        if (!localPath)
            return false;
        if (localPath->GetName().FileExists())
        {
            const DownloadPath* downloadPath = dynamic_cast<const DownloadPath*>(pItem);
            if (downloadPath) // might be media only, in which case we never downloaded anything.
            {
                CPath file = localPath->GetName(); // must make a copy, otherwise the copy goes out of scope at the ; at the end of the following line.
                CString strFriendlyName(localPath->GetOriginalName());
                FileAuthenticity fileAuthenticity(strFriendlyName,file, downloadPath->GetHashValue(), downloadPath->GetItemSize(), logger);
                HRESULT hr = fileAuthenticity.Verify();
                if (FAILED(hr))
                {
                    LOGEX(logger, ILogger::Result, L"File %s (%s), failed authentication. (Error = %d). It is recommended that you delete this file and retry setup again.", strFriendlyName, file, hr);
                    return false;
                }
            }
        }
        else
        {
            LOGEX(logger, ILogger::Verbose, L"File not found: %s", localPath->GetName());
            return false;
        }

        return true;
    }

    static bool LocalPathExists(const ItemBase* pItem, int createLayoutPathLength, Operation::euiOperation operation, ILogger& logger, bool bPerformFileVerificaiton = false)
     /* //////////////////////////////////////////////////////////////////////////////
     LocalPathExists
     Receives:  ItemBase* -                 Item to check
                createLayoutPathLength -    Length of the createlayout folder 
                                            passed in by the user. 
                                            If length is > 0, installer is runing
                                            in Creatlayout mode.
                Operation -                 One of the (install, repair, uninstall 
                                            or createlayout) operation.
                bPerformFileVerificaiton    When bPerformFileVerificaiton is true, 
                                            this function also performs validation 
                                            as implemented in file authenticity and 
                                            locks the items's file if valid to avoid
                                            the file being modified after authentication.

     Returns :  bool -                      True if item in question is present (exists)
                                            at correct location. False else.
     Purpose :  This function checks to see if the content required for this pItem exists/available.
                If operation is anything other than CreateLayout, this function checks 
                to see if the file exists. When the opration is CreateLayout, this 
                function looks for the item in the layout folder if it is already present.
                If present, item is considered "Available". For LocalExe type of Exe item
                we don't need to check availability when are in CreateLayout mode (length > 0).
                Check is required only operation is not CreateLayout operation. 
                This can be summarized as follows:

                -----------------------------------------------------
                : Operation     | CreateLayout Mode | Other Mode    | 
                -----------------------------------------------------
                : Install       | Ignore            | Check         |
                : Uninstall     | Ignore            | Check         |
                : Repair        | Ignore            | Check         |
                : CreateLayout  | Ignore            | Ignore        |
                -----------------------------------------------------

    ////////////////////////////////////////////////////////////////////////////////// */
    {
        // To perform file verificaion, call VerifyItem() instead of calling this method. 
        IMASSERT (bPerformFileVerificaiton == false);

        if (!pItem) return false; // silence prefast

        const IronMan::ServiceControl* sc = dynamic_cast <const IronMan::ServiceControl*>(pItem);
        // We don't need any binary for service (we dont install service using this installer)
        if (sc)
            return true;
        
        // cleanup block doesn't require any payload.
        if (dynamic_cast <const IronMan::CleanupBlock*>(pItem))
            return true;

        const LocalPath* localPath = dynamic_cast<const LocalPath*>(pItem);
        if (!localPath)
            return false;

        bool bLocalExe = false;
        
        const IronMan::ExeBase* exe = dynamic_cast<const IronMan::ExeBase *>(pItem);
        if (NULL != exe)
        {
            // We should defer the validation of local exe as it is too soon here
            // and we will block CreateLayout scenario if not.
            if (exe->GetExeType().IsLocalExe())
                bLocalExe = true;
        }

        // When calculating CreateLayout state, for localexes, we shouldn't check if it exists.
        // This is independent of Installer running with CreateLayout mode or not
        if (bLocalExe && (operation == Operation::uioCreateLayout || createLayoutPathLength > 0))
        {
            return true; 
        }

        // if MSI or MSP, no need to download iff we're uninstalling
        if ((pItem->GetType() == ItemBase::Msi) || (pItem->GetType() == ItemBase::Msp) )
        {
            if (CCmdLineSwitches().UninstallMode())
                return true;
        }

        // Installer is in CreateLayout mode.
        if (createLayoutPathLength)
        {
            CString exeFolder;
            ::GetModuleFileName(NULL, exeFolder.GetBuffer(MAX_PATH), MAX_PATH);
            exeFolder._ReleaseBuffer();
            CPath exePath(exeFolder);
            exePath.RemoveFileSpec();
            exeFolder = CString(exePath);

            CString name = localPath->GetName();
            name = name.Mid(createLayoutPathLength);

            if (::PathFileExists(exeFolder + name))
                return true;
        }
        return TRUE == ::PathFileExists(CString(localPath->GetName()));
    }
    static bool HasUrl(const ItemBase* pItem)
    {
        const DownloadPath* download  = dynamic_cast<const DownloadPath*>(pItem);
        if (download == NULL)
            return false;
        return !download->GetUrl().IsEmpty();
    }

    static bool IsItemApplicable(const ItemBase* pItem, const IProvideDataToOperand& ipdtoDataToOperand)
    {
        const ApplicableIf* applies = dynamic_cast<const ApplicableIf*>(pItem);
        if (applies == NULL)
            return true;
        return applies->Evaluate(ipdtoDataToOperand);
    }

    // Returns authoredActionTable Action (install, uninstall, repair or noop) based on the 
    //                         "operation"(install, uninstall, repair or createlayout)
    // and if item's IsPresent evaluates  (true or false).
    // CalculateState() is going to use this install action to evaluate state of the give item.
    static ActionTable::Actions GetItemAction(
        const ItemBase* pItem
        , Operation::euiOperation operation
        , const IProvideDataToOperand& dataToOperand)
    {
        const IsPresent* ip = dynamic_cast<const IsPresent*>(pItem);
        if (ip == NULL)
        {
            if (pItem->GetType() != ItemBase::File)
            {
                IMASSERT(0 && L"All items should have IsPresent. Validation should have caught this.");
            }
            return ActionTable::Install; 
        }

        bool isPresent = ip->Evaluate(dataToOperand);

        const ActionTable* actionTable = dynamic_cast<const ActionTable*>(pItem);
        switch(operation)
        {
        case Operation::uioInstalling:      return actionTable->GetInstallAction(isPresent);
        case Operation::uioUninstalling:    return actionTable->GetUninstallAction(isPresent);
        case Operation::uioRepairing:       return actionTable->GetRepairAction(isPresent);
        default:
            return ActionTable::Install; // For createLayout we want to Install
        }
    }

    // True if InstallAction for current operation based on IsPresent is NoOp
    static bool IsActionNoop(const ItemBase* pItem, Operation::euiOperation operation, const IProvideDataToOperand& ipdtoDataToOperand)
    {
        return ActionTable::Noop == GetItemAction(pItem, operation, ipdtoDataToOperand);
    }

    // MSI, AgileMSI, MSP and Patches etc are not Required during Repair or Uninstall
    // ServiceControl is not required during any operation.
    // Takes the install action as input.
    static bool IsPayloadRequired(const ItemBase* pItem, ActionTable::Actions installAction)
    {
        const ActionTable* actionTable = dynamic_cast<const ActionTable*>(pItem);

        if (!actionTable)
        {
            return true; // Unless action table says otherwise, payload will be required.
        }


        return actionTable->IsPayloadRequired(installAction);
    }

};

typedef ItemStateGeneratorT<FileAuthenticity> ItemStateGenerator;

template<typename FileAuthenticity>
class ItemStateDecoratorT : public ItemBase
{
public:
    enum ItemState
    {
        NotApplicable,      // doesn't apply
        Applicable,
        Available,          // available locally, either because it was always there or because it's been downloaded
        AvailableUnVerified,// available locally but verfication is deferred
        NotAvailable,       // not local (yet)
        NotAvailableAskUX,  // either an URL is not specified or the UX asked us not to download this item
        NotAvailableAndIgnorable, // item's download failed but is ok to ignore.
        Error,              // not available and no URL
        Complete            // installed, repaired or uninstalled
    };

private:
    Operation::euiOperation m_operation;
    // Initialize m_item first.
    ItemBase* m_item;
    ItemState m_state;
    const IProvideDataToOperand& m_dataToOperand;

public:
    // Constructor that does not calculate item state.
    ItemStateDecoratorT(const ItemBase* pItem
                        , const IProvideDataToOperand& dataToOperand
                        , ILogger& logger = NullLogger::GetNullLogger())
                        : ItemBase(pItem->GetType(), pItem->GetEstimatedInstallTime(), pItem->IsPerMachine(), pItem->GetId())
        , m_operation(Operation::uioInstalling)
        , m_item(pItem->Clone())
        , m_state(NotApplicable)
        , m_dataToOperand(dataToOperand)
    {
    }

    ItemStateDecoratorT(const ItemBase* pItem
                        , Operation::euiOperation operation
                        , const IProvideDataToOperand& dataToOperand
                        , ILogger& logger = NullLogger::GetNullLogger())
        : ItemBase(pItem->GetType(), pItem->GetEstimatedInstallTime(), pItem->IsPerMachine(), pItem->GetId())
        , m_operation(operation)
        , m_item(pItem->Clone())
        , m_state(NotApplicable)
        , m_dataToOperand(dataToOperand)
    {
        m_state = CalculateState(pItem, operation, dataToOperand, logger);
    }
    // Can be used to calculate state of the item only ignoring its Helper and AgileMsp like child items.
    ItemStateDecoratorT(const ItemBase* pItem
                        , Operation::euiOperation operation
                        , const IProvideDataToOperand& dataToOperand
                        , bool bRestrictToParentItemState
                        , ILogger& logger = NullLogger::GetNullLogger())
        : ItemBase(pItem->GetType(), pItem->GetEstimatedInstallTime())
        , m_operation(operation)
        , m_item(pItem->Clone())
        , m_state(NotApplicable)
        , m_dataToOperand(dataToOperand)
    {
        m_state = CalculateState(pItem, operation, dataToOperand, logger, bRestrictToParentItemState);
    }

    // Detect - Calculates applicability of the item.
    HRESULT Detect()
    {
        if (ItemStateGenerator::IsItemApplicable(m_item, m_dataToOperand))
        {
            m_state = Applicable;
            PACKAGE_STATE packageState = GetPackageCurrentState();
            const ItemBase* pItem = m_item;
            const_cast<ItemBase*>(pItem)->SetDetectState(packageState);
            const_cast<ItemBase*>(pItem)->SetApplicability(true);
        }
        else
        {
            m_state = NotApplicable;
        }

        return S_OK;
    }

    // Plan updates the state with the availability information.
    HRESULT Plan(__in IBurnView* pBurnView, __in Operation::euiOperation operation, __in ILogger& logger)
    {
        if (operation == Operation::uioNone)
        {
            m_state = NotApplicable;
        }
        else
        {
            m_state = CalculateState(m_item, operation, m_dataToOperand, logger);
        }

        return S_OK;
    }

    virtual ~ItemStateDecoratorT()
    {
        delete m_item;
    }

    const ItemBase& GetItem() const 
    { 
        return *m_item; 
    }

    ItemState GetState() const { return m_state; }

    PACKAGE_STATE GetPackageCurrentState()
    {
        IsPresent* ip = dynamic_cast<IsPresent*>(m_item);
        if (ip == NULL)
        {
            IMASSERT(0 && L"All items should have IsPresent. Validation should have caught this.");
            return PACKAGE_STATE_UNKNOWN;
        }

        bool isPresent = ip->Evaluate(m_dataToOperand);

        if (isPresent)
        {
            return PACKAGE_STATE_PRESENT;
        }
        else
        {
            return PACKAGE_STATE_ABSENT;
        }
    }

    void UpdateState(ItemState newState) { m_state = newState; }

    // Compares two states and returns the worst of them (higher state is worst state)
    inline static ItemState GetWorstOfStates(ItemState a, ItemState b)
    {
        return (a > b)? a : b;
    }
private:

    // Given an item, for this operation, based on installaction, this will determine if item needs to be available or not.
    static ItemState ItemAvailability(const ItemBase* pItem, Operation::euiOperation operation, ActionTable::Actions installAction, ILogger& logger, CString& section)
    {
        if (!ItemStateGenerator::IsPayloadRequired(pItem, installAction))
        {
            section += L" - payload not required for this item to perform action.";
            return Available;
        }

        bool bLocalExe = false;
        const IronMan::ExeBase* exe = dynamic_cast<const IronMan::ExeBase *>(pItem);
        if (NULL != exe)
        {
            // We should defer the validation of local exe as it is too soon here
            // and we will block CreateLayout scenario if not.
            bLocalExe = exe->GetExeType().IsLocalExe();
        }

        if (ItemStateGeneratorT<FileAuthenticity>::LocalPathExists(
                      pItem
                    , CCmdLineSwitches().CreateLayout().GetLength()
                    , operation
                    , logger))
        {
            // If file exists and
            // we are running in createlayout mode 
            // or is a local exe, 
            // item is "Available"
            if (Operation::uioCreateLayout == operation || bLocalExe)
            {
                section += L" - available locally";
                return Available;
            }
            else
            {
                section += L" - available but not verified yet";
                return AvailableUnVerified;
            }
        }

        // Check to see if it is in the Attached Container
        const LocalPath* pLocalPath = dynamic_cast<const LocalPath*>(pItem);
        if (NULL != pLocalPath)
        {
            if ( ModuleUtils::IsPayloadEmbedded(pLocalPath->GetOriginalName()) )
            {
                section += L" - available but not verified yet";
                return AvailableUnVerified;
            }
        }

        // Item doesn't exist.
        if (ItemStateGenerator::HasUrl(pItem))
        {
            section += L" - to be downloaded";
            return NotAvailable;
        }
        else
        {
            // Item doesn't exist and no URL was authored for it.
            CString warningMessage = L" not locally available, but no URL to be downloaded - warning!";
            section += warningMessage;
            LOG(logger, ILogger::Warning, warningMessage);
            return NotAvailableAskUX;
        }
    }

    // Given pItem, this method will calculate it's state alone, disregarding its siblings and child items.
    // As this performs generic function, passing uioperation in so that CalcualteState()functions can reuse.
    static ItemState CalculateJustItemState(const ItemBase* pItem, Operation::euiOperation operation, const IProvideDataToOperand& ipdtoDataToOperand, ILogger& logger)
    {
        CString name(L"nameless item");
        
        const LocalPath* lp = dynamic_cast<const LocalPath*>(pItem);
        const IronMan::CleanupBlock* cub = dynamic_cast <const IronMan::CleanupBlock*>(pItem);

        if (lp)
            name = CString(lp->GetName());
        else
        {
            if (cub)
                name = cub->GetCleanupBlockName();
            const IronMan::ServiceControl* sc = dynamic_cast <const IronMan::ServiceControl*>(pItem);
            if (sc)
                name = sc->GetServiceName();
        }

        CString section = L" of " + name;
        PUSHLOGSECTIONPOP(logger, L"Determining state", L" of " + name, section);

        ActionTable::Actions installAction = ActionTable::Install;

        // For CreateLayout, no applicability, install action or payload requirement is necessary.
        if (Operation::uioCreateLayout != operation)
        {
            if (!ItemStateGenerator::IsItemApplicable(pItem, ipdtoDataToOperand))
            {
                section += L" - not applicable ";
                return NotApplicable;
            }

            installAction = ItemStateGenerator::GetItemAction(pItem, operation, ipdtoDataToOperand);

            if (ActionTable::Noop == installAction)
            {
                section += L" - authored action for this item is NoOp";
                return NotApplicable;
            }

            // For cleanupblock and related products items, it doesn't apply unless there is at least 1 product that is 
            // affected by performing it.
            bool bNoAffectedProducts = false;
            if (cub)
            {
                if (cub->GetAffectedProducts().GetSize() < 1)
                {
                    bNoAffectedProducts = true;
                }
            }
            else
            {
                const IronMan::RelatedProducts* rp = dynamic_cast <const IronMan::RelatedProducts*>(pItem);
                if (rp)
                {
                    if (rp->GetRelatedProducts().GetSize() < 1)
                    {
                        bNoAffectedProducts = true;
                    }
                }
            }
            if (bNoAffectedProducts)
            {
                section += L" - no products affected by this item. Not Applicable. ";
                return NotApplicable;
            }
        }

        return ItemAvailability(pItem, operation, installAction, logger, section);
    }

    // Given an item, this method will calculate combined (worst) state of all its helpers
    static ItemState CalculateHelpersState(const ItemBase* pItem, Operation::euiOperation operation, const IProvideDataToOperand& ipdtoDataToOperand, ILogger& logger)
    {
        ItemState worstState = NotApplicable;
        const HelperItems* pHelperItems = dynamic_cast<const HelperItems*>(pItem);
        if (!pHelperItems)
            return worstState;
        ItemState current;
        for (int i = 0; i < pHelperItems->GetSize(); ++i)
        {
            current = CalculateJustItemState(pHelperItems->GetValueAt(i), operation, ipdtoDataToOperand, logger);
            // If there is a helper item with this state (not avaiable and no URL authored)
            // that helper state should be ignored.
            if (current == Error)
            {
                current = NotApplicable;
                LOG(logger, ILogger::Warning, L"Ignoring the unavailable helper item.");
            }
            worstState = GetWorstOfStates(current, worstState);
        }
        return worstState;
    }

    // Given an Exe item, this method will calculate combined (worst state) of Exe, overrides combined.
    static ItemState CalculateExeItemStateEx
                           ( const ItemBase* pItem
                           , Operation::euiOperation operation
                           , const IProvideDataToOperand& dataToOperand
                           , ILogger& logger
                           , bool bRestrictToParentItemState = false)
    {
        ItemState worstState = NotApplicable;
        const IronMan::Exe* pExe = dynamic_cast<const IronMan::Exe*>(pItem);

        worstState = CalculateJustItemState(pItem, operation, dataToOperand, logger);

        // Helper is ExeBase but not Exe, though both of them have ItemType as Exe.
        // So, in this case, pItem is ExeBase - just return its own independent state
        // We look at only Exe state if we have to restrict to parent item
        if (!pExe || bRestrictToParentItemState)
            return worstState; 

        if (worstState == NotApplicable)
            return worstState;
        
        ActionTable::Actions installAction = ItemStateGenerator::GetItemAction(pItem, operation, dataToOperand);

        const ExeBase *pUninstallOverride = NULL, *pRepairOverride = NULL;

        pUninstallOverride = pExe->GetUninstallOverride();
        pRepairOverride    = pExe->GetRepairOverride();

        ItemState uninstallOverrideState = NotApplicable;
        if ( NULL != pUninstallOverride && 
            (ActionTable::Uninstall == installAction || ActionTable::Install == installAction))
        {
            uninstallOverrideState = CalculateJustItemState(pUninstallOverride, operation, dataToOperand, logger);
            if (ActionTable::Install == installAction)
            {
                // During install, we need to be concerned with Exe item and UninstallOverride item, if it exists.
                worstState = GetWorstOfStates(uninstallOverrideState , worstState);
            }
            else
                worstState = uninstallOverrideState;
        }

        ItemState repairOverrideState = NotApplicable;
        if (ActionTable::Repair == installAction && NULL != pRepairOverride)
        {
            repairOverrideState    = CalculateJustItemState(pRepairOverride, operation, dataToOperand, logger);
            // During repair operation, return only Repair override state if it exists
            worstState = repairOverrideState;
        }

        if (operation == Operation::uioCreateLayout)
        {
            // Caclculate uninstall override state
            worstState = GetWorstOfStates(uninstallOverrideState, worstState);

            // Caclculate repair override state
            worstState = GetWorstOfStates(repairOverrideState , worstState);

        }
        return worstState;
    }
    // Given an Patches item, this method will calculate combined (worst state) of all msps.
    static ItemState CalculatePatchesItemStateEx(const ItemBase* pItem, Operation::euiOperation operation, const IProvideDataToOperand& ipdtoDataToOperand, ILogger& logger, bool bRestrictToParentItemState = false)
    {
        ItemState worstState = NotApplicable;

        // Patches are a special case: loop over all child nodes and return a single uber-state
        const IronMan::Patches* patches = static_cast<const IronMan::Patches*>(pItem);
        for (unsigned int i=0; i<patches->GetCount(); ++i)
        {
            ItemState current = CalculateState(&patches->GetMsp(i), operation,  ipdtoDataToOperand, logger, bRestrictToParentItemState);
            worstState = GetWorstOfStates(current, worstState);
        }
        return worstState;
    }

    // Given an agile msi item, this method will calculate combined (worst state) of all msps and itself together.
    static ItemState CalculateAgileItemStateEx(const ItemBase* pItem, Operation::euiOperation operation, const IProvideDataToOperand& ipdtoDataToOperand, ILogger& logger, bool bRestrictToParentItemState = false)
    {
            const IronMan::AgileMSI* agileMSI = dynamic_cast<const IronMan::AgileMSI*>(pItem);

            ItemState worstState = NotApplicable;
            ItemState current;

            if (!bRestrictToParentItemState)
            {
                for (unsigned int i=0; i<agileMSI->GetCount(); ++i)
                {
                    current = CalculateState(&agileMSI->GetAgileMsp(i), operation,  ipdtoDataToOperand, logger, bRestrictToParentItemState);
                    worstState = GetWorstOfStates(current, worstState);
                }
            }

            // Calculate state of MSI itself
            current = CalculateJustItemState(pItem, operation, ipdtoDataToOperand, logger);
            worstState = GetWorstOfStates(current, worstState);

            return worstState;
    }

     /* //////////////////////////////////////////////////////////////////////////////
     LocalPathExists
     Receives:  ItemBase* -                 Item to check
                bPerformFileVerificaiton    When bPerformFileVerificaiton is false, 
                                            and the item is set into AvailableUnVerified.
                                            When this falg is true, verification is performed and
                                            Available state is returned.
                bRestrictToParentItemState  When true, only items state is calculated without considering states of
                                            child items (AgileMsps, Helper items etc)
     Returns :  bool -                      ItemState
     Purpose :  Returns state of the pItem, based on its availability and verificaiton flag.
    ////////////////////////////////////////////////////////////////////////////////// */
    static ItemState CalculateState(const ItemBase* pItem, Operation::euiOperation operation, const IProvideDataToOperand& dataToOperand, ILogger& logger, bool bRestrictToParentItemState = false)
    {
        /* figure out state, in this order

        1. not applicable
        2. available
        3. available un verified
        4. has url (not available)
        5. error
        */

        ItemState worstState = NotApplicable;

        if (pItem->GetType() == ItemBase::Patches)
        { 
            worstState = CalculatePatchesItemStateEx(pItem, operation, dataToOperand, logger);
        }
        else if (pItem->GetType() == ItemBase::AgileMsi)
        {
            worstState = CalculateAgileItemStateEx(pItem, operation, dataToOperand, logger, bRestrictToParentItemState);
        }
        else if (pItem->GetType() == ItemBase::Exe)
        {
            worstState = CalculateExeItemStateEx(pItem, operation, dataToOperand, logger, bRestrictToParentItemState);
        }
        else
        {
            worstState = CalculateJustItemState(pItem, operation, dataToOperand, logger);
        }

        // Helpers state needs to be considered only when just item state has not been requested AND
        // item istelf is applicable.
        if (!bRestrictToParentItemState && worstState != NotApplicable)
        {
            // update worstState to reflect helper items states too
            ItemState current = CalculateHelpersState(pItem, operation, dataToOperand, logger);
            worstState = GetWorstOfStates(current, worstState);
        }
        return worstState;
    }

private:
    virtual ItemBase* Clone() const { return m_item->Clone(); } // hmmm.
};

typedef ItemStateDecoratorT<FileAuthenticity> ItemStateDecorator;

template<typename FileAuthenticity>
class CoordinatorT : public ICoordinator
{
    Items m_unDecoratedItems;
    Items m_items;
    CRITICAL_SECTION m_cs;

    class DownloadItemsView : public IDownloadItems
    {
        Items& m_items;
        const IProvideDataToOperand& m_dataToOperand;

        enum ExeChildIndex
        {
            ExeUninstallOverrideIndex = 0,
            ExeRepairOverrideIndex = 1
        };

        struct IndexPlusParentItemInfo
        {
            enum Special
            {
                IsPatchesElement  = 0,
                IsAgileMSIElement = 1,
                IsExeElement      = 2,
                Neither           = 3
            } type;
            unsigned int indexIntoItems;            // accessor to get item from decorated m_items.
            unsigned int indexIntoChildItem;        // index to Agile MSPs, Patches MSPs or Exe Overrides
            CString indexIntoChildItemsByName;      // Helper name
            ItemStateDecorator::ItemState subState; // State of each of the 
            bool bDownloadFailureIgnored;           // Set when an item's (child item's) download is 
                                                    // failed AND the failure is ignorable.
            bool bIsParentItem;                     // Set for all items that are not child items 
                                                    // Eg. Agile MSPs, MSPs under patches, RetryHelper, 
                                                    // RepairOverride or uninstallOverride
        };
        CSimpleMap<unsigned int, IndexPlusParentItemInfo> m_mapping;

    public:
        DownloadItemsView(Items& items, const IProvideDataToOperand& dataToOperand)
            : m_items(items)
            , m_dataToOperand(dataToOperand)
        {}

         /* ///////////////////////////////////////////////////////////////////////////////////////////
         CreateMapping
         Receives: Operation
            One of Install, Uninstall, Repair and CreateLayout 
         Purpose : Creates mapping between 
                        items (IPPI.IndexIntoItems)and 
                        download items (ItemCount).
                   Note that IPPI.IndexIntoPatch can be used to retrieve the sub item.
         The following is sample mapping table that should help understanding it. 


         :::::Exe Item Details::::::
         One exe item (with two helpers, HelperA.Exe, HelperB.Exe, 0.cab and 1.cab)
         0.cab, 1.cab, required by Exe 

         :::::AgileMsi Item Details::::::
         AgileMsi item (with two helpers, HelpersA.Exe and HelperB.exe) and 3 Msps.
         Of this 3 msps, Patch1.msp is present but Patch2 and 3.msp are not.

         This results in three items to install in InstallItemsView.
         This also results in 10 items to download, in DownloadItemsView.
            3 exe items and  (.Exe, HelperA.Exe, HelperB.Exe, 0.cab, 1.cab)
            4 agile items (HelloWorld.msi, Patch2.msp, Patch3.msp, 2.cab and HelperC.Exe
         (This count ignores Exe item 6).
        -------------------------------------------------------------------------------------------------------------------------------------------
        ItemCount   IPPI.Type   IPPI.IndexIntoItems       indexIntoChildItem   indexIntoChildItemsByName    Comments                         IndexOfPayload
        -------------------------------------------------------------------------------------------------------------------------------------------
          0         Exe            0                           -1                                             Exe
          1         Exe            0                           -1               HelperA.Exe
          2         Exe            0                           -1               HelperB.Exe
          3         File           0                                            0.cab
          4         File           0                                            1.cab
          5         Agile          5                           -1               HelloWorld.msi
          6         Agile          5                            1               Patch2.msp
          7         Agile          5                            2               Patch3.msp
          8         Agile          5                            X               HelperA.Exe                    This is marked as Available**
          9         Agile          5                           -1
         10         File           5                                            1.cab
         11         File           5                                            2.cab
         12         Exe            6                           -1                                              Exe
         13         Exe            6                            0               ExeUninstallOverride
         14         Exe            6                            1               ExeRepairOverride
        -------------------------------------------------------------------------------------------------------------------------------------------
       
        ** Marked Available because by the time Agile item's HelperA.Exe is being operated by 
           download performer, it is already downloaded/verified and hence is Available.
           All we need to do is call UpdateItemState() on this HelperA.Exe item.

        ////////////////////////////////////////////////////////////////////////////////////////////// */
        void CreateMapping(Operation::euiOperation operation)
        {
            m_mapping.RemoveAll();

            unsigned int count = 0;
            CSimpleArray<CString> childItemsCache;
            for(unsigned int i=0; i<m_items.GetCount(); ++i)
            {
                IndexPlusParentItemInfo ippi;
                ippi.subState = ItemStateDecorator::NotAvailable;

                const ItemStateDecorator* isd = dynamic_cast<const ItemStateDecorator*>(m_items.GetItem(i));
                ippi.subState = isd->GetState();
                // We only add items that are NotAvailable to the download items list
                if (!CanAddItemToMap(ippi.subState))
                    continue;

                ippi.indexIntoItems = i;
                ippi.indexIntoChildItem = -1; // uninitialized
                ippi.type = IndexPlusParentItemInfo::Neither;
                ippi.bDownloadFailureIgnored = false;
                ippi.bIsParentItem = true;

                switch(isd->GetType())
                {
                    case ItemBase::Patches:
                    {
                        ippi.type = IndexPlusParentItemInfo::IsPatchesElement;
                        const Patches& patches = dynamic_cast<const Patches&>(isd->GetItem());
                        for (unsigned int j=0; j<patches.GetCount(); ++j)
                        {
                            ItemStateDecorator::ItemState currentState = ItemStateDecorator(&patches.GetMsp(j), operation, m_dataToOperand).GetState();
                            if (CanAddItemToMap(currentState))
                            {
                                ippi.subState = currentState;
                                ippi.indexIntoChildItem = j;
                                m_mapping.Add(count++, ippi);
                            }
                        }
                        break;
                    }
                    case ItemBase::AgileMsi:
                    {
                        // AgileMsi item state as of now is the combined state of the msi and its msps.
                        // i.e, the worst of all the msps and msi combined.
                        // We need to calculate msi state by iteself.

                        //const ItemStateDecorator* decoratedItem = dynamic_cast<const ItemStateDecorator*>(m_items.GetItem(i));
                        //const ItemBase& pUnDecoratedItem = decoratedItem->GetItem();
                        ippi.subState = ItemStateDecorator(&isd->GetItem(), operation, m_dataToOperand, true).GetState(); 

                        ippi.type = IndexPlusParentItemInfo::IsAgileMSIElement;
                        // Add the agile msi to the map, with its individual state and then add the msp items
                        m_mapping.Add(count++, ippi);
                        const AgileMSI& agileMsi = dynamic_cast<const AgileMSI&>(isd->GetItem());
                        for (unsigned int j=0; j<agileMsi.GetCount(); ++j)
                        {
                            ippi.bIsParentItem = false;
                            ItemStateDecorator::ItemState currentState = ItemStateDecorator(&agileMsi.GetAgileMsp(j), operation, m_dataToOperand).GetState();
                            if (CanAddItemToMap(currentState))
                            {
                                ippi.subState = currentState;
                                ippi.indexIntoChildItem = j;
                                m_mapping.Add(count++, ippi);
                            }
                        }
                        break;
                    }
                    case ItemBase::Exe:
                    {
                        ippi.type = IndexPlusParentItemInfo::IsExeElement;
                        ippi.subState = ItemStateDecorator(&isd->GetItem(), operation, m_dataToOperand, true).GetState();
                        const IronMan::Exe& exe = dynamic_cast<const IronMan::Exe&>(isd->GetItem());
                        
                        // For Installing or Createlayout, we need Exe and UninstallOverride, if it is authored.
                        if (Operation::uioInstalling == operation || Operation::uioCreateLayout == operation)
                        {
                            if (CanAddItemToMap(ippi.subState))
                                m_mapping.Add(count++, ippi);

                            const IronMan::ExeBase* pUninstallOverride = exe.GetUninstallOverride();
                            if (pUninstallOverride)
                            {
                                ItemStateDecorator::ItemState overrideState = ItemStateDecorator(pUninstallOverride, operation, m_dataToOperand, true).GetState();
                                if (CanAddItemToMap(overrideState))
                                {
                                    ippi.subState = overrideState;
                                    ippi.indexIntoChildItem = ExeUninstallOverrideIndex;
                                    ippi.bIsParentItem = false;
                                    m_mapping.Add(count++, ippi);
                                }
                            }
                            // For Createlayout, we also need to check to see if RepairOverride needs to be considered.
                            if (Operation::uioCreateLayout == operation)
                            {
                                const IronMan::ExeBase* pRepairOverride = exe.GetRepairOverride();
                                if (pRepairOverride)
                                {
                                    ippi.subState = ItemStateDecorator(pRepairOverride, operation, m_dataToOperand, true).GetState();
                                    ippi.indexIntoChildItem = ExeRepairOverrideIndex;
                                    ippi.bIsParentItem = false;
                                    m_mapping.Add(count++, ippi);
                                    if (CanAddItemToMap(ippi.subState))
                                        m_mapping.Add(count++, ippi);
                                }
                            }
                        }
                        // For Repairing, we need Exe OR RepairOverride, if it is authored.
                        if (Operation::uioRepairing == operation)
                        {
                            const IronMan::ExeBase* pRepairOverride = exe.GetRepairOverride();
                            if (pRepairOverride)
                            {
                                ippi.subState = ItemStateDecorator(pRepairOverride, operation, m_dataToOperand, true).GetState();
                                ippi.indexIntoChildItem = ExeRepairOverrideIndex;
                                ippi.bIsParentItem = false;
                            }
                            if (CanAddItemToMap(ippi.subState))
                                m_mapping.Add(count++, ippi);
                        }
                        // For Uninstalling, we need Exe or UninstallOverride, if it is authored.
                        if (Operation::uioUninstalling == operation)
                        {
                            const IronMan::ExeBase* pUninstallOverride = exe.GetUninstallOverride();
                            if (pUninstallOverride)
                            {
                                ippi.subState = ItemStateDecorator(pUninstallOverride, operation, m_dataToOperand, true).GetState();
                                ippi.indexIntoChildItem = ExeUninstallOverrideIndex;
                                ippi.bIsParentItem = false;
                            }
                            m_mapping.Add(count++, ippi);
                        }
                        break;
                    }
                    default:
                    {
                        // Get the state of just item itself
                        ippi.subState = ItemStateDecorator(&isd->GetItem(), operation, m_dataToOperand, true).GetState(); 
                        m_mapping.Add(count++, ippi);
                        break;
                    }
                }

                // Now add the Helpers
                // Only if they don't exist in the helper cache.
                const HelperItems* helperItems = dynamic_cast<const HelperItems*>(&isd->GetItem());
                if (helperItems)
                {
                    ippi.indexIntoChildItem = -1; // Reset the index as now we go on by the name of the exe as index
                    int countHelperItems = helperItems->GetSize();
                    for (int helperIndex = 0; helperIndex < countHelperItems; ++ helperIndex)
                    {
                        ippi.bIsParentItem = false;
                        const LocalPath* lp = dynamic_cast<const LocalPath *>(helperItems->GetValueAt(helperIndex));
                        if (!lp)
                        {
                            IMASSERT(0 && L"Helpers should derive from LocalPath");
                            continue;
                        }
                        const CPath& helperPath = lp->GetName();
                        CPath helperNamePath(helperPath);
                        helperNamePath.StripPath();
                        CString helperName(helperNamePath);
                        ItemStateDecorator::ItemState currentState = ItemStateDecorator(helperItems->GetValueAt(helperIndex), operation, m_dataToOperand, true).GetState();
                        if (CanAddItemToMap(currentState))
                        {
                            if (-1 == childItemsCache.Find(helperName))
                            {
                                ippi.subState = currentState;
                                ippi.indexIntoChildItemsByName = helperName;
                                m_mapping.Add(count++, ippi);
                                childItemsCache.Add(helperName);
                            }
                            else
                            {
                                ippi.subState = ItemStateDecorator::Available;
                                ippi.indexIntoChildItemsByName = helperName;
                                m_mapping.Add(count++, ippi);
                            }
                        }
                    }
                }

                // Add File elements of this item
                const CSimpleArray<CString>& fileIds = isd->GetItem().GetReferencedFiles();
                for (int j = 0; j < fileIds.GetSize(); j++)
                {
                    CString fileId = fileIds[j];
                    if (fileId.IsEmpty())
                    {
                        continue;
                    }

                    const ItemBase* fileItem = m_items.GetItem(fileId);
                    if (!fileItem)
                    {
                        IMASSERT(0 && L"File child item is missing.");
                        continue;
                    }

                    //ItemStateDecorator::ItemState currentState = ItemStateDecorator(fileItem, operation, m_dataToOperand, true).GetState();
                    const ItemStateDecorator* pFileISD = dynamic_cast<const ItemStateDecorator*>(fileItem);
                    if (CanAddItemToMap(pFileISD->GetState()))
                    {
                        if (-1 == childItemsCache.Find(fileId))
                        {
                            ippi.subState = pFileISD->GetState();
                            ippi.indexIntoChildItemsByName = fileId;
                            childItemsCache.Add(fileId);
                        }
                        else
                        {
                            ippi.subState = ItemStateDecorator::Available;
                            ippi.indexIntoChildItemsByName = fileId;
                        }

                        m_mapping.Add(count++, ippi);
                    }

                }
            }
        }

        void SetSubStateOfPackageItems(unsigned int nIndex, ItemStateDecorator::ItemState subState)
        {   // find all items in Map that match Patches index (ippi.indexIntoItems)
            unsigned int packageIndex =MapIndex(nIndex).indexIntoItems;
            for(int i=0; i<m_mapping.GetSize(); ++i)
            {
                IndexPlusParentItemInfo& ippi = m_mapping.GetValueAt(i);
                if (ippi.indexIntoItems == packageIndex)
                {
                    ippi.subState = subState;
                }
            }
        }

    private: // IDownloadItems
        virtual unsigned int GetCount() const
        {
            return m_mapping.GetSize();
        }

        virtual const ItemBase* GetItem(unsigned int nIndex) const
        {
            IndexPlusParentItemInfo ippi = MapIndex(nIndex);
            ItemStateDecorator* isd = static_cast<ItemStateDecorator *>(m_items.GetItem(ippi.indexIntoItems));
            const ItemBase* pItem = &isd->GetItem();

            // IMASSERT((dynamic_cast<const Patches*>(pItem) == NULL) == (-1 == ippi.indexIntoChildItem));
            if (ippi.type == IndexPlusParentItemInfo::IsPatchesElement)
            {
                const Patches& patches = dynamic_cast<const Patches&>(*pItem);
                if (ippi.indexIntoChildItem != (unsigned int)-1)
                {
                    // indexIntoChildItem points to an MSP
                    pItem = &patches.GetMsp(ippi.indexIntoChildItem);
                }
                // else
                    // indexIntoChildItem points to helper, which gets returned at the end.
            }
            else if (ippi.type == IndexPlusParentItemInfo::IsAgileMSIElement)
            {
                const AgileMSI& agileMsi = dynamic_cast<const AgileMSI&>(*pItem);
                if (ippi.indexIntoChildItem != (unsigned int)-1)
                {
                    // indexIntoChildItem points to an Agile MSP
                    pItem = &agileMsi.GetAgileMsp(ippi.indexIntoChildItem);
                }
                // else
                    // indexIntoChildItem points to helper, which gets returned at the end.
            }
            else if (ippi.type == IndexPlusParentItemInfo::IsExeElement)
            {
                const Exe& exe = dynamic_cast<const Exe&>(*pItem);
                if (ippi.indexIntoChildItem == ExeUninstallOverrideIndex)
                    pItem = exe.GetUninstallOverride();
                if (ippi.indexIntoChildItem == ExeRepairOverrideIndex)
                    pItem = exe.GetRepairOverride();
            }

            // Item to get is not parent item, not child item (msp, agile msp) and not an Overried item
            // Verify if it is a helper item and return it.
            if (!ippi.indexIntoChildItemsByName.IsEmpty())
            {
                // use current item to get the file item
                const ItemBase* pFileItem = m_items.GetItem(ippi.indexIntoChildItemsByName);
                const ItemStateDecorator* pFileISD = dynamic_cast<const ItemStateDecorator*>(pFileItem);
                if (pFileISD)
                {
                    pItem = &pFileISD->GetItem();
                }
                else
                {
                    // use current item to get the helper, by name.
                    const HelperItems* pHelpers = dynamic_cast<const HelperItems*>(pItem);
                    bool bFoundHelper = pHelpers->GetHelper(ippi.indexIntoChildItemsByName, &pItem);
                    if (!bFoundHelper)
                        throw E_FAIL; // Might consider adding custom exception or assertion here since it should never happen
                }
            }

            return pItem;
        }

        // We have two types of items added to the download list.
        // Not Available and AvailableUnVerfied.
        // There are two types of items by nature - 
        //      Simple  (MSI, Exe, MSP)
        //      Group   (Patches AgileMSI)
        // Simple Availability
        //      The state of the item tells that.
        // Group
        //      Individual items can be of one state and Group another.
        //      UpdateItemState() is storing the individual item state in ippi.subState.
        //      When all the subStates are avaialbe, this download item should be marked 
        //      as Available.

        // Returns download related attributes for given itemIndex (item or child item).
        // For Compressed payload, compressed size and hash are returned.
        virtual void GetItem(unsigned int nIndex
                            , CUrl* pUrl
                            , CString &csHash
                            , CPath &pthPath
                            , ULONGLONG &ulItemSize
                            , bool &bIgnoreDownloadFailure
                            , CString& csItemName
                            , CString& csItemId) const
        {
            const ItemBase* pItem = GetItem(nIndex);
            
            csItemId = pItem->GetId();

            // we've got the guy, now get URL and local path out of him
            const DownloadPath* downloadable = dynamic_cast<const DownloadPath*>(pItem);
            if (downloadable)
            {
                pUrl->CrackUrl(downloadable->GetUrl());
                if (IsItemCompressed(nIndex))
                {
                    ulItemSize = downloadable->GetCompressedItemSize(); // Get download size of item
                    csHash = downloadable->GetCompressedHashValue();
                }
                else
                {
                    ulItemSize = downloadable->GetItemSize(); // Get download size of item
                    csHash = downloadable->GetHashValue();
                }
            }

            const LocalPath* path         = dynamic_cast<const LocalPath   *>(pItem);
            if (path)
            {
                CPath pthLocalPath(path->GetName());
                IMASSERT(!pthLocalPath.IsRelative());
                pthPath = pthLocalPath;
                csItemName = CString(path->GetOriginalName());
            }

            bIgnoreDownloadFailure = false;
            const IgnoreDownloadFailure* ignoreDownloadFailure = dynamic_cast<const IgnoreDownloadFailure*>(pItem);
            if (ignoreDownloadFailure != NULL && ignoreDownloadFailure->ShouldIgnoreDownloadFailure())
            {
                bIgnoreDownloadFailure = true;
            }
        }

        virtual void SetItemStateAsAvailable(unsigned int nIndex)
        {
            IndexPlusParentItemInfo& ippi = MapIndex(nIndex);
            ItemStateDecorator* isd = dynamic_cast<ItemStateDecorator *>(m_items.GetItem(ippi.indexIntoItems));
            isd->UpdateState(ItemStateDecorator::Available);
        }

        virtual void UpdateItemPath(unsigned int nIndex, const CPath& itemPath)
        {
            IndexPlusParentItemInfo& ippi = MapIndex(nIndex);
            ItemStateDecorator* isd = dynamic_cast<ItemStateDecorator *>(m_items.GetItem(ippi.indexIntoItems));
            const LocalPath& lp = dynamic_cast<const LocalPath&>(isd->GetItem());
            lp.UpdateName(itemPath);
            isd->UpdateState(ItemStateDecorator::AvailableUnVerified);
        }

        /*  Inputs:  nIndex                  - Index into Download map
                     bDownloadFailureIgnored - Set to true when an item's download failure can be ignored and 
            Returns: void

            This method changes the state of the item to Available.
            The item will be in AvailableUnVerified if it was locally available file.
            This item will be verified by VerifyItem()and Updated.

            When bDownloadFailureIgnored is true, if parent item failed to download,it is 
            remembered. The availability of child items is not considered when declaring 
            that item (all items except Patches)is NotAvailableAndIgnorable. For Patches case,
            some of the MSPs can be available and some not and we still need to install Patches item.
        */
        virtual void UpdateItemState(unsigned int nIndex, bool bDownloadFailureIgnored = false)
        {
            IndexPlusParentItemInfo& ippi = MapIndex(nIndex);
            ItemStateDecorator* isd = dynamic_cast<ItemStateDecorator *>(m_items.GetItem(ippi.indexIntoItems));

            if (bDownloadFailureIgnored && ippi.bIsParentItem)
            {
                // Failure is ignored, 
                ippi.bDownloadFailureIgnored = true;
                ippi.subState = ItemStateDecorator::NotAvailableAndIgnorable;
            }
            else // meaning, for msps of Patches element, we set them available though they were ignored.
                ippi.subState = ItemStateDecorator::Available;
            if (true == AreAllSubStatesAvailable(nIndex))
            {
                if (IsParentItemNotAvailableAndIgnorable(nIndex))
                    isd->UpdateState(ItemStateDecorator::NotAvailableAndIgnorable);
                else
                    isd->UpdateState(ItemStateDecorator::Available);
            }
        }

        // True if item is in this state; This method operates on individual item.
        virtual bool IsItemAvailableUnVerified(unsigned int nIndex) const
        {
            const IndexPlusParentItemInfo& ippi = MapIndex(nIndex);
            return (ippi.subState == ItemStateDecorator::AvailableUnVerified);
        }
        virtual bool IsItemOnDownloadList(unsigned int nIndex) const
        {
            const IndexPlusParentItemInfo& ippi = MapIndex(nIndex);
            return (ippi.subState == ItemStateDecorator::NotAvailable);
        }

        //
        // methods needed by the cache manager
        //

        virtual CString GetItemName(unsigned int nIndex) const
        {
            CUrl src;
            CString hash;
            CPath dstPath;
            ULONGLONG itemSize = 0;
            bool bIgnoreDownloadFailure = false;
            CString itemName;
            CString itemId;
            this->GetItem(nIndex, &src, hash, dstPath, itemSize, bIgnoreDownloadFailure, itemName, itemId);
            return dstPath;
        }

        virtual bool DoesItemNeedToBeCached(unsigned int nIndex, CPath& pathCachedFileName, bool& bPerMachine) const
        {
            const LocalPath* lp = dynamic_cast<const LocalPath*>(GetItem(nIndex));
            return lp->DoesItemNeedToBeCached(pathCachedFileName, bPerMachine);
        }

        virtual unsigned int GetChildItemCount(unsigned int nParentIndex) const
        {
            const IndexPlusParentItemInfo& ippi = MapIndex(nParentIndex);
            const ItemStateDecorator* isd = dynamic_cast<ItemStateDecorator*>(m_items.GetItem(ippi.indexIntoItems));
            const ItemBase& realParentItem = isd->GetItem();
            return realParentItem.GetReferencedFiles().GetSize();
        }

        virtual const ItemBase* GetChildItem(unsigned int nParentIndex, unsigned int nChildIndex) const
        {
            const IndexPlusParentItemInfo& ippi = MapIndex(nParentIndex);
            const ItemStateDecorator* isd = dynamic_cast<ItemStateDecorator*>(m_items.GetItem(ippi.indexIntoItems));
            const ItemBase& realParentItem = isd->GetItem();
            CString csChildId(realParentItem.GetReferencedFiles()[nChildIndex]);
            return m_items.GetItem(csChildId);
        }

        //
        // methods needed to handle unavailable items with URL, for which UX want's us instead to ask it to resolve source for.
        //

        // True if item is in Available or NotAvailableAndIgnorable; This method operates on individual item.
        // From download point of view, both states mean download part is done.
        // From Install point of view (via IInstallItems), NotAvailableAndIgnorable item is just ignored/skipped
        // for install operation.
        virtual bool IsItemAvailable(unsigned int nIndex) const
        {
            const IndexPlusParentItemInfo& ippi = MapIndex(nIndex);
            return ( ippi.subState == ItemStateDecorator::Available 
                  || ippi.subState == ItemStateDecorator::NotAvailableAndIgnorable);
        }

        // Returns true of current item/child item is compressed.
        virtual bool IsItemCompressed(unsigned int nIndex) const
        {
            const LocalPath* lp = dynamic_cast<const LocalPath*>(GetItem(nIndex));
            if (lp)
            {
                return lp->IsItemCompressed();
            }
            return false;
        }

        // This method operates on individual item.
        // Returns true if the item is checks out FileAuthenticity.
        // The check is performed on single item. If the index points to  to AgileMsi or Patches,
        // we get the particular individual item and verify it.
        virtual bool VerifyItem(unsigned int nIndex, ILogger& logger)
        {
            const ItemBase* pItem = GetItem(nIndex);
            // With this call, we actully perform file validation by setting bPerformFileVerificaiton to true
            return ItemStateGeneratorT<FileAuthenticity>::VerifyItem(pItem, logger);
        }

        // Download space in BYTES required by the download items
        virtual ULONGLONG GetDownloadDiskSpaceRequirement() const
        {
            ULONGLONG totalBytes = 0;
            for ( unsigned int nIndex = 0; nIndex < GetCount(); ++nIndex)
            {
                // Only Not Available items should participate in this calculations.
                if (IsItemAvailableUnVerified(nIndex))
                    continue;
                CUrl url;
                CString hash;
                CPath path;
                ULONGLONG itemSize = 0;
                bool bIgnoreDownloadFailure = false;
                CString itemName;
                CString itemId;
                GetItem(nIndex, &url, hash, path, itemSize, bIgnoreDownloadFailure, itemName, itemId);
                if (IsItemCompressed(nIndex))
                {
                    // we got compressed size already. Now get uncompressed size
                    const ItemBase* pItem = GetItem(nIndex);
                    // we've got the guy, now get URL and local path out of him
                    const DownloadPath* downloadable = dynamic_cast<const DownloadPath*>(pItem);
                    if (downloadable)
                    {
                        itemSize += downloadable->GetItemSize(); // Get download size of the uncompressed item
                    }
                }
                // Get Disk Space required for all items that will be downloaded
                totalBytes += itemSize;
            }
            return totalBytes; // return the number of items to download
        }
    private:

        // Returns true if item needs to be added to the download map.
        bool CanAddItemToMap(ItemStateDecorator::ItemState itemState)
        {
            if ((ItemStateDecoratorT<FileAuthenticity>::NotAvailable        == itemState)|| 
                (ItemStateDecoratorT<FileAuthenticity>::AvailableUnVerified == itemState)||
                (ItemStateDecoratorT<FileAuthenticity>::NotAvailableAskUX   == itemState))
            {
                return true;
            }
            return false;
        }

        IndexPlusParentItemInfo& MapIndex(unsigned int nIndex) const
        {
            if (nIndex >= GetCount())
                throw E_FAIL; // Might consider adding custom exception or assertion here since it should never happen

            return m_mapping.GetValueAt(m_mapping.FindKey(nIndex));
        }
        bool AreAllSubStatesAvailable(unsigned int nIndex) const
        {   // find all items in Map that match Patches index (ippi.indexIntoItems)
            unsigned int patchesIndex = MapIndex(nIndex).indexIntoItems;
            for(int i=0; i<m_mapping.GetSize(); ++i)
            {
                const IndexPlusParentItemInfo& ippi = m_mapping.GetValueAt(i);
                if (ippi.indexIntoItems == patchesIndex)
                    if (ippi.subState != ItemStateDecorator::Available && ippi.subState != ItemStateDecorator::NotAvailableAndIgnorable && ippi.subState != ItemStateDecorator::AvailableUnVerified)
                        return false;
            }
            return true;
        }

        // Iterates thru the Map and finds the Parent item's  indexthat this nIndex's
        // indexIntoItems matches with and returns true if it is NotAvailableAndIgnorable
        bool IsParentItemNotAvailableAndIgnorable(unsigned int nIndex) const
        {
            unsigned int patchesIndex = MapIndex(nIndex).indexIntoItems;
            for(int i=0; i<m_mapping.GetSize(); ++i)
            {
                const IndexPlusParentItemInfo& ippi = m_mapping.GetValueAt(i);
                if (ippi.indexIntoItems == patchesIndex)
                    if (ippi.bIsParentItem && ippi.bDownloadFailureIgnored)
                        return true;
            }
            return false;
        }
    } *m_pDownloadItems;

    class InstallItemsView : public InstallItemsBase
    {
        Items& m_items;
        ILogger& m_logger;
        CSimpleMap<unsigned int, unsigned int> m_mapping; // mapping between installable items index and the one m_items array
        const IProvideDataToOperand& m_dataToOperand;
        const ItemArray& m_childFileItems;

        mutable CSimpleMap<unsigned int, Patches> m_PatchesCache; // special cache for Patches with some applicable and some non-applicable MSPs
        Operation::euiOperation m_operation;

    public:
        InstallItemsView(Items& items, const IProvideDataToOperand& dataToOperand, const ItemArray& childFileItems, ILogger& logger)
            : m_items(items)
            , m_logger(logger)
            , m_dataToOperand(dataToOperand)
            , m_childFileItems(childFileItems)
            , m_operation(Operation::uioInstalling) // as good a default as any
        {}
        void CreateMapping(Operation::euiOperation operation, bool bAddAllPackages = false)
        {
            m_mapping.RemoveAll();
            m_PatchesCache.RemoveAll();

            m_operation = operation;
            if (operation == Operation::uioCreateLayout)
                return; // nothing to do:  in CreateLayout mode, we don't install, we xcopy to the layout folder

            unsigned int count = 0;
            for(unsigned int i=0; i<m_items.GetCount(); ++i)
            {
                const ItemStateDecorator* isd = static_cast<const ItemStateDecorator*>(m_items.GetItem(i));
                if ((isd->GetState() == ItemStateDecorator::NotApplicable) && !bAddAllPackages)
                    continue;

                m_mapping.Add(count++, i);
            }
        }
    private: // IInstallItems
        virtual unsigned int GetCount() const
        {
            return m_mapping.GetSize();
        }
        virtual const ItemBase* GetItem(unsigned int nIndex) const
        {
            if (m_PatchesCache.FindKey(nIndex) != -1)
                return &m_PatchesCache.GetValueAt(m_PatchesCache.FindKey(nIndex));

            ItemStateDecorator* isd = static_cast<ItemStateDecorator*>(m_items.GetItem(MapIndex(nIndex)));
            const ItemBase* pItem = &isd->GetItem();
            if (pItem->GetType() != ItemBase::Patches)
                return pItem;

            // create a filtered Patches item (based on applicability) and cache it
            const Patches& patches = dynamic_cast<const Patches&>(isd->GetItem());
            const ActionTable& at = dynamic_cast<const ActionTable&>(isd->GetItem());
            const CustomErrorHandling& ceh = dynamic_cast<const CustomErrorHandling&>(isd->GetItem());
            const HelperItems& helpers = dynamic_cast<const HelperItems&>(isd->GetItem());

            Patches newPatches(patches.ShouldIgnoreDownloadFailure()
                                , patches.ShouldRollBack()
                                , m_logger
                                , at
                                , ceh
                                , helpers);

            for (unsigned int j=0; j<patches.GetCount(); ++j)
            {
                if (ItemStateDecorator::NotApplicable != ItemStateDecorator(&patches.GetMsp(j), m_operation, m_dataToOperand).GetState())
                {
                    newPatches.AddMsp(patches.GetMsp(j));
                }
            }
            IMASSERT(newPatches.GetCount() != 0);

            m_PatchesCache.Add(nIndex, newPatches);
            return &m_PatchesCache.GetValueAt(m_PatchesCache.FindKey(nIndex));
        }

        virtual CString GetItemID(unsigned int nIndex) const
        {
            const ItemStateDecorator* isd = static_cast<const ItemStateDecorator*>(m_items.GetItem(MapIndex(nIndex)));
            return isd->GetItem().GetId();
        }

        // This method queries the decorated item, its state
        virtual bool IsItemAvailable(unsigned int nIndex) const
        {
            const ItemStateDecorator* isd = static_cast<const ItemStateDecorator*>(m_items.GetItem(MapIndex(nIndex)));
            return isd->GetState() == ItemStateDecorator::Available;
        }

        virtual bool VerifyItem(unsigned int nIndex, ILogger& logger)
        {
            const ItemStateDecorator* isd = static_cast<const ItemStateDecorator*>(m_items.GetItem(MapIndex(nIndex)));
            return ItemStateGeneratorT<FileAuthenticity>::VerifyItem(&isd->GetItem(), logger);
        }

        virtual bool IsItemAvailableUnVerified(unsigned int nIndex) const
        {
            const ItemStateDecorator* isd = static_cast<const ItemStateDecorator*>(m_items.GetItem(MapIndex(nIndex)));
            return isd->GetState() == ItemStateDecorator::AvailableUnVerified;
        }

        virtual bool IsItemOnDownloadList(unsigned int nIndex) const
        {
            const ItemStateDecorator* isd = static_cast<const ItemStateDecorator*>(m_items.GetItem(MapIndex(nIndex)));
            return isd->GetState() == ItemStateDecorator::NotAvailable;
        }

        // This method queries the decorated item, its state
        virtual bool IsItemNotAvailableAndIgnorable(unsigned int nIndex) const
        {
            const ItemStateDecorator* isd = static_cast<const ItemStateDecorator*>(m_items.GetItem(MapIndex(nIndex)));
            return isd->GetState() == ItemStateDecorator::NotAvailableAndIgnorable;
        }

        virtual void SetItemStateAsAvailable(unsigned int nIndex)
        {
            ItemStateDecorator* isd = static_cast<ItemStateDecorator*>(m_items.GetItem(MapIndex(nIndex)));
            return isd->UpdateState(ItemStateDecorator::Available);
        }
        virtual void UpdateItemPath(unsigned int nIndex, const CPath& itemPath)
        {
            ItemStateDecorator* isd = static_cast<ItemStateDecorator*>(m_items.GetItem(MapIndex(nIndex)));
            const LocalPath& lp = dynamic_cast<const LocalPath&>(isd->GetItem());
            lp.UpdateName(itemPath);
            isd->UpdateState(ItemStateDecorator::AvailableUnVerified);
        }
        virtual void UpdateItemState(unsigned int nIndex)
        {
            ItemStateDecorator* isd = static_cast<ItemStateDecorator*>(m_items.GetItem(MapIndex(nIndex)));
            return isd->UpdateState(ItemStateDecorator::Complete);
        }

        // This method queries the decorated item, its state
        virtual bool IsItemComplete(unsigned int nIndex) const
        {
            const ItemStateDecorator* isd = static_cast<const ItemStateDecorator*>(m_items.GetItem(MapIndex(nIndex)));
            return isd->GetState() == IronMan::ItemStateDecorator::Complete;            
        }

        virtual bool DoesItemNeedToBeCached(unsigned int nIndex, CPath& pathCachedFileName, bool& bPerMachine) const
        {
            const ItemStateDecorator* isd = static_cast<const ItemStateDecorator*>(m_items.GetItem(MapIndex(nIndex)));
            const LocalPath& localPath = dynamic_cast<const LocalPath&>(isd->GetItem());
            return localPath.DoesItemNeedToBeCached(pathCachedFileName, bPerMachine);
        }

        virtual unsigned int GetChildItemCount(unsigned int nParentIndex) const
        {
            const ItemStateDecorator* isd = static_cast<const ItemStateDecorator*>(m_items.GetItem(MapIndex(nParentIndex)));
            const ItemBase& realParentItem = isd->GetItem();
            return realParentItem.GetReferencedFiles().GetSize();
        }

        virtual const ItemBase* GetChildItem(unsigned int nParentIndex, unsigned int nChildIndex) const
        {
            const ItemStateDecorator* isd = static_cast<const ItemStateDecorator*>(m_items.GetItem(MapIndex(nParentIndex)));
            const ItemBase& realParentItem = isd->GetItem();
            CString csChildId(realParentItem.GetReferencedFiles()[nChildIndex]);
            for ( unsigned int nItemIndex = 0; nItemIndex < m_childFileItems.GetSize(); ++nItemIndex)
            {
                if ( m_childFileItems[nItemIndex]->GetId() == csChildId )
                {
                    return m_childFileItems[nItemIndex];
                }
            }
            return NULL;
        }

        virtual void SetItemStateToIgnorable(unsigned int nIndex) const
        {
            ItemStateDecorator* isd = static_cast<ItemStateDecorator*>(m_items.GetItem(MapIndex(nIndex)));
            return isd->UpdateState(ItemStateDecorator::NotAvailableAndIgnorable);
        }

    private:
        unsigned int MapIndex(unsigned int nIndex) const
        {
            if (nIndex >= GetCount())
                throw E_FAIL; // Might consider adding custom exception or assertion here since it should never happen

            return m_mapping.GetValueAt(m_mapping.FindKey(nIndex));
        }
    } *m_pInstallItems;

private:
   // const Items& m_engineDataItems;
    Operation::euiOperation m_operation;
    const IProvideDataToOperand& m_dataToOperand;
    ILogger& m_logger;
    bool m_bInitialized;
    bool m_bDetectComplete;
    bool m_bRunningElevated;
    IBurnView *m_pBurnView;

public:
    // m_unDecoratedItems copies items so that m_items can be computed
    // with item decorated state, and download and install items and their mapping.
    CoordinatorT(
        __in const Items& items,
        __in Operation::euiOperation operation,
        __in const IProvideDataToOperand& dataToOperand,
        __in ILogger& logger
        )
        : m_bInitialized(false)
        , m_bDetectComplete(false)
        , m_bRunningElevated(false)
        , m_unDecoratedItems(items)
        , m_dataToOperand(dataToOperand)
        , m_items(items.GetRetries(), items.GetDelay(), items.GetCopyPackageFilesFlag(), logger)
        , m_operation(operation)
        , m_logger(logger)
        , m_pDownloadItems(NULL)
        , m_pInstallItems(NULL)
        , m_pBurnView(NULL)
    {
        InitializeCriticalSection(&m_cs);
    }
private:
    bool Initialize()
    {
        HRESULT hr = S_OK;
        EnterCriticalSection(&m_cs);
        if (!m_bInitialized)
        {
            if (m_bRunningElevated && m_bDetectComplete)
            {
                // In elevated process detect is enough. No need to compute download items view or execute searches
                m_pInstallItems  = new InstallItemsView(m_items, m_dataToOperand, m_unDecoratedItems.GetChildItems(), m_logger);
                m_pInstallItems->CreateMapping(m_operation, true);
            }
            else
            {
                // Unelevaed process code path
                if (vpEngineState)
                {
                    // execute searches
                    hr = SearchesExecute(&vpEngineState->searches, &vpEngineState->variables);
                    ExitOnFailure(hr, "Failed to execute searches.");
                }

                CString strOperation(Operation::GetOperationCanonicalString(m_operation));
                CString section(L" determination is complete");
                CString strApplicabilityForOperation(L"Applicability for " + strOperation);
                PUSHLOGSECTIONPOP(m_logger, strApplicabilityForOperation, L"evaluating each item", section);

                // convert engineData Items into decorated Items
                for (unsigned int i=0; i<m_unDecoratedItems.GetCount(); ++i)
                {
                    // Decorating items with State is lengthy operation
                    m_items.Add(new ItemStateDecorator(m_unDecoratedItems.GetItem(i), m_operation, m_dataToOperand, m_logger));
                }

                // Create download and install item views of decorated items
                m_pDownloadItems = new DownloadItemsView(m_items, m_dataToOperand);
                m_pInstallItems  = new InstallItemsView(m_items, m_dataToOperand, m_unDecoratedItems.GetChildItems(), m_logger);

                // Create mapping of items based on view.
                m_pDownloadItems->CreateMapping(m_operation);
                m_pInstallItems->CreateMapping(m_operation);
            }

            m_bInitialized = true;
        }
    LExit:
        LeaveCriticalSection(&m_cs);
        return m_bInitialized;
    }

    void CheckIfPackagesCanBeDownloaded(void)
    {
        // See if we have items to download
        bool bHaveItemsToDownload = false;
        for (unsigned int i=0; i<m_items.GetCount(); ++i)
        {
            const ItemStateDecorator* isd = static_cast<const ItemStateDecorator*>(m_items.GetItem(i));
            if (isd->GetState() == ItemStateDecorator::NotAvailable)
            {
                ACTION_STATE actionState = ACTION_STATE_NONE;
                isd->GetItem().GetPackageState(NULL, NULL, NULL, &actionState);
                if (actionState != ACTION_STATE_NONE && actionState != ACTION_STATE_UNINSTALL)
                {
                    bHaveItemsToDownload = true;
                    break;
                }
            }
        }

        // If we have items to download ask UX if we can download packages from the web.
        if (bHaveItemsToDownload)
        {
            if (m_pBurnView != NULL && m_pBurnView->CanPackagesBeDownloaded() == FALSE)
            {
                bHaveItemsToDownload = false;

                for(unsigned int i=0; i<m_items.GetCount(); ++i)
                {
                    ItemStateDecorator* isd = dynamic_cast<ItemStateDecorator*>(m_items.GetItem(i));
                    if (isd->GetState() == ItemStateDecorator::NotAvailable)
                    {
                        m_pDownloadItems->SetSubStateOfPackageItems(i, ItemStateDecorator::NotAvailableAskUX);
                        isd->UpdateState(ItemStateDecorator::NotAvailableAskUX);
                    }
                }
            }
        }
    }
public:
    virtual ~CoordinatorT() 
    {
        delete m_pDownloadItems;
        delete m_pInstallItems;
        DeleteCriticalSection(&m_cs);
    }

public:
    void SetBurnView(__in IBurnView *pBurnView)
    {
        m_pBurnView = pBurnView;
    }
    
#ifdef FeaturesAreImplented
    // Features feature not implemented
    void Refresh(const Items& items, Features& features, Operation::euiOperation operation, ILogger& logger)
    {   
        Initialize();
        // for each top-level feature in features, find the matching feature in m_items and update the state
        for(unsigned int i=0; i<features.GetNumberOfChildren(); ++i)
        {
            IFeature* pFeature = features.GetChild(i);
            CString textId = pFeature->GetTextId();

            for(unsigned int j=0; j<m_items.GetCount(); ++j)
            {
                ItemBase* pItem = m_items.GetItem(j);
                ItemStateDecorator* pItemState = static_cast<ItemStateDecorator*>(pItem);

                const Features* itemcFeatures = dynamic_cast<const Features*>(&pItemState->GetItem());
                if (itemcFeatures)
                {
                    Features* itemFeatures = const_cast<Features*>(itemcFeatures);
                    if (itemFeatures->GetNumberOfChildren() > 0)
                    {
                        IFeature* pF = itemFeatures->GetChild(0);
                        if (textId == pF->GetTextId())
                        {
                            pF->SetSelectionState(pFeature->GetDefaultSelectionState()); // update the feature state

                            // now, if the top-level feature guy's state is false, flip this guy into non-Applicable mode
                            if (pFeature->GetDefaultSelectionState() == false)
                            {
                                if ((pItemState->GetState() == ItemStateDecorator::Available) ||
                                    (pItemState->GetState() == ItemStateDecorator::NotAvailable))
                                {
                                    pItemState->UpdateState(ItemStateDecorator::NotApplicable);
                                }
                            }
                            break;
                        }
                    }
                }
            }
        }

        m_pDownloadItems->CreateMapping(operation);
        m_pInstallItems->CreateMapping(operation);
    }
#endif //FeaturesAreImplented

    IDownloadItems& GetDownloadItems() 
    {
        Initialize();
        return *m_pDownloadItems; 
    }
    IInstallItems & GetInstallItems () 
    {
        Initialize();
        return *m_pInstallItems; 
    }
    const Items   & GetStatefulItems() 
    {
        Initialize();
        return m_items; 
    }
    bool ValidateStatefulItems() const
    {
        // Coordinator must be initialized for validation to succeed.
        if (!m_bInitialized)
            return false;
        for (unsigned int i=0; i<m_items.GetCount(); ++i)
        {
            const ItemStateDecorator* isd = static_cast<const ItemStateDecorator*>(m_items.GetItem(i));
            if (isd->GetState() == ItemStateDecorator::Error)
                return false;
        }
        return true;
    }

    // Returns count of items authored
    virtual unsigned int GetAuthoredItemCount() const
    {
        return m_items.GetCount();
    }

    // Returns count of items authored of the given type.
    virtual unsigned int GetAuthoredItemCount(ItemBase::ItemType itemType) const
    {
        return m_items.GetCount(itemType);
    }

    // UX calls this method to detect all the applicable items (packages).
    virtual HRESULT Detect(__in IBurnView* pBurnView)
    {
        IMASSERT(m_bInitialized == false);
        HRESULT hr = S_OK;

        Trace(REPORT_STANDARD, "Coordinator::OnDetect() - enter");

        // Loop thru the items and check applicability
        if (!m_bDetectComplete )
        {
            // Searches should be done only in unelevatd process
            if (!m_bRunningElevated && vpEngineState)
            {
                // execute searches
                hr = SearchesExecute(&vpEngineState->searches, &vpEngineState->variables);
                ExitOnFailure(hr, "Failed to execute searches.");
            }

            CString strOperation(Operation::GetOperationCanonicalString(m_operation));
            CString section(L" Detect is complete");
            CString strApplicabilityForOperation(L"Applicability for " + strOperation);
            PUSHLOGSECTIONPOP(m_logger, strApplicabilityForOperation, L"evaluating each item", section);

            //// Check if this is an Update Bundle and if it is applicable
            //if ( !m_bRunningElevated  // only in unelevated process
            //    && vpEngineState 
            //    && 0 != vpEngineState->registration.cUpdateBundleIds  // Upgrade Bundle
            //    && 0 == vpEngineState->registration.cRegistrationKeys ) // Any Applicable Products
            //{
            //    // This is an Update bundle and there are no Product Bundles that this applies to
            //    m_bDetectComplete = true;
            //    LOG(m_logger, ILogger::Information, L"This is an Update Bundle and there are no Product Bundles that this applies to.");
            //    ExitOnWin32Error(ERROR_PATCH_TARGET_NOT_FOUND, hr, "This is an Update Bundle and there are no Product Bundles that this applies to.");
            //}

            //// Check if a newer versioned Bundle of the Same Family is already installed
            //if ( !m_bRunningElevated  // only in unelevated process
            //    && vpEngineState 
            //    && TRUE == vpEngineState->registration.fNewerVersionInstalled ) // Newer version installed
            //{
            //    // This is an Update bundle and there are no Product Bundles that this applies to
            //    m_bDetectComplete = true;
            //    LOG(m_logger, ILogger::Information, L"There is an newer version of the Product already installed on this machine.");
            //    ExitOnWin32Error(ERROR_PRODUCT_VERSION, hr, "There is an newer version of the Product already installed on this machine.");
            //}
            
            int nResult = OnDetectPriorBundle(pBurnView);
            hr = HRESULT_FROM_VIEW(nResult);
            ExitOnFailure(hr, "UX aborted detect prior bundle.");

            // convert engineData Items into decorated Items
            for (unsigned int i=0; i<m_unDecoratedItems.GetCount(); ++i)
            {
                // Decorating items with State is lengthy operation
                ItemStateDecorator* pDecoratedItem = new ItemStateDecorator(m_unDecoratedItems.GetItem(i), m_dataToOperand, m_logger);
                CString id = pDecoratedItem->GetItem().GetId();

                hr = pDecoratedItem->Detect();
                ExitOnFailure1(hr, "Detect failed on pakcage: %S", id);

                bool bApplicable = false;
                PACKAGE_STATE detectState = PACKAGE_STATE_UNKNOWN;

                pDecoratedItem->GetItem().GetPackageState(&bApplicable, NULL, NULL, NULL);

                if (bApplicable)
                {
                    int nResult = pBurnView->OnDetectPackageBegin(id);
                    hr = HRESULT_FROM_VIEW(nResult);
                    ExitOnRootFailure(hr, "UX aborted detect package begin.");

                    pDecoratedItem->GetItem().GetPackageState(NULL, &detectState, NULL, NULL);
                    pBurnView->OnDetectPackageComplete(id, hr, detectState);
                }

                LOGEX(m_logger, ILogger::Information, L"Package [%s] - Applicable: %d; Detect state:%d.", id, bApplicable, detectState);
                m_items.Add(pDecoratedItem);
            }

            // Process child elements
            for (unsigned int i=0; i<m_unDecoratedItems.GetChildItemCount(); ++i)
            {
                ItemStateDecorator* pDecoratedChildItem = new ItemStateDecorator(m_unDecoratedItems.GetChildItem(i), m_dataToOperand, m_logger);
                m_items.AddChildItem(pDecoratedChildItem);
            }

            m_bDetectComplete = true;
        }

    LExit:
        return hr;
    }

    HRESULT OnDetectPriorBundle(__in IBurnView* pBurnView)
    {
        HRESULT hr = S_OK;
        int nResult = IDOK;

        //for (DWORD i = 0; i < vpEngineState->registration.cRegdataFamily; ++i)
        //{
        //    nResult = pBurnView->OnDetectPriorBundle(vpEngineState->registration.rgRegdataFamily[i].sczBundleId);
        //    hr = HRESULT_FROM_VIEW(nResult);
        //    ExitOnFailure(hr, "UX aborted detect prior bundle.");
        //}

        //for (DWORD i = 0; i < vpEngineState->registration.cRegdataUpgrades; ++i)
        //{
        //    nResult = pBurnView->OnDetectPriorBundle(vpEngineState->registration.rgRegdataUpgrades[i].sczBundleId);
        //    hr = HRESULT_FROM_VIEW(nResult);
        //    ExitOnFailure(hr, "UX aborted detect prior bundle.");
        //}

    LExit:
        return hr;
    }


    // Plan
    virtual HRESULT Plan(__in BURN_ACTION action, __in IBurnView* pBurnView)
    {
        HRESULT hr = S_OK;
        BOOL fResult = FALSE;
        CSimpleArray<CString> fileIdCache;

        // Loop thru the items and Plan 
        CString strOperation(Operation::GetOperationCanonicalString(m_operation));
        CString section(L" Plan is complete");
        CString strPlanForOperation(L"Plan for " + strOperation);
        PUSHLOGSECTIONPOP(m_logger, strPlanForOperation, L"evaluating each item", section);

        REQUEST_STATE defaultRequestState = REQUEST_STATE_NONE;

        hr = OnPlanPriorBundle(pBurnView);
        ExitOnFailure(hr, "Ux aborted plan prior bundle");

        hr = GetRequestStateFromBurnAction(action, &defaultRequestState);
        ExitOnFailure1(hr, "Failed to get request state for BURN_ACTION: %d", action);


        // convert engineData Items into decorated Items
        for (unsigned int i=0; i<m_items.GetCount(); ++i)
        {
            // Decorating items with State is lengthy operation
            ItemStateDecorator* pDecoratedItem = dynamic_cast<ItemStateDecorator *>(m_items.GetItem(i));
            CString id = pDecoratedItem->GetItem().GetId();

            bool bApplicable = false;
            PACKAGE_STATE detectState = PACKAGE_STATE_UNKNOWN;
            pDecoratedItem->GetItem().GetPackageState(&bApplicable, &detectState, NULL, NULL);

            if (bApplicable)
            {
                // Bundle request state
                REQUEST_STATE requestState = defaultRequestState;

                // Request state can be overriden by Install Conditions
                if (pDecoratedItem->GetItem().GetInstallCondition())
                {
                    // evaluate InstallCondition
                    hr = ConditionEvaluate(&vpEngineState->variables, pDecoratedItem->GetItem().GetInstallCondition(), &fResult);
                    ExitOnFailure1(hr, "Failed to evaluate install condition: %S", pDecoratedItem->GetItem().GetInstallCondition());

                    if (fResult)
                    {
                        if (pDecoratedItem->GetItem().GetRepairCondition())
                        {
                            hr = ConditionEvaluate(&vpEngineState->variables, pDecoratedItem->GetItem().GetRepairCondition(), &fResult);
                            ExitOnFailure1(hr, "Failed to evaluate repair condition: %S", pDecoratedItem->GetItem().GetRepairCondition());

                            requestState = fResult ? REQUEST_STATE_REPAIR : REQUEST_STATE_PRESENT;
                        }
                        else
                        {
                            requestState = REQUEST_STATE_PRESENT;
                        }
                    }
                    else
                    {
                        requestState = REQUEST_STATE_ABSENT;
                    }
                }

                int nResult = pBurnView->OnPlanPackageBegin(id, &requestState);
                hr = HRESULT_FROM_VIEW(nResult);
                ExitOnRootFailure(hr, "UX aborted plan package begin.");

                const ItemBase* pItem = &pDecoratedItem->GetItem();
                const_cast<ItemBase*>(pItem)->SetRequestState(requestState);

                ACTION_STATE actionState;

                hr = GetPackageAction(detectState, requestState, &actionState);
                ExitOnRootFailure(hr, "Failed to get package action.");
                const_cast<ItemBase*>(pItem)->SetActionState(actionState);

                Operation::euiOperation operation;

                hr = GetOperationFromActionState(actionState, &operation);
                ExitOnRootFailure(hr, "Failed to get operation from action state.");

                hr = pDecoratedItem->Plan(pBurnView, operation, m_logger);
                ExitOnFailure1(hr, "Plan failed on package: %S", id);

                ACTION_STATE rollbackState = ACTION_STATE_NONE;
                if (ACTION_STATE_INSTALL == actionState)
                {
                    rollbackState = ACTION_STATE_UNINSTALL;
                }

                // Add to list of files that this package requires.
                if (ACTION_STATE_NONE != actionState)
                {
                    CSimpleArray<CString> externalFiles = pItem->GetReferencedFiles();
                    for (unsigned int j = 0; j < externalFiles.GetSize(); j++)
                    {
                        if (-1 == fileIdCache.Find(externalFiles[j]))
                        {
                            fileIdCache.Add(externalFiles[j]);
                        }
                    }
                }

                LOGEX(m_logger, ILogger::Information, L"Package [%s] - Detect state: %d; Request state: %d; Action state: %d; Rollback state: %d", id, detectState, requestState, actionState, rollbackState);

                pBurnView->OnPlanPackageComplete(id, hr, detectState, requestState, actionState, rollbackState, pItem->IsPerMachine()); 
            }
        }

        // Process the files (external cabs) that all the packages require.
        for (unsigned int i=0; i<m_items.GetChildItemCount(); ++i)
        {
            ItemStateDecorator* pDecoratedItem = dynamic_cast<ItemStateDecorator *>(m_items.GetChildItem(i));
            CString id = pDecoratedItem->GetItem().GetId();
            if (-1 != fileIdCache.Find(id))
            {
                hr = pDecoratedItem->Plan(pBurnView, Operation::uioInstalling, m_logger);
                ExitOnFailure1(hr, "Plan failed on File: %S", id);
            }
        }

        // Create download and install item views of decorated items
        m_pDownloadItems = new DownloadItemsView(m_items, m_dataToOperand);
        m_pInstallItems  = new InstallItemsView(m_items, m_dataToOperand, m_unDecoratedItems.GetChildItems(), m_logger);

        // Create mapping of items based on view.
        m_pDownloadItems->CreateMapping(m_operation);
        m_pInstallItems->CreateMapping(m_operation, true);

        CheckIfPackagesCanBeDownloaded();
        m_bInitialized = true;

    LExit:
        return hr;
    }

    HRESULT OnPlanPriorBundle(__in IBurnView* pBurnView)
    {
        HRESULT hr = S_OK;
        REQUEST_STATE state = REQUEST_STATE_PRESENT;
        int nResult = IDOK;

        //for (DWORD i = 0; i < vpEngineState->registration.cRegdataFamily; ++i)
        //{
        //    nResult = pBurnView->OnPlanPriorBundle(vpEngineState->registration.rgRegdataFamily[i].sczBundleId
        //                                , &vpEngineState->registration.rgRegdataFamily[i].requestState);
        //    hr = HRESULT_FROM_VIEW(nResult);
        //    ExitOnFailure(hr, "UX aborted plan prior bundle.");
        //}

        //for (DWORD i = 0; i < vpEngineState->registration.cRegdataUpgrades; ++i)
        //{
        //    nResult = pBurnView->OnPlanPriorBundle(vpEngineState->registration.rgRegdataUpgrades[i].sczBundleId
        //                                , &vpEngineState->registration.rgRegdataUpgrades[i].requestState);
        //    hr = HRESULT_FROM_VIEW(nResult);
        //    ExitOnFailure(hr, "UX aborted plan prior bundle.");
        //}

    LExit:
        return hr;
    }


    void SetRunningElevated()
    {
        m_bRunningElevated = true;
    }

private:
    
    // GetPackageAction 
    static HRESULT GetPackageAction(
        PACKAGE_STATE detectedState, 
        REQUEST_STATE requestState,
        ACTION_STATE *pActionState
        )
    {
        HRESULT hr = S_OK;
        *pActionState = ACTION_STATE_NONE;

        if (PACKAGE_STATE_PRESENT == detectedState)
        {
            switch (requestState)
            {
            case REQUEST_STATE_PRESENT: __fallthrough;
            case REQUEST_STATE_NONE:
                *pActionState = ACTION_STATE_NONE;
                break;
            case REQUEST_STATE_REPAIR:
                *pActionState = ACTION_STATE_RECACHE;
                break;
            case REQUEST_STATE_ABSENT:
                *pActionState = ACTION_STATE_UNINSTALL;
                break;
            default:
                hr = E_INVALIDARG;
                ExitOnRootFailure(hr, "Unsupported request state encountered.");
            }
        }
        else if (PACKAGE_STATE_ABSENT == detectedState)
        {
            switch (requestState)
            {
            case REQUEST_STATE_PRESENT:
                *pActionState = ACTION_STATE_INSTALL;
                break;
            case REQUEST_STATE_REPAIR:
                *pActionState = ACTION_STATE_RECACHE;
                break;
            case REQUEST_STATE_ABSENT: __fallthrough;
            case REQUEST_STATE_NONE:
                *pActionState = ACTION_STATE_NONE;
                break;
            default:
                hr = E_INVALIDARG;
                ExitOnRootFailure(hr, "Unsupported request state encountered.");
            }
        }
        else
        {
            hr = E_INVALIDARG;
            ExitOnRootFailure(hr, "Invalid package detection result encountered.");
        }
    LExit:
        return hr;
    }

    // GetOperationFromActionState
    static HRESULT GetOperationFromActionState(
        __in ACTION_STATE actionState, 
        __out Operation::euiOperation *pOperation
        )
    {
        HRESULT hr = S_OK;
        *pOperation = Operation::uioUninitalized;

        switch (actionState)
        {
        case ACTION_STATE_UNINSTALL:
            *pOperation = Operation::uioUninstalling;
            break;
        case ACTION_STATE_INSTALL:
            *pOperation = Operation::uioInstalling;
            break;
        case ACTION_STATE_MAINTENANCE:
            *pOperation = Operation::uioRepairing;
            break;
        case ACTION_STATE_RECACHE:
            *pOperation = Operation::uioRepairing;
            break;
        case ACTION_STATE_NONE:
            *pOperation = Operation::uioNone;
            break;
        default:
            hr = E_INVALIDARG;
            ExitOnRootFailure(hr, "Unsupported action state encountered.");
        }
    LExit:
        return hr;
    }

    static HRESULT GetRequestStateFromBurnAction(
        __in BURN_ACTION action,
        __out REQUEST_STATE* pRequestState
        )
    {
        HRESULT hr = S_OK;
        *pRequestState = REQUEST_STATE_NONE;

        switch (action)
        {
        case BURN_ACTION_INSTALL:
            *pRequestState = REQUEST_STATE_PRESENT;
            break;
        case BURN_ACTION_REPAIR:
            *pRequestState = REQUEST_STATE_REPAIR;
            break;
        case BURN_ACTION_UNINSTALL:
            *pRequestState = REQUEST_STATE_ABSENT;
            break;
        case BURN_ACTION_MODIFY:
            *pRequestState = REQUEST_STATE_NONE;
            break;
        default:
            hr = E_INVALIDARG;
            ExitOnFailure(hr, "Invalid action state.");
        }

    LExit:
        return hr;
    }

};
typedef CoordinatorT<FileAuthenticity> Coordinator;
}
