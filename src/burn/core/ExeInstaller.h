//-------------------------------------------------------------------------------------------------
// <copyright file="ExeInstaller.h" company="Microsoft">
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
//    The set of classes for install an Exe itemtype.
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once

#include "schema\EngineData.h"
#include "Interfaces\IPerformer.h"
#include "Interfaces\ILogger.h"
#include "common\MsgWaitForObject.h"
#include "common\SystemUtil.h"
#include "Ux\Ux.h"
#include "common\MsiUtils.h"
#include "common\ProcessUtils.h"
#include "FindFile.h"
#include "Action.h"

namespace IronMan
{

class ExeInstallerBase : public AbortPerformerBase, public Action
{
//Make it protected so that derive class can access it to set datapoints.
protected:
    Ux& m_uxLogger;
    ILogger& m_logger;

private:
    const ExeBase& m_exe;
    HANDLE m_hProcess;
    DWORD m_dwRetryCount;

protected:
    const ExeBase& GetExe() const 
    { 
        return m_exe;
    }

    ILogger& GetLogger() 
    { 
        return m_logger;
    }

    Ux& GetUx() 
    { 
        return m_uxLogger;
    }

public:
    ExeInstallerBase(
        const ExeBase& exe
        , IProgressObserver::State action
        , ILogger& logger
        , Ux& uxLogger)
        : Action(action)
        , m_exe(exe)
        , m_logger(logger)
        , m_uxLogger(uxLogger)
        , m_hProcess(NULL)
        , m_dwRetryCount(0)
    {
        LOG(m_logger, ILogger::Verbose, L"Created new ExePerformer for Exe item");
    }

    virtual ~ExeInstallerBase(void)
    {
        m_hProcess = NULL;
    }

private:
    virtual bool ExeReportsProgress(void) const
    {
        return false;
    }

private:  //For Unit Testing
    virtual void GetSystemTime(__out LPSYSTEMTIME lpSystemTime)
    {
        ::GetSystemTime(lpSystemTime);
    }


private:
    const UxEnum::technologyEnum GetExeEnum() const
    {
        if (m_exe.GetExeType().IsCartmanExe())
        {
            return UxEnum::tCartmanExe;
        }
        else if (m_exe.GetExeType().IsIronManExe())
        {
            return UxEnum::tIronManExe;
        }
        else if (m_exe.GetExeType().IsMsuPackageExe())
        {
            return UxEnum::tMSUExe;
        }
        else if (m_exe.GetExeType().IsUnknownExe())            
        {
            return UxEnum::tExe;
        }
        else if (m_exe.GetExeType().IsLocalExe())
        {
            return UxEnum::tLocalExe;
        }
        else
        {
            IMASSERT2(0, L"Exe types are not yet implemented");
            throw E_NOTIMPL;
        }
    }   

protected: // Because derived classes may want to wrap the call, as in the case of the MsuInstaller
    //------------------------------------------------------------------------------
    // PerformAction
    //
    // Note: 
    // 1.  We are not checking and setting ABORT because we are leaving it to the 
    //     outer layer to handle it.  
    //
    //-------------------------------------------------------------------------------
    virtual void PerformAction(IProgressObserver& observer)
    {
        CString section = L" complete";
        CString localPathName = m_exe.GetName();

        observer.OnStateChangeDetail(GetAction(), m_exe.GetCanonicalTargetName());
        PUSHLOGSECTIONPOP(m_logger, L"Action", L"Performing Action on Exe at " + localPathName, section);
        SYSTEMTIME actionStartTime;
        GetSystemTime(&actionStartTime);

        m_uxLogger.StartRecordingItem(m_exe.GetOriginalName()
                                        , UxEnum::pInstall
                                        , GetUxAction(GetAction())                                        
                                        , GetExeEnum());
        DWORD exitCode = Launch(observer);
        m_uxLogger.StopRecordingItem(m_exe.GetOriginalName()
                                     , 0
                                     , HRESULT_FROM_WIN32(exitCode)
                                     , WatsonData::WatsonDataStatic()->GetInternalErrorString()     
                                     , m_dwRetryCount++);

        // Process exit code and get appropriate HRESULT
        HRESULT hr = ProcessReturnCode(exitCode);

        CString strResult;
        CSimpleArray<CString> logFiles;
        if (m_exe.GetLogFileHint().GetLength() == 0)
        {
            CString log;
            log.Format(L"%s - Exe installer does not provide a log file name", m_exe.GetName());
            LOG(GetLogger(), ILogger::Verbose, log);
        }
        else
        {
            FILETIME actionStartFileTime;
            ::SystemTimeToFileTime(&actionStartTime, &actionStartFileTime);

            GenerateLogFilesList(actionStartFileTime, logFiles);
            // Add the Exe log file to the list of files for Watson to send
            if (0 < logFiles.GetSize())
            {
                CString strMessage(L"Exe log file(s) :");
                LOG(GetLogger(), ILogger::Verbose, strMessage);

                for(int i = 0; i < logFiles.GetSize(); ++i)
                {
                    LOG(GetLogger(), ILogger::Verbose, logFiles[i]);
                    // Add the Exe log file to the list of files for Watson to send
                    LOG(GetLogger(), ILogger::InternalUseOnly, logFiles[i]);
                }
            }
            else
            {
                // Couldn't find log file given in hint - carry on. Logging removed since it was found to confuse people
            }
        }
        
        switch(exitCode)
        {
        case ERROR_UNKNOWN_PRODUCT: // not an error if the product isn't installed
        case ERROR_UNKNOWN_PATCH:
            hr = HRESULT_FROM_WIN32(ERROR_SUCCESS);
            strResult.Format(L"Exe (%s) succeeded (but does not apply to any products on this machine)", localPathName);
            LOG(m_logger, ILogger::Result, strResult);
            break;
        case ERROR_SUCCESS:
            //Logging Result
            strResult.Format(L"Exe (%s) succeeded.", localPathName);
            LOG(m_logger, ILogger::Result, strResult);
            break;
        case ERROR_SUCCESS_REBOOT_REQUIRED:
            //Logging Result
            strResult.Format(L"Exe (%s) succeeded and requires reboot.", localPathName);
            LOG(m_logger, ILogger::Result, strResult);
            break;
        case ERROR_SUCCESS_RESTART_REQUIRED:
            strResult.Format(L"Exe %s returned success, but changes will not be effective until the service is restarted.", localPathName);
            LOG(m_logger, ILogger::Result, strResult);
            break;
        case ERROR_SUCCESS_REBOOT_INITIATED:              
            strResult.Format(L"Exe %s has initiated a restart.", localPathName);
            LOG(m_logger, ILogger::Result, strResult);
            break;
        default: 
            {
                CComBSTR bstrErrorString;
                CSystemUtil::GetErrorString(hr, bstrErrorString);
                //Logging Result
                strResult.Format(L"Exe (%s) failed with 0x%x - %s.", localPathName, hr, bstrErrorString);
                LOG(m_logger, ILogger::Result, strResult);
                break;
            }
        }       

        CString strLogResult;
        for(int i = 0; i < logFiles.GetSize(); ++i)
        {
            LOG(m_logger, ILogger::Result, FormatLogPathString(m_logger,logFiles[i]));
        }

        // When user cancelled but install succeeded, we can launch Rollback.
        if (HasAborted() && MSIUtils::IsSuccess(hr))
        {           
            Rollback(NullProgressObserver::GetNullProgressObserver());  //we can use NullObserver because we are not showing progress anyway.
            // Ignore OnProgress's return value here because we're aborting and rolling back
            observer.OnProgress(0);  //Rollback the progress bar            
            // Returning hr instead of E_ABORT as CompositePerformer will handle this with SetErrorWithAbort()
            observer.Finished(hr);
        }
        else 
        {
            if (!MSIUtils::IsSuccess(hr))
            {
                CString log;
                log.Format(L"PerformOperation on exe returned exit code %u (translates to HRESULT = 0x%x)", exitCode, hr);
                LOG(m_logger, ILogger::Error, log);
            }        
            observer.Finished(hr);
        }   
    }

private:

    // Process exit code and get appropriate HRESULT
    virtual HRESULT ProcessReturnCode(DWORD& exitCode)
    {
        EnsureExitCodeIsAnMSIErrorCode(exitCode);
        return HRESULT_FROM_WIN32(exitCode);
    }

    void EnsureExitCodeIsAnMSIErrorCode(DWORD& exitCode)
    {
        if (E_ABORT == exitCode)
        {
            return;
        }

        DWORD dwSavedExitCode = exitCode;
        if (HRESULT_FACILITY(dwSavedExitCode) == FACILITY_WIN32)
        {
            exitCode =  SCODE_CODE(exitCode);
        }       

        //Logging of these return code is done in PerformAction().
        if (MSIUtils::IsSuccess(HRESULT_FROM_WIN32(exitCode)))
        {
            return;
        }

        // For an Exe, these two cases are not errors
        if (exitCode == ERROR_UNKNOWN_PRODUCT || 
            exitCode == ERROR_UNKNOWN_PATCH)
        {
            return;
        }

        //Anything after this line is an error.
        int error = GetInternalError(exitCode);
        CString strCurrentItemStep = GetCurrentItemStep();
        WatsonData::WatsonDataStatic()->SetInternalErrorState(error, strCurrentItemStep);

        if (exitCode == ERROR_INVALID_DATA || 
            exitCode == ERROR_INVALID_PARAMETER ||
            exitCode == ERROR_CALL_NOT_IMPLEMENTED ||
            exitCode == ERROR_APPHELP_BLOCK)
        {
            return;
        }

        //Translate the error when
        // - it is not an MSI error or
        // - the exist code has been changed by the SCODE_CODE and it is not Win32 code.  This is to take care of the scenario where return code has not facility and/or severity.
        if ((exitCode < 1600 || exitCode > 1699) || (HRESULT_FACILITY(dwSavedExitCode) != FACILITY_WIN32 && (dwSavedExitCode != exitCode)))
        {
            CString log;
            CComBSTR bstrErrorString;
            CSystemUtil::GetErrorString(dwSavedExitCode, bstrErrorString);
            log.Format(L"Original exit code: %s returned non-MSI error code: 0x%x - %s ",m_exe.GetName(), dwSavedExitCode, bstrErrorString);
            LOG(m_logger, ILogger::Error, log);

            exitCode = ERROR_INSTALL_FAILURE;
            CSystemUtil::GetErrorString(exitCode, bstrErrorString);
            log.Format(L"Modified exit code: %s returned error code: 0x%x - %s ",m_exe.GetName(), exitCode, bstrErrorString);
            LOG(m_logger, ILogger::Error, log);
        }
    }

private:
    class SigmoidalProgress : public IProgressObserver
    {
        const double m_xShift;
        const double m_slopeModifier;
        const double m_yIntercept;
        const DWORD m_initialTickCount;
        IProgressObserver& m_actualObserver;
        ILogger& m_logger;

        const bool m_bExeReportsProgress;
        unsigned char m_soFar;
        const bool& m_abort;

        // Hide default constuctor
        SigmoidalProgress();

        // Hide assignemnt operator
        SigmoidalProgress& operator= (const SigmoidalProgress&);

        static double ComputeYIntercept(double xShift)
        {
            double denominator = 1 + exp(-xShift);
            return (1 / denominator);
        }

        SigmoidalProgress(double xShift, double slopeModifier, bool bExeReportsProgress, IProgressObserver& actualObserver, const bool& abort, ILogger& logger) 
            : m_actualObserver(actualObserver)
            , m_logger(logger)
            , m_initialTickCount(GetTickCount())
            , m_xShift(xShift)
            , m_slopeModifier(slopeModifier) 
            , m_yIntercept(ComputeYIntercept(xShift))
            , m_bExeReportsProgress(bExeReportsProgress)
            , m_abort(abort)
            , m_soFar(0)
        {
        }
    public:
        // OK to allow default copy constuctor since there is no deep copy needed
        //SigmoidalProgress(const SigmoidalProgress&);
       
        static SigmoidalProgress CreateSigmoidalProgressThatHandlesActualProgress(IProgressObserver& observer, const bool& abort, ILogger& logger) 
        {
            return SigmoidalProgress(0.85, 0.01, true, observer, abort, logger);
        }
        static SigmoidalProgress CreateSigmoidalProgressWhenNoProgressIsReported(ULONGLONG downloadSize, const bool& abort, ILogger& logger) 
        {
            (void)downloadSize;
            double slopeModifier = 0.01;
            return SigmoidalProgress(1.5, slopeModifier, false, NullProgressObserver::GetNullProgressObserver(), abort, logger);
        }
        int OnProgress(unsigned char soFar)
        {
            m_soFar = soFar;
            return IDOK;
        }

        int OnProgressDetail(unsigned char soFar)
        {
            return IDOK;
        }

        void Finished(HRESULT hr)
        {
            int nResult = IDOK;

            nResult = m_actualObserver.OnProgress(255);
            // If the passed-in hresult is failure, preserve the error - otherwise check for any cancellation or error returned by OnProgress
            if (SUCCEEDED(hr))
            {
                hr = HRESULT_FROM_VIEW(nResult);
                if (FAILED(hr))
                {
                    LOG(m_logger, ILogger::Error, L"User interface commanded engine to abort");
                }
            }

            m_actualObserver.Finished(hr);
        }
        virtual void OnStateChange(State enumVal)  { m_actualObserver.OnStateChange(enumVal); }
        virtual void OnStateChangeDetail (const State enumVal, const CString changeInfo) 
        { 
            m_actualObserver.OnStateChangeDetail(enumVal, changeInfo);
        }
        virtual void OnRebootPending()
        {
            m_actualObserver.OnRebootPending();
        }
        void operator() (void)
        {
            int nResult = IDOK;
            HRESULT hr = S_OK;

            if (HasAborted())
                return;

            unsigned char soFarOnSigmodialCurve = GetPrgressFromSigmoidalCurve();
            
            if (m_bExeReportsProgress) 
            {
                // For Exe's reporting true progress, this forumula below is not right.
                // We should fix/tune the parameters of sigmoidal progress. 
                // Till then, we should report true progress as is and not soFarAverage.
                //unsigned char soFarAverage = (soFarOnSigmodialCurve + m_soFar) / 2;
                nResult = m_actualObserver.OnProgress(m_soFar);
            }
            else
            {
                nResult = m_actualObserver.OnProgress(soFarOnSigmodialCurve);
            }

            hr = HRESULT_FROM_VIEW(nResult);
            if (FAILED(hr))
            {
                LOG(m_logger, ILogger::Error, L"User interface commanded engine to abort");
                m_actualObserver.Finished(hr);
            }
        }
    private:
        bool HasAborted() const 
        { 
            return m_abort;
        }
        unsigned char GetPrgressFromSigmoidalCurve(void)
        {
            double x = ((double)(GetTickCount() - m_initialTickCount)) / 500;

            double denominator = 1 + exp(- (m_xShift + x * m_slopeModifier));
            double y = 1 / denominator;
            y -= m_yIntercept;
            y *= 255;
            y /= (1 - m_yIntercept);

            return (unsigned char)y;
        }
    };

public:
    HANDLE GetProcess(void)
    {
        return m_hProcess;
    }
    virtual DWORD Launch(IProgressObserver& observer)
    {
        STARTUPINFO startupInfo;
        ::ZeroMemory(&startupInfo, sizeof(STARTUPINFO));
        startupInfo.cb = sizeof( STARTUPINFO );
        ProcessUtils::CProcessInformation processInformation;

        const CPath& exeFullName = GetExecutable();

        CPath workingDir(exeFullName);
        workingDir.RemoveFileSpec();

        CPath exeName(exeFullName);
        exeName.StripPath();
        exeName.QuoteSpaces();

        CString commandLine;
        commandLine.Format(L"%s %s", exeName, GetCommandLine());

        PreCreateProcess(commandLine);

        LOG(m_logger, ILogger::Verbose, L"Launching CreateProcess with command line = " + commandLine);

        if (HasAborted())
        {
            return HRESULT_CODE(E_ABORT);
        }

        bool bCreateProcessSucceded = CreateProcess(exeFullName,
                                                      commandLine.GetBuffer(),
                                                      NULL, // lpProcessAttributes
                                                      NULL, // lpThreadAttributes
                                                      TRUE, // bInheritHandles
                                                      CREATE_NO_WINDOW, // dwCreationFlags
                                                      NULL, // lpEnvironment
                                                      workingDir,
                                                      &startupInfo,
                                                      &processInformation);
        if (bCreateProcessSucceded == false)
        {   
            DWORD err = GetLastError();
            CComBSTR errMsg;
            CSystemUtil::GetErrorString(err, errMsg);
            LOG(m_logger, ILogger::Error, L"Error launching CreateProcess with command line = " + commandLine + errMsg);
            LOG(m_logger, ILogger::Error, L"      CreateProcess returned error = " + CString(errMsg));

            return err;
        }

        
        SigmoidalProgress sigmoidalProgress = 
            ExeReportsProgress() ? SigmoidalProgress::CreateSigmoidalProgressThatHandlesActualProgress(observer, AbortFlagRef(), m_logger)
                                 : SigmoidalProgress::CreateSigmoidalProgressWhenNoProgressIsReported(m_exe.GetItemSize(), AbortFlagRef(), m_logger);

        IProgressObserver& sigmoidalProgressOberver = sigmoidalProgress;
        m_hProcess = processInformation.hProcess;
        PostCreateProcess(sigmoidalProgressOberver);

        // This is a blocking call and gets signalled at regular interval (100 ms).
        // This Waits on the process, while allowing sigmoidal progress to continue.
        MsgWaitForObjectT<SigmoidalProgress>::Wait(processInformation.hProcess, sigmoidalProgress);

        m_hProcess = NULL;
        DWORD exitCode = 0;
        ::GetExitCodeProcess(processInformation.hProcess, &exitCode);
        OnProcessExit(exitCode);

        return exitCode;
    }


private:
    virtual CString GetCommandLine() const = 0;
    virtual void Rollback(IProgressObserver& observer) = 0;
    virtual const CPath GetExecutable() const
    {
        return GetExe().GetName();
    }
    
protected:
    CString GetInstallCommandLine() const
    {
        return m_exe.GetInstallCommandLine();
    }

protected:
    CString GetUnInstallCommandLine() const
    {
        return m_exe.GetUninstallCommandLine();
    }


protected:
    CString GetRepairCommandLine() const
    {
        return m_exe.GetRepairCommandLine();
    }

protected:
    virtual HRESULT PreCreateProcess(CString& cmdLine) 
    {
        return S_FALSE;
    }

    virtual HRESULT PostCreateProcess(IProgressObserver& observer) 
    {
        return S_FALSE;
    }

    virtual HRESULT OnProcessExit(DWORD dwExitCode)
    {
        WatsonData::WatsonDataStatic()->SetInternalErrorState(dwExitCode, GetCurrentItemStep());
        return S_FALSE;
    }

    //This function is added so that we can return the 
    //appropriate internal Error from Ironman Exe.
    virtual HRESULT GetInternalError(int exitCode)
    {
        return exitCode;
    }

    virtual CString GetCurrentItemStep() const
    {
        return L"";
    }

    // Returns true if Rollback flag is authored 'true'
    virtual bool ShouldRollback()
    {
        const RollbackOnPackageInstallFailure* rollbackOnPackageInstallFailure = dynamic_cast<const RollbackOnPackageInstallFailure*>(&m_exe);
        if (rollbackOnPackageInstallFailure != NULL  && rollbackOnPackageInstallFailure->ShouldRollBack())
        {
            return true;
        }
        return false;
    }

private:
    // Formats an HTML HREF link to the executable's log file by extracting name from the path and making it relative if possible
    static CString FormatLogPathString(ILogger & logger, const CString& strExeLogFileSpecification)
    {
        CString strFormattedLinkToExeLogFile;
        CPath logFileName(strExeLogFileSpecification);
        logFileName.StripPath();

        // If possible, use a relative path from the main log file to the exe log file so that the HTML links work if files are moved
        CPath pthSetupLogFilePath(logger.GetFilePath());
        pthSetupLogFilePath.RemoveFileSpec();
        CPath pthRelativeToSetupLog;
        bool fCanUseRelativePath = false;
        if (pthRelativeToSetupLog.RelativePathTo(pthSetupLogFilePath, FILE_ATTRIBUTE_DIRECTORY, strExeLogFileSpecification, FILE_ATTRIBUTE_NORMAL))
        {
            fCanUseRelativePath = true;
        }

        strFormattedLinkToExeLogFile.Format(L"Exe Log File: <a href=\"%s\">%s</a>", fCanUseRelativePath ? CString(pthRelativeToSetupLog): strExeLogFileSpecification , logFileName);
        return strFormattedLinkToExeLogFile;
    }

    // Compares if the last write time of the log is later than install time
    // and adds it to the log file collection.
    // Returns the html formatted stirng for writing to the log.
    static void FilterLogFilesWithInstallStartTime(const CFindFile& findFile, const FILETIME& installStartTime, CSimpleArray<CString>& logFiles)
    {
        if (!(findFile.IsDirectory()||findFile.IsDots()))
        {
            FILETIME currentFileWriteTime;
            if (findFile.GetLastWriteTime(&currentFileWriteTime))
            {
                if (::CompareFileTime(&currentFileWriteTime, &installStartTime) > 0)
                {
                    logFiles.Add(findFile.GetFilePath());
                }
            }
        }
    }

    // Expand the wildcards passed in via 'csLogFileHint' and processes
    // all matching files.
    static void ProcessWildCardInLogFileHint(const CString& logFileHint, const FILETIME& installStartTime, CSimpleArray<CString>& logFiles)
    {
        CFindFile fileFinder;
        if (fileFinder.FindFile(logFileHint))
        {
            FilterLogFilesWithInstallStartTime(fileFinder, installStartTime, logFiles);

            while (fileFinder.FindNextFile())
            {
                FilterLogFilesWithInstallStartTime(fileFinder, installStartTime, logFiles);
            }
        }
    }

    // Creates a list of log files that are newer than install start time which
    // match the LogFileHint attribute.
    void GenerateLogFilesList(const FILETIME& installStartTime, CSimpleArray<CString>& logFiles)
    {
        CString csInputlogFileHint = m_exe.GetLogFileHint();

        CString logFileHint;
        int iTokenPosition = 0;
        int iLogFilesCount = 0;
        CString logFileHintToken = csInputlogFileHint.Tokenize(L"|", iTokenPosition);

        while (iTokenPosition != -1)
        {
            CSystemUtil::ExpandEnvironmentVariables(logFileHintToken, logFileHint);
            //If it is relative path, we need to resolve it by:
            // a. Check the current installer directory
            // b. Check the %temp% directory.
            if (CPath(logFileHint).IsRelative())
            {
                // check if log file exists in the same directory as IronMan log
                // Get directory the IronMan log file is in
                CPath logFileIronManPath(m_logger.GetFilePath() );
                logFileIronManPath.RemoveFileSpec();
                CPath fullPathToLogFile(logFileIronManPath);
                fullPathToLogFile.Append(logFileHint);

                ProcessWildCardInLogFileHint(fullPathToLogFile, installStartTime, logFiles);
            }
            else
            {
                ProcessWildCardInLogFileHint(logFileHint, installStartTime, logFiles);
            }
            logFileHintToken = csInputlogFileHint.Tokenize(L"|", iTokenPosition);
        }
    }

private: // test sub-class test hook
    virtual BOOL CreateProcess(
            __in_opt    LPCTSTR lpApplicationName,
            __inout_opt LPTSTR lpCommandLine,
            __in_opt    LPSECURITY_ATTRIBUTES lpProcessAttributes,
            __in_opt    LPSECURITY_ATTRIBUTES lpThreadAttributes,
            __in        BOOL bInheritHandles,
            __in        DWORD dwCreationFlags,
            __in_opt    LPVOID lpEnvironment,
            __in_opt    LPCTSTR lpCurrentDirectory,
            __in        LPSTARTUPINFOW lpStartupInfo,
            __out       LPPROCESS_INFORMATION lpProcessInformation)
    {
        // Process handles closed in ~CProcessInformation()
#pragma warning (push)
#pragma warning( disable:25028 )
        return ::CreateProcess(
            lpApplicationName,
            lpCommandLine,
            lpProcessAttributes,
            lpThreadAttributes,
            bInheritHandles,
            dwCreationFlags,
            lpEnvironment,
            lpCurrentDirectory,
            lpStartupInfo,
            lpProcessInformation);
#pragma warning (pop)
    }
};

class ExeUnInstaller : public ExeInstallerBase
{
public:
    ExeUnInstaller(const ExeBase& exe, ILogger& logger, Ux& uxLogger) 
        : ExeInstallerBase(exe, IProgressObserver::Uninstalling, logger, uxLogger)
    {
    }

    virtual ~ExeUnInstaller(void)
    {
    }

private:    
    virtual CString GetCommandLine() const
    {
        return GetUnInstallCommandLine();
    }

    virtual void Rollback(IProgressObserver& observer)
    {}

    virtual const CPath GetExecutable() const
    {
        return GetExe().GetName();
    }
};

class ExeInstaller : public ExeInstallerBase
{
public:
    ExeInstaller(const ExeBase& exe, ILogger& logger, Ux& uxLogger) 
        : ExeInstallerBase(exe, IProgressObserver::Installing, logger, uxLogger)
    {}
    virtual ~ExeInstaller(void) {}

private:    
    virtual CString GetCommandLine() const
    {
        return GetInstallCommandLine();
    }


    virtual void Rollback(IProgressObserver& observer)
    {   
        // If rollback flag is authored false, we should not rollback.
        if (ShouldRollback())
        {
            LOG(GetLogger(), ILogger::Verbose, L"PerformOperation was aborted - Rollback the install now");
            ExeUnInstaller uninstaller(GetExe(), GetLogger(), GetUx());
            static_cast<IPerformer&>(uninstaller).PerformAction(observer);
        }
        else
        {
            LOG(GetLogger(), ILogger::Verbose, L"PerformOperation was aborted - not Rolling back as Rollback is false.");
        }
    }

    virtual const CPath GetExecutable() const
    {
        return GetExe().GetName();
    }

};

class ExeRepairer : public ExeInstallerBase
{
public:
    ExeRepairer(const ExeBase& exe,  ILogger& logger, Ux& uxLogger) 
        : ExeInstallerBase(exe, IProgressObserver::Repairing, logger, uxLogger)
    {}
    virtual ~ExeRepairer(void) {}

private:    
    virtual CString GetCommandLine() const
    {
        return GetRepairCommandLine();
    }

    virtual void Rollback(IProgressObserver& observer)
    {}  

    virtual const CPath GetExecutable() const
    {
        return GetExe().GetName();
    }
};

} // namespace IronMan
