//-----------------------------------------------------------------------
// <copyright file="ExeItem.cs" company="Microsoft">
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
// <summary>Exe element OM</summary>
//-----------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.ElementAttribute;
using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.InstallableItems;

namespace Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnInstallableItems
{
    [BurnXmlElement("Exe")]
    public class ExeItem : BurnInstallableItem
    {
        # region Public Enum

        /// <summary>
        /// Type of Exe item
        /// </summary>
        public enum ExeTypeEnum
        {
            Cartman,
            HotIron,
            IronMan,
            MsuPackage,
            LocalExe,
            Unknown
        }

        #endregion

        # region Private member variables

        private string m_CacheId;

        private string m_CanonicalTargetName;

        private string m_InstallCommandLine;

        private string m_UninstallCommandLine;

        private string m_RepairCommandLine;

        private string m_LogFileHint;

        private ExeTypeEnum? m_ExeType;

        #endregion

        #region Public Property

        /// <summary>
        /// The CacheId of the package.
        /// Default values should be:
        /// Exe: Sha1HashValue
        /// </summary>
        [BurnXmlAttribute("CacheId")]
        public override string CacheId
        {
            get
            {
                if (string.IsNullOrEmpty(m_CacheId))
                    m_CacheId = this.Sha1HashValue;

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
        /// Commandline args passed to Exe item at install time
        /// </summary>
        [BurnXmlAttribute("InstallCommandLine")]
        public string InstallCommandLine
        {
            get
            {
                return m_InstallCommandLine;
            }

            set
            {
                m_InstallCommandLine = value;
            }
        }

        /// <summary>
        /// Commandline args passed to Exe item at uninstall time
        /// </summary>
        [BurnXmlAttribute("UninstallCommandLine")]
        public string UninstallCommandLine
        {
            get
            {
                return m_UninstallCommandLine;
            }

            set
            {
                m_UninstallCommandLine = value;
            }
        }

        /// <summary>
        /// Commandline args passed to Exe item at repair time
        /// </summary>
        [BurnXmlAttribute("RepairCommandLine")]
        public string RepairCommandLine
        {
            get
            {
                return m_RepairCommandLine;
            }

            set
            {
                m_RepairCommandLine = value;
            }
        }

        /// <summary>
        /// Hint of the name of the log file the Exe will produce
        /// </summary>
        [BurnXmlAttribute("LogFileHint")]
        public string LogFileHint
        {
            get
            {
                return m_LogFileHint;
            }

            set
            {
                m_LogFileHint = value;
            }
        }

        /// <summary>
        /// Defines the type of Exe item like Cartman, IronMan, HotIron, MsuPackage, LocalExe or Unknown .exe type
        /// </summary>
        [BurnXmlAttribute("ExeType", ExeTypeEnum.Unknown)]
        public ExeTypeEnum? ExeType
        {
            get
            {
                return m_ExeType;
            }

            set
            {
                m_ExeType = value;
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
            this.CanonicalTargetName = "Test Exe " + System.IO.Path.GetFileName(file);
            this.DownloadSize = null;  // null value will cause it to be calculated
            this.EstimatedInstallTime = 5;
            this.InstallCommandLine = " ";
            this.InstalledProductSize = 1024;
            this.IsPresent.Expression = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.NeverTrue();
            this.LogFileHint = "";
            this.PerMachine = true;
            this.RepairCommandLine = " ";
            this.Rollback = true;
            this.Sha1HashValue = null;  // null value will cause it to be calculated
            this.CacheId = null;  // null value will cause it to be calculated
            this.SystemDriveSize = 0;
            this.UninstallCommandLine = " ";
            this.URL = null;
            this.Cache = true;
        }

        #endregion
    }
}
