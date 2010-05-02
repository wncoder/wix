//-----------------------------------------------------------------------
// <copyright file="TargetArchitecture.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>TargetArchitecture element OM</summary>
//-----------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.ElementAttribute;

namespace Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnOperands
{
    /// <summary>
    /// returns chip architecture, one of the following:  x86, x64 or ia64.
    /// </summary>
    [BurnXmlElement("TargetArchitecture")]
    public class TargetArchitecture : Operands
    {
    }
}
