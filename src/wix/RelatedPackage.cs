//-------------------------------------------------------------------------------------------------
// <copyright file="RelatedPackage.cs" company="Microsoft">
//    Copyright (c) Microsoft Corporation.  All rights reserved.
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
