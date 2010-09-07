//-----------------------------------------------------------------------
// <copyright file="ExePackageElement.cs" company="Microsoft">
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
