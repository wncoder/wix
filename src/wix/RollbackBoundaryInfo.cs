//-------------------------------------------------------------------------------------------------
// <copyright file="RollbackBoundaryInfo.cs" company="Microsoft">
//    Copyright (c) Microsoft Corporation.  All rights reserved.
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
