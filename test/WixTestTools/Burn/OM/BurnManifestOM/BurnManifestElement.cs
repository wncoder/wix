//-----------------------------------------------------------------------
// <copyright file="BurnManifestElement.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>BurnManifest element OM</summary>
//-----------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.ElementAttribute;

namespace Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.BurnManifestOM
{
    [BurnXmlElement("BurnManifest")]
    public class BurnManifestElement
    {
        // Xml attributes
        private string m_Xmlns;

        [BurnXmlAttribute("xmlns")]
        public string Xmlns
        {
            get
            { return m_Xmlns; }
            set
            { m_Xmlns = value; }
        }

        private Stub.StubElement m_Stub;
        private UX.UxElement m_UX;
        private Registration.RegistrationElement m_Registration;
        private Chain.ChainElement m_Chain;
        private List<Variables.VariableElement> m_Variables;

        [BurnXmlChildElement()]
        public Stub.StubElement Stub
        {
            get { return m_Stub; }
            set { m_Stub = value; }
        }

        [BurnXmlChildElement()]
        public UX.UxElement UX
        {
            get { return m_UX; }
            set { m_UX = value; }
        }

        [BurnXmlChildElement()]
        public Registration.RegistrationElement Registration
        {
            get { return m_Registration; }
            set { m_Registration = value; }
        }

        [BurnXmlChildElement()]
        public Chain.ChainElement Chain
        {
            get { return m_Chain; }
            set { m_Chain = value; }
        }

        public List<Variables.VariableElement> Variables
        {
            get
            {
                if (m_Variables == null) m_Variables = new List<Variables.VariableElement>();
                return m_Variables;
            }
            set
            {
                m_Variables = value;
            }
        }
        [BurnXmlChildElement()]
        public Variables.VariableElement[] VariablesArray
        {
            get { return Variables.ToArray(); }
        }

    }
}
