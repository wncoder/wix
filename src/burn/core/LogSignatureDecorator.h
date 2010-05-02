//-------------------------------------------------------------------------------------------------
// <copyright file="LogSignatureDecorator.h" company="Microsoft">
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

#ifdef IRONMAN_FUNCTIONNAMEINLOGFILE
#define LOG(logger, lvl, log) (logger).Log(lvl, IronMan::LogHelper::DecorateFunctionName(CString(__FUNCTION__)) + log)
#else
#define LOG(logger, lvl, log) (logger).Log(lvl, log)
#endif


namespace IronMan
{

#ifdef IRONMAN_FUNCTIONNAMEINLOGFILE
    struct LogHelper
    {
    private:	const static char chStart = 0x1;
                const static char chEnd = 0x2;

    public:
        static CString DecorateFunctionName(const CString& sig)
        {
            CString str;			
            str.Format(L"FUNCTIONNAME%c%s%c", chStart, CString(sig), chEnd);
            return str;
        }
        static CString GetFunctionNameFromDecoration(const CString& decoration)
        {			
            CString cs(decoration);
            CString str;
            str.Format(L"FUNCTIONNAME%c", chStart);
            int start = cs.Find(str);
            if (start == -1)
                return L""; // not decorated

            start += 13; // number of wchars in L"FUNCTIONNAME<"

            int end = cs.Find(chEnd, start);
            return cs.Mid(start, end - start);
        }
        static CString RemoveFunctionNameFromDecoration(const CString& decoration)
        {
            CString cs(decoration);			
            CString sig = GetFunctionNameFromDecoration(cs);
            cs.Replace(DecorateFunctionName(sig), L"");
            return cs;
        }
    };
#endif

    class EnterAndExitSignatureDecorator
    {
        CString& m_ref;
        CString m_sig;
        ILogger& m_logger;
    public:
        EnterAndExitSignatureDecorator(ILogger& logger, const CString& sig, CString& ref)
            : m_logger(logger)
            , m_sig(sig)
            , m_ref(ref)
        {
            m_logger.PushSection(L"Entering Function", m_sig);
        }
        ~EnterAndExitSignatureDecorator()
        {
#ifdef IRONMAN_FUNCTIONNAMEINLOGFILE
            m_logger.Log(ILogger::Verbose, IronMan::LogHelper::DecorateFunctionName(m_sig) + L" exiting function/method");
            m_logger.PopSection(IronMan::LogHelper::DecorateFunctionName(m_sig) + m_ref);
#else
            m_logger.Log(ILogger::Verbose, L" exiting function/method");
            m_logger.PopSection(m_ref);
#endif

        }
    };

    class PushAndPopLogSection
    {
        CString m_action;
        CString& m_ref;
        ILogger& m_logger;
    public:
        PushAndPopLogSection(ILogger& logger, LPCWSTR strAction, LPCWSTR strSectionWording, CString& ref)
            : m_logger(logger)
            , m_ref(ref)
            , m_action(strAction)
        {
            m_logger.PushSection(strAction, strSectionWording);
        }
        ~PushAndPopLogSection()
        {
            m_logger.PopSection(m_action + m_ref);
        }
    };

    class LogEx
    {
        const char* func;
        LogEx();
    public:
        LogEx(const char* f) : func(f) {}
    
        void LogV(ILogger& logger, ILogger::LoggingLevel level, LPCWSTR szMsg, ...)
        {
            CString strMsg;
            
            va_list argptr;
            va_start(argptr, szMsg);
            strMsg.FormatV(szMsg,argptr);
            va_end(argptr);
    
#ifdef IRONMAN_FUNCTIONNAMEINLOGFILE
            logger.Log(level, IronMan::LogHelper::DecorateFunctionName(CString(func)) + strMsg);
#else
            logger.Log(level, strMsg);
#endif
        }
    };


}

#define ENTERLOGEXIT(logger, ref) IronMan::EnterAndExitSignatureDecorator eaesd_logger(logger, CString(__FUNCTION__), ref)

#define MAKEPAPLS2(x,y) x##y
#define MAKEPAPLS1(x,y) MAKEPAPLS2(x,y)
#define MAKEPAPLS()     MAKEPAPLS1(papls, __LINE__)

#define PUSHLOGSECTIONPOP(logger, action, log, ref) IronMan::PushAndPopLogSection MAKEPAPLS()(logger, action, log, ref);
#define LOGEX LogEx(__FUNCTION__).LogV
