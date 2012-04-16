//-------------------------------------------------------------------------------------------------
// <copyright file="PatchTargetCodeRow.cs" company="Microsoft">
//    Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// 
// <summary>
// Row for payload information.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml
{
    using System;
    using System.Collections.Generic;
    using System.Text;

    /// <summary>
    /// Specialization of a row for the PatchTargetCode table.
    /// </summary>
    public class WixBundlePatchTargetCodeRow : Row
    {
        /// <summary>
        /// Creates a PatchTargetCodeRow row that does not belong to a table.
        /// </summary>
        /// <param name="sourceLineNumbers">Original source lines for this row.</param>
        /// <param name="tableDef">TableDefinition this PatchTargetCode row belongs to and should get its column definitions from.</param>
        public WixBundlePatchTargetCodeRow(SourceLineNumberCollection sourceLineNumbers, TableDefinition tableDef) :
            base(sourceLineNumbers, tableDef)
        {
        }

        /// <summary>
        /// Creates a PatchTargetCodeRow row that belongs to a table.
        /// </summary>
        /// <param name="sourceLineNumbers">Original source lines for this row.</param>
        /// <param name="table">Table this PatchTargetCode row belongs to and should get its column definitions from.</param>
        public WixBundlePatchTargetCodeRow(SourceLineNumberCollection sourceLineNumbers, Table table) :
            base(sourceLineNumbers, table)
        {
        }

        public string MspPackageId
        {
            get { return (string)this.Fields[0].Data; }
            set { this.Fields[0].Data = value; }
        }

        public string TargetCode
        {
            get { return (string)this.Fields[1].Data; }
            set { this.Fields[1].Data = value; }
        }

        public bool Product
        {
            get { return (null != this.Fields[2].Data) && (1 == (int)this.Fields[2].Data); }
            set { this.Fields[2].Data = value ? 1 : 0; }
        }
    }
}
