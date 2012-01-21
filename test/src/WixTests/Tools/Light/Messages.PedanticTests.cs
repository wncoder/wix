//-----------------------------------------------------------------------
// <copyright file="Messages.PedanticTests.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>
//     Tests for pedantic output
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
    /// Tests for pedantic output
    /// </summary>
    [TestClass]
    public class PedanticTests : WixTests
    {
        [TestMethod]
        [Description("Verify that Light prints pedantic output")]
        [Priority(1)]
        public void SimplePedantic()
        {
            Candle candle = new Candle();
            candle.SourceFiles.Add(Path.Combine(WixTests.SharedAuthoringDirectory, "BasicProduct.wxs"));
            candle.Run();

            Light light = new Light(candle);
            light.Pedantic = true;
            light.Run();
        }
    }
}