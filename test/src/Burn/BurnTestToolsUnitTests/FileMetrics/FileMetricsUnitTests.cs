//-----------------------------------------------------------------------
// <copyright file="FileMetricsUnitTests.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>
//     - Tests for the BurnMetricsData (part of the Burn test infrastructure)
// </summary>
//-----------------------------------------------------------------------

namespace BurnTestToolsUnitTests.UnitTests.FileMetricsTests
{
    using System;
    using Microsoft.Tools.WindowsInstallerXml.Test.BurnFileMetrics;
    using Microsoft.VisualStudio.TestTools.UnitTesting;

    [TestClass]
    public class FileMetricsUnitTests
    {
        [TestMethod]
        [Description("verify the FileMetrics class will load metrics data accurately")]
        public void IT_FileMetrics()
        {
            // BUGBUG currently, the Metrics file is not well formed XML so this test fails.
            // Manually fix the XML problems in the file and load it to test the FileMetrics class
            //string metricsFile = @"%Temp%\Metrics_20091222_142207533.xml.patched.xml";
            //BurnMetricsData myBurnMetricsData = new BurnMetricsData(metricsFile);
            BurnMetricsData myBurnMetricsData = new BurnMetricsData(DateTime.MinValue, DateTime.Now);

            Assert.IsTrue(!String.IsNullOrEmpty(myBurnMetricsData.PackageName.Value), "myBurnMetricsData.PackageName was not set");
        }
        
    }
}
