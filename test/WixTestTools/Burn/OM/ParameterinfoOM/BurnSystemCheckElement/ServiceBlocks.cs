//-----------------------------------------------------------------------
// <copyright file="ServiceBlocks.cs" company="Microsoft">
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
// <summary>ServiceBlocks element OM</summary>
//-----------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.ElementAttribute;

namespace Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnSystemCheckElement
{
    [BurnXmlElement("ServiceBlocks")]
    public class ServiceBlocksGroup
    {
        [BurnXmlElement("ServiceBlock")]
        public class ServiceBlock
        {
            private string m_ServiceName;

            [BurnXmlAttribute("ServiceName")]
            public string ServiceName
            {
                get
                {
                    return m_ServiceName;
                }
                set
                {
                    m_ServiceName = value;
                }
            }
        }

        private List<ServiceBlock> m_ServiceBlocks;

        [BurnXmlChildElement()]
        public ServiceBlock[] ServiceBlocksArray
        {
            get
            {
                return m_ServiceBlocks.ToArray();
            }
        }

        public List<ServiceBlock> ServiceBlocks
        {
            get
            {
                if (m_ServiceBlocks == null) m_ServiceBlocks = new List<ServiceBlock>();
                return m_ServiceBlocks;
            }
            set
            {
                m_ServiceBlocks = value;
            }
        }

    }
}
