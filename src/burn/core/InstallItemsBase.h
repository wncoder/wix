//-------------------------------------------------------------------------------------------------
// <copyright file="InstallItemsBase.h" company="Microsoft">
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

#include "IInstallItems.h"
#include "Schema\EngineData.h"

namespace IronMan
{
class InstallItemsBase : public IInstallItems
{
    virtual unsigned int GetCount() const = 0;
    virtual const ItemBase* GetItem(unsigned int nIndex) const = 0;
    virtual bool IsItemAvailable(unsigned int nIndex) const = 0;
    virtual bool IsItemComplete(unsigned int nIndex) const = 0;
    virtual void UpdateItemState(unsigned int nIndex) = 0;
    virtual bool IsItemNotAvailableAndIgnorable(unsigned int nIndex) const = 0;

protected:
    virtual ULONGLONG GetSystemDriveSize(unsigned int nIndex)
    {
        ULONGLONG systemDriveSize = 0;
        const ItemBase* pItem = GetItem(nIndex);
        if ( ItemBase::Patches == pItem->GetType() )
        {
                const Patches* patches = static_cast<const Patches*>(pItem);
                for(unsigned int index=0; index < patches->GetCount(); ++index)
                {
                    systemDriveSize += (patches->GetMsp(index)).GetSystemDriveSize();
                }
        }
        else if ( ItemBase::AgileMsi == pItem->GetType() )
        {
                const AgileMSI* agileMsi = static_cast<const AgileMSI*>(pItem);

                systemDriveSize = agileMsi->GetSystemDriveSize();
                for(unsigned int index=0; index < agileMsi->GetCount(); ++index)
                {
                    systemDriveSize += (agileMsi->GetAgileMsp(index)).GetSystemDriveSize();
                }
        }
        else
        {
            const LocalPath* localPath = dynamic_cast<const LocalPath*>(pItem);
            if ( localPath != NULL)
            {
                systemDriveSize = localPath->GetSystemDriveSize();                  
            }
            else
            {
                systemDriveSize = 1;
                if (ItemBase::ServiceControl != pItem->GetType() && ItemBase::CleanupBlockType != pItem->GetType() && ItemBase::RelatedProductsType != pItem->GetType())
                {
                    IMASSERT2(false, L"Failed to get LocalPath");
                }
            }
        }
        return systemDriveSize;
    }
    virtual ULONGLONG GetInstalledProductSize(unsigned int nIndex)
    {
        ULONGLONG installedProductSize = 0;
        const ItemBase* pItem = GetItem(nIndex);
        if ( ItemBase::Patches == pItem->GetType() )
        {
                const Patches* patches = static_cast<const Patches*>(pItem);
                for(unsigned int index=0; index < patches->GetCount(); ++index)
                {
                    installedProductSize += (patches->GetMsp(index)).GetInstalledProductSize();
                }
        }
        else if ( ItemBase::AgileMsi == pItem->GetType() )
        {
                const AgileMSI* agileMsi = static_cast<const AgileMSI*>(pItem);

                installedProductSize = agileMsi->GetInstalledProductSize();
                for(unsigned int index=0; index < agileMsi->GetCount(); ++index)
                {
                    installedProductSize += (agileMsi->GetAgileMsp(index)).GetInstalledProductSize();
                }
        }
        else
        {
            const LocalPath* localPath = dynamic_cast<const LocalPath*>(pItem);
            if ( localPath != NULL)
            {
                installedProductSize = localPath->GetInstalledProductSize();                    
            }
            else
            {
                installedProductSize = 1;
                if (ItemBase::ServiceControl != pItem->GetType() && ItemBase::CleanupBlockType != pItem->GetType() && ItemBase::RelatedProductsType != pItem->GetType())
                {
                    IMASSERT2(false, L"Failed to get LocalPath");
                }
            }
        }
        return installedProductSize;
    }

    virtual CString GetItemOriginalName(unsigned int nIndex) const
    {
        const ItemBase* pItem = GetItem(nIndex);

        if (   ItemBase::Msi == pItem->GetType() 
            || ItemBase::Msp == pItem->GetType() 
            || ItemBase::Exe == pItem->GetType() )
        {
            const LocalPath* localPath = dynamic_cast<const LocalPath*>(pItem);
            return localPath->GetOriginalName();
        }
        else
        {
            IMASSERT2(false, L"Original Name for an item other than exe, msi or msp!!");
            return L"";
        }
    }

    virtual CString GetItemName(unsigned int nIndex) const
    {
        CString name;

        const ItemBase* pItem = GetItem(nIndex);
        if ( ItemBase::AgileMsi == pItem->GetType() )
        {
                const AgileMSI* agileMsi = static_cast<const AgileMSI*>(pItem);
                CPath path = agileMsi->GetName();
                path.StripPath();
                name = path.m_strPath;
                for ( unsigned int index = 0; index < agileMsi->GetCount(); ++index)
                {
                    path = agileMsi->GetAgileMsp(index).GetName();
                    path.StripPath();
                    name += L";" + path.m_strPath;
                }
        }
        else if ( ItemBase::Patches == pItem->GetType() )
        {
                const Patches* patches = static_cast<const Patches*>(pItem);
                for(unsigned int index=0; index < patches->GetCount(); ++index)
                {
                    if ( 0 != index )
                    {
                        name += L";" ;
                    }
                    CPath path = (patches->GetMsp(index)).GetName();
                    path.StripPath();
                    name += path.m_strPath;
                }
        }
        else if ( ItemBase::ServiceControl == pItem->GetType() )
        {
            const ServiceControl* serviceControl = static_cast<const ServiceControl*>(pItem);
            name = serviceControl->GetServiceName();
        }
        else if ( ItemBase::CleanupBlockType == pItem->GetType() )
        {
            const CleanupBlock* cub = static_cast<const CleanupBlock*>(pItem);
            name = cub->GetCleanupBlockName();
            if (name.IsEmpty())
                name = L"CleanupBlock";
        }
        else if ( ItemBase::RelatedProductsType == pItem->GetType())
        {
            const RelatedProducts* products = static_cast<const RelatedProducts*>(pItem);
            name = products->GetName();
        }
        else
        {
            // MSI, MSP, Exe, ServiceControl
            const LocalPath* localPath = dynamic_cast<const LocalPath*>(pItem);
            if ( localPath != NULL)
            {
                return localPath->GetName();
            }
            else
            {
                IMASSERT2(false, L"Failed to get LocalPath in GetItemName");
            }
        }

        return name;
    }
    virtual ULONGLONG GetEstimatedInstallTime(unsigned int nIndex)
    {
        ULONGLONG estimatedInstallSize = 0;
        const ItemBase* pItem = GetItem(nIndex);
        if ( ItemBase::Patches == pItem->GetType() )
        {
                const Patches* patches = static_cast<const Patches*>(pItem);
                for(unsigned int index=0; index < patches->GetCount(); ++index)
                {
                    estimatedInstallSize += (patches->GetMsp(index)).GetEstimatedInstallTime();
                }
        }
        else
        {
            estimatedInstallSize = pItem->GetEstimatedInstallTime();
        }

        return estimatedInstallSize;
    }

    virtual ULONGLONG GetPackageCacheSize(unsigned int nIndex) const
    {
        ULONGLONG ullPackageCacheSize = 0;
        const ItemBase* pItem = GetItem(nIndex);
        if ( ItemBase::Patches == pItem->GetType() )
        {
                const Patches* patches = static_cast<const Patches*>(pItem);
                for(unsigned int index=0; index < patches->GetCount(); ++index)
                {
                    ullPackageCacheSize += (patches->GetMsp(index)).GetCompressedItemSize();
                }
        }
        else if ( ItemBase::AgileMsi == pItem->GetType() )
        {
                const AgileMSI* agileMsi = static_cast<const AgileMSI*>(pItem);

                ullPackageCacheSize = agileMsi->GetCompressedItemSize();
                for(unsigned int index=0; index < agileMsi->GetCount(); ++index)
                {
                    ullPackageCacheSize += (agileMsi->GetAgileMsp(index)).GetCompressedItemSize();
                }
        }
        else
        {
            const DownloadPath* downloadPath = dynamic_cast<const DownloadPath*>(pItem);
            if ( downloadPath != NULL)
            {
                ullPackageCacheSize = downloadPath->GetCompressedItemSize();
            }
            else
            {
                ullPackageCacheSize = 1;
                if (ItemBase::ServiceControl != pItem->GetType() && ItemBase::CleanupBlockType != pItem->GetType() && ItemBase::RelatedProductsType != pItem->GetType())
                {
                    IMASSERT2(false, L"Failed to get DownloadPath");
                }
            }
        }
        return ullPackageCacheSize;
    }
};
}
