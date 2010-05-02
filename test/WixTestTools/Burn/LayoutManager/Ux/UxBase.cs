//-----------------------------------------------------------------------
// <copyright file="UxBase.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>base for different Burn UX</summary>
//-----------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Microsoft.Tools.WindowsInstallerXml.Test.Burn.LayoutManager.Ux
{
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

        public abstract void CopyAndConfigureUx(string LayoutLocation);

        public abstract Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.BurnManifestOM.UX.UxElement GetBurnManifestUxElement();
    }
}
