//-----------------------------------------------------------------------
// <copyright file="ChainElement.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>Chain element OM</summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.WixAuthoringOM.Bundle.Chain
{
    using System.Collections.Generic;
    using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ElementAttribute;

    [BurnXmlElement("Chain")]
    public class ChainElement
    {
        private string m_DisableRollback;
        [BurnXmlAttribute("DisableRollback")]
        public string DisableRollback
        {
            get { return m_DisableRollback; }
            set { m_DisableRollback = value; }
        }

        private List<Package> m_Packages;

        public List<Package> Packages
        {
            get
            {
                if (m_Packages == null) m_Packages = new List<Package>();
                return m_Packages;
            }
            set
            {
                m_Packages = value;
            }
        }
        [BurnXmlChildElement()]
        public Package[] PackagesArray
        {
            get { return Packages.ToArray(); }
        }
    }
}
