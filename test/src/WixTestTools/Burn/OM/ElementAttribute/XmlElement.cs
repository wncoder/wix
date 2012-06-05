//-----------------------------------------------------------------------
// <copyright file="XmlElement.cs" company="Microsoft Corporation">
//   Copyright (c) 2004, Microsoft Corporation.
//   This software is released under Common Public License Version 1.0 (CPL).
//   The license and further copyright text can be found in the file LICENSE.TXT
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
// <summary>class attribute to define xml element</summary>
//-----------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace WixTest.Burn.OM.ElementAttribute
{
    [System.AttributeUsage(System.AttributeTargets.Class)]
    public class BurnXmlElement : System.Attribute
    {
        public string Name;
        public string NamespacePrefix;

        public BurnXmlElement(string name)
	   : this(name, string.Empty)
        {
        }

        public BurnXmlElement(string name, string namespacePrefix)
        {
            this.Name = name;
            this.NamespacePrefix = namespacePrefix;
        }
    }
}