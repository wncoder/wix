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
// <summary>FileSearch element OM</summary>
//-----------------------------------------------------------------------

namespace WixTest.Burn.OM.WixAuthoringOM.Bundle.Searches
{
    using WixTest.Burn.OM.ElementAttribute;

    [BurnXmlElement("FileSearch", BundleElement.wixUtilExtNamespace)]
    public class FileSearchElement : Searches
    {
        public enum FileSearchResultType
        {
            Exists,
            Version
        }

        # region Private member

        private string m_Path;
        private FileSearchResultType m_ResultType;

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
        public FileSearchResultType Result
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
