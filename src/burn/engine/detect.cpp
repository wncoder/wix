//-------------------------------------------------------------------------------------------------
// <copyright file="detect.cpp" company="Outercurve Foundation">
//   Copyright (c) 2004, Outercurve Foundation.
//   This software is released under Microsoft Reciprocal License (MS-RL).
//   The license and further copyright text can be found in the file LICENSE.TXT
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
//
// <summary>
//    Module: Core
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"

// internal function definitions


// function definitions

extern "C" void DetectReset(
    __in BURN_PACKAGES* pPackages
    )
{
    for (DWORD iPackage = 0; iPackage < pPackages->cPackages; ++iPackage)
    {
        BURN_PACKAGE* pPackage = pPackages->rgPackages + iPackage;

        pPackage->currentState = BOOTSTRAPPER_PACKAGE_STATE_UNKNOWN;

        pPackage->cache = BURN_CACHE_STATE_NONE;
        for (DWORD iPayload = 0; iPayload < pPackage->cPayloads; ++iPayload)
        {
            BURN_PACKAGE_PAYLOAD* pPayload = pPackage->rgPayloads + iPayload;
            pPayload->fCached = FALSE;
        }

        if (BURN_PACKAGE_TYPE_MSI == pPackage->type)
        {
            for (DWORD iFeature = 0; iFeature < pPackage->Msi.cFeatures; ++iFeature)
            {
                BURN_MSIFEATURE* pFeature = pPackage->Msi.rgFeatures + iFeature;

                pFeature->currentState = BOOTSTRAPPER_FEATURE_STATE_UNKNOWN;
            }
        }
        else if (BURN_PACKAGE_TYPE_MSP == pPackage->type)
        {
            ReleaseNullMem(pPackage->Msp.rgTargetProducts);
            pPackage->Msp.cTargetProductCodes = 0;
        }
    }

    for (DWORD iPatchInfo = 0; iPatchInfo < pPackages->cPatchInfo; ++iPatchInfo)
    {
        MSIPATCHSEQUENCEINFOW* pPatchInfo = pPackages->rgPatchInfo + iPatchInfo;
        pPatchInfo->dwOrder = 0;
        pPatchInfo->uStatus = 0;
    }
}
