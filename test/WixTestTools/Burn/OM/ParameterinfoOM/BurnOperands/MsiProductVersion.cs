//-----------------------------------------------------------------------
// <copyright file="MsiProductVersion.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
//    
//    The use and distribution terms for this software are covered by the
//    Common Public License 1.0 (http://opensource.org/licenses/cpl.php)
//    which can be found in the file CPL.TXT at the root of this distribution.
//    By using this software in any fashion, you are agreeing to be bound by
//    the terms of this license.
//    
//    You must not remove this notice, or any other, from this software.
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
