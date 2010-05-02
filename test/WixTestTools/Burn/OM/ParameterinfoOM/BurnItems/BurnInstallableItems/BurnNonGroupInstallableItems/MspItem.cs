//-----------------------------------------------------------------------
// <copyright file="MspItem.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>MSP element OM</summary>
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
    /// <summary>
    /// 
    /// </summary>
    [BurnXmlElement("MSP")]
    public class MspItem : BurnInstallableItem
    {
        # region Private member variables

        private string m_CacheId;
        private string m_PatchCode;

        #endregion

        # region Public Property

        /// <summary>
        /// The CacheId of the package.
        /// Default values should be:
        /// MSP: {PatchCode}
        /// </summary>
        [BurnXmlAttribute("CacheId")]
        public override string CacheId
        {
            get
            {
                if (string.IsNullOrEmpty(m_CacheId))
                    m_CacheId = this.PatchCode;

                return m_CacheId;
            }

            set
            {
                m_CacheId = value;
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


        #region Methods for generating default values for all the required Properties

        public void GenerateDefaults(string file)
        {
            this.SourceFilePath = file;
            this.Name = System.IO.Path.GetFileName(file);
            this.Id = this.Name;
            this.ActionTable = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemElements.ActionTableElement();
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnOperands.MsiXmlBlob msiXmlBlob = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnOperands.MsiXmlBlob(file);
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.Exists existsXmlBlob = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.Exists(msiXmlBlob);
            this.ApplicableIf.ApplicableExpression = existsXmlBlob;
            this.DownloadSize = null;  // null value will cause it to be calculated
            this.PatchCode = null; // null value will cause it to be calculated
            this.EstimatedInstallTime = 5;
            this.InstalledProductSize = 1024;
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnOperands.MsiGetCachedPatchPath msiGetCachedPatchPath = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnOperands.MsiGetCachedPatchPath(this.PatchCode);
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.Exists existsMsiGetCachedPatchPath = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.Exists(msiGetCachedPatchPath);
            this.IsPresent.Expression = existsMsiGetCachedPatchPath;
            this.PerMachine = true;
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
