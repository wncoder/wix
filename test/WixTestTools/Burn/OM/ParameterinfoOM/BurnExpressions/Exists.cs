//-----------------------------------------------------------------------
// <copyright file="Exists.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
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
