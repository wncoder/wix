//-----------------------------------------------------------------------
// <copyright file="UXElement.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
//    
//    The use and distribution terms for this software are covered by the
//    Common Public License 1.0 (http://opensource.org/licenses/cpl1.0.php)
//    which can be found in the file CPL.TXT at the root of this distribution.
//    By using this software in any fashion, you are agreeing to be bound by
//    the terms of this license.
//    
//    You must not remove this notice, or any other, from this software.
// </copyright>
// <summary>Ux element OM</summary>
//-----------------------------------------------------------------------

namespace WixTest.Burn.OM.WixAuthoringOM.Bundle.UX
{
    using System.Collections.Generic;
    using WixTest.Burn.OM.ElementAttribute;

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
