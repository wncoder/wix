//-----------------------------------------------------------------------
// <copyright file="BlockIfGroup.cs" company="Microsoft">
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
// <summary>BlockIfGroup element OM</summary>
//-----------------------------------------------------------------------
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.ElementAttribute;
using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions;

namespace Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnBlockers.BlockerBaseElement
{
    [BurnXmlElement("BlockIfGroup")]
    public class BlockIfGroup : BlockIfBaseItem
    {
        private List<BlockIf> m_BlockIfs;

        /// <summary>
        /// List of BlockIfs converted to an array.  Burn XmlGenerator requires arrays not lists for xml elements
        /// </summary>
        [BurnXmlChildElement()]
        public BlockIf[] BlockIfsArr
        {
            get
            {
                return m_BlockIfs.ToArray();
            }
        }

        public List<BlockIf> BlockIfs
        {
            get
            {
                if (m_BlockIfs == null) m_BlockIfs = new List<BlockIf>();
                return m_BlockIfs;
            }

            set
            {
                m_BlockIfs = value;
            }
        }
    }
}
