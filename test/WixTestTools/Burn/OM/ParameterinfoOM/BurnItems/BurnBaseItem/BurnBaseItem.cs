//-----------------------------------------------------------------------
// <copyright file="BurnBaseItem.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>base class for installable, non-installable and downloadable items</summary>
//-----------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.ElementAttribute;
using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemElements;

namespace Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems
{
    /// <summary>
    /// 
    /// </summary>
    public abstract class BurnBaseItems
    {
        # region Public enum

        public enum YesNoType
        {
            yes,
            no
        }

        #endregion

        # region Private member variables

        private string m_Id;

        private IsPresent m_IsPresent;

        private ApplicableIf m_ApplicableIf;

        private ActionTableElement m_ActionTable;        

        private string m_SourceFilePath;

        #endregion

        # region Public Properties

        /// <summary>
        /// The Item's Id
        /// </summary>
        [BurnXmlAttribute("Id")]
        public string Id
        {
            get
            {
                return m_Id;
            }

            set
            {
                m_Id = value;
            }
        }

        /// <summary>
        /// Element that wraps a bunch of expressions and operands, to decide if its parent has been installed based on the state of this machine
        /// </summary>
        [BurnXmlChildElement()]
        public IsPresent IsPresent
        {
            get
            {
                if (m_IsPresent == null)
                    m_IsPresent = new IsPresent();

                return m_IsPresent;
            }

            set
            {
                m_IsPresent = value;
            }
        }

        /// <summary>
        /// Element that wraps a bunch of expressions and operands, to decide if its parent is applicable for install on the machine or not
        /// </summary>
        [BurnXmlChildElement()]
        public ApplicableIf ApplicableIf
        {
            get
            {
                if (m_ApplicableIf == null)
                    m_ApplicableIf = new ApplicableIf();

                return m_ApplicableIf;
            }

            set
            {
                m_ApplicableIf = value;
            }
        }

        /// <summary>
        /// Per item table that specifies what operation to take when the item is present and not-present at install, uninstall and repair
        /// </summary>
        [BurnXmlChildElement()]
        public ActionTableElement ActionTable
        {
            get
            {
                if (m_ActionTable == null)
                    m_ActionTable = new ActionTableElement();

                return m_ActionTable;
            }

            set
            {
                m_ActionTable = value;
            }
        }

        /// <summary>
        /// To define the source file path of payload
        /// </summary>
        public string SourceFilePath
        {
            get
            {
                return m_SourceFilePath;
            }

            set
            {
                m_SourceFilePath = value;
            }
        }

        #endregion
    }
}
