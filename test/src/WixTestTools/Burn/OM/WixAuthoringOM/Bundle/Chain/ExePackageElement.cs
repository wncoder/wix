//-----------------------------------------------------------------------
// <copyright file="ExePackageElement.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>Exe element OM</summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.WixAuthoringOM.Bundle.Chain
{
    using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ElementAttribute;

    [BurnXmlElement("ExePackage")]
    public class ExePackageElement : Package
    {
        // Xml attributes

        private string m_PerMachine;
        [BurnXmlAttribute("PerMachine")]
        public string PerMachine
        {
            get { return m_PerMachine; }
            set { m_PerMachine = value; }
        }

        private string m_DetectCondition;
        [BurnXmlAttribute("DetectCondition")]
        public string DetectCondition
        {
            get { return m_DetectCondition; }
            set { m_DetectCondition = value; }
        }

        private string m_InstallCommand;
        [BurnXmlAttribute("InstallCommand")]
        public string InstallCommand
        {
            get { return m_InstallCommand; }
            set { m_InstallCommand = value; }
        }

        private string m_RepairCommand;
        [BurnXmlAttribute("RepairCommand")]
        public string RepairCommand
        {
            get { return m_RepairCommand; }
            set { m_RepairCommand = value; }
        }

        private string m_UninstallCommand;
        [BurnXmlAttribute("UninstallCommand")]
        public string UninstallCommand
        {
            get { return m_UninstallCommand; }
            set { m_UninstallCommand = value; }
        }
    }
}
