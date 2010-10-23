//-------------------------------------------------------------------------------------------------
// <copyright file="ii7util.cpp" company="Microsoft">
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
//    Time helper functions.
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"

extern "C" HRESULT DAPI Iis7PutPropertyVariant(
    __in IAppHostElement *pElement,
    __in LPCWSTR wszPropName,
    __in VARIANT vtPut
    )
{
    HRESULT hr = S_OK;
    IAppHostProperty *pProperty = NULL;
    BSTR bstrPropName = NULL;

    bstrPropName = ::SysAllocString(wszPropName);
    ExitOnNull(bstrPropName, hr, E_OUTOFMEMORY, "failed SysAllocString");

    hr = pElement->GetPropertyByName(bstrPropName, &pProperty);
    ExitOnFailure1(hr, "Failed to get property object for %S", wszPropName);

    hr = pProperty->put_Value(vtPut);
    ExitOnFailure1(hr, "Failed to set property value for %S", wszPropName);
LExit:
    ReleaseNullBSTR(bstrPropName);
    // caller responsible vor cleaning up variant vtPut
    ReleaseNullObject(pProperty);

    return hr;
}

extern "C" HRESULT DAPI Iis7PutPropertyString(
    __in IAppHostElement *pElement,
    __in LPCWSTR wszPropName,
    __in LPCWSTR wszString
    )
{
    HRESULT hr = S_OK;
    VARIANT vtPut;

    VariantInit(&vtPut);
    vtPut.vt = VT_BSTR;
    vtPut.bstrVal = SysAllocString(wszString);
    hr = Iis7PutPropertyVariant(pElement, wszPropName, vtPut);
    VariantClear(&vtPut);
    return hr;
}

extern "C" HRESULT DAPI Iis7PutPropertyInteger(
    __in IAppHostElement *pElement,
    __in LPCWSTR wszPropName,
    __in DWORD dValue
    )
{
    VARIANT vtPut;
    VariantInit(&vtPut);
    vtPut.vt = VT_I4;
    vtPut.lVal = dValue;
    return Iis7PutPropertyVariant(pElement, wszPropName, vtPut);
}

extern "C" HRESULT DAPI Iis7PutPropertyBool(
    __in IAppHostElement *pElement,
    __in LPCWSTR wszPropName,
    __in BOOL fValue)
{
    VARIANT vtPut;
    VariantInit(&vtPut);
    vtPut.vt = VT_BOOL;
    vtPut.boolVal = (fValue == FALSE) ? VARIANT_FALSE : VARIANT_TRUE;
    return Iis7PutPropertyVariant(pElement, wszPropName, vtPut);
}

extern "C" HRESULT DAPI Iis7GetPropertyVariant(
    __in IAppHostElement *pElement,
    __in LPCWSTR wszPropName,
    __in VARIANT* vtGet
    )
{
    HRESULT hr = S_OK;
    IAppHostProperty *pProperty = NULL;
    BSTR bstrPropName = NULL;

    bstrPropName = ::SysAllocString(wszPropName);
    ExitOnNull(bstrPropName, hr, E_OUTOFMEMORY, "failed SysAllocString");

    hr = pElement->GetPropertyByName(bstrPropName, &pProperty);
    ExitOnFailure1(hr, "Failed to get property object for %S", wszPropName);

    hr = pProperty->get_Value(vtGet);
    ExitOnFailure1(hr, "Failed to get property value for %S", wszPropName);
LExit:
    ReleaseNullBSTR(bstrPropName);
    // caller responsible vor cleaning up variant vtGet
    ReleaseNullObject(pProperty);
    return hr;
}

extern "C" HRESULT DAPI Iis7GetPropertyString(
    __in IAppHostElement *pElement,
    __in LPCWSTR wszPropName,
    __in LPWSTR* pwzGet
    )
{
    HRESULT hr = S_OK;
    VARIANT vtGet;

    VariantInit(&vtGet);
    hr = Iis7GetPropertyVariant(pElement, wszPropName, &vtGet);
    if (SUCCEEDED(hr))
    {
        Assert(VT_BSTR == vtGet.vt);
        hr = StrAllocString(pwzGet, vtGet.bstrVal, 0);
    }
    VariantClear(&vtGet);
    return hr;
}

BOOL CompareVariantDefault(
    __in VARIANT* pVariant1,
    __in VARIANT* pVariant2
    )
{
    BOOL fEqual = FALSE;
    ULONGLONG ull1 = 0;
    ULONGLONG ull2 = 0;
    switch(pVariant1->vt)
    {
        // VarCmp doesn't work for unsigned ints
        // We'd like to allow signed/unsigned comparison as well since
        // IIS doesn't document variant type for integer fields
    case VT_I1:
    case VT_UI1:
        if (VT_I1 == pVariant2->vt || VT_UI1 == pVariant2->vt)
        {
            fEqual = pVariant1->bVal == pVariant2->bVal;
        }
        break;
    case VT_I2:
    case VT_UI2:
        if (VT_I2 == pVariant2->vt || VT_UI2 == pVariant2->vt)
        {
            fEqual = pVariant1->uiVal == pVariant2->uiVal;
        }
        break;
    case VT_UI4:
    case VT_I4:
        if (VT_I4 == pVariant2->vt || VT_UI4 == pVariant2->vt)
        {
            fEqual = pVariant1->ulVal == pVariant2->ulVal;
        }
        break;
    case VT_UI8:
    case VT_I8:
        if (VT_I8 == pVariant2->vt || VT_UI8 == pVariant2->vt)
        {
            fEqual = pVariant1->ullVal == pVariant2->ullVal;
        }
        break;
    default:
        fEqual = VARCMP_EQ == ::VarCmp(pVariant1,
                                       pVariant2,
                                       LOCALE_NEUTRAL,
                                       NORM_IGNORECASE);
    }
    return fEqual;
}

BOOL DAPI IsMatchingAppHostElementCallback(
    __in IAppHostElement *pElement,
    __in LPVOID pContext
    )
{
    IIS7_APPHOSTELEMENTCOMPARISON* pComparison = (IIS7_APPHOSTELEMENTCOMPARISON*) pContext;
    return Iis7IsMatchingAppHostElement(pElement, pComparison);
}

extern "C" BOOL DAPI Iis7IsMatchingAppHostElement(
    __in IAppHostElement *pElement,
    __in IIS7_APPHOSTELEMENTCOMPARISON* pComparison
    )
{
    HRESULT hr = S_OK;
    BOOL fResult = FALSE;
    IAppHostProperty *pProperty = NULL;
    BSTR bstrElementName = NULL;

    VARIANT vPropValue;
    VariantInit(&vPropValue);

    hr = pElement->get_Name(&bstrElementName);
    ExitOnFailure(hr, "Failed to get name of element");
    if (CSTR_EQUAL != ::CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, pComparison->pwzElementName, -1, bstrElementName, -1))
    {
        ExitFunction();
    }

    hr = Iis7GetPropertyVariant(pElement, pComparison->pwzAttributeName, &vPropValue);
    ExitOnFailure2(hr, "Failed to get value of %S attribute of %S element", pComparison->pwzAttributeName, pComparison->pwzElementName);

    if (TRUE == CompareVariantDefault(pComparison->pvAttributeValue, &vPropValue))
    {
        fResult = TRUE;
    }
LExit:
    ReleaseNullBSTR(bstrElementName);
    VariantClear(&vPropValue);
    ReleaseNullObject(pProperty);
    return fResult;
}

BOOL DAPI IsMatchingAppHostMethod(
    __in IAppHostMethod *pMethod,
    __in LPCWSTR pwzMethodName
   )
{
    HRESULT hr = S_OK;
    BOOL fResult = FALSE;
    BSTR bstrName = NULL;

    hr = pMethod->get_Name(&bstrName);
    ExitOnFailure(hr, "Failed to get name of element");
    if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, pwzMethodName, -1, bstrName, -1))
    {
        fResult = TRUE;
    }
LExit:
    ReleaseNullBSTR(bstrName);
    return fResult;
}

extern "C" HRESULT DAPI Iis7FindAppHostElementString(
    __in IAppHostElementCollection *pCollection,
    __in LPCWSTR pwzElementName,
    __in LPCWSTR pwzAttributeName,
    __in LPCWSTR pwzAttributeValue,
    __out IAppHostElement** ppElement,
    __out DWORD* pdwIndex
    )
{
    HRESULT hr = S_OK;
    VARIANT vtValue;
    VariantInit(&vtValue);

    vtValue.vt = VT_BSTR;
    vtValue.bstrVal = ::SysAllocString(pwzAttributeValue);

    hr = Iis7FindAppHostElementVariant(pCollection,
                                       pwzElementName,
                                       pwzAttributeName,
                                       &vtValue,
                                       ppElement,
                                       pdwIndex);

    VariantClear(&vtValue);
    return hr;
}

extern "C" HRESULT DAPI Iis7FindAppHostElementInteger(
    __in IAppHostElementCollection *pCollection,
    __in LPCWSTR pwzElementName,
    __in LPCWSTR pwzAttributeName,
    __in DWORD dwAttributeValue,
    __out IAppHostElement** ppElement,
    __out DWORD* pdwIndex
    )
{
    HRESULT hr = S_OK;
    VARIANT vtValue;
    VariantInit(&vtValue);

    vtValue.vt = VT_UI4;
    vtValue.ulVal = dwAttributeValue;

    hr = Iis7FindAppHostElementVariant(pCollection,
                                       pwzElementName,
                                       pwzAttributeName,
                                       &vtValue,
                                       ppElement,
                                       pdwIndex);

    VariantClear(&vtValue);
    return hr;
}

extern "C" HRESULT DAPI Iis7FindAppHostElementVariant(
    __in IAppHostElementCollection *pCollection,
    __in LPCWSTR pwzElementName,
    __in LPCWSTR pwzAttributeName,
    __in VARIANT* pvAttributeValue,
    __out IAppHostElement** ppElement,
    __out DWORD* pdwIndex
    )
{
    IIS7_APPHOSTELEMENTCOMPARISON comparison = { };
    comparison.pwzElementName = pwzElementName;
    comparison.pwzAttributeName = pwzAttributeName;
    comparison.pvAttributeValue = pvAttributeValue;

    return Iis7EnumAppHostElements(pCollection,
                                   IsMatchingAppHostElementCallback,
                                   &comparison,
                                   ppElement,
                                   pdwIndex);
}

extern "C" HRESULT DAPI Iis7EnumAppHostElements(
    __in IAppHostElementCollection *pCollection,
    __in ENUMAPHOSTELEMENTPROC pCallback,
    __in LPVOID pContext,
    __out IAppHostElement** ppElement,
    __out DWORD* pdwIndex
    )
{
    HRESULT hr = S_OK;
    IAppHostElement *pElement = NULL;
    DWORD dwElements = 0;

    VARIANT vtIndex;
    VariantInit(&vtIndex);

    if (NULL != ppElement)
    {
        *ppElement = NULL;
    }
    if (NULL != pdwIndex)
    {
        *pdwIndex = MAXDWORD;
    }

    hr = pCollection->get_Count(&dwElements);
    ExitOnFailure(hr, "Failed get application IAppHostElementCollection count");

    vtIndex.vt = VT_UI4;
    for( DWORD i = 0; i < dwElements; i++ )
    {
        vtIndex.ulVal = i;
        hr = pCollection->get_Item(vtIndex , &pElement);
        ExitOnFailure(hr, "Failed get IAppHostElement element");

        if (pCallback(pElement, pContext))
        {
            if (NULL != ppElement)
            {
                *ppElement = pElement;
                pElement = NULL;
            }
            if (NULL != pdwIndex)
            {
                *pdwIndex = i;
            }
            break;
        }

        ReleaseNullObject(pElement);
    }
LExit:
    ReleaseNullObject(pElement);
    VariantClear(&vtIndex);

    return hr;
}

extern "C" HRESULT DAPI Iis7FindAppHostMethod(
    __in IAppHostMethodCollection *pCollection,
    __in LPCWSTR pwzMethodName,
    __out IAppHostMethod** ppMethod,
    __out DWORD* pdwIndex
    )
{
    HRESULT hr = S_OK;
    IAppHostMethod *pMethod = NULL;
    DWORD dwMethods = 0;

    VARIANT vtIndex;
    VariantInit(&vtIndex);

    if (NULL != ppMethod)
    {
        *ppMethod = NULL;
    }
    if (NULL != pdwIndex)
    {
        *pdwIndex = MAXDWORD;
    }

    hr = pCollection->get_Count(&dwMethods);
    ExitOnFailure(hr, "Failed get application IAppHostMethodCollection count");

    vtIndex.vt = VT_UI4;
    for( DWORD i = 0; i < dwMethods; i++ )
    {
        vtIndex.ulVal = i;
        hr = pCollection->get_Item(vtIndex , &pMethod);
        ExitOnFailure(hr, "Failed get IAppHostMethod element");

        if (IsMatchingAppHostMethod(pMethod, pwzMethodName))
        {
            if (NULL != ppMethod)
            {
                *ppMethod = pMethod;
                pMethod = NULL;
            }
            if (NULL != pdwIndex)
            {
                *pdwIndex = i;
            }
            break;
        }

        ReleaseNullObject(pMethod);
    }
LExit:
    ReleaseNullObject(pMethod);
    VariantClear(&vtIndex);

    return hr;
}
