//-------------------------------------------------------------------------------------------------
// <copyright file="LocalizedData.h" company="Microsoft">
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

#include "..\common\xmlutils.h"
#include "..\common\CoInitializer.h"
#include "ModuleUtils.h"
#include "..\CmdLineParser.h"
#include "..\CreateXmlForUninstallPatch.h"
#include "..\common\ParamSubstitutter.h"

namespace IronMan
{

class Language
{
    ILogger& m_logger;
    CString m_langID;
    CSimpleMap<CString, CString> m_map; // maps ID Value attribute to its string value 

public:
    Language(void)
        : m_logger(NullLogger::GetNullLogger())
    {
    }
    Language(const Language& rhs)
        : m_logger(rhs.m_logger)
        , m_langID(rhs.m_langID)
    {
        for(int i=0; i<rhs.m_map.GetSize(); ++i)
        {
            m_map.Add(rhs.m_map.GetKeyAt(i), rhs.m_map.GetValueAt(i));
        }
    }
    const Language& operator= (const Language& rhs)
    {
        m_logger = rhs.m_logger;
        m_langID = rhs.m_langID;
        for(int i=0; i<rhs.m_map.GetSize(); ++i)
        {
            m_map.Add(rhs.m_map.GetKeyAt(i), rhs.m_map.GetValueAt(i));
        }
    }

    Language(const CComPtr<IXMLDOMElement>& spElement, LANGID langID, ILogger& logger) 
        : m_logger(logger)
    {
        if (spElement == NULL)
        {
            LOGEX(logger, ILogger::Information, L"Unable to find Language element for LangID=\"%d\" in localized data", langID);
            throw CHResultException(E_INVALIDARG);
        }

        ElementUtils::VerifyName(spElement, L"Language", logger);
        m_langID.Format(L"%d",langID);
        
        long count = 0;
        CComPtr<IXMLDOMNodeList> spChildren = ElementUtils::GetChildElementsByName(spElement, L"Text", count, logger);
        HRESULT hr;

        for(int i = 0; i < count; i++)
        {
            CComPtr<IXMLDOMNode> spChild;
            hr = spChildren->get_item(i, &spChild);
            if (S_OK == hr)
            {
                CString id = ElementUtils::GetAttributeByName(CComQIPtr<IXMLDOMElement>(spChild), L"ID", logger, false);
                CString value = ElementUtils::GetAttributeByName(CComQIPtr<IXMLDOMElement>(spChild), L"LocalizedText", logger, false);
                AddUniqueEntry(id, value);
            }
        }
    }

    virtual ~Language() {}

    CString LangID(void) const
    {
        return m_langID;
    }

    unsigned int GetCount() const { return m_map.GetSize(); }

    CString GetLocalizedText(const CString& ID) const
    {
        int index = m_map.FindKey(ID);
        if (index != -1)
            return m_map.GetValueAt(index);

        CDetailException nfe(L"ID '" + ID + L"' was not found in " + m_langID + L"\\LocalizedData.xml");
        LOG(m_logger, ILogger::Error, nfe.GetMessage());
        throw nfe;
    }

private:

    void AddUniqueEntry(CString id, CString value) 
    {
        if (-1 != m_map.FindKey(id))
        {
            CInvalidXmlException ixe(L"Found duplicate ID attribute \"" + id + L"\" for Text element in " + m_langID + L"\\LocalizedData.xml. Duplicates not allowed.");
            LOG(m_logger, ILogger::Error, ixe.GetMessage());
            throw ixe;
        }
        m_map.Add(id, value);
    }
};

class LocIDSubstituter
{
    const Language& m_lang;
    ParamSubstituter<LocIDSubstituter> m_paramSubstituter;
public:         
    LocIDSubstituter(const Language& lang) 
        : m_lang(lang)
        , m_paramSubstituter(*this, L"#(loc.", L")")
    {}
    CString GetParamValue(CString param) const
    { 
        CString value = m_lang.GetLocalizedText(param); 
        return value;
    }
    CString Substitute(const CString strIn) 
    {
        CString strOut;
        CString strTemp = strIn;
        for (int i=0; i<6; ++i)
        {
            bool bMadeSubstitutions = false;
            strOut = m_paramSubstituter.SubstituteAnyParams(strTemp, bMadeSubstitutions);
            if (bMadeSubstitutions == false)
                break;

            strTemp = strOut;
        }
        return strOut;
    }
};

class LocalizedData : public ILocalizedData
{
    Language m_language;

public:
    LocalizedData()
    {
    }

    LocalizedData(const LocalizedData& rhs) 
        : m_language(rhs.m_language)
    {
    }
    virtual ~LocalizedData() {}

    const LocalizedData& operator=(const LocalizedData& rhs)
    {
        m_language = rhs.m_language;
    }

    static const CPath GetLocalizedDataFile(LANGID langID)
    {
        CPath pathToLocalizedData = ModuleUtils::GetParameterFilePath(L"");

        CString relPath;
        relPath.Format(L"%d\\LocalizedData.xml", langID); 
        pathToLocalizedData.Append(relPath);
        return pathToLocalizedData;
    }

    static const LocalizedData CreateLocalizedData(LANGID langID, ILogger& logger)
    {
        return CreateLocalizedDataT<CCmdLineSwitches>(langID, logger);
    }

    template<typename TCmdLineSwitches>
    static const LocalizedData CreateLocalizedDataT(LANGID langID, ILogger& logger)
    {   
        // read the file into a BSTR, then call xmldom.loadXML version
        CPath file(GetLocalizedDataFile(langID));
        LOGEX(logger, ILogger::Information, L"Loading localized engine data for language %d from %s", langID, file);
        
        // If file does not exist that will be caught in ReadXml
        CComBSTR bstrXML = XmlUtils::ReadXml(file,logger);

        // If the /UninstallPatch switch is passed in, update the LocalizedData with the DisplayName from the Patch
        // Look for command line option /UninstallPatch
        TCmdLineSwitches switches;
        CString patchCode(switches.UninstallPatch());
        if (!patchCode.IsEmpty())
        {
            CComBSTR updatedLocalizedDataString( 
                CreateXmlForUninstallPatch::UpdateLocalizedData(  bstrXML
                                                                , patchCode
                                                                , logger));
            return CreateLocalizedData(updatedLocalizedDataString, langID, logger);
        }
        else
        {
            // use unmodified LocalizedData
            return CreateLocalizedData(bstrXML, langID, logger);
        }
    }

    static const LocalizedData CreateLocalizedData(const CComBSTR& bstrXML, LANGID langID, ILogger& logger)
    {
        CString pop(L"threw exception");
        ENTERLOGEXIT(logger, pop);

        try
        {
            CoInitializer ci;

            // NOTE NOTE NOTE:  
            // no validation is done during XML Loading, as this would mean shipping the .xsd file.
            // Instead, the validation should be done at authoring time (see validate.js), and again at SetupBuild time.
            // Here we expect valid XML, validate via code and throw exceptions if it's not in the right format.

            CComPtr<IXMLDOMDocument2> spDoc;
            HRESULT hr = spDoc.CoCreateInstance(__uuidof(DOMDocument30)); // this also works:  spDoc.CoCreateInstance(_T("Msxml2.DOMDocument.3.0"));
            if (SUCCEEDED(hr))
            {
                VARIANT_BOOL vb = VARIANT_FALSE;
                hr = spDoc->loadXML(bstrXML, &vb);
                if ((S_OK == hr) && // S_FALSE is a failure
                    (vb == VARIANT_TRUE))
                {
                    CComPtr<IXMLDOMElement> spElement;
                    hr = spDoc->get_documentElement(&spElement);
                    if (S_OK == hr)
                    {
                        pop = L"succeeded";
                        return LocalizedData(spElement, langID, logger);
                    }
                } 
                throw CInvalidLocalizedDataXMLException( CString(XmlUtils::GetReasonForParseError(spDoc, logger)) );
            }
            throw CHResultException(hr);
        }
        catch(const CException& e)
        {
          pop = L"threw exception";
          LOG(logger, ILogger::Error, e.GetMessage());
          throw;
        }
        catch(...)
        {
            pop = L"threw exception";
            LOG(logger, ILogger::Error, L"unknown exception thrown, caught and about to be rethrown.");
            throw;
        }
    }

    // public accessors
    virtual const CString GetLocalizedText(const CString& strID) const
    {
        LocIDSubstituter locIDSubstituter(m_language);
        return locIDSubstituter.Substitute(strID);
    }

public:
    static const CComPtr<IXMLDOMElement> FindLanguage(const CComPtr<IXMLDOMElement> spElement, ILogger& logger)
    {
        if (spElement == NULL)
            return NULL;

        CComPtr<IXMLDOMNode> spNode = CComQIPtr<IXMLDOMNode>(spElement);
        CComPtr<IXMLDOMNode> spLangNode;
        CComPtr<IXMLDOMElement> child;
        CString path(L"//Setup/LocalizedData/Language");

        spNode->selectSingleNode(CComBSTR(path.GetBuffer()), &spLangNode);
        child = CComQIPtr<IXMLDOMElement>(spLangNode);

        return child;
    }

private:
    LocalizedData(const CComPtr<IXMLDOMElement>& spElement, LANGID langID, ILogger& logger)
        : m_language(FindLanguage(spElement, logger), langID, logger)
    {
        if (spElement == NULL)
        {
            LOGEX(logger, ILogger::Information, L"Unable to find Language element for LangID=\"%d\" in localized data", langID);
            throw CHResultException(E_INVALIDARG);
        }
        
        if (1 < ElementUtils::CountChildElements(spElement))
        {
            CString str;
            str.Format(L"Schema validation failure in file %d%s", langID, L"\\LocalizedData.xml: should have atleast one 'Language' child element!");
            CInvalidXmlException ixe(str);
            LOG(logger, ILogger::Error, ixe.GetMessage());
            throw ixe;
        }
    }

public:
    static bool IsLocalizedDataID(CString strData)
    {
        strData.Trim();
        UINT length = CString::StringLength(strData);
        LPCWSTR psz = strData.GetBuffer();
        
        const LPCWSTR szPrefix = L"#(loc.";
        const UINT cchPrefix = (const UINT)wcslen(szPrefix);
        const WCHAR wcPostfix = L')';
        
        LPCWSTR pszLastChar = &psz[length-1];
        if (
            (*pszLastChar != wcPostfix)
            ||
            (wcsncmp(psz, szPrefix, cchPrefix) != 0)
           )
        {
            return false;
        }

        if (psz[cchPrefix] == wcPostfix)
            return false;

        for (psz += cchPrefix; psz != pszLastChar; ++psz)
        {
            if (
                (*psz == L'_') ||
                (*psz >= L'a' && *psz <= L'z') ||
                (*psz >= L'A' && *psz <= L'Z') ||
                (*psz >= L'0' && *psz <= L'9')
               )
            {
                continue;
            }
            else
            {
                return false;
            }
        }

        return true;
    }


};

class LocalizedDataProvider : public ILocalizedDataProvider
{
private:
    static void GetLocIDsForAttribute(const CComPtr<IXMLDOMElement> spRoot, 
                                      CSimpleArray<CString>& locIDsInParameterInfo, 
                                      LPCWSTR attribute,
                                      ILogger& logger)
    {
        CString strXPath;
        strXPath.Format(L".//*[@%s]", attribute);
        CComPtr<IXMLDOMNodeList> spNodeList;
        HRESULT hr = spRoot->selectNodes(CComBSTR(strXPath), &spNodeList);
        strXPath._ReleaseBuffer();
        if (FAILED(hr))
            return;

        LONG length;
        spNodeList->get_length(&length);
        for(long l=0; l<length; ++l)
        {
            CComPtr<IXMLDOMNode> node;
            if (SUCCEEDED(spNodeList->get_item(l, &node)))
            {
                CComBSTR bstrTargetProductCode;
                node->get_text(&bstrTargetProductCode);
                CString attValue = ElementUtils::GetAttributeByName(CComQIPtr<IXMLDOMElement>(node), attribute, logger);
                if (LocalizedData::IsLocalizedDataID(attValue))
                {
                    locIDsInParameterInfo.Add(attValue);
                }
            }
        }
    }

private:
    static bool GetLocIDsFromParameterInfo(CSimpleArray<CString>& locIDsInParameterInfo, ILogger& logger)
    {
        CComBSTR bstrXML(XmlUtils::ReadXml(ModuleUtils::GetParameterFilePath(L"ParameterInfo.xml"),logger));
        bool bParameterInfoHasLocIDs = GetLocIDsFromXML(bstrXML, locIDsInParameterInfo, logger);
        return (bParameterInfoHasLocIDs );
    }

public:
    // Goes through the passid in XML string, looking for valid loc-ids. All the loc-ids are returned in an array.
    static bool GetLocIDsFromXML(const CComBSTR& bstrXML, CSimpleArray<CString>& locIDsInUiInfo, ILogger& logger)
    {
        CString strXML(bstrXML);

        const LPCWSTR locIDPrefix = L"#(loc.";
        const int locIDPrefixLength = (const int)wcslen(locIDPrefix);
        const LPCWSTR locIDPostfix = L")";

        int iStart = 0;
        iStart = strXML.Find(locIDPrefix, iStart);

        while (iStart != -1)
        {
            int iEnd = strXML.Find(locIDPostfix, (iStart + locIDPrefixLength));
            if (iEnd == -1)
                break;
            
            UINT cCharsToCopy = iEnd - iStart + 1;
            CString temp;
            CString::CopyChars(temp.GetBuffer(cCharsToCopy + 1), cCharsToCopy + 1, strXML.GetBuffer() + iStart, cCharsToCopy);
            temp._ReleaseBuffer(cCharsToCopy + 1);
            temp.SetAt(cCharsToCopy, L'\0');
            locIDsInUiInfo.Add(temp);
        
            iStart = strXML.Find(L"#(loc.", iEnd);
        } 
        return (locIDsInUiInfo.GetSize() > 0);
    }
    

private:
    static bool DataFileHasLocalizedData(LPCWSTR dataFile)
    {
        bool bRet = false;
        CComBSTR bstrXML = XmlUtils::ReadXml(dataFile);
        CString strXml(bstrXML);

        LPCWSTR pszXml = strXml.GetBuffer();

        for (int iStart=0;  iStart >= 0; )
        {
            iStart = strXml.Find(L"#(loc.", iStart);

            if (iStart == -1)
                break;
            
            iStart += (int)wcslen(L"#(loc.");

            LPCWSTR pszTemp = ::StrPBrk(&(pszXml[iStart]), L") <>\"");
            if (pszTemp == NULL)
                break;

            if (*pszTemp == L')')
            {
                bRet = true;
                break;
            }
        }

        strXml._ReleaseBuffer();

        return bRet;
    }

    struct PassthroughLocalizedData : public ILocalizedData
    {
        virtual const CString GetLocalizedText(const CString& text) const
        {
            return text;
        }
    } m_passthroughLocalizedData;

    CSimpleArray<CString> m_locIDsInParameterInfo; 

    ILogger& m_logger;
    
    bool m_bHasLocalizedData;

    LCID m_uiLocale;
    LocalizedData m_localizedData;

    LocalizedDataProvider(); // Hide

public:   
    LocalizedDataProvider(ILogger& logger) 
        : m_logger(logger)
        , m_uiLocale(1033)
        , m_bHasLocalizedData(GetLocIDsFromParameterInfo(m_locIDsInParameterInfo, logger))
        , m_localizedData(m_bHasLocalizedData ? LocalizedData::CreateLocalizedData(LANGIDFROMLCID(m_uiLocale), logger) : LocalizedData())
    {
    }

    ~LocalizedDataProvider()
    {
    }

    LCID GetUILanguage(void)
    {
        return m_uiLocale;
    }

    ILocalizedData& GetLocalizedData(void) 
    {
        if (m_bHasLocalizedData)
            return m_localizedData;  // returns localized text.
        else 
            return m_passthroughLocalizedData;   // returns input text as is.
    }


private:
    static CSimpleArray<CString> GetLangs(CPath path)
    {
        WIN32_FIND_DATA findFileData = {0};
        path.Append(L"????");
        HANDLE hFile = FindFirstFile(path, &findFileData);
        bool bMoreFiles = (hFile != INVALID_HANDLE_VALUE);
        CSimpleArray<CString> langs;

        while(bMoreFiles)
        {
            LPCWSTR file = findFileData.cFileName;            
            if (wcslen(file) == 4)
            {
                if (isdigit(file[0]) && isdigit(file[1]) && isdigit(file[2]) && isdigit(file[3]))
                    langs.Add(file);
            }
            bMoreFiles = FindNextFile(hFile, &findFileData);
        }

        FindClose(hFile);

        return langs;
    }


public:
    bool ValidateLanguageInLocalizedData(const CPath& ld, const CString& langID)
    {
        CComBSTR bstrXML(XmlUtils::ReadXml(ld,m_logger));

        CoInitializer ci;

        CComPtr<IXMLDOMDocument2> spDoc;
        HRESULT hr = spDoc.CoCreateInstance(__uuidof(DOMDocument30));
        if (SUCCEEDED(hr))
        {
            VARIANT_BOOL vb = VARIANT_FALSE;
            hr = spDoc->loadXML(bstrXML, &vb);
            if ((S_OK == hr) && // S_FALSE is a failure
                (vb == VARIANT_TRUE))
            {
                CComPtr<IXMLDOMElement> spRoot;
                hr = spDoc->get_documentElement(&spRoot);
                if (SUCCEEDED(hr))
                {  
                    const CComPtr<IXMLDOMElement>& spLang = LocalizedData::FindLanguage(spRoot, NullLogger::GetNullLogger());
                    return (spLang != NULL);
                }
            }
        }
        return false;
    }
    void ValidateLocalizedDataLanguageInResourceFolders(void)
    {
        if (false == m_bHasLocalizedData)
        {
            return;
        }

        CPath root = ModuleUtils::GetParameterFilePath(L"");

        CSimpleArray<CString>& langs = GetLangs(root);

        for (int i=0; i<langs.GetSize(); ++i)
        {
            CPath ld(root);
            ld.Append(langs[i]);
            ld.Append(L"LocalizedData.xml");
            if (ld.FileExists())
            {
                if (ValidateLanguageInLocalizedData(ld, langs[i]) == false)
                {
                    CString str;
                    str.Format(L"LocalizedData.xml in resource folder %s, does not have a Language element", langs[i]);
                    LOG(m_logger, ILogger::Error, str);
                    CInvalidXmlException ixe(str);
                    throw ixe;
                }
            }
            else
            {
                CString str;
                str.Format(L"LocalizedData.xml is missing in resource folder %s.  Every resource folder needs a LocalizedData.xml", langs[i]);
                LOG(m_logger, ILogger::Error, str);
                CNotFoundException nfe(str);
                throw nfe;
            }
        }
    }
   
    //  To validate that localized data exists for a LangID 
    //      step 1: Ensure all the loc-ids in prama info are present in the LocalizedData.xml
    //      step 2: Ensure all the loc-ids in UI info are present in the LocalizedData.xml
    bool Validate(void)
    {
        try 
        {
            // step 1: Ensure all the loc-ids in prama info are present in the LocalizedData.xml
            for (int i=0; i < m_locIDsInParameterInfo.GetSize(); ++i)
            {
                m_localizedData.GetLocalizedText(m_locIDsInParameterInfo[i]);
            }
        }
        catch(...)
        {
            return false;
        }
        return true;
    }

    static bool LocalizedDataForLANGIDExists(LANGID langID)
    {
        const CPath& ldf = LocalizedData::GetLocalizedDataFile(langID);
        if (ldf.FileExists())
        {
            CComBSTR bstrXML = XmlUtils::ReadXml(ldf);
            CoInitializer ci;
            CComPtr<IXMLDOMDocument2> spDoc;
            HRESULT hr = spDoc.CoCreateInstance(__uuidof(DOMDocument30));
            if (SUCCEEDED(hr))
            {
                VARIANT_BOOL vb = VARIANT_FALSE;
                hr = spDoc->loadXML(bstrXML, &vb);
                if ((S_OK == hr) && // S_FALSE is a failure
                    (vb == VARIANT_TRUE))
                {
                    CComPtr<IXMLDOMElement> spRoot;
                    hr = spDoc->get_documentElement(&spRoot);
                    if (SUCCEEDED(hr))
                    {
                        const CComPtr<IXMLDOMElement> langElement = LocalizedData::FindLanguage(spRoot, NullLogger::GetNullLogger());
                        return (langElement != NULL);
                    }
                }
            }
        }
        return false;
    }

    static bool LocalizedDataFileMustExist(void)
    {
        return DataFileHasLocalizedData(ModuleUtils::GetParameterFilePath(L"ParameterInfo.xml"));
    }

};


}
