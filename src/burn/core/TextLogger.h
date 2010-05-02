//-------------------------------------------------------------------------------------------------
// <copyright file="TextLogger.h" company="Microsoft">
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

#include "common\SystemUtil.h"
#include "interfaces\ILogger.h"
#include "interfaces\IExceptions.h"
#include "LogSignatureDecorator.h"
#include "common\ProcessUtils.h"

static long s_lLogUtilRefCount = 0;
static bool s_fLogNameInitialized = false;
static CString s_filename;
static CString s_fileExtension;
static bool s_bIsFinalResultSet = false;

static struct CriticalSection : CRITICAL_SECTION
{
    CriticalSection()
    {
        ::InitializeCriticalSection(this);
    }

    ~CriticalSection()
    {
        ::DeleteCriticalSection(this);
    }
} s_cs;

namespace IronMan
{
class TextLogger : public ILogger
{

public:
    template<typename B> class LoggerWithForkedName : public B
    {
        CPath m_name;
    public:
        // Constructor
        LoggerWithForkedName(const CPath& name)
            : B()
            , m_name(name)
        {
        }

        // Virtual destructor
        virtual ~LoggerWithForkedName()
        {
        }

        virtual CPath GetFilePath()
        {
            return m_name;
        }

        virtual bool RenameLog(const CPath & pthNewName)
        {
            m_name = pthNewName;
            return true;
        }
    };

public:
    // Constructor
    TextLogger(LPCWSTR fqfnLogFile = NULL, LPCWSTR headerFile = NULL, LPCWSTR fileExtension = L".txt")
    {
        ::EnterCriticalSection(&s_cs);
        if (!s_fLogNameInitialized)
        {
            s_filename = fqfnLogFile;
            s_fileExtension = fileExtension;
        }
        ::LeaveCriticalSection(&s_cs);

        OpenLog(eomOverwrite, headerFile);
    }

    // Destructor
    ~TextLogger()
    {
    }

    //------------------------------------------------------------------------------
    // Log
    //
    // Write the log to the Log file.
    //
    // Note: szFunctionName is being used here.  It is being used in the DHTMLLogger 
    //       class
    //------------------------------------------------------------------------------
    virtual void Log(LoggingLevel lvl, LPCWSTR szLogMessage)
    {

#ifdef IRONMAN_FUNCTIONNAMEINLOGFILE
        CString sig = LogHelper::GetFunctionNameFromDecoration   (szLogMessage);
        CString log = LogHelper::RemoveFunctionNameFromDecoration(szLogMessage);
        CString message = sig.IsEmpty() ? log : sig + L"(): " + log;
#else
        CString message = szLogMessage;
#endif
        #ifdef DEBUG
            // In a debug build, copy all logging to the debug console.
            switch (lvl)
            {
                case Error : 
                        ::OutputDebugString(L"Error:   "); 
                        break;
                case Warning : 
                        ::OutputDebugString(L"Warning: "); 
                        break;
                case Information: 
                        ::OutputDebugString(L"Info:    "); 
                        break;
                case UX : 
                        ::OutputDebugString(L"UX:      "); 
                        break;
                case Verbose : 
                        ::OutputDebugString(L"Verbose: "); 
                        break;
                case Debug : 
                        ::OutputDebugString(L"Debug:   "); 
                        break;
            }
            ::OutputDebugString(message);
            ::OutputDebugString(L"\n");
        #else
            // In a retail build, don't log debug and Internal messages.
            if ((lvl == Debug) || (lvl == InternalUseOnly))
            {
                return;
            }
        #endif

        // Only one thread at a time can write to the log.
        ::EnterCriticalSection(&s_cs);
        {
            //We don't want to show the namespace, so we remove it here before actual writing.
            CString strPrefix(__FUNCTION__);
            strPrefix = strPrefix.Left(strPrefix.Find(L"::") + 2);
            CString csMessage = message;
            csMessage.Replace(strPrefix, L"");

            REPORT_LEVEL rl = REPORT_STANDARD;

            switch (lvl)
            {
            case Error:
                rl = REPORT_ERROR;
                break;
            case Warning:
                rl = REPORT_STANDARD;
                csMessage = L"Warning:" + csMessage;
                break;

            case Information:
                rl = REPORT_STANDARD;
                csMessage = L"Information:" + csMessage;
                break;

            case UX:
                rl = REPORT_STANDARD;
                csMessage = L"UX:" + csMessage;
                break;

            case Verbose:
                rl = REPORT_VERBOSE;
                break;

            case Debug:
                rl = REPORT_DEBUG;
                break;

            case InternalUseOnly: __fallthrough;
            case Result: __fallthrough;
            case FinalResult: __fallthrough;
                rl = REPORT_STANDARD;
                break;
            }

            // TODO: check the HRESULT return codes when there's something to do with the result
            ::LogString(rl, CStringA(BeforeLog()));
            ::LogString(rl, CStringA(csMessage));
            ::LogString(rl, CStringA(AfterLog()));
        }
        ::LeaveCriticalSection(&s_cs);
    }

    virtual void LogFinalResult(LPCWSTR szLogMessage)
    {
        //Log only 1 FinalResult
        ::EnterCriticalSection(&s_cs);
        if (s_bIsFinalResultSet)
        {
            ::LeaveCriticalSection(&s_cs);
            return;
        }
        s_bIsFinalResultSet = true;
        ::LeaveCriticalSection(&s_cs);

        Log(FinalResult, szLogMessage);
    }

    virtual void PushSection(LPCWSTR strAction, LPCWSTR strDescription) {}
    virtual void PopSection(LPCWSTR) {}

private: 
    ILogger::LoggingLevel m_lvl;
    CString m_logAsIs;
    int m_currListLevel;
    static const int m_maxListLevel = 4;
    WCHAR m_listIndex[m_maxListLevel+1];

protected:
    //------------------------------------------------------------------------------
    // GetFormattedTimeStamp
    //
    // Format the the time stamp for the log file.
    //------------------------------------------------------------------------------
    static CString GetFormattedTimeStamp()
    {
        CString strTimeStamp;
        //[07/25/07,15:04:21]
        SYSTEMTIME lpSystemTime ;
        ::GetLocalTime(&lpSystemTime);     
        strTimeStamp.Format(L"[%02u/%02u/%04u, %02u:%02u:%02u]", 
            lpSystemTime.wMonth, 
            lpSystemTime.wDay,
            lpSystemTime.wYear,
            lpSystemTime.wHour,
            lpSystemTime.wMinute,
            lpSystemTime.wSecond);

        return strTimeStamp;
    }

public:
    virtual void BeginLogAsIs(ILogger::LoggingLevel lvl, CString szLogStart)
    {
        m_lvl = lvl;
        m_logAsIs = szLogStart;
        m_currListLevel = -1;
    }
    virtual void LogLine(LPCWSTR szLine)
    {
        m_logAsIs += CString(L"\r\n") + szLine;
    }
    virtual void LogStartList()
    {
        if (m_currListLevel >= m_maxListLevel)
        {
            Log(ILogger::Error, L"LOGGING ERROR - Only two level of lists allowed");
        }
        else
        {
            ++m_currListLevel;
            
            if (m_currListLevel < 0)
            {
                Log(ILogger::Error, L"LOGGING ERROR - currListLevel is less than 0");
                m_currListLevel = 0;
            }
            else if (m_currListLevel > m_maxListLevel)
            {
                m_currListLevel = m_maxListLevel;
                Log(ILogger::Error, L"LOGGING ERROR - currListLevel is greater than maxIndex");
            }
            
            m_listIndex[m_currListLevel] = L'1' - 1;
        }
    }
    CString GetIndex(void)
    {
        CString index;
        index.Format(L"%c", m_listIndex[0]);
        
        IMASSERT(m_currListLevel <= m_maxListLevel); 

        for (int i=1; i<=m_currListLevel; ++i)
        {
            CString temp;
            temp.Format(L".%c", m_listIndex[i]);
            index += temp;
        }
        return index;
    }
    virtual void LogListItem(LPCWSTR item)
    {
        LPWSTR indent[5] = {L"\t", L"\t\t", L"\t\t\t", L"\t\t\t\t", L"\t\t\t\t\t"};

        if (m_currListLevel >= 0 && m_currListLevel <= m_maxListLevel)
        {
            ++(m_listIndex[m_currListLevel]);
            CString strIndex;
            strIndex.Format(L"\r\n%s%s) ", indent[m_currListLevel], GetIndex());
            m_logAsIs += strIndex + item;
        }
        else
        {
            Log(ILogger::Error, L"LOGGING ERROR - m_currListLevel is out of bounds.");
        }
    }
    virtual void LogEndList()
    {
        if (m_currListLevel >=0 && m_currListLevel <= m_maxListLevel )
        {
            m_listIndex[m_currListLevel] = L'1';
            --m_currListLevel;
        }
        else
            Log(ILogger::Error, L"LOGGING ERROR - LogEndList called too many times?");
    }
    virtual void EndLogAsIs()
    {
        Log(m_lvl, m_logAsIs);
    }

    //Re-open the file handle and append to the end.  
    virtual void OpenForAppend() 
    {
        OpenLog(eOpenMode::eomAppend, NULL);
    }
    
    //Close the file handle.  This is mainly for Watson upload.
    virtual void Close() 
    {
        // Don't write logutil's footer
        ::EnterCriticalSection(&s_cs);
        s_lLogUtilRefCount--;
        if (0 == s_lLogUtilRefCount)
        {
            LogUninitialize(FALSE);
        }
        ::LeaveCriticalSection(&s_cs);
    }

public:  // Part of ILogger interface
    CPath GetFilePath()
    {
        return CPath(s_filename);
    }

public:  // Part of ILogger interface
    //------------------------------------------------------------------------------
    // RenameLog
    //
    // Rename the log without changing the extension.
    // Take a path without extension and use the current cache extension.
    //------------------------------------------------------------------------------
    virtual bool RenameLog (const CPath & pthNewNameWithoutExtension)
    {
        HRESULT hr = S_OK;

        // Expand any environment variables like %TEMP% and add the extension.
        CString strExpandedFileSpec = ExpandToUsersTempIfNecessary(pthNewNameWithoutExtension) + GetFileExtension();

        ::EnterCriticalSection(&s_cs);

        hr = LogRename(strExpandedFileSpec);
        if (FAILED(hr))
        {
            ::LeaveCriticalSection(&s_cs);
            return false;
        }

        s_filename = strExpandedFileSpec;

        ::LeaveCriticalSection(&s_cs);

        return true;
    }

public: // fork / merge
    virtual ILogger* Fork()
    {
        return this;
    }
    virtual void Merge(ILogger* logger)
    {   
    }

    virtual void DeleteFork(ILogger* logger)
    {
    }

    virtual CPath GetForkedName()
    {
        return CPath(s_filename);
    }

public: // Not part of ILogger interface
    LONGLONG GetLogSize() const
    {
        LONGLONG llSize = 0;

        ::EnterCriticalSection(&s_cs);
        // Todo: what to do with return value?
        FileSize(s_filename, &llSize);
        ::LeaveCriticalSection(&s_cs);

        return llSize;
    }

private: // template method design pattern
    virtual CString BeforeLog()
    {
        return GetFormattedTimeStamp() + L" ";
    }

    virtual CString AfterLog () { return L"\r\n"; }
    CString GetFileExtension()  { return s_fileExtension; }

protected: // utility methods for derived classes
    void Write(const CString& string)
    {
        ::LogString(REPORT_STANDARD, CStringA(string));
    }

private:
    enum eOpenMode
    {
        eomOverwrite,
        eomAppend
    };

    void OpenLog(eOpenMode eomMode, LPCWSTR headerFile)
    {
        HRESULT hr = S_OK;
        BOOL fAppend = FALSE;

        // cases:
        // 1.  nothing passed to ctor
        // 2.  empty string passed to ctor
        // 3.  %TEMP%\blah
        // 4.  c:\full-path\blah
        // 5.  c:\bar\%USERNAME%\foo\blah

        // convert cases 1 and 2 into 3
        ::EnterCriticalSection(&s_cs);
        if (!s_fLogNameInitialized)
        {
            CString tempFolder = ProcessUtils::GetTempFolderOfDelevatedUser();
            CString defaultFileExtension = GetFileExtension().MakeLower();
            CString fileExtension = CPath(s_filename).GetExtension().MakeLower();

            if ( tempFolder.IsEmpty() )
            {
                ::LeaveCriticalSection(&s_cs);
                throw CLogCreationException(L"Cannot get valid temp folder");
            }
            if ( !ProcessUtils::CanFileBeCreatedAndDeletedInFolder(tempFolder, s_filename) )
            {
                ::LeaveCriticalSection(&s_cs);
                throw CLogCreationException(L"Cannot create file or delete file in Temp directory " + tempFolder);
            }

            if (fileExtension != L".htm" && fileExtension != defaultFileExtension)
            {
                s_filename += defaultFileExtension;
            }
        }
        ::LeaveCriticalSection(&s_cs);

        if (NULL != headerFile && !FileExistsEx(headerFile, NULL))
        {
            // If the header doesn't exist, throw - caller should catch this, and try again with a simpler logging method
            throw CLogCreationException(L"Header file doesn't exist");
        }
        
        ::EnterCriticalSection(&s_cs);
        if (0 == s_lLogUtilRefCount)
        {
            if (eOpenMode::eomAppend == eomMode)
            {
                fAppend = TRUE;

                if (NULL != headerFile && FileExistsEx(s_filename, NULL))
                {
                    ::LeaveCriticalSection(&s_cs);
                    throw CLogCreationException(L"Cannot add a header and append to an existing log at the same time");
                }
            }

            if (NULL != headerFile)
            {
                hr = FileEnsureCopy(headerFile, s_filename, TRUE);
                if (FAILED(hr))
                {
                    ::LeaveCriticalSection(&s_cs);
                    throw CUnableToOpenLogFileException(s_filename);
                }
                fAppend = TRUE;
            }

            ::SetFileAttributesW(s_filename, FILE_ATTRIBUTE_NORMAL);

            // last parameter FALSE to avoid writing logutil's header
            s_fLogNameInitialized = TRUE;
            hr = LogInitialize(NULL, s_filename, NULL, fAppend, FALSE);
            if (FAILED(hr))
            {
                ::LeaveCriticalSection(&s_cs);
                throw CUnableToOpenLogFileException(s_filename);
            }
        }
        ::LeaveCriticalSection(&s_cs);

        ::InterlockedIncrement(&s_lLogUtilRefCount);
    }

    static bool CreateSecurityAttributes(CSecurityAttributes* psa)
    {
        CDacl dacl;
        dacl.AddDeniedAce(Sids::Guests(), GENERIC_ALL);              // deny Guests everything (always a good policy)

        // Get Sid for current user
        CAccessToken procToken;
        CSid sidUser;
        procToken.GetEffectiveToken(TOKEN_QUERY);
        procToken.GetUser(&sidUser);
        dacl.AddAllowedAce(sidUser, GENERIC_ALL);       // allow current user r/w access

        dacl.AddAllowedAce(Sids::AuthenticatedUser(), GENERIC_READ); // allow authenticated users read access
        dacl.AddAllowedAce(Sids::Admins(), GENERIC_ALL);             // allow admins r/w access

        CSecurityDesc sd;
        sd.SetDacl(dacl);
        psa->Set(sd);

        return true;
    }

    static CString ExpandToUsersTempIfNecessary(const CString& filename)
    {
        CString upper(filename);
        upper.MakeUpper();
        int index = upper.Find(L"%TEMP%");
        if (index != -1)
        {
            // call new improved temp folder handling
            CPath tempFolder = ProcessUtils::GetTempFolderOfDelevatedUser();
            tempFolder.Append(filename.Mid(index + 6)); // 6 == number of chars in "%temp%
            return CString(tempFolder);
        }
        return filename;
    }

};

}
