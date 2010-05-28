//-------------------------------------------------------------------------------------------------
// <copyright file="Option.cs" company="Microsoft">
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
// User selectable installation unit.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Bootstrapper
{
    using System;
    using System.Collections.ObjectModel;
    using System.Diagnostics;

    /// <summary>
    /// Class representing a user selectable installation unit.
    /// </summary>
    public class Option : PropertyNotifyBase
    {
        // =========================================================================================
        // Member Variables
        // =========================================================================================
        private string name;
        private bool isSelected;
        private Collection<Option> dependencyList;
        private Collection<Option> dependsOnMeList;
        private Collection<Package> packageList;

        // =========================================================================================
        // Constructors
        // =========================================================================================
        /// <summary>
        /// Initializes a new instance of the <see cref="Option"/> class.
        /// </summary>
        /// <param name="name">The name of the user option</param>
        public Option(string name)
        {
            this.name = name;
            // A user option will always have at least one package
            this.packageList = new Collection<Package>();
        }

        // =========================================================================================
        // Properties
        // =========================================================================================
        /// <summary>
        /// Gets the name of the user option
        /// </summary>
        public string Name
        {
            get { return this.name; }
        }

        /// <summary>
        /// Gets or sets a value indicating whether this user option is selected
        /// </summary>
        public bool IsSelected
        {
            get 
            { 
                return this.isSelected; 
            }

            set
            {
                if (value != this.isSelected)
                {
                    this.isSelected = value;

                    // Notify the View about a change
                    this.OnPropertyChanged("IsSelected");

                    // Update selection state of each package
                    foreach (Package package in this.PackageList)
                    {
                        package.IsSelected = value;
                    }


                    // Update selection of dependencies or those that depend on me
                    if (true == value)
                    {
                        // This user option got selected. Select all dependencies too.
                        foreach (Option dependencyOption in this.DependencyList)
                        {
                            dependencyOption.IsSelected = true;
                        }
                    }
                    else
                    {
                        // This user option got unselected.  Unselect all options that depend on me.
                        foreach (Option dependsOnMeOption in this.DependsOnMeList)
                        {
                            dependsOnMeOption.IsSelected = false;
                        }
                    }
                }
            }
        }

        /// <summary>
        /// Gets a value indicating whether this user option is detected as installed on the current machine
        /// </summary>
        public bool IsDetectedAsInstalled
        {
            get
            {
                this.VerifyConsistentPackageDetectedState();
                return PackageState.Present == this.PackageList[0].DetectedState;
            }
        }

        // TODO: implement IsApplicable

        /// <summary>
        /// Gets a list of user options that this user option depends on
        /// </summary>
        public Collection<Option> DependencyList
        {
            get
            {
                // It is likely for a user option to not have any dependencies,
                // so creating this list on the fly to save memory
                if (null == this.dependencyList)
                {
                    this.dependencyList = new Collection<Option>();
                }

                return this.dependencyList;
            }
        }

        /// <summary>
        /// Gets a list of user options that depend on this user option
        /// </summary>
        public Collection<Option> DependsOnMeList
        {
            get
            {
                // It is likely for a user option to not have any dependents,
                // so creating this list on the fly to save memory
                if (null == this.dependsOnMeList)
                {
                    this.dependsOnMeList = new Collection<Option>();
                }

                return this.dependsOnMeList;
            }
        }

        /// <summary>
        /// Gets a list of packages associated with this user option
        /// </summary>
        internal Collection<Package> PackageList
        {
            get { return this.packageList; }
        }

        // =========================================================================================
        // Internal Methods
        // =========================================================================================

        /// <summary>
        /// Updates selection of this user option based on the selection state of its packages
        /// that results from package detection and overall user action
        /// </summary>
        internal void UpdateSelection()
        {
            this.VerifyConsistentPackageSelection();

            this.IsSelected = this.PackageList[0].IsSelected;
        }

        // TODO: talk to test team to see if it makes sense to move this code out into tests
        /// <summary>
        /// Verifies that all packages have the same selection
        /// </summary>
        [Conditional("DEBUG")]
        [DebuggerStepThrough]
        private void VerifyConsistentPackageSelection()
        {
            Debug.Assert(this.PackageList.Count >= 1, "Empty package list");
            bool isSelectedFirst = this.PackageList[0].IsSelected;

            foreach (Package package in this.PackageList)
            {
                // Package selection needs to be kept in sync
                Debug.Assert(
                    package.IsSelected == isSelectedFirst, 
                    "Selection of all packages belonging to the same user option must be the same");
            }
        }

        /// <summary>
        /// Verifies that all packages have the same detected state
        /// </summary>
        [Conditional("DEBUG")]
        [DebuggerStepThrough]
        private void VerifyConsistentPackageDetectedState()
        {
            Debug.Assert(this.PackageList.Count >= 1, "Empty package list");
            PackageState detectedStateFirst = this.PackageList[0].DetectedState; 

            foreach (Package package in this.PackageList)
            {
                Debug.Assert(
                    package.DetectedState == detectedStateFirst, 
                    "Detected state of all packages belonging to the same user option must be the same");
            }
        }

        /// <summary>
        /// Verifies that all packages have the same desired state
        /// </summary>
        [Conditional("DEBUG")]
        [DebuggerStepThrough]
        private void VerifyConsistentPackageDesiredState()
        {
            Debug.Assert(this.PackageList.Count >= 1, "Empty package list");
            RequestState desiredStateFirst = this.PackageList[0].DesiredState;

            foreach (Package package in this.PackageList)
            {
                Debug.Assert(
                    package.DesiredState == desiredStateFirst,
                    "Desired state of all packages belonging to the same user option must be the same");
            }
        }

    }
}
