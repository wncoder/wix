//-------------------------------------------------------------------------------------------------
// <copyright file="CleanupBlockInstaller.h" company="Microsoft">
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
//    This is the class supports following operations: 
//      RemoveProduct
//      RemovePatch
//      UnadvertiseProduct
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once

#include "MspInstaller.h"
#include "OrphanedLdrBaseliner.h"

namespace IronMan
{

class CleanupBlockInstaller : public BaseMspInstaller
{
    ILogger& m_logger;
    DWORD m_dwRetryCount;

    const CSimpleArray<CString> m_productCodesToUnAdvertise;
    const CSimpleArray<CString> m_productCodesToRemove;
    const CleanupBlock& m_cleanupBlock;

    CSimpleMap<CString, CString> m_productToPatchesMapping; // patches are semi-colon delimited
public:
    CleanupBlockInstaller(const CleanupBlock& cleanupBlock
                        , const FailureActionEnum& subFailureAction
                        , ILogger& logger
                        , Ux& uxLogger)
        : BaseMspInstaller( subFailureAction
                            , IProgressObserver::Updating
                            , logger
                            , uxLogger)
        , m_logger(logger)
        , m_cleanupBlock(cleanupBlock)
        , m_productCodesToUnAdvertise(cleanupBlock.GetProductCodesToUnAdvertise())
        , m_productCodesToRemove(cleanupBlock.GetProductCodesToRemove())
        , m_dwRetryCount(0)
    {
        // m_productToPatchesMapping(cleanupBlock.GetProductsToPatchesMapping())
        cleanupBlock.GetProductsToPatchesMapping(m_productToPatchesMapping);
    }

public:


    // BaseMspInstaller virutal methods
    virtual UINT PopulateMSPInformationToBeUpdated()
    {
        return ERROR_SUCCESS;
    }

    virtual CString ActionString()
    {
        return L"Updating";
    }

    virtual int GetSize()
    {
        return m_cleanupBlock.GetAffectedProducts().GetSize();
    }

    virtual CString GetPatchName(int iIndex) 
    {
        return L"";
    }

    virtual CString GetProductName(int iIndex)
    {
        return L"";
    }

    virtual void Rollback(Phaser& phaser, int iFailedAt, int iTotal)
    {
        return;
    }

    virtual UINT Execute(int iIndex, const CString& csMsiOptions, const  MsiExternalUiHandler& uih) 
    {
        return 0;
    }

private:

    // Helper method to findout if product is installed based on MsiQueryProductState
    static bool IsProductInstalled(const CString& productCode)
    {
        INSTALLSTATE productInstalledState =  ::MsiQueryProductState(productCode);
        return ((productInstalledState != INSTALLSTATE_INVALIDARG) && (productInstalledState != INSTALLSTATE_UNKNOWN));
    }

    ILogger& GetLogger()
    {
        return m_logger;
    }

    // Unadvertise Features for the given product.
    virtual UINT UnAdvertiseFeatures(const CString& productCode)
    {
        UINT installResult = ERROR_SUCCESS;
        CString advertisedFeatures = MSIUtils::GetAdvertisedFeatures(productCode);
        if (!advertisedFeatures.IsEmpty())
        {
            CString cmd(L"REINSTALLMODE=omus ADDLOCAL=" + advertisedFeatures);

            WCHAR szLocalPackage[MAX_PATH] = {0};
            DWORD cchLocalPackage = MAX_PATH;
            MsiGetProductInfo(productCode, INSTALLPROPERTY_LOCALPACKAGE, szLocalPackage, &cchLocalPackage);

            // This line may be truncated in the html viewer but will not be in the source.
            LOGEX(GetLogger(), ILogger::Verbose, L"Reinstalling product (%s) to un advertise features (%s) locally", MSIUtils::GetProductNameFromProductGuid(productCode), advertisedFeatures);
            installResult = MsiInstallProduct(szLocalPackage, cmd);
            if (installResult != ERROR_SUCCESS)
            {
                LOGEX(GetLogger(), ILogger::Result, L"UnAdvertiseFeatures failed for %s product.", MSIUtils::GetProductNameFromProductGuid(productCode));
                LOGEX(GetLogger(), ILogger::Error,  L"MsiInstallProduct failed with error 0x%08x", installResult);
            }
        }
        else
        {
            LOGEX(GetLogger(), ILogger::Verbose, L"For %s product no advertised features were found.", MSIUtils::GetProductNameFromProductGuid(productCode));
        }
        return installResult;
    }

    // Entry point to perform CleanupBlock 
    UINT PerformMsiOperation(Phaser& phaser, const CString& csMsiOptions, const  MsiExternalUiHandler& uih)
    {
        int nResult = IDOK;
        HRESULT hr = S_OK;

        CString pop(L"no products to be updated");
        ENTERLOGEXIT(GetLogger(), pop);

        pop = L"if you see this, it's a bug";

        bool needsReboot = false;
        UINT prevError = 0;

        // RemovePatches
        for(int i=0; i<m_productToPatchesMapping.GetSize(); ++i)
        {
            CString strProductCode = m_productToPatchesMapping.GetKeyAt(i);
            CString strLocalPatchList = m_productToPatchesMapping.GetValueAt(i);

            {
                // This is in a block so the resources for OrphanedLdrBaseliner get released right away
                // Add LDR Baseliner Patches that would be orphaned after the uninstall
                // and filter out any LDR Baseliner Patches that would still have LDR patches
                // dependent upon it after the uninstall
                // patches might be updated after the call to AddLdrBaselinerIfOrphaned
                OrphanedLdrBaseliner orphanedLdrBaseliner(strProductCode, GetLogger());
                orphanedLdrBaseliner.AddLdrBaselinerIfOrphaned(strLocalPatchList);

                //Report user experience data
                PatchTrain<> pt;
                pt.ReportUnInstallTrainMetrics( strLocalPatchList
                                    , strProductCode
                                    , GetLogger()
                                    , m_uxLogger);
            }

            CString strProductName = MSIUtils::GetProductNameFromProductGuid(strProductCode);

            CString section = L" complete (" + strProductCode + L")";
            PUSHLOGSECTIONPOP(GetLogger(), L"Action", L"For product " +  strProductName+ L" Uninstalling patches (" + strLocalPatchList  + L")", section);

            nResult = phaser.SetNextPhase();
            hr = HRESULT_FROM_VIEW(nResult);
            if (FAILED(hr))
            {
                if (IDCANCEL == nResult && 0 == prevError)
                {
                    prevError = ERROR_INSTALL_USEREXIT;
                }
                else if (0 == prevError)
                {
                    prevError = ERROR_INSTALL_FAILURE;
                }
                LOG(m_logger, ILogger::Error, L"User interface commanded engine to abort");
                Abort();
                break;
            }

            phaser.GetObserver().OnStateChange(IProgressObserver::Uninstalling);
            phaser.GetObserver().OnStateChangeDetail(IProgressObserver::Updating, strProductName);

            // Change log file name for each product to which patch is applied
            CString strPatchLogSuffix;
            strPatchLogSuffix.Format(L"-%s-RemovePatch%d", strProductCode, i);
            CString strMsiLogFile = MSIUtils::SetMsiLoggingParameters(strPatchLogSuffix, m_logger);
            section.Format(L" complete.  Log File: %s", strMsiLogFile);

            CString log;
            // Get the full path to the cached msi
            CString targetPackageInstallPath;
            UINT err = GetMsiLocalCachedPackagePath(strProductCode, targetPackageInstallPath);
            if ( ERROR_SUCCESS == err )
            {
                CString commandLine;
                commandLine.Format(L"MSIPATCHREMOVE=\"%s\"", strLocalPatchList);

                if ( m_cleanupBlock.DoUnAdvertiseFeaturesOnRemovePatch())
                {
                    CString advertisedFeatures = MSIUtils::GetAdvertisedFeatures( strProductCode);
                    if (!advertisedFeatures.IsEmpty())
                    {
                        commandLine = L"ADDLOCAL=" + advertisedFeatures;
                    }
                }

                log.Format(L"Calling MsiInstallProduct on product %s(%s) to remove patches with commandline %s."
                            , strProductCode, strProductName, commandLine);
                LOG(GetLogger(), ILogger::Verbose, log);


                //Use "Patches" if there is more than one msp.
                CString csKey = CString(strLocalPatchList).Find(';') != -1 ? L"Patches" : strLocalPatchList;
                m_uxLogger.StartRecordingItem(csKey
                                                , UxEnum::pInstall
                                                , UxEnum::aUninstall    //We only remove patches during uninstall.
                                                , UxEnum::tMsp);

                err = MsiInstallProduct(targetPackageInstallPath, commandLine);

                CString csInternalError;
                csInternalError.Format(L"%u", uih.GetInternalError());
                m_uxLogger.StopRecordingItem(csKey
                                             , 0                     // No need to populate size for cleanup block
                                             , HRESULT_FROM_WIN32(err)
                                             , StringUtil::FromDword(uih.GetInternalError())
                                             , m_dwRetryCount++); 

                MSIUtils::ProcessReturnValue(err, L"CleanupBlock", GetLogger(),  L"RemovePatch", strProductName, strMsiLogFile, needsReboot);
                if (prevError == 0) // N.B.  hanging onto first error only
                {
                    prevError = err;
                }

                if ( m_cleanupBlock.DoUnAdvertiseFeaturesOnRemovePatch())
                {
                    // Un advertise any features found advertised after patch removal.
                    // Change log file name for each product to which patch is applied
                    strPatchLogSuffix.Format(L"-%s-UnAdvertiseFeatures%d", strProductCode, i);
                    strMsiLogFile = MSIUtils::SetMsiLoggingParameters(strPatchLogSuffix, m_logger);
                    section.Format(L" complete.  Log File: %s", strMsiLogFile);

                    err = UnAdvertiseFeatures(strProductCode);
                    MSIUtils::ProcessReturnValue(err, L"CleanupBlock", GetLogger(), L"UnAdvertiseFeatures", strProductName, strMsiLogFile, needsReboot);
                    if (prevError == 0) // N.B.  hanging onto first error only
                    {
                        prevError = err;
                    }
                }
                if (HasAborted())
                    break;
            }
            else
            {
                log.Format(L"GetMsiLocalCachedPackagePath returned 0x%X", err);
            }
        }

        // Unadvertise Features
        for(int i = 0; i < m_productCodesToUnAdvertise.GetSize(); ++i)
        {
            CString strProductName = MSIUtils::GetProductNameFromProductGuid(m_productCodesToUnAdvertise[i]);
            if (strProductName.IsEmpty())
            {
                strProductName = m_productCodesToUnAdvertise[i];
            }

            CString section = L" complete (" + strProductName + L")";
            PUSHLOGSECTIONPOP(GetLogger(), L"Action", L"Un advertising features of product " +  strProductName, section);

            nResult = phaser.SetNextPhase();
            hr = HRESULT_FROM_VIEW(nResult);
            if (FAILED(hr))
            {
                if (IDCANCEL == nResult && 0 == prevError)
                {
                    prevError = ERROR_INSTALL_USEREXIT;
                }
                else if (0 == prevError)
                {
                    prevError = ERROR_INSTALL_FAILURE;
                }
                Abort();
                break;
            }

            phaser.GetObserver().OnStateChange(IProgressObserver::Updating);
            phaser.GetObserver().OnStateChangeDetail(IProgressObserver::Updating, strProductName);

            // Un Advertise
            UINT err = ERROR_SUCCESS;
            if (!IsProductInstalled(m_productCodesToUnAdvertise[i]))
            {
                LOG(GetLogger(), ILogger::Information, L"UnAdvertiseFeatures - " + strProductName + L" is not installed. No features to unadvertise.");
            }
            else
            {
                // Change log file name for each product to which patch is applied
                CString strPatchLogSuffix;
                strPatchLogSuffix.Format(L"-%s-UnAdvertise%d", m_productCodesToUnAdvertise[i], i);
                CString strMsiLogFile = MSIUtils::SetMsiLoggingParameters(strPatchLogSuffix, m_logger);
                section.Format(L" complete.  Log File: %s", strMsiLogFile);
                err = UnAdvertiseFeatures(m_productCodesToUnAdvertise[i]);
                MSIUtils::ProcessReturnValue(err, L"CleanupBlock", GetLogger(), L"UnAdvertiseFeatures", strProductName, strMsiLogFile, needsReboot);
            }

            if (prevError == 0) // N.B.  hanging onto first error only
            {
                prevError = err;
            }
        }

        if (!HasAborted())
        {
            // Remove Products
            LPCWSTR szCommandLine = L"REMOVE=ALL";
            for(int i = 0; i < m_productCodesToRemove.GetSize(); ++i)
            {
                CString strProductName = MSIUtils::GetProductNameFromProductGuid(m_productCodesToRemove[i]);
                if (strProductName.IsEmpty())
                {
                    strProductName = m_productCodesToRemove[i];
                }

                CString section = L" complete (" + strProductName + L")";
                PUSHLOGSECTIONPOP(GetLogger(), L"Action", L"RemoveProduct - " + strProductName, section);

                nResult = phaser.SetNextPhase();
                hr = HRESULT_FROM_VIEW(nResult);
                if (FAILED(hr))
                {
                    Abort();
                    break;
                }

                phaser.GetObserver().OnStateChange(IProgressObserver::Uninstalling);
                phaser.GetObserver().OnStateChangeDetail(IProgressObserver::Uninstalling, strProductName);
                
                UINT err = ERROR_SUCCESS;
                if (!IsProductInstalled(m_productCodesToRemove[i]))
                {
                    LOG(GetLogger(), ILogger::Information, L"RemoveProduct - " + strProductName + L" is not installed.");
                }
                else
                {
                    CString strProductLogSuffix;
                    strProductLogSuffix.Format(L"-%s-RemoveProduct", m_productCodesToRemove[i]);
                    CString strMsiLogFile = MSIUtils::SetMsiLoggingParameters(strProductLogSuffix, m_logger);
                    section.Format(L" complete.  Log File: %s", strMsiLogFile);

                    CString log(L"MsiConfigureProductEx(");
                    log += m_productCodesToRemove[i] + L", INSTALLLEVEL_DEFAULT, INSTALLSTATE_ABSENT, " + szCommandLine;
                    LOGEX(GetLogger(), ILogger::Verbose, L"REMOVING product (%s) to add advertised features (%s) locally", strProductName, log);

                    err = MsiConfigureProductEx(m_productCodesToRemove[i], INSTALLLEVEL_DEFAULT, INSTALLSTATE_ABSENT, szCommandLine);
                    MSIUtils::ProcessReturnValue(err, L"CleanupBlock", GetLogger(), L"RemoveProduct", strProductName, strMsiLogFile, needsReboot);
                }

                if (prevError == 0) // N.B.  hanging onto first error only
                {
                    prevError = err;
                }
            }
        }

        if (prevError != 0)
        {
            pop = L"failed";
            return prevError;
        }
        if (needsReboot)
        {
            pop = L"succeeded but needs reboot";
            return ERROR_SUCCESS_REBOOT_REQUIRED;
        }
        pop = L"succeeded";
        return ERROR_SUCCESS;
    }

private: 

        //------------------------------------------------------------------------------
        // GetMsiLocalCachedPackagePath
        //
        // This function is get the path to the local cach of the MSI given the product code.
        //------------------------------------------------------------------------------
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

private: // subclass-n-override test hook
    virtual UINT MsiEnumProductsEx(__in_opt LPCWSTR szProductCode, __in_opt LPCWSTR szUserSid, __in DWORD dwContext, __in DWORD dwIndex, __out_ecount_opt(MAX_GUID_CHARS+1) WCHAR szInstalledProductCode[39], __out_opt MSIINSTALLCONTEXT *pdwInstalledContext, __out_ecount_opt(*pcchSid) LPWSTR szSid, __inout_opt LPDWORD pcchSid)
    { return ::MsiEnumProductsEx(szProductCode, szUserSid, dwContext, dwIndex, szInstalledProductCode, pdwInstalledContext, szSid, pcchSid); }
    virtual UINT MsiGetPatchInfoEx(__in LPCWSTR szPatchCode, __in LPCWSTR szProductCode, __in_opt LPCWSTR szUserSid, __in MSIINSTALLCONTEXT dwContext, __in LPCWSTR szProperty, __out_ecount_opt(*pcchValue) LPWSTR lpValue, __inout_opt LPDWORD pcchValue)
    { return ::MsiGetPatchInfoEx(szPatchCode, szProductCode, szUserSid, dwContext, szProperty, lpValue, pcchValue); }
    virtual UINT MsiRemovePatches(__in LPCWSTR szPatchList, __in LPCWSTR szProductCode, __in INSTALLTYPE eUninstallType, __in_opt LPCWSTR szPropertyList)
    { return ::MsiRemovePatches(szPatchList, szProductCode, eUninstallType, szPropertyList); }
    virtual UINT MsiEnumFeatures(__in LPCWSTR szProduct, __in DWORD iFeatureIndex, __out_ecount(MAX_FEATURE_CHARS+1) LPWSTR lpFeatureBuf, __out_ecount_opt(MAX_FEATURE_CHARS+1) LPWSTR lpParentBuf)
    { return ::MsiEnumFeatures(szProduct, iFeatureIndex, lpFeatureBuf, lpParentBuf); }
    virtual UINT MsiGetProductInfo(__in LPCTSTR szProduct, __in LPCTSTR szAttribute, __out_ecount_opt(*pcchValueBuf) LPTSTR lpValueBuf, __inout_opt LPDWORD pcchValueBuf)
    { return ::MsiGetProductInfo(szProduct, szAttribute, lpValueBuf, pcchValueBuf);
    }
};

//
// CreateCleanupBlockPerformer
//
static HRESULT CreateCleanupBlockPerformer(
                   __in const ActionTable::Actions action,
                   __in const CleanupBlock& cleanupBlock,
                   __in 
                   __in FailureActionEnum subFailureAction,
                   __in ILogger& logger,
                   __in Ux& uxLogger,
                   __out IPerformer** ppPerformer
                   )
{
    LOG(logger, ILogger::Verbose, L"Creating new Performer for MSP item");
    HRESULT hr = S_OK;
    switch(action)
    {
    case ActionTable::Install:
        *ppPerformer = new CleanupBlockInstaller(cleanupBlock, subFailureAction, logger, uxLogger);
        break;

    case ActionTable::Uninstall: __fallthrough;
    case ActionTable::Repair: __fallthrough;
    case ActionTable::Noop:
        *ppPerformer = new DoNothingPerformer();

    default:
        IMASSERT2(0, L"Invalid action type; can't create performer");
        LOG(logger, ILogger::Warning, L"Invalid action type. Can't create performer.");
        hr = E_INVALIDARG;
    }

    return hr;
}

}
