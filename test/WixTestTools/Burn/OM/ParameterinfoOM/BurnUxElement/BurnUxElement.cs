//-----------------------------------------------------------------------
// <copyright file="BurnUxElement.cs" company="Microsoft">
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
// <summary>BurnUxElement element OM</summary>
//-----------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.ElementAttribute;

namespace Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnUxElement
{
    [BurnXmlElement("Ux")]
    public class BurnUxElement
    {
        [BurnXmlElement("Payload")]
        public class BurnPayload
        {
            //Id='BurnUx_dll' FilePath='BurnUx.dll' Packaging='embedded' SourcePath='BurnUx.dll'

            private string m_Id;
            private string m_FilePath;
            private string m_Packaging;
            private string m_SourcePath;

            [BurnXmlAttribute("Id")]
            public string Id
            {
                get { return m_Id; }
                set { m_Id = value; }
            }

            [BurnXmlAttribute("FilePath")]
            public string FilePath
            {
                get { return m_FilePath; }
                set { m_FilePath = value; }
            }

            [BurnXmlAttribute("Packaging")]
            public string Packaging
            {
                get { return m_Packaging; }
                set { m_Packaging = value; }
            }

            [BurnXmlAttribute("SourcePath")]
            public string SourcePath
            {
                get { return m_SourcePath; }
                set { m_SourcePath = value; }
            }

        }

        private string m_UxDllPayloadId;
        private List<BurnPayload> m_Payloads;

        [BurnXmlAttribute("UxDllPayloadId")]
        public string UxDllPayloadId
        {
            get { return m_UxDllPayloadId; }
            set { m_UxDllPayloadId = value; }
        }

        public List<BurnPayload> Payloads
        {
            get
            {
                if (m_Payloads == null) m_Payloads = new List<BurnPayload>();
                return m_Payloads;
            }
            set
            {
                m_Payloads = value;
            }
        }
        [BurnXmlChildElement()]
        public BurnPayload[] PayloadsArray
        {
            get { return Payloads.ToArray(); }
        }

    }
}
