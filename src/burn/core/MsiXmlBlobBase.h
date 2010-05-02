//-------------------------------------------------------------------------------------------------
// <copyright file="MsiXmlBlobBase.h" company="Microsoft">
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

#include "common\CoInitializer.h"
#include "common\MSIUtils.h"
#include "MsiInstallContext.h"
#include "MsiPatchSequenceInfoArray.h"

namespace IronMan
{

//------------------------------------------------------------------------------
// MsiXmlBlobBaseT class
//
// Used to find Products that the patches apply to
//-------------------------------------------------------------------------------
template <typename MsiInstallContext>
class MsiXmlBlobBaseT
{
protected:
    virtual ~MsiXmlBlobBaseT() {}

    //------------------------------------------------------------------------------
    // EnumerateApplicableProducts
    //
    // class that derives from this class must implement OnApplicableProduct()
    // EnumerateApplicableProducts() call OnApplicableProduct with the Product
    // Code of a Product the the patch described by the blob applies to.
    //-------------------------------------------------------------------------------
    bool EnumerateApplicableProducts(const CComBSTR& blob) // true if any applicable product was found
    {
        bool applicableProductFound = false;

        CoInitializer ci; // turn COM on

        CComPtr<IXMLDOMDocument2> spDoc;
        HRESULT hr = spDoc.CoCreateInstance(__uuidof(DOMDocument30)); // this also works:  spDoc.CoCreateInstance(_T("Msxml2.DOMDocument.3.0"));
        if (SUCCEEDED(hr))
        {
            VARIANT_BOOL vb = VARIANT_FALSE;
            hr = spDoc->loadXML(blob, &vb);
            if ((S_OK == hr) && // S_FALSE is a failure
                (vb == VARIANT_TRUE))
            {
                CComPtr<IXMLDOMElement> spRoot;
                hr = spDoc->get_documentElement(&spRoot);
                if (SUCCEEDED(hr))
                {   // create an XPath query that gets TargetProductCode elements from the blob
                    CComPtr<IXMLDOMNodeList> spNodeList;
                    hr = spRoot->selectNodes(CComBSTR(L"//MsiXmlBlob/MsiPatch/TargetProductCode"), &spNodeList);
                    if (SUCCEEDED(hr))
                    {
                        MsiPatchSequenceInfoArray psiArray(spRoot);

                        long length = 0;
                        spNodeList->get_length(&length);
                        for(long l=0; l<length; ++l)
                        {
                            CComPtr<IXMLDOMNode> node;
                            if (SUCCEEDED(spNodeList->get_item(l, &node)))
                            {
                                CComBSTR bstrTargetProductCode;
                                hr = node->get_text(&bstrTargetProductCode);
                                if (SUCCEEDED(hr))
                                {
                                    if (true == HasAtleastOneApplicablePatch(psiArray, bstrTargetProductCode))
                                    {
                                        applicableProductFound = true;
                                        if (OnApplicableProduct(bstrTargetProductCode) == false)
                                            return true;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        return applicableProductFound;
    }
private:
    virtual bool OnApplicableProduct(const CComBSTR& productCode) = 0; // return true to keep enumerating

private:
    bool HasAtleastOneApplicablePatch(MsiPatchSequenceInfoArray& psiArray, const CComBSTR& productCode)
    {
        MSIINSTALLCONTEXT context = MSIINSTALLCONTEXT_NONE;
        DWORD result = MsiInstallContext(CString(productCode)).GetContext(context);
        if (result != ERROR_SUCCESS)
        {
            return false;
        }
        
        result = this->MsiDeterminePatchSequence(productCode, NULL, context, psiArray.Size(), psiArray);
        if (result == ERROR_SUCCESS)
        {
            return psiArray.HasAtleastOneApplicablePatch();
        }

        return false;
    }

private: // test hook
    virtual UINT MsiDeterminePatchSequence(__in LPCWSTR szProductCode, __in_opt LPCWSTR szUserSid, __in MSIINSTALLCONTEXT dwContext, __in DWORD cPatchInfo, __inout_ecount(cPatchInfo) PMSIPATCHSEQUENCEINFO pPatchInfo)
    {
        return ::MsiDeterminePatchSequence(szProductCode, szUserSid, dwContext, cPatchInfo, pPatchInfo);
    }
};

typedef MsiXmlBlobBaseT<CMsiInstallContext> MsiXmlBlobBase;

}
