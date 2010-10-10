//-------------------------------------------------------------------------------------------------
// <copyright file="CFrostCore.h" company="Microsoft">
//    Copyright (c) Microsoft Corporation.  All rights reserved.
//    
//    The use and distribution terms for this software are covered by the
//    Common Public License 1.0 (http://opensource.org/licenses/cpl1.0.php)
//    which can be found in the file CPL.TXT at the root of this distribution.
//    By using this software in any fashion, you are agreeing to be bound by
//    the terms of this license.
//    
//    You must not remove this notice, or any other, from this software.
// </copyright>
// 
// <summary>
//    CFrostCore defines the proxy engine's core.
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once

namespace Microsoft
{
namespace Tools
{
namespace WindowsInstallerXml
{
namespace Test
{
namespace Frost
{
    class CFrostCore : public IBurnCore
    {
    public:
        CFrostCore();
        virtual ~CFrostCore();

        // IUnknown
        /*
        virtual HRESULT __stdcall QueryInterface(__in const IID& riid,  __out void** ppvObject);
        virtual ULONG __stdcall AddRef();
        virtual ULONG __stdcall Release();
        */

        // IBurnCore

        STDMETHODIMP Elevate(__in_opt HWND hwndParent);
        STDMETHODIMP Detect();
        STDMETHODIMP Plan(__in BURN_ACTION action);
        STDMETHODIMP Apply(__in_opt HWND hwndParent);
        STDMETHODIMP Suspend();
        STDMETHODIMP Reboot();
        STDMETHODIMP GetPackageCount(__out DWORD* pcPackages);
        STDMETHODIMP GetCommandLineParameters(__out_ecount_opt(*pcchCommandLine) LPWSTR psczCommandLine, __inout DWORD* pcchCommandLine);
        STDMETHODIMP GetVariableNumeric(__in_z LPCWSTR wzProperty, __out LONGLONG* pllValue);
        STDMETHODIMP GetVariableString( __in_z LPCWSTR wzVariable, __out_ecount_opt(*pcchValue) LPWSTR wzValue, __inout DWORD* pcchValue);
        STDMETHODIMP GetVariableVersion(__in_z LPCWSTR wzProperty, __in DWORD64* pqwValue);
        STDMETHODIMP SetVariableNumeric(__in_z LPCWSTR wzProperty, __in LONGLONG llValue);
        STDMETHODIMP SetVariableString(__in_z LPCWSTR wzProperty, __in_z_opt LPCWSTR wzValue);
        STDMETHODIMP SetVariableVersion(__in_z LPCWSTR wzProperty, __in DWORD64 qwValue);
        STDMETHODIMP FormatString(__in_z LPCWSTR wzIn, __out_ecount_opt(*pcchOut) LPWSTR wzOut, __inout DWORD* pcchOut);
        STDMETHODIMP EscapeString(__in_z LPCWSTR wzIn, __out_ecount_opt(*pcchOut) LPWSTR wzOut, __inout DWORD* pcchOut);
        STDMETHODIMP EvaluateCondition(__in_z LPCWSTR wzCondition, __out BOOL* pf);
        STDMETHODIMP Log(__in BURN_LOG_LEVEL level, __in_z LPCWSTR wzMessage);
        STDMETHODIMP SetSource(__in LPCWSTR wzSourcePath);

    private:
        long m_cReferences;
    };
}
}
}
}
}