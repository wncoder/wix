//-----------------------------------------------------------------------
// <copyright file="PackageBuilder.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>
//     Provides methods for building an MSI.
// </summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests
{
    using System;
    using System.IO;
    using System.Linq;
    using Microsoft.Tools.WindowsInstallerXml.Test.Utilities;

    /// <summary>
    /// Provides methods for building an MSI.
    /// </summary>
    public class PackageBuilder : BuilderBase<PackageBuilder>
    {
        public PackageBuilder(WixTests test, string name)
            : base(test, name)
        {
        }

        /// <summary>
        /// Builds the package.
        /// </summary>
        /// <returns>The path to the built MSI package.</returns>
        protected override PackageBuilder BuildItem()
        {
            // Create paths.
            string source = String.IsNullOrEmpty(this.SourceFile) ? Path.Combine(this.test.TestDataDirectory2, String.Concat(this.Name, ".wxs")) : this.SourceFile;
            string rootDirectory = FileUtilities.GetUniqueFileName();
            string objDirectory = Path.Combine(rootDirectory, Settings.WixobjFolder);
            string msiDirectory = Path.Combine(rootDirectory, Settings.MSIFolder);
            string package = Path.Combine(msiDirectory, String.Concat(this.Name, ".msi"));

            // Add the root directory to be cleaned up.
            this.test.TestArtifacts.Add(new DirectoryInfo(rootDirectory));

            // Compile.
            Candle candle = new Candle();
            candle.Extensions.AddRange(this.Extensions);
            candle.OtherArguments = String.Concat("-dTestName=", this.test.TestContext.TestName);
            this.PreprocessorVariables.ToList().ForEach(kv => candle.OtherArguments = String.Concat(candle.OtherArguments, " -d", kv.Key, "=", kv.Value));
            candle.OutputFile = String.Concat(objDirectory, @"\");
            candle.SourceFiles.Add(source);
            candle.SourceFiles.AddRange(this.AdditionalSourceFiles);
            candle.WorkingDirectory = this.test.TestDataDirectory2;
            candle.Run();

            // Make sure the output directory is cleaned up.
            this.test.TestArtifacts.Add(new DirectoryInfo(objDirectory));

            // Link.
            Light light = new Light();
            light.Extensions.AddRange(this.Extensions);
            light.OtherArguments = String.Concat("-b data=", Environment.ExpandEnvironmentVariables(@"%WIX_ROOT%\test\data\"));
            this.BindPaths.ToList().ForEach(kv => light.OtherArguments = String.Concat(light.OtherArguments, " -b ", kv.Key, "=", kv.Value));
            light.ObjectFiles = candle.ExpectedOutputFiles;
            light.OutputFile = package;
            light.SuppressMSIAndMSMValidation = true;
            light.WorkingDirectory = this.test.TestDataDirectory2;
            light.Run();

            // Make sure the output directory is cleaned up.
            this.test.TestArtifacts.Add(new DirectoryInfo(msiDirectory));

            this.Output = light.OutputFile;
            return this;
        }

        /// <summary>
        /// Ensures the packages built previously are uninstalled.
        /// </summary>
        protected override void UninstallItem(BuiltItem item)
        {
            MSIExec exec = new MSIExec();
            exec.ExecutionMode = MSIExec.MSIExecMode.Uninstall;
            exec.OtherArguments = "IGNOREDEPENDENCIES=ALL";
            exec.Product = item.Path;

            // Generate the log file name.
            string logFile = String.Format("{0}_{1:yyyyMMddhhmmss}_Cleanup_{2}.log", item.TestName, DateTime.UtcNow, Path.GetFileNameWithoutExtension(item.Path));
            exec.LogFile = Path.Combine(Path.GetTempPath(), logFile);

            exec.Run(false);
        }
    }
}
