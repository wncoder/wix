//-----------------------------------------------------------------------
// <copyright file="QTests.cs" company="Microsoft">
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
