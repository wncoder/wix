//-----------------------------------------------------------------------
// <copyright file="BlockIfBaseItem.cs" company="Microsoft">
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
// <summary>BlockIf base class BlockIfGroup and BlockIf element class would derive from this class</summary>
//-----------------------------------------------------------------------
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.ElementAttribute;

namespace Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnBlockers.BlockerBaseElement
{
    public class BlockIfBaseItem
    {
        private string m_DisplayText;

        [BurnXmlAttribute("DisplayText")]
        public string DisplayText
        {
            get
            {
                if (m_DisplayText == null) m_DisplayText = "";
                return m_DisplayText;
            }
            set
            {
                m_DisplayText = value;
            }
        }
    }
}
