//-------------------------------------------------------------------------------------------------
// <copyright file="WixProjectReferenceNode.cs" company="Microsoft">
//    Copyright (c) Microsoft Corporation.  All rights reserved.
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
// Contains the WixProjectReferenceNode class.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.VisualStudio
{
    using System;
    using System.Diagnostics.CodeAnalysis;
    using System.IO;
    using System.Runtime.InteropServices;
    using Microsoft.VisualStudio;
    using Microsoft.VisualStudio.Package;
    using Microsoft.VisualStudio.Shell;
    using Microsoft.VisualStudio.Shell.Interop;

    using OleConstants = Microsoft.VisualStudio.OLE.Interop.Constants;
    using VsCommands = Microsoft.VisualStudio.VSConstants.VSStd97CmdID;
    using VsCommands2K = Microsoft.VisualStudio.VSConstants.VSStd2KCmdID;
    using VsMenus = Microsoft.VisualStudio.Package.VsMenus;

    /// <summary>
    /// Represents a Wix project reference node.
    /// </summary>
    public class WixProjectReferenceNode : ProjectReferenceNode
    {
        // =========================================================================================
        // Constructors
        // =========================================================================================

        /// <summary>
        /// Initializes a new instance of the <see cref="WixProjectReferenceNode"/> class.
        /// </summary>
        /// <param name="root">The root <see cref="WixProjectNode"/> that contains this node.</param>
        /// <param name="element">The element that contains MSBuild properties.</param>
        public WixProjectReferenceNode(WixProjectNode root, ProjectElement element)
            : base(root, element)
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="WixProjectReferenceNode"/> class.
        /// </summary>
        /// <param name="root">The root <see cref="WixProjectNode"/> that contains this node.</param>
        /// <param name="referencedProjectName">The name of the referenced project.</param>
        /// <param name="projectPath">The path to the referenced project file.</param>
        /// <param name="projectReference">Project reference GUID.</param>
        public WixProjectReferenceNode(WixProjectNode root, string referencedProjectName, string projectPath, string projectReference)
            : base(root, referencedProjectName, projectPath, projectReference)
        {
        }

        /// <summary>
        /// Gets the filter used in the Add Reference dialog box.
        /// </summary>
        private static string AddReferenceDialogFilter
        {
            get { return WixStrings.AddReferenceDialogFilter.Replace("\\0", "\0"); }
        }

        /// <summary>
        /// Gets the title used in the Add Reference dialog box.
        /// </summary>
        private static string AddReferenceDialogTitle
        {
            get { return WixStrings.AddReferenceDialogTitle; }
        }

        // =========================================================================================
        // Methods
        // =========================================================================================

        /// <summary>
        /// Hides all of the tabs in the Add Reference dialog except for the browse tab, which will search for wixlibs.
        /// </summary>
        /// <param name="project">The project that will contain the reference.</param>
        /// <returns>If the method succeeds, it returns S_OK. If it fails, it returns an error code.</returns>
        internal static int AddProjectReference(WixProjectNode project)
        {
            CCITracing.TraceCall();

            Guid showOnlyThisTabGuid = Guid.Empty;
            Guid startOnThisTabGuid = VSConstants.GUID_BrowseFilePage;
            string helpTopic = "VS.AddReference";
            string machineName = String.Empty;
            string browseFilters = WixProjectReferenceNode.AddReferenceDialogFilter;
            string browseLocation = WixProjectReferenceNode.GetAddReferenceDialogInitialDirectory(project.WixPackage);

            // initialize the structure that we have to pass into the dialog call
            VSCOMPONENTSELECTORTABINIT[] tabInitializers = new VSCOMPONENTSELECTORTABINIT[2];

            // tab 1 is the Project References tab: passing VSHPROPID_ShowProjInSolutionPage will tell the Add Reference
            // dialog to call into our GetProperty to determine if we should show ourself in the dialog
            tabInitializers[0].dwSize = (uint)Marshal.SizeOf(typeof(VSCOMPONENTSELECTORTABINIT));
            tabInitializers[0].guidTab = VSConstants.GUID_SolutionPage;
            tabInitializers[0].varTabInitInfo = (int)__VSHPROPID.VSHPROPID_ShowProjInSolutionPage;

            // tab 2 is the Browse tab
            tabInitializers[1].dwSize = (uint)Marshal.SizeOf(typeof(VSCOMPONENTSELECTORTABINIT));
            tabInitializers[1].guidTab = VSConstants.GUID_BrowseFilePage;
            tabInitializers[1].varTabInitInfo = 0;

            // initialize the flags to control the dialog
            __VSCOMPSELFLAGS flags = __VSCOMPSELFLAGS.VSCOMSEL_HideCOMClassicTab |
                __VSCOMPSELFLAGS.VSCOMSEL_HideCOMPlusTab |
                __VSCOMPSELFLAGS.VSCOMSEL_IgnoreMachineName |
                __VSCOMPSELFLAGS.VSCOMSEL_MultiSelectMode;

            // get the dialog service from the environment
            IVsComponentSelectorDlg dialog = WixHelperMethods.GetService<IVsComponentSelectorDlg, SVsComponentSelectorDlg>(project.Site);

            try
            {
                // show the dialog
                ErrorHandler.ThrowOnFailure(dialog.ComponentSelectorDlg(
                    (uint)flags,
                    (IVsComponentUser)project,
                    WixProjectReferenceNode.AddReferenceDialogTitle,
                    helpTopic,
                    ref showOnlyThisTabGuid,
                    ref startOnThisTabGuid,
                    machineName,
                    (uint)tabInitializers.Length,
                    tabInitializers,
                    browseFilters,
                    ref browseLocation));
            }
            catch (COMException e)
            {
                CCITracing.Trace(e);
                return e.ErrorCode;
            }

            return VSConstants.S_OK;
        }

        /// <summary>
        /// Creates an object derived from <see cref="NodeProperties"/> that will be used to expose
        /// properties specific for this object to the property browser.
        /// </summary>
        /// <returns>A new <see cref="WixProjectReferenceNodeProperties"/> object.</returns>
        protected override NodeProperties CreatePropertiesObject()
        {
            return new WixProjectReferenceNodeProperties(this);
        }

        /// <summary>
        /// Handles command status on a node. Should be overridden by descendant nodes. If a command cannot be handled then the base should be called.
        /// </summary>
        /// <param name="cmdGroup">A unique identifier of the command group. The pguidCmdGroup parameter can be NULL to specify the standard group.</param>
        /// <param name="cmd">The command to query status for.</param>
        /// <param name="pCmdText">Pointer to an OLECMDTEXT structure in which to return the name and/or status information of a single command. Can be NULL to indicate that the caller does not require this information.</param>
        /// <param name="result">An out parameter specifying the QueryStatusResult of the command.</param>
        /// <returns>If the method succeeds, it returns S_OK. If it fails, it returns an error code.</returns>
        [SuppressMessage("Microsoft.Naming", "CA1725:ParameterNamesShouldMatchBaseDeclaration", MessageId = "0#", Justification = "Suppressing to avoid conflict with style cop.")]
        protected override int QueryStatusOnNode(Guid cmdGroup, uint cmd, IntPtr pCmdText, ref QueryStatusResult result)
        {
            WixProjectNode projectNode = this.ProjectMgr as WixProjectNode;
            if (projectNode != null && projectNode.QueryStatusOnProjectNode(cmdGroup, cmd, ref result))
            {
                return VSConstants.S_OK;
            }
            else if (cmdGroup == VsMenus.guidStandardCommandSet2K)
            {
                if ((VsCommands2K)cmd == VsCommands2K.QUICKOBJECTSEARCH)
                {
                    Guid browseGuid = this.GetBrowseGuid();
                    if (browseGuid != Guid.Empty)
                    {
                        result |= QueryStatusResult.SUPPORTED | QueryStatusResult.ENABLED;
                    }
                    else
                    {
                        result |= QueryStatusResult.NOTSUPPORTED;
                    }

                    return VSConstants.S_OK;
                }
            }

            return base.QueryStatusOnNode(cmdGroup, cmd, pCmdText, ref result);
        }

        /// <summary>
        /// Handles command execution.
        /// </summary>
        /// <param name="cmdGroup">Unique identifier of the command group</param>
        /// <param name="cmd">The command to be executed.</param>
        /// <param name="cmdexecopt">Values describe how the object should execute the command.</param>
        /// <param name="pvaIn">Pointer to a VARIANTARG structure containing input arguments. Can be NULL</param>
        /// <param name="pvaOut">VARIANTARG structure to receive command output. Can be NULL.</param>
        /// <returns>If the method succeeds, it returns S_OK. If it fails, it returns an error code.</returns>
        [SuppressMessage("Microsoft.Naming", "CA1725:ParameterNamesShouldMatchBaseDeclaration", MessageId = "0#", Justification = "Suppressing to avoid conflict with style cop.")]
        [SuppressMessage("Microsoft.Naming", "CA1725:ParameterNamesShouldMatchBaseDeclaration", MessageId = "2#", Justification = "Suppressing to avoid conflict with style cop.")]
        protected override int ExecCommandOnNode(Guid cmdGroup, uint cmd, uint cmdexecopt, IntPtr pvaIn, IntPtr pvaOut)
        {
            if (cmdGroup == VsMenus.guidStandardCommandSet97)
            {
                switch ((VsCommands)cmd)
                {
                    case VsCommands.Refresh:
                        WixHelperMethods.RefreshProject(this);
                        return VSConstants.S_OK;
                }
            }

            if (cmdGroup == VsMenus.guidStandardCommandSet2K)
            {
                switch ((VsCommands2K)cmd)
                {
                    case VsCommands2K.SLNREFRESH:
                        WixHelperMethods.RefreshProject(this);
                        return VSConstants.S_OK;
                }
            }

            return base.ExecCommandOnNode(cmdGroup, cmd, cmdexecopt, pvaIn, pvaOut);
        }

        /// <summary>
        /// Shows a referenced C#, VB, or VC project in the Object Browser.
        /// </summary>
        /// <returns>S_OK on success, else an error code.</returns>
        protected override int ShowObjectBrowser()
        {
            Guid browseGuid = this.GetBrowseGuid();

            if (browseGuid == Guid.Empty)
            {
                return (int)OleConstants.OLECMDERR_E_NOTSUPPORTED;
            }

            IntPtr guidPtr = IntPtr.Zero;
            try
            {
                guidPtr = Marshal.AllocCoTaskMem(Marshal.SizeOf(typeof(Guid)));
                Marshal.StructureToPtr(browseGuid, guidPtr, false);

                VSOBJECTINFO[] objInfo = new VSOBJECTINFO[1];
                objInfo[0].pszLibName = this.ReferencedProjectName;
                objInfo[0].pguidLib = guidPtr;

                IVsObjBrowser objBrowser = this.ProjectMgr.Site.GetService(typeof(SVsObjBrowser)) as IVsObjBrowser;
                ErrorHandler.ThrowOnFailure(objBrowser.NavigateTo(objInfo, 0));

                return VSConstants.S_OK;
            }
            catch (COMException e)
            {
                return e.ErrorCode;
            }
            finally
            {
                if (guidPtr != IntPtr.Zero)
                {
                    System.Runtime.InteropServices.Marshal.FreeCoTaskMem(guidPtr);
                }
            }
        }

        /// <summary>
        /// Gets the initial directory for the Add Reference dialog box.
        /// </summary>
        /// <param name="package">The package to retrieve the settings from.</param>
        /// <returns>Directory path for the Add Reference dialog box.</returns>
        private static string GetAddReferenceDialogInitialDirectory(WixPackage package)
        {
            // get the tools directory from the registry, which has the wixlibs that we ship with
            string toolsDirectory = package.Settings.ToolsDirectory;
            if (String.IsNullOrEmpty(toolsDirectory) || !Directory.Exists(toolsDirectory))
            {
                return Directory.GetCurrentDirectory();
            }

            return toolsDirectory;
        }

        /// <summary>
        /// Gets the guid for the browse library for the referenced project type.
        /// </summary>
        /// <returns>Object Browser library guid, or Guid.Empty if browsing is not supported for the project type.</returns>
        private Guid GetBrowseGuid()
        {
            Guid browseGuid = Guid.Empty;

            if (!String.IsNullOrEmpty(this.Url))
            {
                string projectExtension = Path.GetExtension(this.Url);
                if (String.Compare(projectExtension, ".csproj", StringComparison.OrdinalIgnoreCase) == 0)
                {
                    browseGuid = new Guid(BrowseLibraryGuids80.CSharp);
                }
                else if (String.Compare(projectExtension, ".vbproj", StringComparison.OrdinalIgnoreCase) == 0)
                {
                    browseGuid = new Guid(BrowseLibraryGuids80.VB);
                }
            }

            return browseGuid;
        }
    }
}
