//-----------------------------------------------------------------------
// <copyright file="ExpressionConnector.cs" company="Microsoft">
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
