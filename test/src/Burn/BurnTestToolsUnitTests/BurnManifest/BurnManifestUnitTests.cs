//-----------------------------------------------------------------------
// <copyright file="BurnManifestUnitTests.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>
//     - Tests for the BurnManifest (part of the Burn test infrastructure)
// </summary>
//-----------------------------------------------------------------------

namespace BurnTestToolsUnitTests.UnitTests.BurnManifest
{
    using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.BurnManifestOM;
    using Microsoft.VisualStudio.TestTools.UnitTesting;
    using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.ParameterInfoConfiguratorEngine;

    [TestClass]
    public class BurnManifestUnitTests
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


        [TestMethod]
        public void IT_BurnManifest()
        {
            BurnManifestElement bmElement = new BurnManifestElement();
            bmElement.Xmlns = "myNameSpace";

            bmElement.Stub = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.BurnManifestOM.Stub.StubElement();
            bmElement.Stub.SourceFile = "MyStub.exe";

            bmElement.UX = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.BurnManifestOM.UX.UxElement();
            bmElement.UX.SourceFile = "MyUx.dll";
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.BurnManifestOM.UX.ResourceElement res1 = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.BurnManifestOM.UX.ResourceElement();
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.BurnManifestOM.UX.ResourceElement res2 = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.BurnManifestOM.UX.ResourceElement();
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.BurnManifestOM.UX.ResourceElement res3 = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.BurnManifestOM.UX.ResourceElement();
            res1.SourceFile = "MyRes1.dll";
            res2.SourceFile = "MyRes2.dll";
            res3.SourceFile = "MyRes3.dll";
            bmElement.UX.Resources.Add(res1);
            bmElement.UX.Resources.Add(res2);
            bmElement.UX.Resources.Add(res3);

            bmElement.Registration = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.BurnManifestOM.Registration.RegistrationElement();
            bmElement.Registration.Arp = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.BurnManifestOM.Registration.RegistrationElement.ArpElement();
            bmElement.Registration.Arp.Name = "MyBurnPackageName";
            bmElement.Registration.Arp.Version = "MyBurnPackageVersion";

            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.BurnManifestOM.Chain.ExePackageElement exe = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.BurnManifestOM.Chain.ExePackageElement();
            exe.FileName = "MyFilename.exe";

            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.BurnManifestOM.Chain.MsiPackageElement msi = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.BurnManifestOM.Chain.MsiPackageElement();
            msi.FileName = "MyFilename.msi";

            bmElement.Chain = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.BurnManifestOM.Chain.ChainElement();
            bmElement.Chain.Packages.Add(exe);
            bmElement.Chain.Packages.Add(msi);

            string expectedXml =
                "<BurnManifest xmlns=\"myNameSpace\">" +
                "<Stub SourceFile=\"MyStub.exe\">" + "</Stub>" +
                "<UX SourceFile=\"MyUx.dll\">" +
                "<Resource SourceFile=\"MyRes1.dll\">" + "</Resource>" +
                "<Resource SourceFile=\"MyRes2.dll\">" + "</Resource>" +
                "<Resource SourceFile=\"MyRes3.dll\">" + "</Resource>" +
                "</UX>" +
                "<Registration><Arp Name=\"MyBurnPackageName\" Version=\"MyBurnPackageVersion\"></Arp></Registration>" +
                "<Chain>" +
                "<ExePackage FileName=\"MyFilename.exe\">" + "</ExePackage>" +
                "<MsiPackage FileName=\"MyFilename.msi\">" + "</MsiPackage>" +
                "</Chain>" +
                "</BurnManifest>";
            string actualXml = XMLGenerator.GetXmlString(bmElement);
            Assert.AreEqual(expectedXml, actualXml);
        }
    }
}
