//-----------------------------------------------------------------------
// <copyright file="TargetOS.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
//    
//    The use and distribution terms for this software are covered by the
//    Common Public License 1.0 (http://opensource.org/licenses/cpl.php)
//    which can be found in the file CPL.TXT at the root of this distribution.
//    By using this software in any fashion, you are agreeing to be bound by
//    the terms of this license.
//    
//    You must not remove this notice, or any other, from this software.
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
