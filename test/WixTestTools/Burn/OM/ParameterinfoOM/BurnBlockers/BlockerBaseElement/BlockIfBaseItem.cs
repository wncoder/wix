//-----------------------------------------------------------------------
// <copyright file="BlockIfBaseItem.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>BlockIf base class BlockIfGroup and BlockIf element class would derive from this class</summary>
//-----------------------------------------------------------------------
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.ElementAttribute;

namespace Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnBlockers.BlockerBaseElement
{
    public class BlockIfBaseItem
    {
        private string m_DisplayText;

        [BurnXmlAttribute("DisplayText")]
        public string DisplayText
        {
            get
            {
                if (m_DisplayText == null) m_DisplayText = "";
                return m_DisplayText;
            }
            set
            {
                m_DisplayText = value;
            }
        }
    }
}
