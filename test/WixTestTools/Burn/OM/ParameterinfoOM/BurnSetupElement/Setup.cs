//-----------------------------------------------------------------------
// <copyright file="Setup.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>Setup element OM</summary>
//-----------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.ElementAttribute;

namespace Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnSetupElement
{
    [BurnXmlElement("Setup")]
    public class Setup
    {
        // Xml attributes
        private string m_Xmlns;
        private string m_XmlnsIm;
        private string m_SetupVersion;

        [BurnXmlAttribute("xmlns")]
        public string Xmlns
        {
            get
            { return m_Xmlns; }
            set
            { m_Xmlns = value; }
        }

        [BurnXmlAttribute("xmlns:im")]
        public string XmlnsIm
        {
            get
            { return m_XmlnsIm; }
            set
            { m_XmlnsIm = value; }
        }

        [BurnXmlAttribute("SetupVersion")]
        public string SetupVersion
        {
            get
            { return m_SetupVersion; }
            set
            { m_SetupVersion = value; }
        }
        


        // Xml elements:
        //  - UI               1 
        //  - Configuration    0-1
        //  - EnterMaintenanceModeIf 1
        //  - Blockers         0-1
        //  - Items            1
        //  - SystemCheck      0-1

        private Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnUiElement.BurnUiElement m_UI;
        private Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnConfigurationElement.BurnConfiguration m_Configuration;
        private Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnUxElement.BurnUxElement m_Ux;
        private Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnEnterMaintenanceModeIf.EnterMaintenanceModeIf m_EnterMaintenanceModeIf;
        private Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnBlockers.Blockers m_Blockers;
        private Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemsGroup m_Items;
        private Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnSystemCheckElement.SystemCheck m_SystemCheck;
        private Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnRegistrationElement.BurnRegistrationElement m_Registration;
        private Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnSearches.DirectorySearch m_DirectorySearch;
        private Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnSearches.FileSearch m_FileSearch;
        private Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnSearches.MsiComponentSearch m_MsiComponentSearch;
        private Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnSearches.MsiProductSearch m_MsiProductSearch;
        private Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnSearches.RegistrySearch m_RegistrySearch;
        private Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnVariable.Variable m_Variable;

        [BurnXmlChildElement()]
        public Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnUiElement.BurnUiElement UI
        {
            get
            {
                return m_UI;
            }
            set
            {
                m_UI = value;
            }
        }

        [BurnXmlChildElement()]
        public Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnConfigurationElement.BurnConfiguration Configuration
        {
            get
            {
                return m_Configuration;
            }
            set
            {
                m_Configuration = value;
            }
        }

        [BurnXmlChildElement()]
        public Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnUxElement.BurnUxElement Ux
        {
            get
            {
                return m_Ux;
            }
            set
            {
                m_Ux = value;
            }
        }

        [BurnXmlChildElement()]
        public Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnRegistrationElement.BurnRegistrationElement Registration
        {
            get
            {
                return m_Registration;
            }
            set
            {
                m_Registration = value;
            }
        }

        [BurnXmlChildElement()]
        public Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnEnterMaintenanceModeIf.EnterMaintenanceModeIf EnterMaintenanceModeIf
        {
            get
            {
                return m_EnterMaintenanceModeIf;
            }
            set
            {
                m_EnterMaintenanceModeIf = value;
            }
        }

        [BurnXmlChildElement()]
        public Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnBlockers.Blockers Blockers
        {
            get
            {
                return m_Blockers;
            }
            set
            {
                m_Blockers = value;
            }
        }

        [BurnXmlChildElement()]
        public Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemsGroup Items
        {
            get
            {
                return m_Items;
            }
            set
            {
                m_Items = value;
            }
        }

        [BurnXmlChildElement()]
        public Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnSystemCheckElement.SystemCheck SystemCheck
        {
            get
            {
                return m_SystemCheck;
            }
            set
            {
                m_SystemCheck = value;
            }
        }

        [BurnXmlChildElement()]
        public Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnSearches.DirectorySearch DirectorySearch
        {
            get
            {
                return m_DirectorySearch;
            }
            set
            {
                m_DirectorySearch = value;
            }
        }

        [BurnXmlChildElement()]
        public Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnSearches.FileSearch FileSearch
        {
            get
            {
                return m_FileSearch;
            }
            set
            {
                m_FileSearch = value;
            }
        }

        [BurnXmlChildElement()]
         public Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnSearches.MsiComponentSearch MsiComponentSearch
         {
             get
             {
                 return m_MsiComponentSearch;
             }
             set
             {
                 m_MsiComponentSearch = value;
             }
         }

        [BurnXmlChildElement()]
         public Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnSearches.MsiProductSearch MsiProductSearch
         {
             get
             {
                 return m_MsiProductSearch;
             }
             set
             {
                 m_MsiProductSearch = value;
             }
         }

        [BurnXmlChildElement()]
         public Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnSearches.RegistrySearch RegistrySearch
         {
             get
             {
                 return m_RegistrySearch;
             }
             set
             {
                 m_RegistrySearch = value;
             }
         }

        [BurnXmlChildElement()]
         public Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnVariable.Variable Variable
         {
             get
             {
                 return m_Variable;
             }
             set
             {
                 m_Variable = value;
             }
         }


        // Constructors

        public Setup()
        {
        }

        public Setup(bool useDefaults)
        {
            if (useDefaults)
            {
                // define default attributes
                Xmlns = @"http://schemas.microsoft.com/Setup/2008/01/im";
                XmlnsIm = @"http://schemas.microsoft.com/Setup/2008/01/im";
                SetupVersion = "1.0";

                // create default elements
                UI = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnUiElement.BurnUiElement();
                Configuration = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnConfigurationElement.BurnConfiguration();
                EnterMaintenanceModeIf = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnEnterMaintenanceModeIf.EnterMaintenanceModeIf();
                Blockers = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnBlockers.Blockers();
                Items = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemsGroup();
                SystemCheck = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnSystemCheckElement.SystemCheck();
            }
        }
    }
}
