//-----------------------------------------------------------------------
// <copyright file="ProcessBlocks.cs" company="Microsoft">
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
// <summary>ProcessBlocks element OM</summary>
//-----------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.ElementAttribute;

namespace Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnSystemCheckElement
{
    [BurnXmlElement("ProcessBlocks")]
    public class ProcessBlocksGroup
    {
        [BurnXmlElement("ProcessBlock")]
        public class ProcessBlock
        {
            private string m_ImageName;

            [BurnXmlAttribute("ImageName")]
            public string ImageName
            {
                get
                {
                    return m_ImageName;
                }
                set
                {
                    m_ImageName = value;
                }
            }
        }

        private List<ProcessBlock> m_ProcessBlocks;

        [BurnXmlChildElement()]
        public ProcessBlock[] ProcessBlocksArray
        {
            get
            {
                return m_ProcessBlocks.ToArray();
            }
        }

        public List<ProcessBlock> ProcessBlocks
        {
            get
            {
                if (m_ProcessBlocks == null) m_ProcessBlocks = new List<ProcessBlock>();
                return m_ProcessBlocks;
            }
            set
            {
                m_ProcessBlocks = value;
            }
        }

    }
}
