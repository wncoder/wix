//-----------------------------------------------------------------------
// <copyright file="Or.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>Or element OM</summary>
//-----------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.ElementAttribute;

namespace Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions
{
    [BurnXmlElement("Or")]
    public class Or : ExpressionConnector
    {
        public Or(Expression leftSideExpression, Expression rightSideExpression)
        {
            this.LeftSideExpression = leftSideExpression;
            this.RightSideExpression = rightSideExpression;
        }        
    }
}
