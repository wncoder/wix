//-----------------------------------------------------------------------
// <copyright file="BlockerBaseItem.cs" company="Microsoft">
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
// <summary>Blocker base class. Warn, Stop, Success block would derive this class</summary>
//-----------------------------------------------------------------------
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.ElementAttribute;
using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnBlockers.BlockerBaseElement;

namespace Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnBlockers
{
    public abstract class BlockerBaseItem
    {
        // Make this nullable so if an author doesn't set it to a value it doesn't default to "0" which is almost certainly not what they want
        // if it is null, the XML generator won't write the attribute or value
        private int? m_ReturnCode;
        private List<BlockIfBaseItem> m_BlockIfBaseItems;

        [BurnXmlAttribute("ReturnCode")]
        public int? ReturnCode
        {
            get
            {
                return m_ReturnCode;
            }
            set
            {
                m_ReturnCode = value;
            }
        }

        public List<BlockIfBaseItem> BlockIfBaseItems
        {
            get
            {
                if (m_BlockIfBaseItems == null) m_BlockIfBaseItems = new List<BlockIfBaseItem>();
                return m_BlockIfBaseItems;
            }
            set
            {
                m_BlockIfBaseItems = value;
            }
        }

        /// <summary>
        /// List of BlockIfs converted to an array.  Burn XmlGenerator requires arrays not lists for xml elements
        /// </summary>
        [BurnXmlChildElement()]
        public BlockIfBaseItem[] BlockIfBaseItemsArray
        {
            get
            {
                return m_BlockIfBaseItems.ToArray();
            }
        }

    }
}
