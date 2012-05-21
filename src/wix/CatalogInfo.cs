//-------------------------------------------------------------------------------------------------
// <copyright file="CatalogInfo.cs" company="Microsoft">
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
// Catalog information for binding Bundles.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml
{
    using System;
    using System.IO;

    /// <summary>
    /// Catalog information for binding Bundles.
    /// </summary>
    internal class CatalogInfo
    {
        public CatalogInfo(Row row, string payloadId)
        {
            this.Initialize((string)row[0], (string)row[1], payloadId);
        }

        private void Initialize(string id, string sourceFile, string payloadId)
        {
            this.Id = id;
            this.FileInfo = new FileInfo(sourceFile);
            this.PayloadId = payloadId;
        }

        public string Id { get; private set; }
        public FileInfo FileInfo { get; private set; }
        public string PayloadId { get; private set; }
    }
}
