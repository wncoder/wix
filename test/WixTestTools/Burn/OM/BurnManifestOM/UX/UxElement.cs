//-----------------------------------------------------------------------
// <copyright file="UxElement.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>Ux element OM</summary>
//-----------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.ElementAttribute;

namespace Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.BurnManifestOM.UX
{
    [BurnXmlElement("UX")]
    public class UxElement
    {
        // Xml attributes
        private string m_SourceFile;

        [BurnXmlAttribute("SourceFile")]
        public string SourceFile
        {
            get { return m_SourceFile; }
            set { m_SourceFile = value; }
        }

        private List<ResourceElement> m_Resources;

        public List<ResourceElement> Resources
        {
            get
            {
                if (m_Resources == null) m_Resources = new List<ResourceElement>();
                return m_Resources;
            }
            set
            {
                m_Resources = value;
            }
        }
        [BurnXmlChildElement()]
        public ResourceElement[] ResourcesArray
        {
            get { return Resources.ToArray(); }
        }
    }
}
