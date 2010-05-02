//-----------------------------------------------------------------------
// <copyright file="MsiXmlBlob.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
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
