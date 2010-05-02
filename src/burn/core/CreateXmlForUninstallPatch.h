//-------------------------------------------------------------------------------------------------
// <copyright file="CreateXmlForUninstallPatch.h" company="Microsoft">
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
//   This creates a stripped down ParameterInfo.xml used when the UninstallPatch switch is passed
//   It includes only the needed elements
//   including a single CleanupBlock item containing the patch to remove
//   No other SetupItems are authored
//   EnterMaintenanceModeIf that is AlwaysTrue
//   Does not contain
//      Configuration
//      Blockers
//      Empty SystemCheck 
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once

#include "common\MsiUtils.h"
#include "common\MsiTableUtils.h"

namespace IronMan
{

//------------------------------------------------------------------------------
// Class: IronMan::CreateXmlForUninstallPatch
// This creates a stripped down ParameterInfo.xml used when the 
// UninstallPatch switch is passed
// It includes only the needed elements
// including a single CleanupBlock item containing the patch to remove
// No other SetupItems are authored
// EnterMaintenanceModeIf that is AlwaysTrue
// Does not contain
// Configuration
// Blockers
// Empty SystemCheck 
//------------------------------------------------------------------------------
class CreateXmlForUninstallPatch
{
private:
    
    //------------------------------------------------------------------------------
    // GetTemplateParameterInfoString()
    // Returns a template ParameterInfo string that can be used to create
    // a real ParameterInfo string after substituting the real patch code for
    // {patchcode}, the original UI\@Version with the version and 
    // the original UI\@Dll with the Ui Dll
    //------------------------------------------------------------------------------
    static const LPCSTR GetTemplateParameterInfoString()
    {
        static const CHAR templateParameterInfoString[] =
            "<?xml version=\"1.0\" encoding=\"utf-16\"?>"
            "<Setup xmlns=\"http://schemas.microsoft.com/Setup/2008/01/im\" xmlns:im=\"http://schemas.microsoft.com/Setup/2008/01/im\" SetupVersion=\"1.0\" >"
            "    <UI Name=\"#(loc.UIProductName)\" Version=\"{version}\" Dll=\"{uidll}\" />"
            "    <EnterMaintenanceModeIf>"
            "        <AlwaysTrue />"
            "    </EnterMaintenanceModeIf>"
            "    <Ux UxDllPayloadId='BurnUx_dll'>"
            "        <Payload Id='BurnUx_dll' FilePath='BurnUx.dll' Packaging='embedded' SourcePath='BurnUx.dll' />"
            "    </Ux>"
            "    <Registration Id='{patchcode}' ExecutableName='setup.exe' PerMachine='yes' />"
            "    <Items DownloadRetries=\"3\" DelayBetweenRetries=\"6\" >"
            "        <CleanupBlock InstalledProductSize=\"100\" CanonicalTargetName=\"#(loc.UIProductName)\" DoUnAdvertiseFeaturesOnRemovePatch=\"false\">"
            "            <IsPresent>"
            "                <Exists>"
            "                    <MsiGetCachedPatchPath PatchCode=\"{patchcode}\" />"
            "                </Exists>"
            "            </IsPresent>"
            "            <ApplicableIf>"
            "                <AlwaysTrue />"
            "            </ApplicableIf>"
            "            <ActionTable>"
            "                <InstallAction IfPresent=\"noop\" IfAbsent=\"noop\" OnFailureBehavior=\"Continue\" />"
            "                <UninstallAction IfPresent=\"install\" IfAbsent=\"noop\" />"
            "                <RepairAction IfPresent=\"noop\" IfAbsent=\"noop\" />"
            "            </ActionTable>"
            "            <RemovePatch PatchCode=\"{patchcode}\" />"
            "        </CleanupBlock>"
            "    </Items>"
            "    <SystemCheck>"
            "        <ProcessBlocks/>"
            "        <ServiceBlocks/>"
            "    </SystemCheck>"
            "</Setup>"
            ;
        return templateParameterInfoString;
    }
public:
    //------------------------------------------------------------------------------
    // Returns a string containing a ParameterInfo with the data from the Patch
    // integrated into it.
    //------------------------------------------------------------------------------
    static const CComBSTR CreateParameterInfo(const CString& patchCode
                                            , const CString& uiVersion
                                            , const CString& uiDll
                                            , ILocalizedData& localizedData
                                            , ILogger& logger)
    {
        CString parameterInfo(GetTemplateParameterInfoString());
        parameterInfo.Replace(L"{patchcode}", patchCode);
        parameterInfo.Replace(L"{version}", uiVersion);
        parameterInfo.Replace(L"{uidll}", uiDll);
        parameterInfo.Replace(L"#(loc.UIProductName)", localizedData.GetLocalizedText(L"#(loc.UIProductName)"));
        return CComBSTR(parameterInfo);
    }

    //------------------------------------------------------------------------------
    // Returns a string containing a ParameterInfo with the data from the Patch
    // integrated into it.
    //------------------------------------------------------------------------------
    static const CComBSTR UpdateLocalizedData(const CComBSTR& localizedDataOld, const CString& patchCode, ILogger& logger)
    {
        CString localizedDataNew(localizedDataOld);
        CString uiProductName(MSIUtils::GetMspDisplayName(patchCode, logger));
        CString textElement(L"<Text ID=\"#(loc.UIProductName)\"       LocalizedText=\"" + uiProductName + L"\" />");
        CString idUIProductName = L"ID=\"#(loc.UIProductName)\"";
        int iStartUIProductName = localizedDataNew.Find(idUIProductName);
        if ( -1 == iStartUIProductName )
        {
            // If a loc.UIProductName does not exist yet, add a new one
            // Add #(loc.UIProductName) as first Text element under the Language element
            // do this by inserting before first <Text tag
            int iFirstTextTag = localizedDataNew.Find(L"<Text");
            if ( -1 != iFirstTextTag)
            {
                localizedDataNew = localizedDataNew.Left(iFirstTextTag) 
                    + textElement
                    + localizedDataNew.Right(localizedDataNew.GetLength() - iFirstTextTag);
            }
        }
        else
        {
            // A loc.UIProductName already exists, so replace the localizedText with the DisplayName of the MSP
            int iOpenQuoteOfLocalizedText = localizedDataNew.Find(L"\"", iStartUIProductName + idUIProductName.GetLength());
            int iCloseQuoteOfLocalizedText = localizedDataNew.Find(L"\"", iOpenQuoteOfLocalizedText + 1);
            localizedDataNew = localizedDataNew.Left(iOpenQuoteOfLocalizedText + 1) 
                + uiProductName
                + localizedDataNew.Right(localizedDataNew.GetLength() - iCloseQuoteOfLocalizedText);
        }
        return CComBSTR(localizedDataNew);
    }
};
}
