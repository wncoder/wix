//-----------------------------------------------------------------------
// <copyright file="Equals.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>Equals element OM</summary>
//-----------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnOperands;
using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.ElementAttribute;

namespace Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions
{
    [BurnXmlElement("Equals")]
    public class Equals : ExpressionAttribute
    {
        public Equals(string leftHandSide, BoolWhenNonExistentType boolWhenNonExistent, Operands operands)
        {
            this.LeftHandSide = leftHandSide;
            this.BoolWhenNonExistent = boolWhenNonExistent;
            this.ExpressionOperands = operands;
        }
    }
}
