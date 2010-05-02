//-------------------------------------------------------------------------------------------------
// <copyright file="ValidateXMLFile.h" company="Microsoft">
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

namespace IronMan
{
    
class ValidateXMLFile
{
    ILogger& m_logger;
    CComPtr<IXMLDOMDocument2> m_pIXMLDOMDocument2;
    CComPtr<IXMLDOMSchemaCollection2> m_pIXMLDOMSchemaCollection2;

    //
    // Create the XML DOM
    //
    void CreateXmlDOM(void)
    {
        HRESULT hr = m_pIXMLDOMDocument2.CoCreateInstance(__uuidof(DOMDocument40));
        if (hr != S_OK) 
            throw CHResultException(hr);

        m_pIXMLDOMDocument2->put_async(VARIANT_FALSE);
        m_pIXMLDOMDocument2->put_resolveExternals(VARIANT_FALSE);
        m_pIXMLDOMDocument2->put_validateOnParse(VARIANT_TRUE);
    }

    //
    // Load the document into the XML DOM
    //
    void LoadDocumentIntoXmlDOM(LPCWSTR xmlFile)
    {
        VARIANT_BOOL isSuccessful = VARIANT_FALSE;
        CComVariant varXML(xmlFile);
        HRESULT hr = m_pIXMLDOMDocument2->load(varXML, &isSuccessful);
        if (FAILED(hr) || isSuccessful != VARIANT_TRUE)
        {
            CComPtr<IXMLDOMParseError> err;
            m_pIXMLDOMDocument2->get_parseError(&err);
            CString msg;

            long line, pos;
            err->get_line(&line);
            err->get_linepos(&pos);

            CComBSTR reason, srcText;
            err->get_reason(&reason);
            err->get_srcText(&srcText);

            msg.Format(L"\nValidation FAILED \n\nErr on line: %d @column: %d\n\nReason:\n%s \n\nSrcText:\n%s", 
                 line, pos, CString((LPCWSTR)reason), CString((LPCWSTR)srcText));

            LOG(m_logger, ILogger::Error, msg);
            throw CDetailException(msg);
        }
    }

    //
    // Create the Schema Collections
    //
    void CreateSchemaCollection(void)
    {
        HRESULT hr = m_pIXMLDOMSchemaCollection2.CoCreateInstance(__uuidof(XMLSchemaCache40));
        if (hr != S_OK) 
            throw CHResultException(hr);
    }

    //
    // Add the schema to the collection
    //
    void AddSchemaToTheCollection(LPCWSTR namespaceURI, LPCWSTR xsdFile)
    {
        HRESULT hr = m_pIXMLDOMSchemaCollection2->add(CComBSTR(namespaceURI), CComVariant(xsdFile));
        if (hr != S_OK) 
            throw CHResultException(hr);
    }

    //
    // Attach the schema collection to the doc
    //
    void AttachTheSchemaCollectionToTheDoc(void)
    {
        m_pIXMLDOMDocument2->putref_schemas(CComVariant(m_pIXMLDOMSchemaCollection2));
    }

    //
    //  Validate the doc
    //
    void ValidateTheDoc(void)
    {
        CComPtr<IXMLDOMParseError> err;
        hr = m_pIXMLDOMDocument2->validate(&err);
        long errorCode = 0;
        hr = err->get_errorCode(&errorCode);
        if (errorCode)
        {
            CComBSTR reason;
            err->get_reason(&reason);
    
            CString msg;
            msg.Format(L"\nValidation FAILED \n\n\nReason:\n%s", CString((LPCWSTR)reason));
            LOG(logger, ILogger::Error, msg);
            throw CDetailException(msg);
        }
    }


public:
    ValidateXMLFile(ILogger& logger)
        : m_logger(logger)
    {
    }

    void Validate(LPCWSTR namespaceURI, LPCWSTR xmlFile, LPCWSTR xsdFile)
    {
        CreateXmlDOM();
        LoadDocumentIntoXmlDOM(xmlFile);

        CreateSchemaCollection();
        AddSchemaToTheCollection(namespaceURI,xsdFile);
        AttachTheSchemaCollectionToTheDoc();

        ValidateTheDoc();
    }

    virtual ~ValidateXMLFile()
    {
    }

private:
    ValidateXMLFile();
    ValidateXMLFile(const ValidateXMLFile&);
    ValidateXMLFile& operator= (const ValidateXMLFile&);
};

} // namespace IronMan
