//-----------------------------------------------------------------------
// <copyright file="AgileMsiItem.cs" company="Microsoft">
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
// <summary>AgileMsi element OM</summary>
//-----------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.ElementAttribute;
using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems;
using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.Utility;

namespace Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnInstallableItems.BurnGroupInstallableItems
{
    /// <summary>
    /// 
    /// </summary>
    [BurnXmlElement("AgileMSI")]
    public class AgileMsiItem : BurnBaseItems
    {
        public class AgileMspItem
        {
            # region Private member variables

            private string m_SourceFilePath;

            private string m_Name;

            private string m_Url;

            private uint m_DownloadSize;

            private string m_HashValue;

            private uint m_InstalledProductSize;

            private uint m_SystemDriveSize;

            private string m_PatchCode;

            #endregion

            # region Public Property

            /// <summary>
            /// The name of the Item
            /// </summary>
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

            /// <summary>
            /// URL of the file to download if necessary
            /// </summary>
            [BurnXmlAttribute("URL")]
            public string URL
            {
                get
                {
                    return m_Url;
                }

                set
                {
                    m_Url = value;
                }
            }

            /// <summary>
            /// The size of the item in bytes. This value is used to calibrate download progressbar and estimate download time
            /// </summary>
            [BurnXmlAttribute("DownloadSize")]
            public uint DownloadSize
            {
                get
                {
                    return m_DownloadSize;
                }

                set
                {
                    m_DownloadSize = value;
                }
            }

            /// <summary>
            /// The SHA256 hash value of the file being downloaded
            /// </summary>
            [BurnXmlAttribute("HashValue")]
            public string HashValue
            {
                get
                {
                    return m_HashValue;
                }

                set
                {
                    m_HashValue = value;
                }
            }

            /// <summary>
            /// The estimated size that the item will take on the drive the Product is installed in bytes
            /// </summary>
            [BurnXmlAttribute("InstalledProductSize")]
            public uint InstalledProductSize
            {
                get
                {
                    return m_InstalledProductSize;
                }

                set
                {
                    m_InstalledProductSize = value;
                }
            }

            /// <summary>
            /// The estimated size that the item will take on the system drive in bytes after install
            /// </summary>
            [BurnXmlAttribute("SystemDriveSize")]
            public uint SystemDriveSize
            {
                get
                {
                    return m_SystemDriveSize;
                }

                set
                {
                    m_SystemDriveSize = value;
                }
            }

            /// <summary>
            /// To define the source file path of msp payload
            /// </summary>
            [BurnXmlAttribute("SourceFilePath")]
            public string SourceFilePath
            {
                get
                {
                    return m_SourceFilePath;
                }

                set
                {
                    m_SourceFilePath = value;
                }
            }


            /// <summary>
            /// Patch code of the MSP. Mostly used during patch uninstall
            /// </summary>
            [BurnXmlAttribute("PatchCode")]
            public string PatchCode
            {
                get
                {
                    if (string.IsNullOrEmpty(m_PatchCode))
                        m_PatchCode = MsiUtils.GetPatchCode(this.SourceFilePath);

                    return m_PatchCode;
                }

                set
                {
                    m_PatchCode = value;
                }
            }

            #endregion
        }
        
        # region Private member variables
        
        private uint m_DownloadSize;

        private uint m_SystemDriveSize;

        private string m_ProductCode;

        private string m_MSIOptions;

        private string m_MSIUninstallOptions;

        private string m_MSIRepairOptions;
       
        private string m_Url;      

        private uint m_InstalledProductSize;

        private string m_HashValue;

        private uint m_EstimatedInstallTime;

        private string m_CanonicalTargetName;

        private List<AgileMspItem> m_AgileMspItems;
        
        #endregion

        # region Public Property

        /// <summary>
        /// Product code of the MSI. Used to uninstall and repair the product
        /// </summary>
        [BurnXmlAttribute("ProductCode")]
        public string ProductCode
        {
            get
            {
                if ( string.IsNullOrEmpty(m_ProductCode))
                m_ProductCode = MsiUtils.GetMSIProductCode(this.SourceFilePath);

                return m_ProductCode;
            }

            set
            {
                m_ProductCode = value;
            }
        }

        /// <summary>
        /// The MSI command-line options to pass in during install
        /// </summary>
        [BurnXmlAttribute("MSIOptions")]
        public string MSIOptions
        {
            get
            {
                return m_MSIOptions;
            }

            set
            {
                m_MSIOptions = value;
            }
        }

        /// <summary>
        /// The MSI command-line options to pass in during uninstall
        /// </summary>
        [BurnXmlAttribute("MSIUninstallOptions")]
        public string MSIUninstallOptions
        {
            get
            {
                return m_MSIUninstallOptions;
            }

            set
            {
                m_MSIUninstallOptions = value;
            }
        }

        /// <summary>
        /// The MSI command-line options to pass in during repair
        /// </summary>
        [BurnXmlAttribute("MSIRepairOptions")]
        public string MSIRepairOptions
        {
            get
            {
                return m_MSIRepairOptions;
            }

            set
            {
                m_MSIRepairOptions = value;
            }
        }        

        /// <summary>
        /// URL of the file to download if necessary
        /// </summary>
        [BurnXmlAttribute("URL")]
        public string URL
        {
            get
            {
                return m_Url;
            }

            set
            {
                m_Url = value;
            }
        }

        /// <summary>
        /// The size of the item in bytes. This value is used to calibrate download progressbar and estimate download time
        /// </summary>
        [BurnXmlAttribute("DownloadSize")]
        public uint DownloadSize
        {
            get
            {
                return m_DownloadSize;
            }

            set
            {
                m_DownloadSize = value;
            }
        }

        /// <summary>
        /// The estimated size that the item will take on the system drive in bytes after install
        /// </summary>
        [BurnXmlAttribute("SystemDriveSize")]
        public uint SystemDriveSize
        {
            get
            {
                return m_SystemDriveSize;
            }

            set
            {
                m_SystemDriveSize = value;
            }
        }

        /// <summary>
        /// The estimated size that the item will take on the drive the Product is installed in bytes
        /// </summary>
        [BurnXmlAttribute("InstalledProductSize")]
        public uint InstalledProductSize
        {
            get
            {
                return m_InstalledProductSize;
            }

            set
            {
                m_InstalledProductSize = value;
            }
        }

        /// <summary>
        /// The SHA256 hash value of the file being downloaded
        /// </summary>
        [BurnXmlAttribute("HashValue")]
        public string HashValue
        {
            get
            {
                return m_HashValue;
            }

            set
            {
                m_HashValue = value;
            }
        }

        /// <summary>
        /// The estimated install time of an item in seconds
        /// </summary>
        [BurnXmlAttribute("EstimatedInstallTime")]
        public uint EstimatedInstallTime
        {
            get
            {
                return m_EstimatedInstallTime;
            }

            set
            {
                m_EstimatedInstallTime = value;
            }
        }

        /// <summary>
        /// Display target package name. It's used in UI to show user-friendly item name
        /// </summary>
        [BurnXmlAttribute("CanonicalTargetName")]
        public string CanonicalTargetName
        {
            get
            {
                return m_CanonicalTargetName;
            }

            set
            {
                m_CanonicalTargetName = value;
            }
        }

        /// <summary>
        /// List of AgileMsp items
        /// </summary>
        public List<AgileMspItem> AgileMspItems
        {
            get
            {
                return m_AgileMspItems;
            }

            set
            {
                m_AgileMspItems = value;
            }
        }

        #endregion
    }
}
