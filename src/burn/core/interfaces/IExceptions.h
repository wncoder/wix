//-------------------------------------------------------------------------------------------------
// <copyright file="IExceptions.h" company="Microsoft">
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

#include "common\LanguageSelector.h"

namespace IronMan
{

class CException
{
public:
    virtual ~CException() {}
    virtual CString GetMessage() const = 0;
};

class CHResultException : public CException
{
    HRESULT m_hr;
public:
    CHResultException(HRESULT hr) : m_hr(hr) {}
    virtual CString GetMessage() const
    {
        return AtlGetErrorDescription(m_hr, IronMan::ThreadLanguageSelector::GetLangId());
    }
};

class CDetailException : public CException
{
    CString m_detail;
public:
    CDetailException(const CString& detail) : m_detail(detail) {}
    virtual CString GetMessage() const
    {
        return m_detail;
    }
};

struct CInvalidXmlException : public CDetailException
{
protected:
    CString m_strFile;
public:
    CInvalidXmlException(const CString& detail, const CString &strFile=CString(L"ParameterInfo.xml")) :
        CDetailException(detail)
    {
        if (!strFile.IsEmpty())
        {
            m_strFile = CString(L" in file ") + strFile;
        }
    }

    virtual CString GetMessage() const
    {		
        return L"Invalid XML" + m_strFile + L".\n\nParse error:\n\t" + CDetailException::GetMessage();
    }

};

struct CInvalidLocalizedDataXMLException : public CDetailException
{
    CInvalidLocalizedDataXMLException(const CString& detail) : CDetailException(detail) {}
    virtual CString GetMessage() const
    {
        return L"Invalid XML in LocalizedData.xml file.\n\nParse error:\n\t" + CDetailException::GetMessage();
    }
};

struct COutOfBoundsException : public CDetailException
{
    COutOfBoundsException(const CString& what) : CDetailException(what) {}
    virtual CString GetMessage() const
    {
        return CDetailException::GetMessage() + L" is out of bounds.";
    }
};

struct CNotFoundException : public CDetailException
{
    CNotFoundException(const CString& what) : CDetailException(what) {}
    virtual CString GetMessage() const
    {
        return CDetailException::GetMessage() + L" was not found.";
    }
};

struct CUnableToOpenLogFileException : public CDetailException
{
    CUnableToOpenLogFileException(const CString& what) : CDetailException(what) {}
    virtual CString GetMessage() const
    {
        return L"Unable to open log file " + CDetailException::GetMessage();
    }
};

struct CCreationException : public CDetailException
{
    CCreationException(const CString& what) : CDetailException(what) {}
    virtual CString GetMessage() const
    {
        return L"Unable to create " + CDetailException::GetMessage() + L".";
    }
};

struct CObjectNotInitializedException : public CDetailException
{
    CObjectNotInitializedException(const CString& what) : CDetailException(what) {}
    virtual CString GetMessage() const
    {
        return L"This object has not been initialized - " + CDetailException::GetMessage() + L".";
    }
};

struct CLogCreationException : public CDetailException
{
    CLogCreationException(const CString& what) : CDetailException(what) {}
    virtual CString GetMessage() const
    {
        return L"Unable to create Log - " + CDetailException::GetMessage() + L".";
    }
};

struct CIncompatibleLogHeader : public CDetailException
{
    CIncompatibleLogHeader(const CString& what) : CDetailException(what) {}
    virtual CString GetMessage() const
    {
        return L"Log file to be appended doesn't have header information corresponding to: " + CDetailException::GetMessage() + L" file.";
    }
};

struct CBadStatefulItems : public CDetailException
{
    CBadStatefulItems(const CString& what) : CDetailException(what) {}
    virtual CString GetMessage() const
    {
        return L"Stateful Item List: " + CDetailException::GetMessage() + L".";
    }
};
class CWinAPIException : public CException
{
    DWORD m_dwError;
    CString m_failedFunction;
public:
    CWinAPIException( const CString& failedFunction, DWORD dwError) 
        : m_dwError(dwError)
        , m_failedFunction(failedFunction)
    {}
    virtual CString GetMessage() const
    {
        CString message;
        message.Format(L"%s failed with error: %u", m_failedFunction, m_dwError);
        return message;
    }
};

}
