//-------------------------------------------------------------------------------------------------
// <copyright file="NullMetrics.h" company="Microsoft">
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
//      This class encapsulate all the functionality to collect usage data.
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once

#include "interfaces\IMetrics.h"

namespace IronMan
{
    class NullMetrics : public IMetrics
    {
    public:
        NullMetrics()
        {}

        virtual HRESULT ContinueSession()
        {
            return S_OK;
        }

        virtual HRESULT StartSession()
        {
            return S_OK;
        }

        virtual HRESULT EndAndSend(const CPath& pthDataFileFullPath, const CPath& pthLoaderFileFullPath)
        {
            return S_OK;
        }

        virtual HRESULT WriteStream(unsigned int iDatapointId, int iNumberofColumn, const CString& csStringToWrite)
        {
            return S_OK;
        }

        virtual HRESULT WriteStream(unsigned int iDatapointId, int iNumberofColumn, const DWORD& dw)
        {
            return S_OK;
        }

        virtual HRESULT Write(__in DWORD dwId, __in_opt DWORD dwVal)
        {
            return S_OK;
        }

        virtual HRESULT Write(__in DWORD dwId, __in_opt HRESULT hrVal)
        {
            return S_OK;
        }

        virtual HRESULT Write(__in DWORD dwId, __in_opt bool bVal)
        {
            return S_OK;
        }

        virtual HRESULT Write(__in DWORD dwId, __in_opt const CString& csVal)
        {
            return S_OK;
        }
    };
}