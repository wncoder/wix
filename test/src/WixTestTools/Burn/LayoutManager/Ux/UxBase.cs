//-----------------------------------------------------------------------
// <copyright file="UxBase.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>base for different Burn UX</summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Burn.LayoutManager.UX
{
    using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.WixAuthoringOM;

    public abstract class UxBase
    {
        private string m_UxBinaryFilename;
        public string UxBinaryFilename
        {
            get
            {
                return m_UxBinaryFilename;
            }
            set
            {
                m_UxBinaryFilename = value;
            }
        }

        public abstract void CopyAndConfigureUx(string LayoutLocation, WixElement Wix);

        public abstract Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.WixAuthoringOM.Bundle.UX.UXElement GetWixBundleUXElement();
    }
}
