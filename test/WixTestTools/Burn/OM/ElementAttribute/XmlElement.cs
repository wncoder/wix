//-----------------------------------------------------------------------
// <copyright file="XmlElement.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
//    
//    The use and distribution terms for this software are covered by the
//    Common Public License 1.0 (http://opensource.org/licenses/cpl1.0.php)
//    which can be found in the file CPL.TXT at the root of this distribution.
//    By using this software in any fashion, you are agreeing to be bound by
//    the terms of this license.
//    
//    You must not remove this notice, or any other, from this software.
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