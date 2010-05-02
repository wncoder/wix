//-----------------------------------------------------------------------
// <copyright file="BinaryPathTests.cs" company="Microsoft">
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//    
//    The use and distribution terms for this software are covered by the
//    Common Public License 1.0 (http://opensource.org/licenses/cpl.php)
//    which can be found in the file CPL.TXT at the root of this distribution.
//    By using this software in any fashion, you are agreeing to be bound by
//    the terms of this license.
//    
//    You must not remove this notice, or any other, from this software.
// </copyright>
// <summary>Test how Dark handles the -x switch</summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests.Tools.Dark.BinaryPath
{
    using System;
    using System.IO;
    using Microsoft.VisualStudio.TestTools.UnitTesting;
    using Microsoft.Tools.WindowsInstallerXml.Test;
    
    /// <summary>
    /// Test how Dark handles the -x switch.
    /// </summary>
    [TestClass]
    public class BinaryPathTests : WixTests
    {
        private static readonly string TestDataDirectory = Environment.ExpandEnvironmentVariables(@"%WIX_ROOT%\test\data\Tools\Dark\BinaryPath\BinaryPathTests");

        [TestMethod]
        [Description("Verify that Dark generates the expected error for a missing path after the -x switch.")]
        [Priority(2)]
        public void MissingBinaryPath()
        {
            Dark dark = new Dark();
            dark.InputFile = Path.Combine(WixTests.SharedBaselinesDirectory, @"MSIs\BasicProduct.msi");
            dark.OtherArguments = " -x";
            dark.ExpectedWixMessages.Add(new WixMessage(280,String.Concat("The -x option requires a directory, but the provided path is a file: ", dark.InputFile), WixMessage.MessageTypeEnum.Error));
            dark.ExpectedExitCode = 280;
            dark.Run();
        }

        [TestMethod]
        [Description("Verify that Dark fails gracefully when given -x option with read only path.")]
        [Priority(2)]
        public void ExportBinariesToReadOnlyPath()
        {
            // Create a directory in temporary directory and set its property to read only
            DirectoryInfo binaryDirectory = Directory.CreateDirectory(Path.Combine(Path.GetTempPath(), "ReadOnlyDirectory"));
            System.Security.AccessControl.DirectorySecurity binaryDirectorySecurity = new System.Security.AccessControl.DirectorySecurity();
            binaryDirectorySecurity.AddAccessRule(new System.Security.AccessControl.FileSystemAccessRule("everyone", System.Security.AccessControl.FileSystemRights.CreateDirectories, System.Security.AccessControl.AccessControlType.Deny));
            Directory.SetAccessControl(binaryDirectory.FullName, binaryDirectorySecurity);

            Dark dark = new Dark();
            dark.InputFile = Builder.BuildPackage(Path.Combine(BinaryPathTests.TestDataDirectory, @"Product.wxs"));
            dark.BinaryPath = binaryDirectory.FullName;
            dark.ExpectedExitCode = 1;
            dark.ExpectedWixMessages.Add(new WixMessage(1, string.Format("Access to the path '{0}' is denied.", Path.Combine(dark.BinaryPath, "Binary")), WixMessage.MessageTypeEnum.Error));
            dark.Run();
        }
    }
}