//-----------------------------------------------------------------------
// <copyright file="MsiXmlBlob.cs" company="Microsoft">
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
// <summary>MsiXmlBlob element OM</summary>
//-----------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.Utility;
using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.ElementAttribute;

namespace Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnOperands
{
    /// <summary>
    /// Element that wraps MSI's MsiPatch element, which is an opaque blob that we get 
    /// back from calling MsiExtractPatchXMLData; used to determine whether a patch applies
    /// to any products on the box, without having the .msp file
    /// </summary>
    [BurnXmlElement("MsiXmlBlob")]
    public class MsiXmlBlob : Operands
    {
        private string m_MspLocation;
        private string m_msiXmlBlob;

        public MsiXmlBlob(string mspLocation)
        {
            m_MspLocation = mspLocation;
            this.XmlBlob = XmlBlob;
        }

        [BurnXmlBlob()]
        public string XmlBlob
        {
            get
            {
                if (string.IsNullOrEmpty(m_msiXmlBlob))
                    m_msiXmlBlob = MsiUtils.GetPatchXmlBlob(m_MspLocation);

                return m_msiXmlBlob;
            }

            set
            {
                m_msiXmlBlob = value;
            }
        }
    }
}
