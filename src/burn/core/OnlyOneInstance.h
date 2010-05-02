//-------------------------------------------------------------------------------------------------
// <copyright file="OnlyOneInstance.h" company="Microsoft">
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

#include "interfaces\ILogger.h"
#include "schema\ConfigurationElement.h"

namespace IronMan
{

//------------------------------------------------------------------------------
// Class: IronMan::OnlyOneInstance
// Ensures that only a single instance of a package runs
//------------------------------------------------------------------------------
class OnlyOneInstance
{
    HANDLE m_packageMutex;
    CString m_mutexName;
    const CString& m_packageName;
    bool m_isFirstInstance;
    ILogger& m_logger;

public:
    
    //
    // Constructor: Checks if Mutex has already been created, if not creates it
    // 
    OnlyOneInstance(const BlockingMutex& mutex
                    , const CString& packageName
                    , ILogger& logger)
        : m_packageName(packageName)
        , m_logger(logger)
        , m_packageMutex(NULL)
        , m_isFirstInstance(false)
    {
        if (!mutex.IsDefined())
        {
            m_isFirstInstance = true;
            return;
        }
        if (!mutex.GetMutexName().IsEmpty())
        {
            m_mutexName= L"Global\\" + mutex.GetMutexName();
        }
        else
        {
            m_mutexName = L"Global\\" + packageName;
        }

        // If the Mutex cannot be opened, then it means that the Mutex hasn't been 
        // created yet and this instance is the first instance of the packag to run;
        m_packageMutex = OpenMutex(SYNCHRONIZE, NULL, m_mutexName);
        if (NULL == m_packageMutex)
        {            
            m_packageMutex = CreateMutex(NULL, NULL, m_mutexName);
            if (ERROR_ALREADY_EXISTS != GetLastError())
            {
                m_isFirstInstance = true;
            }
        }
    }


    // Destructor
    ~OnlyOneInstance()
    {
        CloseHandle(m_packageMutex);
    }


    //
    // Checks if another instance of this package is running
    //
    HRESULT CheckIfAnotherInstanceOfPackageIsRunning()
    {
        if (m_isFirstInstance)
        {
            return S_OK;
        }
        else
        {
            CString message;
            HRESULT hr = ERROR_INSTALL_ALREADY_RUNNING;
            return hr;
        }
    }
};
}
