//-----------------------------------------------------------------------
// <copyright file="ApplicableIf.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>ApplicableIf element OM</summary>
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
    [BurnXmlElement("ApplicableIf")]
    public class ApplicableIf
    {
        private Expression m_applicablefExpression;

        public ApplicableIf()
        {

        }

        public ApplicableIf(Expression expression)
        {
            this.ApplicableExpression = expression;
        }

        [BurnXmlChildElement()]
        public Expression ApplicableExpression
        {
            get
            {
                return m_applicablefExpression;
            }

            set
            {
                m_applicablefExpression = value;
            }
        }
    }
}
