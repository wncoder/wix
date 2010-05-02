//-------------------------------------------------------------------------------------------------
// <copyright file="UXFactory.h" company="Microsoft">
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
//      UXFactory - Creates Burn UX and related objects.
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once

namespace IronMan
{

class UXFactory
{
    HMODULE m_hUxDll;

public:
    UXFactory()
    {
        m_hUxDll = NULL;
    }

    HRESULT Load(LPCWSTR uxDllPath, BURN_COMMAND* pCommand, IBurnUserExperience** ppUserExperience)
    {
        HRESULT hr = S_OK;

        // load UX DLL
        m_hUxDll = ::LoadLibraryW(uxDllPath);
        ExitOnNullWithLastError(m_hUxDll, hr, "Failed to load UX DLL.");

        // get SetupUXCreate entry-point
        PFN_CREATE_USER_EXPERIENCE pfnSetupUXCreate = (PFN_CREATE_USER_EXPERIENCE)::GetProcAddress(m_hUxDll, "SetupUXCreate");
        ExitOnNullWithLastError(pfnSetupUXCreate, hr, "Failed to get SetupUXCreate entry-point");

        // create UX
        hr = pfnSetupUXCreate(pCommand, ppUserExperience);
        ExitOnFailure(hr, "Failed to create UX.");

    LExit:
        return hr;
    }

    void Unload()
    {
        if (m_hUxDll)
        {
            // get SetupUXDestroy entry-point
            PFN_DESTROY_USER_EXPERIENCE pfnSetupUXDestroy = (PFN_DESTROY_USER_EXPERIENCE)::GetProcAddress(m_hUxDll, "SetupUXDestroy");

            // destroy UX
            if (pfnSetupUXDestroy)
            {
                pfnSetupUXDestroy();
            }

            // free UX DLL
            ::FreeLibrary(m_hUxDll);
        }
    }

    virtual ~UXFactory() 
    {
    }
};

}
