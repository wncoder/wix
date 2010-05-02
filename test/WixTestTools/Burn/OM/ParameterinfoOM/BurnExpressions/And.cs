//-----------------------------------------------------------------------
// <copyright file="And.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>And element OM</summary>
//-----------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.ElementAttribute;

namespace Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions
{
    [BurnXmlElement("And")]
    public class And : ExpressionConnector
    {
        public And(Expression leftSideExpression, Expression rightSideExpression)
        {
            this.LeftSideExpression = leftSideExpression;
            this.RightSideExpression = rightSideExpression;
        }
    }
}
