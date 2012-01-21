//-----------------------------------------------------------------------
// <copyright file="WixTests.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
//    
//    The use and distribution terms for this software are covered by the
//    Common Public License 1.0 (http://opensource.org/licenses/cpl1.0.php)
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
    using System.Diagnostics;
    using System.Runtime.InteropServices;

    using Microsoft.Tools.WindowsInstallerXml.Test;

    /// <summary>
    /// Contains variables and methods used by this test assembly
    /// </summary>
    [TestClass]
    public class WixTests
    {
        #region Static Variables

        /// <summary>
        /// Stores the original value of the WIX environment variable before the test run
        /// </summary>
        private static string OriginalWIXValue;

        /// <summary>
        /// A seed used by all of the random number generators in the test suite
        /// </summary>
        private static int seed = 0;

        /// <summary>
        /// True if the WIX_ROOT environment variable was set, false otherwise
        /// </summary>
        private static bool wixRootIsSet = false;

        #region Static variables for environment variable names and values

        /// <summary>
        /// The name of the environment variable that stores the build flavor to run tests against
        /// </summary>
        private const string flavorEnvironmentVariable = "Flavor";

        /// <summary>
        /// The name of the environment variable that stores the MSBuild directory
        /// </summary>
        private const string msBuildDirectoryEnvironmentVariable = "WixTestMSBuildDirectory";

        /// <summary>
        /// The name of the environment variable that states that the runtime tests are enabled on this machine
        /// </summary>
        private const string runtimeTestsEnabledEnvironmentVariable = "RuntimeTestsEnabled";

        /// <summary>
        /// The name of the environment variable that stores the global seed
        /// </summary>
        private const string seedEnvironmentVariable = "WixTestsSeed";

        /// <summary>
        /// The name of the environment variable that stores the WiX bin directory
        /// </summary>
        private const string wixToolsPathEnvironmentVariable = "WixToolsPath";

        /// <summary>
        /// The name of the environment variable that stores the wix.targets path
        /// </summary>
        private const string wixTargetsPathEnvironmentVariable = "WixTargetsPath";

        /// <summary>
        /// The name of the environment variable that stores the WixTasks.dll path
        /// </summary>
        private const string wixTasksPathEnvironmentVariable = "WixTasksPath";

        #endregion

        #region Static variables for file paths

        /// <summary>
        /// The full path to BasicProduct.msi, which is a shared test file
        /// </summary>
        public static readonly string BasicProductMsi = Environment.ExpandEnvironmentVariables(@"%WIX_ROOT%\test\data\SharedData\Baselines\BasicProduct.msi");

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

        #endregion

        #endregion

        #region Instance Variables

        /// <summary>
        /// Keeps track of the current working directory
        /// </summary>
        private Stack<string> workingDirectories = new Stack<string>();

        #endregion

        /// <summary>
        /// A seed used by all of the random number generators in the test suite
        /// </summary>
        public static int Seed { get { return WixTests.seed; } }

        /// <summary>
        /// Gets or sets the test context which provides
        /// information about and functionality for the current test run.
        /// </summary>
        public TestContext TestContext { get; set; }

        /// <summary>
        /// Gets the directory for the data used by test test. Defaults to the unique fully qualified name of the test under the "%WIX_ROOT%\test\data" directory.
        /// </summary>
        public virtual string TestDataDirectory2
        {
            get
            {
                string directory = Environment.ExpandEnvironmentVariables(@"%WIX_ROOT%\test\data\");
                if (this.TestContext.FullyQualifiedTestClassName.StartsWith("Microsoft.Tools.WindowsInstallerXml.Test.Tests."))
                {
                    directory = Path.Combine(directory, this.TestContext.FullyQualifiedTestClassName.Substring("Microsoft.Tools.WindowsInstallerXml.Test.Tests.".Length).Replace('.', '\\'));
                }

                return directory;
            }
        }

        /// <summary>
        /// A test directory that is unique to this test (for storing test artifacts)
        /// </summary>
        public string TestDirectory { get; private set; }

        /// <summary>
        /// A list of files and directories that are considered to be test artifacts
        /// </summary>
        public List<FileSystemInfo> TestArtifacts { get; private set; }

        /// <summary>
        /// Clean up all of the artifacts that were created by the test
        /// </summary>
        public bool CleanTestArtifacts { get; set; }

        #region P/Invoke declarations
        /// <summary>
        /// Returns true if the current process is Wow64 process.
        /// </summary>
        /// <param name="hProcess">Process handle</param>
        /// <param name="wow64Process">Return bool</param>
        /// <returns></returns>
        [DllImport("kernel32.dll", SetLastError = true, CallingConvention = CallingConvention.Winapi)]
        [return: MarshalAs(UnmanagedType.Bool)]
        private static extern bool IsWow64Process([In] IntPtr hProcess, [Out] out bool wow64Process);
        #endregion

        /// <summary>
        /// Returns true if the current OS is a 64 bit OS
        /// </summary>
        public bool Is64BitMachine
        {
            get
            {
                bool isWow64Process;
                IsWow64Process(Process.GetCurrentProcess().Handle, out isWow64Process);
                // it is a 64 bit system iff this is a 64 bit process or a 32 bit process running on WoW
                return (IntPtr.Size == 8 || (IntPtr.Size == 4 && isWow64Process));
            }
        }

        /// <summary>
        /// Determines whether runtime tests are enabled on the current machine.
        /// </summary>
        public bool IsRuntimeTestsEnabled
        {
            get
            {
                string runtimeTestsEnabled = Environment.GetEnvironmentVariable(WixTests.runtimeTestsEnabledEnvironmentVariable);
                return "true".Equals(runtimeTestsEnabled, StringComparison.OrdinalIgnoreCase);
            }
        }

        /// <summary>
        /// Perform some initialization before the tests are run
        /// </summary>
        /// <param name="context">A TestContext</param>
        [AssemblyInitialize]
        public static void AssemblyInitialize(TestContext context)
        {
            // set the build flavor
            WixTests.SetTestFalvor();

            // Set the location of MSBuild
            WixTests.SetMSBuildPaths();

            // Set the location of the binaries
            WixTests.SetWixToolsPathDirectory();

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
        /// Initialize Wix tests.
        /// </summary>
        /// <remarks>This method will check that a test has pre-reqs set.</remarks>
        /// <remarks>This method will end the execution of tests marked as IsRuntimeTest=true if the Runtime tests are not enabled on current machine.</remarks>
        /// <remarks>This method will end the execution of tests marked as Is64BitSpecificTest=true if the current OS is not a 64 bit OS.</remarks>
        /// <remarks>Create a unique directory for this test to store test artifacts</remarks>
        [TestInitialize]
        public void WixTestTestInitialize()
        {
            this.CheckTestPreReqs();

            // Check if test is a runtime test and if test is 64 bit specific
            System.Reflection.MethodInfo testMethodInformation = this.GetType().GetMethod(this.TestContext.TestName);
            TestPropertyAttribute[] customTestMethodProperties = (TestPropertyAttribute[])testMethodInformation.GetCustomAttributes(typeof(TestPropertyAttribute), false);

            foreach (TestPropertyAttribute property in customTestMethodProperties)
            {
                if (property.Name.Equals("IsRuntimeTest") && property.Value.Equals("true") && !this.IsRuntimeTestsEnabled)
                {
                    Assert.Fail("Runtime tests are not enabled on this test environment. To enable Runtime tests set the environment variable '{0}'=true.", WixTests.runtimeTestsEnabledEnvironmentVariable);
                }

                if (property.Name.Equals("Is64BitSpecificTest") && property.Value.Equals("true") && !this.Is64BitMachine)
                {
                    Assert.Fail("64-bit specific tests are not enabled on 32-bit machines.");
                }
            }

            this.TestArtifacts = new List<FileSystemInfo>();

            // Create a unique directory for this test and add it to the list of test artifacts
            this.TestDirectory = Utilities.FileUtilities.GetUniqueFileName(this.TestContext.TestDir);
            Utilities.FileUtilities.CreateOutputDirectory(this.TestDirectory);
            this.TestArtifacts.Add(new DirectoryInfo(this.TestDirectory));

            // Set the current working directory to the test directory
            this.workingDirectories.Push(Environment.CurrentDirectory);
            this.workingDirectories.Push(this.TestDirectory);
            Utilities.FileUtilities.SetWorkingDirectory(this.TestDirectory);
        }

        /// <summary>
        /// Cleanup Wix tests.
        /// </summary>
        [TestCleanup]
        public virtual void CleanUp()
        {
            // Make sure all built products are removed
            PackageBuilder.CleanupByUninstalling();
            MSIExec.UninstallAllInstalledProducts();
            BundleBuilder.CleanupByUninstalling();

            // Update the working directory
            this.workingDirectories.Pop();
            Environment.CurrentDirectory = this.workingDirectories.Peek();

            // Clean up the test artifacts iff the test passed, else keep them for analysis
            if (this.TestContext.CurrentTestOutcome == UnitTestOutcome.Passed)
            {
                if (this.CleanTestArtifacts)
                {
                    foreach (FileSystemInfo artifact in this.TestArtifacts)
                    {
                        if (artifact is DirectoryInfo)
                        {
                            Directory.Delete(artifact.FullName, true);
                        }
                        else
                        {
                            artifact.Delete();
                        }
                    }
                }
            }
        }

        /// <summary>
        /// Checks that certain conditions are met before the test can run
        /// </summary>
        protected virtual void CheckTestPreReqs()
        {
            if (!WixTests.wixRootIsSet)
            {
                Assert.Inconclusive(@"The WIX_ROOT environment variable must be set to the WiX root directory (eg. c:\delivery\dev\wix)");
            }
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
                WixTests.wixRootIsSet = true;
                Environment.SetEnvironmentVariable("WIX", WIX_ROOT);
            }
            else
            {
                WixTests.wixRootIsSet = false;
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
        /// Sets the default build flavor to run tests against
        /// </summary>
        private static void SetTestFalvor()
        {
            string flavor = Environment.GetEnvironmentVariable(WixTests.flavorEnvironmentVariable);

            if (null == flavor)
            {
                flavor = "debug";
                Console.WriteLine("The environment variable '{0}' was not set. Using the default build flavor '{1}' to run tests aginst", WixTests.flavorEnvironmentVariable, flavor);
            }

            Settings.Flavor = flavor;
        }


        /// <summary>
        /// Sets the default location for the WiX binaries
        /// </summary>
        private static void SetWixToolsPathDirectory()
        {
            string wixToolsPathDirectory = Environment.GetEnvironmentVariable(WixTests.wixToolsPathEnvironmentVariable);

            if (null == wixToolsPathDirectory)
            {
                if (string.IsNullOrEmpty(Settings.Flavor))
                {
                    Assert.Fail("The build Flavor is not set. Please set the flavor environment variable to the desired build flavor.");
                }
                else
                {
                    wixToolsPathDirectory = Environment.ExpandEnvironmentVariables(string.Format(@"%WIX_ROOT%\build\{0}\x86", Settings.Flavor));
                    Console.WriteLine("The environment variable '{0}' was not set. Using the default location '{1}' for the WiX binaries", wixToolsPathEnvironmentVariable, wixToolsPathDirectory);
                }
            }

            Settings.WixToolDirectory = wixToolsPathDirectory;
        }

        /// <summary>
        /// Set tracing options to output to Console.Out too.
        /// </summary>
        /// <remarks>This is done because some test harneses only record output from StdOut so write the results there too.</remarks>
        /// <remarks>This must be done after AssemblyInitialize.  If you do it in AssemblyInitialize, it gets overwritten.</remarks>
        /// <remarks>This should only need to be called once.  
        /// But for data driven tests, the textwriter gets closed after the test executes against the first data record.
        /// So data driven tests should call this in their own TestInitialize or constructor or in each TestMethod to ensure
        /// the trace is being written to Console.Out.  If you don't an exception will be thrown when the 2nd test data
        /// record executes and tries to write to the trace.</remarks>
        public static void SetTraceToOutputToConsole()
        {
            TextWriterTraceListener twtl = new TextWriterTraceListener(Console.Out);
            twtl = new TextWriterTraceListener(Console.Out);
            twtl.Name = "WriteTracesToConsoleOut";
            Trace.Listeners.Remove(twtl.Name);
            Trace.Listeners.Add(twtl);
            Trace.AutoFlush = true;
            Trace.IndentSize = 5;
        }
    }
}