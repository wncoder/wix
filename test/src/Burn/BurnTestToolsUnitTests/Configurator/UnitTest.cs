//-----------------------------------------------------------------------
// <copyright file="UnitTest.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>
//     - Tests for the Configurator (part of the Burn test infrastructure)
// </summary>
//-----------------------------------------------------------------------

namespace ParameterInfoConfiguratorUnitTest
{
    using System.Collections.Generic;
    using System.Xml;
    using Microsoft.VisualStudio.TestTools.UnitTesting;
    using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.ParameterInfoConfiguratorEngine;

    /// <summary>
    /// Summary description for UnitTest1
    /// </summary>
    [TestClass]
    public class UnitTest
    {
        private TestContext testContextInstance;

        /// <summary>
        ///Gets or sets the test context which provides
        ///information about and functionality for the current test run.
        ///</summary>
        public TestContext TestContext
        {
            get
            {
                return testContextInstance;
            }
            set
            {
                testContextInstance = value;
            }
        }

        #region Additional test attributes
        //
        // You can use the following additional attributes as you write your tests:
        //
        // Use ClassInitialize to run code before running the first test in the class
        // [ClassInitialize()]
        // public static void MyClassInitialize(TestContext testContext) { }
        //
        // Use ClassCleanup to run code after all tests in a class have run
        // [ClassCleanup()]
        // public static void MyClassCleanup() { }
        //
        // Use TestInitialize to run code before running each test 
        // [TestInitialize()]
        // public void MyTestInitialize() { }
        //
        // Use TestCleanup to run code after each test has run
        // [TestCleanup()]
        // public void MyTestCleanup() { }
        //
        #endregion

        #region verify all of BurnOperands

        [TestMethod]
        public void ITBurnOperandsFileVersion()
        {
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnOperands.FileVersion element = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnOperands.FileVersion("MyPath");
            string expectedXml = "<FileVersion Location=\"MyPath\">" + "</FileVersion>";
            string actualXml = XMLGenerator.GetXmlString(element);
            Assert.AreEqual(expectedXml, actualXml);
        }

        [TestMethod]
        public void ITBurnOperandsHasAdvertisedFeatures()
        {
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnOperands.HasAdvertisedFeatures element = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnOperands.HasAdvertisedFeatures("MyProductCode");
            string expectedXml = "<HasAdvertisedFeatures ProductCode=\"MyProductCode\">" + "</HasAdvertisedFeatures>";
            string actualXml = XMLGenerator.GetXmlString(element);
            Assert.AreEqual(expectedXml, actualXml);
        }

        [TestMethod]
        public void ITBurnOperandsIsAdministrator()
        {
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnOperands.IsAdministrator element = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnOperands.IsAdministrator();
            string expectedXml = "<IsAdministrator>" + "</IsAdministrator>";
            string actualXml = XMLGenerator.GetXmlString(element);
            Assert.AreEqual(expectedXml, actualXml);
        }

        [TestMethod]
        public void ITBurnOperandsIsInOSCompatibilityMode()
        {
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnOperands.IsInOSCompatibilityMode element = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnOperands.IsInOSCompatibilityMode();
            string expectedXml = "<IsInOSCompatibilityMode>" + "</IsInOSCompatibilityMode>";
            string actualXml = XMLGenerator.GetXmlString(element);
            Assert.AreEqual(expectedXml, actualXml);
        }

        [TestMethod]
        public void ITBurnOperandsLCID()
        {
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnOperands.LCID element = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnOperands.LCID();
            string expectedXml = "<LCID>" + "</LCID>";
            string actualXml = XMLGenerator.GetXmlString(element);
            Assert.AreEqual(expectedXml, actualXml);
        }

        [TestMethod]
        public void ITBurnOperandsMsiGetCachedPatchPath()
        {
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnOperands.MsiGetCachedPatchPath element = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnOperands.MsiGetCachedPatchPath("MyPatchCode");
            string expectedXml = "<MsiGetCachedPatchPath PatchCode=\"MyPatchCode\">" + "</MsiGetCachedPatchPath>";
            string actualXml = XMLGenerator.GetXmlString(element);
            Assert.AreEqual(expectedXml, actualXml);
        }

        [TestMethod]
        public void ITBurnOperandsMsiProductVersion()
        {
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnOperands.MsiProductVersion element = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnOperands.MsiProductVersion("MyProductCode");
            string expectedXml = "<MsiProductVersion ProductCode=\"MyProductCode\">" + "</MsiProductVersion>";
            string actualXml = XMLGenerator.GetXmlString(element);
            Assert.AreEqual(expectedXml, actualXml);
        }

        [TestMethod]
        public void ITBurnOperandsMsiXmlBlob()
        {
            string testMspFile = System.Environment.ExpandEnvironmentVariables(@"%WIX_ROOT%\test\data\BurnTestPayloads\MSIsandMSPs\GDR1\gdr1.msp"); // MSP that will target testMsiFile
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnOperands.MsiXmlBlob element = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnOperands.MsiXmlBlob(testMspFile);

            string expectedXml = "" +
                "<MsiXmlBlob>" +
                "<MsiPatch xmlns=\"http://www.microsoft.com/msi/patch_applicability.xsd\" SchemaVersion=\"1.0.0.0\" PatchGUID=\"{780B530E-03AD-47AF-A128-DA08DB495253}\" MinMsiVersion=\"5\">" +
                "<TargetProduct MinMsiVersion=\"300\">" +
                "<TargetProductCode Validate=\"true\">{FB94421B-7FA3-4495-A9D7-212099C19147}</TargetProductCode>" +
                "<TargetVersion Validate=\"true\" ComparisonType=\"Equal\" ComparisonFilter=\"MajorMinorUpdate\">1.0.1.0</TargetVersion>" +
                "<TargetLanguage Validate=\"false\">1033</TargetLanguage>" +
                "<UpdatedLanguages>1033</UpdatedLanguages>" +
                "<UpgradeCode Validate=\"true\">{5A990E27-3480-4D0C-BCA3-75B726C7C048}</UpgradeCode>" +
                "</TargetProduct>" +
                "<TargetProduct MinMsiVersion=\"300\">" +
                "<TargetProductCode Validate=\"true\">{FB94421B-7FA3-4495-A9D7-212099C19147}</TargetProductCode>" +
                "<TargetVersion Validate=\"true\" ComparisonType=\"Equal\" ComparisonFilter=\"MajorMinorUpdate\">1.0.0.0</TargetVersion>" +
                "<TargetLanguage Validate=\"false\">1033</TargetLanguage>" +
                "<UpdatedLanguages>1033</UpdatedLanguages>" +
                "<UpgradeCode Validate=\"true\">{5A990E27-3480-4D0C-BCA3-75B726C7C048}</UpgradeCode>" +
                "</TargetProduct>" +
                "<TargetProductCode>{FB94421B-7FA3-4495-A9D7-212099C19147}</TargetProductCode>" +
                "<SequenceData>" +
                "<PatchFamily>Source</PatchFamily>" +
                "<Sequence>1.0.1.1</Sequence>" +
                "<Attributes>1</Attributes>" +
                "</SequenceData>" +
                "<SequenceData>" +
                "<PatchFamily>Patch</PatchFamily>" +
                "<Sequence>1.0.1.1</Sequence>" +
                "<Attributes>1</Attributes>" +
                "</SequenceData>" +
                "</MsiPatch>" +
                "\r\n</MsiXmlBlob>";
            string actualXml = XMLGenerator.GetXmlString(element);
            Assert.AreEqual(expectedXml, actualXml);
        }

        [TestMethod]
        public void ITBurnOperandsOperation()
        {
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnOperands.Operation element = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnOperands.Operation();
            string expectedXml = "<Operation>" + "</Operation>";
            string actualXml = XMLGenerator.GetXmlString(element);
            Assert.AreEqual(expectedXml, actualXml);
        }

        [TestMethod]
        public void ITBurnOperandsPath()
        {
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnOperands.Path element = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnOperands.Path("MyPath");
            string expectedXml = "<Path Location=\"MyPath\">" + "</Path>";
            string actualXml = XMLGenerator.GetXmlString(element);
            Assert.AreEqual(expectedXml, actualXml);
        }

        [TestMethod]
        public void ITBurnOperandsRebootPending()
        {
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnOperands.RebootPending element = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnOperands.RebootPending();
            string expectedXml = "<RebootPending>" + "</RebootPending>";
            string actualXml = XMLGenerator.GetXmlString(element);
            Assert.AreEqual(expectedXml, actualXml);
        }

        [TestMethod]
        public void ITBurnOperandsRegKey()
        {
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnOperands.RegKey element = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnOperands.RegKey("MyRegKey");
            string expectedXml = "<RegKey Location=\"MyRegKey\">" + "</RegKey>";
            string actualXml = XMLGenerator.GetXmlString(element);
            Assert.AreEqual(expectedXml, actualXml);
        }

        [TestMethod]
        public void ITBurnOperandsRegKeyFileVersion()
        {
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnOperands.RegKeyFileVersion element = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnOperands.RegKeyFileVersion("MyRegKey");
            string expectedXml = "<RegKeyFileVersion Location=\"MyRegKey\">" + "</RegKeyFileVersion>";
            string actualXml = XMLGenerator.GetXmlString(element);
            Assert.AreEqual(expectedXml, actualXml);
        }

        [TestMethod]
        public void ITBurnOperandsRegKeyValue()
        {
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnOperands.RegKeyValue element = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnOperands.RegKeyValue("MyRegKey");
            string expectedXml = "<RegKeyValue Location=\"MyRegKey\">" + "</RegKeyValue>";
            string actualXml = XMLGenerator.GetXmlString(element);
            Assert.AreEqual(expectedXml, actualXml);
        }

        [TestMethod]
        public void ITBurnOperandsTargetArchitecture()
        {
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnOperands.TargetArchitecture element = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnOperands.TargetArchitecture();
            string expectedXml = "<TargetArchitecture>" + "</TargetArchitecture>";
            string actualXml = XMLGenerator.GetXmlString(element);
            Assert.AreEqual(expectedXml, actualXml);
        }

        [TestMethod]
        public void ITBurnOperandsTargetOS1()
        {
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnOperands.TargetOS element = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnOperands.TargetOS();
            string expectedXml = "<TargetOS>" + "</TargetOS>";
            string actualXml = XMLGenerator.GetXmlString(element);
            Assert.AreEqual(expectedXml, actualXml);
        }

        [TestMethod]
        public void ITBurnOperandsTargetOSType()
        {
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnOperands.TargetOSType element = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnOperands.TargetOSType();
            string expectedXml = "<TargetOSType>" + "</TargetOSType>";
            string actualXml = XMLGenerator.GetXmlString(element);
            Assert.AreEqual(expectedXml, actualXml);
        }
        
        #endregion

        #region verify all of BurnExpressions

        [TestMethod]
        public void ITBurnExpressionsNestedOrExpressionsElement()
        {

            string expectedXml = "" +
                "<IsPresent>" +
                "<Or>" +
                "<Or>" +
                "<Or>" +
                "<AlwaysTrue>" + "</AlwaysTrue>" +
                "<AlwaysTrue>" + "</AlwaysTrue>" +
                "</Or>" +
                "<Or>" +
                "<NeverTrue>" + "</NeverTrue>" +
                "<NeverTrue>" + "</NeverTrue>" +
                "</Or>" +
                "</Or>" +
                "<Or>" +
                "<Or>" +
                "<AlwaysTrue>" + "</AlwaysTrue>" +
                "<NeverTrue>" + "</NeverTrue>" +
                "</Or>" +
                "<Or>" +
                "<NeverTrue>" + "</NeverTrue>" +
                "<AlwaysTrue>" + "</AlwaysTrue>" +
                "</Or>" +
                "</Or>" +
                "</Or>" +
                "</IsPresent>";

            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemElements.IsPresent element = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemElements.IsPresent();

            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.Or orGrandChild1 = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.Or(new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.AlwaysTrue(), new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.AlwaysTrue());
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.Or orGrandChild2 = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.Or(new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.NeverTrue(), new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.NeverTrue());
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.Or orGrandChild3 = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.Or(new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.AlwaysTrue(), new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.NeverTrue());
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.Or orGrandChild4 = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.Or(new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.NeverTrue(), new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.AlwaysTrue());

            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.Or orChild1 = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.Or(orGrandChild1, orGrandChild2);
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.Or orChild2 = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.Or(orGrandChild3, orGrandChild4);

            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.Or or1 = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.Or(orChild1, orChild2);

            element.Expression = or1;

            string actualXml = XMLGenerator.GetXmlString(element);
            Assert.AreEqual(expectedXml, actualXml);
        }

        [TestMethod]
        public void ITBurnExpressionsComplexIsPresentElement()
        {
            string expectedXml = "" +
                "<IsPresent>" +
                "<Or>" +
                "<And>" +
                "<Equals LeftHandSide=\"x86\" BoolWhenNonExistent=\"False\">" +
                "<TargetArchitecture>" + "</TargetArchitecture>" +
                "</Equals>" +
                "<GreaterThan LeftHandSide=\"9.1.2600.2180\" BoolWhenNonExistent=\"False\">" +
                "<FileVersion Location=\"%windir%\\notepad.exe\">" + "</FileVersion>" +
                "</GreaterThan>" +
                "</And>" +
                "<And>" +
                "<Equals LeftHandSide=\"x64\" BoolWhenNonExistent=\"False\">" +
                "<TargetArchitecture>" + "</TargetArchitecture>" +
                "</Equals>" +
                "<GreaterThan LeftHandSide=\"5.0.6001.17000\" BoolWhenNonExistent=\"False\">" +
                "<FileVersion Location=\"%windir%\\system32\\notepad.exe\">" + "</FileVersion>" +
                "</GreaterThan>" +
                "</And>" +
                "</Or>" +
                "</IsPresent>";

            // Build an expression and store it in an IsPresent element.
            // Use expressions: "And", "Or" 
            // Use operands: "FileVersion", "TargetArchitecture"
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemElements.IsPresent element = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemElements.IsPresent();

            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnOperands.FileVersion fvWinNotepad = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnOperands.FileVersion(@"%windir%\notepad.exe");
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnOperands.FileVersion fvWinSys32Notepad = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnOperands.FileVersion(@"%windir%\system32\notepad.exe");

            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.GreaterThan gt91 = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.GreaterThan("9.1.2600.2180",
                Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.ExpressionAttribute.BoolWhenNonExistentType.False,
                fvWinNotepad);

            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.GreaterThan gt50 = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.GreaterThan("5.0.6001.17000",
                Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.ExpressionAttribute.BoolWhenNonExistentType.False,
                fvWinSys32Notepad);

            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.Equals eqX86Arch = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.Equals("x86",
                Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.ExpressionAttribute.BoolWhenNonExistentType.False,
                new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnOperands.TargetArchitecture());

            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.Equals eqX64Arch = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.Equals("x64",
                Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.ExpressionAttribute.BoolWhenNonExistentType.False,
                new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnOperands.TargetArchitecture());

            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.And andX86NotepadVer = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.And(eqX86Arch, gt91);

            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.And andX64NotepadVer = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.And(eqX64Arch, gt50);

            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.Or orNotepadVerAcceptible1 = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.Or(andX86NotepadVer, andX64NotepadVer);

            element.Expression = orNotepadVerAcceptible1;

            string actualXml = XMLGenerator.GetXmlString(element);
            Assert.AreEqual(expectedXml, actualXml);
        }

        [TestMethod]
        public void ITBurnExpressionsAlwaysTrue()
        {
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.AlwaysTrue element = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.AlwaysTrue();
            string expectedXml = "<AlwaysTrue>" + "</AlwaysTrue>";
            string actualXml = XMLGenerator.GetXmlString(element);
            Assert.AreEqual(expectedXml, actualXml);
        }

        [TestMethod]
        public void ITBurnExpressionsAnd()
        {
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.And element = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.And(
                new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.AlwaysTrue(),
                new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.AlwaysTrue());
            string expectedXml = "<And>" + "<AlwaysTrue>" + "</AlwaysTrue>" + "<AlwaysTrue>" + "</AlwaysTrue>" + "</And>";
            string actualXml = XMLGenerator.GetXmlString(element);
            Assert.AreEqual(expectedXml, actualXml);
        }

        [TestMethod]
        public void ITBurnExpressionsEquals()
        {
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.Equals element = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.Equals(
                "x86", 
                Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.ExpressionAttribute.BoolWhenNonExistentType.False,
                new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnOperands.TargetOS());
            string expectedXml = "<Equals LeftHandSide=\"x86\" BoolWhenNonExistent=\"False\">" + "<TargetOS>" + "</TargetOS>" + "</Equals>";
            string actualXml = XMLGenerator.GetXmlString(element);
            Assert.AreEqual(expectedXml, actualXml);
        }

        [TestMethod]
        public void ITBurnExpressionsExists()
        {
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.Exists element = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.Exists(new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnOperands.Path("c:\\TDDBlock\\Success\\"));
            string expectedXml = "<Exists>" + "<Path Location=\"c:\\TDDBlock\\Success\\\">" + "</Path>" + "</Exists>";
            string actualXml = XMLGenerator.GetXmlString(element);
            Assert.AreEqual(expectedXml, actualXml);
        }

        [TestMethod]
        public void ITBurnExpressionsGreaterThan()
        {
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.GreaterThan element = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.GreaterThan(
                "1", 
                Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.ExpressionAttribute.BoolWhenNonExistentType.False, 
                new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnOperands.TargetOS());
            string expectedXml = "<GreaterThan LeftHandSide=\"1\" BoolWhenNonExistent=\"False\">" + "<TargetOS>" + "</TargetOS>" + "</GreaterThan>";
            string actualXml = XMLGenerator.GetXmlString(element);
            Assert.AreEqual(expectedXml, actualXml);
        }

        [TestMethod]
        public void ITBurnExpressionsGreaterThanOrEqualTo()
        {
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.GreaterThanOrEqualTo element = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.GreaterThanOrEqualTo(
                "1",
                Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.ExpressionAttribute.BoolWhenNonExistentType.False,
                new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnOperands.TargetOS());
            string expectedXml = "<GreaterThanOrEqualTo LeftHandSide=\"1\" BoolWhenNonExistent=\"False\">" + "<TargetOS>" + "</TargetOS>" + "</GreaterThanOrEqualTo>";
            string actualXml = XMLGenerator.GetXmlString(element);
            Assert.AreEqual(expectedXml, actualXml);
        }

        [TestMethod]
        public void ITBurnExpressionsLessThan()
        {
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.LessThan element = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.LessThan(
                "1",
                Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.ExpressionAttribute.BoolWhenNonExistentType.False,
                new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnOperands.TargetOS());
            string expectedXml = "<LessThan LeftHandSide=\"1\" BoolWhenNonExistent=\"False\">" + "<TargetOS>" + "</TargetOS>" + "</LessThan>";
            string actualXml = XMLGenerator.GetXmlString(element);
            Assert.AreEqual(expectedXml, actualXml);
        }

        [TestMethod]
        public void ITBurnExpressionsLessThanOrEqualTo()
        {
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.LessThanOrEqualTo element = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.LessThanOrEqualTo(
                "1",
                Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.ExpressionAttribute.BoolWhenNonExistentType.False,
                new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnOperands.TargetOS());
            string expectedXml = "<LessThanOrEqualTo LeftHandSide=\"1\" BoolWhenNonExistent=\"False\">" + "<TargetOS>" + "</TargetOS>" + "</LessThanOrEqualTo>";
            string actualXml = XMLGenerator.GetXmlString(element);
            Assert.AreEqual(expectedXml, actualXml);
        }

        [TestMethod]
        public void ITBurnExpressionsNeverTrue()
        {
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.NeverTrue element = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.NeverTrue();
            string expectedXml = "<NeverTrue>" + "</NeverTrue>";
            string actualXml = XMLGenerator.GetXmlString(element);
            Assert.AreEqual(expectedXml, actualXml);
        }

        [TestMethod]
        public void ITBurnExpressionsNot()
        {
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.Not element = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.Not(new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.NeverTrue());
            string expectedXml = "<Not>" + "<NeverTrue>" + "</NeverTrue>" + "</Not>";
            string actualXml = XMLGenerator.GetXmlString(element);
            Assert.AreEqual(expectedXml, actualXml);
        }

        [TestMethod]
        public void ITBurnExpressionsOr()
        {
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.Or element = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.Or(
                new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.NeverTrue(),
                new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.AlwaysTrue());
            string expectedXml = "<Or>" + "<NeverTrue>" + "</NeverTrue>" + "<AlwaysTrue>" + "</AlwaysTrue>" + "</Or>";
            string actualXml = XMLGenerator.GetXmlString(element);
            Assert.AreEqual(expectedXml, actualXml);
        }

        #endregion

        #region verify all of BurnBlockers

        [TestMethod]
        public void ITBurnBlockersBlockerBaseElementBlockIf()
        {
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnBlockers.BlockerBaseElement.BlockIf element = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnBlockers.BlockerBaseElement.BlockIf();
            element.ID = "myBlock";
            element.DisplayText = "Block You!";
            element.Expression = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.AlwaysTrue();
            string expectedXml = "<BlockIf ID=\"myBlock\" DisplayText=\"Block You!\">" + "<AlwaysTrue>" + "</AlwaysTrue>" + "</BlockIf>";
            string actualXml = XMLGenerator.GetXmlString(element);
            Assert.AreEqual(expectedXml, actualXml);
        }

        [TestMethod]
        public void ITBurnBlockersBlockerBaseElementBlockIfGroup()
        {
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnBlockers.BlockerBaseElement.BlockIf bi1 = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnBlockers.BlockerBaseElement.BlockIf();
            bi1.ID = "myBlock1";
            bi1.DisplayText = "Block You! 1";
            bi1.Expression = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.AlwaysTrue();
                        
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnBlockers.BlockerBaseElement.BlockIf bi2 = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnBlockers.BlockerBaseElement.BlockIf();
            bi2.ID = "myBlock2";
            bi2.DisplayText = "Block You! 2";
            bi2.Expression = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.AlwaysTrue();

            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnBlockers.BlockerBaseElement.BlockIfGroup element = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnBlockers.BlockerBaseElement.BlockIfGroup();
            element.BlockIfs.Add(bi1);
            element.BlockIfs.Add(bi2);
            element.DisplayText = "This group blocked:";

            string expectedXml = "<BlockIfGroup DisplayText=\"This group blocked:\">" +
                "<BlockIf ID=\"myBlock1\" DisplayText=\"Block You! 1\">" + "<AlwaysTrue>" + "</AlwaysTrue>" + "</BlockIf>" +
                "<BlockIf ID=\"myBlock2\" DisplayText=\"Block You! 2\">" + "<AlwaysTrue>" + "</AlwaysTrue>" + "</BlockIf>" + 
                "</BlockIfGroup>";
            string actualXml = XMLGenerator.GetXmlString(element);
            Assert.AreEqual(expectedXml, actualXml);
        }

        [TestMethod]
        public void ITBurnBlockersStopBlock()
        {
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnBlockers.BlockerBaseElement.BlockIf bi1 = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnBlockers.BlockerBaseElement.BlockIf();
            bi1.ID = "myBlock1";
            bi1.DisplayText = "Block You! 1";
            bi1.Expression = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.AlwaysTrue();

            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnBlockers.BlockerBaseElement.BlockIf bi2 = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnBlockers.BlockerBaseElement.BlockIf();
            bi2.ID = "myBlock2";
            bi2.DisplayText = "Block You! 2";
            bi2.Expression = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.AlwaysTrue();

            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnBlockers.BlockerBaseElement.BlockIfGroup biGrp = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnBlockers.BlockerBaseElement.BlockIfGroup();
            biGrp.BlockIfs.Add(bi1);
            biGrp.BlockIfs.Add(bi2);
            biGrp.DisplayText = "This group blocked:";

            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnBlockers.BlockerBaseElement.BlockIf bi3 = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnBlockers.BlockerBaseElement.BlockIf();
            bi3.ID = "myBlock3";
            bi3.DisplayText = "Block You! 3";
            bi3.Expression = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.AlwaysTrue();

            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnBlockers.StopBlockers element = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnBlockers.StopBlockers();
            element.BlockIfBaseItems.Add(biGrp);
            element.BlockIfBaseItems.Add(bi3);
            element.ReturnCode = 1234;

            string expectedXml = "<StopBlockers ReturnCode=\"1234\">" + 
                "<BlockIfGroup DisplayText=\"This group blocked:\">" +
                "<BlockIf ID=\"myBlock1\" DisplayText=\"Block You! 1\">" + "<AlwaysTrue>" + "</AlwaysTrue>" + "</BlockIf>" +
                "<BlockIf ID=\"myBlock2\" DisplayText=\"Block You! 2\">" + "<AlwaysTrue>" + "</AlwaysTrue>" + "</BlockIf>" + 
                "</BlockIfGroup>" +
                "<BlockIf ID=\"myBlock3\" DisplayText=\"Block You! 3\">" + "<AlwaysTrue>" + "</AlwaysTrue>" + "</BlockIf>" + 
                "</StopBlockers>";
            string actualXml = XMLGenerator.GetXmlString(element);
            Assert.AreEqual(expectedXml, actualXml);
        }

        [TestMethod]
        public void ITBurnBlockersSuccessBlock()
        {
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnBlockers.BlockerBaseElement.BlockIf bi1 = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnBlockers.BlockerBaseElement.BlockIf();
            bi1.ID = "myBlock1";
            bi1.DisplayText = "Block You! 1";
            bi1.Expression = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.AlwaysTrue();

            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnBlockers.BlockerBaseElement.BlockIf bi2 = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnBlockers.BlockerBaseElement.BlockIf();
            bi2.ID = "myBlock2";
            bi2.DisplayText = "Block You! 2";
            bi2.Expression = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.AlwaysTrue();

            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnBlockers.BlockerBaseElement.BlockIfGroup biGrp = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnBlockers.BlockerBaseElement.BlockIfGroup();
            biGrp.BlockIfs.Add(bi1);
            biGrp.BlockIfs.Add(bi2);
            biGrp.DisplayText = "This group blocked:";

            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnBlockers.BlockerBaseElement.BlockIf bi3 = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnBlockers.BlockerBaseElement.BlockIf();
            bi3.ID = "myBlock3";
            bi3.DisplayText = "Block You! 3";
            bi3.Expression = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.AlwaysTrue();

            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnBlockers.SuccessBlockers element = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnBlockers.SuccessBlockers();
            element.BlockIfBaseItems.Add(biGrp);
            element.BlockIfBaseItems.Add(bi3);
            element.ReturnCode = 0;

            string expectedXml = "<SuccessBlockers ReturnCode=\"0\">" +
                "<BlockIfGroup DisplayText=\"This group blocked:\">" +
                "<BlockIf ID=\"myBlock1\" DisplayText=\"Block You! 1\">" + "<AlwaysTrue>" + "</AlwaysTrue>" + "</BlockIf>" +
                "<BlockIf ID=\"myBlock2\" DisplayText=\"Block You! 2\">" + "<AlwaysTrue>" + "</AlwaysTrue>" + "</BlockIf>" +
                "</BlockIfGroup>" +
                "<BlockIf ID=\"myBlock3\" DisplayText=\"Block You! 3\">" + "<AlwaysTrue>" + "</AlwaysTrue>" + "</BlockIf>" +
                "</SuccessBlockers>";
            string actualXml = XMLGenerator.GetXmlString(element);
            Assert.AreEqual(expectedXml, actualXml);
        }

        [TestMethod]
        public void ITBurnBlockersWarnBlock()
        {
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnBlockers.BlockerBaseElement.BlockIf bi1 = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnBlockers.BlockerBaseElement.BlockIf();
            bi1.ID = "myBlock1";
            bi1.DisplayText = "Block You! 1";
            bi1.Expression = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.AlwaysTrue();

            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnBlockers.BlockerBaseElement.BlockIf bi2 = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnBlockers.BlockerBaseElement.BlockIf();
            bi2.ID = "myBlock2";
            bi2.DisplayText = "Block You! 2";
            bi2.Expression = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.AlwaysTrue();

            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnBlockers.BlockerBaseElement.BlockIfGroup biGrp = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnBlockers.BlockerBaseElement.BlockIfGroup();
            biGrp.BlockIfs.Add(bi1);
            biGrp.BlockIfs.Add(bi2);
            biGrp.DisplayText = "This group blocked:";

            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnBlockers.BlockerBaseElement.BlockIf bi3 = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnBlockers.BlockerBaseElement.BlockIf();
            bi3.ID = "myBlock3";
            bi3.DisplayText = "Block You! 3";
            bi3.Expression = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.AlwaysTrue();

            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnBlockers.WarnBlockers element = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnBlockers.WarnBlockers();
            element.BlockIfBaseItems.Add(biGrp);
            element.BlockIfBaseItems.Add(bi3);

            string expectedXml = "<WarnBlockers>" +
                "<BlockIfGroup DisplayText=\"This group blocked:\">" +
                "<BlockIf ID=\"myBlock1\" DisplayText=\"Block You! 1\">" + "<AlwaysTrue>" + "</AlwaysTrue>" + "</BlockIf>" +
                "<BlockIf ID=\"myBlock2\" DisplayText=\"Block You! 2\">" + "<AlwaysTrue>" + "</AlwaysTrue>" + "</BlockIf>" +
                "</BlockIfGroup>" +
                "<BlockIf ID=\"myBlock3\" DisplayText=\"Block You! 3\">" + "<AlwaysTrue>" + "</AlwaysTrue>" + "</BlockIf>" +
                "</WarnBlockers>";
            string actualXml = XMLGenerator.GetXmlString(element);
            Assert.AreEqual(expectedXml, actualXml);
        }

        [TestMethod]
        public void ITBurnBlockers()
        {
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnBlockers.BlockerBaseElement.BlockIf bi1 = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnBlockers.BlockerBaseElement.BlockIf();
            bi1.ID = "myBlock1";
            bi1.DisplayText = "Block You! 1";
            bi1.Expression = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.AlwaysTrue();

            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnBlockers.BlockerBaseElement.BlockIf bi2 = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnBlockers.BlockerBaseElement.BlockIf();
            bi2.ID = "myBlock2";
            bi2.DisplayText = "Block You! 2";
            bi2.Expression = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.AlwaysTrue();

            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnBlockers.BlockerBaseElement.BlockIfGroup biGrp = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnBlockers.BlockerBaseElement.BlockIfGroup();
            biGrp.BlockIfs.Add(bi1);
            biGrp.BlockIfs.Add(bi2);
            biGrp.DisplayText = "This group blocked:";

            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnBlockers.BlockerBaseElement.BlockIf bi3 = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnBlockers.BlockerBaseElement.BlockIf();
            bi3.ID = "myBlock3";
            bi3.DisplayText = "Block You! 3";
            bi3.Expression = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.AlwaysTrue();

            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnBlockers.StopBlockers stopBlockers = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnBlockers.StopBlockers();
            stopBlockers.BlockIfBaseItems.Add(biGrp);
            stopBlockers.BlockIfBaseItems.Add(bi3);
            stopBlockers.ReturnCode = 1234;

            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnBlockers.Blockers element = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnBlockers.Blockers();
            element.StopBlockersGroup = stopBlockers;
            element.SuccessBlockersGroup = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnBlockers.SuccessBlockers();
            element.WarnBlockersGroup = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnBlockers.WarnBlockers();

            string expectedXml = 
                "<Blockers>" +
                "<SuccessBlockers>" + "</SuccessBlockers>" +
                "<StopBlockers ReturnCode=\"1234\">" + "<BlockIfGroup DisplayText=\"This group blocked:\">" + "<BlockIf ID=\"myBlock1\" DisplayText=\"Block You! 1\">" + "<AlwaysTrue>" + "</AlwaysTrue>" + "</BlockIf>" + "<BlockIf ID=\"myBlock2\" DisplayText=\"Block You! 2\">" + "<AlwaysTrue>" + "</AlwaysTrue>" + "</BlockIf>" + "</BlockIfGroup>" + "<BlockIf ID=\"myBlock3\" DisplayText=\"Block You! 3\">" + "<AlwaysTrue>" + "</AlwaysTrue>" + "</BlockIf>" + "</StopBlockers>" +
                "<WarnBlockers>" + "</WarnBlockers>" +
                "</Blockers>";
            string actualXml = XMLGenerator.GetXmlString(element);
            Assert.AreEqual(expectedXml, actualXml);
        }

        #endregion

        #region verify all of BurnEnterMaintenanceModeIf

        [TestMethod]
        public void ITBurnEnterMaintenanceModeIfEnterMaintenanceModeIf()
        {
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnEnterMaintenanceModeIf.EnterMaintenanceModeIf element = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnEnterMaintenanceModeIf.EnterMaintenanceModeIf();
            element.Expression = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.AlwaysTrue();

            string expectedXml = "<EnterMaintenanceModeIf>" + "<AlwaysTrue>" + "</AlwaysTrue>" + "</EnterMaintenanceModeIf>";
            string actualXml = XMLGenerator.GetXmlString(element);
            Assert.AreEqual(expectedXml, actualXml);
        }

        #endregion

        #region verify all of BurnSystemCheckElement
        
        [TestMethod]
        public void ITBurnSystemCheckElementSystemCheckEmpty()
        {
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnSystemCheckElement.SystemCheck element = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnSystemCheckElement.SystemCheck();
            string expectedXml = "<SystemCheck>" + "</SystemCheck>";
            string actualXml = XMLGenerator.GetXmlString(element);
            Assert.AreEqual(expectedXml, actualXml);
        }

        [TestMethod]
        public void ITBurnSystemCheckElementSystemCheckFull()
        {
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnSystemCheckElement.SystemCheck element = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnSystemCheckElement.SystemCheck();
            element.ProcessBlocks = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnSystemCheckElement.ProcessBlocksGroup();
            element.ServiceBlocks = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnSystemCheckElement.ServiceBlocksGroup();
            element.ProductDriveHints = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnSystemCheckElement.ProductDriveHints();

            string expectedXml =
                "<SystemCheck>" + 
                "<ProcessBlocks>" + "</ProcessBlocks>" + 
                "<ServiceBlocks>" + "</ServiceBlocks>" + 
                "<ProductDriveHints>" + "</ProductDriveHints>" + 
                "</SystemCheck>";
            string actualXml = XMLGenerator.GetXmlString(element);
            Assert.AreEqual(expectedXml, actualXml);
        }

        [TestMethod]
        public void ITBurnSystemCheckElementProcessBlocksEmpty()
        {
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnSystemCheckElement.ProcessBlocksGroup element = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnSystemCheckElement.ProcessBlocksGroup();
            string expectedXml = "<ProcessBlocks>" + "</ProcessBlocks>";
            string actualXml = XMLGenerator.GetXmlString(element);
            Assert.AreEqual(expectedXml, actualXml);
        }

        [TestMethod]
        public void ITBurnSystemCheckElementProcessBlocksFull()
        {
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnSystemCheckElement.ProcessBlocksGroup element = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnSystemCheckElement.ProcessBlocksGroup();
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnSystemCheckElement.ProcessBlocksGroup.ProcessBlock pb1 = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnSystemCheckElement.ProcessBlocksGroup.ProcessBlock();
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnSystemCheckElement.ProcessBlocksGroup.ProcessBlock pb2 = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnSystemCheckElement.ProcessBlocksGroup.ProcessBlock();
            pb1.ImageName = "Process1.exe";
            pb2.ImageName = "Process2.exe";
            element.ProcessBlocks.Add(pb1);
            element.ProcessBlocks.Add(pb2);
            string expectedXml =
                "<ProcessBlocks>" + 
                "<ProcessBlock ImageName=\"Process1.exe\">" + "</ProcessBlock>" + 
                "<ProcessBlock ImageName=\"Process2.exe\">" + "</ProcessBlock>" + 
                "</ProcessBlocks>";
            string actualXml = XMLGenerator.GetXmlString(element);
            Assert.AreEqual(expectedXml, actualXml);
        }

        [TestMethod]
        public void ITBurnSystemCheckElementServiceBlocksEmpty()
        {
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnSystemCheckElement.ServiceBlocksGroup element = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnSystemCheckElement.ServiceBlocksGroup();
            string expectedXml = "<ServiceBlocks>" + "</ServiceBlocks>";
            string actualXml = XMLGenerator.GetXmlString(element);
            Assert.AreEqual(expectedXml, actualXml);
        }

        [TestMethod]
        public void ITBurnSystemCheckElementServiceBlocksFull()
        {
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnSystemCheckElement.ServiceBlocksGroup element = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnSystemCheckElement.ServiceBlocksGroup();
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnSystemCheckElement.ServiceBlocksGroup.ServiceBlock sb1 = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnSystemCheckElement.ServiceBlocksGroup.ServiceBlock();
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnSystemCheckElement.ServiceBlocksGroup.ServiceBlock sb2 = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnSystemCheckElement.ServiceBlocksGroup.ServiceBlock();
            sb1.ServiceName = "Service1";
            sb2.ServiceName = "Service2";
            element.ServiceBlocks.Add(sb1);
            element.ServiceBlocks.Add(sb2);
            string expectedXml =
                "<ServiceBlocks>" + 
                "<ServiceBlock ServiceName=\"Service1\">" + "</ServiceBlock>" + 
                "<ServiceBlock ServiceName=\"Service2\">" + "</ServiceBlock>" + 
                "</ServiceBlocks>";
            string actualXml = XMLGenerator.GetXmlString(element);
            Assert.AreEqual(expectedXml, actualXml);
        }

        [TestMethod]
        public void ITBurnSystemCheckElementProductDriveHintsEmpty()
        {
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnSystemCheckElement.ProductDriveHints element = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnSystemCheckElement.ProductDriveHints();
            string expectedXml = "<ProductDriveHints>" + "</ProductDriveHints>";
            string actualXml = XMLGenerator.GetXmlString(element);
            Assert.AreEqual(expectedXml, actualXml);
        }

        [TestMethod]
        public void ITBurnSystemCheckElementProductDriveHintsFull()
        {
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnSystemCheckElement.ProductDriveHints element = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnSystemCheckElement.ProductDriveHints();
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnSystemCheckElement.ProductDriveHints.ComponentHint ch1 = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnSystemCheckElement.ProductDriveHints.ComponentHint();
            ch1.Guid = "GUID1";
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnSystemCheckElement.ProductDriveHints.ComponentHint ch2 = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnSystemCheckElement.ProductDriveHints.ComponentHint();
            ch2.Guid = "GUID2";
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnSystemCheckElement.ProductDriveHints.RegKeyHint rkh1 = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnSystemCheckElement.ProductDriveHints.RegKeyHint();
            rkh1.Location = "HKLM\regkey1";
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnSystemCheckElement.ProductDriveHints.RegKeyHint rkh2 = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnSystemCheckElement.ProductDriveHints.RegKeyHint();
            rkh2.Location = "HKLM\regkey2";
            element.Hints.Add(ch1);
            element.Hints.Add(ch2);
            element.Hints.Add(rkh1);
            element.Hints.Add(rkh2);
            string expectedXml =
                "<ProductDriveHints>" + 
                "<ComponentHint Guid=\"GUID1\">" + "</ComponentHint>" + 
                "<ComponentHint Guid=\"GUID2\">" + "</ComponentHint>" + 
                "<RegKeyHint Location=\"HKLM\regkey1\">" + "</RegKeyHint>" + 
                "<RegKeyHint Location=\"HKLM\regkey2\">" + "</RegKeyHint>" + 
                "</ProductDriveHints>";
            string actualXml = XMLGenerator.GetXmlString(element);
            Assert.AreEqual(expectedXml, actualXml);
        }

        #endregion

        #region verify all of ParameterInfoConfigurator.BurnInstallableItems.BurnGroupInstallableItems

        [TestMethod]
        public void ITBurnInstallableItemsBurnGroupInstallableItemsAgileMsiItem()
        {
            Assert.Inconclusive("I don't think AgileMsi Items will be supported in Burn engine so I'm not testing this or implementing it in the ParameterInfoConfigurator at this time");
            //ParameterInfoConfigurator.BurnInstallableItems.BurnGroupInstallableItems.AgileMsiItem element = new ParameterInfoConfigurator.BurnInstallableItems.BurnGroupInstallableItems.AgileMsiItem();
            //string expectedXml = "Not Implemented Yet";
            //string actualXml = XMLGenerator.GetXmlString(element);
            //Assert.AreEqual(expectedXml, actualXml);
        }

        [TestMethod]
        public void ITBurnInstallableItemsBurnGroupInstallableItemsPatchesItemEmpty()
        {
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnInstallableItems.BurnGroupInstallableItems.PatchesItem element = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnInstallableItems.BurnGroupInstallableItems.PatchesItem();
            string expectedXml =
                "<Patches>" + 
                "<ActionTable>" + 
                "<InstallAction IfPresent=\"noop\" IfAbsent=\"install\">" + "</InstallAction>" + 
                "<UninstallAction IfPresent=\"uninstall\" IfAbsent=\"noop\">" + "</UninstallAction>" + 
                "<RepairAction IfPresent=\"repair\" IfAbsent=\"install\">" + "</RepairAction>" + 
                "</ActionTable>" + 
                "</Patches>";
            string actualXml = XMLGenerator.GetXmlString(element);
            Assert.AreEqual(expectedXml, actualXml);
        }

        [TestMethod]
        public void ITBurnInstallableItemsBurnGroupInstallableItemsPatchesItemFull()
        {
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnInstallableItems.BurnGroupInstallableItems.PatchesItem element = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnInstallableItems.BurnGroupInstallableItems.PatchesItem();
            element.CustomErrorHandling = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemElements.CustomErrorHandling();
            element.IgnoreDownloadFailure = true;
            element.Rollback = true;
            element.RetryHelper.Add(new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemElements.RetryHelper());
            element.Msps.Add(new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnInstallableItems.MspItem());

            string expectedXml =
                "<Patches IgnoreDownloadFailure=\"True\" Rollback=\"True\">" + 
                "<ActionTable>" + 
                "<InstallAction IfPresent=\"noop\" IfAbsent=\"install\">" + "</InstallAction>" + 
                "<UninstallAction IfPresent=\"uninstall\" IfAbsent=\"noop\">" + "</UninstallAction>" + 
                "<RepairAction IfPresent=\"repair\" IfAbsent=\"install\">" + "</RepairAction>" + 
                "</ActionTable>" + 
                "<CustomErrorHandling>" + "</CustomErrorHandling>" + 
                "<RetryHelper DownloadSize=\"0\" SystemDriveSize=\"0\" InstalledProductSize=\"0\">" + 
                "<IsPresent>" + "</IsPresent>" + 
                "<ApplicableIf>" + "</ApplicableIf>" + 
                "<ActionTable>" + "<InstallAction IfPresent=\"noop\" IfAbsent=\"install\">" + "</InstallAction>" + "<UninstallAction IfPresent=\"uninstall\" IfAbsent=\"noop\">" + "</UninstallAction>" + "<RepairAction IfPresent=\"repair\" IfAbsent=\"install\">" + "</RepairAction>" + "</ActionTable>" + 
                "</RetryHelper>" + 
                "<MSP EstimatedInstallTime=\"0\" PerMachine=\"False\" DownloadSize=\"0\" SystemDriveSize=\"0\" InstalledProductSize=\"0\">" + 
                "<IsPresent>" + "</IsPresent>" + 
                "<ApplicableIf>" + "</ApplicableIf>" + 
                "<ActionTable>" + "<InstallAction IfPresent=\"noop\" IfAbsent=\"install\">" + "</InstallAction>" + "<UninstallAction IfPresent=\"uninstall\" IfAbsent=\"noop\">" + "</UninstallAction>" + "<RepairAction IfPresent=\"repair\" IfAbsent=\"install\">" + "</RepairAction>" + "</ActionTable>" + 
                "</MSP>" + 
                "</Patches>";
            string actualXml = XMLGenerator.GetXmlString(element);
            Assert.AreEqual(expectedXml, actualXml);
        }
        #endregion

        #region verify all of ParameterInfoConfigurator.BurnInstallableItems
        
        [TestMethod]
        public void ITBurnInstallableItemsExeItemEmpty()
        {
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnInstallableItems.ExeItem element = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnInstallableItems.ExeItem();
            string expectedXml = 
                "<Exe EstimatedInstallTime=\"0\" PerMachine=\"False\" DownloadSize=\"0\" SystemDriveSize=\"0\" InstalledProductSize=\"0\">" +
                "<IsPresent>" + "</IsPresent>" +
                "<ApplicableIf>" + "</ApplicableIf>" +
                "<ActionTable>" +
                "<InstallAction IfPresent=\"noop\" IfAbsent=\"install\">" + "</InstallAction>" +
                "<UninstallAction IfPresent=\"uninstall\" IfAbsent=\"noop\">" + "</UninstallAction>" +
                "<RepairAction IfPresent=\"repair\" IfAbsent=\"install\">" + "</RepairAction>" +
                "</ActionTable>" +
                "</Exe>";
            string actualXml = XMLGenerator.GetXmlString(element);
            Assert.AreEqual(expectedXml, actualXml);
        }

        [TestMethod]
        public void ITBurnInstallableItemsExeItemFull()
        {
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnInstallableItems.ExeItem element = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnInstallableItems.ExeItem();
            element.CanonicalTargetName = "MyCanonicalTargetName";
            element.Compressed = true;
            element.CompressedDownloadSize = 1234;
            element.CustomErrorHandling = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemElements.CustomErrorHandling();
            element.DownloadSize = 2345;
            element.EstimatedInstallTime = 111;
            element.ExeType = Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnInstallableItems.ExeItem.ExeTypeEnum.LocalExe;
            element.Sha1HashValue = "MyHashValue";
            element.IgnoreDownloadFailure = true;
            element.InstallCommandLine = "installCmdline";
            element.InstalledProductSize = 3333;
            element.LogFileHint = "*hint*";
            element.Name = "MyName.exe";
            element.PerMachine = true;
            element.RepairCommandLine = "repairCmdline";
            element.RetryHelper.Add(new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemElements.RetryHelper());
            element.Rollback = true;
            element.SourceFilePath = "MySrcFilePath";
            element.SystemDriveSize = 111;
            element.UninstallCommandLine = "uninstallCmdline";
            element.URL = "MyURL";

            string expectedXml = 
                "<Exe CacheId=\"MyHashValue\" CanonicalTargetName=\"MyCanonicalTargetName\" InstallCommandLine=\"installCmdline\" UninstallCommandLine=\"uninstallCmdline\" RepairCommandLine=\"repairCmdline\" " +
                "LogFileHint=\"*hint*\" ExeType=\"LocalExe\" EstimatedInstallTime=\"111\" Rollback=\"True\" PerMachine=\"True\" " +
                "Name=\"MyName.exe\" IgnoreDownloadFailure=\"True\" URL=\"MyURL\" DownloadSize=\"2345\" Sha1HashValue=\"MyHashValue\" " +
                "Compressed=\"True\" SystemDriveSize=\"111\" InstalledProductSize=\"3333\" CompressedDownloadSize=\"1234\">" + 
                "<CustomErrorHandling>" + "</CustomErrorHandling>" + 
                "<RetryHelper DownloadSize=\"0\" SystemDriveSize=\"0\" InstalledProductSize=\"0\">" +
                "<IsPresent>" + "</IsPresent>" +
                "<ApplicableIf>" + "</ApplicableIf>" +
                "<ActionTable>" + "<InstallAction IfPresent=\"noop\" IfAbsent=\"install\">" + "</InstallAction>" + "<UninstallAction IfPresent=\"uninstall\" IfAbsent=\"noop\">" + "</UninstallAction>" + "<RepairAction IfPresent=\"repair\" IfAbsent=\"install\">" + "</RepairAction>" + "</ActionTable>" +
                "</RetryHelper>" +
                "<IsPresent>" + "</IsPresent>" +
                "<ApplicableIf>" + "</ApplicableIf>" +
                "<ActionTable>" +
                "<InstallAction IfPresent=\"noop\" IfAbsent=\"install\">" + "</InstallAction>" +
                "<UninstallAction IfPresent=\"uninstall\" IfAbsent=\"noop\">" + "</UninstallAction>" +
                "<RepairAction IfPresent=\"repair\" IfAbsent=\"install\">" + "</RepairAction>" +
                "</ActionTable>" + "</Exe>";
            string actualXml = XMLGenerator.GetXmlString(element);
            Assert.AreEqual(expectedXml, actualXml);
        }

        [TestMethod]
        public void ITBurnInstallableItemsMsiItemEmpty()
        {
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnInstallableItems.MsiItem element = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnInstallableItems.MsiItem();
            string expectedXml = "" + 
                "<MSI CacheId=\"v\" EstimatedInstallTime=\"0\" PerMachine=\"False\" DownloadSize=\"0\" SystemDriveSize=\"0\" InstalledProductSize=\"0\">" + 
                "<IsPresent>" + "</IsPresent>" + 
                "<ApplicableIf>" + "</ApplicableIf>" + 
                "<ActionTable>" + 
                "<InstallAction IfPresent=\"noop\" IfAbsent=\"install\">" + "</InstallAction>" + 
                "<UninstallAction IfPresent=\"uninstall\" IfAbsent=\"noop\">" + "</UninstallAction>" + 
                "<RepairAction IfPresent=\"repair\" IfAbsent=\"install\">" + "</RepairAction>" + 
                "</ActionTable>" + 
                "</MSI>";
            string actualXml = XMLGenerator.GetXmlString(element);
            Assert.AreEqual(expectedXml, actualXml);
        }

        [TestMethod]
        public void ITBurnInstallableItemsMsiItemFull()
        {
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnInstallableItems.MsiItem element = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnInstallableItems.MsiItem();
            element.CanonicalTargetName = "MyCanonicalTargetName";
            element.Compressed = true;
            element.CompressedDownloadSize = 1234;
            element.CustomErrorHandling = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemElements.CustomErrorHandling();
            element.DownloadSize = 2345;
            element.EstimatedInstallTime = 222;
            element.Sha1HashValue = "MyHash";
            element.IgnoreDownloadFailure = true;
            element.InstalledProductSize = 3333;
            element.MSIOptions = "MyMsiOptions";
            element.MSIRepairOptions = "MyMsiRepairOptions";
            element.MSIUninstallOptions = "MyMsiUninstallOptions";
            element.Name = "MyName";
            element.PerMachine = true;
            element.ProductCode = "MyProductCode";
            element.RetryHelper.Add(new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemElements.RetryHelper());
            element.Rollback = true;
            element.SystemDriveSize = 111;
            element.URL = "MyUrl";

            string expectedXml = "" +
                "<MSI CacheId=\"MyProductCodev\" CanonicalTargetName=\"MyCanonicalTargetName\" ProductCode=\"MyProductCode\" " +
                "MSIOptions=\"MyMsiOptions\" MSIUninstallOptions=\"MyMsiUninstallOptions\" MSIRepairOptions=\"MyMsiRepairOptions\" " +
                "EstimatedInstallTime=\"222\" Rollback=\"True\" PerMachine=\"True\" Name=\"MyName\" " + 
                "IgnoreDownloadFailure=\"True\" URL=\"MyUrl\" DownloadSize=\"2345\" Sha1HashValue=\"MyHash\" Compressed=\"True\" " + 
                "SystemDriveSize=\"111\" InstalledProductSize=\"3333\" CompressedDownloadSize=\"1234\">" + 
                "<CustomErrorHandling>" + "</CustomErrorHandling>" + 
                "<RetryHelper DownloadSize=\"0\" SystemDriveSize=\"0\" InstalledProductSize=\"0\">" + 
                "<IsPresent>" + "</IsPresent>" + 
                "<ApplicableIf>" + "</ApplicableIf>" + 
                "<ActionTable>" + 
                "<InstallAction IfPresent=\"noop\" IfAbsent=\"install\">" + "</InstallAction>" + 
                "<UninstallAction IfPresent=\"uninstall\" IfAbsent=\"noop\">" + "</UninstallAction>" + 
                "<RepairAction IfPresent=\"repair\" IfAbsent=\"install\">" + "</RepairAction>" + 
                "</ActionTable>" + 
                "</RetryHelper>" + 
                "<IsPresent>" + "</IsPresent>" + 
                "<ApplicableIf>" + "</ApplicableIf>" + 
                "<ActionTable>" + 
                "<InstallAction IfPresent=\"noop\" IfAbsent=\"install\">" + "</InstallAction>" + 
                "<UninstallAction IfPresent=\"uninstall\" IfAbsent=\"noop\">" + "</UninstallAction>" + 
                "<RepairAction IfPresent=\"repair\" IfAbsent=\"install\">" + "</RepairAction>" + 
                "</ActionTable>" + 
                "</MSI>";
            string actualXml = XMLGenerator.GetXmlString(element);
            Assert.AreEqual(expectedXml, actualXml);
        }

        [TestMethod]
        public void ITBurnInstallableItemsMspItemEmpty()
        {
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnInstallableItems.MspItem element = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnInstallableItems.MspItem();
            string expectedXml =
                "<MSP EstimatedInstallTime=\"0\" PerMachine=\"False\" DownloadSize=\"0\" SystemDriveSize=\"0\" InstalledProductSize=\"0\">" +
                "<IsPresent>" + "</IsPresent>" +
                "<ApplicableIf>" + "</ApplicableIf>" +
                "<ActionTable>" +
                "<InstallAction IfPresent=\"noop\" IfAbsent=\"install\">" + "</InstallAction>" +
                "<UninstallAction IfPresent=\"uninstall\" IfAbsent=\"noop\">" + "</UninstallAction>" +
                "<RepairAction IfPresent=\"repair\" IfAbsent=\"install\">" + "</RepairAction>" +
                "</ActionTable>" +
                "</MSP>";
            string actualXml = XMLGenerator.GetXmlString(element);
            Assert.AreEqual(expectedXml, actualXml);
        }

        [TestMethod]
        public void ITBurnInstallableItemsMspItemFull()
        {
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnInstallableItems.MspItem element = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnInstallableItems.MspItem();
            element.Compressed = true;
            element.CompressedDownloadSize = 1234;
            element.CustomErrorHandling = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemElements.CustomErrorHandling();
            element.DownloadSize = 2345;
            element.EstimatedInstallTime = 111;
            element.Sha1HashValue = "MyHash";
            element.IgnoreDownloadFailure = true;
            element.InstalledProductSize = 2222;
            element.Name = "MyName.msp";
            element.PatchCode = "MyPatchCode";
            element.PerMachine = true;
            element.RetryHelper.Add(new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemElements.RetryHelper());
            element.Rollback = true;
            element.SystemDriveSize = 111;
            element.URL = "MyURL";

            string expectedXml = 
                "<MSP CacheId=\"MyPatchCode\" PatchCode=\"MyPatchCode\" EstimatedInstallTime=\"111\" Rollback=\"True\" PerMachine=\"True\" Name=\"MyName.msp\" " +
                "IgnoreDownloadFailure=\"True\" URL=\"MyURL\" DownloadSize=\"2345\" Sha1HashValue=\"MyHash\" Compressed=\"True\" " + 
                "SystemDriveSize=\"111\" InstalledProductSize=\"2222\" CompressedDownloadSize=\"1234\">" + 
                "<CustomErrorHandling>" + "</CustomErrorHandling>" + 
                "<RetryHelper DownloadSize=\"0\" SystemDriveSize=\"0\" InstalledProductSize=\"0\">" +
                "<IsPresent>" + "</IsPresent>" + "<ApplicableIf>" +
                "</ApplicableIf>" +
                "<ActionTable>" + "<InstallAction IfPresent=\"noop\" IfAbsent=\"install\">" + "</InstallAction>" + "<UninstallAction IfPresent=\"uninstall\" IfAbsent=\"noop\">" + "</UninstallAction>" + "<RepairAction IfPresent=\"repair\" IfAbsent=\"install\">" + "</RepairAction>" + "</ActionTable>" +
                "</RetryHelper>" +
                "<IsPresent>" + "</IsPresent>" +
                "<ApplicableIf>" + "</ApplicableIf>" +
                "<ActionTable>" +
                "<InstallAction IfPresent=\"noop\" IfAbsent=\"install\">" + "</InstallAction>" +
                "<UninstallAction IfPresent=\"uninstall\" IfAbsent=\"noop\">" + "</UninstallAction>" +
                "<RepairAction IfPresent=\"repair\" IfAbsent=\"install\">" + "</RepairAction>" +
                "</ActionTable>" +
                "</MSP>";
            string actualXml = XMLGenerator.GetXmlString(element);
            Assert.AreEqual(expectedXml, actualXml);
        }


        #endregion

        #region verify all of BurnItems.BurnItemsGroup

        [TestMethod]
        public void ITBurnItemsBurnItemsGroupEmpty()
        {
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemsGroup element = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemsGroup();
            string expectedXml =
                "<Items>" + "</Items>";
            string actualXml = XMLGenerator.GetXmlString(element);
            Assert.AreEqual(expectedXml, actualXml);
        }

        [TestMethod]
        public void ITBurnItemsBurnItemsGroupFull()
        {
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemsGroup element = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemsGroup();
            element.Items.Add(new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnInstallableItems.ExeItem());
            element.Items.Add(new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnInstallableItems.MsiItem());
            element.Items.Add(new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnInstallableItems.MspItem());
            element.Items.Add(new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnNonInstallableItems.CleanupBlockItem());
            element.Items.Add(new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnNonInstallableItems.FileItem());
            element.Items.Add(new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnNonInstallableItems.RelatedProductItem());
            element.Items.Add(new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnNonInstallableItems.ServiceControlItem());


            string expectedXml =
                "<Items>" + 
                "<Exe EstimatedInstallTime=\"0\" PerMachine=\"False\" DownloadSize=\"0\" SystemDriveSize=\"0\" InstalledProductSize=\"0\">" + "<IsPresent>" + "</IsPresent>" + "<ApplicableIf>" + "</ApplicableIf>" + "<ActionTable>" + "<InstallAction IfPresent=\"noop\" IfAbsent=\"install\">" + "</InstallAction>" + "<UninstallAction IfPresent=\"uninstall\" IfAbsent=\"noop\">" + "</UninstallAction>" + "<RepairAction IfPresent=\"repair\" IfAbsent=\"install\">" + "</RepairAction>" + "</ActionTable>" + "</Exe>" + 
                "<MSI CacheId=\"v\" EstimatedInstallTime=\"0\" PerMachine=\"False\" DownloadSize=\"0\" SystemDriveSize=\"0\" InstalledProductSize=\"0\">" + "<IsPresent>" + "</IsPresent>" + "<ApplicableIf>" + "</ApplicableIf>" + "<ActionTable>" + "<InstallAction IfPresent=\"noop\" IfAbsent=\"install\">" + "</InstallAction>" + "<UninstallAction IfPresent=\"uninstall\" IfAbsent=\"noop\">" + "</UninstallAction>" + "<RepairAction IfPresent=\"repair\" IfAbsent=\"install\">" + "</RepairAction>" + "</ActionTable>" + "</MSI>" + 
                "<MSP EstimatedInstallTime=\"0\" PerMachine=\"False\" DownloadSize=\"0\" SystemDriveSize=\"0\" InstalledProductSize=\"0\">" + "<IsPresent>" + "</IsPresent>" + "<ApplicableIf>" + "</ApplicableIf>" + "<ActionTable>" + "<InstallAction IfPresent=\"noop\" IfAbsent=\"install\">" + "</InstallAction>" + "<UninstallAction IfPresent=\"uninstall\" IfAbsent=\"noop\">" + "</UninstallAction>" + "<RepairAction IfPresent=\"repair\" IfAbsent=\"install\">" + "</RepairAction>" + "</ActionTable>" + "</MSP>" + 
                "<CleanupBlock InstalledProductSize=\"0\">" + "<IsPresent>" + "</IsPresent>" + "<ApplicableIf>" + "</ApplicableIf>" + "<ActionTable>" + "<InstallAction IfPresent=\"noop\" IfAbsent=\"install\">" + "</InstallAction>" + "<UninstallAction IfPresent=\"uninstall\" IfAbsent=\"noop\">" + "</UninstallAction>" + "<RepairAction IfPresent=\"repair\" IfAbsent=\"install\">" + "</RepairAction>" + "</ActionTable>" + "</CleanupBlock>" + 
                "<File DownloadSize=\"0\" SystemDriveSize=\"0\" InstalledProductSize=\"0\">" + "<IsPresent>" + "</IsPresent>" + "<ApplicableIf>" + "</ApplicableIf>" + "<ActionTable>" + "<InstallAction IfPresent=\"noop\" IfAbsent=\"install\">" + "</InstallAction>" + "<UninstallAction IfPresent=\"uninstall\" IfAbsent=\"noop\">" + "</UninstallAction>" + "<RepairAction IfPresent=\"repair\" IfAbsent=\"install\">" + "</RepairAction>" + "</ActionTable>" + "</File>" + 
                "<RelatedProductItem EstimatedInstallTime=\"0\">" + "<IsPresent>" + "</IsPresent>" + "<ApplicableIf>" + "</ApplicableIf>" + "<ActionTable>" + "<InstallAction IfPresent=\"noop\" IfAbsent=\"install\">" + "</InstallAction>" + "<UninstallAction IfPresent=\"uninstall\" IfAbsent=\"noop\">" + "</UninstallAction>" + "<RepairAction IfPresent=\"repair\" IfAbsent=\"install\">" + "</RepairAction>" + "</ActionTable>" + "</RelatedProductItem>" + 
                "<ServiceControlItem Control=\"Start\" EstimatedInstallTime=\"0\">" + "<IsPresent>" + "</IsPresent>" + "<ApplicableIf>" + "</ApplicableIf>" + "<ActionTable>" + "<InstallAction IfPresent=\"noop\" IfAbsent=\"install\">" + "</InstallAction>" + "<UninstallAction IfPresent=\"uninstall\" IfAbsent=\"noop\">" + "</UninstallAction>" + "<RepairAction IfPresent=\"repair\" IfAbsent=\"install\">" + "</RepairAction>" + "</ActionTable>" + "</ServiceControlItem>" + 
                "</Items>";
            string actualXml = XMLGenerator.GetXmlString(element);
            Assert.AreEqual(expectedXml, actualXml);
        }

        #endregion

        #region verify all of BurnItems.BurnItemElements

        [TestMethod]
        public void ITBurnItemsBurnItemElementsActionTableElementEmpty()
        {
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemElements.ActionTableElement element = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemElements.ActionTableElement();
            string expectedXml =
                "<ActionTable>" +
                "<InstallAction IfPresent=\"noop\" IfAbsent=\"install\">" + "</InstallAction>" +
                "<UninstallAction IfPresent=\"uninstall\" IfAbsent=\"noop\">" + "</UninstallAction>" +
                "<RepairAction IfPresent=\"repair\" IfAbsent=\"install\">" + "</RepairAction>" +
                "</ActionTable>";
            string actualXml = XMLGenerator.GetXmlString(element);
            Assert.AreEqual(expectedXml, actualXml);
        }

        [TestMethod]
        public void ITBurnItemsBurnItemElementsActionTableElementFull()
        {
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemElements.ActionTableElement element = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemElements.ActionTableElement(
                Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemElements.ActionTableElement.ActionTableSubelement.ActionType.noop, 
                Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemElements.ActionTableElement.ActionTableSubelement.ActionType.noop, 
                Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemElements.ActionTableElement.ActionTableSubelement.ActionType.noop, 
                Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemElements.ActionTableElement.ActionTableSubelement.ActionType.noop, 
                Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemElements.ActionTableElement.ActionTableSubelement.ActionType.noop, 
                Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemElements.ActionTableElement.ActionTableSubelement.ActionType.noop);
            string expectedXml =
                "<ActionTable>" +
                "<InstallAction IfPresent=\"noop\" IfAbsent=\"noop\">" + "</InstallAction>" +
                "<UninstallAction IfPresent=\"noop\" IfAbsent=\"noop\">" + "</UninstallAction>" +
                "<RepairAction IfPresent=\"noop\" IfAbsent=\"noop\">" + "</RepairAction>" +
                "</ActionTable>";
            string actualXml = XMLGenerator.GetXmlString(element);
            Assert.AreEqual(expectedXml, actualXml);
        }

        [TestMethod]
        public void ITBurnItemsDefaultActionTableElement()
        {
            string expectedXml = "" +
                "<ActionTable>" +
                "<InstallAction IfPresent=\"noop\" IfAbsent=\"install\">" + "</InstallAction>" +
                "<UninstallAction IfPresent=\"uninstall\" IfAbsent=\"noop\">" + "</UninstallAction>" +
                "<RepairAction IfPresent=\"repair\" IfAbsent=\"install\">" + "</RepairAction>" +
                "</ActionTable>";

            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemElements.ActionTableElement actionTable = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemElements.ActionTableElement();
            string actualXml = XMLGenerator.GetXmlString(actionTable);
            Assert.AreEqual(expectedXml, actualXml);
        }

        [TestMethod]
        public void ITBurnItemsBurnItemElementsApplicableIfEmpty()
        {
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemElements.ApplicableIf element = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemElements.ApplicableIf();
            string expectedXml = "<ApplicableIf>" + "</ApplicableIf>";
            string actualXml = XMLGenerator.GetXmlString(element);
            Assert.AreEqual(expectedXml, actualXml);
        }

        [TestMethod]
        public void ITBurnItemsBurnItemElementsApplicableIfFull()
        {
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemElements.ApplicableIf element = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemElements.ApplicableIf(new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.AlwaysTrue());
            string expectedXml = "<ApplicableIf>" + "<AlwaysTrue>" + "</AlwaysTrue>" + "</ApplicableIf>";
            string actualXml = XMLGenerator.GetXmlString(element);
            Assert.AreEqual(expectedXml, actualXml);
        }

        [TestMethod]
        public void ITBurnItemsBurnItemElementsCustomErrorHandlingEmpty()
        {
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemElements.CustomErrorHandling element = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemElements.CustomErrorHandling();
            string expectedXml = "<CustomErrorHandling>" + "</CustomErrorHandling>";
            string actualXml = XMLGenerator.GetXmlString(element);
            Assert.AreEqual(expectedXml, actualXml);
        }

        [TestMethod]
        public void ITBurnItemsBurnItemElementsCustomErrorHandlingFull()
        {
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemElements.CustomErrorHandling element = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemElements.CustomErrorHandling();
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemElements.CustomErrorHandling.CustomError ceSuccess = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemElements.CustomErrorHandling.CustomError();
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemElements.CustomErrorHandling.CustomError ceFailure = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemElements.CustomErrorHandling.CustomError();
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemElements.CustomErrorHandling.CustomError ceRetry = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemElements.CustomErrorHandling.CustomError();

            ceSuccess.CustomErrorAction = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemElements.CustomErrorHandling.Success();
            ceSuccess.MSIErrorMessage = "MyMSIErrorMessage";
            ceSuccess.ReturnCode = "0x1234";

            ceFailure.CustomErrorAction = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemElements.CustomErrorHandling.Failure();
            ceFailure.MSIErrorMessage = "MyMSIErrorMessage2";
            ceFailure.ReturnCode = "0x2345";

            ceRetry.CustomErrorAction = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemElements.CustomErrorHandling.Retry();
            ceRetry.MSIErrorMessage = "MyMSIErrorMessage3";
            ceRetry.ReturnCode = "0x2345";
            ((Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemElements.CustomErrorHandling.Retry)ceRetry.CustomErrorAction).Delay = 1;
            ((Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemElements.CustomErrorHandling.Retry)ceRetry.CustomErrorAction).Limit = 2;
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemElements.CustomErrorHandling.ItemRef ir = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemElements.CustomErrorHandling.ItemRef();
            ir.CommandLine = "MyCommandLine";
            ir.LogFile = "MyLogFile";
            ir.Name = "MyName";
            ((Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemElements.CustomErrorHandling.Retry)ceRetry.CustomErrorAction).ItemRef = ir;

            element.CustomErrors.Add(ceSuccess);
            element.CustomErrors.Add(ceFailure);
            element.CustomErrors.Add(ceRetry);

            string expectedXml =
                "<CustomErrorHandling>" +
                "<CustomError ReturnCode=\"0x1234\" MSIErrorMessage=\"MyMSIErrorMessage\">" +
                "<Success>" + "</Success>" +
                "</CustomError>" +
                "<CustomError ReturnCode=\"0x2345\" MSIErrorMessage=\"MyMSIErrorMessage2\">" +
                "<Failure>" + "</Failure>" +
                "</CustomError>" +
                "<CustomError ReturnCode=\"0x2345\" MSIErrorMessage=\"MyMSIErrorMessage3\">" +
                "<Retry Delay=\"1\" Limit=\"2\">" +
                "<ItemRef Name=\"MyName\" CommandLine=\"MyCommandLine\" LogFile=\"MyLogFile\">" + "</ItemRef>" +
                "</Retry>" +
                "</CustomError>" + "</CustomErrorHandling>";
            string actualXml = XMLGenerator.GetXmlString(element);
            Assert.AreEqual(expectedXml, actualXml);
        }

        [TestMethod]
        public void ITBurnItemsBurnItemElementsIsPresentEmpty()
        {
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemElements.IsPresent element = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemElements.IsPresent();
            string expectedXml = "<IsPresent>" + "</IsPresent>";
            string actualXml = XMLGenerator.GetXmlString(element);
            Assert.AreEqual(expectedXml, actualXml);
        }

        [TestMethod]
        public void ITBurnItemsBurnItemElementsIsPresentFull()
        {
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemElements.IsPresent element = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemElements.IsPresent(new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.AlwaysTrue());
            string expectedXml = "<IsPresent>" + "<AlwaysTrue>" + "</AlwaysTrue>" + "</IsPresent>";
            string actualXml = XMLGenerator.GetXmlString(element);
            Assert.AreEqual(expectedXml, actualXml);
        }

        [TestMethod]
        public void ITBurnItemsBurnItemElementsRetryHelperEmpty()
        {
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemElements.RetryHelper element = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemElements.RetryHelper();
            string expectedXml = 
                "<RetryHelper DownloadSize=\"0\" SystemDriveSize=\"0\" InstalledProductSize=\"0\">" +
                "<IsPresent>" + "</IsPresent>" +
                "<ApplicableIf>" + "</ApplicableIf>" +
                "<ActionTable>" +
                "<InstallAction IfPresent=\"noop\" IfAbsent=\"install\">" + "</InstallAction>" +
                "<UninstallAction IfPresent=\"uninstall\" IfAbsent=\"noop\">" + "</UninstallAction>" +
                "<RepairAction IfPresent=\"repair\" IfAbsent=\"install\">" + "</RepairAction>" +
                "</ActionTable>" +
                "</RetryHelper>";
            string actualXml = XMLGenerator.GetXmlString(element);
            Assert.AreEqual(expectedXml, actualXml);
        }

        [TestMethod]
        public void ITBurnItemsBurnItemElementsRetryHelperFull()
        {
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemElements.RetryHelper element = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemElements.RetryHelper();
            element.Name = "MyName";
            element.URL = "MyURL";
            element.DownloadSize = 1234;
            element.SystemDriveSize = 2345;
            element.InstalledProductSize = 3456;
            element.Sha1HashValue = "MyHashValue";
            element.Name = "MyName";
            element.CanonicalTargetName = "MyCanonicalTargetName";
            element.CommandLine = "MyCommandLine";
            element.LogFileHint = "MyLogFileHint";
            element.ExeType = Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnInstallableItems.ExeItem.ExeTypeEnum.LocalExe;

            string expectedXml =
                "<RetryHelper CommandLine=\"MyCommandLine\" CanonicalTargetName=\"MyCanonicalTargetName\" LogFileHint=\"MyLogFileHint\" ExeType=\"LocalExe\" Name=\"MyName\" URL=\"MyURL\" DownloadSize=\"1234\" Sha1HashValue=\"MyHashValue\" SystemDriveSize=\"2345\" InstalledProductSize=\"3456\">" + 
                "<IsPresent>" + "</IsPresent>" + 
                "<ApplicableIf>" + "</ApplicableIf>" + 
                "<ActionTable>" + 
                "<InstallAction IfPresent=\"noop\" IfAbsent=\"install\">" + "</InstallAction>" + 
                "<UninstallAction IfPresent=\"uninstall\" IfAbsent=\"noop\">" + "</UninstallAction>" + 
                "<RepairAction IfPresent=\"repair\" IfAbsent=\"install\">" + "</RepairAction>" + 
                "</ActionTable>" + 
                "</RetryHelper>";

            string actualXml = XMLGenerator.GetXmlString(element);
            Assert.AreEqual(expectedXml, actualXml);
        }

        #endregion

        #region verify all of BurnItems.BurnNonInstallableItems

        [TestMethod]
        public void ITBurnItemsBurnNonInstallableItemsCleanBlockItemEmpty()
        {
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnNonInstallableItems.CleanupBlockItem element = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnNonInstallableItems.CleanupBlockItem();
            string expectedXml =
                "<CleanupBlock InstalledProductSize=\"0\">" + 
                "<IsPresent>" + "</IsPresent>" + 
                "<ApplicableIf>" + "</ApplicableIf>" + 
                "<ActionTable>" + 
                "<InstallAction IfPresent=\"noop\" IfAbsent=\"install\">" + "</InstallAction>" + 
                "<UninstallAction IfPresent=\"uninstall\" IfAbsent=\"noop\">" + "</UninstallAction>" + 
                "<RepairAction IfPresent=\"repair\" IfAbsent=\"install\">" + "</RepairAction>" + 
                "</ActionTable>" + 
                "</CleanupBlock>";
            string actualXml = XMLGenerator.GetXmlString(element);
            Assert.AreEqual(expectedXml, actualXml);
        }

        [TestMethod]
        public void ITBurnItemsBurnNonInstallableItemsCleanBlockItemFull()
        {
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnNonInstallableItems.CleanupBlockItem element = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnNonInstallableItems.CleanupBlockItem();
            element.ApplicableIf = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemElements.ApplicableIf(new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.AlwaysTrue());
            element.CanonicalTargetName = "MyCanonicalTargetName";
            element.DoUnAdvertiseFeaturesOnRemovePatch = true;
            element.InstalledProductSize = 1234;
            element.IsPresent = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemElements.IsPresent(new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.AlwaysTrue());
            element.Name = "MyName";
            element.PerMachine = true;

            List<Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnNonInstallableItems.CleanupBlockItem.RemovePatchElement> removePatches = new List<Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnNonInstallableItems.CleanupBlockItem.RemovePatchElement>();
            removePatches.Add(new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnNonInstallableItems.CleanupBlockItem.RemovePatchElement("patchCode1"));
            removePatches.Add(new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnNonInstallableItems.CleanupBlockItem.RemovePatchElement("patchCode2"));
            element.RemovePatch = removePatches;

            List<Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnNonInstallableItems.CleanupBlockItem.RemoveProductElement> removeProducts = new List<Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnNonInstallableItems.CleanupBlockItem.RemoveProductElement>();
            removeProducts.Add(new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnNonInstallableItems.CleanupBlockItem.RemoveProductElement("prodCode1"));
            removeProducts.Add(new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnNonInstallableItems.CleanupBlockItem.RemoveProductElement("prodCode2"));
            element.RemoveProduct = removeProducts;

            List<Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnNonInstallableItems.CleanupBlockItem.UnAdvertiseFeaturesElement> unAdvertiseFeatures = new List<Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnNonInstallableItems.CleanupBlockItem.UnAdvertiseFeaturesElement>();
            unAdvertiseFeatures.Add(new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnNonInstallableItems.CleanupBlockItem.UnAdvertiseFeaturesElement("prodCode3"));
            unAdvertiseFeatures.Add(new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnNonInstallableItems.CleanupBlockItem.UnAdvertiseFeaturesElement("prodCode4"));
            element.UnAdvertiseFeatures = unAdvertiseFeatures;

            string expectedXml =
                "<CleanupBlock Name=\"MyName\" CanonicalTargetName=\"MyCanonicalTargetName\" InstalledProductSize=\"1234\" DoUnAdvertiseFeaturesOnRemovePatch=\"True\" PerMachine=\"True\">" +
                "<RemovePatch PatchCode=\"patchCode1\">" + "</RemovePatch>" +
                "<RemovePatch PatchCode=\"patchCode2\">" + "</RemovePatch>" +
                "<UnAdvertiseFeatures ProductCode=\"prodCode3\">" + "</UnAdvertiseFeatures>" +
                "<UnAdvertiseFeatures ProductCode=\"prodCode4\">" + "</UnAdvertiseFeatures>" +
                "<RemoveProduct ProductCode=\"prodCode1\">" + "</RemoveProduct>" +
                "<RemoveProduct ProductCode=\"prodCode2\">" + "</RemoveProduct>" +
                "<IsPresent>" + "<AlwaysTrue>" + "</AlwaysTrue>" + "</IsPresent>" +
                "<ApplicableIf>" + "<AlwaysTrue>" + "</AlwaysTrue>" + "</ApplicableIf>" +
                "<ActionTable>" +
                "<InstallAction IfPresent=\"noop\" IfAbsent=\"install\">" + "</InstallAction>" +
                "<UninstallAction IfPresent=\"uninstall\" IfAbsent=\"noop\">" + "</UninstallAction>" +
                "<RepairAction IfPresent=\"repair\" IfAbsent=\"install\">" + "</RepairAction>" +
                "</ActionTable>" +
                "</CleanupBlock>";
            string actualXml = XMLGenerator.GetXmlString(element);
            Assert.AreEqual(expectedXml, actualXml);
        }

        [TestMethod]
        public void ITBurnItemsBurnNonInstallableItemsFileItemEmpty()
        {
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnNonInstallableItems.FileItem element = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnNonInstallableItems.FileItem();
            string expectedXml = "" + 
                "<File DownloadSize=\"0\" SystemDriveSize=\"0\" InstalledProductSize=\"0\">" +
                "<IsPresent>" + "</IsPresent>" +
                "<ApplicableIf>" + "</ApplicableIf>" +
                "<ActionTable>" +
                "<InstallAction IfPresent=\"noop\" IfAbsent=\"install\">" + "</InstallAction>" +
                "<UninstallAction IfPresent=\"uninstall\" IfAbsent=\"noop\">" + "</UninstallAction>" +
                "<RepairAction IfPresent=\"repair\" IfAbsent=\"install\">" + "</RepairAction>" +
                "</ActionTable>" +
                "</File>";
            string actualXml = XMLGenerator.GetXmlString(element);
            Assert.AreEqual(expectedXml, actualXml);
        }

        [TestMethod]
        public void ITBurnItemsBurnNonInstallableItemsFileItemFull()
        {
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnNonInstallableItems.FileItem element = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnNonInstallableItems.FileItem();
            element.ActionTable = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemElements.ActionTableElement();
            element.ApplicableIf = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemElements.ApplicableIf();
            element.Compressed = true;
            element.CompressedDownloadSize = 12345;
            element.DownloadSize = 6789;
            element.Sha1HashValue = "1234123412341234";
            element.IgnoreDownloadFailure = true;
            element.InstalledProductSize = 1234;
            element.IsPresent = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemElements.IsPresent();
            element.Name = "MyFile";
            element.SourceFilePath = "C:\\MyFileSrc";
            element.SystemDriveSize = 123;
            element.URL = "http://foobar";

            string expectedXml = "" +
                "<File Name=\"MyFile\" IgnoreDownloadFailure=\"True\" URL=\"http://foobar\" DownloadSize=\"6789\" Sha1HashValue=\"1234123412341234\" Compressed=\"True\" SystemDriveSize=\"123\" InstalledProductSize=\"1234\" CompressedDownloadSize=\"12345\">" + 
                "<IsPresent>" + "</IsPresent>" + 
                "<ApplicableIf>" + "</ApplicableIf>" + 
                "<ActionTable>" + 
                "<InstallAction IfPresent=\"noop\" IfAbsent=\"install\">" + "</InstallAction>" + 
                "<UninstallAction IfPresent=\"uninstall\" IfAbsent=\"noop\">" + "</UninstallAction>" + 
                "<RepairAction IfPresent=\"repair\" IfAbsent=\"install\">" + "</RepairAction>" + 
                "</ActionTable>" + 
                "</File>";
            string actualXml = XMLGenerator.GetXmlString(element);
            Assert.AreEqual(expectedXml, actualXml);
        }

        [TestMethod]
        public void ITBurnItemsBurnNonInstallableItemsRelatedProductItemEmpty()
        {
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnNonInstallableItems.RelatedProductItem element = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnNonInstallableItems.RelatedProductItem();
            string expectedXml =
                "<RelatedProductItem EstimatedInstallTime=\"0\">" + 
                "<IsPresent>" + "</IsPresent>" + 
                "<ApplicableIf>" + "</ApplicableIf>" + 
                "<ActionTable>" + 
                "<InstallAction IfPresent=\"noop\" IfAbsent=\"install\">" + "</InstallAction>" + 
                "<UninstallAction IfPresent=\"uninstall\" IfAbsent=\"noop\">" + "</UninstallAction>" + 
                "<RepairAction IfPresent=\"repair\" IfAbsent=\"install\">" + "</RepairAction>" + 
                "</ActionTable>" + 
                "</RelatedProductItem>";
            string actualXml = XMLGenerator.GetXmlString(element);
            Assert.AreEqual(expectedXml, actualXml);
        }

        [TestMethod]
        public void ITBurnItemsBurnNonInstallableItemsRelatedProductItemFull()
        {
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnNonInstallableItems.RelatedProductItem element = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnNonInstallableItems.RelatedProductItem();
            element.ApplicableIf = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemElements.ApplicableIf(new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.AlwaysTrue());
            element.CanonicalTargetName = "MyCanonicalTargetName";
            element.CustomErrorHandling = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemElements.CustomErrorHandling();
            element.EstimatedInstallTime = 1234;
            element.IsPresent = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemElements.IsPresent(new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.AlwaysTrue());
            element.MSIninstallOptions = "MyMsiIntallOptions";
            element.MSIRepairOptions = "MyMsiRepairOptions";
            element.Name = "MyName";
            element.PerMachine = true;
            element.RetryHelper.Add(new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemElements.RetryHelper());
            element.SourceFilePath = "not-applicable";

            string expectedXml =
                "<RelatedProductItem Name=\"MyName\" MSIninstallOptions=\"MyMsiIntallOptions\" MSIRepairOptions=\"MyMsiRepairOptions\" EstimatedInstallTime=\"1234\" CanonicalTargetName=\"MyCanonicalTargetName\" PerMachine=\"True\">" + 
                "<CustomErrorHandling>" + "</CustomErrorHandling>" + 
                "<RetryHelper DownloadSize=\"0\" SystemDriveSize=\"0\" InstalledProductSize=\"0\">" + "<IsPresent>" + "</IsPresent>" + "<ApplicableIf>" + "</ApplicableIf>" + "<ActionTable>" + "<InstallAction IfPresent=\"noop\" IfAbsent=\"install\">" + "</InstallAction>" + "<UninstallAction IfPresent=\"uninstall\" IfAbsent=\"noop\">" + "</UninstallAction>" + "<RepairAction IfPresent=\"repair\" IfAbsent=\"install\">" + "</RepairAction>" + "</ActionTable>" + "</RetryHelper>" + 
                "<IsPresent>" + "<AlwaysTrue>" + "</AlwaysTrue>" + "</IsPresent>" + 
                "<ApplicableIf>" + "<AlwaysTrue>" + "</AlwaysTrue>" + "</ApplicableIf>" +
                "<ActionTable>" +
                "<InstallAction IfPresent=\"noop\" IfAbsent=\"install\">" + "</InstallAction>" +
                "<UninstallAction IfPresent=\"uninstall\" IfAbsent=\"noop\">" + "</UninstallAction>" +
                "<RepairAction IfPresent=\"repair\" IfAbsent=\"install\">" + "</RepairAction>" +
                "</ActionTable>" +
                "</RelatedProductItem>";
            string actualXml = XMLGenerator.GetXmlString(element);
            Assert.AreEqual(expectedXml, actualXml);
        }

        [TestMethod]
        public void ITBurnItemsBurnNonInstallableItemsServiceControlItemEmpty()
        {
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnNonInstallableItems.ServiceControlItem element = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnNonInstallableItems.ServiceControlItem();
            string expectedXml = "<ServiceControlItem Control=\"Start\" EstimatedInstallTime=\"0\">" + 
                "<IsPresent>" + "</IsPresent>" + 
                "<ApplicableIf>" + "</ApplicableIf>" + 
                "<ActionTable>" + 
                "<InstallAction IfPresent=\"noop\" IfAbsent=\"install\">" + "</InstallAction>" + 
                "<UninstallAction IfPresent=\"uninstall\" IfAbsent=\"noop\">" + "</UninstallAction>" + 
                "<RepairAction IfPresent=\"repair\" IfAbsent=\"install\">" + "</RepairAction>" + 
                "</ActionTable>" + 
                "</ServiceControlItem>";
            string actualXml = XMLGenerator.GetXmlString(element);
            Assert.AreEqual(expectedXml, actualXml);
        }

        [TestMethod]
        public void ITBurnItemsBurnNonInstallableItemsServiceControlItemFull()
        {
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnNonInstallableItems.ServiceControlItem element = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnNonInstallableItems.ServiceControlItem();
            element.Control = Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnNonInstallableItems.ServiceControlItem.ServiceControlType.Stop;
            element.CanonicalTargetName = "MyName";
            element.ApplicableIf = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemElements.ApplicableIf(new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.AlwaysTrue());
            element.CustomErrorHandling = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemElements.CustomErrorHandling();
            element.EstimatedInstallTime = 1234;
            element.IsPresent = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemElements.IsPresent(new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.AlwaysTrue());
            element.Name = "MyServiceControl";
            element.PerMachine = false;
            element.RetryHelper.Add(new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemElements.RetryHelper());
            element.SourceFilePath = "not-applicable";

            string expectedXml =
                "<ServiceControlItem Name=\"MyServiceControl\" Control=\"Stop\" EstimatedInstallTime=\"1234\" CanonicalTargetName=\"MyName\" PerMachine=\"False\">" + 
                "<CustomErrorHandling>" + "</CustomErrorHandling>" + 
                "<RetryHelper DownloadSize=\"0\" SystemDriveSize=\"0\" InstalledProductSize=\"0\">" + "<IsPresent>" + "</IsPresent>" + "<ApplicableIf>" + "</ApplicableIf>" + "<ActionTable>" + "<InstallAction IfPresent=\"noop\" IfAbsent=\"install\">" + "</InstallAction>" + "<UninstallAction IfPresent=\"uninstall\" IfAbsent=\"noop\">" + "</UninstallAction>" + "<RepairAction IfPresent=\"repair\" IfAbsent=\"install\">" + "</RepairAction>" + "</ActionTable>" + "</RetryHelper>" +
                "<IsPresent>" + "<AlwaysTrue>" + "</AlwaysTrue>" + "</IsPresent>" +
                "<ApplicableIf>" + "<AlwaysTrue>" + "</AlwaysTrue>" + "</ApplicableIf>" +
                "<ActionTable>" +
                "<InstallAction IfPresent=\"noop\" IfAbsent=\"install\">" + "</InstallAction>" +
                "<UninstallAction IfPresent=\"uninstall\" IfAbsent=\"noop\">" + "</UninstallAction>" +
                "<RepairAction IfPresent=\"repair\" IfAbsent=\"install\">" + "</RepairAction>" +
                "</ActionTable>" +
                "</ServiceControlItem>";
            string actualXml = XMLGenerator.GetXmlString(element);
            Assert.AreEqual(expectedXml, actualXml);
        }
        
        #endregion

        #region verify all of BurnConfigurationElement

        [TestMethod]
        public void ITBurnConfigurationElementBurnConfigurationEmpty()
        {
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnConfigurationElement.BurnConfiguration element = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnConfigurationElement.BurnConfiguration();
            string expectedXml = "<Configuration>" + "</Configuration>";
            string actualXml = XMLGenerator.GetXmlString(element);
            Assert.AreEqual(expectedXml, actualXml);
        }

        [TestMethod]
        public void ITBurnConfigurationElementBurnConfigurationFull()
        {
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnConfigurationElement.BurnConfiguration.BurnCommandLineSwitch cls1 = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnConfigurationElement.BurnConfiguration.BurnCommandLineSwitch();
            cls1.Name = "Switch1";
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnConfigurationElement.BurnConfiguration.BurnCommandLineSwitch cls2 = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnConfigurationElement.BurnConfiguration.BurnCommandLineSwitch();
            cls2.Name = "Switch2";

            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnConfigurationElement.BurnConfiguration.BurnAdditionalCommandLineSwitches addCLS = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnConfigurationElement.BurnConfiguration.BurnAdditionalCommandLineSwitches();
            addCLS.CommandLineSwitches.Add(cls1);
            addCLS.CommandLineSwitches.Add(cls2);

            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnConfigurationElement.BurnConfiguration.BurnCommandLineSwitch cls3 = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnConfigurationElement.BurnConfiguration.BurnCommandLineSwitch();
            cls3.Name = "Switch3";
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnConfigurationElement.BurnConfiguration.BurnCommandLineSwitch cls4 = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnConfigurationElement.BurnConfiguration.BurnCommandLineSwitch();
            cls4.Name = "Switch4";

            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnConfigurationElement.BurnConfiguration.BurnDisabledCommandLineSwitches disableCLS = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnConfigurationElement.BurnConfiguration.BurnDisabledCommandLineSwitches();
            disableCLS.CommandLineSwitches.Add(cls3);
            disableCLS.CommandLineSwitches.Add(cls4);

            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnConfigurationElement.BurnConfiguration.BurnBlockingMutex bmn = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnConfigurationElement.BurnConfiguration.BurnBlockingMutex();
            bmn.Name = "MyMutexName";

            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnConfigurationElement.BurnConfiguration.BurnFilesInUseSettingPrompt fiusp = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnConfigurationElement.BurnConfiguration.BurnFilesInUseSettingPrompt();
            fiusp.Prompt = true;

            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnConfigurationElement.BurnConfiguration.BurnDownloadInstallSetting sd = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnConfigurationElement.BurnConfiguration.BurnDownloadInstallSetting();
            sd.SerialDownload = true;

            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnConfigurationElement.BurnConfiguration.BurnUserExperienceDataCollection uxdc = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnConfigurationElement.BurnConfiguration.BurnUserExperienceDataCollection();
            uxdc.Policy = Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnConfigurationElement.BurnConfiguration.BurnUserExperienceDataCollection.UxPolicy.UserControlled;


            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnConfigurationElement.BurnConfiguration.BurnAcceptableCertificates.BurnCertificate cert = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnConfigurationElement.BurnConfiguration.BurnAcceptableCertificates.BurnCertificate();
            cert.AuthorityKeyIdentifier = "MyAKI";
            cert.Name = "MyName";
            cert.Thumbprint = "MyThumbprint";

            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnConfigurationElement.BurnConfiguration.BurnAcceptableCertificates acceptibleCerts = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnConfigurationElement.BurnConfiguration.BurnAcceptableCertificates();
            acceptibleCerts.Certificates.Add(cert);
            
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnConfigurationElement.BurnConfiguration element = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnConfigurationElement.BurnConfiguration();
            element.AdditionalCommandLineSwitches = addCLS;
            element.DisabledCommandLineSwitches = disableCLS;
            element.BlockingMutex = bmn;
            element.FilesInUseSettingPrompt = fiusp;
            element.DownloadInstallSetting = sd;
            element.UserExperienceDataCollection = uxdc;
            element.AcceptableCertificates = acceptibleCerts;

            string expectedXml = "" +
                "<Configuration>" + 
                "<DisabledCommandLineSwitches>" + 
                "<CommandLineSwitch Name=\"Switch3\">" + "</CommandLineSwitch>" + 
                "<CommandLineSwitch Name=\"Switch4\">" + "</CommandLineSwitch>" + 
                "</DisabledCommandLineSwitches>" + 
                "<AdditionalCommandLineSwitches>" + 
                "<CommandLineSwitch Name=\"Switch1\">" + "</CommandLineSwitch>" + 
                "<CommandLineSwitch Name=\"Switch2\">" + "</CommandLineSwitch>" + 
                "</AdditionalCommandLineSwitches>" + 
                "<UserExperienceDataCollection Policy=\"UserControlled\">" + "</UserExperienceDataCollection>" + 
                "<DownloadInstallSetting SerialDownload=\"True\">" + "</DownloadInstallSetting>" + 
                "<BlockingMutex Name=\"MyMutexName\">" + "</BlockingMutex>" + 
                "<FilesInUseSettingPrompt Prompt=\"True\">" + "</FilesInUseSettingPrompt>" + 
                "<AcceptableCertificates>" +
                "<Certificate Name=\"MyName\" AuthorityKeyIdentifier=\"MyAKI\" Thumbprint=\"MyThumbprint\">" +
                "</Certificate>" + "</AcceptableCertificates>" +
                "</Configuration>";
            string actualXml = XMLGenerator.GetXmlString(element);
            Assert.AreEqual(expectedXml, actualXml);
        }

        #endregion

        #region verify all of UI

        [TestMethod]
        public void ITBurnUiElementEmpty()
        {
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnUiElement.BurnUiElement element = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnUiElement.BurnUiElement();
            string expectedXml = "<UI>" + "</UI>";
            string actualXml = XMLGenerator.GetXmlString(element);
            Assert.AreEqual(expectedXml, actualXml);
        }

        [TestMethod]
        public void ITBurnUiElementFull()
        {
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnUiElement.BurnUiElement element = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnUiElement.BurnUiElement();
            element.Dll = "setupui.dll";
            element.Name = "MyName";
            element.Version = "1.0";

            string expectedXml = "<UI Dll=\"setupui.dll\" Name=\"MyName\" Version=\"1.0\">" + "</UI>";
            string actualXml = XMLGenerator.GetXmlString(element);
            Assert.AreEqual(expectedXml, actualXml);
        }

        #endregion

        #region verify all of Ux

        [TestMethod]
        public void ITBurnUxElementEmpty()
        {
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnUxElement.BurnUxElement element = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnUxElement.BurnUxElement();
            string expectedXml = "<Ux>" + "</Ux>";
            string actualXml = XMLGenerator.GetXmlString(element);
            Assert.AreEqual(expectedXml, actualXml);
        }

        [TestMethod]
        public void ITBurnUxElementFull()
        {
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnUxElement.BurnUxElement element = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnUxElement.BurnUxElement();
            element.UxDllPayloadId = "MyUxDllPayloadId";

            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnUxElement.BurnUxElement.BurnPayload payload = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnUxElement.BurnUxElement.BurnPayload();
            payload.Id = "MyId";
            payload.FilePath = "MyFilePath";
            payload.Packaging = "MyPackaging";
            payload.SourcePath = "MySourcePath";

            element.Payloads.Add(payload);

            string expectedXml = "" +
                "<Ux UxDllPayloadId=\"MyUxDllPayloadId\">" +
                "<Payload Id=\"MyId\" FilePath=\"MyFilePath\" Packaging=\"MyPackaging\" SourcePath=\"MySourcePath\">" + "</Payload>" +
                "</Ux>";
            string actualXml = XMLGenerator.GetXmlString(element);
            Assert.AreEqual(expectedXml, actualXml);
        }

        #endregion

        #region verify all of Registration

        [TestMethod]
        public void ITBurnRegistrationElementEmpty()
        {
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnRegistrationElement.BurnRegistrationElement element = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnRegistrationElement.BurnRegistrationElement();
            string expectedXml = "<Registration>" + "</Registration>";
            string actualXml = XMLGenerator.GetXmlString(element);
            Assert.AreEqual(expectedXml, actualXml);
        }

        [TestMethod]
        public void ITBurnRegistrationElementFull()
        {
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnRegistrationElement.BurnRegistrationElement element = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnRegistrationElement.BurnRegistrationElement();
            element.ExecutableName = "MyExecutableName";
            element.Id = "MyId";
            element.PerMachine = "MyPerMachine";

            string expectedXml = "<Registration Id=\"MyId\" ExecutableName=\"MyExecutableName\" PerMachine=\"MyPerMachine\">" + "</Registration>";
            string actualXml = XMLGenerator.GetXmlString(element);
            Assert.AreEqual(expectedXml, actualXml);
        }

        #endregion

        #region verify all of ParameterInfoConfigurator.Setup

        [TestMethod]
        public void ITSetupEmpty()
        {
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnSetupElement.Setup element = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnSetupElement.Setup();
            string expectedXml = "<Setup>" + "</Setup>";
            string actualXml = XMLGenerator.GetXmlString(element);
            Assert.AreEqual(expectedXml, actualXml);
        }

        [TestMethod]
        public void ITSetupDefault()
        {
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnSetupElement.Setup element = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnSetupElement.Setup(true);
            string expectedXml =
                "<Setup xmlns=\"http://schemas.microsoft.com/Setup/2008/01/im\" xmlns:im=\"http://schemas.microsoft.com/Setup/2008/01/im\" SetupVersion=\"1.0\">" + 
                "<UI>" + "</UI>" + 
                "<Configuration>" + "</Configuration>" + 
                "<EnterMaintenanceModeIf>" + "</EnterMaintenanceModeIf>" + 
                "<Blockers>" + "</Blockers>" + 
                "<Items>" + "</Items>" + 
                "<SystemCheck>" + "</SystemCheck>" + 
                "</Setup>";
            string actualXml = XMLGenerator.GetXmlString(element);
            Assert.AreEqual(expectedXml, actualXml);
        }
        
        #endregion

        [TestMethod]
        public void ITCreateObjectTest()
        {
            ObjectGenerator obj = new ObjectGenerator();

            string expectedXml =
               "<ActionTable>" +
               "<InstallAction IfPresent=\"noop\" IfAbsent=\"noop\">" + "</InstallAction>" +
               "<UninstallAction IfPresent=\"noop\" IfAbsent=\"noop\">" + "</UninstallAction>" +
               "<RepairAction IfPresent=\"noop\" IfAbsent=\"noop\">" + "</RepairAction>" +
               "</ActionTable>";

            XmlDocument xmlDoc = new XmlDocument();
            xmlDoc.LoadXml(expectedXml);

            object outputObj = obj.GetObjectFromXml(xmlDoc.FirstChild);

            string actualXml = XMLGenerator.GetXmlString(outputObj);

            Assert.AreEqual(expectedXml, actualXml);
        }
    }
}
