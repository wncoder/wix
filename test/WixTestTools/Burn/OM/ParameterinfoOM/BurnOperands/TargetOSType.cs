//-----------------------------------------------------------------------
// <copyright file="TargetOSType.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>TargetOS element OM</summary>
//-----------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.ElementAttribute;

namespace Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnOperands
{
    /// <summary>
    /// returns OS type, either "Server" or "Client".
    /// </summary>
    [BurnXmlElement("TargetOSType")]
    public class TargetOSType : Operands
    {
    }
}
