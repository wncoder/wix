//-------------------------------------------------------------------------------------------------
// <copyright file="SmartLock.h" company="Microsoft">
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

#include "Interfaces\IExceptions.h"

namespace IronMan
{

class SmartLibrary
{
private:
    HMODULE m_hModule;
    CString m_strLibraryName; // Store library name

public:
    SmartLibrary(const CPath& dll) :
      m_hModule(0)
    {
        m_strLibraryName = dll.m_strPath;
        // Load the library if one was specified
        if (!m_strLibraryName.IsEmpty())
        {
            Load(dll);
            if (!m_hModule)
            {
                throw( CWinAPIException(L"LoadLibrary", ::GetLastError()) );
            }
        }
    }

    virtual ~SmartLibrary()
    {
        if (m_hModule)
        {
            ::FreeLibrary(m_hModule);
            m_hModule = 0;
        }
    }

    HINSTANCE Load(const CPath& dll)
    {
        // Load the library , freeiung up existing library if there is one
        if (m_hModule)
        {
            ::FreeLibrary(m_hModule);
            m_hModule = 0;
        }

        // Store library name
        m_strLibraryName = dll.m_strPath;
        m_hModule = ::LoadLibrary(m_strLibraryName);

        return m_hModule;
    }

    template<typename T> T GetProcAddress(LPCSTR functionName)
    {
        T t = reinterpret_cast<T>(::GetProcAddress(m_hModule, functionName));
        if (!t)
        {
            DWORD dwLastError = ::GetLastError();
            CString strProblemDescription(L"GetProcAddress looking for ");
            // Add function and library name
            strProblemDescription += CString(functionName) + CString(L" in ") + m_strLibraryName;
            throw( CWinAPIException(strProblemDescription,dwLastError));
        }

        return t;
    }
};

}
