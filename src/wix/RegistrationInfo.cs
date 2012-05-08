//-------------------------------------------------------------------------------------------------
// <copyright file="RegistrationInfo.cs" company="Microsoft">
//    Copyright (c) Microsoft Corporation.  All rights reserved.
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
