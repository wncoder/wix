//-------------------------------------------------------------------------------------------------
// <copyright file="UserExperience.cs" company="Microsoft">
// Copyright (c) Microsoft Corporation. All rights reserved.
// </copyright>
// 
// <summary>
// The default user experience.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Bootstrapper
{
    using System;
    using System.Reflection;
    using System.Runtime.InteropServices;

    /// <summary>
    /// The default user experience.
    /// </summary>
    [ClassInterface(ClassInterfaceType.None)]
    public class UserExperience : MarshalByRefObject, IBurnUserExperience
    {
        private Engine engine;
        private Command command;

        /// <summary>
        /// Creates a new instance of the <see cref="UserExperience"/> class.
        /// </summary>
        public UserExperience()
        {
            this.engine = null;
        }

        /// <summary>
        /// Fired when the overall detection phase has begun.
        /// </summary>
        public event EventHandler<DetectBeginEventArgs> DetectBegin;

        /// <summary>
        /// Fired when the detection for a prior bundle has begun.
        /// </summary>
        public event EventHandler<DetectPriorBundleEventArgs> DetectPriorBundle;

        /// <summary>
        /// Fired when the detection for a specific package has begun.
        /// </summary>
        public event EventHandler<DetectPackageBeginEventArgs> DetectPackageBegin;

        /// <summary>
        /// Fired when the detection for a specific package has completed.
        /// </summary>
        public event EventHandler<DetectPackageCompleteEventArgs> DetectPackageComplete;

        /// <summary>
        /// Fired when the detection phase has completed.
        /// </summary>
        public event EventHandler<DetectCompleteEventArgs> DetectComplete;

        /// <summary>
        /// Fired when the engine has begun planning the installation.
        /// </summary>
        public event EventHandler<PlanBeginEventArgs> PlanBegin;

        /// <summary>
        /// Fired when the engine has begun planning for a prior bundle.
        /// </summary>
        public event EventHandler<PlanPriorBundleEventArgs> PlanPriorBundle;

        /// <summary>
        /// Fired when the engine has begun planning the installation of a specific package.
        /// </summary>
        public event EventHandler<PlanPackageBeginEventArgs> PlanPackageBegin;

        /// <summary>
        /// Fired when the engine has completed planning the installation of a specific package.
        /// </summary>
        public event EventHandler<PlanPackageCompleteEventArgs> PlanPackageComplete;

        /// <summary>
        /// Fired when the engine has completed planning the installation.
        /// </summary>
        public event EventHandler<PlanCompleteEventArgs> PlanComplete;

        /// <summary>
        /// Fired when the engine has begun installing the bundle.
        /// </summary>
        public event EventHandler<ApplyBeginEventArgs> ApplyBegin;

        /// <summary>
        /// Fired when the engine has begun registering the location and visibility of the bundle.
        /// </summary>
        public event EventHandler<RegisterBeginEventArgs> RegisterBegin;

        /// <summary>
        /// Fired when the engine has completed registering the location and visibility of the bundle.
        /// </summary>
        public event EventHandler<RegisterCompleteEventArgs> RegisterComplete;

        /// <summary>
        /// Fired when the engine has begun removing the registration for the location and visibility of the bundle.
        /// </summary>
        public event EventHandler<UnregisterBeginEventArgs> UnregisterBegin;

        /// <summary>
        /// Fired when the engine has completed removing the registration for the location and visibility of the bundle.
        /// </summary>
        public event EventHandler<UnregisterCompleteEventArgs> UnregisterComplete;

        /// <summary>
        /// Fired when the engine has begun caching the installation sources.
        /// </summary>
        public event EventHandler<CacheBeginEventArgs> CacheBegin;

        /// <summary>
        /// Fired after the engine has cached the installation sources.
        /// </summary>
        public event EventHandler<CacheCompleteEventArgs> CacheComplete;

        /// <summary>
        /// Fired when the engine has begun installing packages.
        /// </summary>
        public event EventHandler<ExecuteBeginEventArgs> ExecuteBegin;

        /// <summary>
        /// Fired when the engine has begun installing a specific package.
        /// </summary>
        public event EventHandler<ExecutePackageBeginEventArgs> ExecutePackageBegin;

        /// <summary>
        /// Fired when the engine has encountered an error.
        /// </summary>
        public event EventHandler<ErrorEventArgs> Error;

        /// <summary>
        /// Fired when the engine has changed progress for the bundle installation.
        /// </summary>
        public event EventHandler<ProgressEventArgs> Progress;

        /// <summary>
        /// Fired when Windows Installer sends an installation message.
        /// </summary>
        public event EventHandler<ExecuteMsiMessageEventArgs> ExecuteMsiMessage;

        /// <summary>
        /// Fired when Windows Installer sends a files in use installation message.
        /// </summary>
        public event EventHandler<ExecuteMsiFilesInUseEventArgs> ExecuteMsiFilesInUse;

        /// <summary>
        /// Fired when the engine has completed installing a specific package.
        /// </summary>
        public event EventHandler<ExecutePackageCompleteEventArgs> ExecutePackageComplete;

        /// <summary>
        /// Fired when the engine has completed installing packages.
        /// </summary>
        public event EventHandler<ExecuteCompleteEventArgs> ExecuteComplete;

        /// <summary>
        /// Fired by the engine to request a restart now or inform the user a manual restart is required later.
        /// </summary>
        public event EventHandler<RestartRequiredEventArgs> RestartRequired;

        /// <summary>
        /// Fired when the engine has completed installing the bundle.
        /// </summary>
        public event EventHandler<ApplyCompleteEventArgs> ApplyComplete;

        /// <summary>
        /// Fired by the engine to allow the user experience to change the source using <see cref="Engine.SetSource"/>.
        /// </summary>
        public event EventHandler<ResolveSourceEventArgs> ResolveSource;

        /// <summary>
        /// Fired when the engine has begun caching a specific package.
        /// </summary>
        public event EventHandler<CachePackageBeginEventArgs> CachePackageBegin;

        /// <summary>
        /// Fired when the engine has completed caching a specific package.
        /// </summary>
        public event EventHandler<CachePackageCompleteEventArgs> CachePackageComplete;

        /// <summary>
        /// Fired by the engine when it has begun downloading a specific payload.
        /// </summary>
        public event EventHandler<DownloadPayloadBeginEventArgs> DownloadPayloadBegin;

        /// <summary>
        /// Fired by the engine when it has completed downloading a specific payload.
        /// </summary>
        public event EventHandler<DownloadPayloadCompleteEventArgs> DownloadPayloadComplete;

        /// <summary>
        /// Fired by the engine while downloading payload.
        /// </summary>
        public event EventHandler<DownloadProgressEventArgs> DownloadProgress;

        /// <summary>
        /// Fired by the engine while executing on payload.
        /// </summary>
        public event EventHandler<ExecuteProgressEventArgs> ExecuteProgress;

        /// <summary>
        /// Gets whether packages can be downloaded.
        /// </summary>
        public virtual bool CanPackagesBeDownloaded
        {
            get { return false; }
        }

        /// <summary>
        /// Gets the <see cref="Command"/> information for how the UX should be started.
        /// </summary>
        public Command Command
        {
            get { return this.command; }
            internal set { this.command = value; }
        }

        /// <summary>
        /// Gets the <see cref="Engine"/> for interaction with the Engine.
        /// </summary>
        public Engine Engine
        {
            get { return this.engine; }
        }

        /// <summary>
        /// Called by the engine to initialize the user experience.
        /// </summary>
        /// <param name="show">Message box states to use for any user interface.</param>
        /// <param name="resume">Describes why the bungle installation was resumed.</param>
        protected virtual void Initialize(int show, ResumeType resume)
        {
        }

        /// <summary>
        /// Called by the engine to uninitialize the user experience.
        /// </summary>
        protected virtual void Uninitialize()
        {
        }

        /// <summary>
        /// Called when the overall detection phase has begun.
        /// </summary>
        /// <param name="args">Additional arguments for this event.</param>
        protected virtual void OnDetectBegin(DetectBeginEventArgs args)
        {
            EventHandler<DetectBeginEventArgs> handler = this.DetectBegin;
            if (null != handler)
            {
                handler(this, args);
            }
        }

        /// <summary>
        /// Called when the detection for a prior bundle has begun.
        /// </summary>
        /// <param name="args">Additional arguments for this event.</param>
        protected virtual void OnDetectPriorBundle(DetectPriorBundleEventArgs args)
        {
            EventHandler<DetectPriorBundleEventArgs> handler = this.DetectPriorBundle;
            if (null != handler)
            {
                handler(this, args);
            }
        }

        /// <summary>
        /// Called when the detection for a specific package has begun.
        /// </summary>
        /// <param name="args">Additional arguments for this event.</param>
        protected virtual void OnDetectPackageBegin(DetectPackageBeginEventArgs args)
        {
            EventHandler<DetectPackageBeginEventArgs> handler = this.DetectPackageBegin;
            if (null != handler)
            {
                handler(this, args);
            }
        }

        /// <summary>
        /// Called when the detection for a specific package has completed.
        /// </summary>
        /// <param name="args">Additional arguments for this event.</param>
        protected virtual void OnDetectPackageComplete(DetectPackageCompleteEventArgs args)
        {
            EventHandler<DetectPackageCompleteEventArgs> handler = this.DetectPackageComplete;
            if (null != handler)
            {
                handler(this, args);
            }
        }

        /// <summary>
        /// Called when the detection phase has completed.
        /// </summary>
        /// <param name="args">Additional arguments for this event.</param>
        protected virtual void OnDetectComplete(DetectCompleteEventArgs args)
        {
            EventHandler<DetectCompleteEventArgs> handler = this.DetectComplete;
            if (null != handler)
            {
                handler(this, args);
            }
        }

        /// <summary>
        /// Called when the engine has begun planning the installation.
        /// </summary>
        /// <param name="args">Additional arguments for this event.</param>
        protected virtual void OnPlanBegin(PlanBeginEventArgs args)
        {
            EventHandler<PlanBeginEventArgs> handler = this.PlanBegin;
            if (null != handler)
            {
                handler(this, args);
            }
        }

        /// <summary>
        /// Called when the engine has begun planning for a prior bundle.
        /// </summary>
        /// <param name="args">Additional arguments for this event.</param>
        protected virtual void OnPlanPriorBundle(PlanPriorBundleEventArgs args)
        {
            EventHandler<PlanPriorBundleEventArgs> handler = this.PlanPriorBundle;
            if (null != handler)
            {
                handler(this, args);
            }
        }

        /// <summary>
        /// Called when the engine has begun planning the installation of a specific package.
        /// </summary>
        /// <param name="args">Additional arguments for this event.</param>
        protected virtual void OnPlanPackageBegin(PlanPackageBeginEventArgs args)
        {
            EventHandler<PlanPackageBeginEventArgs> handler = this.PlanPackageBegin;
            if (null != handler)
            {
                handler(this, args);
            }
        }

        /// <summary>
        /// Called when then engine has completed planning the installation of a specific package.
        /// </summary>
        /// <param name="args">Additional arguments for this event.</param>
        protected virtual void OnPlanPackageComplete(PlanPackageCompleteEventArgs args)
        {
            EventHandler<PlanPackageCompleteEventArgs> handler = this.PlanPackageComplete;
            if (null != handler)
            {
                handler(this, args);
            }
        }

        /// <summary>
        /// Called when the engine has completed planning the installation.
        /// </summary>
        /// <param name="args">Additional arguments for this event.</param>
        protected virtual void OnPlanComplete(PlanCompleteEventArgs args)
        {
            EventHandler<PlanCompleteEventArgs> handler = this.PlanComplete;
            if (null != handler)
            {
                handler(this, args);
            }
        }

        /// <summary>
        /// Called when the engine has begun installing the bundle.
        /// </summary>
        /// <param name="args">Additional arguments for this event.</param>
        protected virtual void OnApplyBegin(ApplyBeginEventArgs args)
        {
            EventHandler<ApplyBeginEventArgs> handler = this.ApplyBegin;
            if (null != handler)
            {
                handler(this, args);
            }
        }

        /// <summary>
        /// Called when the engine has begun registering the location and visibility of the bundle.
        /// </summary>
        /// <param name="args">Additional arguments for this event.</param>
        protected virtual void OnRegisterBegin(RegisterBeginEventArgs args)
        {
            EventHandler<RegisterBeginEventArgs> handler = this.RegisterBegin;
            if (null != handler)
            {
                handler(this, args);
            }
        }

        /// <summary>
        /// Called when the engine has completed registering the location and visilibity of the bundle.
        /// </summary>
        /// <param name="args">Additional arguments for this event.</param>
        protected virtual void OnRegisterComplete(RegisterCompleteEventArgs args)
        {
            EventHandler<RegisterCompleteEventArgs> handler = this.RegisterComplete;
            if (null != handler)
            {
                handler(this, args);
            }
        }

        /// <summary>
        /// Called when the engine has begun removing the registration for the location and visibility of the bundle.
        /// </summary>
        /// <param name="args">Additional arguments for this event.</param>
        protected virtual void OnUnregisterBegin(UnregisterBeginEventArgs args)
        {
            EventHandler<UnregisterBeginEventArgs> handler = this.UnregisterBegin;
            if (null != handler)
            {
                handler(this, args);
            }
        }

        /// <summary>
        /// Called when the engine has completed removing the registration for the location and visibility of the bundle.
        /// </summary>
        /// <param name="args">Additional arguments for this event.</param>
        protected virtual void OnUnregisterComplete(UnregisterCompleteEventArgs args)
        {
            EventHandler<UnregisterCompleteEventArgs> handler = this.UnregisterComplete;
            if (null != handler)
            {
                handler(this, args);
            }
        }

        /// <summary>
        /// Called when the engine has begun caching the installation sources.
        /// </summary>
        /// <param name="args"></param>
        protected virtual void OnCacheBegin(CacheBeginEventArgs args)
        {
            EventHandler<CacheBeginEventArgs> handler = this.CacheBegin;
            if (null != handler)
            {
                handler(this, args);
            }
        }

        /// <summary>
        /// Called after the engine has cached the installation sources.
        /// </summary>
        /// <param name="args">Additional arguments for this event.</param>
        protected virtual void OnCacheComplete(CacheCompleteEventArgs args)
        {
            EventHandler<CacheCompleteEventArgs> handler = this.CacheComplete;
            if (null != handler)
            {
                handler(this, args);
            }
        }

        /// <summary>
        /// Called when the engine has begun installing packages.
        /// </summary>
        /// <param name="args">Additional arguments for this event.</param>
        protected virtual void OnExecuteBegin(ExecuteBeginEventArgs args)
        {
            EventHandler<ExecuteBeginEventArgs> handler = this.ExecuteBegin;
            if (null != handler)
            {
                handler(this, args);
            }
        }

        /// <summary>
        /// Called when the engine has begun installing a specific package.
        /// </summary>
        /// <param name="args">Additional arguments for this event.</param>
        protected virtual void OnExecutePackageBegin(ExecutePackageBeginEventArgs args)
        {
            EventHandler<ExecutePackageBeginEventArgs> handler = this.ExecutePackageBegin;
            if (null != handler)
            {
                handler(this, args);
            }
        }

        /// <summary>
        /// Called when the engine has encountered an error.
        /// </summary>
        /// <param name="args">Additional arguments for this event.</param>
        protected virtual void OnError(ErrorEventArgs args)
        {
            EventHandler<ErrorEventArgs> handler = this.Error;
            if (null != handler)
            {
                handler(this, args);
            }
        }

        /// <summary>
        /// Called when the engine has changed progress for the bundle installation.
        /// </summary>
        /// <param name="args">Additional arguments for this event.</param>
        protected virtual void OnProgress(ProgressEventArgs args)
        {
            EventHandler<ProgressEventArgs> handler = this.Progress;
            if (null != handler)
            {
                handler(this, args);
            }
        }

        /// <summary>
        /// Called when Windows Installer sends an installation message.
        /// </summary>
        /// <param name="args">Additional arguments for this event.</param>
        protected virtual void OnExecuteMsiMessage(ExecuteMsiMessageEventArgs args)
        {
            EventHandler<ExecuteMsiMessageEventArgs> handler = this.ExecuteMsiMessage;
            if (null != handler)
            {
                handler(this, args);
            }
        }

        /// <summary>
        /// Called when Windows Installer sends a file in use installation message.
        /// </summary>
        /// <param name="args">Additional arguments for this event.</param>
        protected virtual void OnExecuteMsiFilesInUse(ExecuteMsiFilesInUseEventArgs args)
        {
            EventHandler<ExecuteMsiFilesInUseEventArgs> handler = this.ExecuteMsiFilesInUse;
            if (null != handler)
            {
                handler(this, args);
            }
        }

        /// <summary>
        /// Called when the engine has completed installing a specific package.
        /// </summary>
        /// <param name="args">Additional arguments for this event.</param>
        protected virtual void OnExecutePackageComplete(ExecutePackageCompleteEventArgs args)
        {
            EventHandler<ExecutePackageCompleteEventArgs> handler = this.ExecutePackageComplete;
            if (null != handler)
            {
                handler(this, args);
            }
        }

        /// <summary>
        /// Called when the engine has completed installing packages.
        /// </summary>
        /// <param name="args">Additional arguments for this event.</param>
        protected virtual void OnExecuteComplete(ExecuteCompleteEventArgs args)
        {
            EventHandler<ExecuteCompleteEventArgs> handler = this.ExecuteComplete;
            if (null != handler)
            {
                handler(this, args);
            }
        }

        /// <summary>
        /// Called by the engine to request a restart now or inform the user a manual restart is required later.
        /// </summary>
        /// <param name="args">Additional arguments for this event.</param>
        protected virtual void OnRestartRequired(RestartRequiredEventArgs args)
        {
            EventHandler<RestartRequiredEventArgs> handler = this.RestartRequired;
            if (null != handler)
            {
                handler(this, args);
            }
        }

        /// <summary>
        /// Called when the engine has completed installing the bundle.
        /// </summary>
        /// <param name="args">Additional arguments for this event.</param>
        protected virtual void OnApplyComplete(ApplyCompleteEventArgs args)
        {
            EventHandler<ApplyCompleteEventArgs> handler = this.ApplyComplete;
            if (null != handler)
            {
                handler(this, args);
            }
        }

        /// <summary>
        /// Called by the engine to allow the user experience to change the source using <see cref="Engine.SetSource"/>.
        /// </summary>
        /// <param name="args">Additional arguments for this event.</param>
        protected virtual void OnResolveSource(ResolveSourceEventArgs args)
        {
            EventHandler<ResolveSourceEventArgs> handler = this.ResolveSource;
            if (null != handler)
            {
                handler(this, args);
            }
        }

        /// <summary>
        /// Called by the engine when it has begun caching a specific package.
        /// </summary>
        /// <param name="args"></param>
        protected virtual void OnCachePackageBegin(CachePackageBeginEventArgs args)
        {
            EventHandler<CachePackageBeginEventArgs> handler = this.CachePackageBegin;
            if (null != handler)
            {
                handler(this, args);
            }
        }

        /// <summary>
        /// Called by the engine when it has completed caching a specific package.
        /// </summary>
        /// <param name="args"></param>
        protected virtual void OnCachePackageComplete(CachePackageCompleteEventArgs args)
        {
            EventHandler<CachePackageCompleteEventArgs> handler = this.CachePackageComplete;
            if (null != handler)
            {
                handler(this, args);
            }
        }

        /// <summary>
        /// Called by the engine when it has begun downloading a specific payload.
        /// </summary>
        /// <param name="args">Additional arguments for this event.</param>
        protected virtual void OnDownloadPayloadBegin(DownloadPayloadBeginEventArgs args)
        {
            EventHandler<DownloadPayloadBeginEventArgs> handler = this.DownloadPayloadBegin;
            if (null != handler)
            {
                handler(this, args);
            }
        }

        /// <summary>
        /// Called by the engine when it has completed downloading a specific payload.
        /// </summary>
        /// <param name="args">Additional arguments for this event.</param>
        protected virtual void OnDownloadPayloadComplete(DownloadPayloadCompleteEventArgs args)
        {
            EventHandler<DownloadPayloadCompleteEventArgs> handler = this.DownloadPayloadComplete;
            if (null != handler)
            {
                handler(this, args);
            }
        }

        /// <summary>
        /// Called by the engine while downloading payload.
        /// </summary>
        /// <param name="args">Additional arguments for this event.</param>
        protected virtual void OnDownloadProgress(DownloadProgressEventArgs args)
        {
            EventHandler<DownloadProgressEventArgs> handler = this.DownloadProgress;
            if (null != handler)
            {
                handler(this, args);
            }
        }

        /// <summary>
        /// Called by the engine while executing on payload.
        /// </summary>
        /// <param name="args">Additional arguments for this event.</param>
        protected virtual void OnExecuteProgress(ExecuteProgressEventArgs args)
        {
            EventHandler<ExecuteProgressEventArgs> handler = this.ExecuteProgress;
            if (null != handler)
            {
                handler(this, args);
            }
        }

        #region IBurnUserExperience Members

        void IBurnUserExperience.Initialize(IBurnCore pCore, int nCmdShow, ResumeType resumeType)
        {
            this.engine = new Engine(pCore);
            this.Initialize(nCmdShow, resumeType);
        }

        void IBurnUserExperience.Uninitialize()
        {
            this.Uninitialize();
        }

        Result IBurnUserExperience.OnDetectBegin(int cPackages)
        {
            DetectBeginEventArgs args = new DetectBeginEventArgs(cPackages);
            this.OnDetectBegin(args);

            return args.Result;
        }

        Result IBurnUserExperience.OnDetectPriorBundle(string wzBundleId)
        {
            DetectPriorBundleEventArgs args = new DetectPriorBundleEventArgs(wzBundleId);
            this.OnDetectPriorBundle(args);

            return args.Result;
        }

        Result IBurnUserExperience.OnDetectPackageBegin(string wzPackageId)
        {
            DetectPackageBeginEventArgs args = new DetectPackageBeginEventArgs(wzPackageId);
            this.OnDetectPackageBegin(args);

            return args.Result;
        }

        void IBurnUserExperience.OnDetectPackageComplete(string wzPackageId, int hrStatus, PackageState state)
        {
            this.OnDetectPackageComplete(new DetectPackageCompleteEventArgs(wzPackageId, hrStatus, state));
        }

        void IBurnUserExperience.OnDetectComplete(int hrStatus)
        {
            this.OnDetectComplete(new DetectCompleteEventArgs(hrStatus));
        }

        Result IBurnUserExperience.OnPlanBegin(int cPackages)
        {
            PlanBeginEventArgs args = new PlanBeginEventArgs(cPackages);
            this.OnPlanBegin(args);

            return args.Result;
        }

        Result IBurnUserExperience.OnPlanPriorBundle(string wzBundleId, ref RequestState pRequestedState)
        {
            PlanPriorBundleEventArgs args = new PlanPriorBundleEventArgs(wzBundleId, pRequestedState);
            this.OnPlanPriorBundle(args);

            pRequestedState = args.State;
            return args.Result;
        }

        Result IBurnUserExperience.OnPlanPackageBegin(string wzPackageId, ref RequestState pRequestedState)
        {
            PlanPackageBeginEventArgs args = new PlanPackageBeginEventArgs(wzPackageId, pRequestedState);
            this.OnPlanPackageBegin(args);

            pRequestedState = args.State;
            return args.Result;
        }

        void IBurnUserExperience.OnPlanPackageComplete(string wzPackageId, int hrStatus, PackageState state, RequestState requested, ActionState execute, ActionState rollback)
        {
            this.OnPlanPackageComplete(new PlanPackageCompleteEventArgs(wzPackageId, hrStatus, state, requested, execute, rollback));
        }

        void IBurnUserExperience.OnPlanComplete(int hrStatus)
        {
            this.OnPlanComplete(new PlanCompleteEventArgs(hrStatus));
        }

        Result IBurnUserExperience.OnApplyBegin()
        {
            ApplyBeginEventArgs args = new ApplyBeginEventArgs();
            this.OnApplyBegin(args);

            return args.Result;
        }

        Result IBurnUserExperience.OnRegisterBegin()
        {
            RegisterBeginEventArgs args = new RegisterBeginEventArgs();
            this.OnRegisterBegin(args);

            return args.Result;
        }

        void IBurnUserExperience.OnRegisterComplete(int hrStatus)
        {
            this.OnRegisterComplete(new RegisterCompleteEventArgs(hrStatus));
        }

        void IBurnUserExperience.OnUnregisterBegin()
        {
            this.OnUnregisterBegin(new UnregisterBeginEventArgs());
        }

        void IBurnUserExperience.OnUnregisterComplete(int hrStatus)
        {
            this.OnUnregisterComplete(new UnregisterCompleteEventArgs(hrStatus));
        }

        Result IBurnUserExperience.OnCacheBegin()
        {
            CacheBeginEventArgs args = new CacheBeginEventArgs();
            this.OnCacheBegin(args);

            return args.Result;
        }

        void IBurnUserExperience.OnCacheComplete(int hrStatus)
        {
            this.OnCacheComplete(new CacheCompleteEventArgs(hrStatus));
        }

        Result IBurnUserExperience.OnExecuteBegin(int cExecutingPackages)
        {
            ExecuteBeginEventArgs args = new ExecuteBeginEventArgs(cExecutingPackages);
            this.OnExecuteBegin(args);

            return args.Result;
        }

        Result IBurnUserExperience.OnExecutePackageBegin(string wzPackageId, bool fExecute)
        {
            ExecutePackageBeginEventArgs args = new ExecutePackageBeginEventArgs(wzPackageId, fExecute);
            this.OnExecutePackageBegin(args);

            return args.Result;
        }

        Result IBurnUserExperience.OnError(string wzPackageId, int dwCode, string wzError, int dwUIHint)
        {
            ErrorEventArgs args = new ErrorEventArgs(wzPackageId, dwCode, wzPackageId, dwUIHint);
            this.OnError(args);

            return args.Result;
        }

        Result IBurnUserExperience.OnProgress(int dwProgressPercentage, int dwOverallPercentage)
        {
            ProgressEventArgs args = new ProgressEventArgs(dwProgressPercentage, dwOverallPercentage);
            this.OnProgress(args);

            return args.Result;
        }

        Result IBurnUserExperience.OnExecuteMsiMessage(string wzPackageId, InstallMessage mt, int uiFlags, string wzMessage)
        {
            ExecuteMsiMessageEventArgs args = new ExecuteMsiMessageEventArgs(wzPackageId, mt, uiFlags, wzMessage);
            this.OnExecuteMsiMessage(args);

            return args.Result;
        }

        Result IBurnUserExperience.OnExecuteMsiFilesInUse(string wzPackageId, int cFiles, string[] rgwzFiles)
        {
            ExecuteMsiFilesInUseEventArgs args = new ExecuteMsiFilesInUseEventArgs(wzPackageId, rgwzFiles);
            this.OnExecuteMsiFilesInUse(args);

            return args.Result;
        }

        void IBurnUserExperience.OnExecutePackageComplete(string wzPackageId, int hrExitCode)
        {
            this.OnExecutePackageComplete(new ExecutePackageCompleteEventArgs(wzPackageId, hrExitCode));
        }

        void IBurnUserExperience.OnExecuteComplete(int hrStatus)
        {
            this.OnExecuteComplete(new ExecuteCompleteEventArgs(hrStatus));
        }

        bool IBurnUserExperience.OnRestartRequired()
        {
            RestartRequiredEventArgs args = new RestartRequiredEventArgs();
            this.OnRestartRequired(args);

            return args.Restart;
        }

        void IBurnUserExperience.OnApplyComplete(int hrStatus)
        {
            this.OnApplyComplete(new ApplyCompleteEventArgs(hrStatus));
        }

        Result IBurnUserExperience.ResolveSource(string wzPackageId, string wzPackageOrContainerPath)
        {
            ResolveSourceEventArgs args = new ResolveSourceEventArgs(wzPackageId, wzPackageOrContainerPath);
            this.OnResolveSource(args);

            return args.Result;
        }

        bool IBurnUserExperience.CanPackagesBeDownloaded()
        {
            return this.CanPackagesBeDownloaded;
        }

        Result IBurnUserExperience.OnCachePackageBegin(string wzPackageId, long dw64PackageCacheSize)
        {
            CachePackageBeginEventArgs args = new CachePackageBeginEventArgs(wzPackageId, dw64PackageCacheSize);
            this.OnCachePackageBegin(args);

            return args.Result;
        }

        void IBurnUserExperience.OnCachePackageComplete(string wzPackageId, int hrStatus)
        {
            this.OnCachePackageComplete(new CachePackageCompleteEventArgs(wzPackageId, hrStatus));
        }

        Result IBurnUserExperience.OnDownloadPayloadBegin(string wzPayloadId, string wzPayloadFileName)
        {
            DownloadPayloadBeginEventArgs args = new DownloadPayloadBeginEventArgs(wzPayloadId, wzPayloadFileName);
            this.OnDownloadPayloadBegin(args);

            return args.Result;
        }

        void IBurnUserExperience.OnDownloadPayloadComplete(string wzPayloadId, string wzPayloadFileName, int hrStatus)
        {
            DownloadPayloadCompleteEventArgs args = new DownloadPayloadCompleteEventArgs(wzPayloadId, wzPayloadFileName, hrStatus);
            this.OnDownloadPayloadComplete(args);
        }

        Result IBurnUserExperience.OnDownloadProgress(int dwProgressPercentage, int dwOverallPercentage)
        {
            DownloadProgressEventArgs args = new DownloadProgressEventArgs(dwProgressPercentage, dwOverallPercentage);
            this.OnDownloadProgress(args);

            return args.Result;
        }

        Result IBurnUserExperience.OnExecuteProgress(int dwProgressPercentage, int dwOverallPercentage)
        {
            ExecuteProgressEventArgs args = new ExecuteProgressEventArgs(dwProgressPercentage, dwOverallPercentage);
            this.OnExecuteProgress(args);

            return args.Result;
        }

        #endregion
    }
}
