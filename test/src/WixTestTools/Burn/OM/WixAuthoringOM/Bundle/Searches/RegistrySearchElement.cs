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
// <summary>RegistrySearch element OM</summary>
//-----------------------------------------------------------------------

namespace WixTest.Burn.OM.WixAuthoringOM.Bundle.Searches
{
    using WixTest.Burn.OM.ElementAttribute;

    [BurnXmlElement("RegistrySearch", BundleElement.wixUtilExtNamespace)]
    public class RegistrySearchElement : Searches
    {
        public enum RegistrySearchResultType
        {
            Exists,
            Value
        }

        public enum RegRoot
        {
            HKCU,
            HKLM,
            HKCR,
            HKU
        }

        public enum YesNoType
        {
            yes,
            no
        }

        public enum ResultFormat
        {
            Raw,
            Compatible
        }

        # region Private member

        private YesNoType m_ExpandEnvironmentVariables;
        private ResultFormat m_Format;
        private string m_Key;
        private RegistrySearchResultType m_ResultType;
        private RegRoot m_Root;
        private string m_Value;

        # endregion

        # region Public property

        [BurnXmlAttribute("ExpandEnvironmentVariables")]
        public YesNoType ExpandEnvironmentVariables
        {
            get
            {
                return m_ExpandEnvironmentVariables;
            }
            set
            {
                m_ExpandEnvironmentVariables = value;
            }
        }

        [BurnXmlAttribute("Format")]
        public ResultFormat Format
        {
            get
            {
                return m_Format;
            }
            set
            {
                m_Format = value;
            }
        }

        [BurnXmlAttribute("Key")]
        public string Key
        {
            get
            {
                return m_Key;
            }
            set
            {
                m_Key = value;
            }
        }

        [BurnXmlAttribute("Result")]
        public RegistrySearchResultType Result
        {
            get
            {
                return m_ResultType;
            }
            set
            {
                m_ResultType = value;
            }
        }

        [BurnXmlAttribute("Root")]
        public RegRoot Root
        {
            get
            {
                return m_Root;
            }
            set
            {
                m_Root = value;
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

        # endregion

    }
}
