//-----------------------------------------------------------------------
// <copyright file="BurnRebootResumeScenariosFixture.cs" company="Microsoft">
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
// <summary>
//     - Test fixture for Burn Reboot and Resume feature.
// </summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests.Burn.RebootResume
{
    using System;
    using System.Diagnostics;
    using Microsoft.Tools.WindowsInstallerXml.Test.Tests.Burn.CommonTestFixture;
    
    class BurnRebootResumeScenariosFixture : BurnCommonTestFixture
    {
        protected RebootType m_RebootType;

        public BurnRebootResumeScenariosFixture()
            : base()
        {
            this.Layout = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.LayoutManager.LayoutManager(new Microsoft.Tools.WindowsInstallerXml.Test.Burn.LayoutManager.Ux.SampleUx());
        }

        public enum RebootType
        {
            None,
            Beginning,
            Middle,
            End
        }

        public override void CleanUp()
        {
            System.Environment.SetEnvironmentVariable("VerificationUXRebootInstant", "", EnvironmentVariableTarget.Machine);
            System.Environment.SetEnvironmentVariable("VerificationUXRebootInstant", "");
            base.CleanUp();
        }

        public void RunTest(RebootType rebootType,
            InstallMode installMode,
            UiMode uiMode,
            UserType userType,
            bool useABundle)
        {
            m_RebootType = rebootType;
            InstallModeToUse = installMode;
            UiModeToUse = uiMode;
            UserTypeToUse = userType;
            UseABundle = useABundle;

            BuildLayout();
            InitializeMachineState();
            // BUGBUG TODO implement a RunRestartableScenario()
            // it will need to verify the exit code is 3010, re-run what is in the RunOnce key to restart Burn (fake a reboot)
            // since the reboot will be detoured and not actually occur
            VerifiableProperties = this.RunScenario(
                installMode,
                uiMode,
                userType);
        }

        private void BuildLayout()
        {
            // add a detour to burnstub.exe to hijack reboots 
            // so the machine isn't really shut down when any shutdown API is called
            using (Microsoft.Tools.WindowsInstallerXml.Test.Burn.Detours.HijackReboots hr = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.Detours.HijackReboots())
            {
                // add several Exe's that will each require a reboot.
                int totalItems = 3;
                for (int i = 0; i < totalItems; i++)
                {
                    string exeExitCode = "0";
                    if (i == 0 && this.m_RebootType == RebootType.Beginning)
                    {
                        exeExitCode = "3010";
                    }
                    else if (i == (totalItems / 2) && this.m_RebootType == RebootType.Middle)
                    {
                        exeExitCode = "3010";
                    }
                    else if (i == (totalItems - 1) && this.m_RebootType == RebootType.End)
                    {
                        exeExitCode = "3010";
                    }

                    this.Layout.AddExe(testExeFile, i.ToString() + "_" + System.IO.Path.GetFileName(testExeFile), null, true);
                    ((Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.BurnManifestOM.Chain.ExePackageElement)this.Layout.BurnManifest.Chain.Packages[i]).InstallArguments = "/s 1000 /ec " + exeExitCode + " /log %temp%\\TestExeLog_" + i.ToString() + ".txt ";
                    ((Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.BurnManifestOM.Chain.ExePackageElement)this.Layout.BurnManifest.Chain.Packages[i]).RepairArguments = "/s 1000 /ec " + exeExitCode + " /log %temp%\\TestExeLog_" + i.ToString() + ".txt ";
                    ((Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.BurnManifestOM.Chain.ExePackageElement)this.Layout.BurnManifest.Chain.Packages[i]).UninstallArguments = "/s 1000 /ec " + exeExitCode + " /log %temp%\\TestExeLog_" + i.ToString() + ".txt ";
                    //((Microsoft.Tools.WindowsInstallerXml.Test.Burn.BurnManifestGenerator.Chain.ExePackageElement)this.Layout.BurnManifest.Chain.Packages[i]).InstallCondition = ""; // BUGBUG what goes here?  burn.exe doesn't really have support for IsPresent and IsApplicable yet
                    ((Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnInstallableItems.ExeItem)this.Layout.ParameterInfo.Items.Items[i]).InstallCommandLine = "/s 1000 /ec " + exeExitCode + " /log %temp%\\TestExeLog_" + i.ToString() + ".txt ";
                    ((Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnInstallableItems.ExeItem)this.Layout.ParameterInfo.Items.Items[i]).RepairCommandLine = "/s 1000 /ec " + exeExitCode + " /log %temp%\\TestExeLog_" + i.ToString() + ".txt ";
                    ((Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnInstallableItems.ExeItem)this.Layout.ParameterInfo.Items.Items[i]).UninstallCommandLine = "/s 1000 /ec " + exeExitCode + " /log %temp%\\TestExeLog_" + i.ToString() + ".txt ";

                    if (this.UserTypeToUse == UserType.NormalUser)
                    {
                        ((Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.BurnManifestOM.Chain.ExePackageElement)this.Layout.BurnManifest.Chain.Packages[i]).PerMachine = "no";
                        ((Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnInstallableItems.ExeItem)this.Layout.ParameterInfo.Items.Items[i]).PerMachine = false;
                    }
                }

                if (UseABundle)
                {
                    this.Layout.GenerateBundle();
                }
                else
                {
                    this.Layout.GenerateLayout();
                }
                hr.CopyDependencies(this.Layout.LayoutFolder);
            }
        }

        private void InitializeMachineState()
        {
            // set VerificationUXRebootInstant=1
            // to cause the Verification UX to reboot after every payload that returns 3010
            System.Environment.SetEnvironmentVariable("VerificationUXRebootInstant", "1", EnvironmentVariableTarget.Machine);
            System.Environment.SetEnvironmentVariable("VerificationUXRebootInstant", "1");
        }

        public override BurnCommonVerifiableProperties RunScenario(InstallMode installMode, UiMode uiMode, UserType userType)
        {
            BurnCommonVerifiableProperties bvp = new BurnCommonVerifiableProperties();
            SetBurnCmdLineSwitchesToUse(installMode, uiMode);
            Args += " /forcerestart";

            bvp.BurnStartTime = DateTime.Now;

            BurnstubLauncher burnstubLauncher = new BurnstubLauncher();
            if (userType == UserType.NormalUser)
            {
                burnstubLauncher.UseNormalUser = true;
            }
            System.Diagnostics.Process burnProcess = burnstubLauncher.StartProcess(Layout, Args, UseABundle);
            if (uiMode == UiMode.Passive ||
                uiMode == UiMode.Silent)
            {
                burnProcess.WaitForExit();
            }
            else
            {
                throw new Exception("Running in UI mode is not supported");
            }
            bvp.BurnExitCode = burnProcess.ExitCode;
            bvp.BurnEndTime = DateTime.Now;

            bvp.BurnDhtmlLogs = BurnLog.GetAllBurnLogsFromEachUsersTempFolder("*_*_*.html", bvp.BurnStartTime, bvp.BurnEndTime); //new BurnLog(bvp.BurnStartTime, bvp.BurnEndTime);
            string tempToLookIn = System.Environment.ExpandEnvironmentVariables("%temp%");
            // look in the correct %temp% folder
            if (this.UserTypeToUse == UserType.NormalUser)
            {
                string adminUsername = System.Environment.ExpandEnvironmentVariables("%USERNAME%");
                string normalUsername = BurnstubLauncher.NormalUserName;
                tempToLookIn = tempToLookIn.Replace(adminUsername, normalUsername);
            }
            bvp.BurnMetricsData = new Microsoft.Tools.WindowsInstallerXml.Test.BurnFileMetrics.BurnMetricsData(bvp.BurnStartTime, bvp.BurnEndTime, 1, tempToLookIn);

            return bvp;
        }

        public override bool TestPasses()
        {
            bool retVal = true;
            string traceCategory = "Verify TestPasses";

            Trace.WriteLine("Start", traceCategory);
            Trace.Indent();

            retVal = true;
            retVal &= ExitCodeMatchesExpectation(-2147021886);  // -2147021886 == 3010
            retVal &= PayloadCacheIsAccurate();
            retVal &= RunKeysSetAccurately();  // run keys are set whenever Reboot() is called, even if the last item is installed and we don't really need to restart.  In that case, it is up to the UX to not call Reboot() before the install is complete.
            retVal &= ArpEntryIsAccurate();  // Arp entry is written when the install begins

            //retVal &= DhtmlLogContainsPerformerSucceededMessage();
            //retVal &= ItemStateMatchesExpectation();
            //retVal &= MetricsDataIsAccurate();

            Trace.WriteLineIf(retVal, "PASS", traceCategory);
            Trace.WriteLineIf(!retVal, "FAIL", traceCategory);

            Trace.Unindent();
            Trace.WriteLine("End", traceCategory);

            return retVal;
        }

        // Use the base class version of PayloadCacheIsAccurate()
        // it will verify all items are in the cache.
        // for now, Burn will cache everything up front, then start installing it.
        // however, it may go back to just caching and installing as needed.
        // in which case, this method would verify that just the items that have run are in the cache
        // and the items that have not run are not in the cache.
        //
        ///// <summary>
        ///// Verifies all Burn packages in the current Layout exist in the cache
        ///// </summary>
        ///// <returns>true if the cache is accurate</returns>
        //protected override bool PayloadCacheIsAccurate()
        //{
        //    bool retVal = true;
        //    string traceCategory = "Verify PayloadCacheIsAccurate";

        //    Trace.WriteLine("Start", traceCategory);
        //    Trace.Indent();

        //    string perMachineCacheRoot = System.Environment.ExpandEnvironmentVariables(@"%ProgramData%\Apps\Cache");
        //    string perUserCacheRoot = System.Environment.ExpandEnvironmentVariables(@"%LOCALAPPDATA%\Apps\Cache");
        //    string cacheRoot = perMachineCacheRoot;

        //    foreach (ParameterInfoConfigurator.BurnItems.BurnBaseItems item in this.Layout.ParameterInfo.Items.Items)
        //    {
        //        Type t = item.GetType();
        //        if (t == typeof(ParameterInfoConfigurator.BurnInstallableItems.ExeItem) ||
        //            t == typeof(ParameterInfoConfigurator.BurnInstallableItems.MsiItem) ||
        //            t == typeof(ParameterInfoConfigurator.BurnInstallableItems.MspItem))
        //        {
        //            cacheRoot = perMachineCacheRoot;
        //            if (((ParameterInfoConfigurator.BurnItems.InstallableItems.BurnInstallableItem)item).PerMachine == false)
        //            {
        //                // admin (current user) installed a per-user package
        //                cacheRoot = perUserCacheRoot;

        //                if (this.UserTypeToUse == UserType.NormalUser)
        //                {
        //                    // non admin (normal user) installed a per-user package
        //                    foreach (string localAppData in Microsoft.Tools.WindowsInstallerXml.Test.Utilities.UserUtilities.GetAllUserLocalAppDataPaths())
        //                    {
        //                        if (localAppData.Contains(BurnstubLauncher.NormalUserName))
        //                        {
        //                            cacheRoot = Path.Combine(localAppData, "Apps\\Cache");
        //                        }
        //                    }
        //                }
        //            }

        //            string itemName = ((ParameterInfoConfigurator.BurnItems.InstallableItems.BurnInstallableItem)item).Name;
        //            string cacheId = ((ParameterInfoConfigurator.BurnItems.InstallableItems.BurnInstallableItem)item).CacheId;
        //            string itemRootPath = Path.Combine(cacheRoot, cacheId);
        //            string itemFullPath = Path.Combine(itemRootPath, Path.GetFileName(itemName)); // Paths are removed from the Item name, only the filename is used.

        //            if ((((this.m_RebootType == RebootType.Beginning) && item.Id.Contains("0")) ||  // verify 1st payload only
        //                ((this.m_RebootType == RebootType.Middle) && (item.Id.Contains("0") || item.Id.Contains("1"))) || // verify 1st & 2nd payloads only
        //                (this.m_RebootType == RebootType.End))  && // verify all payloads
        //                (((ParameterInfoConfigurator.BurnItems.InstallableItems.BurnInstallableItem)item).Cache == true) )
        //            {
        //                    if (!File.Exists(itemFullPath))
        //                    {
        //                        retVal = false;
        //                        Trace.WriteLine("ERROR - Missing File: " + itemFullPath, traceCategory);
        //                    }
        //                    else
        //                    {
        //                        Trace.WriteLine("Found File: " + itemFullPath, traceCategory);
        //                    }

        //                    if (null != ((ParameterInfoConfigurator.BurnItems.InstallableItems.BurnInstallableItem)item).SubFiles)
        //                    {
        //                        foreach (ParameterInfoConfigurator.BurnItems.InstallableItems.SubFile subFile in ((ParameterInfoConfigurator.BurnItems.InstallableItems.BurnInstallableItem)item).SubFiles)
        //                        {
        //                            string subFileFullPath = Path.Combine(itemRootPath, subFile.Name);
        //                            if (!File.Exists(subFileFullPath))
        //                            {
        //                                retVal = false;
        //                                Trace.WriteLine("ERROR - Missing File: " + subFileFullPath, traceCategory);
        //                            }
        //                            else
        //                            {
        //                                Trace.WriteLine("Found File: " + subFileFullPath, traceCategory);
        //                            }
        //                        }
        //                    }
                        
        //            }
        //            else
        //            {
        //                // verify this payload is not cached 
        //                // (i.e. it was not run cause a reboot was initiated before it should have run)
        //                if (File.Exists(itemFullPath))
        //                {
        //                    retVal = false;
        //                    Trace.WriteLine("ERROR - File exists that should NOT exist: " + itemFullPath, traceCategory);
        //                }
        //                else
        //                {
        //                    Trace.WriteLine("Did not find file (that's good): " + itemFullPath, traceCategory);
        //                }
        //            }
        //        }
        //    }

        //    Trace.WriteLineIf(retVal, "PASS", traceCategory);
        //    Trace.WriteLineIf(!retVal, "FAIL", traceCategory);

        //    Trace.Unindent();
        //    Trace.WriteLine("End", traceCategory);

        //    return retVal;
        //}


        // BUGBUG TODO either delete these if they aren't needed by this fixture or move them to the common fixture so they aren't duplicated

        //private bool DhtmlLogContainsPerformerSucceededMessage()
        //{
        //    bool foundStringInAnyDhtmlLog = false;
        //    string traceCategory = "Verify DhtmlLogContainsPerformerSucceededMessage";

        //    Trace.WriteLine("Start", traceCategory);
        //    Trace.Indent();

        //    string logStringToVerifyExists = "";
        //    Trace.WriteLine("PayloadType=" + this.m_PayloadType.ToString(), traceCategory);
        //    switch (this.m_PayloadType)
        //    {
        //        case PayloadType.Exe:
        //            logStringToVerifyExists = System.IO.Path.GetFileName(testExeFile);
        //            break;
        //        case PayloadType.MsiPerMachine:
        //            logStringToVerifyExists = System.IO.Path.GetFileName(testMsiPerMachineFile);
        //            break;
        //        case PayloadType.MsiPerUser:
        //            logStringToVerifyExists = System.IO.Path.GetFileName(testMsiPerUserFile);
        //            break;
        //        case PayloadType.Msp:
        //            logStringToVerifyExists = System.IO.Path.GetFileName(testMspFile);
        //            break;
        //        case PayloadType.FileAndExe:
        //            logStringToVerifyExists = System.IO.Path.GetFileName(testFileFile);
        //            break;
        //    }

        //    if (this.VerifiableProperties.BurnDhtmlLogs.Count < 1)
        //    {
        //        // no logs to verify
        //    }
        //    else
        //    {
        //        foreach (BurnLog bl in this.VerifiableProperties.BurnDhtmlLogs)
        //        {
        //            foundStringInAnyDhtmlLog |= bl.LogContainsRegExMatch(logStringToVerifyExists, System.Text.RegularExpressions.RegexOptions.None);
        //        }
        //    }
        //    Trace.WriteLineIf(foundStringInAnyDhtmlLog, "PASS: performer logged", traceCategory);
        //    Trace.WriteLineIf(!foundStringInAnyDhtmlLog, "FAIL: performer not logged", traceCategory);

        //    Trace.Unindent();
        //    Trace.WriteLine("End", traceCategory);

        //    return foundStringInAnyDhtmlLog;
        //}

        //private bool ItemStateMatchesExpectation()
        //{
        //    bool retVal = false;
        //    string ecVal = "ec 000 ";
        //    string traceCategory = "Verify ItemStateMatchesExpectation";

        //    Trace.WriteLine("Start", traceCategory);
        //    Trace.Indent();
        //    Trace.WriteLine("PayloadType=" + this.m_PayloadType.ToString(), traceCategory);

        //    if (this.m_InstallMode == InstallMode.repair) ecVal = "ec 0000 ";
        //    if (this.m_InstallMode == InstallMode.uninstall) ecVal = "ec 00000 ";

        //    switch (this.m_PayloadType)
        //    {
        //        case PayloadType.Exe:
        //            retVal = FileExistsWithValidTimestampAndContents("%temp%\\TestExeLog.txt", ecVal, RegexOptions.IgnoreCase);
        //            break;
        //        case PayloadType.MsiPerMachine:
        //            retVal = Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.Utility.MsiUtils.IsProductInstalled(
        //                Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.Utility.MsiUtils.GetMSIProductCode(testMsiPerMachineFile));
        //            break;
        //        case PayloadType.MsiPerUser:
        //            retVal = Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.Utility.MsiUtils.IsProductInstalled(
        //                Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.Utility.MsiUtils.GetMSIProductCode(testMsiPerUserFile));
        //            break;
        //        case PayloadType.Msp:
        //            retVal = Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.Utility.MsiUtils.IsPatchInstalled(testMspFile);
        //            break;
        //        case PayloadType.FileAndExe:
        //            // FILE does not install so nothing to verify with it, but check the EXE.
        //            retVal = FileExistsWithValidTimestampAndContents("%temp%\\TestExeLog.txt", ecVal, RegexOptions.IgnoreCase);
        //            break;
        //    }

        //    Trace.WriteLineIf(retVal, "PASS: item did install/repair/uninstall as expected", traceCategory);
        //    Trace.WriteLineIf(!retVal, "FAIL: item did NOT install/repair/uninstall as expected", traceCategory);

        //    Trace.Unindent();
        //    Trace.WriteLine("End", traceCategory);

        //    return retVal;
        //}

        //private bool FileExistsWithValidTimestampAndContents(string FileName, string RegEx, RegexOptions RegexOptions)
        //{
        //    bool retVal = false;
        //    string expandedFileName = System.Environment.ExpandEnvironmentVariables(FileName);

        //    // look in the correct %temp% folder
        //    if (this.m_UserType == UserType.NormalUser)
        //    {
        //        string adminUsername = System.Environment.ExpandEnvironmentVariables("%USERNAME%");
        //        string normalUsername = BurnstubLauncher.NormalUserName;
        //        expandedFileName = expandedFileName.Replace(adminUsername, normalUsername);
        //    }

        //    try
        //    {
        //        DateTime lastWriteTime = File.GetLastWriteTime(expandedFileName);
        //        if (lastWriteTime > this.VerifiableProperties.BurnStartTime &&
        //            lastWriteTime < this.VerifiableProperties.BurnEndTime)
        //        {
        //            TextReader reader = new StreamReader(expandedFileName);
        //            string logFileContent = reader.ReadToEnd();
        //            Match m = Regex.Match(logFileContent, RegEx, RegexOptions);
        //            if (m.Success)
        //            {
        //                retVal = true;
        //            }
        //        }
        //        else
        //        {
        //            Trace.WriteLine(String.Format("File: {0} lastWriteTime: {1}  BurnStartTime: {2} BurnEndTime: {3}", FileName, lastWriteTime.ToString(), this.VerifiableProperties.BurnStartTime.ToString(), this.VerifiableProperties.BurnEndTime.ToString()), "FileExistsWithValidTimestampAndContents");
        //        }
        //    }
        //    catch
        //    {
        //    }

        //    return retVal;
        //}

        //protected override bool MetricsDataIsAccurate()
        //{
        //    bool metricsDataIsAccurate = true;
        //    string traceCategory = "Verify MetricsDataIsAccurate";

        //    Trace.WriteLine("Start", traceCategory);
        //    Trace.Indent();

        //    // perform the standard verifications that should apply to all Metrics scenarios 
        //    metricsDataIsAccurate &= base.MetricsDataIsAccurate();

        //    // TODO: add additional verifications specific to this fixture's scenarios.

        //    Trace.WriteLineIf(metricsDataIsAccurate, "PASS ", traceCategory);
        //    Trace.WriteLineIf(!metricsDataIsAccurate, "FAIL ", traceCategory);

        //    Trace.Unindent();
        //    Trace.WriteLine("End", traceCategory);

        //    return metricsDataIsAccurate;
        //}
    }
}
