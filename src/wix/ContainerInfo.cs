//-------------------------------------------------------------------------------------------------
// <copyright file="MsiFeature.cs" company="Microsoft">
//    Copyright (c) Microsoft Corporation.  All rights reserved.
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
            : this((string)row[0], (string)row[1], (string)row[2], fileManager)
        {
            this.SourceLineNumbers = row.SourceLineNumbers;
        }

        public ContainerInfo(string id, string name, string type, BinderFileManager fileManager)
        {
            this.Id = id;
            this.Name = name;
            this.Type = type;
            this.FileManager = fileManager;
            this.TempPath = Path.Combine(fileManager.TempFilesLocation, name);
            this.FileInfo = new FileInfo(this.TempPath);
        }

        public SourceLineNumberCollection SourceLineNumbers { get; private set; }
        public BinderFileManager FileManager { get; private set; }
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
