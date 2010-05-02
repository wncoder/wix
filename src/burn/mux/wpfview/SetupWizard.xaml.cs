//-------------------------------------------------------------------------------------------------
// <copyright file="SetupWizard.xaml.cs" company="Microsoft">
// Copyright (c) Microsoft Corporation. All rights reserved.
// </copyright>
// 
// <summary>
// Setup wizard for the current setup session.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Bootstrapper
{
    using System;
    using System.Runtime.InteropServices;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Input;
    using System.Windows.Interop;
    using System.Windows.Threading;

    /// <summary>
    /// TODO: Create a custom Wizard control with configurable pages instead
    /// </summary>
    public partial class SetupWizard : Window
    {
        // =========================================================================================
        // Statics
        // =========================================================================================
        private static SetupWizard instance;
        private static RoutedCommand cancelCommand = new RoutedCommand();

        // =========================================================================================
        // Member Variables
        // =========================================================================================
        private WelcomePage welcomePage;
        private EulaPage eulaPage;
        private OptionSelectionPage optionSelectionPage;
        private SettingsSelectionPage settingsSelectionPage;
        private ProgressPage progressPage;
        private FinishPage finishPage;
        private MaintenancePage maintenancePage;

        private PageBase currentPage;
        private SetupSession setupSession;
        private int completionStatus;

        // =========================================================================================
        // Constructors
        // =========================================================================================
        /// <summary>
        /// Initializes a new instance of the <see cref="SetupWizard"/> class.
        /// </summary>
        /// <param name="setupSession">The setup session to display in this wizard.</param>
        public SetupWizard(SetupSession setupSession)
        {
            // We are guaranteed to have only one SetupWizard, so initializing the static instance to the
            // first instance
            instance = this;
            InitializeComponent();

            this.setupSession = setupSession;

            this.setupSession.DetectComplete += new EventHandler<DetectCompleteEventArgs>(this.OnDetectComplete);
            this.setupSession.ApplyComplete += new EventHandler<ApplyCompleteEventArgs>(this.OnApplyComplete);

            this.welcomePage = new WelcomePage();

            // Go to the first page
            this.GoToPage(this.welcomePage);

            // Start detecting packages
            this.Detect();
        }

        // =========================================================================================
        // Delegates
        // =========================================================================================
        /// <summary>
        /// Delegate to handle detect phase completion notification
        /// </summary>
        /// <param name="sender">Event sender</param>
        /// <param name="e">Event arguments</param>
        private delegate void OnDetectCompleteCallback(object sender, DetectCompleteEventArgs e);

        /// <summary>
        /// Delegate to handle apply phase completion notification
        /// </summary>
        /// <param name="sender">Event sender</param>
        /// <param name="e">Event arguments</param>
        private delegate void OnApplyCompleteCallback(object sender, ApplyCompleteEventArgs e);

        // =========================================================================================
        // Properties
        // =========================================================================================
        /// <summary>
        /// Gets the singleton SetupWizard instance.
        /// </summary>
        public static SetupWizard Instance
        {
            get { return SetupWizard.instance; }
        }

        /// <summary>
        /// Gets command to cancel the setup.
        /// This command is not built-in like the others.
        /// </summary>
        public static RoutedCommand Cancel
        {
            get { return cancelCommand; }
        }

        /// <summary>
        /// Gets or sets the current user action.
        /// </summary>
        public LaunchAction UserAction
        {
            get { return this.setupSession.UserAction; }
            set { this.setupSession.UserAction = value; }
        }

        /// <summary>
        /// Gets the setup completion status.
        /// </summary>
        public int CompletionStatus
        {
            get { return this.completionStatus; }
        }

        // =========================================================================================
        // Public methods
        // =========================================================================================
        /// <summary>
        /// Starts the detection setup phase.
        /// </summary>
        public void Detect()
        {
            this.setupSession.Detect();
        }

        /// <summary>
        /// Starts the planning setup phase.
        /// </summary>
        public void Plan()
        {
            this.setupSession.Plan();
        }

        /// <summary>
        /// Starts the downloading and execution setup phase.
        /// </summary>
        public void Apply()
        {
            WindowInteropHelper helper = new WindowInteropHelper(this);
            this.setupSession.Apply(helper.Handle);
        }

        /// <summary>
        /// Moves the setup wizard to the next page.
        /// </summary>
        public void GoToNextPage()
        {
           this.GoToPage(this.currentPage.NextPage);
        }

        /// <summary>
        /// Moves the setup wizard to the previous page.
        /// </summary>
        public void GoToPreviousPage()
        {
            this.GoToPage(this.currentPage.PreviousPage);
        }

        // =========================================================================================
        // Private methods
        // =========================================================================================
        /// <summary>
        /// Moves the setup wizard to a given page.
        /// </summary>
        /// <param name="page">Page to move to.</param>
        private void GoToPage(PageBase page)
        {
            if (null != this.currentPage)
            {
                this.currentPage.Deactivate();
            }

            this.setupSession.Log(LogLevel.Verbose, "Loading " + page.ToString() + " page");

            // TODO: switch to structured navigation instead
            this.gridMain.Children.Clear();
            this.gridMain.Children.Add(page);

            this.currentPage = page;
            this.currentPage.Activate();
        }

        /// <summary>
        /// Handles the completion of the detection phase
        /// </summary>
        /// <param name="sender">Event sender</param>
        /// <param name="e">Event arguments</param>
        private void OnDetectComplete(object sender, DetectCompleteEventArgs e)
        {
            if (this.Dispatcher.CheckAccess())
            {
                this.CreatePageSequence();
                this.GoToNextPage();
            }
            else
            {
                this.Dispatcher.Invoke(
                    DispatcherPriority.Normal,
                    new OnDetectCompleteCallback(this.OnDetectComplete),
                    sender, 
                    e);
            }

        }

        /// <summary>
        /// Handles the completion of the apply phase
        /// </summary>
        /// <param name="sender">Event sender</param>
        /// <param name="e">Event arguments</param>
        private void OnApplyComplete(object sender, ApplyCompleteEventArgs e)
        {
            if (this.Dispatcher.CheckAccess())
            {
                this.completionStatus = e.Status;
                this.GoToNextPage();
            }
            else
            {
                this.Dispatcher.Invoke(
                    DispatcherPriority.Normal,
                    new OnApplyCompleteCallback(this.OnApplyComplete),
                    sender,
                    e);
            }
        }

        /// <summary>
        /// Determines whether any user option was detected as installed on the machine.
        /// </summary>
        /// <returns>A value indicating whether any user option is installed.</returns>
        private bool IsAnythingDetectedAsInstalled()
        {
            bool somethingInstalled = false;

            foreach (Option option in this.setupSession.OptionCollection)
            {
                if (option.IsDetectedAsInstalled)
                {
                    somethingInstalled = true;
                    break;
                }
            }

            return somethingInstalled;
        }

        /// <summary>
        /// Creates page sequence based on the current user action.
        /// TODO: Remove this code when pages can be added to a wizard control in the designer.
        /// </summary>
        private void CreatePageSequence()
        {
            // Pages common to all modes
            this.optionSelectionPage = new OptionSelectionPage();
            this.progressPage = new ProgressPage();
            this.finishPage = new FinishPage();

            if (LaunchAction.Install == this.setupSession.UserAction)
            {
                if (this.IsAnythingDetectedAsInstalled())
                {
                    // Go into maintenance mode. Let user pick the maintenance action. 
                    this.maintenancePage = new MaintenancePage();

                    this.welcomePage.NextPage = this.maintenancePage;
                    this.maintenancePage.NextPage = this.optionSelectionPage;
                    this.optionSelectionPage.NextPage = this.progressPage;
                    this.progressPage.NextPage = this.finishPage;

                    this.optionSelectionPage.PreviousPage = this.maintenancePage;
                }
                else
                {
                    // Go into new installation mode
                    this.eulaPage = new EulaPage();
                    this.settingsSelectionPage = new SettingsSelectionPage();

                    // Installation workflow
                    this.welcomePage.NextPage = this.eulaPage;
                    this.eulaPage.NextPage = this.optionSelectionPage;
                    this.optionSelectionPage.NextPage = this.settingsSelectionPage;
                    this.settingsSelectionPage.NextPage = this.progressPage;
                    this.progressPage.NextPage = this.finishPage;

                    this.settingsSelectionPage.PreviousPage = this.optionSelectionPage;
                    this.optionSelectionPage.PreviousPage = this.eulaPage;
                }
            }
            else if (LaunchAction.Uninstall == this.setupSession.UserAction ||
                LaunchAction.Modify == this.setupSession.UserAction ||
                LaunchAction.Repair == this.setupSession.UserAction)
            {
                // Go into mode specified through the command line arguments
                this.welcomePage.NextPage = this.optionSelectionPage;
                this.optionSelectionPage.NextPage = this.progressPage;
                this.progressPage.NextPage = this.finishPage;
            }
        }

        // =========================================================================================
        // Commands
        // =========================================================================================
        /// <summary>
        /// Answers whether NextPage command can execute.
        /// </summary>
        /// <param name="sender">Event sender</param>
        /// <param name="e">Event arguments</param>
        private void NextPage_CanExecute(object sender, System.Windows.Input.CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = (null != this.currentPage) && (null != this.currentPage.NextPage);
        }

        /// <summary>
        /// Execute the NextPage command.
        /// </summary>
        /// <param name="sender">Event sender</param>
        /// <param name="e">Event arguments</param>
        private void NextPage_Executed(object sender, System.Windows.Input.ExecutedRoutedEventArgs e)
        {
            this.GoToNextPage();
        }

        /// <summary>
        /// Answers whether PreviousPage command can execute.
        /// </summary>
        /// <param name="sender">Event sender</param>
        /// <param name="e">Event arguments</param>
        private void PreviousPage_CanExecute(object sender, System.Windows.Input.CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = (null != this.currentPage) && (null != this.currentPage.PreviousPage);
        }

        /// <summary>
        /// Execute the PreviousPage command.
        /// </summary>
        /// <param name="sender">Event sender</param>
        /// <param name="e">Event arguments</param>
        private void PreviousPage_Executed(object sender, System.Windows.Input.ExecutedRoutedEventArgs e)
        {
            this.GoToPreviousPage();
        }

        /// <summary>
        /// Execute the application close command.
        /// </summary>
        /// <param name="sender">Event sender</param>
        /// <param name="e">Event arguments</param>
        private void Close_Executed(object sender, System.Windows.Input.ExecutedRoutedEventArgs e)
        {
            this.Close();
        }

        /// <summary>
        /// Execute the setup cancel command.
        /// </summary>
        /// <param name="sender">Event sender</param>
        /// <param name="e">Event arguments</param>
        private void Cancel_Executed(object sender, System.Windows.Input.ExecutedRoutedEventArgs e)
        {
            // TODO: Figure out how to tell the Engine about cancellation so it can rollback.

            CancelWindow cancelWindow = new CancelWindow();
            if (true == cancelWindow.DoModal())
            {
                this.Close();
            }
        }

    }
}
