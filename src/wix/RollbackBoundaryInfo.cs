//-------------------------------------------------------------------------------------------------
// <copyright file="RollbackBoundaryInfo.cs" company="Microsoft">
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
// Rollback boundary info for binding Bundles.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml
{
    using System;

    /// <summary>
    /// Rollback boundary info for binding Bundles.
    /// </summary>
    internal class RollbackBoundaryInfo
    {
        public RollbackBoundaryInfo(string id)
        {
            this.Default = true;
            this.Id = id;
            this.Vital = YesNoType.Yes;
        }

        public RollbackBoundaryInfo(Row row)
        {
            this.Id = row[0].ToString();

            this.Vital = (null == row[10] || 1 == (int)row[10]) ? YesNoType.Yes : YesNoType.No;
            this.SourceLineNumbers = row.SourceLineNumbers;
        }

        public bool Default { get; private set; }
        public string Id { get; private set; }
        public YesNoType Vital { get; private set; }
        public SourceLineNumberCollection SourceLineNumbers { get; private set; }
    }
}
