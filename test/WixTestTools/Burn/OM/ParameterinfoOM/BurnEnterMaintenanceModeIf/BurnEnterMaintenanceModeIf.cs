//-----------------------------------------------------------------------
// <copyright file="BurnEnterMaintenanceModeIf.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>EnterMaintenanceModeIf element OM</summary>
//-----------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.ElementAttribute;
using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions;

namespace Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnEnterMaintenanceModeIf
{
    /// <summary>
    /// 
    /// </summary>
    [BurnXmlElement("EnterMaintenanceModeIf")]
    public class EnterMaintenanceModeIf
    {
        private Expression m_Expression;

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
