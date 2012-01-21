//-----------------------------------------------------------------------
// <copyright file="XmlElement.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>class attribute to define xml element</summary>
//-----------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ElementAttribute
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