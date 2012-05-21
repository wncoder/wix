//-------------------------------------------------------------------------------------------------
// <copyright file="WixProductSearchInfo.cs" company="Microsoft">
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
// Utility class for all WixProductSearches.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml
{
    using System;
    using System.Xml;

    /// <summary>
    /// Utility class for all WixProductSearches.
    /// </summary>
    internal class WixProductSearchInfo : WixSearchInfo
    {
        public WixProductSearchInfo(Row row)
            : this((string)row[0], (string)row[1], (int)row[2])
        {
        }

        public WixProductSearchInfo(string id, string productCode, int attributes)
            : base(id)
        {
            this.ProductCode = productCode;
            this.Attributes = (WixProductSearchAttributes)attributes;
        }

        public string ProductCode { get; private set; }
        public WixProductSearchAttributes Attributes { get; private set; }

        /// <summary>
        /// Generates Burn manifest and ParameterInfo-style markup for a product search.
        /// </summary>
        /// <param name="writer"></param>
        public override void WriteXml(XmlTextWriter writer)
        {
            writer.WriteStartElement("MsiProductSearch");
            this.WriteWixSearchAttributes(writer);

            writer.WriteAttributeString("ProductCode", this.ProductCode);

            if (0 != (this.Attributes & WixProductSearchAttributes.Version))
            {
                writer.WriteAttributeString("Type", "version");
            }
            else if (0 != (this.Attributes & WixProductSearchAttributes.Language))
            {
                writer.WriteAttributeString("Type", "language");
            }
            else if (0 != (this.Attributes & WixProductSearchAttributes.State))
            {
                writer.WriteAttributeString("Type", "state");
            }
            else if (0 != (this.Attributes & WixProductSearchAttributes.Assignment))
            {
                writer.WriteAttributeString("Type", "assignment");
            }

            writer.WriteEndElement();
        }
    }
}
