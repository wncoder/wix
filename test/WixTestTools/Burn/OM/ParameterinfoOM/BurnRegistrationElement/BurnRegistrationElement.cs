//-----------------------------------------------------------------------
// <copyright file="BurnRegistrationElement.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>BurnRegistrationElement element OM</summary>
//-----------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.ElementAttribute;

namespace Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnRegistrationElement
{
    [BurnXmlElement("Registration")]
    public class BurnRegistrationElement
    {
        private string m_Id;
        private string m_ExecutableName;
        private string m_PerMachine;
        private string m_Family;

        [BurnXmlAttribute("Id")]
        public string Id
        {
            get { return m_Id; }
            set { m_Id = value; }
        }

        [BurnXmlAttribute("ExecutableName")]
        public string ExecutableName
        {
            get { return m_ExecutableName; }
            set { m_ExecutableName = value; }
        }

        [BurnXmlAttribute("PerMachine")]
        public string PerMachine
        {
            get { return m_PerMachine; }
            set { m_PerMachine = value; }
        }


        [BurnXmlAttribute("Family")]
        public string Family
        {
            get { return m_Family; }
            set { m_Family = value; }
        }

        [BurnXmlElement("Arp")]
        public class BurnArpElement
        {
            private string m_DisplayName;
            [BurnXmlAttribute("DisplayName")]
            public string DisplayName
            {
                get { return m_DisplayName; }
                set { m_DisplayName = value; }
            }

            private string m_DisplayVersion;
            [BurnXmlAttribute("DisplayVersion")]
            public string DisplayVersion
            {
                get { return m_DisplayVersion; }
                set { m_DisplayVersion = value; }
            }

        }

        private BurnArpElement m_Arp;

        [BurnXmlChildElement()]
        public BurnArpElement Arp
        {
            get
            {
                return m_Arp;
            }
            set
            {
                m_Arp = value;
            }
        }

    }
}
