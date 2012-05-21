//-------------------------------------------------------------------------------------------------
// <copyright file="ChainInfo.cs" company="Microsoft">
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
// Chain info for binding Bundles.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml
{
    using System;
    using System.Collections.Generic;

    /// <summary>
    /// Chain info for binding Bundles.
    /// </summary>
    internal class ChainInfo
    {
        public ChainInfo(Row row)
        {
            BundleChainAttributes attributes = (null == row[0]) ? BundleChainAttributes.None : (BundleChainAttributes)row[0];

            this.DisableRollback = (BundleChainAttributes.DisableRollback == (attributes & BundleChainAttributes.DisableRollback));
            this.DisableSystemRestore = (BundleChainAttributes.DisableSystemRestore == (attributes & BundleChainAttributes.DisableSystemRestore));
            this.ParallelCache = (BundleChainAttributes.ParallelCache == (attributes & BundleChainAttributes.ParallelCache));
            this.Packages = new List<ChainPackageInfo>();
            this.RollbackBoundaries = new List<RollbackBoundaryInfo>();
            this.SourceLineNumbers = row.SourceLineNumbers;
        }

        public bool DisableRollback { get; private set; }
        public bool DisableSystemRestore { get; private set; }
        public bool ParallelCache { get; private set; }
        public List<ChainPackageInfo> Packages { get; private set; }
        public List<RollbackBoundaryInfo> RollbackBoundaries { get; private set; }
        public SourceLineNumberCollection SourceLineNumbers { get; private set; }
    }
}
