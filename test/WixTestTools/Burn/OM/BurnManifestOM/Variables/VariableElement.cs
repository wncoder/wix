//-----------------------------------------------------------------------
// <copyright file="VariableElement.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>Variable element OM</summary>
//-----------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.ElementAttribute;

namespace Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.BurnManifestOM.Variables
{
    [BurnXmlElement("Variable")]
    public class VariableElement
    {
        public enum VariableDataType
        {
            String,
            numeric,
            version
        }


        private string m_Id;
        private string m_Value;
        private VariableDataType m_Type;

        [BurnXmlAttribute("Id")]
        public string Id
        {
            get
            {
                return m_Id;
            }
            set
            {
                m_Id = value;
            }
        }

        [BurnXmlAttribute("Value")]
        public string Value
        {
            get
            {
                return m_Value;
            }
            set
            {
                m_Value = value;
            }
        }

        [BurnXmlAttribute("Type")]
        public VariableDataType Type
        {
            get
            {
                return m_Type;
            }

            set
            {
                m_Type = value;
            }
        }
    }
}
