//-----------------------------------------------------------------------
// <copyright file="XmlAttribute.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
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