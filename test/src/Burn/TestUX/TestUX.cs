//-------------------------------------------------------------------------------------------------
// <copyright file="TestUX.cs" company="Microsoft">
// Copyright (c) Microsoft Corporation. All rights reserved.
//    
//    The use and distribution terms for this software are covered by the
//    Common Public License 1.0 (http://opensource.org/licenses/cpl1.0.php)
//    which can be found in the file CPL.TXT at the root of this distribution.
//    By using this software in any fashion, you are agreeing to be bound by
//    the terms of this license.
//    
//    You must not remove this notice, or any other, from this software.
// </copyright>
// 
// <summary>
// A minimal UX used for testing.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.UX
{
    using System;
    using System.Threading;
    using Microsoft.Tools.WindowsInstallerXml.Bootstrapper;

    /// <summary>
    /// A minimal UX used for testing.
    /// </summary>
    public class TestUX : BootstrapperApplication
    {
        private LaunchAction action;
        private ManualResetEvent wait;
        private int result;

        /// <summary>
        /// Initializes test user experience.
        /// </summary>
        public TestUX()
        {
            this.wait = new ManualResetEvent(false);
        }

        /// <summary>
        /// UI Thread entry point for TestUX.
        /// </summary>
        protected override void Run()
        {
            this.action = this.Command.Action;

            this.Engine.Detect();

            this.wait.WaitOne();
            this.Engine.Quit(this.result);
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

        protected override void OnApplyComplete(ApplyCompleteEventArgs args)
        {
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
    }
}
