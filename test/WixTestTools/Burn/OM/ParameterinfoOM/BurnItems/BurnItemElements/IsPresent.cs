//-----------------------------------------------------------------------
// <copyright file="IsPresent.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>IsPresent element OM</summary>
//-----------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.ElementAttribute;
using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions;

namespace Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemElements
{
    /// <summary>
    /// 
    /// </summary>
    [BurnXmlElement("IsPresent")]
    public class IsPresent
    {
        private Expression m_Expression;

        
        public IsPresent()
        {

        }

        public IsPresent(Expression expression)
        {
            this.Expression = expression;
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
