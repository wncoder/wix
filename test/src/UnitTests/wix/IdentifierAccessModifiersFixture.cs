//-------------------------------------------------------------------------------------------------
// <copyright file="IdentifierAccessModifiersFixture.cs" company="Outercurve Foundation">
//   Copyright (c) 2004, Outercurve Foundation.
//   This software is released under Microsoft Reciprocal License (MS-RL).
//   The license and further copyright text can be found in the file
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
//-------------------------------------------------------------------------------------------------

namespace WixTest.WixUnitTest
{
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Xml.Linq;
    using WixToolset;
    using WixToolset.Data;
    using WixToolset.Data.Rows;
    using Xunit;

    public class IdentifierAccessModifiersFixture : WixUnitTestBaseFixture
    {
        [Fact]
        public void ImplicitPublicIsPublic()
        {
            XDocument d = XDocument.Parse("<Wix xmlns='http://wixtoolset.org/schemas/v4/wxs'><Fragment><ComponentGroup Id='PublicGroup' /></Fragment></Wix>");
            XDocument s = new Preprocessor().Process(d.CreateReader(), new Dictionary<string, string>());
            Compiler c = new Compiler();

            Intermediate i = c.Compile(s);
            var row = i.Sections.SelectMany(sec => sec.Tables).Where(t => t.Name.Equals("WixComponentGroup")).SelectMany(t => t.Rows).Single();
            Assert.Equal("PublicGroup", row[0]);
            Assert.Equal(AccessModifier.Public, row.Access);
        }

        [Fact]
        public void ExplicitAccessModifiersValid()
        {
            XDocument d = XDocument.Parse("<Wix xmlns='http://wixtoolset.org/schemas/v4/wxs'><Fragment><ComponentGroup Id='public PublicGroup' Directory='PrivateDirectory'><Component Id='internal InternalComponent'><File Id='protected ProtectedFile' Source='ignored'/></Component></ComponentGroup><DirectoryRef Id='TARGETDIR'><Directory Id='private PrivateDirectory'/></DirectoryRef></Fragment></Wix>");
            XDocument s = new Preprocessor().Process(d.CreateReader(), new Dictionary<string, string>());
            Compiler c = new Compiler();

            Intermediate i = c.Compile(s);
            var tables = i.Sections.SelectMany(sec => sec.Tables).ToDictionary(t => t.Name);
            var componentGroupRow = tables["WixComponentGroup"].Rows.Single();
            Assert.Equal("PublicGroup", componentGroupRow[0].ToString());
            Assert.Equal(AccessModifier.Public, componentGroupRow.Access);

            var componentRow = tables["Component"].Rows.Single();
            Assert.Equal("InternalComponent", componentRow[0].ToString());
            Assert.Equal(AccessModifier.Internal, componentRow.Access);

            var fileRow = tables["File"].Rows.Single();
            Assert.Equal("ProtectedFile", fileRow[0].ToString());
            Assert.Equal(AccessModifier.Protected, fileRow.Access);

            var directoryRow = tables["Directory"].Rows.Single();
            Assert.Equal("PrivateDirectory", directoryRow[0].ToString());
            Assert.Equal(AccessModifier.Private, directoryRow.Access);
        }

        [Fact]
        public void ProtectedLinksAcrossFragments()
        {
            XDocument d = XDocument.Parse("<Wix xmlns='http://wixtoolset.org/schemas/v4/wxs'><Product Id='*' Language='1033' Manufacturer='WixTests' Name='ProtectedLinksAcrossFragments' Version='1.0.0' UpgradeCode='12345678-1234-1234-1234-1234567890AB'><DirectoryRef Id='ProtectedDirectory'/></Product><Fragment><Directory Id='TARGETDIR'><Directory Id='protected ProtectedDirectory'/></Directory></Fragment></Wix>");
            XDocument s = new Preprocessor().Process(d.CreateReader(), new Dictionary<string, string>());
            Compiler c = new Compiler();
            Linker l = new Linker();

            Intermediate i = c.Compile(s);
            Output o = l.Link(i.Sections, OutputType.Product);

            var directoryRows = i.Sections.SelectMany(sec => sec.Tables).Where(t => t.Name.Equals("Directory")).SelectMany(t => t.Rows).OrderBy(r => r[0]).ToArray();

            Assert.Equal(2, directoryRows.Length);

            Assert.Equal("ProtectedDirectory", directoryRows[0][0]);
            Assert.Equal(AccessModifier.Protected, directoryRows[0].Access);

            Assert.Equal("TARGETDIR", directoryRows[1][0]);
            Assert.Equal(AccessModifier.Protected, directoryRows[1].Access);
        }

        [Fact]
        public void PrivateCannotLinkAcrossFragments()
        {
            XDocument d = XDocument.Parse("<Wix xmlns='http://wixtoolset.org/schemas/v4/wxs'><Product Id='*' Language='1033' Manufacturer='WixTests' Name='PrivateCannotLinkAcrossFragments' Version='1.0.0' UpgradeCode='12345678-1234-1234-1234-1234567890AB'><DirectoryRef Id='PrivateDirectory'/></Product><Fragment><Directory Id='TARGETDIR'><Directory Id='private PrivateDirectory'/></Directory></Fragment></Wix>");
            XDocument s = new Preprocessor().Process(d.CreateReader(), new Dictionary<string, string>());
            Compiler c = new Compiler();
            Linker l = new Linker();

            Intermediate i = c.Compile(s);
            Output o = l.Link(i.Sections, OutputType.Product);
            Assert.Null(o);
        }
    }
}
