//-----------------------------------------------------------------------
// <copyright file="PatchBuilder.cs" company="Microsoft">
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
//     Provides methods for building a Patch (.MSP).
// </summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests
{
    using System;
    using System.IO;
    using System.Linq;
    using Microsoft.Tools.WindowsInstallerXml.Test.Utilities;

    /// <summary>
    /// Provides methods for building a Patch.
    /// </summary>
    class PatchBuilder : BuilderBase<PatchBuilder>
    {
        public PatchBuilder(WixTests test, string name)
            : base(test, name)
        {
        }

        /// <summary>
        /// Ges and sets the path to the target MSI.
        /// </summary>
        public string TargetPath { get; set; }

        /// <summary>
        /// Gets and sets the path to the upgrade MSI.
        /// </summary>
        public string UpgradePath { get; set; }

        /// <summary>
        /// Builds a patch using given paths for the target and upgrade packages.
        /// </summary>
        /// <returns>The path to the patch.</returns>
        protected override PatchBuilder BuildItem()
        {
            // Create paths.
            string source = String.IsNullOrEmpty(this.SourceFile) ? Path.Combine(this.test.TestDataDirectory2, String.Concat(this.Name, ".wxs")) : this.SourceFile;
            string rootDirectory = FileUtilities.GetUniqueFileName();
            string objDirectory = Path.Combine(rootDirectory, Settings.WixobjFolder);
            string msiDirectory = Path.Combine(rootDirectory, Settings.MSPFolder);
            string wixmst = Path.Combine(objDirectory, String.Concat(this.Name, ".wixmst"));
            string wixmsp = Path.Combine(objDirectory, String.Concat(this.Name, ".wixmsp"));
            string package = Path.Combine(msiDirectory, String.Concat(this.Name, ".msp"));

            // Add the root directory to be cleaned up.
            this.test.TestArtifacts.Add(new DirectoryInfo(rootDirectory));

            // Compile.
            Candle candle = new Candle();
            candle.Extensions.AddRange(this.Extensions);
            candle.OtherArguments = String.Concat("-dTestName=", this.test.TestContext.TestName);
            this.PreprocessorVariables.ToList().ForEach(kv => candle.OtherArguments = String.Concat(candle.OtherArguments, " -d", kv.Key, "=", kv.Value));
            candle.OutputFile = String.Concat(objDirectory, @"\");
            candle.SourceFiles.Add(source);
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
            light.OutputFile = wixmsp;
            light.SuppressMSIAndMSMValidation = true;
            light.WorkingDirectory = this.test.TestDataDirectory2;
            light.Run();

            // Make sure the output directory is cleaned up.
            this.test.TestArtifacts.Add(new DirectoryInfo(msiDirectory));

            // Torch.
            Torch torch = new Torch();
            torch.TargetInput = Path.ChangeExtension(this.TargetPath, "wixpdb");
            torch.UpdatedInput = Path.ChangeExtension(this.UpgradePath, "wixpdb");
            torch.PreserveUnmodified = true;
            torch.XmlInput = true;
            torch.OutputFile = wixmst;
            torch.WorkingDirectory = this.test.TestDataDirectory2;
            torch.Run();

            // Pyro.
            Pyro pyro = new Pyro();
            pyro.Baselines.Add(torch.OutputFile, this.Name);
            pyro.InputFile = light.OutputFile;
            pyro.OutputFile = package;
            pyro.WorkingDirectory = this.test.TestDataDirectory2;
            pyro.SuppressWarnings.Add("1079");
            pyro.Run();

            this.Output = pyro.OutputFile;
            return this;
        }

        /// <summary>
        /// Patches are uninstalled by the MSIs they target.
        /// </summary>
        protected override void UninstallItem(BuiltItem item)
        {
            // Nothing to do for patches.
        }
    }
}
