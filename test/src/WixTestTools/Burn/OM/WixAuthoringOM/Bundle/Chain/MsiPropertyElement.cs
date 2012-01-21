//-----------------------------------------------------------------------
// <copyright file="MsiPropertyElement.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>MsiProperty element OM</summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.WixAuthoringOM.Bundle.Chain
{
    using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ElementAttribute;

    [BurnXmlElement("MsiProperty")]
    public class MsiPropertyElement
    {
        private string m_Name;
        private string m_Value;

        [BurnXmlAttribute("Name")]
        public string Name
        {
            get
            {
                return m_Name;
            }
            set
            {
                m_Name = value;
            }
        }

        [BurnXmlAttribute("Value")]
        public string Value
        {
            get
            {
                return m_Value;
            }
            set
            {
                m_Value = value;
            }
        }
    }
}
