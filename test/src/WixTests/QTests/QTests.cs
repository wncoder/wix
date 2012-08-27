//-----------------------------------------------------------------------
// <copyright file="QTests.cs" company="Outercurve Foundation">
//   Copyright (c) 2004, Outercurve Foundation.
//   This software is released under Microsoft Reciprocal License (MS-RL).
//   The license and further copyright text can be found in the file
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
// <summary>
//     Contains test methods for the QTests
// </summary>
//-----------------------------------------------------------------------

namespace WixTest.Tests
{
    using System;
    using System.IO;
    using System.Reflection;
    using System.Text;

    using Microsoft.VisualStudio.TestTools.UnitTesting;
    using WixTest;

    /// <summary>
    /// Contains test methods for the QTests
    /// </summary>
    [TestClass]
    public partial class QTests : WixTests
    {
        /// <summary>
        /// The location of the QTests
        /// </summary>
        private static readonly string testDirectory = Environment.ExpandEnvironmentVariables(@"%WIX_ROOT%\examples\test");

        /// <summary>
        /// A WixUnit object used to run the QTests
        /// </summary>
        private static WixUnit wixUnit;

        /// <summary>
        /// Initializes the classes WixUnit object
        /// </summary>
        /// <param name="testContext">A TestContext</param>
        [ClassInitialize]
        public static void ClassInitialize(TestContext testContext)
        {
            wixUnit = new WixUnit();
            wixUnit.WorkingDirectory = QTests.testDirectory;
            wixUnit.VerboseOutput = true;
            wixUnit.TestFile = Path.Combine(QTests.testDirectory, "tests.xml");
        }
    }
}
