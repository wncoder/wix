//-------------------------------------------------------------------------------------------------
// <copyright file="CoInitializer.h" company="Microsoft">
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

namespace IronMan
{

//Remark: This creates a STA COM.  It will not pump message when blocked on WaitForSingleObject() 
class CoInitializer
{
public:
    CoInitializer() :
        m_fComInitialized(FALSE)
    {
        HRESULT hr = ::CoInitialize(NULL);
        if (SUCCEEDED(hr))
        {
            // CoUninitialize should be called even for S_FALSE.
            m_fComInitialized = TRUE;
        }
        else if (RPC_E_CHANGED_MODE == hr)
        {
            TraceError(hr, "COM already initialized using a different threading model.");
        }
    }

    ~CoInitializer()
    {
        if (m_fComInitialized)
        {
            ::CoUninitialize();
        }
    }

private:
    BOOL m_fComInitialized;
};
}
