//-------------------------------------------------------------------------------------------------
// <copyright file="WixPackage.cs" company="Microsoft">
//    Copyright (c) Microsoft Corporation.  All rights reserved.
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
// Contains the WixPackage class.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.VisualStudio
{
    using System;
    using System.Diagnostics;
    using System.Globalization;
    using System.Reflection;
    using System.Runtime.InteropServices;
    using Microsoft.VisualStudio;
    using Microsoft.VisualStudio.Package;
    using Microsoft.VisualStudio.Shell;
    using Microsoft.VisualStudio.Shell.Interop;

    using Microsoft.Tools.WindowsInstallerXml.VisualStudio.PropertyPages;

    /// <summary>
    /// Implements and/or provides all of the required interfaces and services to allow the
    /// Microsoft Windows Installer XML (WiX) project to be integrated into the Visual Studio
    /// environment.
    /// </summary>
    [DefaultRegistryRoot(@"Software\\Microsoft\\VisualStudio\\8.0Exp")]
    [InstalledProductRegistration(true, "WiX", null, null)]
    [Guid("E0EE8E7D-F498-459e-9E90-2B3D73124AD5")]
    [PackageRegistration(RegisterUsing = RegistrationMethod.CodeBase, UseManagedResourcesOnly = true)]
    [ProvideLoadKey("Standard", "3.5", "Votive", "Microsoft", WixPackage.PackageLoadKeyResourceId)]
    [ProvideObject(typeof(WixInstallerPropertyPage), RegisterUsing = RegistrationMethod.CodeBase)]
    [ProvideObject(typeof(WixBuildEventsPropertyPage), RegisterUsing = RegistrationMethod.CodeBase)]
    [ProvideObject(typeof(WixBuildPropertyPage), RegisterUsing = RegistrationMethod.CodeBase)]
    [ProvideObject(typeof(WixPathsPropertyPage), RegisterUsing = RegistrationMethod.CodeBase)]
    [ProvideProjectFactory(typeof(WixProjectFactory), WixProjectNode.ProjectTypeName, "#100", "wixproj", "wixproj", "", LanguageVsTemplate = "WiX")]
    public sealed class WixPackage : ProjectPackage, IVsInstalledProduct
    {
        // =========================================================================================
        // Member Variables
        // =========================================================================================

        private const short PackageLoadKeyResourceId = 150;
        private const uint SplashBitmapResourceId = 300;
        private const uint AboutBoxIconResourceId = 400;

        private static WixPackage instance;
        private string cachedFileVersion;

        private WixPackageSettings settings;

        // =========================================================================================
        // Properties
        // =========================================================================================

        /// <summary>
        /// Gets the singleton WixPackage instance.
        /// </summary>
        public static WixPackage Instance
        {
            get { return WixPackage.instance; }
        }

        /// <summary>
        /// Gets the settings stored in the registry for this package.
        /// </summary>
        /// <value>The settings stored in the registry for this package.</value>
        public WixPackageSettings Settings
        {
            get { return this.settings; }
        }

        private string CachedFileVersion
        {
            get
            {
                if (String.IsNullOrEmpty(this.cachedFileVersion))
                {
                    Assembly executingAssembly = Assembly.GetExecutingAssembly();
                    FileVersionInfo fileVersion = FileVersionInfo.GetVersionInfo(executingAssembly.Location);
                    this.cachedFileVersion = fileVersion.FileVersion;
                }

                return this.cachedFileVersion;
            }
        }

        // =========================================================================================
        // Methods
        // =========================================================================================

        /// <summary>
        /// Visual Studio 2005 no longer calls this method.
        /// </summary>
        /// <param name="pIdBmp">The resource ID of the bitmap to show on the splash screen.</param>
        /// <returns>If the method succeeds, it returns S_OK. If it fails, it returns an error code.</returns>
        int IVsInstalledProduct.IdBmpSplash(out uint pIdBmp)
        {
            pIdBmp = SplashBitmapResourceId;
            return VSConstants.S_OK;
        }

        /// <summary>
        /// Obtains a pointer to the string containing the product details that are displayed in the
        /// About dialog box on the Help menu. Not called for the splash screen.
        /// </summary>
        /// <param name="pbstrProductDetails">The product details to display on the About dialog.</param>
        /// <returns>If the method succeeds, it returns S_OK. If it fails, it returns an error code.</returns>
        int IVsInstalledProduct.ProductDetails(out string pbstrProductDetails)
        {
            pbstrProductDetails = String.Format(CultureInfo.InvariantCulture, WixStrings.ProductDetails, this.CachedFileVersion);
            return VSConstants.S_OK;
        }

        /// <summary>
        /// Obtains the icon used in the splash screen and the About dialog box on the Help menu.
        /// </summary>
        /// <param name="pIdIco">The icon used in the splash screen and the About dialog box on the Help menu.</param>
        /// <returns>If the method succeeds, it returns S_OK. If it fails, it returns an error code.</returns>
        int IVsInstalledProduct.IdIcoLogoForAboutbox(out uint pIdIco)
        {
            pIdIco = AboutBoxIconResourceId;
            return VSConstants.S_OK;
        }

        /// <summary>
        /// Obtains a pointer to the string containing the ID of the product that is displayed in
        /// the About dialog box on the Help menu. Not called for the splash screen.
        /// </summary>
        /// <param name="pbstrPID">The ID of the product.</param>
        /// <returns>If the method succeeds, it returns S_OK. If it fails, it returns an error code.</returns>
        int IVsInstalledProduct.ProductID(out string pbstrPID)
        {
            pbstrPID = WixStrings.ProductId;
            return VSConstants.S_OK;
        }

        /// <summary>
        /// Obtains a pointer to the string containing the official name of the product that is
        /// displayed in the splash screen and About dialog box on the Help menu.
        /// </summary>
        /// <param name="pbstrName">The official name of the product.</param>
        /// <returns>If the method succeeds, it returns S_OK. If it fails, it returns an error code.</returns>
        int IVsInstalledProduct.OfficialName(out string pbstrName)
        {
            pbstrName = WixStrings.OfficialName;
            return VSConstants.S_OK;
        }

        /// <summary>
        /// Initializes the package by registering all of the services that we support.
        /// </summary>
        protected override void Initialize()
        {
            base.Initialize();

            this.settings = new WixPackageSettings(this);
            this.RegisterProjectFactory(new WixProjectFactory(this));

            WixPackage.instance = this;
        }
    }
}
