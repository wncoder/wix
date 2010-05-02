//-----------------------------------------------------------------------
// <copyright file="XmlChildElement.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>property attribute to define xml attributes</summary>
//-----------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.ElementAttribute
{
    [System.AttributeUsage(System.AttributeTargets.Property)]
    public class BurnXmlChildElement : System.Attribute
    {
        public BurnXmlChildElement()
        {
        }       
    }
}
