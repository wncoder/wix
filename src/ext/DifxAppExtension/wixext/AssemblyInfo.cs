﻿//-------------------------------------------------------------------------------------------------
// <copyright file="AssemblyInfo.cs" company="Microsoft Corporation">
//   Copyright (c) 2004, Microsoft Corporation.
//   This software is released under Common Public License Version 1.0 (CPL).
//   The license and further copyright text can be found in the file LICENSE.TXT
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
// 
// <summary>
// The assembly information for the Windows Installer XML Toolset Help Extension.
// </summary>
//-------------------------------------------------------------------------------------------------

using System;
using System.Reflection;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

using Microsoft.Tools.WindowsInstallerXml;
using Microsoft.Tools.WindowsInstallerXml.Extensions;

[assembly: AssemblyTitle("WiX Toolset DIFxApp Extension")]
[assembly: AssemblyDescription("Windows Installer XML Toolset Toolset Driver Install Frameworks for Applications Extension")]
[assembly: AssemblyCulture("")]
[assembly: CLSCompliant(true)]
[assembly: ComVisible(false)]
[assembly: AssemblyDefaultWixExtension(typeof(DifxAppExtension))]
