//-----------------------------------------------------------------------
// <copyright file="ResourceElement.cs" company="Microsoft">
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
// <summary>Resource element OM</summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.WixAuthoringOM.Bundle
{
    using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ElementAttribute;

    [BurnXmlElement("Payload")]
    public class PayloadElement
    {
        // Xml attributes
        private string m_Compressed;
        [BurnXmlAttribute("Compressed")]
        public string Compressed
        {
            get { return m_Compressed; }
            set { m_Compressed = value; }
        }

        private string m_Name;
        [BurnXmlAttribute("Name")]
        public string Name
        {
            get { return m_Name; }
            set { m_Name = value; }
        }

        private string m_SourceFile;
        [BurnXmlAttribute("SourceFile")]
        public string SourceFile
        {
            get { return m_SourceFile; }
            set { m_SourceFile = value; }
        }

        private string m_DownloadUrl;
        [BurnXmlAttribute("DownloadUrl")]
        public string DownloadUrl
        {
            get { return m_DownloadUrl; }
            set { m_DownloadUrl = value; }
        }

        #region Properties that are not part of Wix authoring schema but are used to keep track of things tests use

        /// <summary>
        /// Full path to the package.  Tests use this to verify if packages are installed or not.
        /// </summary>
        private string m_SourceFilePathT;
        public string SourceFilePathT
        {
            get { return m_SourceFilePathT; }
            set { m_SourceFilePathT = value; }
        }

        #endregion
    }
}