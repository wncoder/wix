//-------------------------------------------------------------------------------------------------
// <copyright file="WixSearchInfo.cs" company="Microsoft">
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
// Utility base class for all WixSearches.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml
{
    using System;
    using System.Diagnostics;
    using System.Xml;

    /// <summary>
    /// Utility base class for all WixSearches.
    /// </summary>
    internal abstract class WixSearchInfo
    {
        public WixSearchInfo(string id)
        {
            this.Id = id;
        }

        public void AddWixSearchRowInfo(Row row)
        {
            Debug.Assert((string)row[0] == Id);
            Variable = (string)row[1];
            Condition = (string)row[2];
        }

        public string Id { get; private set; }
        public string Variable { get; private set; }
        public string Condition { get; private set; }

        /// <summary>
        /// Generates Burn manifest and ParameterInfo-style markup a search.
        /// </summary>
        /// <param name="writer"></param>
        public virtual void WriteXml(XmlTextWriter writer)
        {
        }

        /// <summary>
        /// Writes attributes common to all WixSearch elements.
        /// </summary>
        /// <param name="writer"></param>
        protected void WriteWixSearchAttributes(XmlTextWriter writer)
        {
            writer.WriteAttributeString("Id", this.Id);
            writer.WriteAttributeString("Variable", this.Variable);
            if (!String.IsNullOrEmpty(this.Condition))
            {
                writer.WriteAttributeString("Condition", this.Condition);
            }
        }
    }
}
