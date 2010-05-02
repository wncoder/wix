//-----------------------------------------------------------------------
// <copyright file="ExpressionConnector.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>Defines the left and right side expression</summary>
//-----------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.ElementAttribute;

namespace Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions
{
    public abstract class ExpressionConnector : Expression
    {
        private Expression m_LeftSideExpression;

        private Expression m_RightSideExpression;

        [BurnXmlChildElement()]
        public Expression LeftSideExpression
        {
            get
            {
                return m_LeftSideExpression;
            }

            set
            {
                m_LeftSideExpression = value;
            }
        }
        
        [BurnXmlChildElement()]
        public Expression RightSideExpression
        {
            get
            {
                return m_RightSideExpression;
            }

            set
            {
                m_RightSideExpression = value;
            }
        }
    }
}
