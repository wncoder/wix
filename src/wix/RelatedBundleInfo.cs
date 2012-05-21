//-------------------------------------------------------------------------------------------------
// <copyright file="RelatedBundleInfo.cs" company="Microsoft">
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
// Utility class for Burn RelatedBundle information.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml
{
    using System;
    using System.Xml;

    using Wix = Microsoft.Tools.WindowsInstallerXml.Serialize;

    /// <summary>
    /// Utility class for Burn RelatedBundle information.
    /// </summary>
    internal class RelatedBundleInfo
    {
        public RelatedBundleInfo(Row row)
            : this((string)row[0], (int)row[1])
        {
        }

        public RelatedBundleInfo(string id, int action)
        {
            this.Id = id;
            this.Action = (Wix.RelatedBundle.ActionType)action;
        }

        public string Id { get; private set; }
        public Wix.RelatedBundle.ActionType Action { get; private set; }

        /// <summary>
        /// Generates Burn manifest element for a RelatedBundle.
        /// </summary>
        /// <param name="writer"></param>
        public void WriteXml(XmlTextWriter writer)
        {
            string actionString = this.Action.ToString();

            writer.WriteStartElement("RelatedBundle");
            writer.WriteAttributeString("Id", this.Id);
            writer.WriteAttributeString("Action", Convert.ToString(this.Action));
            writer.WriteEndElement();
        }
    }
}
