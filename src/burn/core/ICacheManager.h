//-------------------------------------------------------------------------------------------------
// <copyright file="ICacheManager.h" company="Microsoft">
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
//      The interface for the cache manager
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once

namespace IronMan
{

struct ICacheManager
{
    virtual HRESULT VerifyAndCachePackage(IInstallItems& items, unsigned int nIndex, UnElevatedController* pUnElevatedController = NULL) = 0;
    virtual bool IsCached(IItems& items, unsigned int nIndex, UnElevatedController* pUnElevatedController = NULL) = 0;
    virtual HRESULT DeleteCachedPackage(IItems& items, unsigned int nIndex, UnElevatedController* pUnElevatedController) = 0;
    virtual void DeleteTemporaryCacheDirectories(UnElevatedController* pUnElevatedController) const = 0;
};

class NullCacheManager : public ICacheManager
{
public:
    static ICacheManager& GetNullCacheManager()
    {
        static NullCacheManager nullCacheManager;
        return nullCacheManager;
    }
private:
    virtual HRESULT VerifyAndCachePackage(IInstallItems& items, unsigned int nIndex, UnElevatedController* pUnElevatedController = NULL) { return S_OK; }
    virtual bool IsCached(IItems& items, unsigned int nIndex, UnElevatedController* pUnElevatedController = NULL) { return false; }
    virtual HRESULT DeleteCachedPackage(IItems& items, unsigned int nIndex, UnElevatedController* pUnElevatedController) { return S_OK; }
    virtual void DeleteTemporaryCacheDirectories(UnElevatedController* pUnElevatedController) const { return; }
};

}
