//-----------------------------------------------------------------------
// <copyright file="VariableElement.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>DirectorySearch element OM</summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.WixAuthoringOM.Bundle.Searches
{
    using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ElementAttribute;

    [BurnXmlElement("DirectorySearch", BundleElement.wixUtilExtNamespace)]
    public class DirectorySearchElement : Searches
    {
        public enum DirectorySearchResultType
        {
            Exists
        }

        # region Private member

        private string m_Path;
        private DirectorySearchResultType m_ResultType;

        # endregion

        # region Public property

        [BurnXmlAttribute("Path")]
        public string Path
        {
            get
            {
                return m_Path;
            }
            set
            {
                m_Path = value;
            }
        }

        [BurnXmlAttribute("Result")]
        public DirectorySearchResultType Result
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

        # endregion

    }
}
