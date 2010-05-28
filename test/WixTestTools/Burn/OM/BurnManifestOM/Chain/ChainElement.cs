//-----------------------------------------------------------------------
// <copyright file="ChainElement.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
//    
//    The use and distribution terms for this software are covered by the
//    Common Public License 1.0 (http://opensource.org/licenses/cpl.php)
//    which can be found in the file CPL.TXT at the root of this distribution.
//    By using this software in any fashion, you are agreeing to be bound by
//    the terms of this license.
//    
//    You must not remove this notice, or any other, from this software.
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
