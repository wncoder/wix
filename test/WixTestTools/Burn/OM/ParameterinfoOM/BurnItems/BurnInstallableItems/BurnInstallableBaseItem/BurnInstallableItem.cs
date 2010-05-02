//-----------------------------------------------------------------------
// <copyright file="BurnInstallableItem.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>Class to define all burn installable items like msi, msp, exe etc</summary>
//-----------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.ElementAttribute;
using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemElements;

namespace Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.InstallableItems
{
    /// <summary>
    /// 
    /// </summary>
    public abstract class BurnInstallableItem : BurnDownloadableBaseItem
    {
        # region Private member variables

        private uint m_EstimatedInstallTime;

        private CustomErrorHandling m_CustomErrorHandling;

        private List<RetryHelper> m_RetryHelper;

        private List<SubFileType> m_SubFiles;
        
        private bool? m_Rollback;

        private bool m_PerMachine;

        private bool? m_Cache;

        #endregion

        # region Public Properties

        public abstract string CacheId
        {
            get;

            set;
        }

        /// <summary>
        /// The estimated install time of an item in seconds
        /// </summary>
        [BurnXmlAttribute("EstimatedInstallTime", 6000)]
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
        /// Array of SubFiles that will be used by the parent item (i.e. a cab file that goes with an msi)
        /// </summary>
        [BurnXmlChildElement()]
        public SubFileType[] SubFilesArray
        {
            get
            {
                if (null == SubFiles) return null;
                return SubFiles.ToArray();
            }
        }

        /// <summary>
        /// List of SubFiles that will be used by the parent item (i.e. a cab file that goes with an msi)
        /// </summary>
        public List<SubFileType> SubFiles
        {
            get
            {
                if (m_RetryHelper == null) m_RetryHelper = new List<RetryHelper>();
                return m_SubFiles;
            }

            set { m_SubFiles = value; }
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
        /// Determine if we should rolback the item in the event of a subsequent item install failure
        /// </summary>
        [BurnXmlAttribute("Rollback", true)]
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

        /// <summary>
        /// Controls if the pacakge is executed in elevated process (if true) or not. Default value if not authored is false
        /// </summary>
        [BurnXmlAttribute("PerMachine", false)]
        public bool PerMachine
        {
            get
            {
                return m_PerMachine;
            }

            set
            {
                m_PerMachine = value;
            }
        }

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

    }
}
