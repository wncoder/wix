//-----------------------------------------------------------------------
// <copyright file="MsiProductVersion.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>MsiProductVersion element OM</summary>
//-----------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.ElementAttribute;

namespace Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnOperands
{
    /// <summary>
    /// checks if current user is in the Administrators local group; can only be used with Exists expression
    /// </summary>
    [BurnXmlElement("IsAdministrator")]
    public class IsAdministrator : Operands
    {
    }
}
