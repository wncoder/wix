﻿//-----------------------------------------------------------------------
// <copyright file="ServiceControlItem.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>ServiceControl element OM</summary>
//-----------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems;
using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemElements;
using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.ElementAttribute;

namespace Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnNonInstallableItems
{
    [BurnXmlElement("ServiceControlItem")]
    public class ServiceControlItem : BurnBaseItems
    {
        # region Public enum

        public enum ServiceControlType
        {
            Start,
            Stop,
            Pause,
            Resume
        }

        #endregion

        # region Private member variables

        private CustomErrorHandling m_CustomErrorHandling;

        private List<RetryHelper> m_RetryHelper;

        private string m_Name;

        private ServiceControlType m_Control;

        private uint m_EstimatedInstallTime;

        private string m_CanonicalTargetName;

        private bool? m_PerMachine;

        #endregion

        # region Public Property

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
        /// List of  RetryHelper converted to an array.  Burn XmlGenerator requires arrays not lists for xml elements
        /// </summary>
        [BurnXmlChildElement()]
        public RetryHelper[] RetryHelperArray
        {
            get
            {
                return RetryHelper.ToArray();
            }
        }

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

        [BurnXmlAttribute("Control")]
        public ServiceControlType Control
        {
            get
            {
                return m_Control;
            }

            set
            {
                m_Control = value;
            }
        }

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

        [BurnXmlAttribute("PerMachine")]
        public bool? PerMachine
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

        #endregion
    }
}
