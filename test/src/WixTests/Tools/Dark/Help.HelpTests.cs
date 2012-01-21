//-----------------------------------------------------------------------
// <copyright file="Help.HelpTests.cs" company="Microsoft">
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//    
//    The use and distribution terms for this software are covered by the
//    Common Public License 1.0 (http://opensource.org/licenses/cpl1.0.php)
//    which can be found in the file CPL.TXT at the root of this distribution.
//    By using this software in any fashion, you are agreeing to be bound by
//    the terms of this license.
//    
//    You must not remove this notice, or any other, from this software.
// </copyright>
// <summary>Test how Dark handles the ? switch</summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests.Tools.Dark.Help
{
    using System;
    using Microsoft.VisualStudio.TestTools.UnitTesting;
    using Microsoft.Tools.WindowsInstallerXml.Test;
    
    /// <summary>
    /// Test how Dark handles the ? switch.
    /// </summary>
    [TestClass]
    public class HelpTests : WixTests
    {
        [TestMethod]
        [Description("Verify that Dark accepts -? option and displays the correct usage.")]
        [Priority(2)]
        public void DisplayHelp()
        {
            Dark dark = new Dark();
            dark.Help = true;

            dark.ExpectedOutputStrings.Add("usage: dark.exe [-?] [-nologo] database.msi [source.wxs]");
            dark.ExpectedOutputStrings.Add("-ext <extension>  extension assembly or \"class, assembly\"");
            dark.ExpectedOutputStrings.Add("-nologo    skip printing dark logo information");
            dark.ExpectedOutputStrings.Add("-notidy    do not delete temporary files (useful for debugging)");
            dark.ExpectedOutputStrings.Add("-o[ut]     specify output file (default: write .wxs to current directory)");
            dark.ExpectedOutputStrings.Add("-sct       suppress decompiling custom tables");
            dark.ExpectedOutputStrings.Add("-sdet      suppress dropping empty tables (adds EnsureTable as appropriate)");
            dark.ExpectedOutputStrings.Add("-sras      suppress relative action sequencing");
            dark.ExpectedOutputStrings.Add("(use explicit sequence numbers)");
            dark.ExpectedOutputStrings.Add("-sui       suppress decompiling UI-related tables");
            dark.ExpectedOutputStrings.Add("-sw[N]     suppress all warnings or a specific message ID");
            dark.ExpectedOutputStrings.Add("(example: -sw1059 -sw1067)");
            dark.ExpectedOutputStrings.Add("-swall     suppress all warnings (deprecated)");
            dark.ExpectedOutputStrings.Add("-v         verbose output");
            dark.ExpectedOutputStrings.Add("-wx[N]     treat all warnings or a specific message ID as an error");
            dark.ExpectedOutputStrings.Add("(example: -wx1059 -wx1067)");
            dark.ExpectedOutputStrings.Add("-wxall     treat all warnings as errors (deprecated)");
            dark.ExpectedOutputStrings.Add("-x <path>  export binaries from cabinets and embedded binaries to <path>");
            dark.ExpectedOutputStrings.Add("-xo        output wixout instead of WiX source code");
            dark.ExpectedOutputStrings.Add("(mandatory for transforms and patches)");
            dark.ExpectedOutputStrings.Add("-? | -help this help information");
            dark.ExpectedOutputStrings.Add("Environment variables:");
            dark.ExpectedOutputStrings.Add("WIX_TEMP   overrides the temporary directory used for cab extraction, binary extraction, ...");
            dark.ExpectedOutputStrings.Add("For more information see: http://wix.sourceforge.net");

            dark.Run();
        }
    }
}