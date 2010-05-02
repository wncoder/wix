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
#include "IItems.h"

namespace IronMan
{
struct IInstallItems : public IItems
{
    virtual unsigned int GetCount() const = 0;
    virtual const ItemBase* GetItem(unsigned int nIndex) const = 0;
    virtual bool IsItemComplete(unsigned int nIndex) const = 0;
    virtual void UpdateItemState(unsigned int nIndex) = 0;
    virtual ULONGLONG GetSystemDriveSize(unsigned int nIndex) = 0;
    virtual ULONGLONG GetInstalledProductSize(unsigned int nIndex) = 0;
    virtual ULONGLONG GetEstimatedInstallTime(unsigned int nIndex) = 0;
    virtual bool IsItemNotAvailableAndIgnorable(unsigned int nIndex) const = 0;
    virtual ULONGLONG GetPackageCacheSize(unsigned int nIndex) const = 0;
    
    // methods needed to handle unavailable items that we need to ask UX to resolve source.
    virtual void SetItemStateToIgnorable(unsigned int nIndex) const = 0;
    virtual CString GetItemOriginalName(unsigned int nIndex) const = 0;
    virtual CString GetItemID(unsigned int nIndex) const = 0;
};
}