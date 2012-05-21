//-------------------------------------------------------------------------------------------------
// <copyright file="MsiFeature.cs" company="Microsoft">
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
// Container info for binding Bundles.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml
{
    using System;
    using System.IO;
    using System.Collections.Generic;

    /// <summary>
    /// Container info for binding Bundles.
    /// </summary>
    internal class ContainerInfo
    {
        private List<PayloadInfoRow> payloads = new List<PayloadInfoRow>();

        public ContainerInfo(Row row, BinderFileManager fileManager)
            : this((string)row[0], (string)row[1], (string)row[2], (string)row[3], fileManager)
        {
            this.SourceLineNumbers = row.SourceLineNumbers;
        }

        public ContainerInfo(string id, string name, string type, string downloadUrl, BinderFileManager fileManager)
        {
            this.Id = id;
            this.Name = name;
            this.Type = type;
            this.DownloadUrl = downloadUrl;
            this.FileManager = fileManager;
            this.TempPath = Path.Combine(fileManager.TempFilesLocation, name);
            this.FileInfo = new FileInfo(this.TempPath);
        }

        public SourceLineNumberCollection SourceLineNumbers { get; private set; }
        public BinderFileManager FileManager { get; private set; }
        public string DownloadUrl { get; private set; }
        public string Id { get; private set; }
        public string Name { get; private set; }
        public string Type { get; private set; }
        public string TempPath { get; private set; }
        public FileInfo FileInfo { get; set; }
        public long FileSize { get { return this.FileInfo.Length; } }

        public List<PayloadInfoRow> Payloads
        {
            get
            {
                return this.payloads;
            }
            set
            {
                this.payloads = value;
            }
        }
    }
}
