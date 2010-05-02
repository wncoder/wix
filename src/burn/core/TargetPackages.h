//-------------------------------------------------------------------------------------------------
// <copyright file="TargetPackages.h" company="Microsoft">
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

#include "interfaces\IApplicablePackages.h"
#include "Coordinator.h"
#include "MsiXmlBlobBase.h"

namespace IronMan
{
template <typename MsiInstallContext>
class TargetPackagesT : public IApplicablePackages, private MsiXmlBlobBaseT<MsiInstallContext>
{
    CSimpleArray<CString> m_targetNames;
    bool m_evaluatedTargetPackages;
    const IInstallItems& m_items;
    const bool* m_pbStopProcessing;
public:
    TargetPackagesT(const IInstallItems& items) 
        : m_items(items)
        , m_evaluatedTargetPackages(false)
        , m_pbStopProcessing(NULL)
    { }


    // Stop evaluatingboolean flag is passed to the lengty operation, which periodically
    // checks this flag and exits early if set.
    // Returns true when all the packages have been evaluated and stopEvaluating flag was not turned o.
    // Returns false when stopEvaluating flag is set to true.
    bool EvaluateTargetPackages()
    {
        for(unsigned int i=0, j=m_items.GetCount(); i<j; ++i)
        {
            const ItemBase* pItem = m_items.GetItem(i);
            if (!ShouldStopProcessing() && pItem) // silence (faulty) PREfast warning (6011)
            {
                // either a Patches or an MSP
                switch(pItem->GetType())
                {
                default:
                    break; // File, Service control etc, should not appear in the "welcome page" text
                case ItemBase::Patches:
                {
                    // Each MSP contains a union of MsiPatch elements from all of the MSPs in the MsiXmlBlob
                    // so that only one MSP(MsiXmlBlob) needs to be used to determine the Products the Patches item applies to
                    const Patches* patches = static_cast<const Patches*>(pItem);
                    if ( !ShouldStopProcessing() && patches->GetCount() > 0 )
                    {
                        AddMspTargetName(m_targetNames, patches->GetMsp(0));
                    }
                    break;
                }
                case ItemBase::Msp:
                    AddMspTargetName(m_targetNames, *static_cast<const MSP*>(pItem));
                    break;        
                case ItemBase::CleanupBlockType:
                    {
                        const CleanupBlock* cub = static_cast<const CleanupBlock*>(pItem);
                        CSimpleArray<CString> affectedProducts = cub->GetAffectedProducts();
                        for (int i = 0; i < affectedProducts.GetSize(); ++i)
                        {
                            OnApplicableProduct(CComBSTR(affectedProducts[i]));
                        }
                        break;
                    }
                case ItemBase::RelatedProductsType:
                    {
                        const RelatedProducts* rp = static_cast<const RelatedProducts*>(pItem);
                        for (int i = 0; i < rp->GetRelatedProducts().GetSize(); i++)
                        {
                            OnApplicableProduct(CComBSTR(CString(rp->GetRelatedProducts()[i])));
                        }
                        break;
                    }
                case ItemBase::Exe:
                     //It is intentional that there is no break here, since we want to fall through to add the canonical name to the targetNames
                case ItemBase::AgileMsi:
                     //It is intentional that there is no break here, since we want to fall through to add the canonical name to the targetNames
                case ItemBase::Msi:                    
                    const CanonicalTargetName* name = dynamic_cast<const CanonicalTargetName*>(pItem);
                    if (name) // Exe or MSI or ServiceControl
                    {
                        if (!name->GetCanonicalTargetName().IsEmpty()) // If Canonical name is empty, don't add blank string target names
                        {
                            if (m_targetNames.Find(name->GetCanonicalTargetName()) == -1)
                                m_targetNames.Add (name->GetCanonicalTargetName());
                        }
                    }
                    break;                
                }
            }	
        }
        return !ShouldStopProcessing();
    }
    virtual ~TargetPackagesT() {}
    virtual CSimpleArray<CString> GetTargetPackagesNames(const bool &bStopProcessing)
    {
        if (!m_evaluatedTargetPackages)
        {
            m_pbStopProcessing = &bStopProcessing;
            m_evaluatedTargetPackages = EvaluateTargetPackages();
            m_pbStopProcessing = NULL;
        }
        return m_targetNames;
    }
private:
    // Test Hook
    virtual bool ShouldStopProcessing()
    {
        if (m_pbStopProcessing)
        {
            return *m_pbStopProcessing;
        }
        return false;
    }

    void AddMspTargetName(CSimpleArray<CString>& targetNames, const MSP& msp)
    {
        CComBSTR bigBlob = msp.GetBlob(); // there may be multiple MsiPatch nodes

        CoInitializer ci; // turn COM back on

        CComPtr<IXMLDOMDocument2> spDoc;
        HRESULT hr = spDoc.CoCreateInstance(__uuidof(DOMDocument30)); // this also works:  spDoc.CoCreateInstance(_T("Msxml2.DOMDocument.3.0"));
        if (SUCCEEDED(hr))
        {
            VARIANT_BOOL vb = VARIANT_FALSE;
            hr = spDoc->loadXML(bigBlob, &vb);
            if ((S_OK == hr) && // S_FALSE is a failure
                (vb == VARIANT_TRUE))
            {
                CComPtr<IXMLDOMElement> spRoot;
                hr = spDoc->get_documentElement(&spRoot);
                if (SUCCEEDED(hr))
                {	// create an XPath query that gets MsiXmlBlob elements from the blob
                    CComPtr<IXMLDOMNodeList> spNodeList;
                    hr = spRoot->selectNodes(CComBSTR(L".//MsiXmlBlob"), &spNodeList);
                    if (SUCCEEDED(hr))
                    {
                        long length = 0;
                        spNodeList->get_length(&length);
                        for(long l=0; l<length; ++l)
                        {
                            CComPtr<IXMLDOMNode> node;
                            if (SUCCEEDED(spNodeList->get_item(l, &node)))
                            {
                                CComBSTR blob;
                                hr = node->get_xml(&blob);
                                if (SUCCEEDED(hr))
                                {
                                    EnumerateApplicableProducts(blob);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
private: // MsiXmlBlobBase
    virtual bool OnApplicableProduct(const CComBSTR& productCode)
    {
        WCHAR wszTargetPackageName[MAX_PATH] = { 0 };
        DWORD cchTargetPackageInstallPath = MAX_PATH;				
        UINT err = MsiGetProductInfo(productCode, INSTALLPROPERTY_PRODUCTNAME, wszTargetPackageName, &cchTargetPackageInstallPath);
        if ((err == ERROR_SUCCESS) && (cchTargetPackageInstallPath != 0))
        {
            if (m_targetNames.Find(wszTargetPackageName) == -1)
                m_targetNames.Add (wszTargetPackageName);
        }
        return true;
    }

    // "subclass and override" test hook
    virtual UINT MsiGetProductInfo(__in LPCTSTR szProduct, __in LPCTSTR szAttribute, __out_ecount_opt(*pcchValueBuf) LPTSTR lpValueBuf, __inout_opt LPDWORD pcchValueBuf)
    {
        return ::MsiGetProductInfo(szProduct, szAttribute, lpValueBuf, pcchValueBuf);
    }};
typedef TargetPackagesT<CMsiInstallContext> TargetPackages;
}
