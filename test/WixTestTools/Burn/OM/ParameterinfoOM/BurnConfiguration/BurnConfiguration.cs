//-----------------------------------------------------------------------
// <copyright file="BurnConfiguration.cs" company="Microsoft">
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
// <summary>OM for Configuration element</summary>
//-----------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.ElementAttribute;

namespace Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnConfigurationElement
{
    [BurnXmlElement("Configuration")]
    public class BurnConfiguration
    {
        [BurnXmlElement("CommandLineSwitch")]
        public class BurnCommandLineSwitch
        {
            private string m_Name;

            [BurnXmlAttribute("Name")]
            public string Name
            {
                get
                {
                    return m_Name;
                }
                set
                {
                    m_Name = value;
                }
            }
        }

        public abstract class BurnCommandLineSwitches
        {
            private List<BurnCommandLineSwitch> m_CommandLineSwitches;
            
            [BurnXmlChildElement()]
            public BurnCommandLineSwitch[] CommandLineSwitchesArray
            {
                get
                {
                    return CommandLineSwitches.ToArray();
                }
            }

            public List<BurnCommandLineSwitch> CommandLineSwitches
            {
                get
                {
                    if (m_CommandLineSwitches == null) m_CommandLineSwitches = new List<BurnCommandLineSwitch>();
                    return m_CommandLineSwitches;
                }
                set
                {
                    m_CommandLineSwitches = value;
                }
            }
        }

        [BurnXmlElement("AdditionalCommandLineSwitches")]
        public class BurnAdditionalCommandLineSwitches : BurnCommandLineSwitches
        {
        }

        [BurnXmlElement("DisabledCommandLineSwitches")]
        public class BurnDisabledCommandLineSwitches : BurnCommandLineSwitches
        {
        }

        [BurnXmlElement("UserExperienceDataCollection")]
        public class BurnUserExperienceDataCollection
        {
            # region Public enum

            public enum UxPolicy
            {
                Disabled,
                UserControlled,
                AlwaysUploaded
            }

            #endregion

            private UxPolicy m_Policy;

            [BurnXmlAttribute("Policy")]
            public UxPolicy Policy
            {
                get
                {
                    return m_Policy;
                }
                set
                {
                    m_Policy = value;
                }
            }

            private string m_MetricLoader;

            [BurnXmlAttribute("MetricLoader")]
            public string MetricLoader
            {
                get
                {
                    return m_MetricLoader;
                }
                set
                {
                    m_MetricLoader = value;
                }
            }

        }

        [BurnXmlElement("DownloadInstallSetting")]
        public class BurnDownloadInstallSetting
        {
            private bool m_SerialDownload;
            [BurnXmlAttribute("SerialDownload")]
            public bool SerialDownload
            {
                get
                {
                    return m_SerialDownload;
                }
                set
                {
                    m_SerialDownload = value;
                }
            }
        }

        [BurnXmlElement("BlockingMutex")]
        public class BurnBlockingMutex
        {
            private string m_Name;
            
            [BurnXmlAttribute("Name")]
            public string Name
            {
                get
                {
                    return m_Name;
                }
                set
                {
                    m_Name = value;
                }
            }
        }

        [BurnXmlElement("FilesInUseSettingPrompt")]
        public class BurnFilesInUseSettingPrompt
        {
            private bool m_Prompt;

            [BurnXmlAttribute("Prompt")]
            public bool Prompt
            {
                get 
                {
                    return m_Prompt;
                }
                set 
                {
                    m_Prompt = value;
                }
            }
        }

        [BurnXmlElement("AcceptableCertificates")]
        public class BurnAcceptableCertificates
        {
            [BurnXmlElement("Certificate")]
            public class BurnCertificate
            {
                private string m_Name;
                private string m_AuthorityKeyIdentifier;
                private string m_Thumbprint;

                [BurnXmlAttribute("Name")]
                public string Name
                {
                    get
                    {
                        return m_Name;
                    }
                    set
                    {
                        m_Name = value;
                    }
                }

                [BurnXmlAttribute("AuthorityKeyIdentifier")]
                public string AuthorityKeyIdentifier
                {
                    get
                    {
                        return m_AuthorityKeyIdentifier;
                    }
                    set
                    {
                        m_AuthorityKeyIdentifier = value;
                    }
                }

                [BurnXmlAttribute("Thumbprint")]
                public string Thumbprint
                {
                    get
                    {
                        return m_Thumbprint;
                    }
                    set
                    {
                        m_Thumbprint = value;
                    }
                }
            }

            private List<BurnCertificate> m_Certificates;

            public List<BurnCertificate> Certificates
            {
                get
                {
                    if (m_Certificates == null) m_Certificates = new List<BurnCertificate>();
                    return m_Certificates;
                }
                set
                {
                    m_Certificates = value;
                }
            }
            [BurnXmlChildElement()]
            public BurnCertificate[] ItemsArray
            {
                get { return Certificates.ToArray(); }
            }
        }


        # region Private member variables

        private BurnDisabledCommandLineSwitches m_DisabledCommandLineSwitches;

        private BurnAdditionalCommandLineSwitches m_AdditionalCommandLineSwitches;

        private BurnUserExperienceDataCollection m_UserExperienceDataCollection;

        private BurnDownloadInstallSetting m_DownloadInstallSetting;

        private BurnBlockingMutex m_BlockingMutexName;

        private BurnFilesInUseSettingPrompt m_FilesInUseSettingPrompt;

        private BurnAcceptableCertificates m_AcceptableCertificates;
                
        #endregion

        # region Public Property

        [BurnXmlChildElement()]
        public BurnDisabledCommandLineSwitches DisabledCommandLineSwitches
        {
            get
            {
                return m_DisabledCommandLineSwitches;
            }

            set
            {
                m_DisabledCommandLineSwitches = value;
            }
        }

        [BurnXmlChildElement()]
        public BurnAdditionalCommandLineSwitches AdditionalCommandLineSwitches
        {
            get
            {
                return m_AdditionalCommandLineSwitches;
            }

            set
            {
                m_AdditionalCommandLineSwitches = value;
            }
        }

        [BurnXmlChildElement()]
        public BurnUserExperienceDataCollection UserExperienceDataCollection
        {
            get
            {
                return m_UserExperienceDataCollection;
            }

            set
            {
                m_UserExperienceDataCollection = value;
            }
        }

        [BurnXmlChildElement()]
        public BurnDownloadInstallSetting DownloadInstallSetting
        {
            get
            {
                return m_DownloadInstallSetting;
            }

            set
            {
                m_DownloadInstallSetting = value;
            }
        }

        [BurnXmlChildElement()]
        public BurnBlockingMutex BlockingMutex
        {
            get
            {
                return m_BlockingMutexName;
            }

            set
            {
                m_BlockingMutexName = value;
            }
        }

        [BurnXmlChildElement()]
        public BurnFilesInUseSettingPrompt FilesInUseSettingPrompt
        {
            get
            {
                return m_FilesInUseSettingPrompt;
            }

            set
            {
                m_FilesInUseSettingPrompt = value;
            }
        }

        [BurnXmlChildElement()]
        public BurnAcceptableCertificates AcceptableCertificates
        {
            get
            {
                return m_AcceptableCertificates;
            }

            set
            {
                m_AcceptableCertificates = value;
            }
        }

        #endregion
    }
}
