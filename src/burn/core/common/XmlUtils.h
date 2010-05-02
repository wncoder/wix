//-------------------------------------------------------------------------------------------------
// <copyright file="XmlUtils.h" company="Microsoft">
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
#include "interfaces\IExceptions.h"
#include "interfaces\ILocalizedData.h"
#include "common\CoInitializer.h"
#include "LogSignatureDecorator.h"

namespace IronMan
{
struct ElementUtils
{

private:
    template<typename T>
    static CString GetNameT(const CComPtr<T>& spNode)
    {
        CComBSTR name;
        spNode->get_nodeName(&name);
        return CString(name);
    }
    
    template<typename T> 
    static CComPtr<IXMLDOMNode> GetFirstChildT(const CComPtr<T>& spNode) 
    {
        CComPtr<IXMLDOMNode> spSubNode;
        HRESULT hr = spNode->get_firstChild(&spSubNode);
        if (FAILED(hr) || !spSubNode)
        {
            return NULL;
        }
        return spSubNode;
    }


public:
    static CString GetName(const CComPtr<IXMLDOMElement>& spElement)
    {
        return GetNameT<IXMLDOMElement>(spElement);
    }

    static CString GetName(const CComPtr<IXMLDOMNode>& spElement)
    {
        return GetNameT<IXMLDOMNode>(spElement);
    }

    static void VerifyName(const CComPtr<IXMLDOMElement> spElement, const CString& elementName, ILogger& logger)
    {
        if (elementName.Compare(GetName(spElement)))
        {
            CInvalidXmlException ixe(L"schema validation error:  element name is wrong: " + elementName,L"");
            LOG(logger, ILogger::Error, ixe.GetMessage());
            throw ixe;
        }
    }

    static const CComPtr<IXMLDOMElement> GetParentElement(const CComPtr<IXMLDOMElement>& spElement)
    {
        CComPtr<IXMLDOMNode> spParentNode;
        HRESULT hr = spElement->get_parentNode(&spParentNode);
        if (!SUCCEEDED(hr))
        {
            CInvalidXmlException ixe(L"schema validation error:  cannot get the parent element.",L"");
            throw ixe;
        }

        return CComQIPtr<IXMLDOMElement>(spParentNode);
    }

    static const CComPtr<IXMLDOMElement> GetRootElement(const CComPtr<IXMLDOMElement>& spElement)
    {
        CComPtr<IXMLDOMNode> spCurrentNode;
        CComPtr<IXMLDOMNode> spPreviousNode;
        spCurrentNode = spElement;
        HRESULT hr = S_OK;
        do {
            CComPtr<IXMLDOMNode> spParentNode;
            hr = spCurrentNode->get_parentNode(&spParentNode);
            if (S_OK == hr)
            {
                spPreviousNode = spCurrentNode;
                spCurrentNode = spParentNode;
            }
        } while (S_OK == hr);
        return CComQIPtr<IXMLDOMElement>(spPreviousNode);
    }

public:
    static CComPtr<IXMLDOMNode> GetFirstChild(const CComPtr<IXMLDOMNode>& spNode)
    {
        return GetFirstChildT<IXMLDOMNode>(spNode);
    }
    static CComPtr<IXMLDOMNode> GetFirstChild(const CComPtr<IXMLDOMElement>& spElement)
    {
        return GetFirstChildT<IXMLDOMElement>(spElement);
    }
    
    static CComPtr<IXMLDOMNode> GetNextSibling(const CComPtr<IXMLDOMNode>& spNode)
    {
        CComPtr<IXMLDOMNode> spNextNode;
        spNode->get_nextSibling(&spNextNode);
        return spNextNode;
    }

    static const CComPtr<IXMLDOMElement> FindChildElementByNumber(const CComPtr<IXMLDOMElement>& spElement, int number, ILogger& logger)
    {
        int count = 0;
        CComPtr<IXMLDOMNode> spChild;
        HRESULT hr = spElement->get_firstChild(&spChild);
        if (S_OK == hr)
        {
            if (count == number)
                return CComQIPtr<IXMLDOMElement>(spChild);

            do {
                CComPtr<IXMLDOMNode> spSibling;
                hr = spChild->get_nextSibling(&spSibling);
                if (S_OK == hr)
                {
                    if (++count == number)
                        return CComQIPtr<IXMLDOMElement>(spSibling);
                }
                spChild = spSibling;
            } while (!!spChild);
        }
        CString format;
        format.Format(L"schema validation failure:  child element #%i not found", number);
        CInvalidXmlException ixe(format,L"");
        LOG(logger, ILogger::Error, ixe.GetMessage());
        throw ixe;
    }

    static CComPtr<IXMLDOMElement> FindChildElementByName(const CComPtr<IXMLDOMElement> spElement, const CString& elementName, ILogger& logger)
    {
        CComPtr<IXMLDOMNode> spChild;
        HRESULT hr = spElement->get_firstChild(&spChild);
        if (S_OK == hr)
        {
            if (0 == elementName.Compare(GetName(spChild)))
                return CComQIPtr<IXMLDOMElement>(spChild);

            do {
                CComPtr<IXMLDOMNode> spSibling;
                hr = spChild->get_nextSibling(&spSibling);
                if (S_OK == hr)
                {
                    if (0 == elementName.Compare(GetName(spSibling)))
                        return CComQIPtr<IXMLDOMElement>(spSibling);
                }
                spChild = spSibling;
            } while (!!spChild);
        }

        CInvalidXmlException ixe(L"schema validation failure:  child element not found - " + elementName,L"");
        LOG(logger, ILogger::Error, ixe.GetMessage());
        throw ixe;
    }

    static CComPtr<IXMLDOMNodeList> GetChildElementsByName(const CComPtr<IXMLDOMElement> spElement, const CString& elementName, long &itemCount, ILogger& logger)
    {
        CComPtr<IXMLDOMNodeList> spChildren;
        CComPtr<IXMLDOMNode> spChild;
        itemCount = 0;

        HRESULT hr = spElement->getElementsByTagName(CComBSTR(elementName), &spChildren);
        if (S_OK == hr)
        {
            hr = spChildren->get_length(&itemCount);
            if (S_OK == hr && itemCount > 0)
            {
                return CComQIPtr<IXMLDOMNodeList>(spChildren);
            }
        }

        CInvalidXmlException ixe(L"schema validation failure:  child element not found - " + elementName,L"");
        LOG(logger, ILogger::Error, ixe.GetMessage());
        throw ixe;
    }

    static bool ContainsInvalidChildElementByName(const CComPtr<IXMLDOMElement> spElement, const CString& elementName, ILogger& logger)
    {
        CComPtr<IXMLDOMNodeList> spChildren;
        long itemCount = 0;

        HRESULT hr = spElement->getElementsByTagName(CComBSTR(elementName), &spChildren);
        if (S_OK == hr)
        {
            hr = spChildren->get_length(&itemCount);
            if (S_OK == hr && itemCount > 0)
            {
                // Element contains invalid child element
                return true;
            }
        }
        {
            // Invalid element elementName is not a child of this element
            return false;
        }
    }

    // Returns true if an attribute with given name is present
    static bool ContainsAttribute(const CComPtr<IXMLDOMElement> spElement, const CString& attributeName)
    {
        if (!spElement)
            return false;
        CComVariant cv;
        if (S_OK == spElement->getAttribute(CComBSTR(attributeName), &cv))
            return true; // Contains the given attribute 
        return false;
    }

    static unsigned int CountChildElements(const CComPtr<IXMLDOMElement> spElement)
    {
        unsigned int count = 0;

        CComPtr<IXMLDOMNode> spChild;
        HRESULT hr = spElement->get_firstChild(&spChild);
        if (S_OK == hr)
        {
            ++count;

            do {
                CComPtr<IXMLDOMNode> spSibling;
                hr = spChild->get_nextSibling(&spSibling);
                if (S_OK == hr)
                    ++count;
                spChild = spSibling;
            } while (!!spChild);
        }
        return count;
    }

    static unsigned int CountChildElements(const CComPtr<IXMLDOMElement> spElement, CString csElementName)
    {
        unsigned int count = 0;
        if (!spElement)
            return 0;

        CComPtr<IXMLDOMNode> spChild;
        HRESULT hr = spElement->get_firstChild(&spChild);
        if (S_OK == hr)
        {
            if (GetName(spChild) == csElementName)
            {
                ++count;
            }

            do {
                CComPtr<IXMLDOMNode> spSibling;
                hr = spChild->get_nextSibling(&spSibling);
                if ((S_OK == hr) && (GetName(spSibling) == csElementName))
                {
                    ++count;
                }
                spChild = spSibling;
            } while (!!spChild);
        }
        return count;
    }

    static CComPtr<IXMLDOMElement> FindOptionalChildElementByName(const CComPtr<IXMLDOMElement> spElement, const CString& elementName, ILogger& logger)
    {
        CComPtr<IXMLDOMNode> spChild;

        if (NULL == spElement)
        {
            return NULL;
        }

        HRESULT hr = spElement->get_firstChild(&spChild);
        if (S_OK == hr)
        {
            CString nameOfNextElement(GetName(spChild));
            if (0 == elementName.Compare(nameOfNextElement))
            {
                return CComQIPtr<IXMLDOMElement>(spChild);
            }

            do {
                CComPtr<IXMLDOMNode> spSibling;
                hr = spChild->get_nextSibling(&spSibling);
                if (S_OK == hr)
                {
                    if (0 == elementName.Compare(GetName(spSibling)))
                    {
                        return CComQIPtr<IXMLDOMElement>(spSibling);
                    }
                }
                spChild = spSibling;
            } while (!!spChild);
        }

        // OK not to find optional child element, return empty pointer
        return CComQIPtr<IXMLDOMElement>(spChild);
    }

    static CString GetAttributeByName(const CComPtr<IXMLDOMElement> spElement, const CString& name, ILogger& logger, bool log = true)
    {
        CComVariant cv;
        HRESULT hr = spElement->getAttribute(CComBSTR(name), &cv);
        if ((hr == S_OK) && (cv.vt == VT_BSTR))
            return V_BSTR(&cv);

        CInvalidXmlException ixe(L"schema validation error:  attribute not found - " + name,L"");
        if (log) // will be false, if optional; missing optional attributes is not an error
            LOG(logger, ILogger::Error, ixe.GetMessage());
        throw ixe;
    }

    static CString GetOptionalAttributeByName(const CComPtr<IXMLDOMElement> spElement, const CString& name, ILogger& logger, bool log = true)
    {
        CComVariant cv;
        HRESULT hr = spElement->getAttribute(CComBSTR(name), &cv);
        if ((hr == S_OK) && (cv.vt == VT_BSTR))
        {
            return V_BSTR(&cv);
        }
        else if (log)
        {
            // LOG(logger, ILogger::Information, L"Optional attribute was not specified - " + name);
        }
        
        return CString(L"");
    }

    static CString GetOptionalAttributeByNameDefaultToParent(const CComPtr<IXMLDOMElement> spElement, const CString& name, ILogger& logger)
    {
        try
        {
            return GetAttributeByName(spElement, name, logger, false);
        }
        catch(... /*const CInvalidXmlException& */ )
        {   // didn't find it here; try parent
            CComPtr<IXMLDOMNode> spNode;
            HRESULT hr = spElement->get_parentNode(&spNode);
            if (SUCCEEDED(hr))
            {
                CComQIPtr<IXMLDOMElement> spParent(spNode);
                if (spParent)
                    return GetAttributeByName(spParent, name, logger);
            }
            throw; // parent node is not an Element???  Just re-throw.
        }
    }

    static UINT GetAttributeIntByName(const CComPtr<IXMLDOMElement> spElement, const CString& name, ILogger& logger, bool log = true)
    {
        CComVariant cv;
        HRESULT hr = spElement->getAttribute(CComBSTR(name), &cv);
        if ((hr == S_OK) && (cv.vt == VT_BSTR))
        {
            UINT uint = _wtoi(V_BSTR(&cv));
            if (uint == 0)
            {
                CComBSTR bstr(V_BSTR(&cv));
                if (bstr[0] != L'0')
                {
                    CInvalidXmlException ixe(L"schema validation failure:  non-numeric value for " + name,L"");
                    LOG(logger, ILogger::Error, ixe.GetMessage());
                    throw ixe;
                }
            }
            return uint;
        }
        
        CString msg;
        msg.Format(L"schema validation failure: attribute %s missing for %s %s", name, ElementUtils::GetName(spElement), 
                                                        ElementUtils::GetOptionalAttributeByName(spElement, L"Name", logger, false));
        CInvalidXmlException ixe(msg, L"");
        if (log) // will be false if optional; missing optional attributes is not an error
            LOG(logger, ILogger::Error, msg);
        throw ixe;
    }

    static UINT GetOptionalAttributeIntByNameDefaultToParent(const CComPtr<IXMLDOMElement> spElement, const CString& name, ILogger& logger)
    {
        try
        {
            return GetAttributeIntByName(spElement, name, logger, false);
        }
        catch(... /* const CInvalidXmlException& e */)
        {
            //if (e.GetMessage().Find(L"non-numeric") != -1)
            //  throw;  // found optional attribute, but it's malformed => re-throw it

            // didn't find it here; try parent
            CComPtr<IXMLDOMNode> spNode;
            HRESULT hr = spElement->get_parentNode(&spNode);
            if (SUCCEEDED(hr))
            {
                CComQIPtr<IXMLDOMElement> spParent(spNode);
                if (spParent)
                    return GetAttributeIntByName(spParent, name, logger);
            }
            throw; // parent node is not an Element???  Just re-throw.
        }
    }

    
private:
	static DWORD StringToDword(CString csResult, const CString& name, ILogger& logger)
	{
        DWORD dwResult = _wtol(csResult);
        DWORD dwResultLength = csResult.GetLength() + 1;

        // need to verify conversion since "123fzz" would convert to "123"
        CString csConvertedBackResult;
        if (0 == _itow_s(dwResult, csConvertedBackResult.GetBuffer(dwResultLength), dwResultLength, 10) )
        {
            csConvertedBackResult._ReleaseBuffer();
            if ( csConvertedBackResult != csResult)
            {
                CString message;
                message.Format(L"schema validation failure:  non-numeric value, %s, for %s", csResult, name);
                CInvalidXmlException ixe(message,L"");
                LOG(logger, ILogger::Error, ixe.GetMessage());
                throw ixe;
            }
        }
        return dwResult;
	}
public:
	static DWORD GetAttributeDwordByName(const CComPtr<IXMLDOMElement> spElement
        , const CString& name
        , ILogger& logger)
    {       
        CString csResult = GetAttributeByName(spElement, name, logger);
        return StringToDword(csResult, name, logger);
    }
    static DWORD GetOptionalAttributeDwordByName(const CComPtr<IXMLDOMElement> spElement, const CString& name, DWORD defaultValue, ILogger& logger)
    {
		CString csResult;
        try
        {
			csResult = GetAttributeByName(spElement, name, logger, false);
        }
        catch(const CInvalidXmlException&)
        {
            return defaultValue; //Optional attribute missing, return defaultValue
        }
        return StringToDword(csResult, name, logger);
    }

    static CString GetOptionalAttributeDwordByName(const CComPtr<IXMLDOMElement> spElement, const CString& name, const CString& defaultValue, ILogger& logger)
    {
        CString csResult;
        try
        {
            csResult = GetAttributeByName(spElement, name, logger, false);
        }
        catch(const CInvalidXmlException&)
        {
            return defaultValue; //Optional attribute missing, return defaultValue
        }
        // StringToDword throws if string is not a valid DWORD
        StringToDword(csResult, name, logger);
        // return string
        return csResult;
    }


    //------------------------------------------------------------------------------
    // GetAttributeLongByName
    //
    // Converts the value of named attributed to a long and returns it
    // Supports strings in the form: [{+ | –}] [0 [{ x | X }]] [digits]
    //-------------------------------------------------------------------------------
    static long GetAttributeLongByName(const CComPtr<IXMLDOMElement> spElement
        , const CString& name
        , ILogger& logger
        , bool log = true)
    {
        CString csResult = GetAttributeByName(spElement, name, logger, log);
        if (csResult.IsEmpty())
        {
            // empty value is not supported
            CString message;
            message.Format(L"schema validation failure:  empty value, %s, for %s", csResult, name);
            CInvalidXmlException ixe(message,L"");
            if (log)
            {
                LOG(logger, ILogger::Error, ixe.GetMessage());
            }
            throw ixe;
        }
        wchar_t* pendptr = NULL;
        errno_t error = 0;

        // if first character is - then call wcstol, otherwise call wcstoul
        long lResult = 0;
        // The errno global variable was not being reset between unit tests, 
        //so setting it to zero before call to wcstol or wcstoul
        _set_errno(0);
        if ( csResult.GetLength() > 0 && csResult[0] == L'-')
        {
            lResult = wcstol(csResult, &pendptr, 0);
        }
        else
        {
            lResult = wcstoul(csResult, &pendptr, 0);
        }
        // errno is set to ERANGE if overflow or underflow occurs
        _get_errno(&error);
        // verify that the whole string was used, if not then it was not a valid string
        // and that no overflow or underflow occurred
        if ( (NULL != pendptr && NULL != *pendptr) || ERANGE == error)
        {
            CString message;
            message.Format(L"schema validation failure:  non-numeric value, %s, for %s", csResult, name);
            CInvalidXmlException ixe(message,L"");
            if (log)
            {
                LOG(logger, ILogger::Error, ixe.GetMessage());
            }
            throw ixe;
        }
        return lResult;
    }

    //------------------------------------------------------------------------------
    // GetOptionalAttributeLongByName
    //
    // Converts the value of named attributed to a long and returns it
    // If attribute is missing, returns defaultValue
    // Supports strings in the form: [{+ | –}] [0 [{ x | X }]] [digits]
    //-------------------------------------------------------------------------------
    static long GetOptionalAttributeLongByName(const CComPtr<IXMLDOMElement> spElement
        , const CString& name
        , long defaultValue
        , ILogger& logger)
    {       
        long lResult = defaultValue;
        try
        {
            lResult = GetAttributeLongByName(spElement, name, logger, false);
        }
        catch(const CInvalidXmlException& ixe)
        {
            // If attribute is not found, use default, otherwise rethrow
            if (ixe.GetMessage().Find(L"attribute not found") != -1)
            {
                return defaultValue;
            }
            else
            {
                // attribute exists, but bad form
                LOG(logger, ILogger::Error, ixe.GetMessage());
                throw ixe;
            }
        }
        return lResult;
    }


    static ULONGLONG GetAttributeULONGLONGByName(const CComPtr<IXMLDOMElement> spElement, const CString& name, ILogger& logger, bool log = true)
    {
        CComVariant cv;
        HRESULT hr = spElement->getAttribute(CComBSTR(name), &cv);
        if ((hr == S_OK) && (cv.vt == VT_BSTR))
        {
            ULONGLONG ulonglong = 0;
            AtlStrToNum( &ulonglong, V_BSTR(&cv), static_cast<wchar_t**>(NULL), 10);
            CString string;
            // need to verify conversion since "123fzz" would convert to "123"
            if ( 0 == _ui64tow_s(ulonglong, string.GetBuffer(21), 21, 10) )
            {
                string._ReleaseBuffer();
                if ( string != V_BSTR(&cv))
                {
                    CString message;
                    message.Format(L"schema validation failure: %s is invalid, a non-negitive numeric value is required for %s", V_BSTR(&cv), name);
                    CInvalidXmlException ixe(message,L"");
                    LOG(logger, ILogger::Error, ixe.GetMessage());
                    throw ixe;
                }
            }
            return ulonglong;
        }
        
        CString msg;
        msg.Format(L"schema validation failure: attribute %s missing for %s %s", name, ElementUtils::GetName(spElement), 
                                                        ElementUtils::GetOptionalAttributeByName(spElement, L"Name", logger, false));
        CInvalidXmlException ixe(msg, L"");
        if (log) // will be false if optional; missing optional attributes is not an error
            LOG(logger, ILogger::Error, msg);
        throw ixe;
    }

    static ULONGLONG GetOptionalAttributeULONGLONGByName(const CComPtr<IXMLDOMElement> spElement, const CString& name, ILogger& logger)
    {
        try
        {
            return GetAttributeULONGLONGByName(spElement, name, logger, false);
        }
        catch(const CInvalidXmlException&)
        {
            return 0; //Optional attribute missing, default to 0
        }
    }

    static bool IsNodeComment(IXMLDOMNode * node)
    {
        CComQIPtr<IXMLDOMComment> spComment(node);
        return spComment != NULL;
    }

    //This is a static class because we are using it in the constuctor
    static bool EvaluateToBoolValue(CString valueName, bool defaultValue, const CString& csState, ILogger& logger)
    {
        //The default value is true.
        if (csState.IsEmpty())
            return defaultValue;

        if (0 == csState.CompareNoCase(L"true"))
        {
            return true; 
        }

        if (0 == csState.CompareNoCase(L"false"))
        {
            return false;       
        }

        CInvalidXmlException ixe(L"schema validation failure:  invalid value authored for: " + valueName);
        throw ixe;
    }

    //This is a static class because we are using it in the constuctor
    static bool EvaluateYesNoTypeToBoolValue(CString valueName, bool defaultValue, const CString& csState, ILogger& logger)
    {
        //The default value is true.
        if (csState.IsEmpty())
        {
            return defaultValue;
        }

        if (0 == csState.CompareNoCase(L"yes"))
        {
            return true; 
        }

        if (0 == csState.CompareNoCase(L"no"))
        {
            return false;
        }

        CInvalidXmlException ixe(L"schema validation failure:  invalid value authored for: " + valueName);
        throw ixe;
    }

    static CString EvaluateYesNoTypeTo0or1String(CString valueName, CString defaultValue, const CString& csState, ILogger& logger)
    {
        //The default value is true.
        if (csState.IsEmpty())
        {
            return defaultValue;
        }

        if (0 == csState.CompareNoCase(L"yes"))
        {
            return L"1"; 
        }

        if (0 == csState.CompareNoCase(L"no"))
        {
            return L"0";
        }

        CInvalidXmlException ixe(L"schema validation failure:  invalid value authored for: " + valueName);
        throw ixe;
    }
};

class XmlElement
{
private:
    CComPtr<IXMLDOMElement> m_spElement;
    ILogger& m_logger;
    ILocalizedData& m_localizedData;

public:
    XmlElement(const CComPtr<IXMLDOMElement> spElement, ILocalizedData& localizedData, ILogger& logger)
        : m_spElement(spElement)
        , m_logger(logger)
        , m_localizedData(localizedData)
    {
    }

    XmlElement(const CComPtr<IXMLDOMNode> spNode, ILocalizedData& localizedData, ILogger& logger)
        : m_spElement(CComQIPtr<IXMLDOMElement>(spNode))
        , m_logger(logger)
        , m_localizedData(localizedData)
    {
    }

    XmlElement(const CComPtr<IXMLDOMNode> spNode, XmlElement& parent)
        : m_spElement(CComQIPtr<IXMLDOMElement>(spNode))
        , m_logger(parent.m_logger)
        , m_localizedData(parent.m_localizedData)
    {
    }

    XmlElement(XmlElement &rhs)
        : m_spElement(rhs.m_spElement)
        , m_logger(rhs.m_logger)
        , m_localizedData(rhs.m_localizedData)
    {
    }

    XmlElement& operator= (const XmlElement &rhs)
    {
        m_spElement = rhs.m_spElement;
        m_logger = rhs.m_logger;
        m_localizedData = rhs.m_localizedData;
        return *this;
    }

    CComPtr<IXMLDOMElement> GetXMLDOMElement()
    {
        return m_spElement;
    }

    ILogger& GetLogger() const
    {
        return m_logger;
    }

    ILocalizedData& GetLocalizedData() const
    {
        return m_localizedData;
    }

    bool IsNodeComment()
    {
        return ElementUtils::IsNodeComment(m_spElement);
    }

    void SetElement(const CComPtr<IXMLDOMElement> spElement)
    {
        m_spElement = spElement;
    }
    
    bool Exists(void) const
    {
        return (m_spElement != NULL);
    }

    CString Name() 
    {
        return (m_spElement != NULL) ? ElementUtils::GetName(m_spElement) : L"";
    }

    XmlElement FirstChild()
    {
        CComPtr<IXMLDOMNode> spSubNode;
        HRESULT hr = m_spElement->get_firstChild(&spSubNode);
        if (FAILED(hr) || !spSubNode)
        {
            spSubNode = NULL;
            return XmlElement(spSubNode, m_localizedData, m_logger);
        }

        while (ElementUtils::IsNodeComment(spSubNode))
        {
            CComPtr<IXMLDOMNode> spNodeTemp = spSubNode;
            spSubNode.Release();
            hr = spNodeTemp->get_nextSibling(&spSubNode);
            if (FAILED(hr) || !spSubNode)
            {
                spSubNode = NULL;
                return XmlElement(spSubNode, m_localizedData, m_logger);
            }
        }
        return XmlElement(spSubNode, m_localizedData, m_logger);        
    }

    XmlElement NextSibling()
    {
        CComPtr<IXMLDOMNode> spNextNode;
        CComPtr<IXMLDOMNode> spThisNode = CComQIPtr<IXMLDOMNode>(m_spElement);

        HRESULT hr = spThisNode->get_nextSibling(&spNextNode);
        if (FAILED(hr) || !spNextNode)
        {
            spNextNode = NULL;
            return XmlElement(spNextNode, m_localizedData, m_logger);
        }

        while (ElementUtils::IsNodeComment(spNextNode))
        {
            CComPtr<IXMLDOMNode> spNodeTemp = spNextNode;
            spNextNode.Release();
            hr = spNodeTemp->get_nextSibling(&spNextNode);
            if (FAILED(hr) || !spNextNode)
            {
                spNextNode = NULL;
                return XmlElement(spNextNode, m_localizedData, m_logger);
            }
        }

        return XmlElement(spNextNode, m_localizedData, m_logger);
    }

    XmlElement Child(int number)
    {
        return XmlElement(ElementUtils::FindChildElementByNumber(m_spElement, number, m_logger), m_localizedData, m_logger);
    }

    XmlElement Child(const CString& elementName)
    {
        return XmlElement(ElementUtils::FindChildElementByName(m_spElement, elementName, m_logger), m_localizedData, m_logger);
    }

    CComPtr<IXMLDOMNodeList> ChildElementsByName(const CString& elementName, long &itemCount)
    {
        return ElementUtils::GetChildElementsByName(m_spElement, elementName, itemCount, m_logger);
    }
    
    unsigned int CountChildElements()
    {
        return ElementUtils::CountChildElements(m_spElement);
    }

    unsigned int CountChildElements(CString csElementName)
    {
        return ElementUtils::CountChildElements(m_spElement, csElementName);
    }

    XmlElement OptionalChild(const CString& elementName)
    {
        return XmlElement(ElementUtils::FindOptionalChildElementByName(m_spElement, elementName, m_logger), m_localizedData, m_logger);
    }

    CString Attribute(const CString& name)
    {
        return ElementUtils::GetAttributeByName(m_spElement, name, m_logger, true);
    }

    CString OptionalAttribute(const CString& name)
    {
        if (m_spElement == NULL)
            return L"";

        return ElementUtils::GetOptionalAttributeByName(m_spElement, name, m_logger, true);
    }

    CString Text()
    {
        if (m_spElement == NULL)
            return L"";
            
        CComBSTR text;
        m_spElement->get_text(&text);
        return m_localizedData.GetLocalizedText(CString(text));
    }

    CComPtr<IXMLDOMNodeList> GetChildElements(const CString& elementName)
    {
        CComPtr<IXMLDOMNodeList> spChildren;
        CComBSTR xpath(L"./" + elementName);
        m_spElement->selectNodes(xpath, &spChildren);
        return spChildren;
    }
};

class XmlNodeList 
{
    CComPtr<IXMLDOMNodeList> m_spList;
    ILogger& m_logger;
    ILocalizedData& m_localizedData;

public:
    XmlNodeList(CComPtr<IXMLDOMNodeList> spList, XmlElement& xeParent)
        : m_spList(spList)
        , m_logger(xeParent.GetLogger())
        , m_localizedData(xeParent.GetLocalizedData())
    {
    }

    XmlNodeList(CComPtr<IXMLDOMNodeList> spList, ILocalizedData& localizedData, ILogger& logger)
        : m_spList(spList)
        , m_logger(logger)
        , m_localizedData(localizedData)
    {
    }

    //-------------------------------------------------------------------------
    // SelectNodes does an XPath Query on the Element and returns a list of
    // nodes that match
    //-------------------------------------------------------------------------
    static XmlNodeList SelectNodes(const CString& xpath, XmlElement& element)
    {
        CComPtr<IXMLDOMNodeList> spNodeList;
        element.GetXMLDOMElement()->selectNodes(CComBSTR(xpath), &spNodeList);
        return XmlNodeList(spNodeList, element.GetLocalizedData(), element.GetLogger());
    }

    long Length(void)
    {
        long length = 0;
        m_spList->get_length(&length);
        return length;
    }
    XmlElement operator[] (long index)
    {
        CComPtr<IXMLDOMNode> pXMLDOMNode;
        m_spList->get_item(index, &pXMLDOMNode);
        return XmlElement(pXMLDOMNode, m_localizedData, m_logger);
    }
    XmlElement Next(void)
    {
        CComPtr<IXMLDOMNode> pXMLDOMNode;
        m_spList->nextNode(&pXMLDOMNode);
        return XmlElement(pXMLDOMNode, m_localizedData, m_logger);
    }
    void Reset(void)
    {
        m_spList->reset();
    }
};




class XmlUtils
{
public:

    // read the file into a BSTR, so I can call XMLDOM.loadXML 
    static CComBSTR ReadXml(const CString csInputFilename, ILogger& logger = NullLogger::GetNullLogger() )
    {       
        CPath pthFilename;      
        if (CPath(csInputFilename).IsRelative())        
        {
            CString csModuleFilePath;
            // szFilename is relative (XMLDOM.load does the right thing, which is to prepend the current .exe's path)
            ::GetModuleFileName(_AtlBaseModule.GetModuleInstance(), csModuleFilePath.GetBuffer(MAX_PATH), MAX_PATH);
            csModuleFilePath._ReleaseBuffer(); 
            CPath pthModulePath(csModuleFilePath);
            pthModulePath.RemoveFileSpec();
            pthFilename.Combine(pthModulePath, csInputFilename);                
        }
        else
        {
            pthFilename = CPath(csInputFilename);
        }

        CAtlFile file;
        //Open with FILE_SHARE_READ to prevent race condition failure.
        HRESULT hr = file.Create(pthFilename.m_strPath, GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING);
        if (FAILED(hr))
        {
            LOGEX(logger, ILogger::Error, L"ReadXML failed to open XML file %s, with error %d", pthFilename, hr);
            throw CHResultException(hr);
        }

        file.Seek(0, FILE_END);

        LARGE_INTEGER liOffset;
        liOffset.QuadPart = 0;
        liOffset.LowPart = ::SetFilePointer(file.m_h, 0, &liOffset.HighPart, FILE_CURRENT);
        // assert liOffset.HighPart == 0

        file.Seek(2, FILE_BEGIN); // skip BOM

        // allocate a bstr big enough
        CComBSTR bstrXML((liOffset.LowPart-2)/sizeof(OLECHAR));

        DWORD nBytesRead = 0;
        hr = file.Read(bstrXML.m_str, liOffset.LowPart-2, nBytesRead);
        if (FAILED(hr))
        {
            // Could not find the XML file (or could not open it for read)
            CString strMessage;
            strMessage.Format(L"Could not find mandatory data file %s. This is a bad package.",pthFilename.m_strPath);
            // Error level log message
            LOG(logger, ILogger::Error, strMessage);
            // Make this a final result summary message too
            LOG(logger, ILogger::Result, strMessage);
            throw CHResultException(hr);
        }

        return bstrXML;
    }

    static CComBSTR GetReasonForParseError(const CComPtr<IXMLDOMDocument2> spDoc, ILogger& logger)
    {
        CComBSTR bstrReason;

        CComPtr<IXMLDOMParseError> spParseError;
        HRESULT hr = spDoc->validate(&spParseError);
        if (FAILED(hr))
        {
            LOGEX(logger, ILogger::Error, L"spDoc->get_parseError failed with hr = 0x%08x", hr);
        }
        else
        {
            hr = spParseError->get_reason(&bstrReason);
            if (FAILED(hr))
            {
                LOGEX(logger, ILogger::Error, L"spParseError->get_reason failed with hr = 0x%08x", hr);
            }
        }

        return SUCCEEDED(hr) ? bstrReason : CComBSTR(L"Parse failed for some unknown reason");
    }
    
    //This function is used to expand the ExpressionAlias.  It does not do nested expansion.
    static CComPtr<IXMLDOMElement> ExpandExpressionAlias(const CComPtr<IXMLDOMElement>& spElement
                                                         , ILogger& logger
                                                         , CString& name)
    {                
        CComPtr<IXMLDOMElement> spElementLocal = spElement;
        name = ElementUtils::GetName(spElement);

        // Check if it is a ref expression, if so, handle it.
        // It is intentional that we want to do a case sensitive test.
        if (L"ExpressionAlias" == name)
        {            
            CString csAttributeValue = ElementUtils::GetAttributeByName(spElement, L"Id", logger);              
            CString csQuery;
            csQuery.Format(L"//*[@Id='%s']", csAttributeValue);            
            CComPtr<IXMLDOMNodeList> spnodeList;  
            HRESULT hr = spElement->selectNodes(CComBSTR(csQuery), &spnodeList);

            //Bail out when the following condition are true:
            // a. The selectNodes calls fails.
            // b. We failed to find a node            
            if (FAILED(hr) || NULL == spnodeList) 
            {
                CInvalidXmlException ixe(L"schema validation failure: Invalid ExpressionAlias or Id not found: " + csAttributeValue,L"");
                LOG(logger, ILogger::Error, ixe.GetMessage());
                throw ixe;
            }

            int iCount = 0;
            long lLength; 
            spnodeList->get_length(&lLength);            
            for (int iIndex = 0; iIndex < lLength; ++iIndex)
            {             
                CComPtr<IXMLDOMNode> spnode;
                spnodeList->nextNode(&spnode);
                CString csElementName = ElementUtils::GetName(CComQIPtr<IXMLDOMElement>(spnode));

                if (L"ExpressionAlias" != csElementName)
                {
                    iCount++;

                    //There is duplicated Id being defined.  No point continue.
                    if (iCount > 1) break;

                    name = csElementName;
                    spElementLocal = CComQIPtr<IXMLDOMElement>(spnode);                       
                }
            } 

            //This is to catch the following scenarios:
            // a. When no expected Id is being defined
            // b. When expected Id is defined more than once.
            if (iCount != 1)
            {
                CInvalidXmlException ixe(L"schema validation failure: ExpressionAlias's Id not defined or defined too many times: " + csAttributeValue,L"");
                LOG(logger, ILogger::Error, ixe.GetMessage());
                throw ixe;
            }
        }
        return spElementLocal;
    }
};



class XmlDoc
{
private:
    ILogger& m_logger;
    CoInitializer m_coInitializer;
    CComPtr<IXMLDOMDocument2> m_spDoc;
    XmlElement m_spDocElement;
    
public:
    XmlDoc(ILocalizedData& localizedData, ILogger& logger) : m_logger(logger), m_spDocElement(CComPtr<IXMLDOMElement>(NULL), localizedData, logger)
    {
    }

    HRESULT LoadXML(CPath xmlFile)
    {
        return LoadXML(XmlUtils::ReadXml(xmlFile,m_logger));
    }

    HRESULT LoadXML(CComBSTR bstrXML)
    {
        HRESULT hr = m_spDoc.CoCreateInstance(__uuidof(DOMDocument30)); 
        if (FAILED(hr))
        {
            LOGEX(m_logger, ILogger::Error, L"CoCreateInstance(__uuidof(DOMDocument30)) failed with hr=%d", hr);
            return hr;
        }

        VARIANT_BOOL vb = VARIANT_FALSE;
        hr = m_spDoc->loadXML(bstrXML, &vb);

        if (hr != S_OK || vb != VARIANT_TRUE) // hr=S_FALSE is a failure
        {
            CInvalidXmlException invalidXML(GetParseError(),L"");
            LOGEX(m_logger, ILogger::Error, L"m_spDoc->loadXML() failed. Parse error is: %s", invalidXML.GetMessage());
            throw invalidXML;
        }

        CComPtr<IXMLDOMElement> spElement;
        hr = m_spDoc->get_documentElement(&spElement);
        if (hr != S_OK)
        {
            CInvalidXmlException invalidXML(GetParseError(),L"");
            LOGEX(m_logger, ILogger::Error, L"m_spDoc->get_documentElement() failed. Parse error is: %s", invalidXML.GetMessage());
            throw invalidXML;
        }

        m_spDocElement.SetElement(spElement);
        
        return hr;
    }

    XmlElement& GetDocElement(void)
    {
        return m_spDocElement;
    }

private:
    CString GetParseError()
    {
        // Get IXMLDOMParseError object from IXMLDOMDocument2::validate and get the actual error
        CComPtr<IXMLDOMParseError> spParseError;
        m_spDoc->validate(&spParseError);
        CComBSTR bstrReason;
        spParseError->get_reason(&bstrReason);
        return CString(bstrReason);
    }

private:
    XmlDoc();
    XmlDoc(const XmlDoc&);
    XmlDoc& operator= (const XmlDoc&);
};

}
