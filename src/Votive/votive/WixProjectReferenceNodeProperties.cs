//-------------------------------------------------------------------------------------------------
// <copyright file="WixProjectReferenceNodeProperties.cs" company="Microsoft">
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
// Contains the WixProjectReferenceNodeProperties class.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.VisualStudio
{
    using System;
    using System.ComponentModel;
    using System.Diagnostics;
    using System.Diagnostics.CodeAnalysis;
    using System.Runtime.InteropServices;
    using Microsoft.VisualStudio.Package;

    /// <summary>
    /// Represents WiX project reference node properties.
    /// </summary>
    /// <remarks>This class must be public and marked as ComVisible in order for the DispatchWrapper to work correctly.</remarks>
    [CLSCompliant(false)]
    [ComVisible(true)]
    [Guid("E1E2DC4B-8113-44CE-BD45-5375B9B66B04")]
    [SuppressMessage("Microsoft.Interoperability", "CA1409:ComVisibleTypesShouldBeCreatable")]
    public class WixProjectReferenceNodeProperties : WixReferenceNodeProperties
    {
        // =========================================================================================
        // Constructors
        // =========================================================================================

        /// <summary>
        /// Initializes a new instance of the <see cref="WixProjectReferenceNodeProperties"/> class.
        /// </summary>
        /// <param name="node">The node that contains the properties to expose via the Property Browser.</param>
        public WixProjectReferenceNodeProperties(WixProjectReferenceNode node)
            : base(node)
        {
        }

        // =========================================================================================
        // Properties
        // =========================================================================================

        /// <summary>
        /// Gets the full path to the referenced project file.
        /// </summary>
        /// <value>Full path to the referenced project file.</value>
        public override string FullPath
        {
            get
            {
                return ((ProjectReferenceNode)this.Node).ReferencedProjectOutputPath;
            }
        }

        // =========================================================================================
        // Methods
        // =========================================================================================

        /// <summary>
        /// Returns the name that is displayed in the right hand side of the Properties window drop-down combo box.
        /// </summary>
        /// <returns>The class name of the object, or null if the class does not have a name.</returns>
        public override string GetClassName()
        {
            return WixStrings.WixProjectReferenceProperties;
        }
    }
}
