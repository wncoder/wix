//-----------------------------------------------------------------------
// <copyright file="Components.ServiceInstallTests.cs" company="Microsoft">
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
//     Tests for configuring services that will be installed
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
    /// Tests for configuring Services of a component
    /// </summary>
    [TestClass]
    public class ServiceInstallTests
    {
        [TestMethod]
        [Description("Verify that a service can be added and that values are defaulted correctly")]
        [Priority(1)]
        [Ignore]
        public void ServiceInstall()
        {
        }

        [TestMethod]
        [Description("Verify that a user account can be specified when the ServiceType is ownProcess")]
        [Priority(1)]
        [Ignore]
        public void ValidAccount()
        {
        }

        [TestMethod]
        [Description("Verify that a user account cannot be specified when the ServiceType is not ownProcess")]
        [Priority(1)]
        [Ignore]
        public void InvalidAccount()
        {
        }

        [TestMethod]
        [Description("Verify that any characters, eg. '/' and '\' can be specified as command line arguments for the service")]
        [Priority(1)]
        [Ignore]
        public void Arguments()
        {
        }

        [TestMethod]
        [Description("Verify that the service description is null if this attribute's value is Yes")]
        [Priority(1)]
        [Ignore]
        public void EraseDescription1()
        {
        }

        [TestMethod]
        [Description("Verify that the service description is not ignored if this attribute's value is No")]
        [Priority(1)]
        [Ignore]
        public void EraseDescription2()
        {
        }

        [TestMethod]
        [Description("Verify that the service description is null if this attribute's value is Yes")]
        [Priority(1)]
        [Ignore]
        public void EraseDescription()
        {
        }

        [TestMethod]
        [Description("Verify that there is an error if the ErrorControl attribute is missing")]
        [Priority(1)]
        [Ignore]
        public void MissingErrorControl()
        {
        }

        [TestMethod]
        [Description("Verify that ErrorControl can be properly set to its valid values: ignore, normal, critical")]
        [Priority(1)]
        [Ignore]
        public void ValidErrorControl()
        {
        }

        [TestMethod]
        [Description("Verify that ErrorControl cannot be set to an invalid value")]
        [Priority(1)]
        [Ignore]
        public void InvalidErrorControl()
        {
        }

        [TestMethod]
        [Description("Verify that Interactive can be set to Yes or No")]
        [Priority(1)]
        [Ignore]
        public void Interactive()
        {
        }

        [TestMethod]
        [Description("Verify that there is an error if the value of Name is an invalid service name")]
        [Priority(1)]
        [Ignore]
        public void Name()
        {
        }

        [TestMethod]
        [Description("Verify that there is an error the Password attribute is set but the Account attribute is not")]
        [Priority(1)]
        [Ignore]
        public void Password()
        {
        }

        [TestMethod]
        [Description("Verify that all Windows Installer supported enumerations for Start are allowed (auto, demand, disabled)")]
        [Priority(1)]
        [Ignore]
        public void SupportedStartValues()
        {
        }

        [TestMethod]
        [Description("Verify that all Windows Installer unsupported enumerations for Start are not allowed (boot, system)")]
        [Priority(1)]
        [Ignore]
        public void UnsupportedStartValues()
        {
        }

        [TestMethod]
        [Description("Verify that all Windows Installer supported enumerations for Type are allowed (ownProcess, shareProcess)")]
        [Priority(1)]
        [Ignore]
        public void SupportedTypes()
        {
        }

        [TestMethod]
        [Description("Verify that all Windows Installer supported enumerations for Type are not allowed (kernelDriver, systemDriver)")]
        [Priority(1)]
        [Ignore]
        public void UnsupportedTypes()
        {
        }

        [TestMethod]
        [Description("Verify that there is an error if the Component keypath is a directory")]
        [Priority(1)]
        [Ignore]
        public void InvalidServiceExecutable1()
        {
        }

        [TestMethod]
        [Description("Verify that there is an error if the Component keypath is a registry")]
        [Priority(1)]
        [Ignore]
        public void InvalidServiceExecutable2()
        {
        }

        [TestMethod]
        [Description("Verify that the service executable is the first file in the component if a keypath is not specified")]
        [Priority(1)]
        [Ignore]
        public void ValidServiceExecutable1()
        {
        }

        [TestMethod]
        [Description("Verify that the service executable is the file marked as keypath even if that file is not the first in the component")]
        [Priority(1)]
        [Ignore]
        public void ValidServiceExecutable2()
        {
        }

        [TestMethod]
        [Description("Verify that the Id of ServiceDependency can be the name of a previously installed service")]
        [Priority(1)]
        [Ignore]
        public void ServiceDependencyId1()
        {
        }

        [TestMethod]
        [Description("Verify that the Id of ServiceDependency can be the foreign key referring to another ServiceInstall/@Id")]
        [Priority(1)]
        [Ignore]
        public void ServiceDependencyId2()
        {
        }

        [TestMethod]
        [Description("Verify that the Id of ServiceDependency can be a group of services")]
        [Priority(1)]
        [Ignore]
        public void ServiceDependencyId3()
        {
        }

        [TestMethod]
        [Description("Verify that a ServiceInstall element can have multiple ServiceDependency children")]
        [Priority(1)]
        [Ignore]
        public void MultipleServiceDependencies()
        {
        }

        [TestMethod]
        [Description("Verify that there is an error if ServiceDependency/@Id is a group of services, but ServiceDependency/@Group is 'No'")]
        [Priority(1)]
        [Ignore]
        public void ServiceDependencyMissingGroupAttr()
        {
        }

        [TestMethod]
        [Description("Verify that there is an error if ServiceDependency/@Id is not a group of services, but ServiceDependency/@Group is 'No'")]
        [Priority(1)]
        [Ignore]
        public void ServiceDependencyInvalidGroupAttr()
        {
        }
    }
}
