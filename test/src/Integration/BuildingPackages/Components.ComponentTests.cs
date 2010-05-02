//-----------------------------------------------------------------------
// <copyright file="Components.ComponentTests.cs" company="Microsoft">
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
//     Tests for Components
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
    /// Tests for Components
    /// </summary>
    [TestClass]
    public class ComponentTests
    {
        private static readonly string TestDataDirectory = Environment.ExpandEnvironmentVariables(@"%WIX_ROOT%\test\data\Integration\BuildingPackages\Components\ComponentTests");

        [TestMethod]
        [Description("Verify that a simple Component can be defined and that the expected default values are set")]
        [Priority(1)]
        public void SimpleComponent()
        {
            string sourceFile = Path.Combine(ComponentTests.TestDataDirectory, @"SimpleComponent\product.wxs");
            string msi = Builder.BuildPackage(sourceFile, "test.msi");
            Verifier.VerifyResults(Path.Combine(ComponentTests.TestDataDirectory, @"SimpleComponent\expected.msi"), msi, "Component");
        }

        [TestMethod]
        [Description("Verify that Components/ComponentGroups can be referenced and that ComponentGroups can be nested")]
        [Priority(1)]
        public void ComponentRefsAndGroups()
        {
            string sourceFile = Path.Combine(ComponentTests.TestDataDirectory, @"ComponentRefsAndGroups\product.wxs");
            string msi = Builder.BuildPackage(sourceFile, "test.msi");
            Verifier.VerifyResults(Path.Combine(ComponentTests.TestDataDirectory, @"ComponentRefsAndGroups\expected.msi"), msi, "Component", "Directory", "FeatureComponents");
        }

        [TestMethod]
        [Description("Verify that a floating component can be defined. The component ties itself to a Directory and a Feature through its attributes.")]
        [Priority(1)]
        public void FloatingComponent()
        {
            string sourceFile = Path.Combine(ComponentTests.TestDataDirectory, @"FloatingComponent\product.wxs");
            string msi = Builder.BuildPackage(sourceFile, "test.msi");

            // Verify that Component1 was created and has the correct Directory
            string query = "SELECT `Directory_` FROM `Component` WHERE `Component`='Component1'";
            Verifier.VerifyQuery(msi, query, "WixTestFolder");
        }

        [TestMethod]
        [Description("Verify that there is an error if a floating component references an undefined directory")]
        [Priority(3)]
        public void InvalidFloatingComponent()
        {
            string sourceFile = Path.Combine(ComponentTests.TestDataDirectory, @"InvalidFloatingComponent\product.wxs");

            string wixobj = Candle.Compile(sourceFile);
            
            Light light = new Light();
            light.ObjectFiles.Add(wixobj);
            light.ExpectedExitCode = 94;
            light.ExpectedWixMessages.Add(new WixMessage(94, "Unresolved reference to symbol 'Directory:UndefinedDirectory' in section 'Product:*'.", WixMessage.MessageTypeEnum.Error));
            light.Run();
        }

        [TestMethod]
        [Description("Verify that circular references are detected amongst ComponentRefs and ComponentGroupRefs")]
        [Priority(2)]
        [Ignore]
        public void CircularReferences()
        {
        }

        [TestMethod]
        [Description("Verify that there is an error for an invalid component Id")]
        [Priority(2)]
        [Ignore]
        public void InvalidId()
        {
        }

        [TestMethod]
        [Description("Verify that a component's GUID can be set to an empty string to make it an unmanaged component")]
        [Priority(2)]
        [Ignore]
        public void UnmanagedComponent()
        {
        }

        [TestMethod]
        [Description("Verify that there is an error for a component without a GUID")]
        [Priority(2)]
        [Ignore]
        public void MissingComponentGuid()
        {
        }

        [TestMethod]
        [Description("Verify that a component's resources are tied to the component's DiskId")]
        [Priority(2)]
        [Ignore]
        public void DiskIdInheritance()
        {
        }

        [TestMethod]
        [Description("Verify that there is an error for an invalid component GUID")]
        [Priority(2)]
        [Ignore]
        public void InvalidComponentGuid()
        {
        }

        [TestMethod]
        [Description("Verify that a component's resources are tied to the component's DiskId unless DiskId is explicitly set on a resource")]
        [Priority(2)]
        [Ignore]
        public void DiskIdInheritanceOverride()
        {
        }

        [TestMethod]
        [Description("Verify that a component's directory can be set as the keypath")]
        [Priority(2)]
        [Ignore]
        public void ComponentKeyPath()
        {
        }

        [TestMethod]
        [Description("Verify that a component can be shared")]
        [Priority(2)]
        [Ignore]
        public void Shared()
        {
        }

        [TestMethod]
        [Description("Verify that a component can be marked as 64 bit")]
        [Priority(2)]
        [Ignore]
        public void Win64()
        {
        }

        [TestMethod]
        [Description("Verify that the Win64 attribute overrides the command line -platforms switch. All scenarios should be verified.")]
        [Priority(2)]
        [Ignore]
        public void Win64Override()
        {
        }

        [TestMethod]
        [Description("Verify that generated GUIDs for components take into account the bitness (32-bit vs 64-bit)")]
        [Priority(2)]
        [TestProperty("Bug Link", "http://sourceforge.net/tracker/index.php?func=detail&aid=1833513&group_id=105970&atid=642714")]
        public void Win64ComponentGeneratedGUID()
        {
            string testDirectory = Path.Combine(ComponentTests.TestDataDirectory,"Win64ComponentGeneratedGUID");
            string msi_32bit = Builder.BuildPackage(testDirectory, "product.wxs", "product_32.msi", " -dIsWin64=no", "");
            string msi_64bit = Builder.BuildPackage(testDirectory, "product.wxs", "product_64.msi", " -dIsWin64=yes  -arch x64", "");

            // get the component GUIDs from the resulting msi's
            string query = "SELECT `ComponentId` FROM `Component` WHERE `Component` = 'Component1'";
            string component_32bit_GUID = Verifier.Query(msi_32bit, query);
            string component_64bit_GUID = Verifier.Query(msi_64bit, query);
            Assert.AreNotEqual(component_32bit_GUID, component_64bit_GUID);
        }

        [TestMethod]
        [Description("Verify that there is an error if the component GUID is set to PUT-GUID-HERE")]
        [Priority(3)]
        [Ignore]
        public void PutGuidHere()
        {
        }

        [TestMethod]
        [Description("Verify that a floating component's directory can be set as the keypath")]
        [Priority(3)]
        [Ignore]
        public void FloatingComponentKeyPath()
        {
        }

        [TestMethod]
        [Description("Verify that there is an error if a component is tied to an undefined feature")]
        [Priority(3)]
        [Ignore]
        public void InvalidComponentFeature()
        {
        }

        [TestMethod]
        [Description("Verify that a component can be tied to a feature by using the Feature attribute and tied to another feature through a ComponentRef")]
        [Priority(3)]
        [Ignore]
        public void ComponentFeatureAndReferenced()
        {
        }

        [TestMethod]
        [Description("Verify that there is an error if the component is set as the keypath and it contains a resource that is set as a keypath")]
        [Priority(3)]
        [Ignore]
        public void TwoKeyPaths()
        {
        }

        [TestMethod]
        [Description("Verify that registry reflection can be disabled")]
        [Priority(3)]
        [Ignore]
        public void DisableRegistryReflection()
        {
        }

        [TestMethod]
        [Description("Verify that the run location of a component can be set to local, source or either")]
        [Priority(3)]
        [Ignore]
        public void Location()
        {
        }

        [TestMethod]
        [Description("Verify that there is an error if the component run location is not set to local, source or either")]
        [Priority(3)]
        [Ignore]
        public void InvalidLocation()
        {
        }

        [TestMethod]
        [Description("Verify that a component can be set to never be overwritten")]
        [Priority(3)]
        [Ignore]
        public void NeverOverwrite()
        {
        }

        [TestMethod]
        [Description("Verify that a component can be set be permanent (never uninstalled)")]
        [Priority(3)]
        [Ignore]
        public void Permanent()
        {
        }

        [TestMethod]
        [Description("Verify that a component can be set be permanent and unmanaged (no GUID)")]
        [Priority(3)]
        [Ignore]
        public void PermanentUnmanagedComponent()
        {
        }


        [TestMethod]
        [Description("Verify that an unmanaged component cannot be marked as shared")]
        [Priority(3)]
        [Ignore]
        public void UnmanagedSharedComponent()
        {
        }

        [TestMethod]
        [Description("Verify that the component's key file is marked to have its reference count incremented")]
        [Priority(3)]
        [Ignore]
        public void SharedDllRefCount()
        {
        }

        [TestMethod]
        [Description("Verify that there is an error if the component's key file is not a DLL but the SharedDllRefCount attribute is set to 'yes'")]
        [Priority(3)]
        [Ignore]
        public void InvalidSharedDllRefCount()
        {
        }

        [TestMethod]
        [Description("Verify that a component can be marked as Transitive")]
        [Priority(3)]
        [Ignore]
        public void Transitive()
        {
        }

        [TestMethod]
        [Description("Verify that a component can be marked to be uninstall when it is superseded")]
        [Priority(3)]
        [Ignore]
        public void UninstallWhenSuperseded()
        {
        }

        [TestMethod]
        [Description("Verify that the UninstallWhenSuperseded attribute is not allowed for packages that support Windows Installer version before 4.5")]
        [Priority(3)]
        [Ignore]
        public void PreMsi45UninstallWhenSuperseded()
        {
        }
    }
}