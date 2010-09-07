//-----------------------------------------------------------------------
// <copyright file="MsiPackageElement.cs" company="Microsoft">
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
// <summary>MsiPackage element OM</summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.WixAuthoringOM.Bundle.Chain
{
    using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ElementAttribute;

    [BurnXmlElement("MsiPackage")]
    public class MsiPackageElement : Package
    {
        private MsiPropertyElement m_MsiPropertylement;

        [BurnXmlChildElement()]
        public MsiPropertyElement MsiProperty
        {
            get
            {
                return m_MsiPropertylement;
            }
            set
            {
                m_MsiPropertylement = value;
            }
        }
    }
}
