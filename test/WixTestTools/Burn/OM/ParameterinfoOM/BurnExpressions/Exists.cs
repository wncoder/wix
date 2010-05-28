//-----------------------------------------------------------------------
// <copyright file="Exists.cs" company="Microsoft">
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
// <summary>Exists element OM</summary>
//-----------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.ElementAttribute;
using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnOperands;

namespace Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions
{
    [BurnXmlElement("Exists")]
    public class Exists : Expression
    {
        public Exists(Operands operands)
        {
            this.ExpressionOperands = operands;
        }

        private Operands m_Operands;

        [BurnXmlChildElement()]
        public Operands ExpressionOperands
        {
            get
            {
                return m_Operands;
            }

            set
            {
                m_Operands = value;
            }
        }

    }
}
