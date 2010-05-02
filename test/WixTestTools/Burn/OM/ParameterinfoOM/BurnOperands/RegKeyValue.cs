//-----------------------------------------------------------------------
// <copyright file="RegKeyValue.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>RegKeyValue element OM</summary>
//-----------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.ElementAttribute;

namespace Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnOperands
{
    /// <summary>
    /// returns the value of a RegKey
    /// </summary>
    [BurnXmlElement("RegKeyValue")]
    public class RegKeyValue : Operands
    {
        private string m_Location;

        public RegKeyValue(string location)
        {
            this.Location = location;
        }

        [BurnXmlAttribute("Location")]
        public string Location
        {
            get
            {
                return m_Location;
            }

            set
            {
                m_Location = value;
            }
        }
    }
}
