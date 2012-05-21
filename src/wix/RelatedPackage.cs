//-------------------------------------------------------------------------------------------------
// <copyright file="RelatedPackage.cs" company="Microsoft">
//    Copyright (c) Microsoft Corporation.  All rights reserved.
//    
//    The use and distribution terms for this software are covered by the
//    Common Public License 1.0 (http://opensource.org/licenses/cpl1.0.php)
//    which can be found in the file CPL.TXT at the root of this distribution.
//    By using this software in any fashion, you are agreeing to be bound by
//    the terms of this license.
//    
//    You must not remove this notice, or any other, from this software.
// </copyright>
// 
// <summary>
// Related packages.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml
{
    using System;
    using System.Collections.Generic;

    /// <summary>
    /// Related packages. Typically represents Upgrade table from an MsiPackage.
    /// </summary>
    internal class RelatedPackage
    {
        private List<string> languages = new List<string>();

        public string Id { get; set; }
        public string MinVersion { get; set; }
        public string MaxVersion { get; set; }
        public List<string> Languages { get { return this.languages; } }
        public bool MinInclusive { get; set; }
        public bool MaxInclusive { get; set; }
        public bool LangInclusive { get; set; }
        public bool OnlyDetect { get; set; }
    }
}
