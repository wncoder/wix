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
// <summary>ComponentSearch element OM</summary>
//-----------------------------------------------------------------------

namespace WixTest.Burn.OM.WixAuthoringOM.Bundle.Searches
{
    using WixTest.Burn.OM.ElementAttribute;

    [BurnXmlElement("ComponentSearch", BundleElement.wixUtilExtNamespace)]
    public class ComponentSearchElement : Searches
    {
        public enum ComponentSearchResultType
        {
            KeyPath,
            State,
            Directory
        }

        # region Private member

        private string m_Guid;
        private ComponentSearchResultType m_ResultType;
        private string m_ProductCode;

        # endregion

        # region Public property

        [BurnXmlAttribute("Guid")]
        public string Guid
        {
            get
            {
                return m_Guid;
            }
            set
            {
                m_Guid = value;
            }
        }

        [BurnXmlAttribute("Result")]
        public ComponentSearchResultType Result
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

        [BurnXmlAttribute("ProductCode")]
        public string ProductCode
        {
            get
            {
                return m_ProductCode;
            }
            set
            {
                m_ProductCode = value;
            }
        }


        # endregion
    }
}
