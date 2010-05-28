//-----------------------------------------------------------------------
// <copyright file="FileItem.cs" company="Microsoft">
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
// <summary>File element OM</summary>
//-----------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems;
using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.ElementAttribute;

namespace Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnNonInstallableItems
{
    [BurnXmlElement("File")]
    public class FileItem : BurnDownloadableBaseItem
    {
        # region Private member variables

        private bool? m_Cache;


        #endregion

        # region Public Property

        /// <summary>
        /// Determine if we should cache the item
        /// </summary>
        [BurnXmlAttribute("Cache", true)]
        public bool? Cache
        {
            get
            {
                return m_Cache;
            }

            set
            {
                m_Cache = value;
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
            this.DownloadSize = null;  // null value will cause it to be calculated
            this.InstalledProductSize = (uint)this.DownloadSize;
            this.IsPresent.Expression = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.NeverTrue();
            this.Sha1HashValue = null;  // null value will cause it to be calculated
            this.SystemDriveSize = 0;
            this.URL = null;
            this.Cache = true;
        }

        #endregion
    }
}
