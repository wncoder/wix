//-----------------------------------------------------------------------
// <copyright file="VariableElement.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
//    
//    The use and distribution terms for this software are covered by the
//    Common Public License 1.0 (http://opensource.org/licenses/cpl1.0.php)
//    which can be found in the file CPL.TXT at the root of this distribution.
//    By using this software in any fashion, you are agreeing to be bound by
//    the terms of this license.
//    
//    You must not remove this notice, or any other, from this software.
// </copyright>
// <summary>Variable element OM</summary>
//-----------------------------------------------------------------------

namespace WixTest.Burn.OM.WixAuthoringOM.Bundle.Variable
{
    using WixTest.Burn.OM.ElementAttribute;

    [BurnXmlElement("Variable")]
    public class VariableElement
    {
        public enum VariableDataType
        {
            String,
            numeric,
            version
        }


        private string m_Name;
        private string m_Value;
        private VariableDataType m_Type;

        [BurnXmlAttribute("Name")]
        public string Name
        {
            get
            {
                return m_Name;
            }
            set
            {
                m_Name = value;
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