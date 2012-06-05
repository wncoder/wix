//-----------------------------------------------------------------------
// <copyright file="RegressionTests.cs" company="Microsoft Corporation">
//   Copyright (c) 2004, Microsoft Corporation.
//   This software is released under Common Public License Version 1.0 (CPL).
//   The license and further copyright text can be found in the file LICENSE.TXT
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
// <summary>Regresssion tests for Dark</summary>
//-----------------------------------------------------------------------

namespace WixTest.Tests.Tools.Dark
{
    using System;
    using System.IO;
    using Microsoft.VisualStudio.TestTools.UnitTesting;
    using WixTest;

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
