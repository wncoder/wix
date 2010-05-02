//-----------------------------------------------------------------------
// <copyright file="FileVersion.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>FileVersion element OM</summary>
//-----------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.ElementAttribute;

namespace Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnOperands
{
    [BurnXmlElement("FileVersion")]
    public class FileVersion : Operands
    {
        private string m_Location;

        public FileVersion(string location)
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
