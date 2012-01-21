//-----------------------------------------------------------------------
// <copyright file="VariableElement.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>ComponentSearch element OM</summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.WixAuthoringOM.Bundle.Searches
{
    using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ElementAttribute;

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
