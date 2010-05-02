//-----------------------------------------------------------------------
// <copyright file="ChainElement.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>Chain element OM</summary>
//-----------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.ElementAttribute;

namespace Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.BurnManifestOM.Chain
{
    [BurnXmlElement("Chain")]
    public class ChainElement
    {
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
