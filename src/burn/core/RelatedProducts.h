//-------------------------------------------------------------------------------------------------
// <copyright file="RelatedProducts.h" company="Microsoft">
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

#include "MspInstaller.h"

namespace IronMan
{
class RelatedProductsInstallerBase : public BaseMspInstaller
{
    ILogger& m_logger;
    DWORD m_dwRetryCount;
    Ux& m_uxLogger;

    const CSimpleArray<CString> m_affectedProductCodes;
    const RelatedProducts & m_relatedProducts;

public: 
    RelatedProductsInstallerBase(const RelatedProducts& relatedProducts
                        , const FailureActionEnum& subFailureAction
                        , ILogger& logger
                        , Ux& uxLogger)
        : BaseMspInstaller( subFailureAction
                            , IProgressObserver::Updating
                            , logger
                            , uxLogger)
        , m_logger(logger)
        , m_uxLogger(uxLogger)
        , m_relatedProducts(relatedProducts)
        , m_affectedProductCodes(relatedProducts.GetRelatedProducts())
        , m_dwRetryCount(0)
    {
    }

    virtual const CString GetRelatedProductsCommandLine() const = 0;

    virtual const IProgressObserver::State GetActionState() const = 0;

    virtual const CString GetRelatedProductAction() const = 0;

    virtual const CString GetInstallStateString() const = 0;

    virtual const INSTALLSTATE GetInstallState() const = 0;

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
        return m_affectedProductCodes.GetSize();
    }
    virtual CString GetPatchName(int iIndex) 
    {
        return L"";
    }
    virtual CString GetProductName(int iIndex)         {
        return L"";
    }

    // Returns Repair/Uninstall UX Action Enum
    virtual const UxEnum::actionEnum GetRelatedProductsUxAction() const
    {
        return UxEnum::aRepair;
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
    bool IsProductInstalled(const CString& productCode)
    {
        INSTALLSTATE productInstalledState =  MsiQueryProductState(productCode);
        return ((productInstalledState != INSTALLSTATE_INVALIDARG) && (productInstalledState != INSTALLSTATE_UNKNOWN));
    }

    ILogger& GetLogger()
    {
        return m_logger;
    }

    void PerformRlatedProductsAction(Phaser& phaser, const CString& csMsiOptions, const  MsiExternalUiHandler& uih, UINT& prevError, bool& needsReboot)
    {
        int nResult = IDOK;
        HRESULT hr = S_OK;

        CString  commandLine = GetRelatedProductsCommandLine();
        for(int i = 0; i < m_affectedProductCodes.GetSize(); ++i)
        {
            CString strProductName = MSIUtils::GetProductNameFromProductGuid(m_affectedProductCodes[i]);
            if (strProductName.IsEmpty())
                strProductName = m_affectedProductCodes[i];

            CString section = L" complete (" + strProductName + L")";
            PUSHLOGSECTIONPOP(GetLogger(), L"Action", GetRelatedProductAction() + L" " + strProductName, section);

            nResult = phaser.SetNextPhase();
            hr = HRESULT_FROM_VIEW(nResult);
            if (FAILED(hr))
            {
                LOG(GetLogger(), ILogger::Error, L"User interface commanded engine to abort");
                Abort();
                break;
            }

            phaser.GetObserver().OnStateChange(GetActionState());
            phaser.GetObserver().OnStateChangeDetail(GetActionState(), strProductName);
            
            UINT err = ERROR_SUCCESS;
            if (!IsProductInstalled(m_affectedProductCodes[i]))
            {
                LOG(GetLogger(), ILogger::Information, GetRelatedProductAction() + L" " + strProductName + L" is not installed.");
            }
            else
            {
                CString strProductLogSuffix;
                strProductLogSuffix.Format(L"-%s-%s", m_affectedProductCodes[i], GetRelatedProductAction());
                CString strMsiLogFile = MSIUtils::SetMsiLoggingParameters(strProductLogSuffix, m_logger);
                section.Format(L" complete.  Log File: %s", strMsiLogFile);


                CString log(L"MsiConfigureProductEx(");
                log += m_affectedProductCodes[i] + L", INSTALLLEVEL_DEFAULT, " + GetInstallStateString() + L", " + commandLine + L")";
                LOGEX(GetLogger(), ILogger::Verbose, L"REMOVING product (%s) - %s", strProductName, log);

                // Start recording Ux point
                m_uxLogger.StartRecordingItem(strProductName
                    , UxEnum::pRelatedProducts
                    , GetRelatedProductsUxAction()
                    , UxEnum::tMSI);

                // Execute Msi operation
                err = MsiConfigureProductEx(m_affectedProductCodes[i], INSTALLLEVEL_DEFAULT, GetInstallState(), commandLine);

                // Stop recording Ux point
                m_uxLogger.StopRecordingItem(strProductName
                    , 0
                    , HRESULT_FROM_WIN32(err)
                    , StringUtil::FromDword(uih.GetInternalError()) 
                    , 0); 

                MSIUtils::ProcessReturnValue(err, L"RelatedProducts", GetLogger(), GetRelatedProductAction(), strProductName, strMsiLogFile, needsReboot);
            }
            if (prevError == 0) // N.B.  hanging onto first error only
                prevError = err;
        } // for        
    } // PerformRlatedProductsAction

    // Entry point to perform relatedProducts 
    UINT PerformMsiOperation(Phaser& phaser, const CString& csMsiOptions, const  MsiExternalUiHandler& uih)
    {
        CString pop(L"no products to be updated in the RelatedProducts item.");
        ENTERLOGEXIT(GetLogger(), pop);

        pop = L"if you see this, it's a bug";

        bool needsReboot = false;
        UINT prevError = 0;
       
        PerformRlatedProductsAction(phaser, csMsiOptions, uih, prevError, needsReboot);

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
    virtual UINT MsiConfigureProductEx(__in LPCWSTR szProduct, __in int iInstallLevel, __in INSTALLSTATE eInstallState, __in_opt LPCWSTR szCommandLine)
    { 
        return ::MsiConfigureProductEx(szProduct, iInstallLevel, eInstallState, szCommandLine); 
    }
    virtual INSTALLSTATE MsiQueryProductState(__in LPCWSTR  szProduct)
    {
        return ::MsiQueryProductState(szProduct);
    }
};

class RelatedProductsRepairer : public RelatedProductsInstallerBase
{
    const RelatedProducts& m_item;
public: 
    RelatedProductsRepairer(const RelatedProducts& relatedProducts
        
        , const FailureActionEnum& subFailureAction
        , ILogger& logger
        , Ux& uxLogger)
        : RelatedProductsInstallerBase(
          relatedProducts
        , subFailureAction
        , logger
        , uxLogger)
        , m_item(relatedProducts)
    {
    }

private:


    virtual const IProgressObserver::State GetActionState() const
    {
        return IProgressObserver::Repairing;
    }

    virtual const CString GetRelatedProductsCommandLine() const
    {
        return L"REINSTALL=ALL REINSTALLMODE=pecsmu " + m_item.GetMsiRepairOptions();
    }

    virtual const CString GetRelatedProductAction() const
    {
        return L"RepairProduct";
    }

    virtual const CString GetInstallStateString() const
    {
        return L"INSTALLSTATE_DEFAULT";
    }

    virtual const INSTALLSTATE GetInstallState() const
    {
        return INSTALLSTATE_DEFAULT;
    }
};

class RelatedProductsUninstaller : public RelatedProductsInstallerBase
{
    const RelatedProducts& m_item;
public:
    RelatedProductsUninstaller(const RelatedProducts& relatedProducts
        
        , const FailureActionEnum& subFailureAction
        , ILogger& logger
        , Ux& uxLogger)
        : RelatedProductsInstallerBase(
        relatedProducts
        , subFailureAction
        , logger
        , uxLogger)
        , m_item(relatedProducts)
    {
    }

private:

    virtual const CString GetRelatedProductsCommandLine() const
    {
        return L"REMOVE=ALL " + m_item.GetMsiUninstallOptions();
    }

    virtual const IProgressObserver::State GetActionState() const
    {
        return IProgressObserver::Uninstalling;
    }

    virtual const  CString GetRelatedProductAction() const
    {
        return L"UninstallProduct";
    }

    virtual const CString GetInstallStateString() const
    {
        return L"INSTALLSTATE_ABSENT";
    }

    virtual const INSTALLSTATE GetInstallState() const
    {
        return INSTALLSTATE_ABSENT;
    }   

    // Returns Repair/Uninstall UX Action Enum
    virtual const UxEnum::actionEnum GetRelatedProductsUxAction() const
    {
        return UxEnum::aUninstall;
    }

};

//
// CreateRelatedProductsPerformer
//
static HRESULT CreateRelatedProductsPerformer(
                           __in const ActionTable::Actions action,
                           __in const RelatedProducts& relatedProducts,
                           __in FailureActionEnum subFailureAction,
                           __in ILogger& logger,
                           __in Ux& uxLogger,
                           __out IPerformer** ppPerformer
                           )
{
    HRESULT hr = S_OK;
    LOG(logger, ILogger::Verbose, L"Creating new Performer for ServiceControl item");

    switch(action)
    {
    case ActionTable::Install: __fallthrough;
    case ActionTable::Noop:
        *ppPerformer = new DoNothingPerformer();
        break;
    case ActionTable::Uninstall:
        *ppPerformer = new RelatedProductsUninstaller(relatedProducts, subFailureAction, logger, uxLogger);
        break;
    case ActionTable::Repair:
        *ppPerformer = new RelatedProductsRepairer(relatedProducts, subFailureAction, logger, uxLogger);
        break;
    default:
        IMASSERT2(0, L"Invalid action type; can't create performer");
        LOG(logger, ILogger::Warning, L"Invalid action type. Can't create performer.");
        hr = E_INVALIDARG;
    }

    return hr;
}

}
