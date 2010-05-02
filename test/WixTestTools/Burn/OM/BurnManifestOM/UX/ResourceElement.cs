//-----------------------------------------------------------------------
// <copyright file="ResourceElement.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>Resource element OM</summary>
//-----------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.ElementAttribute;

namespace Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.BurnManifestOM.UX
{
    [BurnXmlElement("Resource")]
    public class ResourceElement
    {
        // Xml attributes
        private string m_SourceFile;
        private string m_Packaging;
        private string m_FileName;

        [BurnXmlAttribute("SourceFile")]
        public string SourceFile
        {
            get { return m_SourceFile; }
            set { m_SourceFile = value; }
        }

        [BurnXmlAttribute("Packaging")]
        public string Packaging
        {
            get { return m_Packaging; }
            set { m_Packaging = value; }
        }

        [BurnXmlAttribute("FileName")]
        public string FileName
        {
            get { return m_FileName; }
            set { m_FileName = value; }
        }
    }
}
