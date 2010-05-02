//-------------------------------------------------------------------------------------------------
// <copyright file="uxEnum.h" company="Microsoft">
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
    class UxEnum
    {
    public:

        //CPU Architecture
        /*
        #define PROCESSOR_ARCHITECTURE_INTEL            0
        #define PROCESSOR_ARCHITECTURE_MIPS             1
        #define PROCESSOR_ARCHITECTURE_ALPHA            2
        #define PROCESSOR_ARCHITECTURE_PPC              3
        #define PROCESSOR_ARCHITECTURE_SHX              4
        #define PROCESSOR_ARCHITECTURE_ARM              5
        #define PROCESSOR_ARCHITECTURE_IA64             6
        #define PROCESSOR_ARCHITECTURE_ALPHA64          7
        #define PROCESSOR_ARCHITECTURE_MSIL             8
        #define PROCESSOR_ARCHITECTURE_AMD64            9
        #define PROCESSOR_ARCHITECTURE_IA32_ON_WIN64    10
        */

        enum operationtypeEnum
        { 
            otUninitalized      = 0,
            otInstall           = 1,
            otUninstall         = 2,
            otRepair            = 3,
            otCreateLayout      = 4,
            otMaintenance       = 5,
            otCreateUnattend    = 6
        };

        enum spigotReportModeEnum
        {
            srmNone     = 0,
            srmCrash    = 1,
            srmError    = 2,
            srmNoError  = 3
        };

        enum spigotInstallerTypeEnum
        {
            sitNone             = 0,
            sitExe              = 1,
            sitMsi              = 2,
            sitMsp              = 3,
            sitFile             = 4,
            sitCartmanExe       = 5,
            sitMsu              = 6,
            sitServiceControl   = 7,
            sitIronManExe       = 8
        };

        enum spigotDownloadProtocolEnum
        {
            sdpNone             = 0,
            sdpBits             = 1,
            sdpHttp             = 2,
            sdpUrlMon           = 3,
            spdLAST_PROTOCOL    = sdpUrlMon, // Keep up to date with above list
            spdNUM_PROTOCOLS    = 4 // Keep up to date with above list
        };

        enum uiModeEnum
        {
            smNone          = 0,
            smInteractive   = 1,
            smSilent        = 2,
            smPassive       = 3
        };

        enum blockerEnum
        {
            bNone       = 0,
            bStop       = 1,    //Stop Blocker
            bWarn       = 2,    //Warn Blocker
            bProcess    = 3,    //Process Blocker
            bService    = 4,    //Service Blocker
            bDiskSpace  = 5,    //Diskspace blocker - Not enough diskspace.
            bSuccess    = 6,     //Success Blocker
            bInternal   = 7     //Internal Blocker e.g. Dialog creation failure, user command line error
        };

        enum phaseEnum
        {
            pNone               = 0,
            pUI                 = 1,   //The user Interface Phase
            pDownload           = 2,   //Downloading the item
            pInstall            = 3,   //Performing the installation of the item
            pRelatedProducts    = 4
        };

        enum actionEnum
        {
            aNone           = 0,
            aInstall        = 1,    //Performing an Install operation on the item.
            aRepair         = 2,    //Performing a Repair operation on the item.
            aUninstall      = 3,    //Performing an Uninstall operation on the item.
            aDownload       = 4,    //Downloading the item.
            aVerify         = 5,    //Verifying the item.
            aRollback       = 6,    //Rolling back the changes.
            aDecompress     = 7,    //Decompress the compressed payload.
            aMajorUpgrade   = 8     //This Install is a major upgrade
        };

        enum technologyEnum
        {
            tNone           = 0,
            tExe            = 1,    //A generic executable.
            tLocalExe       = 2,    //A local executable.
            tIronManExe     = 3,    //An Ironman package.
            tIronSpigotExe  = 4,    //An IronSpigot package.
            tHotIronExe     = 5,    //An HotIron Package
            tCartmanExe     = 6,    //A Cartman package.
            tFile           = 7,   //A file type
            tServiceControl = 8,   //A service Control
            tPatches        = 9,    //A collection of MSP packages
            tMsp            = 10,    //A Windows Installer Patch package.
            tMSUExe         = 11,    //A windows component update package.
            tMSI            = 12,   //A Windows Installer Installation package.
            tAgileMSI       = 13,   //A language pack style MSI package.
            tBITS           = 14,   //Download from a BITS server.
            tWinHttp        = 15,   //Download from an http server.
            tUrlMon         = 16    //Download using URLMon.
        };

        enum patchTrainEnum
        {
            ptNone  = 0,
            ptGDR   = 1,
            ptLDR   = 2
        };

        enum SkuEnum
        {
            sNone   = 0,
            sLocal  = 1,
            sWeb    = 2
        };

        enum UiStateType
        {
            uiNone              = 0,
            uiInitialization    = 1,
            uiWelcome           = 2,
            uiEula              = 3,
            uiPreReq            = 4,
            uiProgress          = 5,
            //Leave 6 empty to be compatible with IronSpigot.
            uiFinish            = 7,
            uiCrash             = 8
        };

        enum rebootPendingEnum
        {
            rpNone                  = 0, //no reboot detected
            rpDetected              = 1, //pending reboot detected (setup blocked due to pending reboot)
            rpIgnoredAuthoring      = 3, //pending reboot detected but ignored due to setup authoring
            rpIgnoredLoop           = 5  //pending reboot detected but ignored to avoid a reboot loop
        };

        /* These are the datapoints that Spigot is capturing for UX */
        enum spigotDataPointsEnum
        {
            sdpCpuArchitecture          = 16,
            sdpNumberOfProcessor        = 15,
            sdpCpuSpeed                 = 19,
            sdpSystemMemory             = 3,
            sdpSystemFreeDiskSpace      = 424,
            sdpOSAbbr                   = 4,
            sdpOSFullBuildNumber        = 453,
            sdpSystemLocale             = 13,
            sdpOsSpLevel                = 7,
            sdpIsInternal               = 426,
            sdpIsAdmin                  = 427,
            sdpIsRetailBuild            = 428,
            sdpDisplayedLcidId          = 450,
            sdpInstallTime              = 434,
            sdpOperationRequested       = 437,
            sdpOperationUi              = 438,
            sdpPackageName              = 439,
            sdpPackageVersion           = 457,      //Application version
            sdpInstallerVersion         = 440,      //Ironman version
            sdpApplicableSKU            = 441,      //stream	
            sdpCurrentState             = 442,
            sdpReturnCode               = 515,
            sdpNumberOfRefresh          = 449,
            sdpTimeToFirstWindow        = 423,
            sdpReportMode               = 446,
            sdpApplicableIfTime         = 454,
            sdpSystemRequirementCheckTime = 455,
            sdpStartupAppid             = 8,        //This datapoint is needed for filtering at the server end.  
            sdpFaultItem                = 461,
            sdpFaultPhase               = 462,
            sdpOSComplete               = 493,      //A combination of MajorVersion, MinorVersion, Productype, ServicePackMajor, ProcessorArchitecture and OSPreRelease
            sdpChainingPackage          = 494,
            sdpCurrentFlag              = 517,      //A combination of Report Mode, Action, Phase and UIMode
            sdpMPC                      = 516,  
            sdpItemStream               = 514,      //stream
            sdpBlocker                  = 499,
            sdpRebootCount              = 500,
            sdpRebootPending            = 616,
            sdpProfileFeatureMap        = 528,
            sdpCustomize                = 529,
            sdpCancelPage               = 530,
            sdpResultDetail             = 501,
            sdpPIDEdition               = 559,
            sdpPIDRange                 = 561,
            sdpPIDInstalledSkus         = 563,
            sdpPatchStream              = 570,      //Patch Name, Product Name, PatchTrain.
            sdpPatchType                = 571,      //The train the package is in.  Optional
            sdpWindowInstallerVersion   = 596,      //Window installer version
            sdpSKU                      = 599,      //Determine if it should be web or local SKU.
            sdpCurrentItemStep          = 642       //The step in an item execution before it fails.
        };

        static CString GetProtocolString( spigotDownloadProtocolEnum protocol )
        {
            switch(static_cast<DWORD>(protocol))
            {
                case sdpHttp:
                    return L"Http";
                case sdpUrlMon:
                    return L"UrlMon";
                case sdpBits:
                    return L"BITS";
                default:
                    return L"None";
            }
        }

        static CString GetUiModeString( uiModeEnum uimode )
        {
            switch(static_cast<DWORD>(uimode))
            {
                case smInteractive:
                    return L"Interactive";
                case smSilent:
                    return L"Silent";
                case smPassive:
                    return L"Passive";
                default:
                    return L"None";
            }
        }
        
        static CString GetPhaseString( phaseEnum phase )
        {
            switch(static_cast<DWORD>(phase))
            {
                case pDownload:
                    return L"D";
                case pInstall:
                    return L"I";
                default:
                    return L"UI";
            }
        }

        static CString GetActionString( actionEnum action )
        {      
            switch(static_cast<DWORD>(action))
            {
                case aDownload: 
                    return L"Download";
                case aInstall:
                    return L"Install";
                case aRepair:
                    return L"Repair";
                case aUninstall:
                    return L"Uninstall";
                case aVerify:
                    return L"Verify";
                case aRollback:
                    return L"Rollback";
                case aMajorUpgrade:
                    return L"MajorUpgrade";
                default:
                    return L"None";
            }
        }

        static CString GetTrainString( patchTrainEnum train)
        {
            switch(static_cast<DWORD>(train))
            {
                case ptLDR:
                    return L"LDR";
                case ptGDR:
                    return L"GDR";
                default:
                    return L"None";
            }
        }

        static patchTrainEnum GetTrainFromString(const CString& strTrain, bool& bIsError)
        {
            patchTrainEnum train;
            if (L"LDR" == strTrain)
            {
                train = UxEnum::ptLDR;
            }
            else if (L"GDR" == strTrain)
            {
                train = UxEnum::ptGDR;
            }
            else if (L"LDRBase" == strTrain)
            {
                train = UxEnum::ptLDR;
            }
            else if (0 == strTrain.CompareNoCase(L"None") || strTrain.IsEmpty())
            {
                train = UxEnum::ptNone;
            }
            else
            {
                train = UxEnum::ptNone;
                bIsError = true;
            }
            return train;
        }
    };
} // namespace IronMan
