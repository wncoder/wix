//-----------------------------------------------------------------------
// <copyright file="SystemCheck.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>SystemCheck element OM</summary>
//-----------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.ElementAttribute;

namespace Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnSystemCheckElement
{
    [BurnXmlElement("SystemCheck")]
    public class SystemCheck
    {
        private ProcessBlocksGroup m_ProcessBlocks;
        private ServiceBlocksGroup m_ServiceBlocks;
        private ProductDriveHints m_ProductDriveHints;

        [BurnXmlChildElement()]
        public ProcessBlocksGroup ProcessBlocks
        {
            get
            {
                return m_ProcessBlocks;
            }
            set
            {
                m_ProcessBlocks = value;
            }
        }

        [BurnXmlChildElement()]
        public ServiceBlocksGroup ServiceBlocks
        {
            get
            {
                return m_ServiceBlocks;
            }
            set
            {
                m_ServiceBlocks = value;
            }
        }

        [BurnXmlChildElement()]
        public ProductDriveHints ProductDriveHints
        {
            get
            {
                return m_ProductDriveHints;
            }
            set
            {
                m_ProductDriveHints = value;
            }
        }
    }
}
