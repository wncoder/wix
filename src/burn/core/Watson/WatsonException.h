//-------------------------------------------------------------------------------------------------
// <copyright file="WatsonException.h" company="Microsoft">
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

#include "WatsonDefinition.h"
#include "VersionUtil.h"
#include "LogSignatureDecorator.h"
#include "ux\ux.h"
#include "ux\NullMetrics.h"
#include "werWatson.h"

const int EMERGENCYSTACKSIZE = 4096*32;

namespace IronMan
{

//Templatize to enable unit testing.
template <typename OSHELPER = OSHelper>
class WatsonException
{
public:
    static WatsonException*& WatsonExceptionStatic()
    {
        static WatsonException* s_WatsonException = NULL;
        return s_WatsonException;
    }

    //------------------------------------------------------------------------------
    // InitialiseExceptionHandler
    //
    // Initialize the WatsonException class and setup the unhandled exception handler
    //------------------------------------------------------------------------------
    static VOID InitialiseExceptionHandler()
    {
        WatsonExceptionStatic() = new WatsonException();
        WatsonExceptionStatic()->InitializeExceptionHandler();
    }

protected: // test-hook
    static void InitializeExceptionHandler()
    {
        // function typedef for "SetThreadStackGuarantee" in Windows Server 2003 and above
        typedef BOOL (__stdcall *SetThreadStackGuaranteePtr)(PULONG);

        // Set the top-level UEF
        // If the OS supports SetThreadStackGuarantee, then we can use the inner DwExceptionFilterEx directly,
        // otherwise we need to wrap the handler it in our emergency stack creation code.
        HMODULE hKernel32 = GetModuleHandle(TEXT("kernel32.dll"));
        if (NULL != hKernel32)
        {
            SetThreadStackGuaranteePtr pfnSetStackGuarantee = reinterpret_cast< SetThreadStackGuaranteePtr >(GetProcAddress(hKernel32, "SetThreadStackGuarantee"));
            ULONG ulStackGuarantee = EMERGENCYSTACKSIZE;
            if (pfnSetStackGuarantee)
            {
                pfnSetStackGuarantee(&ulStackGuarantee);
            }
        }

        __pragma ( prefast(suppress: 28725, "approved") )
        SetUnhandledExceptionFilter(DwExceptionFilterEx);
    }

    //Constructor
protected: // test-hook
    WatsonException(){}
public:
    virtual ~WatsonException() {} // only my unit tests ever call this

protected: // test-hook
    
    //------------------------------------------------------------------------------
    // DwExceptionFilterEx
    //
    // The registered Watson unhandled exception handler
    //
    // Note:
    // 1.  This function prepare and upload the user experience data log before Watson reporting.
    //     This is to ensure the user experience data contains an entry for crash data too.
    //     
    //------------------------------------------------------------------------------
    static LONG WINAPI DwExceptionFilterEx(PEXCEPTION_POINTERS pExceptionPointers)
    {
        /*
        If it's a breakpoint, running under the debugger, or pExceptionPointers
        is NULL, then let the standard windows exception dialog come up; otherwise
        invoke Watson
        */
        if (NULL == pExceptionPointers || NULL == pExceptionPointers->ExceptionRecord)
            return EXCEPTION_CONTINUE_SEARCH;

        if (EXCEPTION_BREAKPOINT == pExceptionPointers->ExceptionRecord->ExceptionCode)
            return EXCEPTION_CONTINUE_SEARCH;

        ILogger *pILogger = WatsonData::WatsonDataStatic()->GetLogger();
        IMASSERT2(NULL != pILogger, L"A NULL logger pointer has been passed, or it was never set");
        if (NULL != pILogger)
        {
            pILogger->Close();
        }

        //Do all the user experience data stuff here
        NullMetrics metrics;
        Ux uxLogger(IronMan::NullLogger::GetNullLogger(), metrics);
        uxLogger.GetDefaultUxLogger();

        //Ensure that user experience data log the same bucketing as Watson. 
        uxLogger.RecordCrashErrorDatapoints(pExceptionPointers->ExceptionRecord->ExceptionCode, true);
        uxLogger.EndAndSendUxReport();
        
        // If LaunchWatson returns zero, then Watson could not be launched so
        // just execute the normal handler (which most likely ends the process)
        // If LaunchWatson returns non-zero, then Watson was launched and it
        // allowed the application to continue. In this case we expect that the
        // user selected "Debug" and there's a JIT debugger registered. We need
        // to return EXCEPTION_CONTINUE_SEARCH so that the exception is re-raised
        // as a "second chance" exception and the JIT debugger will launch.
        WatsonExceptionStatic()->LaunchWatson(pExceptionPointers);
        return EXCEPTION_CONTINUE_SEARCH;
    }

protected:  // test-hook
    /*--------------------------------------------------------------------------------------
    Function: LaunchWatson

    Returns:
    If the Watson UI was not be shown (we're debugging, or something else went wrong),
    this returns 0

    If the Watson UI was shown and the user chose to terminate the process, then this
    never returns. This process was already killed.
    ---------------------------------------------------------------------------------------*/
    HRESULT LaunchWatson(LPEXCEPTION_POINTERS pExceptionPointers)
    {
        HRESULT hr = S_OK;
        OSHELPER os;

        /*
        If it's a breakpoint, running under the debugger, or pExceptionPointers
        is NULL, then let the standard windows exception dialog come up; otherwise
        invoke Watson
        */
        if (NULL == pExceptionPointers || NULL == pExceptionPointers->ExceptionRecord)
            return E_POINTER;

        if (EXCEPTION_BREAKPOINT == pExceptionPointers->ExceptionRecord->ExceptionCode)
            return S_OK;

#if 0
        ::MessageBox(NULL,L"Hello",NULL,NULL);
#endif

        if (os.IsVistaAndAbove())
        {
            CString arstrParams[WER_MAX_PARAM_COUNT];
            //Step 1: Set the bucketting parameters

            C_ASSERT(WER_MAX_PARAM_COUNT >= 9);

            //P1 is the PackageName
            arstrParams[0] = WatsonData::WatsonDataStatic()->GetGeneralAppName();

            //P2 is the Package Version
            arstrParams[1] = WatsonData::WatsonDataStatic()->GetPackageVersion();

            //P3 is the Ironman Version
            arstrParams[2] = CVersionUtil::GetExeFileVersion();

            //P4 is the operation type (Install | Uninstall | Abort)
            CString csOperation;
            csOperation.Format(L"%d", WatsonData::WatsonDataStatic()->GetOperation());
            arstrParams[3] = csOperation;

            //P5 is the item at fault
            arstrParams[4] = WatsonData::WatsonDataStatic()->GetCurrentItemName();

            //P6 is Current Flag
            CString csCurrentFlag;
            csCurrentFlag = WatsonData::WatsonDataStatic()->GetCurrentActionString() + L"_" + WatsonData::WatsonDataStatic()->GetCurrentPhaseString() + L"_" + WatsonData::WatsonDataStatic()->GetUiModeString() + L"_" + L"Crash";
            arstrParams[5] = csCurrentFlag;

            //P7 is the return code from the install
            CString csExceptionCode;
            csExceptionCode.Format(L"0x%x", pExceptionPointers->ExceptionRecord->ExceptionCode);
            arstrParams[6] = csExceptionCode;

            //P8 is the Result Detail
            arstrParams[7] = WatsonData::WatsonDataStatic()->GetInternalErrorString();

            //P9 is Extra Item information about the failure
            arstrParams[8] = WatsonData::WatsonDataStatic()->GetCurrentStep();

            WerWatson wer;
            hr =  wer.SendReport( WatsonData::WatsonDataStatic()->GetGeneralAppName()
                                , WatsonData::WatsonDataStatic()->IsQueue()
                                , WatsonData::WatsonDataStatic()->GetFilesToKeep().ConvertToWatsonString()
                                , arstrParams
                                , pExceptionPointers
                                , WatsonData::WatsonDataStatic()->GetWatsonHeader());

            if (S_OK == hr)
            {
                //Return 1 to be consistent with Watson downlevel behvaiour.
                ExitProcess(1);
            }
        }

        return S_OK;
    }
};

// decorate the logger with Watson-specific functionality
class WatsonLoggerDecorator : public ILogger
{
    ILogger& m_logger;
public:
    WatsonLoggerDecorator(ILogger& logger)
        : m_logger(logger)
    {}
    virtual ~WatsonLoggerDecorator() {}

public: // ILogger interface
    virtual void Log(LoggingLevel lvl, LPCWSTR szLogMessage)
    {
        if (lvl == InternalUseOnly)
        {
#ifdef IRONMAN_FUNCTIONNAMEINLOGFILE
            CString csLogFile = ProcessLogFile(LogHelper::RemoveFunctionNameFromDecoration(szLogMessage));
#else
            CString csLogFile = ProcessLogFile(szLogMessage);
#endif
            if (!csLogFile.IsEmpty())
            {
                WatsonData::WatsonDataStatic()->AddFileToKeep(csLogFile);
            }
        }
        else
        {
            m_logger.Log(lvl, szLogMessage);
        }
    }

    virtual void LogFinalResult(LPCWSTR szLogMessage)
    {
        m_logger.LogFinalResult(szLogMessage);
    }
    virtual void PopSection(LPCWSTR str) { m_logger.PopSection(str); }
    virtual void PushSection(LPCWSTR strAction, LPCWSTR strDescription) {  m_logger.PushSection(strAction, strDescription); }
    virtual CPath GetFilePath() {  return m_logger.GetFilePath(); }
    virtual bool RenameLog(const CPath & pthNewName) { return m_logger.RenameLog(pthNewName); }
    virtual void OpenForAppend() { m_logger.OpenForAppend(); }
    virtual void Close() { m_logger.Close(); }

    virtual void BeginLogAsIs(LoggingLevel lvl, CString szLogStart) { m_logger.BeginLogAsIs(lvl, szLogStart); }
    virtual void LogLine(LPCWSTR szLine) { m_logger.LogLine(szLine); }
    virtual void LogStartList() { m_logger.LogStartList(); }
    virtual void LogListItem(LPCWSTR item) { m_logger.LogListItem(item); }
    virtual void LogEndList() { m_logger.LogEndList(); }
    virtual void EndLogAsIs() { m_logger.EndLogAsIs(); }

    virtual ILogger* Fork() 
    { 
        try
        {
            return new WatsonLoggerDecorator(*m_logger.Fork());
        }
        catch (...)
        {
            // return Un Decorated Null logger
            return &IronMan::NullLogger::GetNullLogger();
        }
    }

    virtual void Merge(ILogger* fork) { m_logger.Merge(fork); }
    virtual void DeleteFork(ILogger* fork)
    {
        m_logger.DeleteFork(fork);
    }
    virtual CPath GetForkedName() { return m_logger.GetForkedName(); }

private:
    //Parse the environment variable if it exists and determine the log file.
    CString ProcessLogFile(const CString& csInputLogFile)
    {
        CString OutputLogFile;
        OutputLogFile.Empty();
        if (!csInputLogFile.IsEmpty())
        {
            CSystemUtil::ExpandEnvironmentVariables(csInputLogFile, OutputLogFile);
            if (CPath(OutputLogFile).FileExists())
            {
                CString log;
                log.Format(L"Log File %s exists and will be added to the Watson upload list", OutputLogFile);
                LOG(m_logger, ILogger::Verbose, log);   
            }
            else
            {
                CString log;
                log.Format(L"Log File %s does not yet exist but may do at Watson upload time", OutputLogFile);
                LOG(m_logger, ILogger::Verbose, log);
                // We don't call OutputLogFile.Empty() here because there may second opportunity to get the log
            }            	
        }
        else
        {
            LOG(m_logger, ILogger::Warning,  L"Log File name provided is empty");
        }
        return OutputLogFile;
    }
};

}
