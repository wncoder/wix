//-----------------------------------------------------------------------
// <copyright file="ProductDriveHints.cs" company="Microsoft">
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
// <summary>ProductDriveHints element OM</summary>
//-----------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Collections.Specialized;
using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.ElementAttribute;

namespace Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnSystemCheckElement
{
    [BurnXmlElement("ProductDriveHints")]
    public class ProductDriveHints
    {
        public abstract class Hint
        {
        }

        [BurnXmlElement("ComponentHint")]
        public class ComponentHint : Hint
        {
            private string m_Guid;

            [BurnXmlAttribute("Guid")]
            public string Guid
            {
                get
                {
                    return m_Guid;
                }
                set
                {
                    m_Guid = value;
                }
            }
        }

        [BurnXmlElement("RegKeyHint")]
        public class RegKeyHint : Hint
        {
            private string m_Location;

            [BurnXmlAttribute("Location")]
            public string Location
            {
                get
                {
                    return m_Location;
                }
                set
                {
                    m_Location = value;
                }
            }
        }

        private List<Hint> m_Hints;

        [BurnXmlChildElement()]
        public Hint[] HintsArray
        {
            get
            {
                return m_Hints.ToArray(); ;
            }
        }

        public List<Hint> Hints
        {
            get
            {
                if (m_Hints == null) m_Hints = new List<Hint>();
                return m_Hints;
            }
            set
            {
                m_Hints = value;
            }
        }

    }
}
