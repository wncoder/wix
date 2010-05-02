//-----------------------------------------------------------------------
// <copyright file="Extensions.ExtensionTests.cs" company="Microsoft">
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//    
//    The use and distribution terms for this software are covered by the
//    Common Public License 1.0 (http://opensource.org/licenses/cpl.php)
//    which can be found in the file CPL.TXT at the root of this distribution.
//    By using this software in any fashion, you are agreeing to be bound by
//    the terms of this license.
//    
//    You must not remove this notice, or any other, from this software.
// </copyright>
// <summary>Test how Dark handles the -ext switch</summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests.Tools.Dark.Extensions
{
    using System;
    using System.IO;
    using Microsoft.VisualStudio.TestTools.UnitTesting;
    using Microsoft.Tools.WindowsInstallerXml.Test;
    
    /// <summary>
    /// Test how Dark handles the -ext switch.
    /// </summary>
    [TestClass]
    public class ExtensionTests
    {
        [TestMethod]
        [Description("Verify that Dark generates the correct error for a missing extension after -ext switch.")]
        [Priority(2)]
        public void MissingExtension()
        {
            Dark dark = new Dark();
            dark.InputFile = Path.Combine(WixTests.SharedBaselinesDirectory, @"MSIs\BasicProduct.msi");
            dark.Extensions.Add(string.Empty);
            dark.ExpectedWixMessages.Add(new WixMessage(113, "The parameter '-ext' must be followed by the extension's type specification.  The type specification should be a fully qualified class and assembly identity, for example: \"MyNamespace.MyClass,myextension.dll\".", WixMessage.MessageTypeEnum.Error));
            dark.ExpectedExitCode = 113;
            dark.Run();
        }

        [TestMethod]
        [Description("Verify that Dark generates the correct error for an invalid extension file after -ext switch.")]
        [Priority(2)]
        public void InvalidExtension()
        {
            Dark dark = new Dark();
            dark.InputFile = Path.Combine(WixTests.SharedBaselinesDirectory, @"MSIs\BasicProduct.msi");
            dark.Extensions.Add(WixTests.BasicProductWxs);
            dark.ExpectedWixMessages.Add(new WixMessage(144, string.Format("The extension '{0}' could not be loaded because of the following reason: Could not load file or assembly 'file:///{1}\\test\\data\\SharedData\\Authoring\\BasicProduct.wxs' or one of its dependencies. The module was expected to contain an assembly manifest.", WixTests.BasicProductWxs,Environment .GetEnvironmentVariable("WIX_ROOT")), WixMessage.MessageTypeEnum.Error));
            dark.ExpectedExitCode = 144;
            dark.Run();
        }
    }
}