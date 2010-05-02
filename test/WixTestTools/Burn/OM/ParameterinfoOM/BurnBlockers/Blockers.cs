
//-----------------------------------------------------------------------
// <copyright file="Blockers.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>Blockers element OM</summary>
//-----------------------------------------------------------------------using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.ElementAttribute;

namespace Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnBlockers
{
    [BurnXmlElement("Blockers")]
    public class Blockers
    {
        private SuccessBlockers m_SuccessBlockersGroup;
        private StopBlockers m_StopBlockersGroup;
        private WarnBlockers m_WarnBlockersGroup;

        [BurnXmlChildElement()]
        public SuccessBlockers SuccessBlockersGroup
        {
            get { return m_SuccessBlockersGroup; }
            set { m_SuccessBlockersGroup = value; }
        }

        [BurnXmlChildElement()]
        public StopBlockers StopBlockersGroup
        {
            get { return m_StopBlockersGroup; }
            set { m_StopBlockersGroup = value; }
        }
        
        [BurnXmlChildElement()]
        public WarnBlockers WarnBlockersGroup
        {
            get { return m_WarnBlockersGroup; }
            set { m_WarnBlockersGroup = value; }
        }
    }
}
