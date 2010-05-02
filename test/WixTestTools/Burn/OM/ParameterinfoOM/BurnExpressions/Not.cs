//-----------------------------------------------------------------------
// <copyright file="Not.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>Not element OM</summary>
//-----------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.ElementAttribute;

namespace Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions
{
    [BurnXmlElement("Not")]
    public class Not : Expression
    {
        private Expression m_Expression;

        public Not(Expression expression)
        {
            this.UnaryExpression = expression;
        }

        [BurnXmlChildElement()]
        public Expression UnaryExpression
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
