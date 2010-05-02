//-------------------------------------------------------------------------------------------------
// <copyright file="LayoutList.h" company="Microsoft">
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

#include "schema\engineData.h"
#include "ModuleUtils.h"
#include "schema\CustomError.h"

namespace IronMan
{
class LayoutList
{
    Items m_items;
public:
    LayoutList(const Items& items, const CPath& layout, bool preferLocal, ILogger& logger)
        : m_items(items.GetRetries(), items.GetDelay(), items.GetCopyPackageFilesFlag(), logger, items.GetFailureAction())
    {
        // get current directory
        CPath cwd = ModuleUtils::GetDllPath();

        // update each item's LocalPath iff it's a base
        for(unsigned int i=0; i<items.GetCount(); ++i)
        {
            const ItemBase* pItem = items.GetItem(i);
            if (!pItem) continue; // silence PREfast
            const HelperItems* pHelperItems = dynamic_cast<const HelperItems*>(pItem);
            // Update the helpers' paths of this item with working directory (if present in the payload)
            // or with layout folder where they will be downloaded to.
            if (pHelperItems)
                pHelperItems->Update(cwd, layout, preferLocal);
            
            const LocalPath* lp = dynamic_cast<const LocalPath*>(pItem);
            if (lp)
            {
                // MSI, MSP, EXE and File items need to be updated with correct working directory 
                // or layout path. This can be accomplished by LocalPath::Twin(inputPath)

                // AgileMSI inherits from MSI which inherits from LocalPath
                // But lp->Twin() is not resolving to AgileMSI->Twin() when the pItem is AgileMSI.
                // Instead, it is calling MSI->Twin(). Work-around for vtable bug (?).
                const AgileMSI* agileMsi = dynamic_cast<const AgileMSI*>(pItem);
                if (agileMsi)
                {
                    // AgileMSI object needs to be Twin-ed to update the msi path with UseLocal
                    const AgileMSI* pAgile = static_cast<const AgileMSI*>(lp->Twin(UseLocal(layout, cwd, preferLocal, *lp)));
                    AgileMSI agile(*pAgile);
                    CSimpleArray<AgileMSP> msps;
                    for(unsigned int j=0; j<pAgile->GetCount(); ++j)
                    {
                        // Each msp is also Twin-ed to be updated with UseLocal
                        AgileMSP* msp = pAgile->GetAgileMsp(j).Twin(UseLocal(layout, cwd, preferLocal, pAgile->GetAgileMsp(j)));
                        msps.Add(*msp);
                        delete msp;
                    }
                    agile.ReplaceAgileMsps(msps);                    
                    
                    // The resulting new agile object is Cloned as it has correct and updated paths
                    m_items.Add(agile.Clone());
                }
                else
                {
                    const Exe* pExe = dynamic_cast<const Exe*>(pItem);
                    if (pExe)
                    {
                        const ExeBase* repairOverride = pExe->GetRepairOverride();
                        if (repairOverride)
                        {
                            const LocalPath* lpRepairOverride = dynamic_cast<const LocalPath*>(repairOverride);
                            IMASSERT(NULL != lpRepairOverride);
                            CPath updatedPath;
                            updatedPath.Combine(UseLocal(layout, cwd, preferLocal, *lpRepairOverride), lpRepairOverride->GetName());
                            lpRepairOverride->UpdateName(updatedPath);
                        }
                        const ExeBase* uninstallOverride = pExe->GetUninstallOverride();
                        if (uninstallOverride)
                        {
                            const LocalPath* lpUninstallOverride = dynamic_cast<const LocalPath*>(uninstallOverride);
                            IMASSERT(NULL != lpUninstallOverride);
                            CPath updatedPath;
                            updatedPath.Combine(UseLocal(layout, cwd, preferLocal, *lpUninstallOverride), lpUninstallOverride->GetName());
                            lpUninstallOverride->UpdateName(updatedPath);
                        }
                    }
                    // Individual item should just be Twin-ed to update its path.
                    m_items.Add(lp->Twin(UseLocal(layout, cwd, preferLocal, *lp)));

                }
            }
            else
            {
                if (pItem->GetType() == ItemBase::Patches)
                {
                    // Item doesn't derive from LocalPath and HAS dependent child items.
                    // Each child item needs to be Twin-ed before Clone-ing the item.

                    const Patches* pPatches = static_cast<const Patches*>(pItem);
                    const ActionTable* pActionTable = dynamic_cast<const ActionTable*>(pItem); 
                    const CustomErrorHandling* pCustomErrorHandling = dynamic_cast<const CustomErrorHandling*>(pItem);


                    IMASSERT(  NULL != pPatches 
                            && NULL != pActionTable 
                            && NULL != pCustomErrorHandling 
                            && NULL != pHelperItems ); 

                    // Create new patches object from current one.
                    Patches patches(pPatches->ShouldIgnoreDownloadFailure()
                                    , pPatches->ShouldRollBack()
                                    , logger
                                    , *pActionTable
                                    , *pCustomErrorHandling
                                    , *pHelperItems);                                    

                    // Update each msp's path with UseLocal path using Twin.
                    for(unsigned int j=0; j<pPatches->GetCount(); ++j)
                    {
                        MSP* msp = pPatches->GetMsp(j).Twin(UseLocal(layout, cwd, preferLocal, pPatches->GetMsp(j)));
                        patches.AddMsp(*msp);
                        delete msp;
                    }

                    // Clone the created patches object to create copy of it.
                    m_items.Add(patches.Clone());
                }
                else
                {
                    // Item doesn't derive from LocalPath and has no dependent child items.
                    // Just Clone() it to make a copy.
                    m_items.Add(pItem->Clone());
                }
            }
        }

        // Add child elements
        for(unsigned int i=0; i<items.GetChildItemCount(); ++i)
        {
            const ItemBase* pItem = items.GetChildItem(i);
            if (!pItem) 
            {
                continue; // silence PREfast
            }

            const LocalPath* lp = dynamic_cast<const LocalPath*>(pItem);
            if (lp)
            {
                // Individual item should just be Twin-ed to update its path.
                m_items.AddChildItem(lp->Twin(UseLocal(layout, cwd, preferLocal, *lp)));
            }
            else
            {
                // Item doesn't derrive from LocalPath and has no dependent child items.
                // Just Clone() it to make a copy.
                m_items.AddChildItem(pItem->Clone());
            }
        }
    }
    virtual ~LayoutList() {}
    const Items& GetItems() const { return m_items; }

private:
    /*-------------------------------------------------------------------------------------------
        UseLocal: 
          input: layout 
                    Takes layout (%temp%\package or path passed in to create layout)
          input: cwd
                    Current working directory. If an item is carried with the payload, 
                    it is found here.
          input: preferlocal
                    True  - creating Non Layout List - which will be used to perform
                            install/repair/uninstall etc operations.
                    False - creating Create Layout List - which will be used during 
                            acquiring/copying payload for /createlayout operation.
         output: lp
                    If creating layoutlist, preferlocal is false and layout path is returned in lp.
                    If creating non-layout list, if the item is present in the payload, lp will
                    be updated to return payload path.
        ------------------------------------------------------------------------------------------*/

    static CPath UseLocal(const CPath& layout, const CPath& cwd, bool preferLocal, const LocalPath& lp)
    {
        if (!preferLocal)
            return layout;

        CPath local;
        local.Combine(cwd, lp.GetName());
        if (local.FileExists())
            return cwd;
        return layout;
    }
};
}
