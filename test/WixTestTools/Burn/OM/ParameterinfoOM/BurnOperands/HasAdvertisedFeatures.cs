//-----------------------------------------------------------------------
// <copyright file="HasAdvertisedFeatures.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>HasAdvertisedFeatures element OM</summary>
//-----------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.ElementAttribute;

namespace Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnOperands
{
    [BurnXmlElement("HasAdvertisedFeatures")]
    public class HasAdvertisedFeatures : Operands
    {
        private string m_ProductCode;

        public HasAdvertisedFeatures(string productCode)
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
