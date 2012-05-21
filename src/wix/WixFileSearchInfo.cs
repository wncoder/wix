//-------------------------------------------------------------------------------------------------
// <copyright file="WixFileSearchInfo.cs" company="Microsoft">
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
// Utility class for all WixFileSearches (file and directory searches).
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml
{
    using System;
    using System.Xml;

    /// <summary>
    /// Utility class for all WixFileSearches (file and directory searches).
    /// </summary>
    internal class WixFileSearchInfo : WixSearchInfo
    {
        public WixFileSearchInfo(Row row)
            : this((string)row[0], (string)row[1], (int)row[9])
        {
        }

        public WixFileSearchInfo(string id, string path, int attributes)
            : base(id)
        {
            this.Path = path;
            this.Attributes = (WixFileSearchAttributes)attributes;
        }

        public string Path { get; private set; }
        public WixFileSearchAttributes Attributes { get; private set; }

        /// <summary>
        /// Generates Burn manifest and ParameterInfo-style markup for a file/directory search.
        /// </summary>
        /// <param name="writer"></param>
        public override void WriteXml(XmlTextWriter writer)
        {
            writer.WriteStartElement((0 == (this.Attributes & WixFileSearchAttributes.IsDirectory)) ? "FileSearch" : "DirectorySearch");
            this.WriteWixSearchAttributes(writer);
            writer.WriteAttributeString("Path", this.Path);
            if (WixFileSearchAttributes.WantExists == (this.Attributes & WixFileSearchAttributes.WantExists))
            {
                writer.WriteAttributeString("Type", "exists");
            }
            else if (WixFileSearchAttributes.WantVersion == (this.Attributes & WixFileSearchAttributes.WantVersion))
            {
                // Can never get here for DirectorySearch.
                writer.WriteAttributeString("Type", "version");
            }
            else
            {
                writer.WriteAttributeString("Type", "path");
            }
            writer.WriteEndElement();
        }
    }
}
