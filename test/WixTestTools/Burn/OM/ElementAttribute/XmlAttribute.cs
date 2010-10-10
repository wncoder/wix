//-----------------------------------------------------------------------
// <copyright file="XmlAttribute.cs" company="Microsoft">
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
// <summary>property attribute to define xml attribute name and default value</summary>
//-----------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ElementAttribute
{
    [System.AttributeUsage(AttributeTargets.Property)]
    public class BurnXmlAttribute : System.Attribute
    {
        public string Name;
        public object DefaultValue;

        public BurnXmlAttribute(string name)
        {
            this.Name = name;
        }

        public BurnXmlAttribute(string name, object defaultValue)
        {
            this.Name = name;
            this.DefaultValue = defaultValue;
        }
    }
}