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
            XDocument d = XDocument.Parse("<Wix xmlns='http://wixtoolset.org/schemas/v4/wxs'><Fragment><ComponentGroup Id='public PublicGroup' Directory='PrivateDirectory'>" +
                                          "<Component Id='internal InternalComponent'>" +
                                          "<File Id='protected ProtectedFile' Source='ignored'/></Component></ComponentGroup>" +
                                          "<DirectoryRef Id='TARGETDIR'><Directory Id='private PrivateDirectory'/></DirectoryRef></Fragment></Wix>");
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
        public void ProtectedLinksAcrossFragmentsButNotFiles()
        {
            XDocument d1 = XDocument.Parse("<Wix xmlns='http://wixtoolset.org/schemas/v4/wxs'><Product Id='*' Language='1033' Manufacturer='WixTests' Name='ProtectedLinksAcrossFragments' Version='1.0.0' UpgradeCode='12345678-1234-1234-1234-1234567890AB'><DirectoryRef Id='ProtectedDirectory'/></Product>" +
                                           "<Fragment><Directory Id='TARGETDIR' Name='SourceDir'><Directory Id='protected ProtectedDirectory' Name='protected'/></Directory></Fragment></Wix>");
            XDocument d2 = XDocument.Parse("<Wix xmlns='http://wixtoolset.org/schemas/v4/wxs'><Fragment><DirectoryRef Id='TARGETDIR'><Directory Id='protected ProtectedDirectory' Name='conflict'/></DirectoryRef></Fragment></Wix>");
            XDocument s1 = new Preprocessor().Process(d1.CreateReader(), new Dictionary<string, string>());
            XDocument s2 = new Preprocessor().Process(d2.CreateReader(), new Dictionary<string, string>());
            Compiler c = new Compiler();
            Linker l = new Linker();
            List<Section> sections = new List<Section>();

            Intermediate i = c.Compile(s1);
            sections.AddRange(i.Sections);

            i = c.Compile(s2);
            sections.AddRange(i.Sections);

            Output o = l.Link(sections, OutputType.Product);

            var directoryRows = o.Sections.SelectMany(sec => sec.Tables).Where(t => t.Name.Equals("Directory")).SelectMany(t => t.Rows).OrderBy(r => r[0]).ToArray();

            Assert.Equal(2, directoryRows.Length);

            Assert.Equal("ProtectedDirectory", directoryRows[0][0]);
            Assert.Equal(AccessModifier.Protected, directoryRows[0].Access);

            Assert.Equal("TARGETDIR", directoryRows[1][0]);
            Assert.Equal(AccessModifier.Public, directoryRows[1].Access);
        }

        [Fact]
        public void PrivateCannotLinkAcrossFragments()
        {
            XDocument d = XDocument.Parse("<Wix xmlns='http://wixtoolset.org/schemas/v4/wxs'><Product Id='*' Language='1033' Manufacturer='WixTests' Name='PrivateCannotLinkAcrossFragments' Version='1.0.0' UpgradeCode='12345678-1234-1234-1234-1234567890AB'><DirectoryRef Id='PrivateDirectory'/></Product>" +
                                          "<Fragment><Directory Id='TARGETDIR' Name='SourceDir'><Directory Id='private PrivateDirectory' Name='private'/></Directory></Fragment></Wix>");
            XDocument s = new Preprocessor().Process(d.CreateReader(), new Dictionary<string, string>());
            Compiler c = new Compiler();
            Linker l = new Linker();

            Intermediate i = c.Compile(s);
            WixException e = Assert.Throws<WixException>(() => l.Link(i.Sections, OutputType.Product));
            Assert.Equal(94, e.Error.Id);
        }

        [Fact]
        public void PrivateDuplicatesAvoidLinkError()
        {
            XDocument d = XDocument.Parse("<Wix xmlns='http://wixtoolset.org/schemas/v4/wxs'><Product Id='*' Language='1033' Manufacturer='WixTests' Name='PrivateDuplicatesAvoidLinkError' Version='1.0.0' UpgradeCode='12345678-1234-1234-1234-1234567890AB'></Product>" +
                                          "<Fragment><DirectoryRef Id='TARGETDIR'><Directory Id='private PrivateDirectory' Name='noconflict1'/></DirectoryRef></Fragment>" +
                                          "<Fragment><DirectoryRef Id='TARGETDIR'><Directory Id='private PrivateDirectory' Name='noconflict2'/></DirectoryRef></Fragment>" +
                                          "<Fragment><Directory Id='TARGETDIR' Name='SourceDir' /></Fragment></Wix>");
            XDocument s = new Preprocessor().Process(d.CreateReader(), new Dictionary<string, string>());
            Compiler c = new Compiler();
            Linker l = new Linker();

            Intermediate i = c.Compile(s);
            Output o = l.Link(i.Sections, OutputType.Product);
            Assert.Empty(o.Sections.SelectMany(sec => sec.Tables).Where(t => t.Name.Equals("Directory")));
        }

        [Fact]
        public void ProtectedDuplicatesInSameFileCauseLinkError()
        {
            XDocument d = XDocument.Parse("<Wix xmlns='http://wixtoolset.org/schemas/v4/wxs'><Product Id='*' Language='1033' Manufacturer='WixTests' Name='ProtectedDuplicatesInSameFileCauseLinkError' Version='1.0.0' UpgradeCode='12345678-1234-1234-1234-1234567890AB'><DirectoryRef Id='ProtectedDirectory'/></Product>" +
                                          "<Fragment><DirectoryRef Id='TARGETDIR'><Directory Id='protected ProtectedDirectory' Name='conflict1'/></DirectoryRef></Fragment>" +
                                          "<Fragment><DirectoryRef Id='TARGETDIR'><Directory Id='protected ProtectedDirectory' Name='conflict2'/></DirectoryRef></Fragment>" +
                                          "<Fragment><Directory Id='TARGETDIR' Name='SourceDir' /></Fragment></Wix>");
            XDocument s = new Preprocessor().Process(d.CreateReader(), new Dictionary<string, string>());
            Compiler c = new Compiler();
            Linker l = new Linker();

            Intermediate i = c.Compile(s);
            WixException e = Assert.Throws<WixException>(() => l.Link(i.Sections, OutputType.Product));
            Assert.Equal(91, e.Error.Id);
        }
    }
}
