//-----------------------------------------------------------------------
// <copyright file="VariableElement.cs" company="Microsoft">
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
