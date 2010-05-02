//-------------------------------------------------------------------------------------------------
// <copyright file="CUnknownImpl.h" company="Microsoft">
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
//-------------------------------------------------------------------------------------------------

#include "precomp.h"

template<class T, class I = T>
class CUnknownImpl : public T
{
public: // IUnknown
    virtual HRESULT __stdcall QueryInterface(
        __in const IID& riid,
        __out void** ppvObject
        )
    {
        HRESULT hr = S_OK;

        ExitOnNull(ppvObject, hr, E_INVALIDARG, "Invalid argument ppvObject");
        *ppvObject = NULL;

        if (::IsEqualIID(__uuidof(I), riid))
        {
            *ppvObject = static_cast<I*>(this);
        }
        else if (::IsEqualIID(IID_IUnknown, riid))
        {
            *ppvObject = reinterpret_cast<IUnknown*>(this);
        }
        else // no interface for requested iid
        {
            ExitFunction1(hr = E_NOINTERFACE);
        }

        AddRef();
    LExit:
        return hr;
    }


    virtual ULONG __stdcall AddRef()
    {
        return ::InterlockedIncrement(&this->m_cReferences);
    }


    virtual ULONG __stdcall Release()
    {
        long l = ::InterlockedDecrement(&this->m_cReferences);
        if (0 < l)
        {
            return l;
        }

        delete this;
        return 0;
    }


protected:
    //
    // Constructor - intitialize member variables.
    //
    CUnknownImpl()
    {
        m_cReferences = 1;
    }

    //
    // Destructor - release member variables.
    //
    virtual ~CUnknownImpl()
    {
    }

protected:
    long m_cReferences;
};


// IUnknown implementation that exposes two interfaces.
template<class T1, class T2>
class CUnknownImpl2 : public T1, public T2
{
public: // IUnknown
    virtual HRESULT __stdcall QueryInterface(
        __in const IID& riid,
        __out void** ppvObject
        )
    {
        HRESULT hr = S_OK;

        ExitOnNull(ppvObject, hr, E_INVALIDARG, "Invalid argument ppvObject");
        *ppvObject = NULL;

        if (::IsEqualIID(__uuidof(T1), riid))
        {
            *ppvObject = static_cast<T1*>(this);
        }
        else if (::IsEqualIID(__uuidof(T2), riid))
        {
            *ppvObject = static_cast<T2*>(this);
        }
        else if (::IsEqualIID(IID_IUnknown, riid))
        {
            *ppvObject = reinterpret_cast<IUnknown*>(this);
        }
        else // no interface for requested iid
        {
            ExitFunction1(hr = E_NOINTERFACE);
        }

        AddRef();
    LExit:
        return hr;
    }


    virtual ULONG __stdcall AddRef()
    {
        return ::InterlockedIncrement(&this->m_cReferences);
    }


    virtual ULONG __stdcall Release()
    {
        long l = ::InterlockedDecrement(&this->m_cReferences);
        if (0 < l)
        {
            return l;
        }

        delete this;
        return 0;
    }


protected:
    //
    // Constructor - intitialize member variables.
    //
    CUnknownImpl2()
    {
        m_cReferences = 1;
    }

    //
    // Destructor - release member variables.
    //
    virtual ~CUnknownImpl2()
    {
    }

protected:
    long m_cReferences;
};
