//-------------------------------------------------------------------------------------------------
// <copyright file="PatchTrain.h" company="Microsoft">
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

//------------------------------------------------------------------------------
// class PackageData
//
// This object compute and record the servicing train - GDR | LDR | None, for both
// install and uninstall.
//
//------------------------------------------------------------------------------
namespace IronMan
{
template <typename PATCHESFILTERED = PatchesFiltered, typename ORPHANEDLDRBASELINER = OrphanedLdrBaseliner>
class PatchTrain
{
public:
    PatchTrain()
    {}

    virtual ~PatchTrain()
    {}

    // Populate the patch stream during an install operation
    // Here are the steps
    // (1) If the package level GDR/LDR attribute is
    //     (a) not set, Train = None  (Non-dual branch Scenario)
    //     (b) set to LDR, Train = LDR (
    //     (c) set to GDR, then evaluate the follow
    //         (i) If the base patch exists in the patch list, then Train = LDR
    //         (ii) If the base patch does not exist in the patch list then 
    //              query the installed patches.
    //              (1) If the base patch has previously been installed, then Train = LDR
    //              (2) Otherwise Train = GDR
    //
    // Although we can simply check the installed patches after the install to 
    // determine the train we are on, it does not handle the scenario where the
    // the patch/patches fails to a apply.
    void ReportInstallTrainMetrics(const CString& strPatchList
                        , const CString& strProductCode
                        , const UxEnum::patchTrainEnum authoredTrain
                        , ILogger& logger
                        , Ux& uxLogger)
    {
        UxEnum::patchTrainEnum actualTrain = UxEnum::ptNone;

        if (!(UxEnum::ptNone == authoredTrain))
        {
            actualTrain = UxEnum::ptLDR;
            if (UxEnum::ptLDR != authoredTrain)
            {
                PATCHESFILTERED patchesFiltered(strPatchList, logger);
                bool IsBaselinerInThePatchList = patchesFiltered.IsLdrBasePatchInTheList();
                if (!IsBaselinerInThePatchList)
                {
                    ORPHANEDLDRBASELINER orphanedLdrBaseliner(strProductCode, logger);
                    bool IsBaselinerInstalled = orphanedLdrBaseliner.IsLdrInstalled();
                    if (!IsBaselinerInstalled)
                    {
                        actualTrain = UxEnum::ptGDR;
                    }
                }
            }
        }

        uxLogger.RecordPatchStream( strPatchList
                                    , strProductCode
                                    , actualTrain);
    }

    //Record 2 train datapoints during an uninstall operation
    // a. The package Train
    // b. The Install Train - e.g a GDR install on a machine with LDRbase becomes a LDR.
    //
    // Note:  Given that PI is not available during patch uninstall, 
    //        it is at this point where we can easily open the patch and retrive the branch information.
    void ReportUnInstallTrainMetrics(const CString& strPatchList
                            , const CString& strProductCode
                            , ILogger& logger
                            , Ux& uxLogger)
    {
        ORPHANEDLDRBASELINER orphanedLdrBaseliner(strProductCode, logger);
        bool bLDRInstalled = orphanedLdrBaseliner.IsLdrInstalled();
        UxEnum::patchTrainEnum train = bLDRInstalled ? UxEnum::ptLDR : UxEnum::ptGDR;

        uxLogger.RecordPatchStream( strPatchList
                                    , strProductCode
                                    , train);

        //Record the package Train
        //Get the branch Info of the patch
        int iStart = 0;
        CString strMspFileName = strPatchList.Tokenize(L";", iStart);
        if (-1 == iStart)
        {
            strMspFileName = strPatchList;
        }

        MsiTableUtils msiTableUtils(MsiTableUtils::CreateMsiTableUtils(strMspFileName, MsiTableUtils::MspFile, logger));
        bool bIsError = false;
        UxEnum::patchTrainEnum mspTrain = UxEnum::GetTrainFromString(PatchInfo::GetBranch(msiTableUtils),bIsError);
        uxLogger.RecordPackagePatchType(mspTrain);
    }
};

}
