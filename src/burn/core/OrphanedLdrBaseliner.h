//-------------------------------------------------------------------------------------------------
// <copyright file="OrphanedLdrBaseliner.h" company="Microsoft">
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

namespace IronMan
{
#include "common\MSITableUtils.h"

//------------------------------------------------------------------------------
// Class: PatchInfo
// class to hold info about a patch include
// the path of the local cache of the patch
// the Branch and BaseLine values from the MsiPatchMetadata table
// IronMan depends on the LDR and LDR Baseliner MSPs being built correctly
// and the MsiPatchMetadata table containing the correct values for 
// the Branch and Baseline properties
//------------------------------------------------------------------------------
template <typename MsiTableUtils>
class PatchInfoT
{
    CString m_localPatch;
    CString m_branch;
    CString m_baseline;
public:
    // Constructor
    PatchInfoT(const CString& localPatch)
        : m_localPatch(localPatch)
    {
        MsiTableUtils msiTableUtils(MsiTableUtils::CreateMsiTableUtils(m_localPatch, MsiTableUtils::MspFile, NullLogger::GetNullLogger()));
        m_branch = GetBranch(msiTableUtils);
        m_baseline = GetBaseline(msiTableUtils);
    }

    //------------------------------------------------------------------------------
    // GetBranch returns the Branch property from the MsiPatchMetadata table
    // if this is an Dual branch LDR, LDRBaseliner or GDR
    //------------------------------------------------------------------------------
    static CString GetBranch(MsiTableUtils& msiTableUtils)
    {
        CString branch;
        UINT err = msiTableUtils.ExecuteScalar(L"Value"
                                             , L"MsiPatchMetadata"
                                             , L"`Company` = 'Microsoft Corporation' AND `Property` = 'Branch'"
                                             , branch);
        return branch;
    }

    //------------------------------------------------------------------------------
    // GetBranch returns the Baseline property from the MsiPatchMetadata table
    // if this is an Dual branch LDR, LDRBaseliner or GDR
    //------------------------------------------------------------------------------
    static CString GetBaseline(MsiTableUtils& msiTableUtils)
    {
        CString baseline;
        UINT err = msiTableUtils.ExecuteScalar(L"Value"
                                             , L"MsiPatchMetadata"
                                             , L"`Company` = 'Microsoft Corporation' AND `Property` = 'Baseline'"
                                             , baseline);
        return baseline;
    }

    const CString& GetLocalPath()
    {
        return m_localPatch;
    }

    const CString& GetBaseline()
    {
        return m_baseline;
    }

    bool IsLDRBase()
    {
        return IsLDRBase(m_branch);
    }

    static bool IsLDRBase(CString branch)
    {
        return L"LDRBase" == branch;
    }

    bool IsLDR()
    {
        return L"LDR" == m_branch;
    }
};
typedef PatchInfoT<MsiTableUtils> PatchInfo;

//------------------------------------------------------------------------------
// Class: OrphanedLdrBaseliner
// Given a Product Code and any MSPs that will be uninstalled this class
// finds if there is an LDR Baseline that is applied to the product
// but there are no actual LDRs applied that use the LDR Baseline.
// This will be used so that orphaned LDR Baseliners will not keep a
// GDR from applying
//------------------------------------------------------------------------------
template <typename MsiTableUtils>
class OrphanedLdrBaselinerT
{
    // Member variables
    CSimpleArray<PatchInfoT<MsiTableUtils> > m_patchInfoList;
    CString m_productCode;
    ILogger& m_logger;

public:
    // Constructor
    OrphanedLdrBaselinerT(const CString& productCode, ILogger& logger )
        : m_productCode(productCode)
        , m_logger(logger)
    {
    }

    //------------------------------------------------------------------------------
    // FillPatchInfoList
    // Add all patches applied to the product to the m_patchInfoList
    //------------------------------------------------------------------------------
    void FillPatchInfoList( )
    {
        m_patchInfoList.RemoveAll();
        DWORD dwIndex = 0;
        CString patchCode;
        while (ERROR_SUCCESS == MsiEnumPatchesEx(
            m_productCode
            , NULL
            , MSIINSTALLCONTEXT_USERMANAGED | MSIINSTALLCONTEXT_USERUNMANAGED | MSIINSTALLCONTEXT_MACHINE
            , MSIPATCHSTATE_APPLIED | MSIPATCHSTATE_SUPERSEDED | MSIPATCHSTATE_OBSOLETED
            , dwIndex
            , patchCode.GetBuffer(64)
            , NULL
            , NULL
            , NULL
            , NULL))
        {
            patchCode._ReleaseBuffer();
            CString localPatch;
            if ( ERROR_SUCCESS == GetMspLocalPackage(patchCode, localPatch, m_logger) )
            {
                m_patchInfoList.Add(PatchInfoT<MsiTableUtils>(localPatch));
            }
            ++dwIndex;  // to get the next patchCode
        } 
    }

    bool IsLdrInstalled()
    {
        // Fill the patch info list with all the MSPs that are installed for the 
        // current product
        FillPatchInfoList();

        // Get the list of LDRBase patches installed and 
        // create the map that keeps count of the LDRs installed per Baseline
        CSimpleArray<PatchInfoT<MsiTableUtils> > ldrBasePatchList;
        CSimpleMap<CString, int> mapBaseLineToLdrCount;
        CreateLDRBasePatchList(m_patchInfoList, ldrBasePatchList, mapBaseLineToLdrCount);
        if ( ldrBasePatchList.GetSize() == 0 )
        {
            // no LDRBase MSPs installed, so no processing needed
            return false;
        }

        return true;
    }

    //------------------------------------------------------------------------------
    // AddLdrBaselinerIfOrphaned
    // If the state of the product after the patches are uninstalled is that
    // there is a LDR Baseliner that will not have any LDR patches dependent on
    // it, then the LDR Baseliner patch is added to the list of patches to be
    // removed.
    // Also if the LDR Baseliner patch is already in the list of patches to be
    // removed, it will filter it out if there will be any LDR Patches still dependent
    // on it after the uninstall
    //------------------------------------------------------------------------------
    void AddLdrBaselinerIfOrphaned(CString& semicolonDelimitedLocalPatches)
    {
        // Fill the patch info list with all the MSPs that are installed for the 
        // current product
        FillPatchInfoList();

        // Get the list of LDRBase patches installed and 
        // create the map that keeps count of the LDRs installed per Baseline
        CSimpleArray<PatchInfoT<MsiTableUtils> > ldrBasePatchList;
        CSimpleMap<CString, int> mapBaseLineToLdrCount;
        CreateLDRBasePatchList(m_patchInfoList, ldrBasePatchList, mapBaseLineToLdrCount);
        if ( ldrBasePatchList.GetSize() == 0 )
        {
            // no LDRBase MSPs installed, so no processing needed
            return;
        }

        // At least 1 LDRBase installed

        // Create an array containing the patches to uninstall
        // this will filter out any LDRBase patches from the list
        CSimpleArray<CString> uninstallPatchList;
        CreateUninstallPatchList(semicolonDelimitedLocalPatches, m_patchInfoList, uninstallPatchList);

        // Get the list of LDR patches installed, not including the ones that will be uninstalled
        CSimpleArray<PatchInfoT<MsiTableUtils> > ldrPatchList;
        CreateLDRPatchList(m_patchInfoList, ldrPatchList, uninstallPatchList);

        // count the number of LDR installed per Baseline (not counting the ones being uninstalled) 
        // and add it to the mapBaseLineToLdrCount map
        // Scan through the LDR patches installed, not including the ones that will be uninstalled
        for (int ldrPatchIndex = 0; ldrPatchList.GetSize() > ldrPatchIndex; ++ ldrPatchIndex)
        {
                int index = mapBaseLineToLdrCount.FindKey(ldrPatchList[ldrPatchIndex].GetBaseline());
                if (-1 == index)
                {   // first time, shouldn't happen
                    IMASSERT2(0, L"LDR installed, but LDR Baseline is not installed");
                    mapBaseLineToLdrCount.Add(ldrPatchList[ldrPatchIndex].GetBaseline(), 1);
                }
                else
                {
                    int countOfLdrsPerBaseline = mapBaseLineToLdrCount.GetValueAt(index) + 1;
                    mapBaseLineToLdrCount.SetAtIndex(index, ldrPatchList[ldrPatchIndex].GetBaseline(), countOfLdrsPerBaseline);
                }
        }

        // Add any LDR Baselines to Uninstall Patch List, if the count of the LDRs applied to it are zero
        for ( int ldrBasePatchIndex = 0; ldrBasePatchList.GetSize() > ldrBasePatchIndex; ++ldrBasePatchIndex)
        {
            int index = mapBaseLineToLdrCount.FindKey(ldrBasePatchList[ldrBasePatchIndex].GetBaseline());
            int count = mapBaseLineToLdrCount.GetValueAt(index);
            if ( 0 == count )
            {
                // Found LDR Baseliner that does not have any LDRs applied to it
                uninstallPatchList.Add(ldrBasePatchList[ldrBasePatchIndex].GetLocalPath());
            }
        }

        // Create semicolon delimited list of the patches to uninstall
        semicolonDelimitedLocalPatches = StringUtil::FromArray(uninstallPatchList, L";");
        return;
    }

private:
    //------------------------------------------------------------------------------
    // CreateUninstallPatchList
    // Addes the patches that are in a semicolon delimited list to a 
    // CSimpleArray list.  This filters out any LDRBase patches in the list
    //------------------------------------------------------------------------------
    static void CreateUninstallPatchList(const CString& semicolonDelimitedLocalPatches
                                        , CSimpleArray<PatchInfoT<MsiTableUtils> >& patchInfoList
                                        , CSimpleArray<CString>& uninstallPatchList)
    {
        // parse up string, filling out patches to uninstall
        int iStart = 0;
        for(;;)
        {
            CString patch = semicolonDelimitedLocalPatches.Tokenize(L";", iStart);
            if (iStart == -1)
                break;
            // scan through the installed patches to find out if "patch" is an LDRBase
            for (int patchInfoIndex = 0; patchInfoList.GetSize() > patchInfoIndex; ++ patchInfoIndex)
            {
                if ( patchInfoList[patchInfoIndex].GetLocalPath() == patch )
                {
                    // Found Match
                    if ( !patchInfoList[patchInfoIndex].IsLDRBase() )
                    {
                        // Not an LDRBase patch
                        uninstallPatchList.Add(patch);
                    }
                }
            }
        }
    }

    //------------------------------------------------------------------------------
    // CreateLDRBasePatchList
    // Creates an array containing the LDRBase patches currently installed
    // and initialize the map that contains the count of LDRs installed per baseline
    //------------------------------------------------------------------------------
    static void CreateLDRBasePatchList(CSimpleArray<PatchInfoT<MsiTableUtils> >& patchInfoList
                                        , CSimpleArray<PatchInfoT<MsiTableUtils> >& ldrBasePatchList
                                        ,  CSimpleMap<CString, int>& mapBaseLineToLdrCount)
    {
        for (int patchInfoIndex = 0; patchInfoList.GetSize() > patchInfoIndex; ++ patchInfoIndex)
        {
            if ( patchInfoList[patchInfoIndex].IsLDRBase() )
            {
                // found an LDRBase patch
                ldrBasePatchList.Add(patchInfoList[patchInfoIndex]);
                if ( -1 == mapBaseLineToLdrCount.FindKey(patchInfoList[patchInfoIndex].GetBaseline()) )
                {
                    // create a map entry with Baseline as Key and the Count of LDRs applied to the 
                    // baseline as 0
                    mapBaseLineToLdrCount.Add(patchInfoList[patchInfoIndex].GetBaseline(), 0);
                }
            }
        }
    }

    //------------------------------------------------------------------------------
    // CreateLDRPatchList
    // Creates an array containing the LDR patches currently installed
    //------------------------------------------------------------------------------
    static void CreateLDRPatchList(CSimpleArray<PatchInfoT<MsiTableUtils> >& patchInfoList
                                , CSimpleArray<PatchInfoT<MsiTableUtils> >& ldrPatchList
                                , CSimpleArray<CString>& uninstallPatchList)
    {
        for (int patchInfoIndex = 0; patchInfoList.GetSize() > patchInfoIndex; ++ patchInfoIndex)
        {
            if ( patchInfoList[patchInfoIndex].IsLDR() )
            {
                // Found LDR patch, if it is not in the uninstall list, add it to the LDR patch list
                bool bFound = false;
                for ( int uninstallPatchIndex = 0; uninstallPatchList.GetSize() > uninstallPatchIndex; ++ uninstallPatchIndex)
                {
                    if (patchInfoList[patchInfoIndex].GetLocalPath() == uninstallPatchList[uninstallPatchIndex])
                    {
                        // remove it from the U
                        bFound = true;
                    }
                }
                if (!bFound)
                {
                    // LDR patch that will not be uninstalled in this transaction
                    ldrPatchList.Add(patchInfoList[patchInfoIndex]);
                }
            }
        }
    }


private:  //test hooks
        virtual UINT GetMspLocalPackage(const CString& patchCode
                                    , CString& localCachedPackage
                                    , ILogger& logger)
        {
            return MSIUtils::GetMspLocalPackage(patchCode, localCachedPackage, logger);
        }

    virtual UINT WINAPI MsiEnumPatchesEx(
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
typedef OrphanedLdrBaselinerT<MsiTableUtils> OrphanedLdrBaseliner;
}
