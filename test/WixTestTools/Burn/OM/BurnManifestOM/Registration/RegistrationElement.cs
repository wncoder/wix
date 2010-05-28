//-----------------------------------------------------------------------
// <copyright file="RegistrationElement.cs" company="Microsoft">
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
// <summary>Registration element OM</summary>
//-----------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.ElementAttribute;

namespace Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.BurnManifestOM.Registration
{
    [BurnXmlElement("Registration")]
    public class RegistrationElement
    {

        [BurnXmlElement("Arp")]
        public class ArpElement
        {
            private string m_Name;
            [BurnXmlAttribute("Name")]
            public string Name
            {
                get { return m_Name; }
                set { m_Name = value; }
            }

            private string m_Version;
            [BurnXmlAttribute("Version")]
            public string Version
            {
                get { return m_Version; }
                set { m_Version = value; }
            }

            private string m_Publisher;
            [BurnXmlAttribute("Publisher")]
            public string Publisher
            {
                get { return m_Publisher; }
                set { m_Publisher = value; }
            }

            private string m_HelpLink;
            [BurnXmlAttribute("HelpLink")]
            public string HelpLink
            {
                get { return m_HelpLink; }
                set { m_HelpLink = value; }
            }

            private string m_HelpTelephone;
            [BurnXmlAttribute("HelpTelephone")]
            public string HelpTelephone
            {
                get { return m_HelpTelephone; }
                set { m_HelpTelephone = value; }
            }

            private string m_AboutUrl;
            [BurnXmlAttribute("AboutUrl")]
            public string AboutUrl
            {
                get { return m_AboutUrl; }
                set { m_AboutUrl = value; }
            }

            private string m_UpdateUrl;
            [BurnXmlAttribute("UpdateUrl")]
            public string UpdateUrl
            {
                get { return m_UpdateUrl; }
                set { m_UpdateUrl = value; }
            }

            private string m_DisableModify;
            [BurnXmlAttribute("DisableModify")]
            public string DisableModify
            {
                get { return m_DisableModify; }
                set { m_DisableModify = value; }
            }

            private string m_DisableRepair;
            [BurnXmlAttribute("DisableRepair")]
            public string DisableRepair
            {
                get { return m_DisableRepair; }
                set { m_DisableRepair = value; }
            }

            private string m_DisableRemove;
            [BurnXmlAttribute("DisableRemove")]
            public string DisableRemove
            {
                get { return m_DisableRemove; }
                set { m_DisableRemove = value; }
            }
        }

        [BurnXmlElement("Upgrade")]
        public class UpgradeElement
        {
            private string m_FamilyId;
            [BurnXmlAttribute("FamilyId")]
            public string FamilyId
            {
                get { return m_FamilyId; }
                set { m_FamilyId = value; }
            }
        }

        [BurnXmlElement("Update")]
        public class UpdateElement
        {
            private string m_BundleId;
            [BurnXmlAttribute("BundleId")]
            public string BundleId
            {
                get { return m_BundleId; }
                set { m_BundleId = value; }
            }
        } 

        // Xml attributes
        private string m_Family;
        [BurnXmlAttribute("Family")]
        public string Family
        {
            get { return m_Family; }
            set { m_Family = value; }
        }

        private RegistrationElement.ArpElement m_Arp;
        [BurnXmlChildElement()]
        public RegistrationElement.ArpElement Arp
        {
            get { return m_Arp; }
            set { m_Arp = value; }
        }

        private List<RegistrationElement.UpgradeElement> m_UpgradeElements;

        public List<RegistrationElement.UpgradeElement> UpgradeElements
        {
            get
            {
                if (m_UpgradeElements == null) m_UpgradeElements = new List<RegistrationElement.UpgradeElement>();
                return m_UpgradeElements;
            }
            set
            {
                m_UpgradeElements = value;
            }
        }
        [BurnXmlChildElement()]
        public RegistrationElement.UpgradeElement[] UpgradeElementsArray
        {
            get { return UpgradeElements.ToArray(); }
        }

        private List<RegistrationElement.UpdateElement> m_UpdateElements;

        public List<RegistrationElement.UpdateElement> UpdateElements
        {
            get
            {
                if (m_UpdateElements == null) m_UpdateElements = new List<RegistrationElement.UpdateElement>();
                return m_UpdateElements;
            }
            set
            {
                m_UpdateElements = value;
            }
        }
        [BurnXmlChildElement()]
        public RegistrationElement.UpdateElement[] UpdateElementsArray
        {
            get { return UpdateElements.ToArray(); }
        }
    }
}
