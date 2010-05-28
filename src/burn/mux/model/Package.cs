//-------------------------------------------------------------------------------------------------
// <copyright file="Package.cs" company="Microsoft">
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
// Class representing an installable package.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Bootstrapper
{
    using System;
    using System.Diagnostics;

    /// <summary>
    /// Class representing an installable package.
    /// </summary>
    internal class Package
    {
        // =========================================================================================
        // Member Variables
        // =========================================================================================
        private string id;
        private PackageState detectedState;
        private bool isSelected;
        private Option containerOption;

        // =========================================================================================
        // Constructors
        // =========================================================================================
        /// <summary>
        /// Initializes a new instance of the <see cref="Package"/> class.
        /// </summary>
        /// <param name="id">The package identification</param>
        /// <param name="containerOption">The user option containing this package</param>
        internal Package(string id, Option containerOption)
        {
            this.id = id;
            this.containerOption = containerOption;
        }

        // =========================================================================================
        // Properties
        // =========================================================================================
        /// <summary>
        /// Gets package identity
        /// </summary>
        internal string Id
        {
            get { return this.id; }
        }

        /// <summary>
        /// Gets or sets package state detected on the system
        /// </summary>
        internal PackageState DetectedState 
        {
            get 
            { 
                return this.detectedState; 
            }

            set
            {
                Debug.Assert(value != PackageState.Unknown, "Invalid detected state");

                if (value != this.detectedState)
                {
                    this.detectedState = value;
                    this.UpdateSelection();
                }
            }
        }

        /// <summary>
        /// Gets final desired state for a package
        /// </summary>
        internal RequestState DesiredState 
        { 
            get; 
            private set; 
        }

        /// <summary>
        /// Gets or sets a value indicating whether package is selected
        /// </summary>
        internal bool IsSelected
        {
            get 
            { 
                return this.isSelected; 
            }

            set
            {
                // Always update desired state as it is dependent on both user selection
                // and overall user action that may have changed
                this.isSelected = value;
                this.UpdateDesiredState();
            }
        }

        /// <summary>
        /// Gets user option that contains this package
        /// </summary>
        internal Option ContainerOption
        {
            get { return this.containerOption; }
        }

        // =========================================================================================
        // Internal Methods
        // =========================================================================================
        /// <summary>
        /// Updates the selection of this package based on the detected state and the overall user action.
        /// </summary>
        internal void UpdateSelection()
        {
            switch (SetupSession.Instance.UserAction)
            {
                case LaunchAction.Install:
                    // Select package regardless of whether it is present, absent or cached
                    this.IsSelected = true;
                    break;

                case LaunchAction.Modify:
                case LaunchAction.Repair:
                case LaunchAction.Uninstall:
                    // Show present package as selected and absent/cached as not selected
                    this.IsSelected = this.DetectedState == PackageState.Present;
                    break;

                default:
                    Debug.Fail("Unexpected user action: " + SetupSession.Instance.UserAction);
                    break;
            }
        }

        // =========================================================================================
        // Private Methods
        // =========================================================================================
        /// <summary>
        /// Updates desired state of this package based on the current package selection and 
        /// overall user action.
        /// </summary>
        private void UpdateDesiredState()
        {
            // We don't want to do anything by default
            this.DesiredState = RequestState.None;

            if (this.isSelected)
            {
                switch (SetupSession.Instance.UserAction)
                {
                    case LaunchAction.Install:
                    case LaunchAction.Modify:
                        this.DesiredState = RequestState.Present;
                        break;

                    case LaunchAction.Uninstall:
                        this.DesiredState = RequestState.Absent;
                        break;

                    case LaunchAction.Repair:
                        this.DesiredState = RequestState.Repair;
                        break;

                    default:
                        Debug.Fail("Unexpected user action: " + SetupSession.Instance.UserAction);
                        break;
                }
            }
            else if (LaunchAction.Modify == SetupSession.Instance.UserAction)
            {
                // If user unselects an option in modify stage, we need to uninstall it
                this.DesiredState = RequestState.Absent;
            }
        }
    }
}
