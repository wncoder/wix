//-----------------------------------------------------------------------
// <copyright file="ExpressionAttribute.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>To define complete expression</summary>
//-----------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.ElementAttribute;
using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnOperands;

namespace Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions
{
    public abstract class ExpressionAttribute : Expression
    {
        public enum BoolWhenNonExistentType
        {
            True,
            False
        }

        private string m_LeftHandSide;

        private Operands m_Operands;

        private BoolWhenNonExistentType m_BoolWhenNonExistent;

        [BurnXmlAttribute("LeftHandSide")]
        public string LeftHandSide
        {
            get
            {
                return m_LeftHandSide;
            }

            set
            {
                m_LeftHandSide = value;
            }
        }

        [BurnXmlAttribute("BoolWhenNonExistent")]
        public BoolWhenNonExistentType BoolWhenNonExistent
        {
            get
            {
                return m_BoolWhenNonExistent;
            }

            set
            {
                m_BoolWhenNonExistent = value;
            }
        }

        [BurnXmlChildElement()]
        public Operands ExpressionOperands
        {
            get
            {
                return m_Operands;
            }

            set
            {
                m_Operands = value;
            }
        }
    }
}
