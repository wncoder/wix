//-----------------------------------------------------------------------
// <copyright file="Package.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>Package element OM</summary>
//-----------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.ElementAttribute;

namespace Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.BurnManifestOM.Chain
{
    public abstract class Package
    {
        // Xml attributes
        private string m_Id;

        [BurnXmlAttribute("Id")]
        public string Id
        {
            get { return m_Id; }
            set { m_Id = value; }
        }

        private string m_Vital;

        [BurnXmlAttribute("Vital")]
        public string Vital
        {
            get { return m_Vital; }
            set { m_Vital = value; }
        }

        private string m_Packaging;

        [BurnXmlAttribute("Packaging")]
        public string Packaging
        {
            get { return m_Packaging; }
            set { m_Packaging = value; }
        }

        private string m_SourceFile;

        [BurnXmlAttribute("SourceFile")]
        public string SourceFile
        {
            get { return m_SourceFile; }
            set { m_SourceFile = value; }
        }

        private string m_FileName;

        [BurnXmlAttribute("FileName")]
        public string FileName
        {
            get { return m_FileName; }
            set { m_FileName = value; }
        }

        private string m_Cache;

        [BurnXmlAttribute("Cache")]
        public string Cache
        {
            get { return m_Cache; }
            set { m_Cache = value; }
        }

        private string m_DownloadUrl;

        [BurnXmlAttribute("DownloadUrl")]
        public string DownloadUrl
        {
            get { return m_DownloadUrl; }
            set { m_DownloadUrl = value; }
        }

        private string m_InstallCondition;

        [BurnXmlAttribute("InstallCondition")]
        public string InstallCondition
        {
            get { return m_InstallCondition; }
            set { m_InstallCondition = value; }
        }

        private string m_RollbackInstallCondition;

        [BurnXmlAttribute("RollbackInstallCondition")]
        public string RollbackInstallCondition
        {
            get { return m_RollbackInstallCondition; }
            set { m_RollbackInstallCondition = value; }
        }
        
    }
}
