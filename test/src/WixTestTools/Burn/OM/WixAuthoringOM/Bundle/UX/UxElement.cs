//-----------------------------------------------------------------------
// <copyright file="UXElement.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>Ux element OM</summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.WixAuthoringOM.Bundle.UX
{
    using System.Collections.Generic;
    using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ElementAttribute;

    [BurnXmlElement("UX")]
    public class UXElement
    {
        // Xml attributes
        private string m_SourceFile;
        [BurnXmlAttribute("SourceFile")]
        public string SourceFile
        {
            get { return m_SourceFile; }
            set { m_SourceFile = value; }
        }

        private List<PayloadElement> m_Payloads;

        public List<PayloadElement> Payloads
        {
            get
            {
                if (m_Payloads == null) m_Payloads = new List<PayloadElement>();
                return m_Payloads;
            }
            set
            {
                m_Payloads = value;
            }
        }
        [BurnXmlChildElement()]
        public PayloadElement[] PayloadsArray
        {
            get { return Payloads.ToArray(); }
        }
    }
}
