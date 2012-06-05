//-----------------------------------------------------------------------
// <copyright file="UxBase.cs" company="Microsoft Corporation">
//   Copyright (c) 2004, Microsoft Corporation.
//   This software is released under Common Public License Version 1.0 (CPL).
//   The license and further copyright text can be found in the file LICENSE.TXT
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
// <summary>base for different Burn UX</summary>
//-----------------------------------------------------------------------

namespace WixTest.Burn.LayoutManager.UX
{
    using WixTest.Burn.OM.WixAuthoringOM;

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

        public abstract WixTest.Burn.OM.WixAuthoringOM.Bundle.UX.UXElement GetWixBundleUXElement();
    }
}
