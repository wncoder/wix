//-------------------------------------------------------------------------------------------------
// <copyright file="DependencyCommon.cs" company="Microsoft">
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
// The Windows Installer XML toolset dependency extension common functionality.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Extensions
{
    using System;
    using Microsoft.Tools.WindowsInstallerXml;

    internal static class DependencyCommon
    {
        // Same values as for the Upgrade table in Windows Installer.
        internal const int RequiresAttributesMinVersionInclusive = 256;
        internal const int RequiresAttributesMaxVersionInclusive = 512;

        // Provider attributes.
        internal const int ProvidesAttributesVersionGuaranteeNone = 0;
        internal const int ProvidesAttributesVersionGuaranteeMajor = 1;
        internal const int ProvidesAttributesVersionGuaranteeMinor = 2;
        internal const int ProvidesAttributesVersionGuaranteeBuild = 3;
        internal const int ProvidesAttributesVersionGuaranteeMask =
            ProvidesAttributesVersionGuaranteeMajor |
            ProvidesAttributesVersionGuaranteeMinor |
            ProvidesAttributesVersionGuaranteeBuild;

        // The root registry key for the dependency extension. We write to Software\Classes explicitly
        // based on the current security context instead of HKCR. See
        // http://msdn.microsoft.com/en-us/library/ms724475(VS.85).aspx for more information.
        internal static readonly string RegistryRoot = @"Software\Classes\Installer\Dependencies\";
        internal static readonly string RegistryDependents = "Dependents";
    }
}
