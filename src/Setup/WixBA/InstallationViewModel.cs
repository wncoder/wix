//-------------------------------------------------------------------------------------------------
// <copyright file="InstallationViewModel.cs" company="Microsoft">
// Copyright (c) Microsoft Corporation. All rights reserved.
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
// The model of the installation view.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.UX
{
    using System;
    using System.Collections.Generic;
    using System.ComponentModel;
    using IO = System.IO;
    using System.Reflection;
    using System.Windows;
    using System.Windows.Input;
    using System.Windows.Interop;
    using Microsoft.Tools.WindowsInstallerXml.Bootstrapper;

    /// <summary>
    /// The states of the installation view model.
    /// </summary>
    public enum InstallationState
    {
        Initializing,
        DetectedAbsent,
        DetectedPresent,
        DetectedNewer,
        Applying,
        Applied,
        Failed,
    }

    /// <summary>
    /// The model of the installation view in WixBA.
    /// </summary>
    public class InstallationViewModel : PropertyNotifyBase
    {
        private RootViewModel root;

        private Dictionary<string, int> downloadRetries;
        private bool downgrade;
        private readonly string wixHomePageUrl = "http://wixtoolset.org/";
        private readonly string wixNewsUrl = "http://wixtoolset.org/news/";

        private bool planAttempted;
        private LaunchAction plannedAction;
        private IntPtr hwnd;

        private ICommand licenseCommand;
        private ICommand launchHomePageCommand;
        private ICommand launchNewsCommand;
        private ICommand installCommand;
        private ICommand repairCommand;
        private ICommand uninstallCommand;
        private ICommand tryAgainCommand;

        private string message;

        /// <summary>
        /// Creates a new model of the installation view.
        /// </summary>
        public InstallationViewModel(RootViewModel root)
        {
            this.root = root;
            this.downloadRetries = new Dictionary<string, int>();

            this.root.PropertyChanged += new System.ComponentModel.PropertyChangedEventHandler(this.RootPropertyChanged);

            WixBA.Model.Bootstrapper.DetectBegin += this.DetectBegin;
            WixBA.Model.Bootstrapper.DetectRelatedBundle += this.DetectedRelatedBundle;
            WixBA.Model.Bootstrapper.DetectPackageComplete += this.DetectedPackage;
            WixBA.Model.Bootstrapper.DetectComplete += this.DetectComplete;
            WixBA.Model.Bootstrapper.PlanPackageBegin += this.PlanPackageBegin;
            WixBA.Model.Bootstrapper.PlanComplete += this.PlanComplete;
            WixBA.Model.Bootstrapper.ApplyBegin += this.ApplyBegin;
            WixBA.Model.Bootstrapper.Error += this.ExecuteError;
            WixBA.Model.Bootstrapper.ResolveSource += this.ResolveSource;
            WixBA.Model.Bootstrapper.ApplyComplete += this.ApplyComplete;
        }

        void RootPropertyChanged(object sender, PropertyChangedEventArgs e)
        {
            if ("State" == e.PropertyName)
            {
                base.OnPropertyChanged("Title");
                base.OnPropertyChanged("CompleteEnabled");
                base.OnPropertyChanged("ExitEnabled");
                base.OnPropertyChanged("RepairEnabled");
                base.OnPropertyChanged("InstallEnabled");
                base.OnPropertyChanged("UninstallEnabled");
            }
        }

        /// <summary>
        /// Gets the title for the application.
        /// </summary>
        public string Version 
        {
            get { return String.Concat("v", WixBA.Model.Version.ToString()); }
        }

        public string Message
        {
            get
            {
                return this.message;
            }

            set
            {
                if (this.message != value)
                {
                    this.message = value;
                    base.OnPropertyChanged("Message");
                }
            }
        }

        /// <summary>
        /// Gets and sets whether the view model considers this install to be a downgrade.
        /// </summary>
        public bool Downgrade
        {
            get
            {
                return this.downgrade;
            }

            set
            {
                if (this.downgrade != value)
                {
                    this.downgrade = value;
                    base.OnPropertyChanged("Downgrade");
                }
            }
        }

        public ICommand LaunchHomePageCommand
        {
            get
            {
                if (this.launchHomePageCommand == null)
                {
                    this.launchHomePageCommand = new RelayCommand(param => WixBA.LaunchUrl(this.wixHomePageUrl), param => true);
                }

                return this.launchHomePageCommand;
            }
        }

        public ICommand LaunchNewsCommand
        {
            get
            {
                if (this.launchNewsCommand == null)
                {
                    this.launchNewsCommand = new RelayCommand(param => WixBA.LaunchUrl(this.wixNewsUrl), param => true);
                }

                return this.launchNewsCommand;
            }
        }

        public ICommand LicenseCommand
        {
            get
            {
                if (this.licenseCommand == null)
                {
                    this.licenseCommand = new RelayCommand(param => this.LaunchLicense(), param => true);
                }

                return this.licenseCommand;
            }
        }

        public bool LicenseEnabled
        {
            get { return this.LicenseCommand.CanExecute(this); }
        }

        public ICommand CloseCommand
        {
            get { return this.root.CloseCommand; }
        }

        public bool ExitEnabled
        {
            get { return this.root.State != InstallationState.Applying; }
        }

        public ICommand InstallCommand
        {
            get
            {
                if (this.installCommand == null)
                {
                    this.installCommand = new RelayCommand(param => this.Plan(LaunchAction.Install), param => this.root.State == InstallationState.DetectedAbsent);
                }

                return this.installCommand;
            }
        }

        public bool InstallEnabled
        {
            get { return this.InstallCommand.CanExecute(this); }
        }

        public ICommand RepairCommand
        {
            get
            {
                if (this.repairCommand == null)
                {
                    this.repairCommand = new RelayCommand(param => this.Plan(LaunchAction.Repair), param => this.root.State == InstallationState.DetectedPresent);
                }

                return this.repairCommand;
            }
        }

        public bool RepairEnabled
        {
            get { return this.RepairCommand.CanExecute(this); }
        }

        public bool CompleteEnabled
        {
            get { return this.root.State == InstallationState.Applied; }
        }

        public ICommand UninstallCommand
        {
            get
            {
                if (this.uninstallCommand == null)
                {
                    this.uninstallCommand = new RelayCommand(param => this.Plan(LaunchAction.Uninstall), param => this.root.State == InstallationState.DetectedPresent);
                }

                return this.uninstallCommand;
            }
        }

        public bool UninstallEnabled
        {
            get { return this.UninstallCommand.CanExecute(this); }
        }

        public ICommand TryAgainCommand
        {
            get
            {
                if (this.tryAgainCommand == null)
                {
                    this.tryAgainCommand = new RelayCommand(param => this.Plan(this.plannedAction), param => this.root.State == InstallationState.Failed);
                }

                return this.tryAgainCommand;
            }
        }

        public bool TryAgainEnabled
        {
            get { return this.TryAgainCommand.CanExecute(this); }
        }

        public string Title
        {
            get
            {
                switch (this.root.State)
                {
                    case InstallationState.Initializing:
                        return "Initializing...";

                    case InstallationState.DetectedPresent:
                        return "Installed";

                    case InstallationState.DetectedNewer:
                        return "Newer version installed";

                    case InstallationState.DetectedAbsent:
                        return "Not installed";

                    case InstallationState.Applying:
                        switch (this.plannedAction)
                        {
                            case LaunchAction.Install:
                                return "Installing...";

                            case LaunchAction.Repair:
                                return "Repairing...";

                            case LaunchAction.Uninstall:
                                return "Uninstalling...";

                            default:
                                return "Unexpected action state";
                        }

                    case InstallationState.Applied:
                        switch (this.plannedAction)
                        {
                            case LaunchAction.Install:
                                return "Successfully installed";

                            case LaunchAction.Repair:
                                return "Successfully repaired";

                            case LaunchAction.Uninstall:
                                return "Successfully uninstalled";

                            default:
                                return "Unexpected action state";
                        }

                    case InstallationState.Failed:
                        if (this.root.Canceled)
                        {
                            return "Canceled";
                        }
                        else if (this.planAttempted)
                        {
                            switch (this.plannedAction)
                            {
                                case LaunchAction.Install:
                                    return "Failed to install";

                                case LaunchAction.Repair:
                                    return "Failed to repair";

                                case LaunchAction.Uninstall:
                                    return "Failed to uninstall";

                                default:
                                    return "Unexpected action state";
                            }
                        }
                        else
                        {
                            return "Unexpected failure";
                        }

                    default:
                        return "Unknown view model state";
                }
            }
        }

        /// <summary>
        /// Causes the installation view to re-detect machine state.
        /// </summary>
        public void Refresh()
        {
            // TODO: verify that the engine is in a state that will allow it to do Detect().

            this.root.Canceled = false;
            WixBA.Model.Engine.Detect();
        }

        /// <summary>
        /// Launches the license in the default viewer.
        /// </summary>
        private void LaunchLicense()
        {
            string folder = IO.Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location);
            WixBA.LaunchUrl(IO.Path.Combine(folder, "License.htm"));
        }

        /// <summary>
        /// Starts planning the appropriate action.
        /// </summary>
        /// <param name="action">Action to plan.</param>
        private void Plan(LaunchAction action)
        {
            this.planAttempted = true;
            this.plannedAction = action;
            this.hwnd = (WixBA.View == null) ? IntPtr.Zero : new WindowInteropHelper(WixBA.View).Handle;

            this.root.Canceled = false;
            WixBA.Model.Engine.Plan(this.plannedAction);
        }

        private void DetectBegin(object sender, DetectBeginEventArgs e)
        {
            this.root.State = InstallationState.Initializing;
            this.planAttempted = false;
        }

        private void DetectedRelatedBundle(object sender, DetectRelatedBundleEventArgs e)
        {
            if (e.Operation == RelatedOperation.Downgrade)
            {
                this.Downgrade = true;
            }
        }

        private void DetectedPackage(object sender, DetectPackageCompleteEventArgs e)
        {
            if (e.PackageId.Equals("Wix", StringComparison.Ordinal))
            {

                this.root.State = (e.State == PackageState.Present) ? InstallationState.DetectedPresent : InstallationState.DetectedAbsent;
            }
        }

        private void DetectComplete(object sender, DetectCompleteEventArgs e)
        {
            if (LaunchAction.Uninstall == WixBA.Model.Command.Action)
            {
                WixBA.Model.Engine.Log(LogLevel.Verbose, "Invoking automatic plan for uninstall");
                WixBA.Dispatcher.Invoke((Action)delegate()
                {
                    this.Plan(LaunchAction.Uninstall);
                }
                );
            }
            else if (Hresult.Succeeded(e.Status))
            {
                if (this.Downgrade)
                {
                    // TODO: What behavior do we want for downgrade?
                    this.root.State = InstallationState.DetectedNewer;
                }

                // If we're not waiting for the user to click install, dispatch plan with the default action.
                if (WixBA.Model.Command.Display != Display.Full)
                {
                    WixBA.Model.Engine.Log(LogLevel.Verbose, "Invoking automatic plan for non-interactive mode.");
                    WixBA.Dispatcher.Invoke((Action)delegate()
                    {
                        this.Plan(WixBA.Model.Command.Action);
                    }
                    );
                }
            }
            else
            {
                this.root.State = InstallationState.Failed;
            }
        }

        private void PlanPackageBegin(object sender, PlanPackageBeginEventArgs e)
        {
            if (e.PackageId.Equals(WixBA.Model.Engine.StringVariables["MbaNetfxPackageId"], StringComparison.Ordinal))
            {
                e.State = RequestState.None;
            }
        }

        private void PlanComplete(object sender, PlanCompleteEventArgs e)
        {
            if (Hresult.Succeeded(e.Status))
            {
                this.root.PreApplyState = this.root.State;
                this.root.State = InstallationState.Applying;
                WixBA.Model.Engine.Apply(this.hwnd);
            }
            else
            {
                this.root.State = InstallationState.Failed;
            }
        }

        private void ApplyBegin(object sender, ApplyBeginEventArgs e)
        {
            this.downloadRetries.Clear();
        }

        private void ExecuteError(object sender, ErrorEventArgs e)
        {
            lock (this)
            {
                if (!this.root.Canceled)
                {
                    // If the error is a cancel coming from the engine during apply we want to go back to the preapply state.
                    if (InstallationState.Applying == this.root.State && (int)Error.UserCancelled == e.ErrorCode)
                    {
                        this.root.State = this.root.PreApplyState;
                    }
                    else
                    {
                        this.Message = e.ErrorMessage;

                        WixBA.View.Dispatcher.Invoke((Action)delegate()
                        {
                            MessageBox.Show(WixBA.View, e.ErrorMessage, "WiX Toolset", MessageBoxButton.OK, MessageBoxImage.Error);
                        }
                            );
                    }
                }

                e.Result = this.root.Canceled ? Result.Cancel : Result.Ok;
            }
        }

        private void ResolveSource(object sender, ResolveSourceEventArgs e)
        {
            int retries = 0;

            this.downloadRetries.TryGetValue(e.PackageOrContainerId, out retries);
            this.downloadRetries[e.PackageOrContainerId] = retries + 1;

            e.Result = retries < 3 && !String.IsNullOrEmpty(e.DownloadSource) ? Result.Download : Result.Ok;
        }

        private void ApplyComplete(object sender, ApplyCompleteEventArgs e)
        {
            // If we're not in Full UI mode, we need to alert the dispatcher to stop and close the window for passive.
            if (Bootstrapper.Display.Full != WixBA.Model.Command.Display)
            {
                // If its passive, send a message to the window to close.
                if (Bootstrapper.Display.Passive == WixBA.Model.Command.Display)
                {
                    WixBA.Model.Engine.Log(LogLevel.Verbose, "Automatically closing the window for non-interactive install");
                    WixBA.Dispatcher.BeginInvoke((Action)delegate()
                    {
                        WixBA.View.Close();
                    }
                    );
                }
                else
                {
                    WixBA.Dispatcher.InvokeShutdown();
                }
            }

            // Set the state to applied or failed unless the state has already been set back to the preapply state
            // which means we need to show the UI as it was before the apply started.
            if (this.root.State != this.root.PreApplyState)
            {
                this.root.State = Hresult.Succeeded(e.Status) ? InstallationState.Applied : InstallationState.Failed;
            }
        }
    }
}
