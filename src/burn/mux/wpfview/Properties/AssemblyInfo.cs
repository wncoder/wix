//-------------------------------------------------------------------------------------------------
// <copyright file="AssemblyInfo.cs" company="Microsoft">
// Copyright (c) Microsoft Corporation. All rights reserved.
//    
//    The use and distribution terms for this software are covered by the
//    Common Public License 1.0 (http://opensource.org/licenses/cpl.php)
//    which can be found in the file CPL.TXT at the root of this distribution.
//    By using this software in any fashion, you are agreeing to be bound by
//    the terms of this license.
//    
//    You must not remove this notice, or any other, from this software.
// </copyright>
// 
// <summary>
// Assembly information.
// </summary>
//-------------------------------------------------------------------------------------------------
using System;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Windows;
using Microsoft.Tools.WindowsInstallerXml.Bootstrapper;

// General Information about an assembly is controlled through the following 
// set of attributes. Change these attribute values to modify the information
// associated with an assembly.
[assembly: AssemblyTitle("wpfview")]
[assembly: AssemblyDescription("WPF Sample Installer User Experience")]

// Some interop stuff is not letting this assembly be CLS compliant
[assembly: CLSCompliant(false)]

// Setting ComVisible to false makes the types in this assembly not visible 
// to COM components.  If you need to access a type in this assembly from 
// COM, set the ComVisible attribute to true on that type.
[assembly: ComVisible(false)]

[assembly: ThemeInfo(
    ResourceDictionaryLocation.None, // where theme specific resource dictionaries are located
    // (used if a resource is not found in the page, 
    // or application resource dictionaries)
    ResourceDictionaryLocation.SourceAssembly // where the generic resource dictionary is located
    // (used if a resource is not found in the page, 
    // app, or any theme specific resource dictionaries)
)]

// Identifies the class that derives from UserExperience and is the UX class that gets
// instantiated by the interop layer
[assembly: UserExperience(typeof(SetupWizardSession))]