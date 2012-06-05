﻿//---------------------------------------------------------------------
// <copyright file="AssemblyInfo.cs" company="Microsoft Corporation">
//   Copyright (c) 2004, Microsoft Corporation.
//   This software is released under Common Public License Version 1.0 (CPL).
//   The license and further copyright text can be found in the file LICENSE.TXT
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
// <summary>
// Part of the Deployment Tools Foundation project.
// </summary>
//---------------------------------------------------------------------

using System;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Security;
using System.Security.Permissions;

[assembly: AssemblyDescription("Abstract base libraries for archive packing and unpacking")]
[assembly: CLSCompliant(true)]
[assembly: ComVisible(false)]

[assembly: SecurityPermission(SecurityAction.RequestMinimum, Unrestricted = true)]

// SECURITY: Review carefully!
// This assembly is designed so that partially trusted callers should be able to
// do compression and extraction in a file path where they have limited
// file I/O permission. Or they can even do in-memory compression and extraction
// with absolutely no file I/O permission.
[assembly: AllowPartiallyTrustedCallers]
