//-------------------------------------------------------------------------------------------------
// <copyright file="PatchTargetCodeRow.cs" company="Microsoft">
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
// Row for payload information.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml
{
    using System;
    using System.Collections.Generic;
    using System.Text;

    /// <summary>
    /// Attributes for the PatchTargetCode table.
    /// </summary>
    [Flags]
    public enum WixBundlePatchTargetCodeAttributes : int
    {
        None = 0,

        /// <summary>
        /// The transform targets a specific ProductCode.
        /// </summary>
        TargetsProductCode = 1,

        /// <summary>
        /// The transform targets a specific UpgradeCode.
        /// </summary>
        TargetsUpgradeCode = 2,
    }

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

        public WixBundlePatchTargetCodeAttributes Attributes
        {
            get { return (WixBundlePatchTargetCodeAttributes)this.Fields[2].Data; }
            set { this.Fields[2].Data = (int)value; }
        }

        public bool TargetsProductCode
        {
            get { return 0 != (WixBundlePatchTargetCodeAttributes.TargetsProductCode & this.Attributes); }
        }

        public bool TargetsUpgradeCode
        {
            get { return 0 != (WixBundlePatchTargetCodeAttributes.TargetsUpgradeCode & this.Attributes); }
        }
    }
}
