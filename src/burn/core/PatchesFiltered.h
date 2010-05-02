//-------------------------------------------------------------------------------------------------
// <copyright file="PatchesFiltered.h" company="Microsoft">
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

#include "Interfaces\IProgressObserver.h"
#include "schema\EngineData.h"

namespace IronMan
{
//---------------------------------------------------------------------------------------------
//PatchesFilteredT
//Purpose: Allows the return of the full path to all the MSPs taking into account
// the if the MSP applies to the product and whether the string will be used for 
// installing or uninstalling
//---------------------------------------------------------------------------------------------	
template <typename MsiInstallContext>
class PatchesFilteredT
{
    //---------------------------------------------------------------------------------------------
    //MspInstallInfo
    //Purpose: Stores full path to MSP and if the MSP is a LDR Baseliner
    //---------------------------------------------------------------------------------------------	
    class MspInstallInfo
    {
        const CString m_fullPath;
        const bool m_bIsLdrBase;

    public:

        MspInstallInfo(const CString& fullPath, bool bIsLdrBase)
            : m_fullPath(fullPath)
            , m_bIsLdrBase(bIsLdrBase)
        {}

        const CString& FullPath() const
        {
            return m_fullPath;
        }

        const bool IsLdrBase() const
        {
            return m_bIsLdrBase;
        }
    };

    CSimpleArray<MspInstallInfo> m_unfilteredMspList;
    ILogger& m_logger;

public:
    // Patches constructor
    PatchesFilteredT(const Patches& patches, ILogger& logger, const bool bAddOnlyAvailablePatches = false)
        : m_logger(logger)
    {
        for (unsigned int mspIndex = 0; mspIndex < patches.GetCount(); ++mspIndex)
        {
            AddToList(patches.GetMsp(mspIndex).GetName(), bAddOnlyAvailablePatches);
        }
    }

    // MSP List constructor
    PatchesFilteredT(const CSimpleArray<MSP>& mspList, ILogger& logger, const bool bAddOnlyAvailablePatches = false)
        : m_logger(logger)
    {
        for (int mspIndex = 0; mspIndex < mspList.GetSize(); ++mspIndex)
        {
            AddToList(mspList[mspIndex].GetName(), bAddOnlyAvailablePatches);
        }
    }

    // MSP constructor
    PatchesFilteredT(const MSP& msp, ILogger& logger)
        : m_logger(logger)
    {
        AddToList(msp.GetName(), false);
    }

    // Semicolon delimited msp file path list constructor
    PatchesFilteredT(const CString& semicolonDelimitedList, ILogger& logger)
        : m_logger(logger)
    {
        AddSemicolonDelimitedListToList(semicolonDelimitedList, logger);
    }

    // Copy constructor
    PatchesFilteredT(const PatchesFilteredT& rhs)
        : m_unfilteredMspList(rhs.m_unfilteredMspList)
        , m_logger(rhs.m_logger)
    {}

    //---------------------------------------------------------------------------------------------
    //SemicolonDelimitedList
    //Purpose: Returns the full path to all the MSPs delimited by Semicolons taking into account
    // the if the MSP applies to the product and whether the string will be used for 
    // installing or uninstalling
    //---------------------------------------------------------------------------------------------	     
    const CString SemicolonDelimitedList(
        const IProgressObserver::State state = IProgressObserver::Installing
        , const CString productCode = CString()) const
    {
        CSimpleArray<CString> mspList = MspList(state, productCode);
        CString list = StringUtil::FromArray(mspList, L";");
        return list;
    }

    //---------------------------------------------------------------------------------------------
    //MspList
    //Purpose: Returns an array containg the full path to the MSPs taking into account
    // the if the MSP applies to the product and whether the string will be used for 
    // installing or uninstalling
    //---------------------------------------------------------------------------------------------	
    const CSimpleArray<CString> MspList(
        const IProgressObserver::State state = IProgressObserver::Installing
        , const CString productCode = CString()) const

    {
        CSimpleArray<CString> mspList;
        for ( int patchCount = 0; patchCount < m_unfilteredMspList.GetSize(); ++patchCount)
        {
            // LDR Baseliner patches are only added if installing
            if ( !m_unfilteredMspList[patchCount].IsLdrBase() 
                || IProgressObserver::Installing == state)
            {
                mspList.Add(m_unfilteredMspList[patchCount].FullPath());
            }
        }
        if ( productCode.IsEmpty() )
        {
            return mspList;
        }
        else
        {
            CSimpleArray<CString> applicableMspList;
            GetApplicableMSPs(applicableMspList, mspList, productCode);
            return applicableMspList;
        }
    }

    //---------------------------------------------------------------------------------------------
    //IsLdrBasePatchInTheList
    //---------------------------------------------------------------------------------------------
    bool IsLdrBasePatchInTheList() const
    {
        bool bResult = false;
        for ( int patchIndex = 0; m_unfilteredMspList.GetSize() > patchIndex; ++patchIndex)
        {
            if ( m_unfilteredMspList[patchIndex].IsLdrBase() )
            {
                bResult = true;
                break;
            }
        }
        return bResult;
    }

    //---------------------------------------------------------------------------------------------
    //GetFirstNonLdrBasePatch
    //Purpose: Returns the full path to the first MSP that is not a LDR Baseliner patch
    // if it can't find one, it returns the first patch
    //---------------------------------------------------------------------------------------------
    CString GetFirstNonLdrBasePatch() const
    {
        CString firstNonLdrBasePatch;
        for ( int patchIndex = 0; m_unfilteredMspList.GetSize() > patchIndex; ++patchIndex)
        {
            firstNonLdrBasePatch = m_unfilteredMspList[patchIndex].FullPath();
            if ( !m_unfilteredMspList[patchIndex].IsLdrBase() )
            {
                break;
            }
        }
        return firstNonLdrBasePatch;
    }

private:
    //---------------------------------------------------------------------------------------------
    //GetApplicableMSPs
    //Purpose: Returns an array containg the full path to the MSPs taking into account
    // the if the MSP applies to the product and whether the string will be used for 
    // installing or uninstalling
    //---------------------------------------------------------------------------------------------	
    void GetApplicableMSPs(CSimpleArray<CString>&applicableMspList
                                , const CSimpleArray<CString>& mspList
                                , const CString& productCode) const
    {
        MSIINSTALLCONTEXT context = MSIINSTALLCONTEXT_NONE;
        DWORD result = MsiInstallContext(productCode).GetContext(context);
        if (result != ERROR_SUCCESS)
        {
            return;
        }
        
        MsiPatchSequenceInfoArray psiArray(mspList);
        result = MsiDeterminePatchSequence(productCode, NULL, context, psiArray.Size(), psiArray);
        applicableMspList = psiArray.ApplicablePatches();
    }

    //---------------------------------------------------------------------------------------------
    //AddSemicolonDelimitedListToList
    //Purpose: Adds the SemicolonDelimitedList of the full path to the MSPs to the list
    //---------------------------------------------------------------------------------------------	
    void AddSemicolonDelimitedListToList(const CString& semicolonDelimitedListToList, ILogger& logger)
    {
        // parse up string, adding each patch to the list
        int iStart = 0;
        for(;;)
        {
            CString patchFile = semicolonDelimitedListToList.Tokenize(L";", iStart);
            if (iStart == -1)
                break;
            AddToList(patchFile);
        }
    }

    //---------------------------------------------------------------------------------------------
    //AddToList
    //Purpose: Adds the patch to the list
    //---------------------------------------------------------------------------------------------	
    void AddToList(const CString& mspFileName, const bool bAddOnlyAvailablePatches = false)
    {
        // Add the MSP to the list only if the file exists or bAddOnlyAvailablePatches is false
        CPath mspFile(mspFileName);
        if ( !bAddOnlyAvailablePatches || mspFile.FileExists())
        {
            // LDR Baseliner patches are filtered out if not installing
            MsiTableUtils msiTableUtils(MsiTableUtils::CreateMsiTableUtils(mspFileName, MsiTableUtils::MspFile, m_logger));
            bool isLdrBase = PatchInfo::IsLDRBase(PatchInfo::GetBranch(msiTableUtils));
            m_unfilteredMspList.Add( MspInstallInfo(mspFileName, isLdrBase));
        }
        else
        {
            LOG(m_logger, ILogger::Warning, L"Skipping the unavailable patch - " + mspFileName);
        }
    }


private: // test hook
    virtual UINT MsiDeterminePatchSequence(__in LPCWSTR szProductCode, __in_opt LPCWSTR szUserSid, __in MSIINSTALLCONTEXT dwContext, __in DWORD cPatchInfo, __inout_ecount(cPatchInfo) PMSIPATCHSEQUENCEINFO pPatchInfo) const
    {
        return ::MsiDeterminePatchSequence(szProductCode, szUserSid, dwContext, cPatchInfo, pPatchInfo);
    }
};
typedef PatchesFilteredT<CMsiInstallContext> PatchesFiltered;
}
