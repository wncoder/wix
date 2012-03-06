//-------------------------------------------------------------------------------------------------
// <copyright file="TestBA.cs" company="Microsoft">
// Copyright (c) Microsoft Corporation. All rights reserved.
// </copyright>
// 
// <summary>
// A minimal BA used for testing.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.BA
{
    using System;
    using System.Threading;
    using Microsoft.Tools.WindowsInstallerXml.Bootstrapper;
    using Microsoft.Win32;

    /// <summary>
    /// A minimal UX used for testing.
    /// </summary>
    public class TestBA : BootstrapperApplication
    {
        private LaunchAction action;
        private ManualResetEvent wait;
        private int result;

        private int sleepDuringCache;
        private int cancelCacheAtProgress;
        private int sleepDuringExecute;
        private int cancelExecuteAtProgress;

        /// <summary>
        /// Initializes test user experience.
        /// </summary>
        public TestBA()
        {
            this.wait = new ManualResetEvent(false);
        }

        /// <summary>
        /// UI Thread entry point for TestUX.
        /// </summary>
        protected override void Run()
        {
            this.action = this.Command.Action;

            this.TestVariables();
            this.Engine.Detect();

            this.wait.WaitOne();
            this.Engine.Quit(this.result & 0xFFFF); // return plain old Win32 error, not HRESULT.
        }

        protected override void OnDetectComplete(DetectCompleteEventArgs args)
        {
            this.result = args.Status;
            if (Hresult.Succeeded(this.result))
            {
                this.Engine.Plan(this.action);
            }
            else
            {
                this.wait.Set();
            }
        }

        protected override void OnPlanPackageBegin(PlanPackageBeginEventArgs args)
        {
            RequestState state;
            string action = ReadPackageAction(args.PackageId, "Requested");
            if (TryParseEnum<RequestState>(action, out state))
            {
                args.State = state;
            }
        }

        protected override void OnPlanComplete(PlanCompleteEventArgs args)
        {
            this.result = args.Status;
            if (Hresult.Succeeded(this.result))
            {
                this.Engine.Apply(IntPtr.Zero);
            }
            else
            {
                this.wait.Set();
            }
        }

        protected override void OnCachePackageBegin(CachePackageBeginEventArgs args)
        {
            this.Engine.Log(LogLevel.Standard, String.Format("TESTBA: OnCachePackageBegin() - package: {0}, payloads to cache: {1}", args.PackageId, args.CachePayloads));

            string slowProgress = ReadPackageAction(args.PackageId, "SlowCache");
            if (String.IsNullOrEmpty(slowProgress) || !Int32.TryParse(slowProgress, out this.sleepDuringCache))
            {
                this.sleepDuringCache = 0;
            }

            string cancelCache = ReadPackageAction(args.PackageId, "CancelCacheAtProgress");
            if (String.IsNullOrEmpty(cancelCache) || !Int32.TryParse(cancelCache, out this.cancelCacheAtProgress))
            {
                this.cancelCacheAtProgress = -1;
            }
        }

        protected override void OnCacheAcquireProgress(CacheAcquireProgressEventArgs args)
        {
            this.Engine.Log(LogLevel.Standard, String.Format("TESTBA: OnCacheAcquireProgress() - container/package: {0}, payload: {1}, progress: {2}, total: {3}, overall progress: {4}%", args.PackageOrContainerId, args.PayloadId, args.Progress, args.Total, args.OverallPercentage));

            if (this.cancelCacheAtProgress > 0 && this.cancelCacheAtProgress <= args.Progress)
            {
                args.Result = Result.Cancel;
            }
            else if (this.sleepDuringCache > 0)
            {
                Thread.Sleep(this.sleepDuringCache);
            }
        }

        protected override void OnExecutePackageBegin(ExecutePackageBeginEventArgs args)
        {
            this.Engine.Log(LogLevel.Standard, String.Format("TESTBA: OnExecutePackageBegin() - package: {0}, rollback: {1}", args.PackageId, !args.ShouldExecute));

            string slowProgress = ReadPackageAction(args.PackageId, "SlowExecute");
            if (String.IsNullOrEmpty(slowProgress) || !Int32.TryParse(slowProgress, out this.sleepDuringExecute))
            {
                this.sleepDuringExecute = 0;
            }

            string cancelExecute = ReadPackageAction(args.PackageId, "CancelExecuteAtProgress");
            if (String.IsNullOrEmpty(cancelExecute) || !Int32.TryParse(cancelExecute, out this.cancelExecuteAtProgress))
            {
                this.cancelExecuteAtProgress = -1;
            }
        }

        protected override void OnExecuteProgress(ExecuteProgressEventArgs args)
        {
            this.Engine.Log(LogLevel.Standard, String.Format("TESTBA: OnExecuteProgress() - package: {0}, progress: {1}%, overall progress: {2}%", args.PackageId, args.ProgressPercentage, args.OverallPercentage));

            if (this.cancelExecuteAtProgress > 0 && this.cancelExecuteAtProgress <= args.ProgressPercentage)
            {
                args.Result = Result.Cancel;
            }
            else if (this.sleepDuringExecute > 0)
            {
                Thread.Sleep(this.sleepDuringExecute);
            }
        }

        protected override void OnExecutePatchTarget(ExecutePatchTargetEventArgs args)
        {
            this.Engine.Log(LogLevel.Verbose, String.Format("TEST: OnExecutePatchTarget - Patch Package: {0}, Target Product Code: {1}", args.PackageId, args.TargetProductCode));
        }

        protected override void OnProgress(ProgressEventArgs args)
        {
            this.Engine.Log(LogLevel.Standard, String.Format("TESTBA: OnProgress() - progress: {0}%, overall progress: {1}%", args.ProgressPercentage, args.OverallPercentage));
        }

        protected override void OnApplyComplete(ApplyCompleteEventArgs args)
        {
            // Output what the privileges are now.
            this.Engine.Log(LogLevel.Verbose, String.Format("TEST: After elevation: WixBundleElevated = {0}", this.Engine.NumericVariables["WixBundleElevated"]));

            this.result = args.Status;
            this.wait.Set();
        }

        protected override void OnSystemShutdown(SystemShutdownEventArgs args)
        {
            // Always prevent shutdown.
            this.Engine.Log(LogLevel.Verbose, "Disallowed system request to shut down the bootstrapper application.");
            args.Result = Result.Cancel;

            this.wait.Set();
        }

        private void TestVariables()
        {
            // First make sure we can check and get standard variables of each type.
            {
                string value = null;
                if (this.Engine.StringVariables.Contains("WindowsFolder"))
                {
                    value = this.Engine.StringVariables["WindowsFolder"];
                    this.Engine.Log(LogLevel.Verbose, "TEST: Successfully retrieved a string variable: WindowsFolder");
                }
                else
                {
                    throw new Exception("Engine did not define a standard string variable: WindowsFolder");
                }
            }

            {
                long value = 0;
                if (this.Engine.NumericVariables.Contains("NTProductType"))
                {
                    value = this.Engine.NumericVariables["NTProductType"];
                    this.Engine.Log(LogLevel.Verbose, "TEST: Successfully retrieved a numeric variable: NTProductType");
                }
                else
                {
                    throw new Exception("Engine did not define a standard numeric variable: NTProductType");
                }
            }

            {
                Version value = new Version();
                if (this.Engine.VersionVariables.Contains("VersionMsi"))
                {
                    value = this.Engine.VersionVariables["VersionMsi"];
                    this.Engine.Log(LogLevel.Verbose, "TEST: Successfully retrieved a version variable: VersionMsi");
                }
                else
                {
                    throw new Exception("Engine did not define a standard version variable: VersionMsi");
                }
            }

            // Now validate that Contians returns false for non-existant variables of each type.
            if (this.Engine.StringVariables.Contains("TestStringVariableShouldNotExist"))
            {
                throw new Exception("Engine defined a string variable that should not exist: TestStringVariableShouldNotExist");
            }
            else
            {
                this.Engine.Log(LogLevel.Verbose, "TEST: Successfully checked for non-existent string variable: TestStringVariableShouldNotExist");
            }

            if (this.Engine.NumericVariables.Contains("TestNumericVariableShouldNotExist"))
            {
                throw new Exception("Engine defined a numeric variable that should not exist: TestNumericVariableShouldNotExist");
            }
            else
            {
                this.Engine.Log(LogLevel.Verbose, "TEST: Successfully checked for non-existent numeric variable: TestNumericVariableShouldNotExist");
            }

            if (this.Engine.VersionVariables.Contains("TestVersionVariableShouldNotExist"))
            {
                throw new Exception("Engine defined a version variable that should not exist: TestVersionVariableShouldNotExist");
            }
            else
            {
                this.Engine.Log(LogLevel.Verbose, "TEST: Successfully checked for non-existent version variable: TestVersionVariableShouldNotExist");
            }

            // Output what the initially run privileges were.
            this.Engine.Log(LogLevel.Verbose, String.Format("TEST: WixBundleElevated = {0}", this.Engine.NumericVariables["WixBundleElevated"]));
        }

        private string ReadPackageAction(string packageId, string state)
        {
            string testName = this.Engine.StringVariables["TestName"];
            using (RegistryKey testKey = Registry.LocalMachine.OpenSubKey(String.Format(@"Software\WiX\Tests\{0}\{1}", testName, packageId)))
            {
                return testKey == null ? null : testKey.GetValue(state) as string;
            }
        }

        private bool TryParseEnum<T>(string value, out T t)
        {
            try
            {
                t = (T)Enum.Parse(typeof(T), value, true);
                return true;
            }
            catch (ArgumentException) { }
            catch (OverflowException) { }

            t = default(T);
            return false;
        }
    }
}
