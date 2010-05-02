//-------------------------------------------------------------------------------------------------
// <copyright file="MSPInstaller.h" company="Microsoft">
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

#include "interfaces\IProgressObserver.h"
#include "LogSignatureDecorator.h"
#include "CmdLineParser.h"
#include "BaseInstaller.h"
#include "Schema\EngineData.h"
#include "Watson\WatsonDefinition.h"
#include "MsiExternalUiHandler.h"
#include "Ux\Ux.h"
#include "common\MsiUtils.h"
#include "MsiInstaller.h"
#include "PhasedProgressObserver.h"
#include "common\ResultObserver.h"
#include "IInstallItems.h"
#include "FirstError.h"
#include "Action.h"
#include "PatchesFiltered.h"
#include "PatchTrain.h"
#include "interfaces\IOperationData.h"

//------------------------------------------------------------------------------
// MsiInstaller
//
// This is the class which knows how to apply .msp patches to products
// It also forms the base class for the MspUininstaller
//
//-------------------------------------------------------------------------------

namespace IronMan
{
//------------------------------------------------------------------------------
// Class BaseMspInstaller provides common functionality which is consumed by
// the MspInstaller and the MspUninstaller classes
//------------------------------------------------------------------------------
    template<typename MsiExternalUiHandler, typename PatchesFiltered>
    class BaseMspInstallerT : public BaseInstaller, public Action
    {
        ILogger& m_logger;
        const FailureActionEnum m_subFailureAction;
        IBurnView *m_pBurnView;
        LPCWSTR m_wzPackageId;

    protected:
        Ux& m_uxLogger;

    public:
        class Phaser
        {
            PhasedProgressObserver& m_phasedObserver;
            MsiExternalUiHandler & m_uiHandler;
        public:
            Phaser(PhasedProgressObserver& phasedObserver, MsiExternalUiHandler& uiHandler)
                : m_phasedObserver(phasedObserver)
                , m_uiHandler(uiHandler)
            {}
            int SetNextPhase()
            {
                HRESULT hr = S_OK;
                int nResult = IDOK;

                /* set previous phase to 100% - 
                if previous phase didn't get quite to 100%,
                it won't look complete until the next ietTimeRemainingOverflow message gets all the way through
                (e.g., iTotal must be non-zero, etc.)
                */
                nResult = m_phasedObserver.OnProgress(0xff);
                hr = HRESULT_FROM_VIEW(nResult);
                if (FAILED(hr))
                {
                    return nResult;
                }

                /*
                get ready for next phase
                */
                nResult = m_phasedObserver.SetNextPhase();
                hr = HRESULT_FROM_VIEW(nResult);
                if (FAILED(hr))
                {
                    return nResult;
                }

                /*
                reinitialize ui handler code back to all defaults;
                ietUninitialized, iTicks, iTotal, etc.
                */
                m_uiHandler.Reset();

                return nResult;
            }
            void ExplicitRollback(int current, int total)
            {
                m_phasedObserver.ExplicitRollback(current, total);
                m_uiHandler.Reset(true);
            }
            IProgressObserver& GetObserver(void)
            {
                IProgressObserver& observer = m_phasedObserver;
                return observer;
            }
        };

    public:
        BaseMspInstallerT(const FailureActionEnum& subFailureAction
                        , IProgressObserver::State action
                        , ILogger& logger
                        , Ux& uxLogger
                        , IBurnView *pBurnView = NULL
                        , __in_z LPCWSTR wzPackageId = NULL) 
            : Action(action)
            , m_logger(logger)
            , m_subFailureAction(subFailureAction)
            , m_uxLogger(uxLogger)
            , m_pBurnView(pBurnView)
            , m_wzPackageId(wzPackageId)
            {}
        virtual ~BaseMspInstallerT() {}
    public: // IInstaller
        virtual bool InInstallMode(void) 
        { 
            return false; 
        }

        virtual void PerformAction(IProgressObserver& observer)
        {
            CString pop;
            ENTERLOGEXIT(m_logger, pop);
            UINT err = PopulateMSPInformationToBeUpdated();

            CString log;
            //It will get >= 0 when either call to PopulateMSPInformationToBeUpdated() fails 
            //                          or the there is not product to patch.
            if (GetSize() > 0)
            {
                PhasedProgressObserver phasedProgressObserver(observer, GetSize());

                MsiExternalUiHandler uih(GetRefOfAbortedFlag()
                                        , phasedProgressObserver
                                        , m_logger
                                        , m_uxLogger
                                        , m_pBurnView
                                        , m_wzPackageId);

                Phaser phaser(phasedProgressObserver, uih);
                CCmdLineSwitches switches; 
                err = PerformMsiOperation(phaser, switches.GetMsiOptions(), uih);

                // set internal error, for Watson report when there is an error.
                if (!MSIUtils::IsSuccess(HRESULT_FROM_WIN32(err)))
                {
                    WatsonData::WatsonDataStatic()->SetInternalErrorState(uih.GetInternalError(), uih.GetCurrentStepName());
                }

                log.Format(L"PerformMsiOperation returned 0x%X", err);
                LOG(GetLogger(), err == ERROR_SUCCESS ? ILogger::Verbose : ILogger::Error, log);
            }

            if (HasAborted())
            {
                pop = L"aborted";
                // Returning result instead of E_ABORT as CompositePerformer will handle this with SetErrorWithAbort()
                // See switch statement below.
            }
            else
            {
                pop = log;
            }
            switch(err)
            {
            case 0:
                observer.Finished(S_OK);
                break;
            default:
                observer.Finished(HRESULT_FROM_WIN32(err));
                break;
            }
        }		

        //------------------------------------------------------------------------------
        // GetMsiLocalCachedPackagePath
        //
        // This function is get the path to the local cach of the MSI given the product code.
        //------------------------------------------------------------------------------
        // should have been static; it's not so I can subclass-n-override 

        UINT GetMsiLocalCachedPackagePath( const CString& productCode, CString& targetPackageInstallPath)
        {
            // Get the size of the buffer needed
            DWORD cchTargetPackageInstallPath = 0;
            UINT err = MsiGetProductInfo( productCode
                                        , INSTALLPROPERTY_LOCALPACKAGE
                                        , targetPackageInstallPath.GetBuffer(cchTargetPackageInstallPath)
                                        , &cchTargetPackageInstallPath);
            if ( ERROR_MORE_DATA == err )
            {
                cchTargetPackageInstallPath += 1;  // add one for the null terminator
                err = MsiGetProductInfo( productCode
                                        , INSTALLPROPERTY_LOCALPACKAGE
                                        , targetPackageInstallPath.GetBuffer(cchTargetPackageInstallPath)
                                        , &cchTargetPackageInstallPath);
            }
            targetPackageInstallPath._ReleaseBuffer();
            return err;
        }

    protected:

        ILogger& GetLogger()
        {
            return m_logger;
        }

        // Set logging level and log name, which is based on the main log name with the specified suffix added at the end
        virtual CString SetMsiLoggingParameters(const CString & strSuffix)
        {
            CString strMsiLogFile;
            CPath pthLogFile = GetLogger().GetFilePath();
            CString strFileName = pthLogFile;
            // Verify that the path to the log file is not empty
            // If it is a NullLogger, then we don't want to call MsiEnableLog with a bad path
            // since that would cause the log not to be written.
            if ( !strFileName.IsEmpty() )
            {
                // Strip the file name out of the path
                pthLogFile.RemoveFileSpec();

                // Add the file name back in along with the supplied suffix
                CString strMsiLogFile = CString(pthLogFile) + CString(L"\\") + ModuleUtils::GetFileNameOnlyWithoutExtension(GetLogger().GetFilePath()) + strSuffix + L".txt";

                // Set the MSI log file spec and level
                UINT uiRet = MsiEnableLog(
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
                if (uiRet != ERROR_SUCCESS)
                {
                    CString strError(L"Error calling MsiEnableLog with log file set to ");
                    strError += strMsiLogFile;
                    LOG(GetLogger(), ILogger::Error, strError);
                }
                else
                {
                    CString strMessage(L"Successfully called MsiEnableLog with log file set to ");
                    strMessage += strMsiLogFile;
                    LOG(GetLogger(), ILogger::Verbose, strMessage);

                    // Add the Msi log file to the list of files for Watson to send
                    LOG(GetLogger(), ILogger::InternalUseOnly, strMsiLogFile);
                }
            }
            return strMsiLogFile;
        }

    private: // subclass-n-override test-hook(s)
        virtual UINT MsiEnableLog(__in DWORD dwLogMode, __in_opt LPCTSTR szLogFile, __in DWORD dwLogAttributes)
        {
            return ::MsiEnableLog(dwLogMode, szLogFile, dwLogAttributes);
        }

    public:
        virtual UINT PopulateMSPInformationToBeUpdated() = 0;
        virtual CString ActionString() = 0;
        virtual int GetSize() = 0;
        virtual CString GetPatchName(int iIndex) = 0;
        virtual CString GetProductName(int iIndex) = 0;
        virtual void Rollback(Phaser& phaser, int iFailedAt, int iTotal) = 0;
        virtual UINT Execute(int iIndex
                            , const CString& csMsiOptions
                            , const MsiExternalUiHandler& uih) = 0;

    protected:
        //------------------------------------------------------------------------------
        // Perform the MSI operation, which is uninstall for this class
        //------------------------------------------------------------------------------
        virtual UINT PerformMsiOperation(Phaser& phaser
                                        , const CString& csMsiOptions
                                        , const  MsiExternalUiHandler& uih)
        {
            int nResult = IDOK;
            HRESULT hr = S_OK;
            CString pop(L" ");
            bool fNeedsReboot = false;
            SYSTEMTIME stTime;

            // parse up string, filling out product code vector
            UINT uPrevError = 0;
            for(int iProduct=0; iProduct<GetSize(); ++iProduct)
            {
                CString strPatchList(GetPatchName(iProduct));
                PUSHLOGSECTIONPOP(GetLogger(), L"Action", L"Performing " + ActionString() + L" on MSP: " + strPatchList + L" targetting Product: " + GetProductName(iProduct), pop);

                // Update StateChangeDetail for use in the Progress Dialog
                // Use the file name without extension for use in the progress string
                // Do not use an LDR Baseliner patch name, unless there is no other choice
                PatchesFiltered patchesFiltered(strPatchList, GetLogger());
                CPath mspMSPFileName(patchesFiltered.GetFirstNonLdrBasePatch());

                mspMSPFileName.StripPath();
                mspMSPFileName.RemoveExtension();
                phaser.GetObserver().OnStateChangeDetail(GetAction(), CString(mspMSPFileName));

                // only if product is actually on the box
                nResult = phaser.SetNextPhase();
                hr = HRESULT_FROM_VIEW(nResult);
                if (FAILED(hr))
                {
                    LOG(GetLogger(), ILogger::Error, L"User interface commanded engine to abort");
                    Abort();
                    break;
                }

                // Change MSP log file name for each product
                CString csPatchLogSuffix;
                ::GetLocalTime(&stTime);     
                csPatchLogSuffix.Format(L"_%04u%02u%02u%02u%02u%02u_%s_MSP%d", stTime.wYear,stTime.wMonth,stTime.wDay,stTime.wHour,stTime.wMinute,stTime.wSecond,GetProductName(iProduct), iProduct);
                
                CString csMsiLogFile = SetMsiLoggingParameters(csPatchLogSuffix);

                UINT uError = Execute(iProduct, csMsiOptions, uih);

                CString csLog;
                CString csResult;
                csLog.Format(L"MSI returned 0x%X", uError);

                switch(uError)
                {
                case ERROR_UNKNOWN_PATCH: // not an error, if the patch was never applied to start with
                    LOG(GetLogger(), ILogger::Verbose, csLog + L": ERROR_UNKNOWN_PATCH (not actually an error - patch was never applied to this product)");
                    break;
                case ERROR_UNKNOWN_PRODUCT: // not an error if the product isn't installed
                    LOG(GetLogger(), ILogger::Verbose, csLog + L": ERROR_UNKNOWN_PRODUCT (not actually an error - patch does not apply to this product)");
                    break;
                case ERROR_SUCCESS:
                    //Logging Result - use GetFileNameAndSuffix(strMsiLogFile) to get relatic links in log so that they work if logs are copied
                    csResult.Format(L"Patch (%s) %s succeeded on product (%s).  Msi Log: <a href=\"%s\">%s</a>"
                        ,strPatchList
                        ,ActionString()
                        ,GetProductName(iProduct)
                        ,ModuleUtils::GetFileNameFromFullPath(csMsiLogFile)
                        ,ModuleUtils::GetFileNameFromFullPath(csMsiLogFile)); 
                    LOG(GetLogger(), ILogger::Result, csResult);

                    LOG(GetLogger(), ILogger::Verbose, csLog + L":  no error");
                    break;
                case ERROR_SUCCESS_REBOOT_REQUIRED:
                    //Logging Result
                    csResult.Format(L"Patch (%s) %s succeeded on product (%s) and requires reboot.  Msi Log: <a href=\"%s\">%s</a>"
                        ,strPatchList
                        ,ActionString()
                        ,GetProductName(iProduct)
                        ,ModuleUtils::GetFileNameFromFullPath(csMsiLogFile)
                        ,ModuleUtils::GetFileNameFromFullPath(csMsiLogFile));
                    LOG(GetLogger(), ILogger::Result, csResult);
                    LOG(GetLogger(), ILogger::Verbose, csLog + L":  ERROR_SUCCESS_REBOOT_REQUIRED");
                    fNeedsReboot = true;
                    break;
                default:
                    //Logging Result
                    csResult.Format(L"Patch (%s) %s failed on product (%s).  Msi Log: <a href=\"%s\">%s</a>"
                        ,strPatchList
                        ,ActionString()
                        ,GetProductName(iProduct)
                        ,ModuleUtils::GetFileNameFromFullPath(csMsiLogFile)
                        ,ModuleUtils::GetFileNameFromFullPath(csMsiLogFile));   
                    LOG(GetLogger(), ILogger::Result, csResult);
                    LOG(GetLogger(), ILogger::Error, csLog);

                    if (m_subFailureAction.IsFailureActionStop())
                    {
                        pop = L" failed and stopped (no rollback)";
                        return uError;
                    }
                    else if (m_subFailureAction.IsFailureActionRollback())
                    {
                        phaser.GetObserver().OnStateChange(IProgressObserver::Rollback);
                        Rollback(phaser, iProduct, GetSize());
                        pop = L" Rollback changes";
                        return uError;
                    }
                    else if (m_subFailureAction.IsFailureActionContinue())
                    {
                        //should translate to Continue from Rollback if we are uninstalling.
                        phaser.GetObserver().OnStateChange(InInstallMode() ? IProgressObserver::Installing : IProgressObserver::Uninstalling);
                        pop = L" Continue";
                    }
                    else 
                    {
                        IMASSERT2(false, L"unknown SubFailureAction");
                    }

                    if (uPrevError == 0) // N.B.  hanging onto first error only
                        uPrevError = uError;
                    break;			
                }
                if (HasAborted())
                    break;
            }

            if (uPrevError)
            {
                pop = L"failed";
                return uPrevError;
            }

            if (fNeedsReboot)
            {
                pop = L"succeeded but needs reboot";			
                return ERROR_SUCCESS_REBOOT_REQUIRED;
            }

            pop = L"succeeded";
            return ERROR_SUCCESS;	
        }

    private:
        virtual UINT MsiGetProductInfo(__in LPCTSTR szProduct, __in LPCTSTR szAttribute, __out_ecount_opt(*pcchValueBuf) LPTSTR lpValueBuf, __inout_opt LPDWORD pcchValueBuf) = 0;
    };
typedef BaseMspInstallerT<MsiExternalUiHandler, PatchesFiltered> BaseMspInstaller;

//------------------------------------------------------------------------------
// MspInstallerT class
//
// Used to install an MSP or Patches setup Item
//-------------------------------------------------------------------------------
template <typename PatchesFiltered>
class MspInstallerT : public BaseMspInstallerT<MsiExternalUiHandler, PatchesFiltered>
{
    const PatchesFiltered m_patchesFiltered;
    CSimpleArray<CString> m_products;
    DWORD m_dwRetryCount;
    const UxEnum::patchTrainEnum m_servicingTrain;

public:
    //------------------------------------------------------------------------------
    // PatchesFiltered Constructor
    //-------------------------------------------------------------------------------
    MspInstallerT(  const PatchesFiltered patchesFiltered 
                    
                    , const FailureActionEnum& subFailureAction
                    , UxEnum::patchTrainEnum servicingTrain
                    , ILogger& logger
                    , Ux& uxLogger
                    , IBurnView *pBurnView = NULL
                    , __in_z LPCWSTR wzPackageId = NULL)
        : BaseMspInstallerT(subFailureAction
                            , IProgressObserver::Installing
                            , logger
                            , uxLogger
                            , pBurnView
                            , wzPackageId)
        , m_patchesFiltered(patchesFiltered)
        , m_servicingTrain(servicingTrain)
        , m_dwRetryCount(0)
    {}

    //------------------------------------------------------------------------------
    // IInstallItems-MSP Constructor
    //-------------------------------------------------------------------------------
    MspInstallerT(  const IInstallItems& items
                    , const MSP& msp
                    
                    , FailureActionEnum subFailureAction
                    , UxEnum::patchTrainEnum servicingTrain
                    , ILogger& logger
                    , Ux& uxLogger
                    , IBurnView *pBurnView = NULL)
        : BaseMspInstallerT(subFailureAction
                            , IProgressObserver::Installing
                            , logger
                            , uxLogger
                            , pBurnView
                            , msp.GetId())
        , m_patchesFiltered(GetPatchesFiltered(msp, logger))
        , m_dwRetryCount(0)
        , m_servicingTrain(servicingTrain)
    {}

    //------------------------------------------------------------------------------
    // MSP Constructor
    //-------------------------------------------------------------------------------
    MspInstallerT(  const MSP& msp
                    
                    , const FailureActionEnum& subFailureAction
                    , UxEnum::patchTrainEnum servicingTrain
                    , ILogger& logger
                    , Ux& uxLogger
                    , IBurnView *pBurnView = NULL)
        : BaseMspInstallerT(subFailureAction
                            , IProgressObserver::Installing
                            , logger
                            , uxLogger
                            , pBurnView
                            , msp.GetId())
        , m_patchesFiltered(GetPatchesFiltered(msp, logger))
        , m_dwRetryCount(0)
        , m_servicingTrain(servicingTrain)
    {}

    //------------------------------------------------------------------------------
    // IInstallItems-Patches Constructor
    //-------------------------------------------------------------------------------
    MspInstallerT(  const IInstallItems& items
                    , const Patches& patches
                    
                    , FailureActionEnum subFailureAction
                    , const UxEnum::patchTrainEnum servicingTrain
                    , ILogger& logger
                    , Ux& uxLogger
                    , IBurnView *pBurnView = NULL)
        : BaseMspInstallerT(subFailureAction
                            , IProgressObserver::Installing
                            , logger
                            , uxLogger
                            , pBurnView
                            , patches.GetId())
        , m_patchesFiltered(GetPatchesFiltered(patches, logger))
        , m_dwRetryCount(0)
        , m_servicingTrain(servicingTrain)
    {}

    //------------------------------------------------------------------------------
    // Patches Constructor
    //-------------------------------------------------------------------------------
    MspInstallerT(  const Patches& patches
                    
                    , FailureActionEnum subFailureAction
                    , const UxEnum::patchTrainEnum servicingTrain
                    , ILogger& logger
                    , Ux& uxLogger
                    , IBurnView *pBurnView = NULL)
        : BaseMspInstallerT(subFailureAction
                            , IProgressObserver::Installing
                            , logger
                            , uxLogger
                            , pBurnView
                            , patches.GetId())
        , m_patchesFiltered(GetAvailablePatches(patches, logger))
        , m_dwRetryCount(0)
        , m_servicingTrain(servicingTrain)
    {}

protected:
    //------------------------------------------------------------------------------
    // IsProductInstalled
    //
    // This function is determine if a product has been applied.
    // It is virtual so that we can override it during testing.
    //------------------------------------------------------------------------------
    virtual bool IsProductInstalled(const CString& csProductGuid)
    {
        return MSIUtils::GetProductNameFromProductGuid(csProductGuid) != L"";
    }

private:
    //------------------------------------------------------------------------------
    // GetAvailablePatches()
    // Filteres patches on only available patches(MSP file exist on disk)
    //-------------------------------------------------------------------------------
    static const PatchesFiltered GetAvailablePatches(const Patches& patches, ILogger& logger)
    {
        return PatchesFiltered(patches, logger, true);
    }

    //------------------------------------------------------------------------------
    // GetPatchesFiltered()
    // Places all patches in a PatchesFiltered object
    //-------------------------------------------------------------------------------
    static const PatchesFiltered GetPatchesFiltered(const Patches& patches, ILogger& logger)
    {
        return PatchesFiltered(patches, logger);
    }

    //------------------------------------------------------------------------------
    // GetPatchesFiltered()
    // Places the MSP in a PatchesFiltered object
    //-------------------------------------------------------------------------------
    static const PatchesFiltered GetPatchesFiltered(const MSP& msp, ILogger& logger)
    {
        return PatchesFiltered(msp, logger);
    }

    virtual UINT PopulateMSPInformationToBeUpdated()
    {
        m_products.RemoveAll();

        CString csProductGuids;
        UINT err = GetProductGuids(csProductGuids);
        if (ERROR_SUCCESS == err)
        {
            // parse up string, filling out product code vector
            int iStart = 0;
            for(;;)
            {
                CString csProduct = csProductGuids.Tokenize(L";", iStart);
                if (-1 == iStart)
                {
                    break;
                }

                //Although this is duplicate test, it would good to have it.
                if (true == IsProductInstalled(csProduct))
                {
                    m_products.Add(csProduct);
                }
            }
            if (0 == m_products.GetSize())
            {
                err = ERROR_UNKNOWN_PRODUCT;
                LOG(GetLogger(), ILogger::Warning, L"no products requiring this update were found on the system");
            }
        }
        return err;
    }

    virtual void Rollback(Phaser& phaser, int iFailedAt, int iTotal) 
    {
        CString pop;
        ENTERLOGEXIT(GetLogger(), pop);

        // We are doing a iFailedAt - 1 here because we don't need to call MsiInstallProduct() on the current failed patch,
        // because MSI has already rolled it back (implicitly).
        for(int i = iFailedAt - 1; i >= 0; --i)
        {
            // Get patches
            CString semicolonDelimitedMspList(GetPatchesFiltered().SemicolonDelimitedList(IProgressObserver::Rollback, m_products[i]));
            if ( semicolonDelimitedMspList.IsEmpty())
            {
                LOG(GetLogger(), ILogger::Verbose, L"There are no patches to uninstall during rollback for product" + m_products[i]);
            }
            else
            {
                TemporaryAbortResetter tar(*this);
                phaser.ExplicitRollback(i, iTotal);

                // Update StateChangeDetail for use in the Progress Dialog
                CString mspFile(GetPatchName(i));
                int firstSemiColon = mspFile.Find(L';');
                mspFile = (-1 == firstSemiColon) ? mspFile : mspFile.Left(firstSemiColon - 1);
                CPath mspMSPFileName(mspFile);
                mspMSPFileName.StripPath();
                mspMSPFileName.RemoveExtension();
                phaser.GetObserver().OnStateChangeDetail(GetAction(), CString(mspMSPFileName));

                CString log;
                // Get the full path to the cached msi
                CString targetPackageInstallPath;
                UINT err = GetMsiLocalCachedPackagePath(m_products[i], targetPackageInstallPath);
                if ( ERROR_SUCCESS == err )
                {
                    log.Format(L"about to call MsiInstallProduct with MSIPATCHREMOVE=\"%s\" on product %s(%s) to remove patches."
                                      ,semicolonDelimitedMspList, m_products[i], targetPackageInstallPath);
                    LOG(GetLogger(), ILogger::Verbose, log);
                    CString commandLine;
                    commandLine.Format(L"MSIPATCHREMOVE=\"%s\"", semicolonDelimitedMspList);
                    err = MsiInstallProduct(targetPackageInstallPath, commandLine);
                    log.Format(L"MsiInstallProduct returned 0x%X", err);
                }
                else
                {
                    log.Format(L"GetMsiLocalCachedPackagePath returned 0x%X", err);
                }
                LOG(GetLogger(), ILogger::Information, log);
            }
        }
    }

    //------------------------------------------------------------------------------
    // Returns the full path to the current patch
    //------------------------------------------------------------------------------
    const PatchesFiltered& GetPatchesFiltered(void) const
    {
        return m_patchesFiltered;
    }   

    //------------------------------------------------------------------------------
    // Returns a string containing a semicolon ; separated list of product GUIDs
    // to which the patch applies
    //------------------------------------------------------------------------------
    CString GetProductGuids()
    {
        CString csProductGuids;
        GetProductGuids(csProductGuids);
        return csProductGuids;
    }

private:
    virtual CString ActionString()
    {
        return L"Install";
    }

    virtual int GetSize()
    {
        return m_products.GetSize();
    }

    // ----------------------------------------------------------------------------------------
    // GetPatchName()
    // return a string containing a semicolon delimted list of the MSPs being installed
    // ----------------------------------------------------------------------------------------
    virtual CString GetPatchName(int iIndex)
    {
        return m_patchesFiltered.SemicolonDelimitedList(IProgressObserver::Installing);
    }

    virtual CString GetProductName(int iIndex)
    {
        return MSIUtils::GetProductNameFromProductGuid(m_products[iIndex]);
    }

    virtual UINT Execute(int iIndex
                        , const CString& csMsiOptions
                        , const  MsiExternalUiHandler& uih)
    {
        CString csProductName = MSIUtils::GetProductNameFromProductGuid(m_products[iIndex]);
        CString semicolonDelimitedMspList(m_patchesFiltered.SemicolonDelimitedList(IProgressObserver::Installing, m_products[iIndex]));
        m_uxLogger.AddApplicableProduct(m_products[iIndex], csProductName);

        // Log the train information in user experience data
        PatchTrain<> pt;
        pt.ReportInstallTrainMetrics(semicolonDelimitedMspList
                            , m_products[iIndex]
                            , m_servicingTrain
                            , GetLogger()
                            , m_uxLogger);

        // Get path to local cache of the MSI the patches will be applied to
        CString targetPackageInstallPath;
        UINT err = GetMsiLocalCachedPackagePath(m_products[iIndex], targetPackageInstallPath);
        if ( ERROR_SUCCESS == err )
        {
            CString strMessage;
            strMessage.Format(L"about to call MsiInstallProduct with PATCH=\"%s\" on product %s(%s) to install patches."
                            , semicolonDelimitedMspList, m_products[iIndex], targetPackageInstallPath);
            LOG(GetLogger(), ILogger::Verbose, strMessage);

            //Use "Patches" when there is more than 1 element.
            CString csKey = semicolonDelimitedMspList.Find(';') != -1 ? L"Patches" : semicolonDelimitedMspList; 
            m_uxLogger.StartRecordingItem(csKey
                                        , UxEnum::pInstall
                                        , GetUxAction(GetAction())
                                        , UxEnum::tMsp);

            // Install Patches
            // Using MsiInstallProdut with the Patches= command line instead of MsiApplyMultiplePatches
            // Because MsiApplyMultiplePatches()  performs applicability checks on each MSP and the limited or absent target ProductCodes may not 
            // reflect all potential targets, the executable wrapper must call MsiInstallProduct() passing the PATCH property to the 
            // szCommandLine parameter. Windows Installer will not modify the PATCH property passed to it, while it will remove patches from the
            // list of patches passed to MsiApplyMultiplePatches()  if the product is not listed in the Template summary property of the MSP.
            CString commandLine;
            commandLine.Format(L"%s PATCH=\"%s\"", csMsiOptions, semicolonDelimitedMspList);
            err = MsiInstallProduct(targetPackageInstallPath, commandLine);

            m_uxLogger.StopRecordingItem(csKey
                                        , 0
                                        , HRESULT_FROM_WIN32(err)
                                        , StringUtil::FromDword(uih.GetInternalError())
                                        , m_dwRetryCount); 
        }
        else
        {
            CString log;
            log.Format(L"GetMsiLocalCachedPackagePath returned 0x%X", err);
        }
        return err;
    }

    //------------------------------------------------------------------------------
    // takes a semi-colon-delimited string possibly with duplicate entries, and
    // outputs a semi-colon-delimited string with no duplicate entries.
    //------------------------------------------------------------------------------
    static CString MakeUnique(const CString& nonUnique)
    {
        CSimpleArray<CString> strings;

        int iStart = 0;
        for(;;)
        {
            CString cs = nonUnique.Tokenize(L";", iStart);
            if (iStart == -1)
                break;
            if (strings.Find(cs) == -1)
                strings.Add(cs);
        }

        CString unique = StringUtil::FromArray(strings, L";");
        return unique;
    }

    UINT GetProductGuids(CString& csProductGuids)
    {
        CSimpleArray<CString> mspList(GetPatchesFiltered().MspList());
        CString nonUniqueGuids;

        for(int mspIndex = 0; mspIndex < mspList.GetSize(); ++mspIndex)
        {
            CString csProductGuidsForEachPatch;
            UINT err = GetProductGuids(mspList[mspIndex], csProductGuidsForEachPatch);
            if (err != ERROR_SUCCESS)
                return err;

            if (!nonUniqueGuids.IsEmpty())
                nonUniqueGuids += L";";
            nonUniqueGuids += csProductGuidsForEachPatch;
        }

        csProductGuids = MakeUnique(nonUniqueGuids);
        return ERROR_SUCCESS;
    }

    UINT GetProductGuids(const CString& mspFilePath, CString& csProductGuids)
    {
        CString log;

        // crack open the MSP to get the patch code and the rest of the stuff
        PMSIHANDLE hSummary = NULL;
        UINT err;
        if ((err = MsiGetSummaryInformation(NULL, mspFilePath, 0, &hSummary)) == ERROR_SUCCESS)
        {
            UINT uiDataType = 0;
            INT iValue;
            FILETIME ft;
                                                            
            // parse up template property (this is where the target product guids go)
            WCHAR wszTooSmallBuffer[2] = {0};
            DWORD cchTooSmallBuffer = 2;
            err = MsiSummaryInfoGetProperty(hSummary, PID_TEMPLATE, &uiDataType, &iValue, &ft, wszTooSmallBuffer, &cchTooSmallBuffer);
            // This will fail with ERROR_MORE_DATA
            WCHAR * lpBuffer = new WCHAR[++cchTooSmallBuffer];
            lpBuffer[cchTooSmallBuffer-1] = 0; // silence PREfast
            err = MsiSummaryInfoGetProperty(hSummary, PID_TEMPLATE, &uiDataType, &iValue, &ft, lpBuffer, &cchTooSmallBuffer);
            if (err == ERROR_SUCCESS)
            {
                csProductGuids = lpBuffer;
            }
            else
            {
                log.Format(L"MsiSummaryInfoGetProperty failed with 0x%X", err);
                LOG(GetLogger(), ILogger::Error, log);
            }
            delete[] lpBuffer;
        }
        else
        {
            log.Format(L"MsiGetSummaryInformation failed with 0x%X", err);
            LOG(GetLogger(), ILogger::Error, log);
        }
        return err;
    }

public:
    virtual bool InInstallMode(void)
    {
        return true;
    }

private:  // "subclass-and-override" test hook
    virtual UINT MsiGetSummaryInformation(MSIHANDLE hDatabase, LPCTSTR szDatabasePath, UINT uiUpdateCount, MSIHANDLE *phSummaryInfo)
    {
        return ::MsiGetSummaryInformation(hDatabase, szDatabasePath, uiUpdateCount, phSummaryInfo);
    }

    virtual UINT MsiSummaryInfoGetProperty(MSIHANDLE hSummaryInfo, UINT uiProperty, __out PUINT puiDataType, __out LPINT piValue, __out FILETIME *pftValue, __out_ecount_opt(*pcchValueBuf) LPTSTR szValueBuf, __inout_opt LPDWORD pcchValueBuf)
    {
        return ::MsiSummaryInfoGetProperty(hSummaryInfo, uiProperty, puiDataType, piValue, pftValue, szValueBuf, pcchValueBuf);
    }

    virtual UINT MsiInstallProduct(__in LPCWSTR szPackagePath, __in_opt LPCWSTR szCommandLine)
    {
        return ::MsiInstallProduct(szPackagePath, szCommandLine);
    }

    virtual UINT MsiGetProductInfo(__in LPCTSTR szProduct, __in LPCTSTR szAttribute, __out_ecount_opt(*pcchValueBuf) LPTSTR lpValueBuf, __inout_opt LPDWORD pcchValueBuf)
    {
        return ::MsiGetProductInfo(szProduct, szAttribute, lpValueBuf, pcchValueBuf);
    }

};
typedef MspInstallerT<PatchesFiltered> MspInstaller;

class MsiRepairerAPIStub
{
public:
    static UINT WINAPI MsiEnumPatchesEx(
    __in_opt LPCWSTR szProductCode,                                   // Enumerate patches on instances of this product
    __in_opt LPCWSTR szUserSid,                                       // Account for enumeration
    __in DWORD dwContext,                                              // Contexts for enumeration
    __in DWORD dwFilter,                                               // Filter for enumeration
    __in DWORD dwIndex,                                                // Index for enumeration
    __out_ecount_opt(39) WCHAR szPatchCode[39],         // Enumerated patch code
    __out_ecount_opt(39) WCHAR szTargetProductCode[39], // Enumerated patch's product code
    __out_opt MSIINSTALLCONTEXT *pdwTargetProductContext,              //Enumerated patch's context
    __out_ecount_opt(*pcchTargetUserSid) LPWSTR  szTargetUserSid,     // Enumerated patch's user account
    __inout_opt LPDWORD pcchTargetUserSid)                            // in/out character count of szTargetUserSid
    {
        return ::MsiEnumPatchesEx(szProductCode, szUserSid, dwContext, dwFilter, dwIndex, szPatchCode, szTargetProductCode, pdwTargetProductContext,szTargetUserSid, pcchTargetUserSid);  
    }
};

template<typename MsiRepairPerformer, typename MspInstallPerformer
        , typename MsiXmlBlobBase, typename MsiRepairerAPIStub>
class MspRepairerT : public AbortPerformerBase, public Action
{
    const IInstallItems& m_items;
    const FailureActionEnum m_subFailureAction;
    ILogger& m_logger;
    Ux& m_uxLogger;
    UxEnum::patchTrainEnum m_train;
    IBurnView *m_pBurnView;
    LPCWSTR m_wzPackageId;


    const bool m_bIgnoreDownloadFailure;
    const bool m_bShouldRollback;

    //Declared as class variables to handle abort correctly.
    IPerformer* m_currentPerformer;
    FirstError m_firstError;

    //The list of MSP or MSI to install and repair respectively.
    CSimpleArray<CString> m_MSIToRepairList;
    CSimpleArray<MSP> m_MSPToApplyList;

    //The reason for the nested class is because we have a virtual method OnApplicableProduct().
    //The virtual method may not be fully constructed when the constructor is called, 
    //thus may result in some undeterministic behaviour.
    class ProductAndPatchesList : public MsiXmlBlobBase
    {
        CSimpleArray<CString>& m_MSIToRepairList;
        CSimpleArray<MSP>& m_MSPToApplyList;
        const MSP& m_msp;
        const CString m_currentPatchName;
        const CString m_currentPatchCode;	
        const IInstallItems& m_items;

    public:
        ProductAndPatchesList(const IInstallItems& items
            , const MSP& msp
            , CSimpleArray<CString>& MSIToRepairList
            , CSimpleArray<MSP>& MSPToApplyList)
            : m_MSIToRepairList(MSIToRepairList)
            , m_MSPToApplyList(MSPToApplyList)
            , m_currentPatchName(CString(msp.GetName()))
            , m_currentPatchCode(msp.GetPatchCode())
            , m_items(items)
            , m_msp(msp)
        {}

        void BuildMapping(const CComBSTR& blob) 
        { 
            EnumerateApplicableProducts(StripOffInstallIf(blob)); 
        }

    private:
        bool IsPatchApplied(const CString csProductCode)
        {
            DWORD dwIndex = 0;
            CString csPatchCode;		
            while (ERROR_SUCCESS == MsiRepairerAPIStub::MsiEnumPatchesEx(csProductCode
                , NULL
                , MSIINSTALLCONTEXT_USERMANAGED | MSIINSTALLCONTEXT_USERUNMANAGED | MSIINSTALLCONTEXT_MACHINE
                , MSIPATCHSTATE_APPLIED | MSIPATCHSTATE_SUPERSEDED | MSIPATCHSTATE_OBSOLETED
                , dwIndex
                , csPatchCode.GetBuffer(64)
                , NULL
                , NULL
                , NULL
                , NULL))
            {
                if (csPatchCode == m_currentPatchCode)
                {
                    return true;
                }
                ++dwIndex;
            } 
            return false;
        }

        bool IsMsiAlreadyRepaired(const CString& csProductCode)
        {
            for (unsigned int iIndex=0; iIndex<m_items.GetCount(); ++iIndex)
            {			
                if (m_items.GetItem(iIndex)->GetType() == ItemBase::Msi)
                {
                    const ItemBase* it = m_items.GetItem(iIndex);
                    const MSI* msi = dynamic_cast<const MSI*>(it);
                    if (msi->GetProductCode() == csProductCode)
                    {
                        return m_items.IsItemComplete(iIndex);
                    }
                }
            }
            return false;
        }

    public:
        virtual bool OnApplicableProduct(const CComBSTR& bstrProductCode)
        {
            //determine if the patch has been applied here
            CString csProductCode = CString(bstrProductCode);
            if (IsPatchApplied(csProductCode) )
            {
                //Add to re-install list if 
                // a.  the MSI has not already been repaired in this session
                // b.  The MSI is not already in the to-be-repair-MSI list.
                if (!IsMsiAlreadyRepaired(csProductCode) && m_MSIToRepairList.Find(csProductCode) == -1)
                {
                    m_MSIToRepairList.Add(csProductCode);
                }			
            }
            //Since the MSP has not been applied, add to the to-be-install-MSP list.
            else
            {
                m_MSPToApplyList.Add(m_msp);
            }

            //return true so that it continues to process other products.
            return true;
        }

        static CComBSTR StripOffInstallIf(const CComBSTR& blob)
        {
            CString cs(blob);

            // assumption:  I'm assuming there's only 1 MsiPatch element.
            // There could be more, but that would be very strange as our Decatur extension tool doesn't generate such blobs,
            // and each MSP's applicability is completely defined by a single MsiPatch element

            int index = cs.Find(L"<MsiPatch");
            if (index != -1)
            {
                cs = cs.Mid(index);
                index = cs.Find(L"</MsiPatch>");
                IMASSERT2(index != -1, L"there's an opening MsiPatch element tag, but no closing tag");
                if (index != -1)
                {
                    cs = cs.Left(index + 11); // 11 = length of "<MsiPatch>"
                }
            }
            return CComBSTR(cs);
        }
    };

public:
    //To handle MSP item type
    MspRepairerT(const IInstallItems& items
                , const MSP& msp
                , FailureActionEnum subFailureAction
                , UxEnum::patchTrainEnum train
                , ILogger& logger
                , Ux& uxLogger
                , IBurnView *pBurnView = NULL
                , __in_z LPCWSTR wzPackageId = NULL)
        : Action(IProgressObserver::Repairing)
        , m_subFailureAction(subFailureAction)
        , m_train(train)
        , m_logger(logger)
        , m_uxLogger(uxLogger)
        , m_pBurnView(pBurnView)
        , m_wzPackageId(wzPackageId)
        , m_currentPerformer(&NullPerformer::GetNullPerformer())
        , m_items(items)
        , m_bIgnoreDownloadFailure(msp.ShouldIgnoreDownloadFailure())
        , m_bShouldRollback(msp.ShouldRollBack())
        , m_firstError(logger)
    {
        ProductAndPatchesList listBuilder(items
                                            , msp
                                            , m_MSIToRepairList
                                            , m_MSPToApplyList);
        listBuilder.BuildMapping(msp.GetBlob());
    }

    //To handle Patches item type
    MspRepairerT(const IInstallItems& items
                , const Patches& patches
                
                , FailureActionEnum subFailureAction
                , UxEnum::patchTrainEnum train
                , ILogger& logger
                , Ux& uxLogger
                , IBurnView *pBurnView = NULL
                , __in_z LPCWSTR wzPackageId = NULL)
        : Action(IProgressObserver::Repairing)
        , m_subFailureAction(subFailureAction)
        , m_train(train)
        , m_logger(logger)
        , m_uxLogger(uxLogger)
        , m_pBurnView(pBurnView)
        , m_wzPackageId(wzPackageId)
        , m_currentPerformer(&NullPerformer::GetNullPerformer())
        , m_items(items)
        , m_bIgnoreDownloadFailure(patches.ShouldIgnoreDownloadFailure())
        , m_bShouldRollback(patches.ShouldRollBack())
        , m_firstError(logger)
    {
        for(unsigned int i=0; i<patches.GetCount(); ++i)
        {
            ProductAndPatchesList listBuilder(items
                                                , patches.GetMsp(i)
                                                , m_MSIToRepairList, m_MSPToApplyList);
            listBuilder.BuildMapping(patches.GetMsp(i).GetBlob());
        }
    }

public:
    virtual void PerformAction(IProgressObserver& observer)
    {
        int nResult = IDOK;
        HRESULT hr = S_OK;
        CString section = L" complete";
        PUSHLOGSECTIONPOP(m_logger, L"Action", L"Repairing MSP", section);
        
        int iNumberOfPhases = m_MSIToRepairList.GetSize();
        iNumberOfPhases += (m_MSPToApplyList.GetSize() > 0) ? 1 : 0;
        PhasedProgressObserver phasedObserver(observer, iNumberOfPhases);  

        ResultObserver resultObserver(phasedObserver, hr);

        //Repair the MSP by installing the MSP since it has not yet been installed.
        //This is only 1 iteration because we are calling MSIApplyMultiplePatches.
        if (m_MSPToApplyList.GetSize() > 0)
        {
            nResult = phasedObserver.OnProgress(255); // in case, last time didn't finish up
            hr = HRESULT_FROM_VIEW(nResult);
            if (FAILED(hr))
            {
                LOG(m_logger, ILogger::Error, L"User interface commanded engine to abort");
                Abort();
            }

            nResult = phasedObserver.SetNextPhase();
            hr = HRESULT_FROM_VIEW(nResult);
            if (FAILED(hr))
            {
                LOG(m_logger, ILogger::Error, L"User interface commanded engine to abort");
                Abort();
            }

            PatchesFiltered patchesFiltered(m_MSPToApplyList, m_logger, false);
            CString csLog;
            csLog.Format(L"Install MSPs (%s) since they have not been installed before", patchesFiltered.SemicolonDelimitedList());
            LOG(m_logger, ILogger::Verbose, csLog);
            MspInstallPerformer mspPerformer(patchesFiltered
                                            , m_subFailureAction
                                            , m_train
                                            , m_logger
                                            , m_uxLogger
                                            , m_pBurnView
                                            , m_wzPackageId);
            m_currentPerformer = &mspPerformer;  //So that we can abort it in the abort() function.

            if (!HasAborted())
            {
                m_currentPerformer->PerformAction(resultObserver);
                m_firstError.SetError(hr, m_bIgnoreDownloadFailure);
                m_currentPerformer = &NullPerformer::GetNullPerformer();
            }
        }

        if (m_firstError.IsError())
        {
            //switch(m_subFailureAction)					
            //{
            if (m_subFailureAction.IsFailureActionRollback() || m_subFailureAction.IsFailureActionStop())
            {
            //Nothing to rollback on.
                observer.Finished(m_firstError.GetError());
                LOG(m_logger, ILogger::Verbose, L"SubFailureAction == Rollback | Stop: There is an error so we are stopping");
                return;
            }
            else if (m_subFailureAction.IsFailureActionContinue())
            {
                LOG(m_logger, ILogger::Verbose, L"SubFailureAction == Continue: There is an error but we are continuing");
            }
            else 
            {
                IMASSERT2(false, L"unknown SubFailureAction");
            }
        }

        //Repair the MSP by repairing the MSI since the MSP has been applied.
        for(int iIndex=0; iIndex < m_MSIToRepairList.GetSize() && !HasAborted(); iIndex++)
        {
            nResult = phasedObserver.OnProgress(255); // in case, last time didn't finish up
            hr = HRESULT_FROM_VIEW(nResult);
            if (FAILED(hr))
            {
                LOG(m_logger, ILogger::Error, L"User interface commanded engine to abort");
                Abort();
            }
            nResult = phasedObserver.SetNextPhase();
            hr = HRESULT_FROM_VIEW(nResult);
            if (FAILED(hr))
            {
                LOG(m_logger, ILogger::Error, L"User interface commanded engine to abort");
                Abort();
            }

            if (!HasAborted())
            {
                CString csProductName = MSIUtils::GetProductNameFromProductGuid(m_MSIToRepairList[iIndex]);
                CString csLog;
                csLog.Format(L"Repairing MSI(%s) since MSP has already been applied", csProductName);
                LOG(m_logger, ILogger::Verbose, csLog);			
                MSI msi(csProductName
                        , L"", m_MSIToRepairList[iIndex]
                        , L"DummyVersion" // Product Version is only used for caching during install and uninstall, not needed during repair.
                        , 0
                        , 1
                        , 0
                        , L"Repairing MSP by Reparing MSI"
                        , m_bIgnoreDownloadFailure
                        , m_bShouldRollback
                        , L"");			
                MsiRepairPerformer msiPerformer(msi, m_logger, m_uxLogger);
                m_currentPerformer = &msiPerformer; //So that we can abort it in the abort() function.

                m_currentPerformer->PerformAction(resultObserver);
                m_firstError.SetError(hr, m_bIgnoreDownloadFailure);
                m_currentPerformer = &NullPerformer::GetNullPerformer();
            }

            if (m_firstError.IsError())
            {
                if (m_subFailureAction.IsFailureActionRollback() || m_subFailureAction.IsFailureActionStop())					
                {   
                //Not supporting rollback because the new boundary group will take care of it.
                    observer.Finished(m_firstError.GetError());
                    LOG(m_logger, ILogger::Verbose, L"SubFailureAction == Rollback | Stop: There is an error so we are stopping");
                    return;
                }
                else if (m_subFailureAction.IsFailureActionContinue())
                {
                    LOG(m_logger, ILogger::Verbose, L"SubFailureAction == Continue: There is an error but we are continuing");
                }
                else
                {
                    IMASSERT2(false, L"unknown SubFailureAction");
                }
            }
        }		
        observer.Finished(m_firstError.GetError());
    }

    virtual void Abort()
    {
        AbortPerformerBase::Abort();
        m_currentPerformer->Abort();
        m_firstError.Abort();		
    }
};

typedef MspRepairerT<MsiRepairer, MspInstaller, MsiXmlBlobBase, MsiRepairerAPIStub> MspRepairer;

}
