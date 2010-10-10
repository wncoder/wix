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
    /// The model of the installation view in WixUX.
    /// </summary>
    public class InstallationViewModel : PropertyNotifyBase
    {
        private RootViewModel root;

        private Dictionary<string, int> downloadRetries;
        private bool canceled;
        private bool downgrade;
        private bool installed;
        private InstallationState state;

        private bool planAttempted;
        private LaunchAction plannedAction;
        private IntPtr hwnd;

        private ICommand licenseCommand;
        private ICommand cancelCommand;
        private ICommand installCommand;
        private ICommand repairCommand;
        private ICommand uninstallCommand;
        private ICommand tryAgainCommand;

        private string message;
        private int progress;
        private int cacheProgress;
        private int executeProgress;

        /// <summary>
        /// Creates a new model of the installation view.
        /// </summary>
        public InstallationViewModel(RootViewModel root)
        {
            this.root = root;
            this.downloadRetries = new Dictionary<string, int>();

            WixUX.Model.Bootstrapper.DetectBegin += this.DetectBegin;
            WixUX.Model.Bootstrapper.DetectRelatedBundle += this.DetectedRelatedBundle;
            WixUX.Model.Bootstrapper.DetectPackageComplete += this.DetectedPackage;
            WixUX.Model.Bootstrapper.DetectComplete += this.DetectComplete;
            WixUX.Model.Bootstrapper.PlanPackageBegin += this.PlanPackageBegin;
            WixUX.Model.Bootstrapper.PlanComplete += this.PlanComplete;
            WixUX.Model.Bootstrapper.CacheAcquireProgress += this.ApplyCacheProgress;
            WixUX.Model.Bootstrapper.ApplyBegin += ApplyBegin;
            WixUX.Model.Bootstrapper.ExecuteProgress += this.ApplyExecuteProgress;
            WixUX.Model.Bootstrapper.Progress += this.ApplyProgress;
            WixUX.Model.Bootstrapper.ExecuteMsiMessage += this.ExecuteMsiMessage;
            WixUX.Model.Bootstrapper.Error += this.ExecuteError;
            WixUX.Model.Bootstrapper.ResolveSource += this.ResolveSource;
            WixUX.Model.Bootstrapper.ApplyComplete += this.ApplyComplete;
        }

        public bool Canceled
        {
            get
            {
                return this.canceled;
            }

            set
            {
                if (this.canceled != value)
                {
                    this.canceled = value;
                    base.OnPropertyChanged("Canceled");
                }
            }
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

        public int Progress
        {
            get
            {
                return this.progress;
            }

            set
            {
                if (this.progress != value)
                {
                    this.progress = value;
                    base.OnPropertyChanged("Progress");
                }
            }
        }

        public int CacheProgress
        {
            get
            {
                return this.cacheProgress;
            }

            set
            {
                if (this.cacheProgress != value)
                {
                    this.cacheProgress = value;
                    base.OnPropertyChanged("CacheProgress");
                }
            }
        }

        public int ExecuteProgress
        {
            get
            {
                return this.executeProgress;
            }

            set
            {
                if (this.executeProgress != value)
                {
                    this.executeProgress = value;
                    base.OnPropertyChanged("ExecuteProgress");
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

        /// <summary>
        /// Gets and sets whether the view model considers WiX installed.
        /// </summary>
        public bool Installed
        {
            get
            {
                return this.installed;
            }

            set
            {
                if (this.installed != value)
                {
                    this.installed = value;
                    base.OnPropertyChanged("Installed");
                }
            }
        }

        public ICommand LicenseCommand
        {
            get
            {
                if (this.licenseCommand == null)
                {
                    this.licenseCommand = new RelayCommand(param => this.LaunchLicense(), param => this.state == InstallationState.DetectedAbsent);
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

        public ICommand CancelCommand
        {
            get
            {
                if (this.cancelCommand == null)
                {
                    this.cancelCommand = new RelayCommand(param =>
                        {
                            lock (this)
                            {
                                this.Canceled = (MessageBoxResult.Yes == MessageBox.Show(WixUX.View, "Are you sure you want to cancel?", "WiX Toolset", MessageBoxButton.YesNo, MessageBoxImage.Error));
                            }
                        });
                }

                return this.cancelCommand;
            }
        }

        public ICommand InstallCommand
        {
            get
            {
                if (this.installCommand == null)
                {
                    this.installCommand = new RelayCommand(param => this.Plan(LaunchAction.Install), param => this.state == InstallationState.DetectedAbsent);
                }

                return this.installCommand;
            }
        }

        public ICommand RepairCommand
        {
            get
            {
                if (this.repairCommand == null)
                {
                    this.repairCommand = new RelayCommand(param => this.Plan(LaunchAction.Repair), param => this.state == InstallationState.DetectedPresent);
                }

                return this.repairCommand;
            }
        }

        public ICommand UninstallCommand
        {
            get
            {
                if (this.uninstallCommand == null)
                {
                    this.uninstallCommand = new RelayCommand(param => this.Plan(LaunchAction.Uninstall), param => this.Installed);
                }

                return this.uninstallCommand;
            }
        }

        public ICommand TryAgainCommand
        {
            get
            {
                if (this.tryAgainCommand == null)
                {
                    this.tryAgainCommand = new RelayCommand(param => this.Plan(this.plannedAction), param => this.state == InstallationState.Failed);
                }

                return this.tryAgainCommand;
            }
        }

        /// <summary>
        /// Gets and sets the state of the view's model.
        /// </summary>
        public InstallationState State
        {
            get
            {
                return this.state;
            }

            set
            {
                if (this.state != value)
                {
                    this.state = value;
                    base.OnPropertyChanged("State");
                    base.OnPropertyChanged("Title"); // updating the state, updates the title.
                }
            }
        }

        public string Title
        {
            get
            {
                switch (this.state)
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
                        if (this.Canceled)
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

            this.Canceled = false;
            WixUX.Model.Engine.Detect();
        }

        /// <summary>
        /// Launches the license in the default viewer.
        /// </summary>
        private void LaunchLicense()
        {
            string folder = IO.Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location);
            WixUX.LaunchUrl(IO.Path.Combine(folder, "License.rtf"));
        }

        /// <summary>
        /// Starts planning the appropriate action.
        /// </summary>
        /// <param name="action">Action to plan.</param>
        private void Plan(LaunchAction action)
        {
            this.planAttempted = true;
            this.plannedAction = action;
            this.hwnd = (WixUX.View == null) ? IntPtr.Zero : new WindowInteropHelper(WixUX.View).Handle;

            this.Canceled = false;
            WixUX.Model.Engine.Plan(this.plannedAction);
        }

        private void DetectBegin(object sender, DetectBeginEventArgs e)
        {
            this.State = InstallationState.Initializing;
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
                this.Installed = (e.State == PackageState.Present);
            }
        }

        private void DetectComplete(object sender, DetectCompleteEventArgs e)
        {
            if (LaunchAction.Uninstall == WixUX.Model.Command.Action)
            {
                WixUX.View.Dispatcher.Invoke((Action)delegate()
                    {
                        this.Plan(LaunchAction.Uninstall);
                    }
                    );
            }
            else if (Hresult.Succeeded(e.Status))
            {
                if (this.Downgrade)
                {
                    this.State = InstallationState.DetectedNewer;
                }
                else if (this.Installed)
                {
                    this.State = InstallationState.DetectedPresent;
                }
                else
                {
                    this.State = InstallationState.DetectedAbsent;
                }
            }
            else
            {
                this.State = InstallationState.Failed;
            }

            // TODO: on /quiet start planning with the default action.
        }

        private void PlanPackageBegin(object sender, PlanPackageBeginEventArgs e)
        {
            if (e.PackageId.Equals(WixUX.Model.Engine.StringVariables["MbaNetfxPackageId"], StringComparison.Ordinal))
            {
                e.State = RequestState.None;
            }
        }

        private void PlanComplete(object sender, PlanCompleteEventArgs e)
        {
            if (Hresult.Succeeded(e.Status))
            {
                this.State = InstallationState.Applying;
                WixUX.Model.Engine.Apply(this.hwnd);
            }
            else
            {
                this.State = InstallationState.Failed;
            }
        }

        private void ApplyBegin(object sender, ApplyBeginEventArgs e)
        {
            this.downloadRetries.Clear();
        }

        private void ApplyProgress(object sender, ProgressEventArgs e)
        {
            lock (this)
            {
                this.Progress = e.OverallPercentage;
                e.Result = this.Canceled ? Result.Cancel : Result.Ok;
            }
        }

        private void ApplyCacheProgress(object sender, CacheAcquireProgressEventArgs e)
        {
            lock (this)
            {
                this.CacheProgress = e.OverallPercentage;
                e.Result = this.Canceled ? Result.Cancel : Result.Ok;
            }
        }

        private void ApplyExecuteProgress(object sender, ExecuteProgressEventArgs e)
        {
            lock (this)
            {
                this.ExecuteProgress = e.OverallPercentage;
                e.Result = this.Canceled ? Result.Cancel : Result.Ok;
            }
        }

        private void ExecuteError(object sender, ErrorEventArgs e)
        {
            lock (this)
            {
                if (!this.Canceled)
                {
                    this.Message = e.ErrorMessage;

                    WixUX.View.Dispatcher.Invoke((Action)delegate()
                        {
                            MessageBox.Show(WixUX.View, e.ErrorMessage, "WiX Toolset", MessageBoxButton.OK, MessageBoxImage.Error);
                        }
                        );
                }

                e.Result = this.Canceled ? Result.Cancel : Result.Ok;
            }
        }

        private void ExecuteMsiMessage(object sender, ExecuteMsiMessageEventArgs e)
        {
            lock (this)
            {
                this.Message = e.Message;
                e.Result = this.Canceled ? Result.Cancel : Result.Ok;

                System.Threading.Thread.Sleep(100); // TODO: remove this
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
            this.State = Hresult.Succeeded(e.Status) ? InstallationState.Applied : InstallationState.Failed;
        }
    }
}
