//-----------------------------------------------------------------------
// <copyright file="InstallPackages.ProductTests.cs" company="Microsoft">
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
// <summary>Tests for the Product element</summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests.Integration.BuildingPackages.InstallPackages
{
    using System;
    using System.IO;
    using System.Text;
    using System.Text.RegularExpressions;
    using System.Collections.Generic;
    using Microsoft.VisualStudio.TestTools.UnitTesting;

    using Microsoft.Tools.WindowsInstallerXml.Test;

    /// <summary>
    /// Tests for the Product element
    /// </summary>
    [TestClass]
    public class ProductTests
    {
        private static readonly string TestDataDirectory = Environment.ExpandEnvironmentVariables(@"%WIX%\test\data\Integration\BuildingPackages\InstallPackages\ProductTests");

        [TestMethod]
        [Description("Verify that a simple MSI can be built and that the expected default values are set for optional attributes")]
        [Priority(1)]
        public void SimpleProduct()
        {
            Candle candle = new Candle();
            candle.SourceFiles.Add(Path.Combine(ProductTests.TestDataDirectory, @"SimpleProduct\product.wxs"));
            candle.ExpectedWixMessages.Add(new WixMessage(1075, "The Product/@UpgradeCode attribute was not found; it is strongly recommended to ensure that this product can be upgraded.", WixMessage.MessageTypeEnum.Warning));
            candle.Run();

            Light light = new Light(candle);
            light.ExpectedWixMessages.Add(new WixMessage(1076, "ICE74: The UpgradeCode property is not authored in the Property table. It is strongly recommended that authors of installation packages specify an UpgradeCode for their application.", WixMessage.MessageTypeEnum.Warning));
            light.Run();

            Verifier.VerifyResults(Path.Combine(ProductTests.TestDataDirectory, @"SimpleProduct\expected.msi"), light.OutputFile);
        }

        [TestMethod]
        [Description("Verify that the attributes on Product can accept non-default or atypical values")]
        [Priority(1)]
        public void NonDefaultProduct()
        {
            Candle candle = new Candle();
            candle.SourceFiles.Add(Path.Combine(ProductTests.TestDataDirectory, @"NonDefaultProduct\product.wxs"));
            candle.Run();

            Light light = new Light(candle);
            light.Run();

            Verifier.VerifyResults(Path.Combine(ProductTests.TestDataDirectory, @"NonDefaultProduct\expected.msi"), light.OutputFile);
        }

        [TestMethod]
        [Description("Verify that valid product codes are allowed and that auto-generation produces the same Id every time")]
        [Priority(2)]
        public void ProductCodes()
        {
            // These are the valid Product codes that will be tested
            Dictionary<string, Regex> ids = new Dictionary<string, Regex>();
            ids.Add("{7B96AB21-31E0-4d23-A51C-2670C932B256}", new Regex("^{7B96AB21-31E0-4D23-A51C-2670C932B256}$"));
            ids.Add("7B96AB21-31E0-4d23-A51C-2670C932B256", new Regex("^{7B96AB21-31E0-4D23-A51C-2670C932B256}$"));
            ids.Add("aaaaaaaa-bbbb-cccc-dddd-eeeeeeffffff", new Regex("^{AAAAAAAA-BBBB-CCCC-DDDD-EEEEEEFFFFFF}$"));
            ids.Add("*", new Regex("^{[0-9A-F]{8}-[0-9A-F]{4}-[0-9A-F]{4}-[0-9A-F]{4}-[0-9A-F]{12}}$"));

            foreach (string id in ids.Keys)
            {
                Candle candle = new Candle();
                candle.SourceFiles.Add(Path.Combine(ProductTests.TestDataDirectory, @"ProductIds\product.wxs"));

                // Set a preprocessor variable that defines the package code
                candle.PreProcessorParams.Add("ProductId", id);

                candle.IgnoreExtraWixMessages = true;
                candle.Run();

                Light light = new Light(candle);
                light.Run();

                // Verify that the product code was set properly
                string productCode = Verifier.Query(light.OutputFile, "SELECT `Value` FROM `Property` WHERE `Property`='ProductCode'");
                Assert.IsTrue(ids[id].IsMatch(productCode), "The product code {0} in {1} does not match the regular expression {2}", productCode, light.OutputFile, ids[id].ToString());
            }
        }

        [TestMethod]
        [Description("Verify that there is an error when an invalid codepage is specified")]
        [Priority(2)]
        public void InvalidCodepage()
        {
            // Create a list of invalid codepages
            List<string> codepages = new List<string>();
            codepages.Add("999");
            codepages.Add("abc");

            foreach (string codepage in codepages)
            {
                Candle candle = new Candle();
                candle.SourceFiles.Add(Path.Combine(ProductTests.TestDataDirectory, @"Codepage\product.wxs"));
                candle.PreProcessorParams.Add("Codepage", codepage);
                candle.ExpectedWixMessages.Add(new WixMessage(276, String.Format("The code page '{0}' is not a valid Windows code page. Please check the Product/@Codepage attribute value in your source file.", codepage), WixMessage.MessageTypeEnum.Error));
                candle.ExpectedExitCode = 276;
                candle.Run();
            }
        }

        [TestMethod]
        [Description("Verify that attribute values with whitespace are treated as null or empty values")]
        [Priority(3)]
        [Ignore()] // Bug
        public void AttributesWithWhitespace()
        {
            // Check the Name attribute
            Candle candle = new Candle();
            candle.SourceFiles.Add(Path.Combine(ProductTests.TestDataDirectory, @"AttributesWithWhitespace\name.wxs"));
            candle.ExpectedWixMessages.Add(new WixMessage(6, "The Product/@Name attribute's value cannot be an empty string.  If you want the value to be null or empty, simply remove the entire attribute.", WixMessage.MessageTypeEnum.Error));
            candle.Run();

            // Check the Manufacturer attribute
            candle = new Candle();
            candle.SourceFiles.Add(Path.Combine(ProductTests.TestDataDirectory, @"AttributesWithWhitespace\manufacturer.wxs"));
            candle.ExpectedWixMessages.Add(new WixMessage(6, "The Product/@Manufacturer attribute's value cannot be an empty string.  If you want the value to be null or empty, simply remove the entire attribute.", WixMessage.MessageTypeEnum.Error));
            candle.Run();
        }

        [TestMethod]
        [Description("Verify that valid codepages are allowed")]
        [Priority(3)]
        [Timeout(10*30000)]
        [Ignore()] // Bug
        public void ValidCodepages()
        {
            // Test a subset of valid codepages
            // http://www.microsoft.com/globaldev/reference/cphome.mspx 
            // http://msdn2.microsoft.com/en-us/library/ms776446(VS.85).aspx

            Dictionary<string, int> codepages = new Dictionary<string, int>();

            // Supported code pages and some case changes
            codepages.Add("0", 0);
            codepages.Add("utf-7", 65000);
            codepages.Add("UTF-8", 65001);
            codepages.Add("WiNdOwS-1252", 1252);

            // Randomly select 3 valid codepages
            Random random = new Random(WixTests.Seed);
            EncodingInfo[] encodings = Encoding.GetEncodings();
            for (int i = 0; i < System.Math.Min(encodings.Length, 3); i++)
            {
                int randomNumber = random.Next(0, encodings.Length - 1);

                if (codepages.ContainsKey(encodings[randomNumber].Name))
                {
                    // The codepage was already added to the list
                    i--;
                    continue;
                }
                else
                {
                    // Add the codepage by its Id and its web name
                    codepages.Add(encodings[randomNumber].Name, encodings[randomNumber].CodePage);
                    codepages.Add(Convert.ToString(encodings[randomNumber].CodePage), encodings[randomNumber].CodePage);
                }
            }

            // Verify that an MSI can be built for each codepage in the list of valid codepages
            foreach (string codepage in codepages.Keys)
            {
                Candle candle = new Candle();
                candle.SourceFiles.Add(Path.Combine(ProductTests.TestDataDirectory, @"Codepage\product.wxs"));

                // Set a preprocessor variable that defines the codepage
                candle.PreProcessorParams.Add("Codepage", codepage);

                candle.Run();

                Light light = new Light(candle);
                light.Run();

                Verifier.VerifyDatabaseCodepage(light.OutputFile, codepages[codepage]);
            }
        }

        [TestMethod]
        [Description("Verify that there is not an exception when utf-32 used for the database codepage")]
        [Priority(3)]
        [Ignore()] // Bug
        public void Codepage_UTF32()
        {
            string codepage = "utf-32";

            Candle candle = new Candle();
            candle.SourceFiles.Add(Path.Combine(ProductTests.TestDataDirectory, @"Codepage\product.wxs"));
            candle.PreProcessorParams.Add("Codepage", codepage);
            candle.Run();

            Light light = new Light(candle);
            light.Run();
        }

        [TestMethod]
        [Description("Verify that there is a proper error message when x-iscii-de used for the database codepage")]
        [Priority(3)]
        [Ignore()] // Bug
        public void Codepage_x_iscii_de()
        {
            string codepage = "x_iscii_de";

            Candle candle = new Candle();
            candle.SourceFiles.Add(Path.Combine(ProductTests.TestDataDirectory, @"Codepage\product.wxs"));
            candle.PreProcessorParams.Add("Codepage", codepage);
            candle.Run();

            Light light = new Light(candle);
            light.Run();
        }

        [TestMethod]
        [Description("Verify that light doesn't hang when the codepage x-EBCDIC-KoreanExtended is used")]
        [Priority(3)]
        [Timeout(30000)]
        [Ignore()] // Bug
        public void Codepage_x_EBCDIC_KoreanExtended()
        {
            string codepage = "x-EBCDIC-KoreanExtended";

            Candle candle = new Candle();
            candle.SourceFiles.Add(Path.Combine(ProductTests.TestDataDirectory, @"Codepage\product.wxs"));
            candle.PreProcessorParams.Add("Codepage", codepage);
            candle.Run();

            Light light = new Light(candle);
            light.Run();
        }

        [TestMethod]
        [Description("Verify that the UpgradeCode allows any string defined in the GUID type")]
        [Priority(3)]
        [Ignore]
        public void ValidUpgradeCodes()
        {
        }

        [TestMethod]
        [Description("Verify that there is an error if the Product Id is not a valid GUID")]
        [Priority(3)]
        [Ignore]
        public void InvalidProductId()
        {
        }

        [TestMethod]
        [Description("Verify that there is an error if the UpgradeCode is a '*'")]
        [Priority(3)]
        [Ignore]
        public void InvalidUpgradeCode()
        {
        }

        [TestMethod]
        [Description("Verify that the Upgrade table can be created by using the Upgrade and UpgradeVersion elements")]
        [Priority(3)]
        [Ignore]
        public void UpgradeTable()
        {
        }

        [TestMethod]
        [Description("Verify that the optional child elements of the Upgrade element are not required")]
        [Priority(3)]
        [Ignore]
        public void UpgradeOptionalChildElements()
        {
        }

        [TestMethod]
        [Description("Verify that all of the attributes of the UpgradeVersion are used in the MSI as they are supposed to be")]
        [Priority(3)]
        [Ignore]
        public void UpgradeVersion()
        {
        }
    }
}