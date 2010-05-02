//-----------------------------------------------------------------------
// <copyright file="MsiProductVersion.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>MsiProductVersion element OM</summary>
//-----------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.ElementAttribute;

namespace Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnOperands
{
    /// <summary>
    /// Returns the path to the local, cached MSP file, given a ProductCode
    /// </summary>
    [BurnXmlElement("MsiGetCachedPatchPath")]
    public class MsiGetCachedPatchPath : Operands
    {
        private string m_PatchCode;

        public MsiGetCachedPatchPath(string patchCode)
        {
            this.PatchCode = patchCode;
        }

        [BurnXmlAttribute("PatchCode")]
        public string PatchCode
        {
            get
            {
                return m_PatchCode;
            }

            set
            {
                m_PatchCode = value;
            }
        }
    }
}
