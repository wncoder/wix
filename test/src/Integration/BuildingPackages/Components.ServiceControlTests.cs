//-----------------------------------------------------------------------
// <copyright file="Components.ServiceControlTests.cs" company="Microsoft">
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
//     Tests for controlling services that are installed
// </summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests.Integration.BuildingPackages.Components
{
    using System;
    using System.IO;
    using System.Text;
    using System.Collections.Generic;
    using Microsoft.VisualStudio.TestTools.UnitTesting;

    using Microsoft.Tools.WindowsInstallerXml.Test;

    /// <summary>
    /// Tests for controlling services that are installed
    /// </summary>
    [TestClass]
    public class ServiceControlTests
    {
        private static readonly string TestDataDirectory = Environment.ExpandEnvironmentVariables(@"%WIX_ROOT%\test\data\Integration\BuildingPackages\Components\ServiceControlTests");

        [TestMethod]
        [Description("Verify that a service can be configured and that values are defaulted correctly")]
        [Priority(1)]
        [Ignore]
        public void ServiceControl()
        {
        }

        [TestMethod]
        [Description("Verify that there is an error if the service has an invalid Name")]
        [Priority(1)]
        [Ignore]
        public void InvalidServiceControlName()
        {
        }

        [TestMethod]
        [Description("Verify that Wait can be set to yes or no")]
        [Priority(1)]
        [Ignore]
        public void Wait()
        {
        }

        [TestMethod]
        [Description("Verify that there is an error for conflicting actions. See code comments for details.")]
        [Priority(1)]
        [Ignore]
        public void ConflictingActions()
        {
            // Remove and Start on install
            // Remove and Start on both
            
            // Stop and Start on install
            // Stop and Start on both

            // Remove and stop on uninstall

        }

        [TestMethod]
        [Description("Verify that a service argument can be specified")]
        [Priority(1)]
        [Ignore]
        public void ServiceArgument()
        {
        }

        [TestMethod]
        [Description("Verify that there is an error if the ServiceArgument element is missing")]
        [Priority(1)]
        [Ignore]
        public void MissingServiceArgument()
        {
        }

        [TestMethod]
        [Description("Verify that there is an error if multiple ServiceArgument elements are used")]
        [Priority(1)]
        [Ignore]
        public void MultipleServiceArguments()
        {
        }

        [TestMethod]
        [Description("Verify that DeleteServices action is scheduled with ServiceControl attribute Remove=both")]
        [Priority(2)]
        public void RemoveBoth()
        {
            string msi = Builder.BuildPackage(Path.Combine(ServiceControlTests.TestDataDirectory, @"RemoveBoth\product.wxs"));
      
            string query = "SELECT `Action` FROM `InstallExecuteSequence` WHERE `Action` = 'DeleteServices'";
            Verifier.VerifyQuery(msi, query, "DeleteServices");
        }
    }
}
