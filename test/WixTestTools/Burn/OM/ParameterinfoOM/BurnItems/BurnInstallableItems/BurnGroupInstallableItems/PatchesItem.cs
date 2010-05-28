//-----------------------------------------------------------------------
// <copyright file="PatchesItem.cs" company="Microsoft">
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
// <summary>Patches element OM</summary>
//-----------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems;
using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemElements;
using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.ElementAttribute;

namespace Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnInstallableItems.BurnGroupInstallableItems
{
    [BurnXmlElement("Patches")]
    public class PatchesItem //: BurnBaseItems
    {
        private ActionTableElement m_ActionTable;
        private CustomErrorHandling m_CustomErrorHandling;
        private List<RetryHelper> m_RetryHelper;
        private List<MspItem> m_Msps;
        private bool? m_IgnoreDownloadFailure;
        private bool? m_Rollback;
   
        /// <summary>
        /// Per item table that specifies what operation to take when the item is present and not-present at install, uninstall and repair
        /// </summary>
        [BurnXmlChildElement()]
        public ActionTableElement ActionTable
        {
            get
            {
                if (m_ActionTable == null)
                    m_ActionTable = new ActionTableElement();

                return m_ActionTable;
            }

            set
            {
                m_ActionTable = value;
            }
        }

        /// <summary>
        /// The custom errors helping block that defines how to handle specific errors
        /// </summary>
        [BurnXmlChildElement()]
        public CustomErrorHandling CustomErrorHandling
        {
            get
            {
                return m_CustomErrorHandling;
            }

            set
            {
                m_CustomErrorHandling = value;
            }
        }

        /// <summary>
        /// Array of Exes that will be used to Retry after the item returns a specific retry return codes
        /// </summary>
        [BurnXmlChildElement()]
        public RetryHelper[] RetryHelperArray
        {
            get
            {
                return RetryHelper.ToArray();
            }
        }

        /// <summary>
        /// List of Exes that will be used to Retry after the item returns a specific retry return codes
        /// </summary>
        public List<RetryHelper> RetryHelper
        {
            get
            {
                if (m_RetryHelper == null) m_RetryHelper = new List<RetryHelper>();
                return m_RetryHelper;
            }

            set
            {
                m_RetryHelper = value;
            }
        }

        /// <summary>
        /// Array of Msps that will be installed by this patches group
        /// </summary>
        [BurnXmlChildElement()]
        public MspItem[] MspArray
        {
            get
            {
                return m_Msps.ToArray();
            }
        }

        /// <summary>
        /// List of Msps that will be installed by this patches group
        /// </summary>
        public List<MspItem> Msps
        {
            get
            {
                if (m_Msps == null) m_Msps = new List<MspItem>();
                return m_Msps;
            }

            set
            {
                m_Msps = value;
            }
        }

        [BurnXmlAttribute("IgnoreDownloadFailure")]
        public bool? IgnoreDownloadFailure
        {
            get
            {
                return m_IgnoreDownloadFailure;
            }

            set
            {
                m_IgnoreDownloadFailure = value;
            }
        }

        [BurnXmlAttribute("Rollback")]
        public bool? Rollback
        {
            get
            {
                return m_Rollback;
            }

            set
            {
                m_Rollback = value;
            }
        }
    }
}
