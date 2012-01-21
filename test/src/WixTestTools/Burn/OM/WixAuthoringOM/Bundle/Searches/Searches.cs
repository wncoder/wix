//-----------------------------------------------------------------------
// <copyright file="VariableElement.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>Searches OM base class</summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.WixAuthoringOM.Bundle.Searches
{
    using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ElementAttribute;

    public abstract class Searches
    {
        # region Private member

        private string m_Id;
        private string m_After;
        private string m_Variable;
        private string m_Condition;

        # endregion

        # region Public property

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

        [BurnXmlAttribute("After")]
        public string After
        {
            get
            {
                return m_After;
            }
            set
            {
                m_After = value;
            }
        }

        [BurnXmlAttribute("Variable")]
        public string Variable
        {
            get
            {
                return m_Variable;
            }
            set
            {
                m_Variable = value;
            }
        }

        [BurnXmlAttribute("Condition")]
        public string Condition
        {
            get
            {
                return m_Condition;
            }
            set
            {
                m_Condition = value;
            }
        }

        # endregion

    }
}
