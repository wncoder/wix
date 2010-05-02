//-------------------------------------------------------------------------------------------------
// <copyright file="Feature.h" company="Microsoft">
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
//    Not Implemented.
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once

#ifdef FeaturesAreImplented
    // Features feature not implemented

#include "Interfaces\IFeature.h"
#include "..\common\XmlUtils.h"

namespace IronMan
{

class FeatureTreeNode : public IFeature
{
    ILogger& m_logger;
    CSimpleArray<FeatureTreeNode> m_children;
    const CString m_textId;
    const CString m_descId;
    bool m_selected;

    static bool IsTrue(const CString& b, ILogger& logger)
    {
        if (b.CompareNoCase(L"true") == 0)
            return true;
        if (b.CompareNoCase(L"false") == 0)
            return false;

        CInvalidXmlException ixe(L"schema validation failure:  required attribute 'DefaultSelectionState' must be either true or false");
        LOG(logger, ILogger::Error, ixe.GetMessage());
        throw ixe;
    }

public:
    FeatureTreeNode(CComPtr<IXMLDOMElement>& spElement, ILogger& logger)
        : m_logger(logger)
        , m_textId(ElementUtils::GetAttributeByName(spElement, L"ID", logger, true))
        , m_descId (ElementUtils::GetAttributeByName(spElement, L"Description", logger))
        , m_selected(IsTrue(ElementUtils::GetAttributeByName(spElement, L"DefaultSelectionState", logger), logger))
    {
        PopulateChildren(spElement, logger, m_children, L"Feature");
    }
    FeatureTreeNode(const FeatureTreeNode& rhs)
        : m_logger(rhs.m_logger)
        , m_textId(rhs.m_textId)
        , m_descId (rhs.m_descId)
        , m_selected(rhs.m_selected)
        , m_children(rhs.m_children)
    {}
    FeatureTreeNode(IFeature* feature, ILogger& logger)
        : m_logger(logger)
        , m_textId(feature->GetTextId())
        , m_descId (feature->GetDescriptionId())
        , m_selected(feature->GetDefaultSelectionState())
    {
        for(unsigned int i=0; i<feature->GetNumberOfChildren(); ++i)
        {
            m_children.Add(FeatureTreeNode(feature->GetChild(i), logger));
        }
    }
    virtual ~FeatureTreeNode() {}

public: // utility functions for use with this class and Features, below
    static void PopulateChildren(CComPtr<IXMLDOMElement>& spElement, ILogger& logger, CSimpleArray<FeatureTreeNode>& children, const CString& elementName)
    {
        ElementUtils::VerifyName(spElement, elementName, logger);

        CComPtr<IXMLDOMNode> spChild;
        HRESULT hr = spElement->get_firstChild(&spChild);
        if (S_OK == hr)
        {
            children.Add(FeatureTreeNode(CComQIPtr<IXMLDOMElement>(spChild), logger));

            do {
                CComPtr<IXMLDOMNode> spSibling;
                hr = spChild->get_nextSibling(&spSibling);
                if (S_OK == hr)
                {
                    children.Add(FeatureTreeNode(CComQIPtr<IXMLDOMElement>(spSibling), logger));
                }
                spChild = spSibling;
            } while (!!spChild);
        }
    }
    static IFeature* GetChild(CSimpleArray<FeatureTreeNode>& children, unsigned int index, ILogger& logger)
    {
        if (index >= static_cast<unsigned int>(children.GetSize()))
        {
            COutOfBoundsException obe(L"Feature index");
            LOG(logger, ILogger::Error, obe.GetMessage());
            throw obe;
        }
        return &children[index];
    }

public: // IFeature
    virtual const CString& GetTextId() { return m_textId; }
    virtual const CString& GetDescriptionId() { return m_descId; }
    virtual bool GetDefaultSelectionState() { return m_selected; }
    virtual void SetSelectionState(bool b) { m_selected = b; }

public: // ITreeT<IFeature>
    virtual unsigned int GetNumberOfChildren() { return m_children.GetSize(); }
    virtual IFeature* GetChild(unsigned int index)
    {
        return GetChild(m_children, index, m_logger);
    }
};

class Features : public IFeatureTreeRoot
{
    ILogger& m_logger;
    CSimpleArray<FeatureTreeNode> m_features;
public:
    Features(CComPtr<IXMLDOMElement>& spElement, ILogger& logger)
        : m_logger(logger)
    {
        if (spElement == NULL)
            return; // "Features" element is optional

        FeatureTreeNode::PopulateChildren(spElement, logger, m_features, L"Features");
        if (m_features.GetSize() == 0)
        {
            CInvalidXmlException ixe(L"schema validation failure:  there must be at least one 'Feature' child element in 'Features'");
            LOG(logger, ILogger::Error, ixe.GetMessage());
            throw ixe;
        }
    }
    Features(const Features& rhs)
        : m_logger(rhs.m_logger)
        , m_features(rhs.m_features)
    {}
    Features(ILogger& logger) : m_logger(logger) {}
    virtual ~Features() {}

    void RemoveAll() { m_features.RemoveAll(); }
    void Add(IFeature* feature)
    {
        m_features.Add(FeatureTreeNode(feature, m_logger));
    }

public: // ITreeT<IFeature>
    virtual unsigned int GetNumberOfChildren() { return m_features.GetSize(); }
    virtual IFeature* GetChild(unsigned int index)
    {
        return FeatureTreeNode::GetChild(m_features, index, m_logger);
    }
};

}
#endif // FeaturesAreImplented
