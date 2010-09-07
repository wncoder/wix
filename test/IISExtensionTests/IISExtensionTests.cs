//-----------------------------------------------------------------------
// <copyright file="IISExtensionTests.cs" company="Microsoft">
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
//     - Contains methods that are shared across this assembly
//     - Performs some initialization before the tests are run
// </summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests.Extensions.IISExtension
{
    using System;
    using Microsoft.VisualStudio.TestTools.UnitTesting;

    /// <summary>
    /// Contains variables and methods used by this test assembly
    /// </summary>
    [TestClass]
    public class IISExtensionTests
    {
        /// <summary>
        /// Perform some initialization before the tests are run
        /// </summary>
        /// <param name="context">A TestContext</param>
        [AssemblyInitialize]
        public static void AssemblyInitialize(TestContext context)
        {
            WixTests.AssemblyInitialize(context);
        }

        /// <summary>
        /// Perform some cleanup after the tests are run
        /// </summary>
        [AssemblyCleanup]
        public static void AssemblyCleanup()
        {
            WixTests.AssemblyCleanup();
        }
    }
}
