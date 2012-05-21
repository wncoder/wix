//-----------------------------------------------------------------------
// <copyright file="UxBase.cs" company="Microsoft">
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
