//-----------------------------------------------------------------------
// <copyright file="IsInOSCompatibilityMode.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>IsInOSCompatibilityMode element OM</summary>
//-----------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.ElementAttribute;

namespace Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnOperands
{
    /// <summary>
    /// checks if current process is running in Compability Mode; can only be used with Exists expression
    /// </summary>
    [BurnXmlElement("IsInOSCompatibilityMode")]
    public class IsInOSCompatibilityMode : Operands
    {
    }
}
