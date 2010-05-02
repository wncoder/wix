//-----------------------------------------------------------------------
// <copyright file="BurnCommonTestFixture.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>
//     - Contains methods commonly used by Burn test fixtures
// </summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests.Burn.CommonTestFixture
{
    using System;
    using System.Collections.Generic;
    using System.Diagnostics;
    using System.IO;
    using System.IO.Pipes;
    using System.Runtime.InteropServices;
    using System.Xml;
    using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.Utility;
    using Microsoft.Tools.WindowsInstallerXml.Test.Burn;
    using Microsoft.Tools.WindowsInstallerXml.Test.BurnFileMetrics;
    using Microsoft.Win32;

    public class BurnCommonTestFixture
    {
        public enum InstallMode
        {
            install,
            repair,
            maintenanceMode,
            uninstall
        }

        public enum UiMode
        {
            UI,
            Silent,
            Passive,
            PassivePromptOnError
        }

        public enum BundleType
        {
            PerMachine,
            PerUser
        }

        public enum PayloadType
        {
            Exe,
            MsiPerMachine,
            MsiPerUser,
            MsiPerUserExtCab,
            Msp,
            File,
            FileAndExe
        }

        public enum PayloadLocation
        {
            InLocalLayout,
            ToBeDownloaded,
            InDownloadCache
        }

        public enum PayloadOutcome
        {
            Succeed,
            Fail
        }

        public enum UserType
        {
            CurrentUser,
            AdminUser,
            NormalUser
        }

        // BUGBUG need to reference %WIX_ROOT% in the paths below.  But that can't happen until we move our tests into WIX_ROOT instead of BURN_TEST_ENLISTMENT_ROOT
        protected string testMsiPerMachineFile = System.Environment.ExpandEnvironmentVariables(@"%WIX_ROOT%\test\data\BurnTestPayloads\Products\Hello_World\hello_world.msi");
        protected string testMsiPerUserFile = System.Environment.ExpandEnvironmentVariables(@"%WIX_ROOT%\test\data\BurnTestPayloads\Products\PerUserMsi\hello_world_non_admin.msi");
        protected string testMsiPerUserExtCabMsiFile = System.Environment.ExpandEnvironmentVariables(@"%WIX_ROOT%\test\data\BurnTestPayloads\Products\PerUserMsiExtCab\HelloWorldNonAdmin_Readme_External.msi");
        protected string testMsiPerUserExtCabCabFile = System.Environment.ExpandEnvironmentVariables(@"%WIX_ROOT%\test\data\BurnTestPayloads\Products\PerUserMsiExtCab\HelloWorldNonAdmin_Readme_External.cab");
        protected string testMsiFile = System.Environment.ExpandEnvironmentVariables(@"%WIX_ROOT%\test\data\BurnTestPayloads\MSIsandMSPs\RtmProduct\product.msi");
        protected string testMspFile = System.Environment.ExpandEnvironmentVariables(@"%WIX_ROOT%\test\data\BurnTestPayloads\MSIsandMSPs\GDR1\gdr1.msp"); // MSP that will target testMsiFile
        protected string testExeFile = System.Environment.ExpandEnvironmentVariables(@"%WIX_ROOT%\test\data\BurnTestPayloads\Products\TestExe\TestExe.exe");
        protected string testFileFile = System.Environment.ExpandEnvironmentVariables(@"%WIX_ROOT%\test\data\BurnTestPayloads\TxtFiles\10000000b.txt");

        protected string testMsiConditionalPass1File = System.Environment.ExpandEnvironmentVariables(@"%WIX_ROOT%\test\data\BurnTestPayloads\Products\Hello_Reg_conditionalPass\hello_world.msi");
        protected string testMsiConditionalPass2File = System.Environment.ExpandEnvironmentVariables(@"%WIX_ROOT%\test\data\BurnTestPayloads\Products\Hello_Reg_conditionalPass\hello_world2.msi");
        protected string testMsiConditionalPass3File = System.Environment.ExpandEnvironmentVariables(@"%WIX_ROOT%\test\data\BurnTestPayloads\Products\Hello_Reg_conditionalPass\hello_world3.msi");

        // Make all URLs reference a root that is customizable so others can easily setup a web server to test download scenarios.
        protected string testMsiPerMachineUrl = System.Environment.ExpandEnvironmentVariables(@"%BURN_TEST_WEBSERVER_ROOT%/BurnTestPayloads/Products/Hello_World/hello_world.msi");
        protected string testFileUrl = System.Environment.ExpandEnvironmentVariables(@"%BURN_TEST_WEBSERVER_ROOT%/BurnTestPayloads/TxtFiles/10000000b.txt");
        protected string testMsiUrl = System.Environment.ExpandEnvironmentVariables(@"%BURN_TEST_WEBSERVER_ROOT%/BurnTestPayloads/MSIsandMSPs/RtmProduct/product.msi");
        protected string testMspUrl = System.Environment.ExpandEnvironmentVariables(@"%BURN_TEST_WEBSERVER_ROOT%/BurnTestPayloads/MSIsandMSPs/GDR1/gdr1.msp");
        protected string testExeUrl = System.Environment.ExpandEnvironmentVariables(@"%BURN_TEST_WEBSERVER_ROOT%/BurnTestPayloads/Products/TestExe/TestExe.exe");
        protected string testMsiPerUserUrl = System.Environment.ExpandEnvironmentVariables(@"%BURN_TEST_WEBSERVER_ROOT%/BurnTestPayloads/Products/PerUserMsi/hello_world_non_admin.msi");
        protected string testMsiPerUserExtCabMsiUrl = System.Environment.ExpandEnvironmentVariables(@"%BURN_TEST_WEBSERVER_ROOT%/BurnTestPayloads/Products/PerUserMsiExtCab/HelloWorldNonAdmin_Readme_External.msi");
        protected string testMsiPerUserExtCabCabUrl = System.Environment.ExpandEnvironmentVariables(@"%BURN_TEST_WEBSERVER_ROOT%/BurnTestPayloads/Products/PerUserMsiExtCab/HelloWorldNonAdmin_Readme_External.cab");
        protected string testMsiConditionalPass3Url = Environment.ExpandEnvironmentVariables(@"%BURN_TEST_WEBSERVER_ROOT%/BurnTestPayloads/Products/Hello_Reg_conditionalPass/hello_world3.msi");

        private string m_Args;
        public string Args
        {
            get
            {
                return m_Args;
            }
            set
            {
                m_Args = value;
            }
        }

        private Microsoft.Tools.WindowsInstallerXml.Test.Burn.LayoutManager.LayoutManager m_Layout;
        public Microsoft.Tools.WindowsInstallerXml.Test.Burn.LayoutManager.LayoutManager Layout
        {
            get
            {
                return m_Layout;
            }
            set
            {
                m_Layout = value;
            }
        }

        private InstallMode m_InstallMode;
        public InstallMode InstallModeToUse
        {
            get { return m_InstallMode; }
            set { m_InstallMode = value; }
        }
        private UiMode m_UiMode;
        public UiMode UiModeToUse
        {
            get { return m_UiMode; }
            set { m_UiMode = value; }
        }
        private UserType m_UserType;
        public UserType UserTypeToUse
        {
            get { return m_UserType; }
            set { m_UserType = value; }
        }

        private bool m_UseABundle = false;
        protected bool UseABundle
        {
            get { return m_UseABundle; }
            set { m_UseABundle = value; }
        }

        protected BurnCommonVerifiableProperties VerifiableProperties;

        /// <summary>
        /// Each test fixture that inherits from the CommonTestFixture should override this
        /// CleanUp method to restore the machine state to whatever it was before the fixture
        /// modifies it.
        /// </summary>
        public virtual void CleanUp()
        {
            this.KillAllRunningBurnLayoutProcesses();

            EngineCleanup ec = new EngineCleanup(this.Layout);
            ec.CleanupEverything();
        }

        /// <summary>
        /// Each test fixture that inherits from the CommonTestFixture should override this
        /// TestPasses method to make it determine if the test passed or failed.
        /// </summary>
        /// <returns>true if the test passes, false otherwise</returns>
        public virtual bool TestPasses()
        {
            bool retVal = false;
            string traceCategory = "Verify TestPasses";

            Trace.WriteLine("Start", traceCategory);
            Trace.Indent();
            Trace.WriteLineIf(!retVal, "FAIL", traceCategory);
            Trace.Unindent();
            Trace.WriteLine("End", traceCategory);

            return retVal;
        }

        public virtual BurnCommonVerifiableProperties RunScenario(InstallMode installMode, UiMode uiMode, UserType userType)
        {
            return RunScenario(this.Layout, installMode, uiMode, userType);
        }
        public virtual BurnCommonVerifiableProperties RunScenario(Microsoft.Tools.WindowsInstallerXml.Test.Burn.LayoutManager.LayoutManager layoutToUse, 
            InstallMode installMode, UiMode uiMode, UserType userType)
        {
            BurnCommonVerifiableProperties bvp = new BurnCommonVerifiableProperties();
            Args = "";
            SetBurnCmdLineSwitchesToUse(installMode, uiMode);

            bvp.BurnStartTime = DateTime.Now;

            BurnstubLauncher burnstubLauncher = new BurnstubLauncher();
            if (userType == UserType.NormalUser)
            {
                burnstubLauncher.UseNormalUser = true;
            }
            System.Diagnostics.Process burnProcess = burnstubLauncher.StartProcess(layoutToUse, Args, UseABundle);
            if (uiMode == UiMode.Passive ||
                uiMode == UiMode.Silent)
            {
                burnProcess.WaitForExit();
            }
            else
            {
                // click thru the UI based on the scenario.
                // This depends on the UX being used
                burnProcess.WaitForExit(20000);
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
            bvp.BurnMetricsData = new BurnMetricsData(bvp.BurnStartTime, bvp.BurnEndTime, 1, tempToLookIn);

            return bvp;
        }

        public virtual BurnCommonVerifiableProperties RunUninstallFromArpScenario(UiMode uiMode, UserType userType)
        {
            BurnCommonVerifiableProperties bvp = new BurnCommonVerifiableProperties();
            SetBurnCmdLineSwitchesToUse(uiMode);

            bvp.BurnStartTime = DateTime.Now;

            BurnstubLauncher burnstubLauncher = new BurnstubLauncher();
            if (userType == UserType.NormalUser)
            {
                burnstubLauncher.UseNormalUser = true;
            }
            System.Diagnostics.Process burnProcess = burnstubLauncher.StartArpProcess(Layout, Args);
            if (uiMode == UiMode.Passive ||
                uiMode == UiMode.Silent)
            {
                burnProcess.WaitForExit();
            }
            else
            {
                // click thru the UI based on the scenario.
                // This depends on the UX being used
                burnProcess.WaitForExit(20000);
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
            bvp.BurnMetricsData = new BurnMetricsData(bvp.BurnStartTime, bvp.BurnEndTime, 1, tempToLookIn);

            return bvp;
        }

        public BurnCommonVerifiableProperties RunScenario(InstallMode installMode, UiMode uiMode, UserType userType, bool waitForExit)
        {
            BurnCommonVerifiableProperties bvp = new BurnCommonVerifiableProperties();
            SetBurnCmdLineSwitchesToUse(installMode, uiMode);

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
                if (waitForExit)
                    burnProcess.WaitForExit();
            }

            return bvp;
        }

        public void InstallHelloWorldMsi(string msiPath)
        {
            string prodCode = MsiUtils.GetMSIProductCode(msiPath);

            if (! MsiUtils.IsProductInstalled(prodCode))
            {
                MsiUtils.InstallMSI(msiPath);
            }
        }

        public void UninstallHelloWorldMsi(string msiPath)
        {
            string prodCode = MsiUtils.GetMSIProductCode(msiPath);
            if (MsiUtils.IsProductInstalled(prodCode))
            {
                MsiUtils.RemoveMSI(msiPath);
            }
        }

        protected void SetBurnCmdLineSwitchesToUse(UiMode uiMode)
        {
            SetBurnCmdLineSwitchesToUse(InstallMode.install, uiMode);
        }

        protected void SetBurnCmdLineSwitchesToUse(InstallMode installMode, UiMode uiMode)
        {
            this.Args = "";
            if ((installMode == InstallMode.repair) ||
                (installMode == InstallMode.maintenanceMode))
            {
                this.Args += " /repair";
            }
            if (installMode == InstallMode.uninstall)
            {
                this.Args += " /uninstall";
            }
            if (uiMode == UiMode.Silent)
            {
                this.Args += " /q";
            }
            if (uiMode == UiMode.Passive)
            {
                this.Args += " /passive";
            }
            if (uiMode == UiMode.PassivePromptOnError)
            {
                throw new NotImplementedException("PassivePromptOnError is not implemented in Burn/BurnUX yet.  What switches would you pass to setup.exe to make that happen?");
            }

        }

        /// <summary>
        /// Kill all existing IronMan instances before continuing with test
        /// </summary>
        protected void KillAllRunningBurnLayoutProcesses()
        {
            if (this.Layout != null)
            {
                string[] exeNames = new string[] { 
                    System.IO.Path.GetFileNameWithoutExtension(this.Layout.SetupExeFilename), 
                    System.IO.Path.GetFileNameWithoutExtension(this.Layout.SetupBundleFilename) };
                foreach (string exeName in exeNames)
                {
                    for (int x = 0; x < 9; x++)
                    {
                        Process[] runningBurnProcesses = Process.GetProcessesByName(exeName);

                        if (runningBurnProcesses.Length > 0)
                        {
                            foreach (Process burnProcess in runningBurnProcesses)
                            {
                                try
                                {
                                    burnProcess.Kill();
                                }
                                catch
                                {
                                    // could throw if the process exits on its own before we call Kill.
                                    // ignore that case.
                                }
                            }
                            System.Threading.Thread.Sleep(1000);
                        }
                    }
                }
            }
        }

        public void CreateLayout()
        {
            this.Layout.GenerateLayout();
        }

        public string ConstructBurnCoreMethodCallMessage(string methodName, string variableName, int stringLen)
        {
            string message = string.Format("<BurnCoreMethod><Method Name='{0}' Variable='{1}' StringLen='{2}'/></BurnCoreMethod>"
                , methodName, variableName, stringLen.ToString());
            return message;
        }

        public string ConstructBurnCoreMethodCallMessage(string methodName, string variableName)
        {
            string message = string.Format("<BurnCoreMethod><Method Name='{0}' Variable='{1}' /></BurnCoreMethod>", methodName, variableName);
            return message;
        }

        public string ConstructBurnCoreMethodCallMessage(string methodName, string variableName, string value)
        {
            string message = string.Format("<BurnCoreMethod><Method Name='{0}' Variable='{1}' Value='{2}'/></BurnCoreMethod>"
                , methodName, variableName, value);
            return message;
        }

        public string ConstructEvalulateConditionCallMessage(string methodName, string conditionStatement)
        {
            conditionStatement = conditionStatement.Replace("<", "&lt;").Replace(">", "&gt;");
            string message = string.Format("<BurnCoreMethod><Method Name='{0}' ConditionStatement='{1}'/></BurnCoreMethod>"
                , methodName, conditionStatement);

            return message;
        }

        public void SendMessageServerPipe(string message)
        {

            using (NamedPipeServerStream pipeServer =
            new NamedPipeServerStream("BurnCoreMethodCall", PipeDirection.Out))
            {
                Console.WriteLine("NamedPipeServerStream object created.");
                // Wait for a client to connect
                Console.Write("Waiting for client connection...");
                pipeServer.WaitForConnection();
                Console.WriteLine("Client connected.");

                using (StreamWriter sw = new StreamWriter(pipeServer))
                {
                    while (true)
                    {
                        sw.AutoFlush = true;
                        sw.WriteLine(message);

                        message = "exit";
                        sw.WriteLine(message);

                        if (message == "exit")
                            break;
                    }
                }
            }
        }

        public struct BurnCoreMethodCallResult
        {
            public string variableName;
            public string variableValue;
            public string methodReturnValue;
        }

        public BurnCoreMethodCallResult GetVariableResult()
        {
            BurnCoreMethodCallResult result = new BurnCoreMethodCallResult();
            string resultFilePath = Environment.ExpandEnvironmentVariables(@"%temp%\BurnGetVariableResult.xml");

            if (File.Exists(resultFilePath))
            {
                XmlDocument xmldoc = new XmlDocument();
                xmldoc.Load(resultFilePath);

                XmlNode node = xmldoc.SelectSingleNode("./BurnCoreMethodResult/Method");
                result.variableName = node.Attributes["variable"].Value;
                result.variableValue = node.Attributes["value"].Value;
                result.methodReturnValue = node.Attributes["returnvalue"].Value;
            }
            else
            {
                Trace.WriteLine(string.Format("Result file: {0} not found.", resultFilePath));
            }

            return result;
        }

        #region common verification methods


        /// <summary>
        /// Verifies Burn will be restarted at logon
        /// </summary>
        /// <returns>true if run key is set to re-launch Burn, false otherwise</returns>
        protected bool RunKeysSetAccurately()
        {
            bool retVal = true;
            string traceCategory = "Verify RunKeysSetAccurately";

            Trace.WriteLine("Start", traceCategory);
            Trace.Indent();

            string registrationId = this.Layout.ParameterInfo.Registration.Id;
            bool foundRunKey = false;
            if (!String.IsNullOrEmpty(registrationId))
            {
                foreach (string sid in Registry.Users.GetSubKeyNames())
                {
                    try
                    {
                        RegistryKey rk = Registry.Users.OpenSubKey(sid + @"\Software\Microsoft\Windows\CurrentVersion\Run", true);
                        if (rk != null)
                        {
                            string restartBurnCmd = null;
                            restartBurnCmd = (string)rk.GetValue(registrationId);
                            if (!String.IsNullOrEmpty(restartBurnCmd))
                            {
                                foundRunKey = true;
                                Trace.WriteLine("Run key exists: " + rk.Name, traceCategory);
                                Trace.WriteLine("    With value: " + restartBurnCmd, traceCategory);
                            }
                        }
                    }
                    catch
                    {
                        // don't throw if the key couldn't be opened.
                    }
                }
            }
            retVal &= foundRunKey;

            Trace.WriteLineIf(retVal, "PASS ", traceCategory);
            Trace.WriteLineIf(!retVal, "FAIL ", traceCategory);

            Trace.Unindent();
            Trace.WriteLine("End", traceCategory);

            return retVal;
        }

        /// <summary>
        /// Verifies the Burn will not be restarted at logon
        /// </summary>
        /// <returns>true if no run key is set to re-launch Burn, false otherwise</returns>
        protected bool NoRunKeysSet()
        {
            bool retVal = true;
            string traceCategory = "Verify NoRunKeysSet";

            Trace.WriteLine("Start", traceCategory);
            Trace.Indent();

            string registrationId = this.Layout.ParameterInfo.Registration.Id;
            bool foundRunKey = false;
            if (!String.IsNullOrEmpty(registrationId))
            {
                foreach (string sid in Registry.Users.GetSubKeyNames())
                {
                    try
                    {
                        RegistryKey rk = Registry.Users.OpenSubKey(sid + @"\Software\Microsoft\Windows\CurrentVersion\Run", true);
                        if (rk != null)
                        {
                            string restartBurnCmd = null;
                            restartBurnCmd = (string)rk.GetValue(registrationId);
                            if (!String.IsNullOrEmpty(restartBurnCmd))
                            {
                                foundRunKey = true;
                                Trace.WriteLine("Error: Run key exists: " + rk.Name, traceCategory);
                                Trace.WriteLine("       With value: " + restartBurnCmd, traceCategory);
                            }
                        }
                    }
                    catch
                    {
                        // don't throw if the key couldn't be opened.
                    }
                }
            }
            retVal &= !foundRunKey;

            Trace.WriteLineIf(retVal, "PASS ", traceCategory);
            Trace.WriteLineIf(!retVal, "FAIL ", traceCategory);

            Trace.Unindent();
            Trace.WriteLine("End", traceCategory);

            return retVal;
        }

        /// <summary>
        /// Verifies the Burn process's ExitCode matches what was expected
        /// </summary>
        /// <param name="expectedExitCode"></param>
        /// <returns>true if they match, false otherwise</returns>
        protected bool ExitCodeMatchesExpectation(int expectedExitCode)
        {
            bool retVal = true;
            string traceCategory = "Verify ExitCodeMatchesExpectation";

            Trace.WriteLine("Start", traceCategory);
            Trace.Indent();

            retVal &= (this.VerifiableProperties.BurnExitCode == expectedExitCode);

            string actualAndExpectedValues = String.Format("actual: '{0}' expected: '{1}'", this.VerifiableProperties.BurnExitCode.ToString(), expectedExitCode.ToString());
            Trace.WriteLineIf(retVal, "PASS: " + actualAndExpectedValues, traceCategory);
            Trace.WriteLineIf(!retVal, "FAIL: " + actualAndExpectedValues, traceCategory);

            Trace.Unindent();
            Trace.WriteLine("End", traceCategory);

            return retVal;
        }

        /// <summary>
        /// Verifies all Burn packages in the current Layout exist in the cache
        /// </summary>
        /// <returns>true if the cache is accurate</returns>
        protected virtual bool PayloadCacheIsAccurate()
        {
            bool retVal = true;
            string traceCategory = "Verify PayloadCacheIsAccurate";

            Trace.WriteLine("Start", traceCategory);
            Trace.Indent();

            string perMachineCacheRoot = System.Environment.ExpandEnvironmentVariables(@"%ProgramData%\Apps\Cache");
            string perUserCacheRoot = System.Environment.ExpandEnvironmentVariables(@"%LOCALAPPDATA%\Apps\Cache");
            string cacheRoot = perMachineCacheRoot;

            foreach (Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnBaseItems item in this.Layout.ParameterInfo.Items.Items)
            {
                Type t = item.GetType();
                if (t == typeof(Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnInstallableItems.ExeItem) ||
                    t == typeof(Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnInstallableItems.MsiItem) ||
                    t == typeof(Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnInstallableItems.MspItem))
                {
                    if (((Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.InstallableItems.BurnInstallableItem)item).Cache == true)
                    {
                        cacheRoot = perMachineCacheRoot;
                        if (((Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.InstallableItems.BurnInstallableItem)item).PerMachine == false)
                        {
                            // admin (current user) installed a per-user package
                            cacheRoot = perUserCacheRoot;

                            if (this.UserTypeToUse == UserType.NormalUser)
                            {
                                // non admin (normal user) installed a per-user package
                                foreach (string localAppData in Microsoft.Tools.WindowsInstallerXml.Test.Utilities.UserUtilities.GetAllUserLocalAppDataPaths())
                                {
                                    if (localAppData.Contains(BurnstubLauncher.NormalUserName))
                                    {
                                        cacheRoot = Path.Combine(localAppData, "Apps\\Cache");
                                    }
                                }
                            }
                        }
                        string itemName = ((Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.InstallableItems.BurnInstallableItem)item).Name;
                        string cacheId = ((Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.InstallableItems.BurnInstallableItem)item).CacheId;
                        string itemRootPath = Path.Combine(cacheRoot, cacheId);
                        string itemFullPath = Path.Combine(itemRootPath, Path.GetFileName(itemName)); // Paths are removed from the Item name, only the filename is used.

                        if (!File.Exists(itemFullPath))
                        {
                            retVal = false;
                            Trace.WriteLine("ERROR - Missing File: " + itemFullPath, traceCategory);
                        }
                        else
                        {
                            Trace.WriteLine("Found File: " + itemFullPath, traceCategory);
                        }

                        if (null != ((Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.InstallableItems.BurnInstallableItem)item).SubFiles)
                        {
                            foreach (Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.InstallableItems.SubFile subFile in ((Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.InstallableItems.BurnInstallableItem)item).SubFiles)
                            {
                                string subFileFullPath = Path.Combine(itemRootPath, subFile.Name);
                                if (!File.Exists(subFileFullPath))
                                {
                                    retVal = false;
                                    Trace.WriteLine("ERROR - Missing File: " + subFileFullPath, traceCategory);
                                }
                                else
                                {
                                    Trace.WriteLine("Found File: " + subFileFullPath, traceCategory);
                                }
                            }
                        }
                    }
                }
            }

            Trace.WriteLineIf(retVal, "PASS", traceCategory);
            Trace.WriteLineIf(!retVal, "FAIL", traceCategory);

            Trace.Unindent();
            Trace.WriteLine("End", traceCategory);

            return retVal;
        }

                
        /// <summary>
        /// Verifies a Burn bundle does not exist in the cache
        /// </summary>
        /// <returns>true if the bundle does not exist in the cache</returns>
        protected virtual bool BundleCacheNotExist(Microsoft.Tools.WindowsInstallerXml.Test.Burn.LayoutManager.LayoutManager myLayout)
        {
            bool retVal = !BundleCacheIsPresent(myLayout, "Verify BundleCacheNotExist", false);
            return retVal;
        }

        /// <summary>
        /// Verifies a Burn bundle exists in the cache
        /// </summary>
        /// <returns>true if the bundle exists in the cache</returns>
        protected virtual bool BundleCacheExists(Microsoft.Tools.WindowsInstallerXml.Test.Burn.LayoutManager.LayoutManager myLayout)
        {
            bool retVal = BundleCacheIsPresent(myLayout, "Verify BundleCacheExists", true);
            return retVal;
        }
        
        /// <summary>
        /// Verifies a Burn bundle exists in the cache
        /// </summary>
        /// <returns>true if the bundle exists in the cache</returns>
        protected virtual bool BundleCacheIsPresent(Microsoft.Tools.WindowsInstallerXml.Test.Burn.LayoutManager.LayoutManager myLayout, 
            string traceCategory,
            bool expectedPresent)
        {
            bool retVal = true;

            Trace.WriteLine("Start", traceCategory);
            Trace.Indent();

            string perMachineCacheRoot = System.Environment.ExpandEnvironmentVariables(@"%ProgramData%\Apps\Cache");
            string perUserCacheRoot = System.Environment.ExpandEnvironmentVariables(@"%LOCALAPPDATA%\Apps\Cache");
            string cacheRoot = perMachineCacheRoot;

            if (!myLayout.ParameterInfo.Registration.PerMachine.Contains("yes"))
            {
                cacheRoot = perUserCacheRoot;
            }

            if (this.UserTypeToUse == UserType.NormalUser)
            {
                // non admin (normal user) installed a per-user package
                foreach (string localAppData in Microsoft.Tools.WindowsInstallerXml.Test.Utilities.UserUtilities.GetAllUserLocalAppDataPaths())
                {
                    if (localAppData.Contains(BurnstubLauncher.NormalUserName))
                    {
                        cacheRoot = Path.Combine(localAppData, "Apps\\Cache");
                    }
                }
            }

            string itemName = myLayout.SetupBundleFilename;
            string cacheId = myLayout.ParameterInfo.Registration.Id;
            string itemRootPath = Path.Combine(cacheRoot, cacheId);
            string itemFullPath = Path.Combine(itemRootPath, Path.GetFileName(itemName)); // Paths are removed from the Item name, only the filename is used.

            // BUGBUG TODO, also verify that external UX Resource file are cached.

            if (!File.Exists(itemFullPath))
            {
                retVal = false;
                Trace.WriteLine("Missing File: " + itemFullPath, traceCategory);
            }
            else
            {
                Trace.WriteLine("Found File: " + itemFullPath, traceCategory);
            }

            Trace.WriteLineIf(retVal == expectedPresent, "PASS", traceCategory);
            Trace.WriteLineIf(retVal != expectedPresent, "FAIL", traceCategory);

            Trace.Unindent();
            Trace.WriteLine("End", traceCategory);

            return retVal;
        }

        /// <summary>
        /// Verifies an ARP entry exists for the current Layout
        /// </summary>
        /// <returns>true if the ARP is accurate</returns>
        protected bool ArpEntryIsAccurate()
        {
            return ArpEntryIsAccurate(this.Layout);
        }

        /// <summary>
        /// Verifies an ARP entry exists for the specified Layout
        /// </summary>
        /// <param name="myLayout">layout you want to verify</param>
        /// <returns>true if the ARP is accurate</returns>
        protected bool ArpEntryIsAccurate(Microsoft.Tools.WindowsInstallerXml.Test.Burn.LayoutManager.LayoutManager myLayout)
        {
            bool retVal = false;
            string traceCategory = "Verify ArpEntryIsAccurate";

            Trace.WriteLine("Start", traceCategory);
            Trace.Indent();

            string RegistrationId;
            string ArpDisplayName;
            List<string> UpdateIds = new List<string>();
            try
            {
                RegistrationId = myLayout.ParameterInfo.Registration.Id;
                ArpDisplayName = myLayout.ParameterInfo.Registration.Arp.DisplayName;
                foreach (Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.BurnManifestOM.Registration.RegistrationElement.UpdateElement updateElement in myLayout.BurnManifest.Registration.UpdateElements)
                {
                    UpdateIds.Add(updateElement.BundleId);
                }
            }
            catch
            {
                // if either of these aren't defined, then don't bother verifying ARP entries
                RegistrationId = null;
                ArpDisplayName = null;
            }

            if (String.IsNullOrEmpty(RegistrationId))
            {
                Trace.WriteLine("No ARP defined for this bundle.  Nothing to verify.", traceCategory);
                retVal = true;
            }
            else
            {
                string[] uninstallSubKeys = new string[] { 
                                             @"SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\", 
                                             @"SOFTWARE\Wow6432Node\Microsoft\Windows\CurrentVersion\Uninstall\" };
                bool foundArpEntry = false;
                if (myLayout.ParameterInfo.Registration.PerMachine == "yes")
                {
                    foreach (string uninstallSubKey in uninstallSubKeys)
                    {
                        RegistryKey rkUninstall = Registry.LocalMachine.OpenSubKey(uninstallSubKey, true);
                        if (null != rkUninstall)
                        {
                            if (UpdateIds.Count == 0) // This is a normal bundle
                            {
                                RegistryKey rkRegistrationId = Registry.LocalMachine.OpenSubKey(uninstallSubKey + RegistrationId, true);
                                if (null != rkRegistrationId)
                                {
                                    foundArpEntry = true;
                                    Trace.WriteLine("Found ARP entry for " + ArpDisplayName, traceCategory);
                                    // TODO verify the display name matches ArpDisplayName
                                }
                            }
                            else // This is a bundle that Updates other bundles.  It can have multiple ARP entries
                            {
                                foreach (string updateId in UpdateIds)
                                {
                                    RegistryKey rkTargetRegistrationId = Registry.LocalMachine.OpenSubKey(uninstallSubKey + updateId, true);
                                    if (null != rkTargetRegistrationId)
                                    {
                                        Trace.WriteLine("Found Target ARP entry for " + updateId, traceCategory);
                                        RegistryKey rkUpdateRegistrationId = Registry.LocalMachine.OpenSubKey(uninstallSubKey + RegistrationId + "_" + updateId, true);
                                        if (null != rkUpdateRegistrationId)
                                        {
                                            foundArpEntry = true;
                                            Trace.WriteLine("Found ARP entry for " + RegistrationId + "_" + updateId, traceCategory);
                                            // TODO verify the Parent values
                                        }
                                    }
                                }
                            }
                        }
                    }
                    retVal = foundArpEntry;
                }
                else
                {
                    // per-user ARP entry
                    // For example:
                    // HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Uninstall\{b12dfdfb-9ef3-471e-b2a0-b960cc00106c}
                    // HKEY_USERS\S-1-5-21-2127521184-1604012920-1887927527-1450143\Software\Microsoft\Windows\CurrentVersion\Uninstall\{b12dfdfb-9ef3-471e-b2a0-b960cc00106c}
                    foreach (string sid in Registry.Users.GetSubKeyNames())
                    {
                        string uninstallSubKey = sid + @"\Software\Microsoft\Windows\CurrentVersion\Uninstall\";
                        RegistryKey rkUserUninstall = Registry.Users.OpenSubKey(uninstallSubKey, true);
                        if (null != rkUserUninstall)
                        {
                            if (UpdateIds.Count == 0) // This is a normal bundle
                            {
                                RegistryKey rkRegistrationId = Registry.Users.OpenSubKey(uninstallSubKey + RegistrationId, true);
                                if (null != rkRegistrationId)
                                {
                                    foundArpEntry = true;
                                    Trace.WriteLine("Found ARP entry for " + ArpDisplayName, traceCategory);
                                    // TODO verify the display name matches ArpDisplayName
                                }
                            }
                            else // This is a bundle that Updates other bundles.  It can have multiple ARP entries
                            {
                                foreach (string updateId in UpdateIds)
                                {
                                    RegistryKey rkTargetRegistrationId = Registry.Users.OpenSubKey(uninstallSubKey + updateId, true);
                                    if (null != rkTargetRegistrationId)
                                    {
                                        Trace.WriteLine("Found Target ARP entry for " + updateId, traceCategory);
                                        RegistryKey rkUpdateRegistrationId = Registry.Users.OpenSubKey(uninstallSubKey + RegistrationId + "_" + updateId, true);
                                        if (null != rkUpdateRegistrationId)
                                        {
                                            foundArpEntry = true;
                                            Trace.WriteLine("Found ARP entry for " + RegistrationId + "_" + updateId, traceCategory);
                                            // TODO verify the Parent values
                                        }
                                    }
                                }
                            }
                        }
                    }
                    retVal = foundArpEntry;
                }

            }


            Trace.WriteLineIf(retVal, "PASS", traceCategory);
            Trace.WriteLineIf(!retVal, "FAIL", traceCategory);

            Trace.Unindent();
            Trace.WriteLine("End", traceCategory);

            return retVal;
        }

        /// <summary>
        /// Verifies an ARP entry does not exist for the current Layout
        /// </summary>
        /// <returns>true if the ARP is accurate</returns>
        protected bool ArpEntryDoesNotExist()
        {
            return ArpEntryDoesNotExist(this.Layout);
        }

        /// <summary>
        /// Verifies an ARP entry does not exist for the specified Layout
        /// </summary>
        /// <param name="myLayout">layout you want to verify</param>
        /// <returns>true if the ARP is accurate</returns>
        protected bool ArpEntryDoesNotExist(Microsoft.Tools.WindowsInstallerXml.Test.Burn.LayoutManager.LayoutManager myLayout)
        {
            bool retVal = false;
            string traceCategory = "Verify ArpEntryDoesNotExist";

            Trace.WriteLine("Start", traceCategory);
            Trace.Indent();

            string RegistrationId;
            List<string> UpdateIds = new List<string>();

            try
            {
                RegistrationId = myLayout.ParameterInfo.Registration.Id;
                foreach (Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.BurnManifestOM.Registration.RegistrationElement.UpdateElement updateElement in myLayout.BurnManifest.Registration.UpdateElements)
                {
                    UpdateIds.Add(updateElement.BundleId);
                }
            }
            catch
            {
                // if this isn't defined, then don't bother verifying ARP entries
                RegistrationId = null;
            }

            if (String.IsNullOrEmpty(RegistrationId))
            {
                Trace.WriteLine("No ARP defined for this bundle.  Nothing to verify.", traceCategory);
                retVal = true;
            }
            else
            {
                string[] uninstallSubKeys = new string[] { 
                                             @"SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\", 
                                             @"SOFTWARE\Wow6432Node\Microsoft\Windows\CurrentVersion\Uninstall\" };
                bool foundArpEntry = false;
                if (myLayout.ParameterInfo.Registration.PerMachine == "yes")
                {
                    foreach (string uninstallSubKey in uninstallSubKeys)
                    {
                        RegistryKey rkUninstall = Registry.LocalMachine.OpenSubKey(uninstallSubKey, true);
                        if (null != rkUninstall)
                        {
                            if (UpdateIds.Count == 0) // This is a normal bundle
                            {
                                RegistryKey rkRegistrationId = Registry.LocalMachine.OpenSubKey(uninstallSubKey + RegistrationId, true);
                                if (null != rkRegistrationId)
                                {
                                    foundArpEntry = true;
                                    Trace.WriteLine("Found ARP entry for " + RegistrationId, traceCategory);
                                }
                            }
                            else // This is a bundle that Updates other bundles.  It can have multiple ARP entries
                            {
                                foreach (string updateId in UpdateIds)
                                {
                                    RegistryKey rkTargetRegistrationId = Registry.LocalMachine.OpenSubKey(uninstallSubKey + updateId, true);
                                    if (null != rkTargetRegistrationId)
                                    {
                                        Trace.WriteLine("Found Target ARP entry for " + updateId, traceCategory);
                                        RegistryKey rkUpdateRegistrationId = Registry.LocalMachine.OpenSubKey(uninstallSubKey + RegistrationId + "_" + updateId, true);
                                        if (null != rkUpdateRegistrationId)
                                        {
                                            foundArpEntry = true;
                                            Trace.WriteLine("Found ARP entry for " + RegistrationId + "_" + updateId, traceCategory);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                else
                {
                    // per-user ARP entry
                    // For example:
                    // HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Uninstall\{b12dfdfb-9ef3-471e-b2a0-b960cc00106c}
                    // HKEY_USERS\S-1-5-21-2127521184-1604012920-1887927527-1450143\Software\Microsoft\Windows\CurrentVersion\Uninstall\{b12dfdfb-9ef3-471e-b2a0-b960cc00106c}
                    foreach (string sid in Registry.Users.GetSubKeyNames())
                    {
                        string uninstallSubKey = sid + @"\Software\Microsoft\Windows\CurrentVersion\Uninstall\";
                        RegistryKey rkUserUninstall = Registry.Users.OpenSubKey(uninstallSubKey, true);
                        if (null != rkUserUninstall)
                        {
                            if (UpdateIds.Count == 0) // This is a normal bundle
                            {
                                RegistryKey rkRegistrationId = Registry.Users.OpenSubKey(uninstallSubKey + RegistrationId, true);
                                if (null != rkRegistrationId)
                                {
                                    foundArpEntry = true;
                                    Trace.WriteLine("Found ARP entry for " + RegistrationId, traceCategory);
                                }
                            }
                            else // This is a bundle that Updates other bundles.  It can have multiple ARP entries
                            {
                                foreach (string updateId in UpdateIds)
                                {
                                    RegistryKey rkTargetRegistrationId = Registry.Users.OpenSubKey(uninstallSubKey + updateId, true);
                                    if (null != rkTargetRegistrationId)
                                    {
                                        Trace.WriteLine("Found Target ARP entry for " + updateId, traceCategory);
                                        RegistryKey rkUpdateRegistrationId = Registry.Users.OpenSubKey(uninstallSubKey + RegistrationId + "_" + updateId, true);
                                        if (null != rkUpdateRegistrationId)
                                        {
                                            foundArpEntry = true;
                                            Trace.WriteLine("Found ARP entry for " + RegistrationId + "_" + updateId, traceCategory);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                retVal = !foundArpEntry;
            }


            Trace.WriteLineIf(retVal, "PASS", traceCategory);
            Trace.WriteLineIf(!retVal, "FAIL", traceCategory);

            Trace.Unindent();
            Trace.WriteLine("End", traceCategory);

            return retVal;
        }

        #region Metrics validation methods
        /// <summary>
        /// Verifies Metrics datapoints that are always logged regardless of the scenario.
        /// </summary>
        /// <returns>true if Metrics data is accurate, false otherwise</returns>
        protected virtual bool MetricsDataIsAccurate()
        {
            bool metricsDataIsAccurate = true;
            string traceCategory = "Verify MetricsDataIsAccurate";

            Trace.WriteLine("Start", traceCategory);
            Trace.Indent();

            // BUGBUG, nothing to verify yet until we get Metrics working again.  Need a metrics uploader.

            //// BUGBUG AppId should not be 23, which is Ironman.  But for now, we'll just use that.
            //bool bAppIdIsValid = (this.VerifiableProperties.BurnMetricsData.AppId.Value == "23");
            //metricsDataIsAccurate &= bAppIdIsValid;
            //Trace.WriteLineIf(!bAppIdIsValid, "BurnMetricsData.AppId != 23", traceCategory);

            //bool bPackageNameIsValid = (this.VerifiableProperties.BurnMetricsData.PackageName.Value == this.Layout.ParameterInfo.UI.Name);
            //metricsDataIsAccurate &= bPackageNameIsValid;
            //Trace.WriteLineIf(!bPackageNameIsValid, String.Format("BurnMetricsData.PackageName is not accurate. Actual: '{0}' Expected: '{1}'", this.VerifiableProperties.BurnMetricsData.PackageName.Value, this.Layout.ParameterInfo.UI.Name), traceCategory);

            //bool bAppVerIsValid = (this.VerifiableProperties.BurnMetricsData.ApplicationVersion.Value == GetBurnVersionInfoString());
            //metricsDataIsAccurate &= bAppVerIsValid;
            //Trace.WriteLineIf(!bAppVerIsValid, String.Format("BurnMetricsData.ApplicationVersion is not accurate. Actual: '{0}' Expected: '{1}'", this.VerifiableProperties.BurnMetricsData.ApplicationVersion.Value, GetBurnVersionInfoString()), traceCategory);

            //bool bPackageVerIsValid = (this.VerifiableProperties.BurnMetricsData.PackageVersion.Value == this.Layout.ParameterInfo.UI.Version);
            //metricsDataIsAccurate &= bPackageVerIsValid;
            //Trace.WriteLineIf(!bPackageVerIsValid, String.Format("BurnMetricsData.PackageVersion is not accurate. Actual: '{0}' Expected: '{1}'", this.VerifiableProperties.BurnMetricsData.PackageVersion.Value, this.Layout.ParameterInfo.UI.Version), traceCategory);

            //bool bOsFullVerIsValid = (this.VerifiableProperties.BurnMetricsData.OsFullVersion.Value == MyOsFullVersion());
            //metricsDataIsAccurate &= bOsFullVerIsValid;
            //Trace.WriteLineIf(!bOsFullVerIsValid, String.Format("BurnMetricsData.OsFullVersion is not accurate. Actual: '{0}' Expected: '{1}'", this.VerifiableProperties.BurnMetricsData.OsFullVersion.Value, MyOsFullVersion()), traceCategory);

            //bool bOsIsValid = (this.VerifiableProperties.BurnMetricsData.OS.Value == MyOS());
            //metricsDataIsAccurate &= bOsIsValid;
            //Trace.WriteLineIf(!bOsIsValid, String.Format("BurnMetricsData.OS is not accurate. Actual: '{0}' Expected: '{1}'", this.VerifiableProperties.BurnMetricsData.OS.Value, MyOS()), traceCategory);


            //UInt32 osBitmask;
            //UInt32 result;
            //try
            //{
            //    osBitmask = UInt32.Parse(this.VerifiableProperties.BurnMetricsData.OS.Value);
            //    result = (osBitmask & 2);
            //    if (result == 2) // the 2nd bit was set.  This means we are in OS compatibility mode
            //    {
            //        metricsDataIsAccurate = false;
            //        Trace.WriteLine("BurnMetricsData.OS is wrong. Expected OS bitmask to have the 2nd bit != 1 meaning it is not in OS compatibility mode.  It was set to 1.", traceCategory);
            //    }
            //}
            //catch
            //{
            //    metricsDataIsAccurate = false;
            //    Trace.WriteLine("BurnMetricsData.OS is wrong. Expected OS bitmask to have the 2nd bit != 1 meaning it is not in OS compatibility mode.  It was set to 1.", traceCategory);
            //}

            //bool bOsLocaleIsValid = !String.IsNullOrEmpty(this.VerifiableProperties.BurnMetricsData.OSLocale.Value);
            //metricsDataIsAccurate &= bOsLocaleIsValid;
            //Trace.WriteLineIf(!bOsLocaleIsValid, "BurnMetricsData.OSLocale is not set", traceCategory);

            //bool bOperationUiIsValid = !String.IsNullOrEmpty(this.VerifiableProperties.BurnMetricsData.OperationUI.Value);
            //metricsDataIsAccurate &= bOperationUiIsValid;
            //Trace.WriteLineIf(!bOperationUiIsValid, "BurnMetricsData.OperationUI is not set", traceCategory);

            //bool bDisplayLcidIsValid = !String.IsNullOrEmpty(this.VerifiableProperties.BurnMetricsData.DisplayLCID.Value);
            //metricsDataIsAccurate &= bDisplayLcidIsValid;
            //Trace.WriteLineIf(!bDisplayLcidIsValid, "BurnMetricsData.DisplayLCID is not set", traceCategory);

            //bool bIsAdminIsValid = !String.IsNullOrEmpty(this.VerifiableProperties.BurnMetricsData.IsAdmin.Value);
            //metricsDataIsAccurate &= bIsAdminIsValid;
            //Trace.WriteLineIf(!bIsAdminIsValid, "BurnMetricsData.IsAdmin is not set", traceCategory);

            //bool bProcFamilyIsValid = (this.VerifiableProperties.BurnMetricsData.ProcFamily.Value == MyProcesorType());
            //metricsDataIsAccurate &= bProcFamilyIsValid;
            //Trace.WriteLineIf(!bProcFamilyIsValid, String.Format("BurnMetricsData.ProcFamily is not accurate. Actual: '{0}' Expected: '{1}'", this.VerifiableProperties.BurnMetricsData.ProcFamily.Value, MyProcesorType()), traceCategory);

            //bool bProcFreqIsValid = (this.VerifiableProperties.BurnMetricsData.ProcFrequency.Value == MyProcesorFrequency());
            //metricsDataIsAccurate &= bProcFreqIsValid;
            //Trace.WriteLineIf(!bProcFreqIsValid, String.Format("BurnMetricsData.ProcFrequency is not accurate. Actual: '{0}' Expected: '{1}'", this.VerifiableProperties.BurnMetricsData.ProcFrequency.Value, MyProcesorFrequency()), traceCategory);

            //bool bCpuCountIsValid = (this.VerifiableProperties.BurnMetricsData.CpuCount.Value == MyNumberOfProcessors());
            //metricsDataIsAccurate &= bCpuCountIsValid;
            //Trace.WriteLineIf(!bCpuCountIsValid, String.Format("BurnMetricsData.CpuCount is not accurate. Actual: '{0}' Expected: '{1}'", this.VerifiableProperties.BurnMetricsData.CpuCount.Value, MyNumberOfProcessors()), traceCategory);

            //bool bMemoryIsValid = (this.VerifiableProperties.BurnMetricsData.Memory.Value == MyMemory());
            //metricsDataIsAccurate &= bMemoryIsValid;
            //Trace.WriteLineIf(!bMemoryIsValid, String.Format("BurnMetricsData.Memory is not accurate. Actual: '{0}' Expected: '{1}'", this.VerifiableProperties.BurnMetricsData.Memory.Value, MyMemory()), traceCategory);

            //bool bWindowsInstallerVerIsValid = (this.VerifiableProperties.BurnMetricsData.WindowsInstallerVersion.Value == MyWindowsInstallerVersion());
            //metricsDataIsAccurate &= bWindowsInstallerVerIsValid;
            //Trace.WriteLineIf(!bWindowsInstallerVerIsValid, String.Format("BurnMetricsData.WindowsInstallerVersion is not accurate. Actual: '{0}' Expected: '{1}'", this.VerifiableProperties.BurnMetricsData.WindowsInstallerVersion.Value, MyWindowsInstallerVersion()), traceCategory);

            //bool bSkuIsValid = !String.IsNullOrEmpty(this.VerifiableProperties.BurnMetricsData.Sku.Value);
            //metricsDataIsAccurate &= bSkuIsValid;
            //Trace.WriteLineIf(!bSkuIsValid, "BurnMetricsData.Sku is not set", traceCategory);

            //// The time at which Burn calculates free disk space
            //// and the time the test calculates it can come up with 
            //// slightly different values since other processes will be performing IO.
            //// so do a comparison for "nearly equal" (i.e. within 10%)
            //try
            //{
            //    float sqmValue = float.Parse(this.VerifiableProperties.BurnMetricsData.SystemFreeDiskSpace.Value);
            //    float currentValue = float.Parse(MySystemFreeDiskSpace());
            //    float xPercent;
            //    bool nearlyEqual = false;
            //    if (sqmValue < currentValue)
            //    {
            //        xPercent = sqmValue / currentValue;
            //    }
            //    else
            //    {
            //        xPercent = currentValue / sqmValue;
            //    }
            //    nearlyEqual = (xPercent > 0.9);  // within 10% (smaller value is > 90% of the larger value)
            //    if (!nearlyEqual)
            //    {
            //        metricsDataIsAccurate = false;
            //        Trace.WriteLine("BurnMetricsData.SystemFreeDiskSpace is not set", traceCategory);
            //        Trace.WriteLine(String.Format("SystemFreeDiskSpace datapoint unable to be verified.  Actual: '{0}'  Expected: '{1}'", this.VerifiableProperties.BurnMetricsData.SystemFreeDiskSpace.Value, MySystemFreeDiskSpace()));
            //    }
            //}
            //catch
            //{
            //    metricsDataIsAccurate = false;
            //    Trace.WriteLine(String.Format("SystemFreeDiskSpace datapoint unable to be verified.  Actual: '{0}'  Expected: '{1}'", this.VerifiableProperties.BurnMetricsData.SystemFreeDiskSpace.Value, MySystemFreeDiskSpace()));
            //}

            //bool bOperationRequestedIsValid = !String.IsNullOrEmpty(this.VerifiableProperties.BurnMetricsData.OperationRequested.Value);
            //metricsDataIsAccurate &= bOperationRequestedIsValid;
            //Trace.WriteLineIf(!bOperationRequestedIsValid, "BurnMetricsData.OperationRequested is not set", traceCategory);

            Trace.WriteLineIf(metricsDataIsAccurate, "PASS ", traceCategory);
            Trace.WriteLineIf(!metricsDataIsAccurate, "FAIL ", traceCategory);

            Trace.Unindent();
            Trace.WriteLine("End", traceCategory);

            return metricsDataIsAccurate;
        }

        /// <summary>
        /// Gets version information from the Burn binary file (setup.exe).  
        /// The format of this string matches what is logged in Burn Metrics data.
        /// </summary>
        /// <returns></returns>
        protected string GetBurnVersionInfoString()
        {
            string retVal = "";
            string BurnExe = System.IO.Path.Combine(this.Layout.LayoutFolder, this.Layout.SetupExeFilename);
            FileVersionInfo fvi = FileVersionInfo.GetVersionInfo(BurnExe);
            // can't use fvi.FileVersion because it contains a value like this "9.0.20716.00 built by: XXXXXX"
            // can't use fvi.ProductVersion because it contains a value like this "9.0.20716.00"
            // so we build our own string from the values used by Burn when it records Metrics data
            retVal = String.Format("{0}.{1}.{2}.{3}", fvi.FileMajorPart, fvi.FileMinorPart, fvi.FileBuildPart, fvi.FilePrivatePart);

            return retVal;
        }

        /// <summary>
        /// Gets OS version number and formats it as a string.
        /// The format of this string matches what is logged in Burn Metrics data.
        /// </summary>
        /// <returns></returns>
        protected static string MyOsFullVersion()
        {
            string osFullVersion = String.Format("{0}.{1}.{2}",
                System.Environment.OSVersion.Version.Major,
                System.Environment.OSVersion.Version.Minor,
                System.Environment.OSVersion.Version.Build);

            return osFullVersion;
        }


        /// <summary>
        /// OSVERSIONINFOEX structure returned by GetVersionEx()
        /// </summary>
        [StructLayout(LayoutKind.Sequential)]
        public struct OSVERSIONINFOEX
        {
            public int dwOSVersionInfoSize;
            public int dwMajorVersion;
            public int dwMinorVersion;
            public int dwBuildNumber;
            public int dwPlatformId;
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 128)]
            public string szCSDVersion;
            public short wServicePackMajor;
            public short wServicePackMinor;
            public short wSuiteMask;
            public byte wProductType;
            public byte wReserved;
        }

        [DllImport("kernel32.Dll", SetLastError = true)]
        public static extern short GetVersionEx(ref OSVERSIONINFOEX o);

        /// <summary>
        /// Gets the OSVERSIONINFOEX data
        /// </summary>
        /// <returns>Abbreviated OS Name</returns>
        static public OSVERSIONINFOEX GetOSVERSIONINFOEX()
        {
            OSVERSIONINFOEX os = new OSVERSIONINFOEX();
            os.dwOSVersionInfoSize = Marshal.SizeOf(typeof(OSVERSIONINFOEX));

            if (GetVersionEx(ref os) == 0)
            {
                throw new Exception("Failed GetVersionEx()");
            }

            return os;
        }

        [StructLayout(LayoutKind.Sequential)]
        public struct SYSTEM_INFO
        {
            public short wProcessorArchitecture;
            public short wReserved;
            public uint dwPageSize;
            public uint lpMinimumApplicationAddress;
            public uint lpMaximumApplicationAddress;
            public uint dwActiveProcessorMask;
            public uint dwNumberOfProcessors;
            public uint dwProcessorType;
            public uint dwAllocationGranularity;
            public uint dwProcessorLevel;
            public uint dwProcessorRevision;
        }

        [DllImport("kernel32")]
        static extern void GetNativeSystemInfo(ref SYSTEM_INFO pSI);

        /// <summary>
        /// Gets the SYSTEM_INFO data.
        /// </summary>
        /// <returns>OS Architecture</returns>
        static public SYSTEM_INFO GetSYSTEM_INFO()
        {
            SYSTEM_INFO si = new SYSTEM_INFO();
            GetNativeSystemInfo(ref si);

            return si;
        }


        /// <summary>
        /// Gets a string containing an int representation of the OSVersion bitmap
        /// </summary>
        /// <returns></returns>
        protected static string MyOS()
        {
            //   100667792 = integer representation
            //        110 00000000 0001 0001 1001 000 0  = binary representation of int above (with breaks between each piece of data in the bitmap)
            //  210987654 32109876 5432 1098 7654 321 0  = index positions in 32 bit binary number above
            //          6        0    1    1    9   0 0  = int values for each field in the bitmap

            //BITMAP: OSVersion (from the spec)
            //31:24	OSMajorVersion	dwMajorVersion from OSVERSIONEX
            //23:16	OSMinorversion	dwMinorVersion from OSVERSIONEX
            //15:12	OSProductType	wProductType from OSVERSIONEX
            //11:8	OSServicePackMajor	wServicePackMajor from OSVERSIONEX
            //7:4	OSProcessorArchitecture	wProcessorArchitecture from SYSTEM_INFO
            //3:1	<reserved>	Set =0
            //0:0	OSPreRelease	"Set to 1 if this registry entry is present and is non-zero:
            //HKLM\SYSTEM\CurrentControlSet\Control\Windows\CSDReleaseType (Dword)"

            string bitmap;
            OSVERSIONINFOEX osVerInfo = GetOSVERSIONINFOEX();
            SYSTEM_INFO sysInfo = GetSYSTEM_INFO();

            UInt32 osMajorVersion = (UInt32)System.Environment.OSVersion.Version.Major; //6
            UInt32 osMinorVersion = (UInt32)System.Environment.OSVersion.Version.Minor; //0
            UInt32 osProductType = (UInt32)osVerInfo.wProductType; // 1
            UInt32 osServicePackMajor = (UInt32)osVerInfo.wServicePackMajor; // 1
            UInt32 osProcessorArchitecture = (UInt32)sysInfo.wProcessorArchitecture; // 9
            UInt32 reserved = 0;
            UInt32 osPreRelease = 0;

            string preRelease = ReadRegistry(@"HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Windows", "CSDReleaseType");
            if (!String.IsNullOrEmpty(preRelease) && (preRelease != "0"))
            {
                osPreRelease = 1;
            }

            UInt32 x;
            x = (UInt32)Math.Pow(2, 24);
            UInt32 value = 0;
            // shift each field into its appropriate position in the 'value' bitmap
            value += (osMajorVersion * (UInt32)Math.Pow(2, 24));
            value += (osMinorVersion * (UInt32)Math.Pow(2, 16));
            value += (osProductType * (UInt32)Math.Pow(2, 12));
            value += (osServicePackMajor * (UInt32)Math.Pow(2, 8));
            value += (osProcessorArchitecture * (UInt32)Math.Pow(2, 4));
            value += (reserved * (UInt32)Math.Pow(2, 1));
            value += (osPreRelease);

            bitmap = String.Format("{0}", value);

            return bitmap;
        }

        public static string ReadRegistry(string regHive, string strKeyName)
        {
            string regValue = null;
            try
            {
                Object regObj = Microsoft.Win32.Registry.GetValue(regHive, strKeyName, null);
                if (regObj != null)
                    regValue = regObj.ToString();
            }
            catch (Exception)
            {
                regValue = null;
            }
            return regValue;
        }

        public static string MyProcesorType()
        {
            SYSTEM_INFO sysInfo = GetSYSTEM_INFO();
            string retVal = sysInfo.dwProcessorType.ToString();
            return retVal;
        }


        public static string MyNumberOfProcessors()
        {
            SYSTEM_INFO sysInfo = GetSYSTEM_INFO();
            string retVal = sysInfo.dwNumberOfProcessors.ToString();
            return retVal;
        }

        public static string MyProcesorFrequency()
        {
            string retVal = ReadRegistry(@"HKEY_LOCAL_MACHINE\HARDWARE\DESCRIPTION\System\CentralProcessor\0", "~MHZ");
            return retVal;
        }

        [StructLayout(LayoutKind.Sequential)]
        public struct MEMORYSTATUSEX
        {
            public UInt32 dwLength;
            public UInt32 dwMemoryLoad;
            public UInt64 ullTotalPhys;
            public UInt64 ullAvailPhys;
            public UInt64 ullTotalPageFile;
            public UInt64 ullAvailPageFile;
            public UInt64 ullTotalVirtual;
            public UInt64 ullAvailVirtual;
            public UInt64 ullAvailExtendedVirtual;
        }

        [DllImport("kernel32.dll", SetLastError = true)]
        public static extern bool GlobalMemoryStatusEx(ref MEMORYSTATUSEX lpBuffer);

        public static MEMORYSTATUSEX GetMEMORYSTATUSEX()
        {
            MEMORYSTATUSEX memStatusEx = new MEMORYSTATUSEX();
            memStatusEx.dwLength = (UInt32)Marshal.SizeOf(memStatusEx);

            GlobalMemoryStatusEx(ref memStatusEx);

            return memStatusEx;
        }

        public static string MyMemory()
        {
            // use MEMORYSTATUSEX not MEMORYSTATUS to get accurate values on machines with > 4gb memory
            MEMORYSTATUSEX ms = GetMEMORYSTATUSEX();
            // convert Total Physical bytes to megabytes
            string retVal = (ms.ullTotalPhys / 1024 / 1024).ToString();
            return retVal;
        }

        public static string MyWindowsInstallerVersion()
        {
            FileVersionInfo myFileVersionInfo = null;
            string retVal = "-1";

            try
            {
                myFileVersionInfo = FileVersionInfo.GetVersionInfo(System.Environment.ExpandEnvironmentVariables("%windir%\\System32\\msi.dll"));
                retVal = myFileVersionInfo.FileVersion;
            }
            catch
            {
            }

            return retVal;
        }

        public static string MySystemFreeDiskSpace()
        {
            string driveLetter = System.Environment.ExpandEnvironmentVariables("%SystemDrive%");
            driveLetter = driveLetter.Replace(":", "");
            System.IO.DriveInfo di = new System.IO.DriveInfo(driveLetter);
            // convert bytes to megabytes
            string retVal = (di.AvailableFreeSpace / 1024 / 1024).ToString();
            return retVal;
        }

        /// <summary>
        /// Processor architecture constants used in SYSTEM_INFO struct
        /// </summary>
        private const int PROCESSOR_ARCHITECTURE_INTEL = 0;
        private const int PROCESSOR_ARCHITECTURE_MIPS = 1;
        private const int PROCESSOR_ARCHITECTURE_ALPHA = 2;
        private const int PROCESSOR_ARCHITECTURE_PPC = 3;
        private const int PROCESSOR_ARCHITECTURE_SHX = 4;
        private const int PROCESSOR_ARCHITECTURE_ARM = 5;
        private const int PROCESSOR_ARCHITECTURE_IA64 = 6;
        private const int PROCESSOR_ARCHITECTURE_ALPHA64 = 7;
        private const int PROCESSOR_ARCHITECTURE_MSIL = 8;
        private const int PROCESSOR_ARCHITECTURE_AMD64 = 9;
        private const int PROCESSOR_ARCHITECTURE_IA32_ON_WIN64 = 10;

        /// <summary>
        /// Gets the OS Architecture that is used in HotIron Watson bucket #7.
        /// </summary>
        /// <returns>OS Architecture</returns>
        static public string GetCPUArchitecture()
        {
            string CPUArch = "";

            SYSTEM_INFO si = new SYSTEM_INFO();
            GetNativeSystemInfo(ref si);

            if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
            {
                CPUArch = "x64";
            }
            else if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64)
            {
                CPUArch = "IA64";
            }
            else if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL)
            {
                CPUArch = "x86";
            }

            return CPUArch;
        }

        public static string MyCpuArchitecture()
        {
            if (GetCPUArchitecture().ToLower() == "x64") return "9";
            if (GetCPUArchitecture().ToLower() == "ia64") return "6";
            if (GetCPUArchitecture().ToLower() == "x86") return "0";
            return "";
        }

        #endregion

        #endregion
    }

    /// <summary>
    /// Container of properties recorded during a Burnstub scenario
    /// </summary>
    public class BurnCommonVerifiableProperties
    {
        private int m_BurnExitCode;
        public int BurnExitCode
        {
            get
            {
                return m_BurnExitCode;
            }
            set
            {
                m_BurnExitCode = value;
            }
        }

        // BUGBUG TODO: There are currently 2 DHTML logs created, 1 for the non-elevated process and 1 for the elevated process.
        // we should store both of them.  There is no easy way to distinguish them other than timestamps.
        // Not sure if this is will continue or if they will be consolidated into 1 log.
        //
        // BUGBUG TODO: need to set the search path appropriately too.  If run as normal user, we need to look in that users %temp%, not the current users
        private List<BurnLog> m_BurnDhtmlLogs;
        public List<BurnLog> BurnDhtmlLogs
        {
            get
            {
                return m_BurnDhtmlLogs;
            }
            set
            {
                m_BurnDhtmlLogs = value;
            }
        }

        private DateTime m_BurnStartTime;
        public DateTime BurnStartTime
        {
            get
            {
                return m_BurnStartTime;
            }
            set
            {
                m_BurnStartTime = value;
            }
        }

        private DateTime m_BurnEndTime;
        public DateTime BurnEndTime
        {
            get
            {
                return m_BurnEndTime;
            }
            set
            {
                m_BurnEndTime = value;
            }
        }

        private BurnMetricsData m_BurnMetricsData;
        public BurnMetricsData BurnMetricsData
        {
            get
            {
                return m_BurnMetricsData;
            }
            set
            {
                m_BurnMetricsData = value;
            }
        }

        // TODO: other properties that we should store:
        // inventory of all MSIs & MSPs installed (current machine state)
        // Watson/WER data (from event log)      
    }
}
