//-------------------------------------------------------------------------------------------------
// <copyright file="IFrostUserExperience.h" company="Microsoft">
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
//    IFrostUserExperience is the interface mapping to IBurnUserExperience
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once

namespace Microsoft
{
namespace Tools
{
namespace WindowsInstallerXml
{
namespace Test
{
namespace Frost
{
    public interface class IFrostUserExperience
    {
        HRESULTS Initialize(Int32 nCmdShow, SETUP_RESUME ResumeState);
        HRESULTS Run();
        void Uninitialize();

        CommandID OnDectectBegin(UInt32 cPackages);
        CommandID OnDetectPackageBegin(String^ wzPackageId);
        void OnDetectPackageComplete(String^ wzPackageId, HRESULTS hrStatus, CUR_PACKAGE_STATE state);
        void OnDetectComplete(HRESULTS hrStatus);

        CommandID OnPlanBegin(UInt32 cPackages);
        CommandID OnPlanPackageBegin(String^ wzPackageId, PKG_REQUEST_STATE% reqState);
        void OnPlanPackageComplete(
            String^ wzPackageId, 
            HRESULTS hrStatus,
            CUR_PACKAGE_STATE state,
            PKG_REQUEST_STATE requested,
            PKG_ACTION_STATE execute,
            PKG_ACTION_STATE rollback);
        void OnPlanComplete(HRESULTS hrStatus);

        CommandID OnApplyBegin();
        CommandID OnRegisterBegin();
        void OnRegisterComplete(HRESULTS hrStatus);
        void OnUnregisterBegin();
        void OnUnregisterComplete(HRESULTS hrStatus);
        void OnCacheComplete(HRESULTS hrStatus);
        CommandID OnExecuteBegin(UInt32 cExecutingPackages);
        CommandID OnExecutePackageBegin(String^ wzPackageId,bool fExecute);
        CommandID OnError(String^ wzPackageId, UInt32 dwCode,String^ wzError,UInt32 dwUIHint);
        CommandID OnProgress(UInt32 dwProgressPercentage,UInt32 dwOverallPercentage);
        CommandID OnExecuteMsiMessage(String^ wzPackageID, INSTALLMESSAGE mt, UInt32 uiFlags, String^wzMessage);
        void OnExecutePackageComplete(String^ wzPackageId,HRESULTS hrExitCode);
        void OnExecuteComplete(HRESULTS hrStatus);
        bool OnRestartRequired();
        void OnApplyComplete(HRESULTS hrStatus);
        int ResolveSource(String^ wzPackageID, String^ wzPackageOrContainerPath);
        bool CanPackageBeDownloaded();
    };
}
}
}
}
}
