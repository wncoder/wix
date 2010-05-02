//-------------------------------------------------------------------------------------------------
// <copyright file="MSPUninstaller.h" company="Microsoft">
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
#include "OrphanedLdrBaseliner.h"

namespace IronMan
{

template <typename MsiInstallContext, typename MsiXmlBlobBase>
class MspUninstallerT : public BaseMspInstaller
{
private:
    CSimpleMap<CString, CString> m_mapFromProductCodeToPatchGuids;
    DWORD m_dwRetryCount;

protected: // test hook only; should really be private
    class ProductToPatchesMapBuilder : public MsiXmlBlobBase
    {
        const CString m_currentPatch;
        CSimpleMap<CString, CString>& m_mapFromProductCodeToPatchGuids;
    public:
        ProductToPatchesMapBuilder(const CString& currentPatch, CSimpleMap<CString, CString>& mapFromProductCodeToPatchGuids)
            : m_currentPatch(currentPatch)
            , m_mapFromProductCodeToPatchGuids(mapFromProductCodeToPatchGuids)
        {}
        void BuildMap(const CComBSTR& blob) { EnumerateApplicableProducts(StripOffInstallIf(blob)); }

        virtual bool OnApplicableProduct(const CComBSTR& bstrProductCode)
        {
            CString productCode(bstrProductCode);

            int index = m_mapFromProductCodeToPatchGuids.FindKey(productCode);
            if (-1 == index)
            {   // first time
                m_mapFromProductCodeToPatchGuids.Add(productCode, m_currentPatch);
            }
            else
            {
                CString patches = m_mapFromProductCodeToPatchGuids.GetValueAt(index);
                if (-1 == patches.Find(m_currentPatch))
                {
                    patches += L";" + m_currentPatch;

                    // replace with updated value
                    m_mapFromProductCodeToPatchGuids.SetAtIndex(index, productCode, patches);
                }
            }

            return true;
        }
        static CComBSTR StripOffInstallIf(const CComBSTR& blob)
        {
            CString cs(blob);

            // assumption:  I'm assuming there's only 1 MsiPatch element.
            // There could be more, but that would be very strange as our Decatur extension tool doesn't generate such blobs,
            // and each MSP's applicability is completely defined by a single MsiPatch element

            int index = cs.Find(L"<MsiXmlBlob");
            if (index != -1)
            {
                cs = cs.Mid(index);
                index = cs.Find(L"</MsiXmlBlob>");
                IMASSERT2(index != -1, L"there's an opening MsiXmlBlob element tag, but no closing tag");
                if (index != -1)
                {
                    cs = cs.Left(index + lstrlen(L"</MsiXmlBlob>"));
                }
            }
            return CComBSTR(cs);
        }
    };

public:
    //------------------------------------------------------------------------------
    // Constructor
    //------------------------------------------------------------------------------        
    MspUninstallerT(const IInstallItems& items
                    , const MSP& msp
                    , FailureActionEnum subFailureAction
                    , ILogger& logger
                    , Ux& uxLogger
                    , IBurnView *pBurnView = NULL)
        : BaseMspInstaller( subFailureAction.IsFailureActionRollback() ? 
                                        FailureActionEnum::GetContinueAction() : subFailureAction
                            , IProgressObserver::Uninstalling
                            , logger
                            , uxLogger
                            , pBurnView
                            , msp.GetId())
        , m_dwRetryCount(0)
    {
        ProductToPatchesMapBuilder mapBuilder(msp.GetPatchCode(), m_mapFromProductCodeToPatchGuids);
        mapBuilder.BuildMap(msp.GetBlob());
    }
    MspUninstallerT(const IInstallItems& items
                    , const Patches& patches
                    , FailureActionEnum subFailureAction
                    , ILogger& logger
                    , Ux& uxLogger
                    , IBurnView *pBurnView = NULL)
        : BaseMspInstaller( subFailureAction.IsFailureActionRollback() ? 
                                    FailureActionEnum::GetContinueAction() : subFailureAction
                            , IProgressObserver::Uninstalling
                            , logger
                            , uxLogger
                            , pBurnView
                            , patches.GetId())
        , m_dwRetryCount(0)
    {
        for(unsigned int i=0; i<patches.GetCount(); ++i)
        {
            ProductToPatchesMapBuilder mapBuilder(patches.GetMsp(i).GetPatchCode(), m_mapFromProductCodeToPatchGuids);
            mapBuilder.BuildMap(patches.GetMsp(i).GetBlob());
        }
    }

    MspUninstallerT(const MSP& msp
                    , FailureActionEnum subFailureAction
                    , ILogger& logger
                    , Ux& uxLogger
                    , IBurnView *pBurnView = NULL)
        : BaseMspInstaller( subFailureAction.IsFailureActionRollback() 
                                    ? FailureActionEnum::GetContinueAction() : subFailureAction
                            , IProgressObserver::Uninstalling
                            , logger
                            , uxLogger
                            , pBurnView
                            , msp.GetId())
        , m_dwRetryCount(0)
    {
        ProductToPatchesMapBuilder mapBuilder(msp.GetPatchCode(), m_mapFromProductCodeToPatchGuids);
        mapBuilder.BuildMap(msp.GetBlob());
    }
    MspUninstallerT(const Patches& patches
                    , FailureActionEnum subFailureAction
                    , ILogger& logger
                    , Ux& uxLogger
                    , IBurnView *pBurnView = NULL)
        : BaseMspInstaller( subFailureAction.IsFailureActionRollback() 
                                        ? FailureActionEnum::GetContinueAction() : subFailureAction
                                , IProgressObserver::Uninstalling
                                , logger
                                , uxLogger
                                , pBurnView
                                , patches.GetId())
        , m_dwRetryCount(0)
    {
        for(unsigned int i=0; i<patches.GetCount(); ++i)
        {
                ProductToPatchesMapBuilder mapBuilder(patches.GetMsp(i).GetPatchCode(), m_mapFromProductCodeToPatchGuids);
                mapBuilder.BuildMap(patches.GetMsp(i).GetBlob());
        }
    }

private:

    virtual UINT PopulateMSPInformationToBeUpdated()
    {
        //Populated in the constructor
        return 0;
    }

    bool IsPatchUninstallable(LPWSTR patchCode, const CString& productCode)
    {   
        DWORD cchBuff = 10;
        WCHAR buff[10];
        MSIINSTALLCONTEXT context = MSIINSTALLCONTEXT_NONE;
        MsiInstallContext ic(productCode);
        UINT uiRet = ic.GetContext(context);
        if (uiRet != ERROR_SUCCESS)
        {
            CString log;
            log.Format(L"Failed to get install context for product: %s, received error: %d", productCode, uiRet);
            LOG(GetLogger(), ILogger::Verbose, log);
            return false;
        }

        uiRet = this->MsiGetPatchInfoEx(patchCode, productCode, NULL, 
                                    context, INSTALLPROPERTY_PATCHSTATE, buff, &cchBuff);

        IMASSERT2(uiRet != ERROR_MORE_DATA, L"Patch state should need ony 2 chars of memory.");
        IMASSERT2(uiRet != ERROR_UNKNOWN_PROPERTY, L"INSTALLPROPERTY_PATCHSTATE is a known property");

        if (uiRet != ERROR_SUCCESS)
        {
            CString log;
            log.Format(L"MsiGetPatchInfoEx failed for product: %s, received error: %d", productCode, uiRet);
            LOG(GetLogger(), ILogger::Verbose, log);
            return false;
        }

        return (buff[0] == L'1');
    }

    UINT RemovePatches(CString& strPatches
                        , const CString& strProductCode
                        , const CString& strMsiOptions
                        , const MsiExternalUiHandler& uih)
    {
        class CPatches
        {
            const CString& m_strPatches;
            int m_curPos;

        public:
            CPatches(const CString& strPatches) : m_strPatches(strPatches), m_curPos(0)
            {
            }
            bool GetNextPatch(CString &strPatch)
            {
                strPatch = m_strPatches.Tokenize(_T(";"), m_curPos);
                return (strPatch != L"");
            }
        };

        // This string will contain semicolon delimited list of patches(full path to the local file) that will be uninstalled
        // It will contain the LDR baseliner if after the uninstall there would be no LDR patches
        CString strUninstallablePatches;

        CPatches cPatches(strPatches);
        CString strGuid;
        CString strPatch;

        while (cPatches.GetNextPatch(strPatch))
        {
            if (IsPatchUninstallable(strPatch.GetBuffer(), strProductCode))
            {
                CString strLocalCachedPackage;
                if ( ERROR_SUCCESS == GetMspLocalPackage(strPatch, strLocalCachedPackage, GetLogger()))
                {
                    if (0 == strUninstallablePatches.GetLength())
                    {
                        strUninstallablePatches = strLocalCachedPackage;
                        strGuid = strPatch;
                    }
                    else
                    {
                        strUninstallablePatches += L";" + strLocalCachedPackage;
                        strGuid += L";" + strPatch;
                    }
                }
            }
        }

        {
            // This is in a block so the resources for OrphanedLdrBaseliner get released right away
            // Add LDR Baseliner Patches that would be orphaned after the uninstall
            // and filter out any LDR Baseliner Patches that would still have LDR patches
            // dependent upon it after the uninstall
            // patches might be updated after the call to AddLdrBaselinerIfOrphaned
            OrphanedLdrBaseliner orphanedLdrBaseliner(strProductCode, GetLogger());
            orphanedLdrBaseliner.AddLdrBaselinerIfOrphaned(strUninstallablePatches);

            //Report user experience data
            PatchTrain<> pt;
            pt.ReportUnInstallTrainMetrics( strUninstallablePatches
                                , strProductCode
                                , GetLogger()
                                , m_uxLogger);
        }

        if (strUninstallablePatches.GetLength() == 0)
        {
            LOGEX(GetLogger(), ILogger::Verbose, L"Did not find any uninstallable patch for product: %s.", strProductCode);
            return ERROR_SUCCESS;
        }

        // Get the full path to the cached msi
        CString strTargetPackageInstallPath;
        UINT uiRet = GetMsiLocalCachedPackagePath(strProductCode, strTargetPackageInstallPath);
        if ( ERROR_SUCCESS == uiRet )
        {
            LOGEX(GetLogger(), ILogger::Verbose, L"Calling MsiInstallProduct with MSIPATCHREMOVE=\"%s\" on product %s(%s) to remove patches."
                        ,strUninstallablePatches, strProductCode, strTargetPackageInstallPath);

            CString strCommandLine;
            strCommandLine.Format(L"MSIPATCHREMOVE=\"%s\" %s", strUninstallablePatches, strMsiOptions);
            strCommandLine.Trim();

            //Use "Patches" if there is more than one msp.
            CString strKey = CString(strGuid).Find(';') != -1 ? L"Patches" : strGuid;
            m_uxLogger.StartRecordingItem(strKey
                                            , UxEnum::pInstall
                                            , UxEnum::aUninstall    //We only remove patches during uninstall.
                                            , UxEnum::tMsp);

            // Call Windows Installer to uninstall the patches
            uiRet = MsiInstallProduct(strTargetPackageInstallPath, strCommandLine);

            CString strInternalError;
            strInternalError.Format(L"%u", uih.GetInternalError());
            m_uxLogger.StopRecordingItem(strKey
                                         , 0
                                         , HRESULT_FROM_WIN32(uiRet)
                                         , StringUtil::FromDword(uih.GetInternalError())
                                         , m_dwRetryCount++); 

            if (uiRet == ERROR_SUCCESS)
            {
                LOG(GetLogger(), ILogger::Verbose, L"MsiInstallProduct successfully removed the patches");
            }
            else
            {
                LOGEX(GetLogger(), ILogger::Error, L"MsiInstallProduct failed to remove the patches, received error = %d", uiRet);
            }
        }
        else
        {
            LOGEX(GetLogger(), ILogger::Error, L"GetMsiLocalCachedPackagePath returned 0x%X", uiRet);
        }

        return uiRet;
    }

    virtual CString ActionString()
    {
        return L"Uninstall";
    }

    virtual int GetSize()
    {
        return m_mapFromProductCodeToPatchGuids.GetSize();
    }

    virtual CString GetPatchName(int iIndex)
    {		
        CString csPatchName = L"";
        DWORD cchPatchName = 2;

        MSIINSTALLCONTEXT context = MSIINSTALLCONTEXT_NONE;
        MsiInstallContext ic(m_mapFromProductCodeToPatchGuids.GetKeyAt(iIndex));
        UINT uiRet = ic.GetContext(context);
        if (uiRet != ERROR_SUCCESS)
        {
            CString log;
            log.Format(L"Failed to get install context for product: %s, received error: %d", GetProductName(iIndex), uiRet);
            LOG(GetLogger(), ILogger::Warning, log);
            return csPatchName;
        }

        uiRet = this->MsiGetPatchInfoEx(m_mapFromProductCodeToPatchGuids.GetValueAt(iIndex)
                                        , m_mapFromProductCodeToPatchGuids.GetKeyAt(iIndex)
                                        , NULL
                                        , context
                                        , INSTALLPROPERTY_DISPLAYNAME
                                        , csPatchName.GetBuffer(cchPatchName)
                                        , &cchPatchName);
        csPatchName._ReleaseBuffer();
        IMASSERT2(uiRet != ERROR_UNKNOWN_PROPERTY, L"INSTALLPROPERTY_DISPLAYNAME is a known property");

        if (ERROR_MORE_DATA == uiRet)
        {
            uiRet = this->MsiGetPatchInfoEx(m_mapFromProductCodeToPatchGuids.GetValueAt(iIndex)
                                            , m_mapFromProductCodeToPatchGuids.GetKeyAt(iIndex)
                                            , NULL
                                            , context
                                            , INSTALLPROPERTY_DISPLAYNAME
                                            , csPatchName.GetBuffer(cchPatchName)
                                            , &cchPatchName);

        }
        csPatchName._ReleaseBuffer();
        return csPatchName;
    }

    virtual CString GetProductName(int iIndex)
    {
        CString csProductGuid = m_mapFromProductCodeToPatchGuids.GetKeyAt(iIndex);
        return MSIUtils::GetProductNameFromProductGuid(csProductGuid);
    }

    //In Uninstall, rollback is not NA
    virtual void Rollback(Phaser& phaser, int iFailedAt, int iTotal)
    {
        return;
    }
    
    virtual UINT Execute(int iIndex
                        , const CString& csMsiOptions
                        , const  MsiExternalUiHandler& uih)
    {
        CString patchGuids = m_mapFromProductCodeToPatchGuids.GetValueAt(iIndex);
        CString csProductGuid = m_mapFromProductCodeToPatchGuids.GetKeyAt(iIndex);
        CString csProductName = MSIUtils::GetProductNameFromProductGuid(csProductGuid);   
        m_uxLogger.AddApplicableProduct(csProductGuid, csProductName);  

        UINT err = this->RemovePatches( patchGuids,
                                        csProductGuid,
                                        csMsiOptions,               // MSI specific command line options passed via /msioptions switch
                                        uih);

        return err;
    }

private: // test-subclass test hooks
    virtual UINT GetMspLocalPackage(const CString& patchCode
                                , CString& localCachedPackage
                                , ILogger& logger)
    {
        return MSIUtils::GetMspLocalPackage(patchCode, localCachedPackage, logger);
    }

    virtual UINT MsiInstallProduct(__in LPCWSTR szPackagePath, __in_opt LPCWSTR szCommandLine)
    {
        return ::MsiInstallProduct(szPackagePath, szCommandLine);
    }

    virtual UINT MsiGetProductInfo(__in LPCTSTR szProduct, __in LPCTSTR szAttribute, __out_ecount_opt(*pcchValueBuf) LPTSTR lpValueBuf, __inout_opt LPDWORD pcchValueBuf)
    {
        return ::MsiGetProductInfo(szProduct, szAttribute, lpValueBuf, pcchValueBuf);
    }

    virtual 
    UINT MsiGetPatchInfoExW(
                __in LPCWSTR szPatchCode,                    // target patch to query
                __in LPCWSTR szProductCode,                  // target product of patch application
                __in_opt LPCWSTR szUserSid,                  // Account of this product instance
                __in MSIINSTALLCONTEXT dwContext,            // context to query for product and patch
                __in LPCWSTR szProperty,                     // property of patch to retrieve
                __out_ecount_opt(*pcchValue) LPWSTR lpValue, // address buffer for data
                __inout_opt LPDWORD pcchValue)               // in/out value buffer character count
    { 
        return ::MsiGetPatchInfoEx(szPatchCode, szProductCode, szUserSid, dwContext, szProperty, lpValue, pcchValue); 
    }

protected: // test hook
    void SetMap(const CSimpleMap<CString, CString>& mapFromProductCodeToPatchGuids)
    {
        m_mapFromProductCodeToPatchGuids.RemoveAll();
        for(int i=0; i<mapFromProductCodeToPatchGuids.GetSize(); ++i)
        {
            m_mapFromProductCodeToPatchGuids.Add(mapFromProductCodeToPatchGuids.GetKeyAt(i),
                                                 mapFromProductCodeToPatchGuids.GetValueAt(i));
        }
    }
};

typedef MspUninstallerT<CMsiInstallContext, MsiXmlBlobBase> MspUninstaller;

//
// CreateMspPerformer
//
static HRESULT CreateMspPerformer(
                   __in const ActionTable::Actions action,
                   __in const IInstallItems& items,
                   __in const MSP& msp,
                   __in FailureActionEnum subFailureAction,
                   __in const IOperationData& operationData,
                   __in ILogger& logger,
                   __in Ux& uxLogger,
                   __out IPerformer** ppPerformer,
                   __in IBurnView* pBurnView = NULL
                   )
{
    LOG(logger, ILogger::Verbose, L"Creating new Performer for MSP item");
    HRESULT hr = S_OK;
    switch(action)
    {
    case ActionTable::Install:
        *ppPerformer = new MspInstaller(
            items,
            msp,
            subFailureAction,
            operationData.GetPackageData().GetServicingTrain(),
            logger,
            uxLogger,
            pBurnView);
        break;

    case ActionTable::Uninstall:
        *ppPerformer = new MspUninstaller(
            items,
            msp,
            subFailureAction,
            logger,
            uxLogger,
            pBurnView);
        break;

    case ActionTable::Repair:
        *ppPerformer = new MspRepairer(
            items,
            msp,
            subFailureAction,
            operationData.GetPackageData().GetServicingTrain(),
            logger,
            uxLogger,
            pBurnView);
        break;

    case ActionTable::Noop:
        *ppPerformer = new DoNothingPerformer();

    default:
        IMASSERT2(0, L"Invalid action type; can't create performer");
        LOG(logger, ILogger::Warning, L"Invalid action type. Can't create performer.");
        hr = E_INVALIDARG;
    }

    return hr;
}

//
// CreatePatchesPerformer
//
static HRESULT CreatePatchesPerformer(
                   __in const ActionTable::Actions action,
                   __in const IInstallItems& items,
                   __in const Patches& patches,
                   __in FailureActionEnum subFailureAction,
                   __in const IOperationData& operationData,
                   __in ILogger& logger,
                   __in Ux& uxLogger,
                   __out IPerformer** ppPerformer,
                   __in IBurnView* pBurnView = NULL
                   )
{
    LOG(logger, ILogger::Verbose, L"Creating new Performer for MSP item");
    HRESULT hr = S_OK;

    switch(action)
    {
    case ActionTable::Install:
        *ppPerformer = new MspInstaller(
            items,
            patches,
            subFailureAction,
            operationData.GetPackageData().GetServicingTrain(),
            logger,
            uxLogger,
            pBurnView);
        break;

    case ActionTable::Uninstall:
        *ppPerformer = new MspUninstaller(
            items,
            patches,
            subFailureAction,
            logger,
            uxLogger,
            pBurnView);
        break;

    case ActionTable::Repair:
        *ppPerformer = new MspRepairer(
            items,
            patches,
            subFailureAction,
            operationData.GetPackageData().GetServicingTrain(),
            logger,
            uxLogger,
            pBurnView);
        break;

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
