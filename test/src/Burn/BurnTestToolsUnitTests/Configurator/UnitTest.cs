//-----------------------------------------------------------------------
// <copyright file="UnitTest.cs" company="Microsoft">
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
//     - Tests for the Configurator (part of the Burn test infrastructure)
// </summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests.Burn.BurnTestToolsUnitTests.Configurator
{
    using System.Collections.Generic;
    using System.Xml;
    using Microsoft.VisualStudio.TestTools.UnitTesting;
    using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.Generator;

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

        [TestMethod]
        public void ITCreateObjectTest()
        {
            ObjectGenerator obj = new ObjectGenerator();

            string expectedXml = "<UX SourceFile=\"UXEntryPoint.dll\"></UX>";

            XmlDocument xmlDoc = new XmlDocument();
            xmlDoc.LoadXml(expectedXml);

            object outputObj = obj.GetObjectFromXml(xmlDoc.FirstChild);

            string actualXml = XMLGenerator.GetXmlString(outputObj);

            Assert.AreEqual(expectedXml, actualXml);
        }
    }
}
