//-----------------------------------------------------------------------
// <copyright file="BlockIf.cs" company="Microsoft">
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
// <summary>BlockIf element OM</summary>
//-----------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.ElementAttribute;
using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions;

namespace Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnBlockers.BlockerBaseElement
{
    [BurnXmlElement("BlockIf")]
    public class BlockIf : BlockIfBaseItem
    {
        private string m_ID;
        private Expression m_Expression;

        [BurnXmlAttribute("ID")]
        public string ID
        {
            get
            {
                return m_ID;
            }
            set
            {
                m_ID = value;
            }
        }

        [BurnXmlChildElement()]
        public Expression Expression
        {
            get
            {
                return m_Expression;
            }

            set
            {
                m_Expression = value;
            }
        }
    }
}
