//-------------------------------------------------------------------------------------------------
// <copyright file="IInstallItems.h" company="Microsoft">
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

#include "Schema\EngineData.h"

namespace IronMan
{
//
// Common interface inherited by IDownloadItems & IInstallItems.
// This interface is used by the cache manager
//
struct IItems
{
    virtual bool IsItemAvailableUnVerified(unsigned int nIndex) const = 0;
    virtual bool IsItemAvailable(unsigned int nIndex) const = 0;
    virtual bool IsItemOnDownloadList(unsigned int nIndex) const = 0;
    virtual bool VerifyItem(unsigned int nIndex, ILogger& logger) = 0;
    virtual void SetItemStateAsAvailable(unsigned int nIndex) = 0;
    virtual void UpdateItemPath(unsigned int nIndex, const CPath& itemPath) = 0;

    // methods needed by the cache manager
    virtual CString GetItemName(unsigned int nIndex) const = 0;
    virtual bool DoesItemNeedToBeCached(unsigned int nIndex, CPath& pathCachedFileName, bool& bPerMachine) const = 0;
    virtual unsigned int GetChildItemCount(unsigned int nParentIndex) const = 0;
    virtual const ItemBase* GetChildItem(unsigned int nParentIndex, unsigned int nChildIndex) const = 0;
};
}
