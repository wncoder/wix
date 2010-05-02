//-----------------------------------------------------------------------
// <copyright file="RegressionTests.cs" company="Microsoft">
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
// <summary>Regresssion tests for Candle</summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests.Tools.Candle
{
    using System;
    using System.IO;
    using System.Text;
    using System.Collections.Generic;
    using Microsoft.VisualStudio.TestTools.UnitTesting;

    using Microsoft.Tools.WindowsInstallerXml.Test;

    /// <summary>
    /// Regresssion tests for Candle
    /// </summary>
    [TestClass]
    public class RegressionTests : WixTests
    {
        [TestMethod]
        [Description("Verify that the proper error when TARGETDIR has Name='SOURCEDIR'")]
        [TestProperty("Bug Link", "http://sourceforge.net/tracker/index.php?func=detail&aid=1667625&group_id=105970&atid=642714")]
        [Priority(3)]
        public void SourceDirTest()
        {
            Candle candle = new Candle();
            candle.SourceFiles.Add(@"%WIX%\test\data\Tools\Candle\RegressionTests\SourceDirTest\product.wxs");

            candle.ExpectedWixMessages.Add(new WixMessage(206, "The 'TARGETDIR' directory has an illegal DefaultDir value of 'tqepgrb4|SOURCEDIR'.  The DefaultDir value is created from the *Name attributes of the Directory element.  The TARGETDIR directory is a special directory which must have its Name attribute set to 'SourceDir'.", WixMessage.MessageTypeEnum.Error));
            candle.ExpectedExitCode = 206;

            candle.Run();
        }

        [TestMethod]
        [Description("Verify that there is no exception from Candle when the Product element is not populated completely")]
        [Priority(2)]
        [Ignore] // Ignored because of a bug
        public void ProductElementNotPopulated()
        {
            Candle candle = new Candle();
            candle.SourceFiles.Add(@"%WIX%\test\data\Tools\Candle\RegressionTests\ProductElementNotPopulated\IncompleteProductElementBug.wxs");
            candle.ExpectedWixMessages.Add(new WixMessage(10, "The Product/@Id attribute was not found; it is required.", WixMessage.MessageTypeEnum.Error));
            candle.ExpectedWixMessages.Add(new WixMessage(10, "The Product/@Language attribute was not found; it is required.", WixMessage.MessageTypeEnum.Error));
            candle.ExpectedWixMessages.Add(new WixMessage(10, "The Product/@Manufacturer attribute was not found; it is required.", WixMessage.MessageTypeEnum.Error));
            candle.ExpectedWixMessages.Add(new WixMessage(10, "The Product/@Name attribute was not found; it is required.", WixMessage.MessageTypeEnum.Error));
            candle.ExpectedWixMessages.Add(new WixMessage(10, "The Product/@Version attribute was not found; it is required.", WixMessage.MessageTypeEnum.Error));
            candle.ExpectedExitCode = 10;
            candle.Run();
        }

        [TestMethod]
        [Description("Verify that there is only one error message from Candle, when the version attribute in the Product element is not populated")]
        [Priority(3)]
        [Ignore] // Ignored because of a bug
        public void ProductVersionAttributeNotPopulated()
        {
            Candle candle = new Candle();
            candle.SourceFiles.Add(@"%WIX%\test\data\Tools\Candle\RegressionTests\ProductVersionAttributeNotPopulated\ProductVersionAttributeMissing.wxs");
            candle.ExpectedWixMessages.Add(new WixMessage(10, "The Product/@Version attribute was not found; it is required.", WixMessage.MessageTypeEnum.Error));
            candle.ExpectedExitCode = 10;
            candle.Run();
        }

        [TestMethod]
        [Description("Verify that there is no exception from Candle, when there is no Directory set for a shortcut")]
        [Priority(1)]
        public void ShortcutDirectoryNotSet()
        {
            Candle candle = new Candle();
            candle.SourceFiles.Add(@"%WIX%\test\data\Tools\Candle\RegressionTests\ShortcutDirectoryNotSet\ShortcutProduct.wxs");
            candle.ExpectedExitCode = 0;
            candle.Run();
        }

        [TestMethod]
        [Description("Verify MinSize in FileSearch element does not generate a Candle error")]
        [Priority(1)]
        [TestProperty("Bug Link", "http://sourceforge.net/tracker/index.php?func=detail&aid=1648088&group_id=105970&atid=642714")]
        public void NoErrorOnSpecifyingMinSizeInFileSearch()
        {
            Candle candle = new Candle();
            candle.SourceFiles.Add(@"%WIX%\test\data\Tools\Candle\RegressionTests\NoErrorOnSpecifyingMinSizeInFileSearch\FileSearch.wxs");
            candle.ExpectedExitCode = 0;
            candle.Run();
        }

        [TestMethod]
        [Description("Verify that the EmbedCab element cannot be specified without the Cabient attribute")]
        [Priority(2)]
        [TestProperty("Bug Link", "http://sourceforge.net/tracker/index.php?func=detail&aid=1690710&group_id=105970&atid=642714")]
        public void EmbedCabAttrWithoutCabinetAttr()
        {
            Candle candle = new Candle();
            candle.SourceFiles.Add(@"%WIX%\test\data\Tools\Candle\RegressionTests\EmbedCabAttrWithoutCabinetAttr\EmbedCabProduct.wxs");
            candle.ExpectedExitCode = 10;
            candle.ExpectedWixMessages.Add(new WixMessage(10, "The Media/@Cabinet attribute was not found; it is required when attribute EmbedCab has a value of 'yes'.", WixMessage.MessageTypeEnum.Error));
            candle.Run();
        }
    }
}
