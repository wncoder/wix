//-----------------------------------------------------------------------
// <copyright file="MsiItem.cs" company="Microsoft">
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
// <summary>MSI element OM</summary>
//-----------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.ElementAttribute;
using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.InstallableItems;
using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.Utility;

namespace Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnInstallableItems
{
    [BurnXmlElement("MSI")]
    public class MsiItem : BurnInstallableItem
    {
        #region Private member variables

        private string m_CacheId;
        private string m_CanonicalTargetName;
        private string m_ProductCode;
        private string m_ProductVersion;
        private string m_MSIOptions;
        private string m_MSIUninstallOptions;
        private string m_MSIRepairOptions;       

        #endregion

        #region Public Property

        /// <summary>
        /// The CacheId of the package.
        /// Default values should be:
        /// MSI: {ProductCode}v{ProductVersion}
        /// </summary>
        [BurnXmlAttribute("CacheId")]
        public override string CacheId
        {
            get
            {
                if (string.IsNullOrEmpty(m_CacheId))
                    m_CacheId = this.ProductCode+"v"+this.ProductVersion;

                return m_CacheId;
            }

            set
            {
                m_CacheId = value;
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
        /// Product Version of the MSI.
        /// </summary>
        [BurnXmlAttribute("ProductVersion")]
        public string ProductVersion
        {
            get
            {
                if (string.IsNullOrEmpty(m_ProductVersion))
                    m_ProductVersion = MsiUtils.GetMSIProductVersion(this.SourceFilePath);

                return m_ProductVersion;
            }

            set
            {
                m_ProductVersion = value;
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

        #endregion

        #region Methods for generating default values for all the required Properties

        public void GenerateDefaults(string file)
        {
            this.SourceFilePath = file;
            this.Name = System.IO.Path.GetFileName(file);
            this.Id = this.Name;
            this.ActionTable = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemElements.ActionTableElement();
            this.ApplicableIf.ApplicableExpression = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.AlwaysTrue();
            this.CanonicalTargetName = "Test Msi " + System.IO.Path.GetFileName(file);
            this.DownloadSize = null;  // null value will cause it to be calculated
            this.EstimatedInstallTime = 5;
            this.InstalledProductSize = 1024;
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnOperands.MsiProductVersion msiProdVer = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnOperands.MsiProductVersion(this.ProductCode);
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.Exists existsMsiProdVer = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.Exists(msiProdVer);
            this.IsPresent.Expression = existsMsiProdVer;
            this.MSIOptions = "";
            this.MSIRepairOptions = "";
            this.MSIUninstallOptions = "";
            this.PerMachine = true;
            this.ProductCode = null; // null value will cause it to be calculated
            this.ProductVersion = null; // null value will cause it to be calculated
            this.Rollback = true;
            this.Sha1HashValue = null;  // null value will cause it to be calculated
            this.CacheId = null;  // null value will cause it to be calculated
            this.SystemDriveSize = 0;
            this.URL = null;
            this.Cache = true;
        }

        #endregion
    }
}
