//-----------------------------------------------------------------------
// <copyright file="TargetOS.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>TargetOS operand OM</summary>
//-----------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.ElementAttribute;

namespace Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnOperands
{
    /// <summary>
    /// returns OS in the following format:  "#.#.#" the first number is the major version,
    /// the second is the minor version and the third is the SP number.  So for example, XPSP2 is "5.1.2".
    /// </summary>
    [BurnXmlElement("TargetOS")]
    public class TargetOS : Operands
    {
    }
}
