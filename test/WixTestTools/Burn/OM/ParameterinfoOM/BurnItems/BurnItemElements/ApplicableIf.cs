//-----------------------------------------------------------------------
// <copyright file="ApplicableIf.cs" company="Microsoft">
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
