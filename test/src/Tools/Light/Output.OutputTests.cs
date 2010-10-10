//-----------------------------------------------------------------------
// <copyright file="Output.OutputTests.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
//    
//    The use and distribution terms for this software are covered by the
//    Common Public License 1.0 (http://opensource.org/licenses/cpl1.0.php)
//    which can be found in the file CPL.TXT at the root of this distribution.
//    By using this software in any fashion, you are agreeing to be bound by
//    the terms of this license.
//    
//    You must not remove this notice, or any other, from this software.
// </copyright>
// <summary>
//     Tests for Output 
// </summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests.Tools.Light.Output
{
    using System;
    using System.Collections.Generic;
    using System.IO;
    using System.Text;

    using Microsoft.VisualStudio.TestTools.UnitTesting;
    using Microsoft.Tools.WindowsInstallerXml.Test;

    /// <summary>
    /// Test for Output
    /// </summary>
    [TestClass]
    public class OutputTests : WixTests
    {
        [TestMethod]
        [Description("Verify that locking light output file results in the expected error message.")]
        [Priority(2)]
        [TestProperty("Bug Link", "http://sourceforge.net/tracker/index.php?func=detail&aid=1835329&group_id=105970&atid=642714")]
        public void LockedOutputFile()
        {
            Candle candle = new Candle();
            candle.SourceFiles.Add(WixTests.BasicProductWxs);
            candle.Run();

            string outputDirectory = Utilities.FileUtilities.GetUniqueFileName();
            System.IO.Directory.CreateDirectory(outputDirectory);
            string outputFile = Path.Combine(outputDirectory, "test.msi");
            System.IO.File.OpenWrite(outputFile);

            Light light = new Light(candle);
            light.OutputFile = outputFile;
            string expectedErrorMessage = string.Format("The process can not access the file '{0}' because it is being used by another process.", outputFile);
            light.ExpectedWixMessages.Add(new WixMessage(128, expectedErrorMessage, WixMessage.MessageTypeEnum.Error));
            light.ExpectedExitCode = 128;
            light.Run();
        }
    }
}