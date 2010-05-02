//-----------------------------------------------------------------------
// <copyright file="MsiProductVersion.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>MsiProductVersion element OM</summary>
//-----------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.ElementAttribute;

namespace Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnOperands
{
    /// <summary>
    /// Returns the Msi Version, given a ProductCode
    /// </summary>
    [BurnXmlElement("MsiProductVersion")]
    public class MsiProductVersion : Operands
    {
        private string m_ProductCode;

        public MsiProductVersion(string productCode)
        {
            this.ProductCode = productCode;
        }

        [BurnXmlAttribute("ProductCode")]
        public string ProductCode
        {
            get
            {
                return m_ProductCode;
            }

            set
            {
                m_ProductCode = value;
            }
        }
    }
}
