//-----------------------------------------------------------------------
// <copyright file="WixElement.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>Wix element OM</summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.WixAuthoringOM
{
    using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ElementAttribute;

    [BurnXmlElement("Wix")]
    public class WixElement
    {
        // Xml attributes
        private string m_Xmlns;

        [BurnXmlAttribute("xmlns")]
        public string Xmlns
        {
            get
            { return m_Xmlns; }
            set
            { m_Xmlns = value; }

        }

        private Bundle.BundleElement m_Bundle;
        
        [BurnXmlChildElement()]
        public Bundle.BundleElement Bundle
        {
            get { return m_Bundle; }
            set { m_Bundle = value; }
        }

    }
}
