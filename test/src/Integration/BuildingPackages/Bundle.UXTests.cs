//-----------------------------------------------------------------------
// <copyright file="Bundle.UXTests.cs" company="Microsoft">
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
//     Tests for Bundle UX element
// </summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests.Integration.BuildingPackages.Bundle
{
    using System;
    using System.Collections.Generic;
    using System.IO;
    using System.Xml;
    using Microsoft.Deployment.Compression.Cab;

    using Microsoft.VisualStudio.TestTools.UnitTesting;
    using Microsoft.Tools.WindowsInstallerXml.Test;
    using Microsoft.Tools.WindowsInstallerXml.Test.Verifiers;

    /// <summary>
    /// Tests for Bundle UX element
    /// </summary>
    [TestClass]
    public class UXTests : BundleTests
    {
        private static readonly string TestDataDirectory = Environment.ExpandEnvironmentVariables(@"%WIX_ROOT%\test\data\Integration\BuildingPackages\Bundle\UXTests");

        [TestMethod]
        [Description("Name is optional and will default to the SourceFile file name")]
        [Priority(2)]
        public void UXNameNotSpecified()
        {
            string sourceFile = Path.Combine(UXTests.TestDataDirectory, @"UXNameNotSpecified\Product.wxs");
            string testFile = Path.Combine(BundleTests.BundleSharedFilesDirectory, @"Bootstrapper.exe");
            string outputDirectory = this.TestDirectory;

            // build the bootstrapper
            string bootstrapper = Builder.BuildBundlePackage(outputDirectory, sourceFile);

            // verify the ParameterInfo and burnManifest has the correct information 
            UXTests.VerifyUXPayloadInformation(outputDirectory, testFile, "Bootstrapper.exe");
        }

        [TestMethod]
        [Description("Name can be explicitly defined.")]
        [Priority(2)]
        public void UXNameSpecified()
        {
            string sourceFile = Path.Combine(UXTests.TestDataDirectory, @"UXNameSpecified\Product.wxs");
            string testFile = Path.Combine(BundleTests.BundleSharedFilesDirectory, @"Bootstrapper.exe");
            string outputDirectory = this.TestDirectory;

            // build the bootstrapper
            string bootstrapper = Builder.BuildBundlePackage(outputDirectory, sourceFile);

            // verify the ParameterInfo and burnManifest has the correct information 
            UXTests.VerifyUXPayloadInformation(outputDirectory, testFile, "Setup.exe");
        }

        [TestMethod]
        [Description("Verify that there is an error if the UX SourceFile attribute has an invalid file name")]
        [Priority(3)]
        public void UXInvalidSourceFile()
        {
            string candleOutput = Candle.Compile(Path.Combine(UXTests.TestDataDirectory, @"UXInvalidSourceFile\Product.wxs"));

            Light light = new Light();
            light.ObjectFiles.Add(candleOutput);
            light.OutputFile = "setup.exe";
            light.ExpectedWixMessages.Add(new WixMessage(300, "Illegal characters in path 'Setup|*.exe'. Ensure you provided a valid path to the file.", Message.MessageTypeEnum.Error));
            light.ExpectedExitCode = 300;
            light.Run();
        }

        [TestMethod]
        [Description("SourceFile can be defined as relative to the current working directory")]
        [Priority(2)]
        public void UXRelativeSourceFilePath()
        {
            string sourceFile = Path.Combine(UXTests.TestDataDirectory, @"UxRelativeSourceFilePath\Product.wxs");
            string testFile = Path.Combine(BundleTests.BundleSharedFilesDirectory, @"Bootstrapper.exe");
            string outputDirectory = this.TestDirectory;

            // Copy a file to the current directory. This file is used to verify relative paths in source files.
            File.Copy(testFile, Path.Combine(outputDirectory, "Bootstrapper.exe"), true);

            // build the bootstrapper
            string bootstrapper = Builder.BuildBundlePackage(outputDirectory, sourceFile);

            // verify the ParameterInfo and burnManifest has the correct information 
            UXTests.VerifyUXPayloadInformation(outputDirectory, testFile, "Bootstrapper.exe");
        }

        [TestMethod]
        [Description("SourceFile can be defined as a UNC path.")]
        [Priority(3)]
        [Ignore]
        public void UxUNCSourceFile()
        {
        }

        [TestMethod]
        [Description("Nonexistent SourceFile produces an error.")]
        [Priority(3)]
        public void UXNonexistentSourceFilePath()
        {
            string candleOutput = Candle.Compile(Path.Combine(UXTests.TestDataDirectory, @"UxNonexistentSourceFilePath\Product.wxs"));

            Light light = new Light();
            light.ObjectFiles.Add(candleOutput);
            light.OutputFile = "setup.exe";
            light.ExpectedWixMessages.Add(new WixMessage(103,"The system cannot find the file 'NonExistentFileBootstrapper.exe' with type ''.",Message.MessageTypeEnum.Error));
            light.ExpectedExitCode = 103;
            light.Run();
        }

        [TestMethod]
        [Description("SourceFile attribute is not defined.")]
        [Priority(3)]
        public void UxMissingSourceFileAttribute()
        {
            Candle candle = new Candle();
            candle.SourceFiles.Add(Path.Combine(UXTests.TestDataDirectory, @"UxMissingSourceFileAttribute\Product.wxs"));
            candle.ExpectedWixMessages.Add (new WixMessage(63, "A UX element must have at least one child element of type Payload.",Message.MessageTypeEnum.Error));
            candle.ExpectedExitCode = 63;
            candle.Run();
        }

        [TestMethod]
        [Description("SourceFile attribute can not be empty.")]
        [Priority(3)]
        public void UxEmptySourceFileAttribute()
        {
            Candle candle = new Candle();
            candle.SourceFiles.Add(Path.Combine(UXTests.TestDataDirectory, @"UxEmptySourceFileAttribute\Product.wxs"));
            candle.ExpectedWixMessages.Add(new WixMessage(6, "The UX/@SourceFile attribute's value cannot be an empty string.  If you want the value to be null or empty, simply remove the entire attribute.", Message.MessageTypeEnum.Error));
            candle.ExpectedExitCode = 6;
            candle.Run();
        }


        [TestMethod]
        [Description("Verify output of a UX element with 1 Payload child elements")]
        [Priority(2)]
        public void UXPayloadChild()
        {
            string sourceFile = Path.Combine(UXTests.TestDataDirectory, @"UXPayloadChild\Product.wxs");
            string payloadFile = Path.Combine(BundleTests.BundleSharedFilesDirectory, @"UXPayload\PayloadFile1.txt");
            string outputDirectory = this.TestDirectory;

            // build the bootstrapper
            string bootstrapper = Builder.BuildBundlePackage(outputDirectory, sourceFile);

            // verify the ParameterInfo and burnManifest has the correct information 
            UXTests.VerifyUXPayloadInformation(outputDirectory, payloadFile, "PayloadFile1.txt", true);
        }

        [TestMethod]
        [Description("Verify output of a UX element with 1 Payload child elements")]
        [Priority(2)]
        public void UXPayloadGroupRefChild()
        {
            string sourceFile = Path.Combine(UXTests.TestDataDirectory, @"UXPayloadGroupRefChild\Product.wxs");
            string payloadFile1 = Path.Combine(BundleTests.BundleSharedFilesDirectory, @"UXPayload\PayloadFile1.txt");
            string payloadFile2 = Path.Combine(BundleTests.BundleSharedFilesDirectory, @"UXPayload\PayloadFile2.txt");
            string outputDirectory = this.TestDirectory;

            // build the bootstrapper
            string bootstrapper = Builder.BuildBundlePackage(outputDirectory, sourceFile);

            // verify the ParameterInfo and burnManifest has the correct information 
            UXTests.VerifyUXPayloadInformation(outputDirectory, payloadFile1, "PayloadFile1.txt", true);
            UXTests.VerifyUXPayloadInformation(outputDirectory, payloadFile2, "PayloadFile2.txt", false);
        }

        [TestMethod]
        [Description("50 Payload child elements.")]
        [Priority(3)]
        [Ignore]
        public void UxMultiplePayloadChildren()
        {
        }

        [TestMethod]
        [Description("Verify that build fails if two sibling Payloads are the same file")]
        [Priority(3)]
        public void UXDuplicatePayloads()
        {
            string candleOutput = Candle.Compile(Path.Combine(UXTests.TestDataDirectory, @"UXDuplicatePayloads\Product.wxs"));

            Light light = new Light();
            light.ObjectFiles.Add(candleOutput);
            light.OutputFile = "setup.exe";
            light.ExpectedWixMessages.Add(new WixMessage(91, Message.MessageTypeEnum.Error));
            light.ExpectedWixMessages.Add(new WixMessage(92, Message.MessageTypeEnum.Error));
            light.ExpectedExitCode = 92;
            light.Run();
        }

        [TestMethod]
        [Description("Verify that build fails if two sibling PayloadGroups contain the same file")]
        [Priority(3)]
        public void UXDuplicatePayloadInPayloadGroups()
        {
            string candleOutput = Candle.Compile(Path.Combine(UXTests.TestDataDirectory, @"UXDuplicatePayloadInPayloadGroups\Product.wxs"));

            Light light = new Light();
            light.ObjectFiles.Add(candleOutput);
            light.OutputFile = "setup.exe";
            light.ExpectedWixMessages.Add(new WixMessage(91, Message.MessageTypeEnum.Error));
            light.ExpectedWixMessages.Add(new WixMessage(92, Message.MessageTypeEnum.Error));
            light.ExpectedExitCode = 92;
            light.Run();
        }

        [TestMethod]
        [Description("Verify that build fails if two sibling PayloadGroupRefs reference the same PayloadGroup")]
        [Priority(3)]
        public void UXDuplicatePayloadGroupRefs()
        {
            string candleOutput = Candle.Compile(Path.Combine(UXTests.TestDataDirectory, @"UXDuplicatePayloadGroupRefs\Product.wxs"));

            Light light = new Light();
            light.ObjectFiles.Add(candleOutput);
            light.OutputFile = "setup.exe";
            light.ExpectedWixMessages.Add(new WixMessage(343, Message.MessageTypeEnum.Error));
            light.IgnoreWixMessageOrder = true;
            light.ExpectedExitCode = 343;
            light.Run();
        }

        #region Verification Methods

        /// <summary>
        /// Verifies UX Payload file information in ParameterInfo.xml and Burn_Manifest.xml
        /// </summary>
        /// <param name="embededResourcesDirectoryPath">Output folder where all the embeded resources are.</param>
        /// <param name="acctualFilePath">Path to the acctual file that was packed in the cab.</param>
        /// <param name="expectedFileName">Expected file name of the file.</param>
        public static void VerifyUXPayloadInformation(string embededResourcesDirectoryPath, string acctualFilePath, string expectedFileName)
        {
            UXTests.VerifyUXPayloadInformation(embededResourcesDirectoryPath, acctualFilePath, expectedFileName, true);
        }

        /// <summary>
        /// Verifies UX Payload file information in ParameterInfo.xml and Burn_Manifest.xml
        /// </summary>
        /// <param name="embededResourcesDirectoryPath">Output folder where all the embeded resources are.</param>
        /// <param name="acctualFilePath">Path to the acctual file that was packed in the cab.</param>
        /// <param name="expectedFileName">Expected file name of the file.</param>
        /// <param name="verifyFileIsPrimaryPayload">Check if this file is the primary payload for the UX.</param>
        public static void VerifyUXPayloadInformation(string embededResourcesDirectoryPath, string acctualFilePath, string expectedFileName, bool verifyFileIsPrimaryPayload /*, string expectedDownloadURL*/)
        {
            string expectedFileSize = new FileInfo(acctualFilePath).Length.ToString();
            string expectedHash = FileVerifier.ComputeFileSHA1Hash(acctualFilePath);

            // verify the ParameterInfo has the correct information 
            string parameterInfoXPath = string.Format(@"//pi:Ux/pi:Resource[@FilePath='{0}']", expectedFileName);
            XmlNodeList parameterInfoNodes = BundleTests.QueryParameterInfo(embededResourcesDirectoryPath, parameterInfoXPath);
            Assert.AreEqual(1, parameterInfoNodes.Count, "No UX payload with the name: '{0}' was found in ParameterInfo.xml.", expectedFileName);
            BundleTests.VerifyAttributeValue(parameterInfoNodes[0], "FileSize", expectedFileSize);
            BundleTests.VerifyAttributeValue(parameterInfoNodes[0], "Sha1Hash", expectedHash);

            // verify the file is the primary payload for the UX
            if (verifyFileIsPrimaryPayload)
            {
                string parameterInfoFileId = parameterInfoNodes[0].Attributes["Id"].Value;
                string parameterInfoPrimaryPayloadXPath = string.Format(@"//pi:Ux[@UxDllPayloadId='{0}']", parameterInfoFileId);
                XmlNodeList parameterInfoPrimaryPayloadNodes = BundleTests.QueryParameterInfo(embededResourcesDirectoryPath, parameterInfoPrimaryPayloadXPath);
                Assert.AreEqual(1, parameterInfoPrimaryPayloadNodes.Count, "The primary UX Payload in ParameterInfo.xml is not '{0}'.", expectedFileName);
            }

            // verify the Burn_Manifest has the correct information 
            string burnManifestXPath = string.Format(@"//burn:UX[@FileName='{0}'] | //burn:UX/burn:Resource[@FileName='{0}'] ", expectedFileName);
            XmlNodeList burnManifestNodes = BundleTests.QueryBurnManifest(embededResourcesDirectoryPath, burnManifestXPath);
            Assert.AreEqual(1, burnManifestNodes.Count, "No UX payload with the name: '{0}' was found in Burn_Manifest.xml.", expectedFileName);
            BundleTests.VerifyAttributeValue(burnManifestNodes[0], "FileSize", expectedFileSize);
            BundleTests.VerifyAttributeValue(burnManifestNodes[0], "SHA1", expectedHash);

            // verify the file is the primary payload for the UX
            if (verifyFileIsPrimaryPayload)
            {
                string burnManifestPrimaryPayloadXPath = string.Format(@"//burn:UX[@FileName='{0}']", expectedFileName);
                XmlNodeList burnManifestPrimaryPayloadNodes = BundleTests.QueryBurnManifest(embededResourcesDirectoryPath, burnManifestPrimaryPayloadXPath);
                Assert.AreEqual(1, burnManifestPrimaryPayloadNodes.Count, "The primary UX Payload in Burn_Manifest.xml is not '{0}'.", expectedFileName);
            }

            // Verify embededId
            Assert.AreEqual(parameterInfoNodes[0].Attributes["SourcePath"].Value, burnManifestNodes[0].Attributes["EmbeddedId"].Value, "EmbededId is diffrent in ParameterInfo.xml and burn_manifest.xml. They are expected to be the same.");

            // verify the correct file is added to the ux cab
            string uxCabFilePath = Path.Combine(embededResourcesDirectoryPath, BundleTests.uxCabFileName);
            CabInfo uxCab = new CabInfo(uxCabFilePath);
            string extractedTestFile = Builder.GetUniqueFileName();
            uxCab.UnpackFile(parameterInfoNodes[0].Attributes["SourcePath"].Value, extractedTestFile);
            FileVerifier.VerifyFilesAreIdentical(acctualFilePath, extractedTestFile);
        }

        #endregion
    }
}
