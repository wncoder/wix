//-------------------------------------------------------------------------------------------------
// <copyright file="oafolderitem.cs" company="Microsoft Corporation">
//   Copyright (c) 2004, Microsoft Corporation.
//   This software is released under Common Public License Version 1.0 (CPL).
//   The license and further copyright text can be found in the file LICENSE.TXT
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
//-------------------------------------------------------------------------------------------------

using System;
using Microsoft.VisualStudio.Shell.Interop;
using Microsoft.VisualStudio.Shell;
using System.Runtime.InteropServices;
using System.Collections.Generic;
using System.Collections;
using System.Diagnostics;
using System.IO;
using IServiceProvider = System.IServiceProvider;
using Microsoft.VisualStudio.OLE.Interop;
using EnvDTE;
using System.Diagnostics.CodeAnalysis;

namespace Microsoft.VisualStudio.Package.Automation
{
	/// <summary>
	/// Represents an automation object for a folder in a project
	/// </summary>
	[SuppressMessage("Microsoft.Interoperability", "CA1405:ComVisibleTypeBaseTypesShouldBeComVisible")]
	[ComVisible(true), CLSCompliant(false)]
	public class OAFolderItem : OAProjectItem<FolderNode>
	{
		#region ctors
		public OAFolderItem(OAProject project, FolderNode node)
			: base(project, node)
		{
		}

		#endregion

		#region overridden methods
		public override ProjectItems Collection
		{
			get
			{
				ProjectItems items = new OAProjectItems(this.Project, this.Node);
				return items;
			}
		}

		public override ProjectItems ProjectItems
		{
			get
			{
				return this.Collection;
			}
		}
		#endregion
	}
}
