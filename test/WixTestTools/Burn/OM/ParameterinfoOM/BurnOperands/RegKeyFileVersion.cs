//-----------------------------------------------------------------------
// <copyright file="RegKeyFileVersion.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>RegKeyFileVersion element OM</summary>
//-----------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.ElementAttribute;

namespace Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnOperands
{
    [BurnXmlElement("RegKeyFileVersion")]
    public class RegKeyFileVersion : Operands
    {
        private string m_Location;

        public RegKeyFileVersion(string location)
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
