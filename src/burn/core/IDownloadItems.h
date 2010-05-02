//-------------------------------------------------------------------------------------------------
// <copyright file="IDownloadItems.h" company="Microsoft">
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

#include "IItems.h"

#define HRESULT_IS_NETPATH_NOT_FOUND(n) (ERROR_BAD_NETPATH == HRESULT_CODE(n) || ERROR_BAD_NET_NAME == HRESULT_CODE(n) || RPC_S_SERVER_UNAVAILABLE == HRESULT_CODE(n) || BG_E_HTTP_ERROR_404 == n)

namespace IronMan
{
struct IDownloadItems : public IItems
{
    virtual unsigned int GetCount() const = 0;
    virtual void GetItem(unsigned int nIndex
                            , CUrl* pUrl
                            , CString &csHash
                            , CPath &pthPath
                            , ULONGLONG &ulItemSize
                            , bool &bIgnoreDownloadFailure
                            , CString& csItemName
                            , CString& csItemId) const = 0;
    virtual void UpdateItemState(unsigned int nIndex, bool bDownloadFailureIgnored = false) = 0;
    virtual bool IsItemCompressed(unsigned int nIndex) const = 0;
    virtual ULONGLONG GetDownloadDiskSpaceRequirement() const = 0;
};
}
