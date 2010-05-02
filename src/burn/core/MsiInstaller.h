//-------------------------------------------------------------------------------------------------
// <copyright file="MsiInstaller.h" company="Microsoft">
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
//    MSI performer
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once

#include "interfaces\IPerformer.h"
#include "schema\EngineData.h"
#include "MsiExternalUiHandler.h"
#include "CmdLineParser.h"
#include "Watson\WatsonDefinition.h"
#include "Ux\Ux.h"
#include "common\MsiUtils.h"
#include "Action.h"

namespace IronMan
{
template<typename MSI>
class MsiInstallerBaseT : public AbortPerformerBase, public Action
{
    const MSI& m_msi;
    ILogger& m_logger;
    Ux& m_uxLogger;
    IBurnView *m_pBurnView;
    DWORD m_dwRetryCount;

public:
    MsiInstallerBaseT(const MSI& msi
        , IProgressObserver::State action
        , ILogger& logger
        , Ux& uxLogger
        , IBurnView *pBurnView = NULL)
        : Action(action)
        , m_msi(msi)
        , m_logger(logger)
        , m_pBurnView(pBurnView)
        , m_uxLogger(uxLogger)
        , m_dwRetryCount(0)
    {}
    virtual ~MsiInstallerBaseT()
    {}

    //
    //  Method Name: PerformAction
    //  
    //  Notes:
    //  ------
    //  1. 
    //  2. This class will not check to see if the product is already installed. 
    //          This should be done outside of this class.
    //
    //  3. This class will not worry about the different UI modes. 
    //          The UI handler passed in will handle it. Actually different UI handlers
    //          will be passed in for diffrent UI modes.
    //
    virtual void PerformAction(IProgressObserver& observer)
    {
        CString section = L" complete";
        CString localPathName = m_msi.GetName();
        observer.OnStateChangeDetail(GetAction(), m_msi.GetCanonicalTargetName());
        PUSHLOGSECTIONPOP(m_logger, L"Action", L"Performing Action on MSI at " + localPathName, section);

        // Rename MSI log file to our scheme
        CString strMsiLogFile = SetMsiLoggingParameters();

        MsiExternalUiHandler uiHandler(AbortFlagRef(), observer, Logger(), m_uxLogger, m_pBurnView, m_msi.GetId());
        uiHandler.SetProductCode(GetMsi().GetProductCode());

        const UxEnum::actionEnum uxAction = GetUxAction(GetAction());

        m_uxLogger.StartRecordingItem(m_msi.GetOriginalName()
                                        , UxEnum::pInstall
                                        , uxAction                                   
                                        , UxEnum::tMSI);
        UINT result = Execute();                  
        m_uxLogger.StopRecordingItem(m_msi.GetOriginalName()
                                     , uiHandler.IsMajorUpgrade()? UxEnum::aMajorUpgrade : uxAction
                                     , 0
                                     , HRESULT_FROM_WIN32(result)
                                     , StringUtil::FromDword(uiHandler.GetInternalError()) 
                                     , m_dwRetryCount++); 

        CString strResult;
        // Is this an Installation or Uninstall?
        CString operation = Operation();

        switch(result)
        {
        case ERROR_UNKNOWN_PRODUCT: // not an error if the product isn't installed
            LOG(Logger(), ILogger::Verbose, L": ERROR_UNKNOWN_PRODUCT (not actually an error: this product is not installed.)");
            result = ERROR_SUCCESS;
            break;
        case ERROR_SUCCESS:
            //Logging Result
            strResult.Format(L"MSI (%s) %s succeeded.  Msi Log: <a href=\"%s\">%s</a>", m_msi.GetName(), operation, ModuleUtils::GetFileNameFromFullPath(strMsiLogFile), ModuleUtils::GetFileNameFromFullPath(strMsiLogFile)); 
            LOG(Logger(), ILogger::Result, strResult);   

            LOG(Logger(), ILogger::Verbose, L":  no error");
            break;
        case ERROR_SUCCESS_REBOOT_REQUIRED:
            //Logging Result
            strResult.Format(L"MSI (%s) %s succeeded and requires reboot.  Msi Log: <a href=\"%s\">%s</a>", m_msi.GetName(), operation, ModuleUtils::GetFileNameFromFullPath(strMsiLogFile), ModuleUtils::GetFileNameFromFullPath(strMsiLogFile));  
            LOG(Logger(), ILogger::Result, strResult);   
            LOG(Logger(), ILogger::Verbose, L":  ERROR_SUCCESS_REBOOT_REQUIRED");
            break;
        default:
            //Logging Result
            strResult.Format(L"MSI (%s) %s failed.  Msi Log: <a href=\"%s\">%s</a>", m_msi.GetName(), operation, ModuleUtils::GetFileNameFromFullPath(strMsiLogFile), ModuleUtils::GetFileNameFromFullPath(strMsiLogFile));   
            LOG(Logger(), ILogger::Result, strResult);

            if (ERROR_INSTALL_FAILURE == result)
            {
                WatsonData::WatsonDataStatic()->SetInternalErrorState(uiHandler.GetInternalError(), uiHandler.GetCurrentStepName());
            }
        }

        if (HasAborted())
        {
            LOG(m_logger, ILogger::Verbose, L"PerformOperation was aborted");
            // Returning result instead of E_ABORT as CompositePerformer will handle this with SetErrorWithAbort()
            observer.Finished(HRESULT_FROM_WIN32(result));
        }
        else 
        {
            CString log;
            log.Format(L"PerformOperation returned %u (translates to HRESULT = 0x%x)", result, HRESULT_FROM_WIN32(result));
            if (!MSIUtils::IsSuccess(HRESULT_FROM_WIN32(result))) 
            {
                LOG(m_logger, ILogger::Error, log);
            }               

            observer.Finished(HRESULT_FROM_WIN32(result));
        }
    }

private:
    virtual UINT Execute() = 0;
    virtual CString Operation() = 0;

private:
    CString SetMsiLoggingParameters(void)      
    {
        SYSTEMTIME stTime;
        ::GetLocalTime(&stTime);     
        CString strMsiLogFile;
        CPath pthMainLog = Logger().GetFilePath();
        // Strip the main log file extension leaving the SKU name part
        pthMainLog.RemoveExtension();
        // SKU name, time action invoked on this package, package type and package ID to enable correct grouping and sorting without clashes
        strMsiLogFile.Format(L"%s_%04u%02u%02u%02u%02u%02u_MSI_%s.txt",CString(pthMainLog),stTime.wYear,stTime.wMonth,stTime.wDay,stTime.wHour,stTime.wMinute,stTime.wSecond,m_msi.GetId());

        // Set the MSI log file spec and level
        UINT result = this->MsiEnableLog(
            // The logging levels here are the same as Brooklyn
            INSTALLLOGMODE_FATALEXIT |
            INSTALLLOGMODE_ERROR |
            INSTALLLOGMODE_WARNING |
            INSTALLLOGMODE_USER |
            INSTALLLOGMODE_INFO |
            INSTALLLOGMODE_RESOLVESOURCE |
            INSTALLLOGMODE_OUTOFDISKSPACE |
            INSTALLLOGMODE_ACTIONSTART |
            INSTALLLOGMODE_ACTIONDATA |
            INSTALLLOGMODE_COMMONDATA |
            INSTALLLOGMODE_PROPERTYDUMP |
            INSTALLLOGMODE_VERBOSE,
            strMsiLogFile,
            INSTALLLOGATTRIBUTES_APPEND);

        if (result != ERROR_SUCCESS)
        {
            Logger().Log(ILogger::Error, L"MsiEnableLog failed!!!");
        }
        else
        {
            // Add the Msi log file to the list of files for Watson to send
            LOG(Logger(), ILogger::InternalUseOnly, strMsiLogFile);
        }

        return strMsiLogFile;
    }  

protected:
    ILogger& Logger() { return m_logger; }
    const MSI& GetMsi() const { return m_msi; }

private: // "sub-class and override test hooks"
    virtual UINT MsiEnableLog(DWORD dwLogMode, LPCWSTR szLogFile, DWORD dwLogAttributes)
    {
        IMASSERT(szLogFile);
        return ::MsiEnableLog(dwLogMode, szLogFile, dwLogAttributes);
    }

}; // class MsiInstallerBaseT
typedef MsiInstallerBaseT<MSI> MsiInstallerBase;

class MsiInstaller : public MsiInstallerBase
{
public:
    MsiInstaller(const MSI& msi,  ILogger& logger, Ux& uxLogger, IBurnView *pBurnView = NULL)
        : MsiInstallerBase(msi, IProgressObserver::Installing, logger, uxLogger, pBurnView)
    {}
    virtual ~MsiInstaller()
    {}

private:
    static bool IsProductInstalled(const CString productCode)
    {
        DWORD dwCount = 0;
        UINT err = MsiGetProductInfo(productCode, INSTALLPROPERTY_VERSIONSTRING, NULL, &dwCount);
        if (err == ERROR_SUCCESS || err == ERROR_MORE_DATA)
        {
            dwCount++;
            CString version;
            err = MsiGetProductInfo(productCode, INSTALLPROPERTY_VERSIONSTRING, version.GetBuffer(dwCount), &dwCount);
            version._ReleaseBuffer();
        }
        return (err == ERROR_SUCCESS);
    }

private:
    virtual UINT Execute()
    {
        HRESULT hr = S_OK;
        LPWSTR sczMsiOptions = NULL;
        UINT ret = 0;
        CCmdLineSwitches switches;
        CString csMsiOptions;
        CString log;

        //
        // The order in which MSIOptions are applied is important here because MSI will get the last set property value.
        //

        //
        //  If we have reached this point then we know that we must install this product. 
        //
        //  Only thing we need to do here is to check if the product is already installed if so we need to
        //  add REINSTALL & REINSTALLMODE commandline properties. 
        //
        if (IsProductInstalled(GetMsi().GetProductCode()))
        {
            csMsiOptions += L" REINSTALL=ALL REINSTALLMODE=vomus ";
        }

        // format MSIOptions attribute
        hr = VariableFormatString(&vpEngineState->variables, (LPCWSTR)GetMsi().GetMsiOptions(), &sczMsiOptions, NULL);
        ExitOnFailure(hr, "Failed to format variable string.");

        csMsiOptions += CString(sczMsiOptions) + L" " + switches.GetMsiOptions();
        csMsiOptions.Trim();            

        log = (L"Calling MsiInstallProduct(");
        log += GetMsi().GetName() + L", " + csMsiOptions;
        LOG(Logger(), ILogger::Verbose, log);

        ret = MsiInstallProduct(GetMsi().GetName(), csMsiOptions);

    LExit:
        ReleaseStr(sczMsiOptions);
        return ret;
    }
    virtual CString Operation(void)
    {
        return CString(L"Installation");
    }

private: // test sub-class test hook
    virtual UINT WINAPI MsiInstallProduct(__in LPCWSTR szPackagePath, __in_opt LPCWSTR szCommandLine)
    {
        IMASSERT(szPackagePath);
        IMASSERT(szCommandLine);
        return ::MsiInstallProduct(szPackagePath, szCommandLine);
    }
};

template<typename MSI>
class MsiUnInstallerT : public MsiInstallerBaseT<MSI>
{
public:
    MsiUnInstallerT(const MSI& msi,  ILogger& logger, Ux& uxLogger, IBurnView *pBurnView = NULL)
        : MsiInstallerBaseT<MSI>(msi, IProgressObserver::Uninstalling, logger, uxLogger, pBurnView)
    {}
    virtual ~MsiUnInstallerT()
    {}

private:
    virtual UINT Execute()
    {
        CString commandLine = CString(L"REMOVE=ALL " + GetMsi().GetMsiUninstallOptions() + L" " + CCmdLineSwitches().GetMsiOptions()).Trim();
        commandLine.Replace(L"  ", L" ");

        CString log(L"Calling MsiConfigureProductEx(");
        log += GetMsi().GetProductCode() + L", INSTALLLEVEL_DEFAULT, INSTALLSTATE_ABSENT, " + commandLine;
        LOG(Logger(), ILogger::Verbose, log);

        return MsiConfigureProductEx(GetMsi().GetProductCode(), INSTALLLEVEL_DEFAULT, INSTALLSTATE_ABSENT, commandLine);
    }
    virtual CString Operation()
    {
        return CString(L"Uninstall");
    }

private: // test subclass test-hook
    virtual UINT MsiConfigureProductEx(__in LPCWSTR szProduct, __in int iInstallLevel, __in INSTALLSTATE eInstallState, __in_opt LPCWSTR szCommandLine)
    { return ::MsiConfigureProductEx(szProduct, iInstallLevel, eInstallState, szCommandLine); }
};
typedef MsiUnInstallerT<MSI> MsiUnInstaller;

template<typename MSI, typename CCmdLineSwitches>
class MsiRepairerT : public MsiInstallerBaseT<MSI>
{
public:
    MsiRepairerT(const MSI& msi,  ILogger& logger, Ux& uxLogger, IBurnView *pBurnView = NULL)
        : MsiInstallerBaseT<MSI>(msi, IProgressObserver::Repairing, logger, uxLogger, pBurnView)
    {}
    virtual ~MsiRepairerT() {}
private:
    virtual UINT Execute()
    {
        CString cmdline = L"REINSTALL=ALL REINSTALLMODE=pecsmu";

        // respect MSIRepairOptions attribute
        if (!GetMsi().GetMsiRepairOptions().IsEmpty())
            cmdline += L" " + GetMsi().GetMsiRepairOptions();

        // respect MSIOptions cmd-line switch
        CCmdLineSwitches switches; 
        if (!switches.GetMsiOptions().IsEmpty())
            cmdline += L" " + switches.GetMsiOptions();

        CString log(L"Calling MsiInstallProduct(");
        log += GetMsi().GetProductCode() + L", " + cmdline + L")";
        LOG(Logger(), ILogger::Verbose, log);

        return MsiConfigureProductEx(GetMsi().GetProductCode(), INSTALLLEVEL_DEFAULT, INSTALLSTATE_DEFAULT, cmdline);
    }
    virtual CString Operation() { return L"repair operation"; }

private: // "test-sub-class"
    virtual UINT MsiConfigureProductEx(__in LPCWSTR szProduct, __in int iInstallLevel, __in INSTALLSTATE eInstallState, __in_opt LPCWSTR szCommandLine) { return ::MsiConfigureProductEx(szProduct, iInstallLevel, eInstallState, szCommandLine); }
};
typedef MsiRepairerT<MSI, CCmdLineSwitches> MsiRepairer;

//
// CreateMsiPerformer
//
static HRESULT CreateMsiPerformer(
                           __in const ActionTable::Actions action,
                           __in const MSI& msi,
                           __in ILogger& logger,
                           __in Ux& uxLogger,
                           __in IBurnView* pBurnView,
                           __out IPerformer** ppPerformer
                           )
{
    HRESULT hr = S_OK;
    LOG(logger, ILogger::Verbose, L"Creating new Performer for MSI item");

    switch(action)
    {
    case ActionTable::Install:
        *ppPerformer = new MsiInstaller(msi, logger, uxLogger, pBurnView);
        break;
    case ActionTable::Uninstall:
        *ppPerformer = new MsiUnInstaller(msi, logger, uxLogger, pBurnView);
        break;
    case ActionTable::Repair:
        *ppPerformer = new MsiRepairer(msi, logger, uxLogger, pBurnView);
        break;
    case ActionTable::Noop:
        *ppPerformer = new DoNothingPerformer();
        break;
    default:
        IMASSERT2(0, L"Invalid action type; can't create performer");
        LOG(logger, ILogger::Warning, L"Invalid action type; can't create performer");
        hr = E_INVALIDARG;
    }

    return hr;
};


} // namespace IronMan
