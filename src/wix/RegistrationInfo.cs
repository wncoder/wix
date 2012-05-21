//-------------------------------------------------------------------------------------------------
// <copyright file="RegistrationInfo.cs" company="Microsoft">
//    Copyright (c) Microsoft Corporation.  All rights reserved.
//    
//    The use and distribution terms for this software are covered by the
//    Common Public License 1.0 (http://opensource.org/licenses/cpl1.0.php)
//    which can be found in the file CPL.TXT at the root of this distribution.
//    By using this software in any fashion, you are agreeing to be bound by
//    the terms of this license.
//    
//    You must not remove this notice, or any other, from this software.
// </copyright>
// 
// <summary>
// Add/Remove Programs registration for the bundle.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml
{
    using System;

    /// <summary>
    /// Add/Remove Programs registration for the bundle.
    /// </summary>
    internal class RegistrationInfo
    {
        public string Name { get; set; }
        public string Publisher { get; set; }
        public string HelpLink { get; set; }
        public string HelpTelephone { get; set; }
        public string AboutUrl { get; set; }
        public string UpdateUrl { get; set; }
        public string ParentName { get; set; }
        public int DisableModify { get; set; }
        public bool DisableRemove { get; set; }
    }
}
