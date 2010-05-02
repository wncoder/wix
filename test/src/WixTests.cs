//-----------------------------------------------------------------------
// <copyright file="WixTests.cs" company="Microsoft">
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
//     - Contains methods that are shared across this assembly
//     - Performs some initialization before the tests are run
// </summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests
{
    using System;
    using System.CodeDom.Compiler;
    using System.IO;
    using System.Text;
    using System.Collections.Generic;
    using Microsoft.VisualStudio.TestTools.UnitTesting;

    using Microsoft.Tools.WindowsInstallerXml.Test;

    /// <summary>
    /// Contains variables and methods used by this test assembly
    /// </summary>
    [TestClass]
    public class WixTests
    {
        /// <summary>
        /// The name of the environment variable that stores the MSBuild directory
        /// </summary>
        private const string msBuildDirectoryEnvironmentVariable = "WixTestMSBuildDirectory";

        /// <summary>
        /// The name of the environment variable that stores the global seed
        /// </summary>
        private const string seedEnvironmentVariable = "WixTestsSeed";

        /// <summary>
        /// The name of the environment variable that stores the WiX bin directory
        /// </summary>
        private const string wixBinDirectoryEnvironmentVariable = "WixTestBinDirectory";

        /// <summary>
        /// The name of the environment variable that stores the wix.targets path
        /// </summary>
        private const string wixTargetsPathEnvironmentVariable = "WixTargetsPath";

        /// <summary>
        /// The name of the environment variable that stores the WixTasks.dll path
        /// </summary>
        private const string wixTasksPathEnvironmentVariable = "WixTasksPath";

        /// <summary>
        /// A seed used by all of the random number generators in the test suite
        /// </summary>
        private static int seed = 0;

        /// <summary>
        /// Stores the original value of the WIX environment variable before the test run
        /// </summary>
        private static string OriginalWIXValue;

        /// <summary>
        /// An instance of a TestContext
        /// </summary>
        private TestContext testContext;

        /// <summary>
        /// The full path to BasicProduct.wxs, which is a shared test file
        /// </summary>
        public static readonly string BasicProductWxs = Environment.ExpandEnvironmentVariables(@"%WIX_ROOT%\test\data\SharedData\Authoring\BasicProduct.wxs");

        /// <summary>
        /// The full path to PropertyFragment.wxs, which is a shared test file
        /// </summary>
        public static readonly string PropertyFragmentWxs = Environment.ExpandEnvironmentVariables(@"%WIX_ROOT%\test\data\SharedData\Authoring\PropertyFragment.wxs");

        /// <summary>
        /// The location of the shared WiX authoring
        /// </summary>
        public static readonly string SharedAuthoringDirectory = Environment.ExpandEnvironmentVariables(@"%WIX_ROOT%\test\data\SharedData\Authoring");

        /// <summary>
        /// The location of the baseline files
        /// </summary>
        public static readonly string SharedBaselinesDirectory = Environment.ExpandEnvironmentVariables(@"%WIX_ROOT%\test\data\SharedData\Baselines");

        /// <summary>
        /// The location of the shared files
        /// </summary>
        public static readonly string SharedFilesDirectory = Environment.ExpandEnvironmentVariables(@"%WIX_ROOT%\test\data\SharedData\Files");

        /// <summary>
        /// A seed used by all of the random number generators in the test suite
        /// </summary>
        public static int Seed
        {
            get { return WixTests.seed; }
        }

        /// <summary>
        /// Gets or sets the test context which provides
        /// information about and functionality for the current test run.
        /// </summary>
        public TestContext TestContext
        {
            get
            {
                return this.testContext;
            }
            set
            {
                this.testContext = value;
            }
        }

        /// <summary>
        /// Perform some initialization before the tests are run
        /// </summary>
        /// <param name="context">A TestContext</param>
        [AssemblyInitialize]
        public static void AssemblyInitialize(TestContext context)
        {
            // Set the location of MSBuild
            WixTests.SetMSBuildPaths();

            // Set the location of the binaries
            WixTests.SetWixBinDirectory();

            // Set the WIX environment variable
            WixTests.SetWIXEnvironmentVariable();

            // Initialize the random number generators' seed
            WixTests.InitializeSeed();
        }

        /// <summary>
        /// Perform some cleanup after the tests are run
        /// </summary>
        [AssemblyCleanup]
        public static void AssemblyCleanup()
        {
            WixTests.ResetWIXEnvironmentVariable();
        }

        /// <summary>
        /// Initialize the seed that will be used by random number generators in the test suite
        /// </summary>
        private static void InitializeSeed()
        {
            string currentSeed = Environment.GetEnvironmentVariable(WixTests.seedEnvironmentVariable);

            if (String.IsNullOrEmpty(currentSeed) || "0" == currentSeed.Trim())
            {
                Random random = new Random();
                WixTests.seed = random.Next();

                Console.Write("The WixTests seed was not set or it was set to '0'. Setting the seed to '{0}'. ", WixTests.Seed);
                Console.WriteLine("To use the same seed in another run, set the environment variable {0}={1} before starting the run.", seedEnvironmentVariable, WixTests.Seed);
            }
            else
            {
                try
                {
                    WixTests.seed = Convert.ToInt32(currentSeed);
                }
                catch (FormatException e)
                {
                    Console.WriteLine(e.Message);
                    Assert.Fail("The environment variable {0}={1} is not a valid integer. Please remove the environment variable or set it to a valid integer.", seedEnvironmentVariable, currentSeed);
                }
            }
        }

        /// <summary>
        /// Set the WIX environment variable back to its original value before the test run
        /// </summary>
        private static void ResetWIXEnvironmentVariable()
        {
            if (null != WixTests.OriginalWIXValue)
            {
                Environment.SetEnvironmentVariable("WIX", WixTests.OriginalWIXValue);
            }
        }

        /// <summary>
        /// Set the WIX environment variable with the value of the WIX_ROOT environment variable
        /// </summary>
        /// <remarks>
        /// Dependency on the WIX variable should eventually be replaced with a WIX_TEST_ROOT variable
        /// </remarks>
        private static void SetWIXEnvironmentVariable()
        {
            WixTests.OriginalWIXValue = Environment.GetEnvironmentVariable("WIX");

            string WIX_ROOT = Environment.GetEnvironmentVariable("WIX_ROOT");

            if (null != WIX_ROOT)
            {
                Environment.SetEnvironmentVariable("WIX", WIX_ROOT);
            }
            else
            {
                Assert.Fail(@"The WIX_ROOT environment variable must be set to the WiX root directory (eg. c:\delivery\dev\wix)");
            }
        }

        /// <summary>
        /// Sets the default location for MSBuild.exe, wix.targets and WixTasks.dll
        /// </summary>
        private static void SetMSBuildPaths()
        {
            // MSBuild Directory
            
            string msBuildDirectory = Environment.GetEnvironmentVariable(WixTests.msBuildDirectoryEnvironmentVariable);

            if (null == msBuildDirectory)
            {
                msBuildDirectory = Path.Combine(Environment.GetEnvironmentVariable("SystemRoot"), @"Microsoft.NET\Framework\v3.5");
                Console.WriteLine("The environment variable '{0}' was not set. Using the default location '{1}' for the MSBuild.exe", WixTests.msBuildDirectoryEnvironmentVariable, msBuildDirectory);
            }

            Settings.MSBuildDirectory = msBuildDirectory;


            // wix.targets

            string wixTargetsPath = Environment.GetEnvironmentVariable(WixTests.wixTargetsPathEnvironmentVariable);

            if (null != wixTargetsPath)
            {
                Settings.WixTargetsPath = wixTargetsPath;
            }
            else
            {
                Console.WriteLine("The environment variable '{0}' was not set. The location for wix.targets will not be explicitly specified to MSBuild.", WixTests.wixTargetsPathEnvironmentVariable);
            }

            
            // WixTasks.dll

            string wixTasksPath = Environment.GetEnvironmentVariable(WixTests.wixTasksPathEnvironmentVariable);

            if (null != wixTasksPath)
            {
                Settings.WixTasksPath = wixTasksPath;
            }
            else
            {
                Console.WriteLine("The environment variable '{0}' was not set. The location for WixTasks.dll will not be explicitly specified to MSBuild.", WixTests.wixTasksPathEnvironmentVariable);
            }
        }

        /// <summary>
        /// Sets the default location for the WiX binaries
        /// </summary>
        private static void SetWixBinDirectory()
        {
            string wixBinDirectory = Environment.GetEnvironmentVariable(WixTests.wixBinDirectoryEnvironmentVariable);

            if (null == wixBinDirectory)
            {
                wixBinDirectory = Environment.ExpandEnvironmentVariables(@"%WIX_ROOT%\build\debug.coverage\x86");
                Console.WriteLine("The environment variable '{0}' was not set. Using the default location '{1}' for the WiX binaries", wixBinDirectoryEnvironmentVariable, wixBinDirectory);
            }

            Settings.WixToolDirectory = wixBinDirectory;
        }
    }
}