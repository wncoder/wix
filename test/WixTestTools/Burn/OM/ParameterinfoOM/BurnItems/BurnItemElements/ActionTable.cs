//-----------------------------------------------------------------------
// <copyright file="ActionTable.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>ActionTable element OM</summary>
//-----------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.ElementAttribute;

namespace Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemElements
{
    /// <summary>
    /// 
    /// </summary>
    [BurnXmlElement("ActionTable")]
    public class ActionTableElement
    {

        [BurnXmlElement("InstallAction")]
        public class InstallActionElement : ActionTableSubelement
        {
            public InstallActionElement()
            {
            }

            public InstallActionElement(ActionType isPresent, ActionType isAbsent)
                : base(isPresent, isAbsent)
            {
            }
        }

        [BurnXmlElement("UninstallAction")]
        public class UninstallActionElement : ActionTableSubelement
        {
            public UninstallActionElement()
            {
            }

            public UninstallActionElement(ActionType isPresent, ActionType isAbsent)
                : base(isPresent, isAbsent)
            {
            }
        }

        [BurnXmlElement("RepairAction")]
        public class RepairActionElement : ActionTableSubelement
        {
            public RepairActionElement()
            {
            }

            public RepairActionElement(ActionType isPresent, ActionType isAbsent)
                : base(isPresent, isAbsent)
            {
            }

        }

        public class ActionTableSubelement
        {
            # region Public enum

            public enum ActionType
            {
                install,
                uninstall,
                repair,
                noop
            }

            #endregion

            public ActionTableSubelement()
            {
                IfPresent = ActionType.noop;
                IfAbsent = ActionType.noop;
            }

            public ActionTableSubelement(ActionType ifPresent, ActionType ifAbsent)
            {
                IfPresent = ifPresent;
                IfAbsent = ifAbsent;
            }

            [BurnXmlAttribute("IfPresent")]
            public ActionType IfPresent
            {
                get { return m_IfPresent; }
                set { m_IfPresent = value; }
            }

            [BurnXmlAttribute("IfAbsent")]
            public ActionType IfAbsent
            {
                get { return m_IfAbsent; }
                set { m_IfAbsent = value; }
            }

            private ActionType m_IfPresent;
            private ActionType m_IfAbsent;
        }

        [BurnXmlChildElement()]
        public InstallActionElement InstallAction
        {
            get
            {
                if (m_InstallAction == null) m_InstallAction = new InstallActionElement();
                return m_InstallAction;
            }
            set
            {
                m_InstallAction = value;
            }
        }

        [BurnXmlChildElement()]
        public UninstallActionElement UninstallAction
        {
            get
            {
                if (m_UninstallAction == null) m_UninstallAction = new UninstallActionElement();
                return m_UninstallAction;
            }
            set
            {
                m_UninstallAction = value;
            }
        }

        [BurnXmlChildElement()]
        public RepairActionElement RepairAction
        {
            get
            {
                if (m_RepairAction == null) m_RepairAction = new RepairActionElement();
                return m_RepairAction;
            }
            set
            {
                m_RepairAction = value;
            }
        }

        public ActionTableElement()
        {
            this.InstallAction = new InstallActionElement(ActionTableSubelement.ActionType.noop, ActionTableSubelement.ActionType.install);
            this.UninstallAction = new UninstallActionElement(ActionTableSubelement.ActionType.uninstall, ActionTableSubelement.ActionType.noop);
            this.RepairAction = new RepairActionElement(ActionTableSubelement.ActionType.repair, ActionTableSubelement.ActionType.install);
        }

        public ActionTableElement(
            ActionTableSubelement.ActionType installIsPresent,
            ActionTableSubelement.ActionType installIsAbsent,
            ActionTableSubelement.ActionType uninstallIsPresent,
            ActionTableSubelement.ActionType uninstallIsAbsent,
            ActionTableSubelement.ActionType repairIsPresent,
            ActionTableSubelement.ActionType repairIsAbsent)
        {
            this.InstallAction = new InstallActionElement(installIsPresent, installIsAbsent);
            this.UninstallAction = new UninstallActionElement(uninstallIsPresent, uninstallIsAbsent);
            this.RepairAction = new RepairActionElement(repairIsPresent, repairIsAbsent);
        }

        private InstallActionElement m_InstallAction;
        private UninstallActionElement m_UninstallAction;
        private RepairActionElement m_RepairAction;


    }
}

