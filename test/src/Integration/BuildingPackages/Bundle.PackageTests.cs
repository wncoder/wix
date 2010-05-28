//-----------------------------------------------------------------------
// <copyright file="Bundle.PackageTests.cs" company="Microsoft">
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
//     Tests for Bundle *Package elements (MsiPackage, MspPackage, MsuPackage and ExePackage)
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
    using Microsoft.Tools.WindowsInstallerXml.Test.Verifiers;

    /// <summary>
    /// Tests for Bundle *Package elements
    /// </summary>
    [TestClass]
    public class PackageTests : BundleTests
    {
        private static readonly string TestDataDirectory = Environment.ExpandEnvironmentVariables(@"%WIX_ROOT%\test\data\Integration\BuildingPackages\Bundle\PackageTests");

        [TestMethod]
        [Description("Package SourceFile is required.")]
        [Priority(3)]
        // bug https://sourceforge.net/tracker/?func=detail&aid=2981298&group_id=105970&atid=642714
        public void PackageSourceFileMissing()
        {
            Candle candle = new Candle();
            candle.SourceFiles.Add(Path.Combine(PackageTests.TestDataDirectory, @"PackageSourceFileMissing\Product.wxs"));
            candle.ExpectedWixMessages.Add(new WixMessage(10, "The MsiPackage/@SourceFile attribute was not found; it is required.", Message.MessageTypeEnum.Error));
            candle.ExpectedWixMessages.Add(new WixMessage(10, "The MspPackage/@SourceFile attribute was not found; it is required.", Message.MessageTypeEnum.Error));
            candle.ExpectedWixMessages.Add(new WixMessage(10, "The MsuPackage/@SourceFile attribute was not found; it is required.", Message.MessageTypeEnum.Error));
            candle.ExpectedWixMessages.Add(new WixMessage(10, "The ExePackage/@SourceFile attribute was not found; it is required.", Message.MessageTypeEnum.Error));
            candle.ExpectedExitCode = 10;
            candle.Run();
        }

        [TestMethod]
        [Description("Package SourceFile is empty.")]
        [Priority(3)]
        public void PackageEmptySourceFile()
        {
            Candle candle = new Candle();
            candle.SourceFiles.Add(Path.Combine(PackageTests.TestDataDirectory, @"PackageEmptySourceFile\Product.wxs"));
            candle.ExpectedWixMessages.Add(new WixMessage(6, "The MsiPackage/@SourceFile attribute's value cannot be an empty string.  If you want the value to be null or empty, simply remove the entire attribute.", Message.MessageTypeEnum.Error));
            candle.ExpectedWixMessages.Add(new WixMessage(6, "The MspPackage/@SourceFile attribute's value cannot be an empty string.  If you want the value to be null or empty, simply remove the entire attribute.", Message.MessageTypeEnum.Error));
            candle.ExpectedWixMessages.Add(new WixMessage(6, "The MsuPackage/@SourceFile attribute's value cannot be an empty string.  If you want the value to be null or empty, simply remove the entire attribute.", Message.MessageTypeEnum.Error));
            candle.ExpectedWixMessages.Add(new WixMessage(6, "The ExePackage/@SourceFile attribute's value cannot be an empty string.  If you want the value to be null or empty, simply remove the entire attribute.", Message.MessageTypeEnum.Error));
            candle.ExpectedExitCode = 6;
            candle.Run();
        }

        [TestMethod]
        [Description("Package @SourceFile contains an invalid path.")]
        [Priority(3)]
        public void PackageInvalidSourceFile()
        {
            string sourceFile = Path.Combine(PackageTests.TestDataDirectory, @"PackageInvalidSourceFile\Product.wxs");
            string candleOutput = Candle.Compile(sourceFile);

            Light light = new Light();
            light.ObjectFiles.Add(candleOutput);
            light.OutputFile = "Setup.exe";
            light.ExpectedWixMessages.Add(new WixMessage(300, @"Illegal characters in path 'MsiPackage|*?.msi'. Ensure you provided a valid path to the file.", Message.MessageTypeEnum.Error));
            light.ExpectedExitCode = 300;
            light.Run();
        }

        [TestMethod]
        [Description("Package @SourceFile target does not exist on disk.")]
        [Priority(3)]
        [Ignore]
        // bug  https://sourceforge.net/tracker/?func=detail&aid=2980318&group_id=105970&atid=642714
        public void PackageNonexistingSourceFile()
        {
            string sourceFile = Path.Combine(PackageTests.TestDataDirectory, @"PackageNonexistingSourceFile\Product.wxs");
            string candleOutput = Candle.Compile(sourceFile);

            Light light = new Light();
            light.ObjectFiles.Add(candleOutput);
            light.OutputFile = "Setup.exe";
            //light.ExpectedWixMessages.Add(new WixMessage(1, @"This installation package could not be opened. Verify that the package exists and that you can access it, or contact the application vendor to verify that this is a valid Windows Installer package.", Message.MessageTypeEnum.Error));
            //light.ExpectedExitCode = 1;
            light.Run();
        }

        [TestMethod]
        [Description("Package @Name contains an invalid path.")]
        [Priority(3)]
        public void PackageInvalidName()
        {
            Candle candle = new Candle();
            candle.SourceFiles.Add(Path.Combine(PackageTests.TestDataDirectory, @"PackageInvalidName\Product.wxs"));
            candle.ExpectedWixMessages.Add(new WixMessage(27, "The MsiPackage/@Name attribute's value, 'MsiPackage|*?.msi', is not a valid long name because it contains illegal characters.  Legal long names contain no more than 260 characters and must contain at least one non-period character.  Any character except for the follow may be used: \\ ? | > < : / * \".", Message.MessageTypeEnum.Error));
            candle.ExpectedWixMessages.Add(new WixMessage(27, "The MspPackage/@Name attribute's value, 'MspPackage|*?.msp', is not a valid long name because it contains illegal characters.  Legal long names contain no more than 260 characters and must contain at least one non-period character.  Any character except for the follow may be used: \\ ? | > < : / * \".", Message.MessageTypeEnum.Error));
            candle.ExpectedWixMessages.Add(new WixMessage(27, "The MsuPackage/@Name attribute's value, 'MsuPackage|*?.msu', is not a valid long name because it contains illegal characters.  Legal long names contain no more than 260 characters and must contain at least one non-period character.  Any character except for the follow may be used: \\ ? | > < : / * \".", Message.MessageTypeEnum.Error));
            candle.ExpectedWixMessages.Add(new WixMessage(27, "The ExePackage/@Name attribute's value, 'ExePackage|*?.exe', is not a valid long name because it contains illegal characters.  Legal long names contain no more than 260 characters and must contain at least one non-period character.  Any character except for the follow may be used: \\ ? | > < : / * \".", Message.MessageTypeEnum.Error));
            candle.ExpectedExitCode = 27;
            candle.Run();
        }

        [TestMethod]
        [Description("After contains an Id of a missing Package.")]
        [Priority(3)]
        public void PackageAfterUndefinedPackage()
        {
            string sourceFile = Path.Combine(PackageTests.TestDataDirectory, @"PackageAfterUndefinedPackage\Product.wxs");
            string candleOutput = Candle.Compile(sourceFile);

            Light light = new Light();
            light.ObjectFiles.Add(candleOutput);
            light.OutputFile = "Setup.exe";
            light.ExpectedWixMessages.Add(new WixMessage(344, "An expected identifier ('UndefinedMsiPackage', of type 'Package') was not found.", Message.MessageTypeEnum.Error));  
            light.ExpectedWixMessages.Add(new WixMessage(344, "An expected identifier ('UndefinedMspPackage', of type 'Package') was not found.", Message.MessageTypeEnum.Error));  
            light.ExpectedWixMessages.Add(new WixMessage(344, "An expected identifier ('UndefinedMsuPackage', of type 'Package') was not found.", Message.MessageTypeEnum.Error));  
            light.ExpectedWixMessages.Add(new WixMessage(344, "An expected identifier ('UndefinedExePackage', of type 'Package') was not found.", Message.MessageTypeEnum.Error));  
            light.ExpectedExitCode = 344;
            light.Run();
        }

        [TestMethod]
        [Description("After contains an Id of a Package after this Package.")]
        [Priority(3)]
        public void PackageRecursiveAfter()
        {
            string sourceFile = Path.Combine(PackageTests.TestDataDirectory, @"PackageRecursiveAfter\Product.wxs");
            string candleOutput = Candle.Compile(sourceFile);

            Light light = new Light();
            light.ObjectFiles.Add(candleOutput);
            light.OutputFile = "Setup.exe";
            light.ExpectedWixMessages.Add(new WixMessage(343, "A circular reference of ordering dependencies was detected. The infinite loop includes: Package:MsuPackage -> Package:MspPackage -> Package:MsuPackage. Ordering dependency references must form a directed acyclic graph.", Message.MessageTypeEnum.Error));
            light.ExpectedWixMessages.Add(new WixMessage(343, "A circular reference of ordering dependencies was detected. The infinite loop includes: Package:MspPackage -> Package:ExePackage -> Package:MspPackage. Ordering dependency references must form a directed acyclic graph.", Message.MessageTypeEnum.Error));
            light.ExpectedWixMessages.Add(new WixMessage(343, "A circular reference of ordering dependencies was detected. The infinite loop includes: Package:MspPackage -> Package:MsiPackage -> Package:MspPackage. Ordering dependency references must form a directed acyclic graph.", Message.MessageTypeEnum.Error));
            light.ExpectedWixMessages.Add(new WixMessage(343, "A circular reference of ordering dependencies was detected. The infinite loop includes: Package:MsiPackage -> Package:MsuPackage -> Package:MsiPackage. Ordering dependency references must form a directed acyclic graph.", Message.MessageTypeEnum.Error));
            light.ExpectedExitCode = 343;
            light.Run();
        }

        [TestMethod]
        [Description("Package @Vital contains an invalid value.")]
        [Priority(3)]
        public void PackageInvalidVital()
        {
            Candle candle = new Candle();
            candle.SourceFiles.Add(Path.Combine(PackageTests.TestDataDirectory, @"PackageInvalidVital\Product.wxs"));
            candle.ExpectedWixMessages.Add(new WixMessage(15, "The MsiPackage/@Vital attribute's value, 'true', is not a legal yes/no value.  The only legal values are 'no' and 'yes'.", Message.MessageTypeEnum.Error));
            candle.ExpectedWixMessages.Add(new WixMessage(15, "The MspPackage/@Vital attribute's value, 'false', is not a legal yes/no value.  The only legal values are 'no' and 'yes'.", Message.MessageTypeEnum.Error));
            candle.ExpectedWixMessages.Add(new WixMessage(15, "The MsuPackage/@Vital attribute's value, 'vital', is not a legal yes/no value.  The only legal values are 'no' and 'yes'.", Message.MessageTypeEnum.Error));
            candle.ExpectedWixMessages.Add(new WixMessage(15, "The ExePackage/@Vital attribute's value, 'TRUE', is not a legal yes/no value.  The only legal values are 'no' and 'yes'.", Message.MessageTypeEnum.Error));
            candle.ExpectedExitCode = 15;
            candle.Run();
        }

        [TestMethod]
        [Description("Package @Cache contains an invalid value.")]
        [Priority(3)]
        public void PackageInvalidCache()
        {
            Candle candle = new Candle();
            candle.SourceFiles.Add(Path.Combine(PackageTests.TestDataDirectory, @"PackageInvalidCache\Product.wxs"));
            candle.ExpectedWixMessages.Add(new WixMessage(15, "The MsiPackage/@Cache attribute's value, 'true', is not a legal yes/no value.  The only legal values are 'no' and 'yes'.", Message.MessageTypeEnum.Error));
            candle.ExpectedWixMessages.Add(new WixMessage(15, "The MspPackage/@Cache attribute's value, 'false', is not a legal yes/no value.  The only legal values are 'no' and 'yes'.", Message.MessageTypeEnum.Error));
            candle.ExpectedWixMessages.Add(new WixMessage(15, "The MsuPackage/@Cache attribute's value, 'Cache', is not a legal yes/no value.  The only legal values are 'no' and 'yes'.", Message.MessageTypeEnum.Error));
            candle.ExpectedWixMessages.Add(new WixMessage(15, "The ExePackage/@Cache attribute's value, 'TRUE', is not a legal yes/no value.  The only legal values are 'no' and 'yes'.", Message.MessageTypeEnum.Error));
            candle.ExpectedExitCode = 15;
            candle.Run();
        }
        
        [TestMethod]
        [Description("Valid Package.")]
        [Priority(2)]
        // bug# https://sourceforge.net/tracker/?func=detail&aid=2980325&group_id=105970&atid=642714
        public void ValidPackage()
        {
            string sourceFile = Path.Combine(PackageTests.TestDataDirectory, @"ValidPackage\Product.wxs");
            string outputDirectory = this.TestDirectory;

            // build the bootstrapper
            string bootstrapper = Builder.BuildBundlePackage(outputDirectory, sourceFile);

            // verify the ParameterInfo and burnManifest has the correct information 
            PackageTests.VerifyMspPackageInformation(outputDirectory, "MspPackage1.msp", "MspPackage1", null, BundleTests.MspPackagePatchCode, true, false, "MspPackage1CacheId", null, BundleTests.MspPackageFile);
            PackageTests.VerifyMsuPackageInformation(outputDirectory, "MsuPackage2.msu", "MsuPackage2", null, true, true, @"ftp://192.168.0.1/testPayload.exe", BundleTests.MsuPackageFile);
            PackageTests.VerifyMsiPackageInformation(outputDirectory, "MsiPackage3.msi", "MsiPackage3", null, BundleTests.MsiPackageProductCode, false, true, string.Format("{0}v0.1.0.0", BundleTests.MsiPackageProductCode), @"http://go.microsoft.com/fwlink/?linkid=164202", BundleTests.MsiPackageFile);
            PackageTests.VerifyExePackageInformation(outputDirectory, "ExePackage5.exe", "ExePackage5", "x!=y", true, true, @"\q", @"\q \r \t", @"\anotherargument -t", @"\\wixbuild\releases\wix\", BundleTests.MsiPackageFile); // using the msi package for an exe

            PackageTests.VerifyMsiPackageOrder(outputDirectory, "ExePackage5.exe", "MspPackage1.msp", "MsuPackage2.msi", "MsiPackage3.msi");
        }

        [TestMethod]
        [Description("Package can have payload children.")]
        [Priority(2)]
        // bug https://sourceforge.net/tracker/?func=detail&aid=2987928&group_id=105970&atid=642714
        public void PackagePayloads()
        {
            string sourceFile = Path.Combine(PackageTests.TestDataDirectory, @"PackagePayloads\Product.wxs");
            string outputDirectory = this.TestDirectory;
            string PayloadFile1 = Path.Combine(BundleTests.BundleSharedFilesDirectory, @"UXPayload\PayloadFile1.txt");
            string PayloadFile2 = Path.Combine(BundleTests.BundleSharedFilesDirectory, @"UXPayload\PayloadFile2.txt");
            string PayloadFile3 = Path.Combine(BundleTests.BundleSharedFilesDirectory, @"UXPayload\PayloadFile3.txt");
            string PayloadFile4 = Path.Combine(BundleTests.BundleSharedFilesDirectory, @"UXPayload\PayloadFile4.txt");
            string PayloadFile5 = Path.Combine(BundleTests.BundleSharedFilesDirectory, @"UXPayload\PayloadFile5.txt");

            // build the bootstrapper
            string bootstrapper = Builder.BuildBundlePackage(outputDirectory, sourceFile);

            // verify the ParameterInfo and burnManifest has the correct information 
            PackageTests.VerifyMsiPackageInformation(outputDirectory, "MsiPackage1.msi", "MsiPackage1", null, BundleTests.MsiPackageProductCode, true, true, string.Format("{0}v0.1.0.0", BundleTests.MsiPackageProductCode), @"http://go.microsoft.com/fwlink/?linkid=164202", BundleTests.MsiPackageFile);
            PackageTests.VerifyMspPackageInformation(outputDirectory, "MspPackage2.msp", "MspPackage2", null, BundleTests.MspPackagePatchCode, true, true, BundleTests.MspPackagePatchCode, null, BundleTests.MspPackageFile);
            PackageTests.VerifyMsuPackageInformation(outputDirectory, "MsuPackage3.msu", "MsuPackage3", null, true, true, @"ftp://192.168.0.1/testPayload.exe", BundleTests.MsuPackageFile);
            PackageTests.VerifyExePackageInformation(outputDirectory, "ExePackage4.exe", "ExePackage4", null, true, true, string.Empty, string.Empty, string.Empty, @"\\wixbuild\releases\wix\", BundleTests.ExePackageFile);

            PackageTests.VerifyPackagePayloadInformation(outputDirectory, PackageTests.PackageType.MSI, "MsiPackage1.msi", "PayloadFile1.txt", null, PayloadFile1);
            PackageTests.VerifyPackagePayloadInformation(outputDirectory, PackageTests.PackageType.MSP, "MspPackage2.msp", "PayloadFile2.txt", null, PayloadFile2);
            PackageTests.VerifyPackagePayloadInformation(outputDirectory, PackageTests.PackageType.MSU, "MsuPackage3.msu", "PayloadFile3.txt", null, PayloadFile3);
            PackageTests.VerifyPackagePayloadInformation(outputDirectory, PackageTests.PackageType.MSU, "MsuPackage3.msu", "PayloadFile4.txt", "http://go.microsoft.com/fwlink/?linkid=164202", PayloadFile4);
            PackageTests.VerifyPackagePayloadInformation(outputDirectory, PackageTests.PackageType.EXE, "ExePackage4.exe", "PayloadFile5.txt", null, PayloadFile5);
        }

        /* MsiPackage specific tests */

        [TestMethod]
        [Description("MsiPackage @SourceFile target is not a valid .msi file.")]
        [Priority(3)]
        public void MsiPackageInvalidMsi()
        {
            string sourceFile = Path.Combine(PackageTests.TestDataDirectory, @"MsiPackageInvalidMsi\Product.wxs");
            string candleOutput = Candle.Compile(sourceFile);

            Light light = new Light();
            light.ObjectFiles.Add(candleOutput);
            light.OutputFile = "Setup.exe";
            light.ExpectedWixMessages.Add(new WixMessage(1, @"This installation package could not be opened. Contact the application vendor to verify that this is a valid Windows Installer package.", Message.MessageTypeEnum.Error));
            light.ExpectedExitCode = 1;
            light.Run();
        }

        [TestMethod]
        [Description("MsiPackage @SourceFile target is in use by another application.")]
        [Priority(3)]
        public void MsiPackageInuse()
        {
            // acquire a loc on the file
            FileStream msiPackage = File.Open(BundleTests.MsiPackageFile, FileMode.Open, FileAccess.ReadWrite);

            string sourceFile = Path.Combine(PackageTests.TestDataDirectory, @"MsiPackageInuse\Product.wxs");
            string candleOutput = Candle.Compile(sourceFile);

            Light light = new Light();
            light.ObjectFiles.Add(candleOutput);
            light.OutputFile = "Setup.exe";
            light.ExpectedWixMessages.Add(new WixMessage(1, @"This installation package could not be opened. Contact the application vendor to verify that this is a valid Windows Installer package.", Message.MessageTypeEnum.Error));
            light.ExpectedExitCode = 1;
            light.Run();

            // release lock
            msiPackage.Close();
        }

        [TestMethod]
        [Description("MsiProperty Name is required.")]
        [Priority(3)]
        public void MsiPropertyNameMissing()
        {
            Candle candle = new Candle();
            candle.SourceFiles.Add(Path.Combine(PackageTests.TestDataDirectory, @"MsiPropertyNameMissing\Product.wxs"));
            candle.ExpectedWixMessages.Add(new WixMessage(10, "The MsiProperty/@Name attribute was not found; it is required.", Message.MessageTypeEnum.Error));
            candle.ExpectedExitCode = 10;
            candle.Run();
        }

        [TestMethod]
        [Description("MsiProperty Value is required.")]
        [Priority(3)]
        public void MsiPropertyValueMissing()
        {
            Candle candle = new Candle();
            candle.SourceFiles.Add(Path.Combine(PackageTests.TestDataDirectory, @"MsiPropertyValueMissing\Product.wxs"));
            candle.ExpectedWixMessages.Add(new WixMessage(10, "The MsiProperty/@Value attribute was not found; it is required.", Message.MessageTypeEnum.Error));
            candle.ExpectedExitCode = 10;
            candle.Run();
        }

        [TestMethod]
        [Description("MsiProperty can not be redefined.")]
        [Priority(3)]
        public void DuplicateMsiProperty()
        {
            string sourceFile = Path.Combine(PackageTests.TestDataDirectory, @"DuplicateMsiProperty\Product.wxs");
            string candleOutput = Candle.Compile(sourceFile);

            Light light = new Light();
            light.ObjectFiles.Add(candleOutput);
            light.OutputFile = "Setup.exe";
            light.ExpectedWixMessages.Add(new WixMessage(91, @"Duplicate symbol 'MsiProperty:MsiProperty1' found.", Message.MessageTypeEnum.Error));
            light.ExpectedWixMessages.Add(new WixMessage(92, @"Location of symbol related to previous error.", Message.MessageTypeEnum.Error));
            light.ExpectedExitCode = 92;
            light.Run();
        }

        [TestMethod]
        [Description("Valid MsiPackage MsiProperty child elements.")]
        [Priority(2)]
        //bug https://sourceforge.net/tracker/?func=detail&aid=2987866&group_id=105970&atid=642714
        public void ValidMsiProperty()
        {
            string sourceFile = Path.Combine(PackageTests.TestDataDirectory, @"ValidMsiProperty\Product.wxs");
            string outputDirectory = this.TestDirectory;

            // build the bootstrapper
            string bootstrapper = Builder.BuildBundlePackage(outputDirectory, sourceFile);

            // verify the ParameterInfo and burnManifest has the correct information 
            PackageTests.VerifyMsiPropertyInformation(outputDirectory, "MsiPackage1.msi", "MsiProperty1", "x!=y | z==y");
            PackageTests.VerifyMsiPropertyInformation(outputDirectory, "MsiPackage1.msi", "MsiProperty2", "http://www.microsoft.com");
            PackageTests.VerifyMsiPropertyInformation(outputDirectory, "MsiPackage1.msi", "MsiProperty3", "23.00");
            PackageTests.VerifyMsiPropertyInformation(outputDirectory, "MsiPackage2.msi", "MsiProperty1", "x!=y | z==y");
        }

        /* MspPackage specific tests */

        [TestMethod]
        [Description("MspPackage @SourceFile target is not a valid .msi file.")]
        [Priority(3)]
        public void MspPackageInvalidMsp()
        {
            string sourceFile = Path.Combine(PackageTests.TestDataDirectory, @"MspPackageInvalidMsp\Product.wxs");
            string candleOutput = Candle.Compile(sourceFile);

            Light light = new Light();
            light.ObjectFiles.Add(candleOutput);
            light.OutputFile = "Setup.exe";
            light.ExpectedWixMessages.Add(new WixMessage(1, @"This installation package could not be opened. Contact the application vendor to verify that this is a valid Windows Installer package.", Message.MessageTypeEnum.Error));
            light.ExpectedExitCode = 1;
            light.Run();
        }

        #region Verification Methods

        /// <summary>
        /// Supported package types
        /// </summary>
        public enum PackageType
        {
            /// <summary>
            /// Msi Package
            /// </summary>
            MSI,

            /// <summary>
            /// Msp Package
            /// </summary>
            MSP,

            /// <summary>
            /// Msu Package
            /// </summary>
            MSU,

            /// <summary>
            /// Exe Package
            /// </summary>
            EXE,
        };

        /// <summary>
        /// Verifies MsiPackage information in ParameterInfo.xml and Burn_Manifest.xml.
        /// </summary>
        /// <param name="embededResourcesDirectoryPath">Output folder where all the embeded resources are.</param> 
        /// <param name="expectedPackageName">Package name; this is the attribute used to locate the package.</param>
        /// <param name="expectedId">Expected Package @Id value.</param>
        /// <param name="expectedInstallCondition">Expected Package @InstallCondition value.</param>
        /// <param name="expectedProductCode">Expected Package @ProductCode value.</param>
        /// <param name="expecteVital">Package is viatal or not.</param>
        /// <param name="expectedCache">Expected Package @Cache value.</param>
        /// <param name="expectedCacheId">Expected Package @CacheId value.</param>
        /// <param name="acctualFilePath">Path to the acctual file for comparison.</param>
        /// <param name="expectedDownloadURL">Expected Package @DownloadURL value.</param>
        public static void VerifyMsiPackageInformation(string embededResourcesDirectoryPath, string expectedPackageName, string expectedId, string expectedInstallCondition, string expectedProductCode, bool expecteVital, bool expectedCache, string expectedCacheId, string expectedDownloadURL,string acctualFilePath)
        {
            VerifyPackageInformation(embededResourcesDirectoryPath, expectedPackageName, PackageType.MSI, expectedId, expectedInstallCondition, expectedProductCode, expecteVital, expectedCache, expectedCacheId, null, null, null,expectedDownloadURL, acctualFilePath);
        }

        /// <summary>
        /// Verifies MspPackage information in ParameterInfo.xml and Burn_Manifest.xml.
        /// </summary>
        /// <param name="embededResourcesDirectoryPath">Output folder where all the embeded resources are.</param> 
        /// <param name="expectedPackageName">Package name; this is the attribute used to locate the package.</param>
        /// <param name="expectedId">Expected Package @Id value.</param>
        /// <param name="expectedInstallCondition">Expected Package @InstallCondition value.</param>
        /// <param name="expectedPatchCode">Expected Package @PatchCode value.</param>
        /// <param name="expecteVital">Package is viatal or not.</param>
        /// <param name="expectedCache">Expected Package @Cache value.</param>
        /// <param name="expectedCacheId">Expected Package @CacheId value.</param>
        /// <param name="acctualFilePath">Path to the acctual file for comparison.</param>
        /// <param name="expectedDownloadURL">Expected Package @DownloadURL value.</param>
        public static void VerifyMspPackageInformation(string embededResourcesDirectoryPath, string expectedPackageName, string expectedId, string expectedInstallCondition, string expectedPatchCode, bool expecteVital, bool expectedCache, string expectedCacheId, string expectedDownloadURL, string acctualFilePath)
        {
            VerifyPackageInformation(embededResourcesDirectoryPath, expectedPackageName, PackageType.MSP, expectedId, expectedInstallCondition, expectedPatchCode, expecteVital, expectedCache, expectedCacheId, null, null, null,expectedDownloadURL, acctualFilePath);
        }

        /// <summary>
        /// Verifies MsuPackage information in ParameterInfo.xml and Burn_Manifest.xml.
        /// </summary>
        /// <param name="embededResourcesDirectoryPath">Output folder where all the embeded resources are.</param> 
        /// <param name="expectedPackageName">Package name; this is the attribute used to locate the package.</param>
        /// <param name="expectedId">Expected Package @Id value.</param>
        /// <param name="expectedInstallCondition">Expected Package @InstallCondition value.</param>
        /// <param name="expecteVital">Package is viatal or not.</param>
        /// <param name="expectedCache">Expected Package @Cache value.</param>
        /// <param name="acctualFilePath">Path to the acctual file for comparison.</param>
        /// <param name="expectedDownloadURL">Expected Package @DownloadURL value.</param>
        public static void VerifyMsuPackageInformation(string embededResourcesDirectoryPath, string expectedPackageName, string expectedId, string expectedInstallCondition, bool expecteVital, bool expectedCache, string expectedDownloadURL, string acctualFilePath)
        {
            VerifyPackageInformation(embededResourcesDirectoryPath, expectedPackageName, PackageType.MSU, expectedId, expectedInstallCondition, null, expecteVital, expectedCache, null, null, null, null, expectedDownloadURL, acctualFilePath);
        }

        /// <summary>
        /// Verifies ExePackage information in ParameterInfo.xml and Burn_Manifest.xml.
        /// </summary>
        /// <param name="embededResourcesDirectoryPath">Output folder where all the embeded resources are.</param> 
        /// <param name="expectedPackageName">Package name; this is the attribute used to locate the package.</param>
        /// <param name="expectedId">Expected Package @Id value.</param>
        /// <param name="expectedInstallCondition">Expected Package @InstallCondition value.</param>
        /// <param name="expecteVital">Package is viatal or not.</param>
        /// <param name="expectedCache">Expected Package @Cache value.</param>
        /// <param name="acctualFilePath">Path to the acctual file for comparison.</param>
        /// <param name="expectedInstallCommmand">Expected @InstallCommand value.</param>
        /// <param name="expectedUninstallCommmand">Expected @UninstallCommand value.</param>
        /// <param name="expectedRepairCommand">Expected @RepairCommand value.</param>
        /// <param name="expectedDownloadURL">Expected Package @DownloadURL value.</param>
        public static void VerifyExePackageInformation(string embededResourcesDirectoryPath, string expectedPackageName, string expectedId, string expectedInstallCondition, bool expecteVital, bool expectedCache, string expectedInstallCommmand, string expectedUninstallCommmand, string expectedRepairCommand, string expectedDownloadURL, string acctualFilePath)
        {
            VerifyPackageInformation(embededResourcesDirectoryPath, expectedPackageName, PackageType.EXE, expectedId, expectedInstallCondition, null, expecteVital, expectedCache, null, expectedInstallCommmand, expectedUninstallCommmand, expectedRepairCommand, expectedDownloadURL, acctualFilePath);
        }

        /// <summary>
        /// Verifies Package information in ParameterInfo.xml and Burn_Manifest.xml.
        /// </summary>
        /// <param name="embededResourcesDirectoryPath">Output folder where all the embeded resources are.</param>
        /// <param name="expectedPackageName">Package name; this is the attribute used to locate the package.</param>
        /// <param name="expectedPackageType">Package type MSI, MSP, MSU or EXE.</param>
        /// <param name="expectedId">Expected Package @Id value.</param>
        /// <param name="expectedInstallCondition">Expected Package @InstallCondition value.</param>
        /// <param name="expectedProductCode">Expected Package @ProductCode value.</param>
        /// <param name="expecteVital">Package is viatal or not.</param>
        /// <param name="expectedCache">Expected Package @Cache value.</param>
        /// <param name="expectedCacheId">Expected Package @CacheId value.</param>
        /// <param name="expectedInstallCommmand">Expected @InstallCommand value.</param>
        /// <param name="expectedUninstallCommmand">Expected @UninstallCommand value.</param>
        /// <param name="expectedRepairCommand">Expected @RepairCommand value.</param>
        /// <param name="expectedDownloadURL">Expected Package @DownloadURL value.</param>
        /// <param name="acctualFilePath">Path to the acctual file for comparison.</param>
        private static void VerifyPackageInformation(string embededResourcesDirectoryPath, string expectedPackageName, PackageType expectedPackageType, string expectedId, 
                                                     string expectedInstallCondition, string expectedProductCode, bool expecteVital, bool expectedCache, string expectedCacheId,
                                                     string expectedInstallCommmand, string expectedUninstallCommmand, string expectedRepairCommand, string expectedDownloadURL, 
                                                     string acctualFilePath)
        {
            string expectedFileSize = new FileInfo(acctualFilePath).Length.ToString();
            string expectedHash = FileVerifier.ComputeFileSHA1Hash(acctualFilePath);
            string expectedProductSize = expectedFileSize;

            // verify the ParameterInfo has the correct information 
            string parameterInfoXPath = string.Format(@"//pi:{0}[@Name='{1}']", GetPackageElementName(expectedPackageType, "ParameterInfo"), expectedPackageName);
            XmlNodeList parameterInfoNodes = BundleTests.QueryParameterInfo(embededResourcesDirectoryPath, parameterInfoXPath);
            Assert.AreEqual(1, parameterInfoNodes.Count, "No MsiPackage with the Name: '{0}' was found in ParameterInfo.xml.", expectedPackageName);
            BundleTests.VerifyAttributeValue(parameterInfoNodes[0], "Id", expectedId);
            BundleTests.VerifyAttributeValue(parameterInfoNodes[0], "InstallCondition", expectedInstallCondition);
            BundleTests.VerifyAttributeValue(parameterInfoNodes[0], "DownloadSize", expectedFileSize);
            BundleTests.VerifyAttributeValue(parameterInfoNodes[0], "InstalledProductSize", expectedProductSize);
            BundleTests.VerifyAttributeValue(parameterInfoNodes[0], "Sha1HashValue", expectedHash);
            BundleTests.VerifyAttributeValue(parameterInfoNodes[0], "Cache", expectedCache ? "true" : "false");
            BundleTests.VerifyAttributeValue(parameterInfoNodes[0], "URL", expectedDownloadURL);

            if (expectedPackageType == PackageType.MSI)
            {
                BundleTests.VerifyAttributeValue(parameterInfoNodes[0], "ProductCode", expectedProductCode);
                BundleTests.VerifyAttributeValue(parameterInfoNodes[0], "CacheId", expectedCacheId);
            }

            if (expectedPackageType == PackageType.MSP)
            {
                BundleTests.VerifyAttributeValue(parameterInfoNodes[0], "PatchCode", expectedProductCode);
                BundleTests.VerifyAttributeValue(parameterInfoNodes[0], "CacheId", expectedCacheId);
            }

            if (expectedPackageType == PackageType.MSU)
            {
                BundleTests.VerifyAttributeValue(parameterInfoNodes[0], "ExeType", "MsuPackage");
            }

            if (expectedPackageType == PackageType.EXE)
            {
                BundleTests.VerifyAttributeValue(parameterInfoNodes[0], "InstallCommandLine", expectedInstallCommmand);
                BundleTests.VerifyAttributeValue(parameterInfoNodes[0], "UninstallCommandLine", expectedUninstallCommmand);
                BundleTests.VerifyAttributeValue(parameterInfoNodes[0], "RepairCommandLine", expectedRepairCommand);
            }

            // verify the Burn_Manifest has the correct information 
            string burnManifestXPath = string.Format(@"//burn:{0}[@FileName='{1}']", GetPackageElementName(expectedPackageType, "BurnManifest"), expectedPackageName);
            XmlNodeList burnManifestNodes = BundleTests.QueryBurnManifest(embededResourcesDirectoryPath, burnManifestXPath);
            Assert.AreEqual(1, burnManifestNodes.Count, "No MsiPackage with the FileName: '{0}' was found in Burn_Manifest.xml.", expectedId);
            BundleTests.VerifyAttributeValue(burnManifestNodes[0], "InstallCondition", expectedInstallCondition);
            BundleTests.VerifyAttributeValue(burnManifestNodes[0], "FileSize", expectedFileSize);
            BundleTests.VerifyAttributeValue(burnManifestNodes[0], "SHA1", expectedHash);
            BundleTests.VerifyAttributeValue(burnManifestNodes[0], "Cache", expectedCache ? "yes" : "no");
            BundleTests.VerifyAttributeValue(burnManifestNodes[0], "DownloadUrl", expectedDownloadURL);

            if (expectedPackageType == PackageType.MSI)
            {
                BundleTests.VerifyAttributeValue(burnManifestNodes[0], "ProductCode", expectedProductCode);
            }

            if (expectedPackageType == PackageType.EXE)
            {
                BundleTests.VerifyAttributeValue(burnManifestNodes[0], "InstallArguments", expectedInstallCommmand);
                BundleTests.VerifyAttributeValue(burnManifestNodes[0], "UninstallArguments", expectedUninstallCommmand);
                BundleTests.VerifyAttributeValue(burnManifestNodes[0], "RepairArguments", expectedRepairCommand);
            }

            // verify the correct file is added to the attached cab
            if (null == burnManifestNodes[0].Attributes["Packaging"] || burnManifestNodes[0].Attributes["Packaging"].Value != "download")
            {
                string attachedCabFilePath = Path.Combine(embededResourcesDirectoryPath, BundleTests.attachedCabFileName);
                CabInfo attachedCab = new CabInfo(attachedCabFilePath);
                string extractedTestFile = Builder.GetUniqueFileName();
                attachedCab.UnpackFile(burnManifestNodes[0].Attributes["EmbeddedId"].Value, extractedTestFile);
                FileVerifier.VerifyFilesAreIdentical(acctualFilePath, extractedTestFile);
            }
        }

        /// <summary>
        /// Verify MsiPackage elements appear in a specific order
        /// </summary>
        /// <param name="embededResourcesDirectoryPath">Output folder where all the embeded resources are.</param>
        /// <param name="packageNames">Names of the MsiPackage elements in order.</param>
        public static void VerifyMsiPackageOrder(string embededResourcesDirectoryPath, params string[] packageNames)
        {
            string parameterInfoXPath = @"//pi:MSI |//pi:MSP |//pi:MSU |//pi:Exe";
            XmlNodeList parameterInfoNodes = BundleTests.QueryParameterInfo(embededResourcesDirectoryPath, parameterInfoXPath);
            BundleTests.VerifyElementOrder(parameterInfoNodes, "Name", packageNames);

            string burnManifestXPath = @"//burn:MsiPackage |//burn:MspPackage |//burn:MsuPackage |//burn:ExePackage";
            XmlNodeList burnManifestNodes = BundleTests.QueryBurnManifest(embededResourcesDirectoryPath, burnManifestXPath);
            BundleTests.VerifyElementOrder(burnManifestNodes, "FileName", packageNames);
        }

        /// <summary>
        /// Verifies MsiProperty information in ParameterInfo.xml and Burn_Manifest.xml
        /// </summary>
        /// <param name="embededResourcesDirectoryPath">Output folder where all the embeded resources are.</param>
        /// <param name="msiPackageName">Package name; this is the attribute used to locate the package.</param>
        /// <param name="expectedPropertyName">Expected MsiProperty @Name value.</param>
        /// <param name="expectedPropertyValue">Expected MsiProperty @Value value.</param>
        public static void VerifyMsiPropertyInformation(string embededResourcesDirectoryPath, string msiPackageName, string expectedPropertyName, string expectedPropertyValue)
        {
            // verify the ParameterInfo has the correct information 
            string parameterInfoXPath = string.Format(@"//pi:{0}[@Name='{1}']", GetPackageElementName(PackageType.MSI, "ParameterInfo"), msiPackageName);
            XmlNodeList parameterInfoNodes = BundleTests.QueryParameterInfo(embededResourcesDirectoryPath, parameterInfoXPath);
            Assert.AreEqual(1, parameterInfoNodes.Count, "No MsiPackage with the Name: '{0}' was found in ParameterInfo.xml.", msiPackageName);
            if (null == parameterInfoNodes[0].Attributes["MSIOptions"])
            {
                Assert.Fail(string.Format("{0} @MSIOptions was defined it was not expected.", GetPackageElementName(PackageType.MSI, "ParameterInfo")));
            }
            else
            {
                string msiOptions = parameterInfoNodes[0].Attributes["MSIOptions"].Value;
                string expectedOption = string.Format("{0}=\"{1}\"", expectedPropertyName,expectedPropertyValue);
                Assert.IsTrue(msiOptions.Contains(expectedOption), string.Format("@MSIOptions value did not contain expected property. Actual:'{0}'. Expected to contain: '{1}'.", msiOptions, expectedOption));
            }

            // verify the Burn_Manifest has the correct information 
            string burnManifestXPath = string.Format(@"//burn:{0}[@FileName='{1}']/burn:MsiProperty[@Id='{2}'] ", GetPackageElementName(PackageType.MSI, "BurnManifest"), msiPackageName, expectedPropertyName);
            XmlNodeList burnManifestNodes = BundleTests.QueryBurnManifest(embededResourcesDirectoryPath, burnManifestXPath);
            Assert.AreEqual(1, burnManifestNodes.Count, "No MsiProperty with the Id: '{0}' was found under MsiPackage: '{1}' in Burn_Manifest.xml.", expectedPropertyName, msiPackageName);
            BundleTests.VerifyAttributeValue(burnManifestNodes[0], "Value", expectedPropertyValue);
        }
        
        /// <summary>
        /// Verify the pacakge Payload information in ParameterInfo.xml and Burn_Manifest.xml
        /// </summary>
        /// <param name="embededResourcesDirectoryPath">Output folder where all the embeded resources are.</param>
        /// <param name="expectedParentPackageType">Parent Package type.</param>
        /// <param name="expectedParentPackageName">Parent Package name; this is the attribute used to locate the package.</param>
        /// <param name="expectedFileName">Payload name; this is the attribute used to locate the payload.</param>
        /// <param name="expectedDownloadURL">@DownloadURL expected value.</param>
        /// <param name="acctualFilePath">Path to the acctual file to compate against file in cab.</param>
        public static void VerifyPackagePayloadInformation(string embededResourcesDirectoryPath, PackageTests.PackageType expectedParentPackageType, string expectedParentPackageName, string expectedFileName, string expectedDownloadURL, string acctualFilePath)
        {
            string expectedFileSize = new FileInfo(acctualFilePath).Length.ToString();
            string expectedHash = FileVerifier.ComputeFileSHA1Hash(acctualFilePath);

            // verify the ParameterInfo has the correct information 
            string parameterInfoXPath = string.Format(@"//pi:{0}[@Name='{1}']/pi:File[@Name='{2}']", GetPackageElementName(expectedParentPackageType, "ParameterInfo"), expectedParentPackageName, expectedFileName);
            XmlNodeList parameterInfoNodes = BundleTests.QueryParameterInfo(embededResourcesDirectoryPath, parameterInfoXPath);
            Assert.AreEqual(1, parameterInfoNodes.Count, "No Package payload with the name: '{0}' was found in ParameterInfo.xml.", expectedFileName);
            BundleTests.VerifyAttributeValue(parameterInfoNodes[0], "DownloadSize", expectedFileSize);
            BundleTests.VerifyAttributeValue(parameterInfoNodes[0], "Sha1HashValue", expectedHash);
            BundleTests.VerifyAttributeValue(parameterInfoNodes[0], "URL", expectedDownloadURL);

            // verify the Burn_Manifest has the correct information 
            string burnManifestXPath = string.Format(@"//burn:{0}[@FileName='{1}']/burn:Resource[@FileName='{2}']", GetPackageElementName(expectedParentPackageType, "BurnManifest"), expectedParentPackageName, expectedFileName);
            XmlNodeList burnManifestNodes = BundleTests.QueryBurnManifest(embededResourcesDirectoryPath, burnManifestXPath);
            Assert.AreEqual(1, burnManifestNodes.Count, "No Package payload with the name: '{0}' was found in Burn_Manifest.xml.", expectedFileName);
            BundleTests.VerifyAttributeValue(burnManifestNodes[0], "FileSize", expectedFileSize);
            BundleTests.VerifyAttributeValue(burnManifestNodes[0], "SHA1", expectedHash);
            BundleTests.VerifyAttributeValue(burnManifestNodes[0], "DownloadUrl", expectedDownloadURL);

            if (null == expectedDownloadURL)
            {
                // verify the correct file is added to the ux cab
                string attachedCabFilePath = Path.Combine(embededResourcesDirectoryPath, BundleTests.attachedCabFileName);
                CabInfo uxCab = new CabInfo(attachedCabFilePath);
                string extractedTestFile = Builder.GetUniqueFileName();
                uxCab.UnpackFile(burnManifestNodes[0].Attributes["EmbeddedId"].Value, extractedTestFile);
                FileVerifier.VerifyFilesAreIdentical(acctualFilePath, extractedTestFile);
            }
        }

        /// <summary>
        /// Return the expected element name in parameterInfo and burnManifest
        /// </summary>
        /// <param name="type">The type of the element</param>
        /// <param name="fileName">parameterInfo or burnManifest</param>
        /// <returns>Element name in the specified file.</returns>
        private static string GetPackageElementName(PackageType type, string fileName)
        {
            if (fileName.ToLower() == "parameterinfo")
            {
                switch (type)
                {
                    case PackageType.MSI:
                        return "MSI";
                    case PackageType.MSP:
                        return "MSP";
                    case PackageType.MSU:
                    case PackageType.EXE:
                        return "Exe";
                    default:
                        throw new ArgumentException(string.Format("Undefined PacakgeType : {0}", type.ToString()));
                };
            }
            else
            {
                switch (type)
                {
                    case PackageType.MSI:
                        return "MsiPackage";
                    case PackageType.MSP:
                        return "MspPackage";
                    case PackageType.MSU:
                        return "MsuPackage";
                    case PackageType.EXE:
                        return "ExePackage";
                    default:
                        throw new ArgumentException(string.Format("Undefined PacakgeType : {0}", type.ToString()));
                };
            }
        }

        #endregion
    }
}
