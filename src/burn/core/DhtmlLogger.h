//-------------------------------------------------------------------------------------------------
// <copyright file="DhtmlLogger.h" company="Microsoft">
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
//    This class's job is to generate the DHTML log
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once

#include "TextLogger.h"
#include "LogSignatureDecorator.h"

static BOOL s_fHasEmbeddedHeader = FALSE;

namespace IronMan
{
    template<typename T> class CDhtmlLoggerT : public T
    {
        CString m_before, m_after;
        CRITICAL_SECTION m_cs;
    
    public:
        //Constructor
        CDhtmlLoggerT(LPCWSTR fqfnLogFile = NULL, LPCWSTR fileExtension = L".html") : T(fqfnLogFile, GetDhtmlHeaderSpec(), fileExtension)
        {
            ::InitializeCriticalSection(&m_cs);

            if (!s_fHasEmbeddedHeader)
            {
                CPath dhtmlHeaderPath (GetDhtmlHeaderSpec());
                if (!dhtmlHeaderPath.FileExists())
                {
                    throw CLogCreationException(L"DHTML Header File doesn't exist");
                }
            }
        }
        virtual ~CDhtmlLoggerT() 
        {
            ::DeleteCriticalSection(&m_cs);
        }

    public:
        virtual void Log(ILogger::LoggingLevel lvl, LPCWSTR szLogMessage)
        {
            // Only one thread at a time can write to the log.
            ::EnterCriticalSection(&m_cs);
            {
                CString log = LogInternal(lvl, szLogMessage);
                T::Log(lvl, log);
            }
            ::LeaveCriticalSection(&m_cs);
        }

        virtual void LogFinalResult(LPCWSTR szLogMessage)
        {
            // Only one thread at a time can write to the log.
            ::EnterCriticalSection(&m_cs);
            {
                CString log = LogInternal(ILogger::FinalResult, szLogMessage);
                T::LogFinalResult(log);
            }
            ::LeaveCriticalSection(&m_cs);
        }

        static HRESULT SetupStaticLoggingSettings(BOOL fHasEmbeddedHeader)
        {
            s_fHasEmbeddedHeader = fHasEmbeddedHeader;

            if (fHasEmbeddedHeader)
            {
                return LogSetSpecialParams((LPSTR)"<span class=\"msg\"><span class=\"t\">", "</span>", "<br></span>\r\n");
            }
            else
            {
                return S_OK;
            }
        }

    private:
        CString LogInternal(ILogger::LoggingLevel lvl, LPCWSTR szLogMessage)
        {
                // level:
                m_before  = L"\r\n<span class=\"" + GetLoggingLevelInStringFormat(lvl) + L"\">";
                // time stamp:
                m_before += L"<span class=\"t\">" + GetFormattedTimeStamp() + L"</span>";

                // signature:
#ifdef IRONMAN_FUNCTIONNAMEINLOGFILE
                CString sig = EscapeXMLString(LogHelper::GetFunctionNameFromDecoration(szLogMessage));
                CString log = LogHelper::RemoveFunctionNameFromDecoration(szLogMessage);
                // set up data for "template methods"
                if (!sig.IsEmpty())
                {
                    m_before += L"<span class=\"f\"> (" + sig + L") </span>";
                }
#else
                CString log = szLogMessage;
#endif

                m_after =   L"<BR></span>";
                return log;
        }

        ILogger::LoggingLevel m_lvl;
        CString m_logAsIs;
        UINT m_currentListLevel;

        virtual void BeginLogAsIs(ILogger::LoggingLevel lvl, CString szLogStart)
        {
            m_lvl = lvl;
            m_logAsIs = CString(L"<h4>") + szLogStart + L"</h4>";
            m_currentListLevel = 0;
        }
        virtual void LogLine(LPCWSTR szLine)
        {
            m_logAsIs += CString(L"<br><h5>") + szLine + L"</h5>";
        }
        virtual void LogStartList()
        {
            ++m_currentListLevel;
            m_logAsIs += (m_currentListLevel == 1) ? L"<ol>" : L"<ul>";
        }
        virtual void LogListItem(LPCWSTR item)
        {
            m_logAsIs += CString(L"<li><em>") + item + L"</em></li>";
        }
        virtual void LogEndList()
        {
            m_logAsIs += (m_currentListLevel == 1) ? L"</ol>" : L"</ul>";
            --m_currentListLevel;
        }
        virtual void EndLogAsIs()
        {
            Log(m_lvl, m_logAsIs);
        }

    private:
        virtual CString BeforeLog() { return m_before; }
        virtual CString AfterLog () { return m_after;  }

    public:
        //------------------------------------------------------------------------------
        // PushSectionHeader
        //
        // Format as section header and write to the log file
        //
        // Example:
        //
        //  <div class="sectionHdr">
        //      <a href="#" onclick="toggleSection(); event.returnValue=false;">
        //          <span class="sectionExp">
        //              Action 
        //              <span class="t">15:55:42: </span>
        //                  INSTALL. 
        //              </span>
        //          <span class="sectionExp2">
        //              . . .
        //          </span>
        //      </a>
        //    
        //------------------------------------------------------------------------------
        virtual void PushSection(LPCWSTR strAction, LPCWSTR strSectionWording) 
        {
            CString strText;
            
            strText  = L"\r\n<span class=\"act\">";
            strText += L"<div class=\"sectionHdr\">";
            strText += L"<a href=\"#\" onclick=\"toggleSection(); event.returnValue=false;\">";
            strText += L"<span class=\"sectionExp\">";
            strText += L"<span class=\"t\">" + GetFormattedTimeStamp() + L" </span>";
            strText += CString(strAction) + L": ";
            strText += CString(strSectionWording);
            strText += L"</span>";
            strText += L"<span class=\"sectionExp2\">";
            strText += L"...<BR>";
            strText += L"</span>";
            strText += L"</a>"; 
            strText += L"</div>";

            strText += L"<div class=\"section\">";
            T::Write(strText);
        }

    public:
        virtual void PopSection(LPCWSTR str)
        {
            CString cs;
            cs += L"</div><div class=\"sectionHdr\">";

            cs += L"<span class=\"t\">" + GetFormattedTimeStamp() + L" </span>";

#ifdef IRONMAN_FUNCTIONNAMEINLOGFILE
            CString sig = EscapeXMLString(LogHelper::GetFunctionNameFromDecoration(str));
            CString log = LogHelper::RemoveFunctionNameFromDecoration(str);

            if (!sig.IsEmpty())
            {
                cs += L"<span class=\"f\"> (" + sig + L") </span>";
            }
#else
            CString log = str;
#endif
            if (!log.IsEmpty())
            {
                cs += log;
            }

            cs += L"<BR></div></span>\r\n";
            T::Write(cs);
        }

    public: // fork / merge
        virtual ILogger* Fork()
        {
            return new TextLogger::LoggerWithForkedName<CDhtmlLoggerT<TextLogger>>(GetFilePath());
        }
        
    private:
    private:
        //------------------------------------------------------------------------------
        // EscapeXMLString
        //
        // To escape all conflicting character
        //------------------------------------------------------------------------------
        static CString EscapeXMLString(const CString& xml)
        {
            CString csXmlDest;
            DWORD dwDestinationLength = xml.GetLength() * 2;
            int  iConverted = EscapeXML(xml
                , xml.GetLength()
                , csXmlDest.GetBuffer(dwDestinationLength)
                , dwDestinationLength);
            csXmlDest._ReleaseBuffer();
            return csXmlDest.Left(iConverted);
        }

    private:
        static CString GetLoggingLevelInStringFormat(ILogger::LoggingLevel lvl)
        {
            switch(lvl)
            {
            case ILogger::Error:            return L"err";
            case ILogger::Warning:          return L"wrn";
            case ILogger::Information:      return L"msg";  
            case ILogger::UX:               return L"ux";
            case ILogger::Debug:            return L"dbg";  
            case ILogger::InternalUseOnly:  return L"itn";  
            case ILogger::FinalResult:
            case ILogger::Result:           return L"r";
            default:
                HIASSERT(false, L"impossible ILogger::LoggingLevel");
                __fallthrough;
            case ILogger::Verbose:          return L"vbe";
            }
        }

    private:
        // Get the DhtmlHeader.html file path
        static CString GetDhtmlHeaderSpec()
        {            
            CString strFileName(L"DHTMLHeader.html");
            unsigned int langId = ThreadLanguageSelector::GetLocaleId();

            // See if the resource file exists in a subdirectory matching the chosen language, e.g. 1033, and use that if it does
            CString strLocalizedFile;
            strLocalizedFile.Format(L"%04d\\%s", langId, strFileName);

            //Need to use GetModuleFileName with a HINSTANCE because of our unit test.  The basic problem
            //all these file does not reside in a relative location to our TDDGUI.exe (Unit test exe).  
            CString dllModulePath;
            GetModuleFileName(reinterpret_cast<HINSTANCE>(&__ImageBase), dllModulePath.GetBuffer(MAX_PATH), MAX_PATH);
            dllModulePath._ReleaseBuffer();

            CPath pthFullPathToModule(dllModulePath);
            pthFullPathToModule.RemoveFileSpec();

            CPath pthReturnPath;
            pthReturnPath.Combine(pthFullPathToModule, strLocalizedFile);

            // If we found the localized version relative to module we are done searching
            if (!pthReturnPath.FileExists())
            {
                // Try non-localized version next to module
                pthReturnPath.Combine(pthFullPathToModule, strFileName);
                if (!pthReturnPath.FileExists())
                {
                    // Try the ParameterInfo.xml location as the root
                    CPath pthParameterFileFolder(ModuleUtils::GetParameterFilePath(L""));
                    pthReturnPath.Combine(pthParameterFileFolder, strLocalizedFile);
                    if (!pthReturnPath.FileExists())
                    {
                        // Try non-localized version next to ParameterInfo.xml
                        pthReturnPath.Combine(pthParameterFileFolder, strFileName);
                        if (!pthReturnPath.FileExists())
                        {
                            // Return unadorned file name next to module
                            pthReturnPath.Combine(pthFullPathToModule, strFileName);
                        }
                    }
                }
            }

            return CString(pthReturnPath);
        }

    };

    typedef CDhtmlLoggerT<TextLogger> CDhtmlLogger;
}
