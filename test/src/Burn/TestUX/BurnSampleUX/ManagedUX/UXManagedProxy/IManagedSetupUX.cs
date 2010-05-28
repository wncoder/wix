//-----------------------------------------------------------------------
// <copyright file="IManagedSetupUX.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
//    
//    The use and distribution terms for this software are covered by the
//    Common Public License 1.0 (http://opensource.org/licenses/cpl.php)
//    which can be found in the file CPL.TXT at the root of this distribution.
//    By using this software in any fashion, you are agreeing to be bound by
//    the terms of this license.
//    
//    You must not remove this notice, or any other, from this software.
// </copyright>
// <summary>Managed side IBurnUserExperience interface</summary>
//-----------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Text;

namespace ManagedSetupUX
{
    public interface IManagedSetupUX
    {
        bool Initialize(UInt32 mode);

        void Uninitialize();

        void ShowForm();

        CommandID OnDetectBegin(UInt32 numPackages);

        CommandID OnDetectPackageBegin(String packageID);

        bool OnDetectPackageComplete(String packageID,
                                     Int32 hrStatus,
                                     Int32 state);

        bool OnDetectComplete(int hrStatus);

        CommandID OnPlanBegin(UInt32 numPackages);

        CommandID OnPlanPackageBegin( ref String packageID);

        bool OnPlanPackageComplete(String packageID,
                                   Int32 hrStatus,
                                   Int32 state,
                                   Int32 requested,
                                   Int32 execute,
                                   Int32 rollback);

        bool OnPlanComplete(int hrStatus);

        CommandID OnApplyBegin();

        CommandID OnRegisterBegin();

        bool OnRegisterComplete(Int32 hrStatus);

        bool OnUnregisterBegin();

        bool OnUnregisterComplete(Int32 hrStatus);

        bool OnApplyComplete(Int32 hrStatus);

        CommandID OnProgress(Int32 progressPct, Int32 overallProgress);
        CommandID OnDownloadProgress(Int32 progressPct, Int32 overallProgress);
        CommandID OnExecuteProgress(Int32 progressPct, Int32 overallProgress);

        bool OnCacheComplete(int hrStatus);

        CommandID OnExecuteBegin(uint numPackages);

        CommandID OnExecutePackageBegin(String packageID, bool installing);
        bool OnExecutePackageComplete(String packageID, Int32 hrStatus);

        bool OnExecuteComplete(Int32 hrStatus);

        Int32 OnError(UInt32 code, String errMsg, UInt32 mbflag);
    }
}