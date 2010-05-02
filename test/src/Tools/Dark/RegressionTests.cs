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
// <summary>Regresssion tests for Dark</summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests.Tools.Dark
{
    using System;
    using System.IO;
    using Microsoft.VisualStudio.TestTools.UnitTesting;
    using Microsoft.Tools.WindowsInstallerXml.Test;

    /// <summary>
    /// Regresssion tests for Dark
    /// </summary>
    [TestClass]
    public class RegressionTests : WixTests
    {
        [TestMethod]
        [Description("Verify that Dark generates the proper warning for invalid command arguments.")]
        [Priority(3)]
        public void InvalidArgument()
        {
            Dark dark = new Dark();
            dark.InputFile = Path.Combine(WixTests.SharedBaselinesDirectory, @"MSIs\BasicProduct.msi");
            dark.OtherArguments = " -abc ";
            dark.ExpectedWixMessages.Add(new WixMessage(1098, "'abc' is not a valid command line argument.", WixMessage.MessageTypeEnum.Warning));
            dark.Run();

            // uppercase version of valid command arrguments
            dark = new Dark();
            dark.InputFile = Path.Combine(WixTests.SharedBaselinesDirectory, @"MSIs\BasicProduct.msi");
            dark.OtherArguments = " -SW1098 ";
            dark.ExpectedWixMessages.Add(new WixMessage(1098, "'SW1098' is not a valid command line argument.", WixMessage.MessageTypeEnum.Warning));
            dark.Run();
        }
    }
}
