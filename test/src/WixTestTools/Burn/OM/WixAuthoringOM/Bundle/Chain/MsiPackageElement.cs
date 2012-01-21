//-----------------------------------------------------------------------
// <copyright file="MsiPackageElement.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
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
