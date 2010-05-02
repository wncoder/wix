//-----------------------------------------------------------------------
// <copyright file="BurnUiElement.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>BurnUiElement element OM</summary>
//-----------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.ElementAttribute;

namespace Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnUiElement
{
    [BurnXmlElement("UI")]
    public class BurnUiElement
    {
        private string m_Dll;
        private string m_Name;
        private string m_Version;

        [BurnXmlAttribute("Dll")]
        public string Dll
        {
            get { return m_Dll; }
            set { m_Dll = value; }
        }

        [BurnXmlAttribute("Name")]
        public string Name
        {
            get { return m_Name; }
            set { m_Name = value; }
        }

        [BurnXmlAttribute("Version")]
        public string Version
        {
            get { return m_Version; }
            set { m_Version = value; }
        }
    }
}
