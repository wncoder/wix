//-------------------------------------------------------------------------------------------------
// <copyright file="SetupSession.cs" company="Microsoft">
// Copyright (c) Microsoft Corporation. All rights reserved.
// </copyright>
// 
// <summary>
// Top-level container for business logic shared between managed user experience components.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Bootstrapper
{
    using System;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
    using System.ComponentModel;
    using System.Diagnostics;
    using System.Globalization;
    using System.IO;
    using System.Reflection;
    using System.Xml;

    /// <summary>
    /// Indicates the type of the progress phase
    /// </summary>
    public enum ProgressPhase
    {
        None,
        Download,
        Install,
        Uninstall,
        Repair,
        Change
    }

    // TODO: Re-factor into Model and Controller as in MVC pattern?
    // Need to be able to override factory creation method of the interop layer.

    /// <summary>
    /// The bootstrapper setup session for UX components.
    /// </summary>
    public class SetupSession : UserExperience, INotifyPropertyChanged
    {
        // =========================================================================================
        // Static Variables
        // =========================================================================================
        private static SetupSession instance;

        // =========================================================================================
        // Member Variables
        // =========================================================================================
        private LaunchAction userAction;
        private string productName;
        private string companyName;
        private string installPath;
        private string installPathMsiVariable;
        private int overallProgressPercentage;
        private Collection<Option> optionCollection;
        private Option currentlyExecutingOption;
        private ProgressPhase currentProgressPhase;
        private Dictionary<string, Package> packageLookup;

        // =========================================================================================
        // Constructors
        // =========================================================================================
        /// <summary>
        /// Initializes a new instance of the <see cref="SetupSession"/> class.
        /// </summary>
        public SetupSession()
        {
            this.packageLookup = new Dictionary<string, Package>();
        }

        // =========================================================================================
        // Events
        // =========================================================================================
        /// <summary>
        /// Raised when a property on this object has a new value.
        /// </summary>
        public event PropertyChangedEventHandler PropertyChanged;


        // =========================================================================================
        // Properties
        // =========================================================================================
        #region Properties

        /// <summary>
        /// Gets the singleton SetupSession instance
        /// </summary>
        public static SetupSession Instance
        {
            get { return SetupSession.instance; }
        }

        /// <summary>
        /// Gets or sets the current user action (install, uninstall, repair, etc.)
        /// </summary>
        public LaunchAction UserAction
        {
            get 
            { 
                return this.userAction; 
            }

            set
            {
                if (value != this.userAction)
                {
                    this.userAction = value;

                    // Update selection state of each package as it is dependent
                    // on the overall user action
                    foreach (Package package in this.packageLookup.Values)
                    {
                        package.UpdateSelection();
                    }

                    // Update selection state of each user option as it is dependent
                    // on the updated state of each package
                    foreach (Option option in this.OptionCollection)
                    {
                        option.UpdateSelection();
                    }

                    this.OnPropertyChanged("UserAction");
                }
            }
        }

        /// <summary>
        /// Gets the name of the product getting installed
        /// </summary>
        public string ProductName
        {
            get 
            { 
                return this.productName; 
            }

            private set
            {
                if (value != this.productName)
                {
                    this.productName = value;
                    this.OnPropertyChanged("ProductName");
                }
            }
        }

        /// <summary>
        /// Gets the name of the company that created the product getting installed
        /// </summary>
        public string CompanyName
        {
            get 
            { 
                return this.companyName; 
            }

            private set
            {
                if (value != this.companyName)
                {
                    this.companyName = value;
                    this.OnPropertyChanged("CompanyName");
                }
            }
        }

        /// <summary>
        /// Gets or sets the installation path
        /// </summary>
        public string InstallPath
        {
            get 
            { 
                return this.installPath; 
            }

            set
            {
                if (value != this.installPath)
                {
                    this.installPath = Environment.ExpandEnvironmentVariables(value);

                    if (null != this.installPathMsiVariable)
                    {
                        // TODO: Figure out why setting variables is not working.  
                        // Msi logs don't show any custom variables being set.
                        this.Engine.StringVariables[this.installPathMsiVariable] = this.installPath;
                    }

                    this.OnPropertyChanged("InstallPath");
                }
            }
        }

        /// <summary>
        /// Gets the overall setup progress percentage
        /// </summary>
        public int OverallProgressPercentage
        {
            get 
            { 
                return this.overallProgressPercentage; 
            }

            private set
            {
                if (value != this.overallProgressPercentage)
                {
                    this.overallProgressPercentage = value;
                    this.OnPropertyChanged("OverallProgressPercentage");
                }
            }
        }

        /// <summary>
        /// Gets a collection of user selectable options
        /// </summary>
        public Collection<Option> OptionCollection
        {
            get { return this.optionCollection; }
        }

        /// <summary>
        /// Gets currently executing user option
        /// </summary>
        public Option CurrentlyExecutingOption
        {
            get 
            { 
                return this.currentlyExecutingOption; 
            }

            private set
            {
                if (value != this.currentlyExecutingOption)
                {
                    this.currentlyExecutingOption = value;
                    this.OnPropertyChanged("CurrentlyExecutingOption");
                }
            }
        }

        /// <summary>
        /// Gets a current progress phase
        /// </summary>
        public ProgressPhase CurrentProgressPhase
        {
            get 
            { 
                return this.currentProgressPhase; 
            }

            private set
            {
                if (value != this.currentProgressPhase)
                {
                    this.currentProgressPhase = value;
                    this.OnPropertyChanged("CurrentProgressPhase");
                }
            }
        }

        #endregion


        // =========================================================================================
        // Public/Protected Methods
        // =========================================================================================
        #region Methods

        /// <summary>
        /// Determine if all installation conditions are fulfilled.
        /// </summary>
        public void Detect()
        {
            this.Engine.Detect();
        }

        /// <summary>
        /// Determine the installation sequencing and costing.
        /// </summary>
        public void Plan()
        {
            this.Engine.Plan(this.UserAction);
        }

        /// <summary>
        /// Apply the packages.
        /// </summary>
        /// <param name="hwndParent">The parent window for the installation user interface.</param>
        public void Apply(IntPtr hwndParent)
        {
            this.Engine.Apply(hwndParent);
        }

        /// <summary>
        /// Logs the <paramref name="message"/>.
        /// </summary>
        /// <param name="level">The logging level.</param>
        /// <param name="message">The message to log.</param>
        public void Log(LogLevel level, string message)
        {
            this.Engine.Log(level, message);
        }

        /// <summary>
        /// Called by the engine to initialize the user experience.
        /// </summary>
        /// <param name="show">Message box states to use for any user interface.</param>
        /// <param name="resume">Describes why the bundle installation was resumed.</param>
        protected override void Initialize(int show, ResumeType resume)
        {
            // Assume UX manifest is present in the same folder as the UX binaries
            string currentAssemblyFolder = Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location);
            string manifestPath = Path.Combine(currentAssemblyFolder, "UserExperienceManifest.xml");
            this.LoadManifest(manifestPath);

            SetupSession.instance = this;

            // Initialize user action to the one specified by command line arguments.
            this.userAction = Command.Action;
        }

        #endregion


        // =========================================================================================
        // IBurnUserExperience Notifications
        // =========================================================================================

        #region Notifications

        /// <summary>
        /// Called when the detection for a specific package has completed.
        /// Record detected package state.
        /// </summary>
        /// <param name="args">Additional arguments for this event.</param>
        protected override void OnDetectPackageComplete(DetectPackageCompleteEventArgs args)
        {
            // Assume Engine and UX manifests are in sync in terms of packages
            Debug.Assert(this.packageLookup.ContainsKey(args.PackageId), "Engine and UX manifests are not in sync");

            this.packageLookup[args.PackageId].DetectedState = args.State;
        }

        /// <summary>
        /// Called when the detection phase has completed.
        /// Update selection of all user options based on detected package state.
        /// </summary>
        /// <param name="args">Additional arguments for this event.</param>
        protected override void OnDetectComplete(DetectCompleteEventArgs args)
        {
            // Update user option selection based on detected state and overall user action
            foreach (Option option in this.OptionCollection)
            {
                option.UpdateSelection();
            }

            // Let base send out notification to other interested parties
            base.OnDetectComplete(args);
        }


        /// <summary>
        /// Called when the engine has begun planning the installation.
        /// Pass desired package state to the Engine.
        /// </summary>
        /// <param name="args">Additional arguments for this event.</param>
        protected override void OnPlanPackageBegin(PlanPackageBeginEventArgs args)
        {
            // Assume Engine and UX manifests are in sync in terms of packages
            Debug.Assert(this.packageLookup.ContainsKey(args.PackageId), "Engine and UX manifests are not in sync");

            // Tell engine what state to put this package into
            args.State = this.packageLookup[args.PackageId].DesiredState;
        }

        /// <summary>
        /// Called when the engine has changed progress for the bundle installation.
        /// </summary>
        /// <param name="args">Additional arguments for this event.</param>
        protected override void OnProgress(ProgressEventArgs args)
        {
            this.OverallProgressPercentage = args.OverallPercentage;
        }

        /// <summary>
        /// Called when the engine has begun installing packages.
        /// </summary>
        /// <param name="args">Additional arguments for this event.</param>
        protected override void OnExecuteBegin(ExecuteBeginEventArgs args)
        {
            switch (this.UserAction)
            {
                case LaunchAction.Install:
                    this.CurrentProgressPhase = ProgressPhase.Install;
                    break;
                case LaunchAction.Uninstall:
                    this.CurrentProgressPhase = ProgressPhase.Uninstall;
                    break;
                case LaunchAction.Repair:
                    this.CurrentProgressPhase = ProgressPhase.Repair;
                    break;
                case LaunchAction.Modify:
                    this.CurrentProgressPhase = ProgressPhase.Change;
                    break;
                default:
                    throw new InvalidOperationException("Unsupported UserAction");
            }

            // Let base send out notification to other interested parties
            base.OnExecuteBegin(args);
        }

        /// <summary>
        /// Called when the engine has begun installing or uninstalling a specific package.
        /// </summary>
        /// <param name="args">Additional arguments for this event.</param>
        protected override void OnExecutePackageBegin(ExecutePackageBeginEventArgs args)
        {
            this.NoteCurrentlyExecutingOption(args.PackageId);
        }

        /// <summary>
        /// Called when the engine has completed installing or uninstalling packages.
        /// </summary>
        /// <param name="args">Additional arguments for this event.</param>
        protected override void OnExecuteComplete(ExecuteCompleteEventArgs args)
        {
            this.CurrentProgressPhase = ProgressPhase.None;

            // Let base send out notification to other interested parties
            base.OnExecuteComplete(args);
        }
        #endregion


        // =========================================================================================
        // INotifyPropertyChanged Implementation
        // =========================================================================================
        #region INotifyPropertyChanged Members

        /// <summary>
        /// Raises this object's PropertyChanged event.
        /// </summary>
        /// <param name="propertyName">The property that has a new value.</param>
        protected void OnPropertyChanged(string propertyName)
        {
            this.VerifyPropertyName(propertyName);

            PropertyChangedEventHandler handler = this.PropertyChanged;
            if (null != handler)
            {
                var e = new PropertyChangedEventArgs(propertyName);
                handler(this, e);
            }
        }

        /// <summary>
        /// Warns the developer if this object does not have
        /// a public property with the specified name. This 
        /// method does not exist in a Release build.
        /// </summary>
        /// <param name="propertyName">Property name to verify</param>
        [Conditional("DEBUG")]
        [DebuggerStepThrough]
        protected void VerifyPropertyName(string propertyName)
        {
            // Verify that the property name matches a real,  
            // public, instance property on this object.
            if (null == TypeDescriptor.GetProperties(this)[propertyName])
            {
                Debug.Fail("Invalid property name: " + propertyName);
            }
        }

        #endregion // INotifyPropertyChanged Members


        // =========================================================================================
        // Private Methods
        // =========================================================================================
        #region Private Methods

        /// <summary>
        /// Loads UserExperienceManifest into an object model.
        /// </summary>
        /// <param name="manifestPath">Path of the manifest file to load</param>
        private void LoadManifest(string manifestPath)
        {
            // Failure to load or parse a manifest is an unexpected error
            // that user can't do anything about, so we'll bubble up
            // the original exception to the top for now so developers can see it.
            //
            // TODO: add some top-level catch to present unexpected errors to the user
            XmlDocument xmlDoc = new XmlDocument();
            xmlDoc.Load(manifestPath);

            XmlNamespaceManager namespaceManager = new XmlNamespaceManager(xmlDoc.NameTable);
            namespaceManager.AddNamespace("ux", "http://schemas.microsoft.com/Setup/2010/01/Burn/UserExperience");

            this.LoadSettings(xmlDoc, namespaceManager);
            this.LoadOptions(xmlDoc, namespaceManager);
        }

        /// <summary>
        /// Loads the settings portion of the UserExperienceManifest into an object model.
        /// </summary>
        /// <param name="xmlDoc">Manifest xml document</param>
        /// <param name="namespaceManager">Namespace manager for the manifest xml document</param>
        private void LoadSettings(XmlDocument xmlDoc, XmlNamespaceManager namespaceManager)
        {
            XmlNode settingsNode = xmlDoc.SelectSingleNode("//ux:Settings", namespaceManager);

            // Load required settings
            XmlNode productNameNode = settingsNode.SelectSingleNode("ux:ProductName", namespaceManager);
            this.ProductName = productNameNode.InnerText;


            // Load optional settings
            XmlNode companyNameNode = settingsNode.SelectSingleNode("ux:Company", namespaceManager);
            if (null != companyNameNode)
            {
                this.CompanyName = companyNameNode.InnerText;
            }

            XmlNode installPathNode = settingsNode.SelectSingleNode("ux:DefaultInstallPath", namespaceManager);
            if (null != installPathNode)
            {
                this.InstallPath = installPathNode.InnerText;
                this.installPathMsiVariable = installPathNode.Attributes["MsiVariable"].InnerText;
            }
        }

        /// <summary>
        /// Loads the user options portion of the UserExperienceManifest into an object model.
        /// </summary>
        /// <param name="xmlDoc">Manifest xml document</param>
        /// <param name="namespaceManager">Namespace manager for the manifest xml document</param>
        private void LoadOptions(XmlDocument xmlDoc, XmlNamespaceManager namespaceManager)
        {
            Dictionary<string, Option> optionCollectionLookup = new Dictionary<string, Option>();
            this.optionCollection = new Collection<Option>();

            XmlNodeList optionNodeList = xmlDoc.SelectNodes("//ux:Options/ux:Option", namespaceManager);

            foreach (XmlNode optionNode in optionNodeList)
            {
                // Get required data
                string id = optionNode.Attributes["Id"].InnerText;
                string name = optionNode.Attributes["Name"].InnerText;

                Option option = new Option(name);

                // Get packages
                XmlNodeList packageNodeList = optionNode.SelectNodes("ux:Packages/ux:Package", namespaceManager);

                foreach (XmlNode packageNode in packageNodeList)
                {
                    string packageId = packageNode.Attributes["Id"].InnerText;
                    Package package = new Package(packageId, option);

                    // Associate a package with a user option
                    option.PackageList.Add(package);

                    // Assume a package is unique and is only associated with a single user option
                    Debug.Assert(!this.packageLookup.ContainsKey(packageId), "Package id is not unique");
                    this.packageLookup.Add(packageId, package);
                }

                // Add option to a collection of options
                this.optionCollection.Add(option);

                // Add option to a lookup dictionary for lookups during dependency node parsing
                optionCollectionLookup.Add(id, option);
            }

            // Load option dependencies
            XmlNodeList dependencyNodeList = xmlDoc.SelectNodes("//ux:Options/ux:Option/ux:Dependencies/ux:Dependency", namespaceManager);

            foreach (XmlNode dependencyNode in dependencyNodeList)
            {
                string dependencyId = dependencyNode.Attributes["Id"].InnerText;
                string currentOptionId = dependencyNode.ParentNode.ParentNode.Attributes["Id"].InnerText;

                Option currentOption = optionCollectionLookup[currentOptionId];
                Option dependencyOption = optionCollectionLookup[dependencyId];
                currentOption.DependencyList.Add(dependencyOption);
                dependencyOption.DependsOnMeList.Add(currentOption);

                // TODO: Detect cycles, or do we trust the tools to create a manifest without cycles?
            }
        }

        /// <summary>
        /// Notes the currently executing user option.
        /// </summary>
        /// <param name="packageId">Package id of the currently executing package</param>
        private void NoteCurrentlyExecutingOption(string packageId)
        {
            // Assume Engine and UX manifests are in sync in terms of packages
            Debug.Assert(this.packageLookup.ContainsKey(packageId), "Engine and UX manifests are not in sync");

            // Lookup the currently executing user option based on the current package
            // for progress UI
            this.CurrentlyExecutingOption = this.packageLookup[packageId].ContainerOption;
        }

        #endregion
    }
}
