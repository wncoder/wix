//-----------------------------------------------------------------------
// <copyright file="Messages.VerboseTests.cs" company="Microsoft">
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
//     Tests for verbose output
// </summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests.Tools.Light.Messages
{
    using System;
    using System.Collections.Generic;
    using System.IO;
    using System.Text;

    using Microsoft.VisualStudio.TestTools.UnitTesting;
    using Microsoft.Tools.WindowsInstallerXml.Test;

    /// <summary>
    /// Tests for verbose output
    /// </summary>
    [TestClass]
    public class VerboseTests : WixTests
    {
        [TestMethod]
        [Description("Verify that Light prints verbose output")]
        [Priority(1)]
        public void SimpleVerbose()
        {
            Candle candle = new Candle();
            candle.SourceFiles.Add(Path.Combine(WixTests.SharedAuthoringDirectory, "BasicProduct.wxs"));
            candle.Run();

            Light light = new Light(candle);
            light.Verbose = true;
            light.ExpectedOutputStrings.Add("Updating file information.");
            light.ExpectedOutputStrings.Add("Creating cabinet files.");
            light.ExpectedOutputStrings.Add("Generating database.");
            light.ExpectedOutputStrings.Add("Merging modules.");
            light.ExpectedOutputStrings.Add("Validating database.");
            light.ExpectedOutputStrings.Add("Laying out media.");
            light.ExpectedOutputStrings.Add("Moving file");
            
            light.Run();
        }
    }
}