//-------------------------------------------------------------------------------------------------
// <copyright file="MuxDomainManager.cs" company="Microsoft">
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
// AppDomainManager for creating the MUX and new AppDomains.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Bootstrapper
{
    using System;
    using System.Diagnostics;
    using System.IO;
    using System.Security.Policy;

    /// <summary>
    /// The <see cref="AppDomainManager"/> used to create the MUX and new AppDomain.
    /// </summary>
    internal sealed class MuxDomainManager : AppDomainManager
    {
        /// <summary>
        /// Creates new AppDomains in the host CLR within this process.
        /// </summary>
        /// <param name="name">The friendly name of the <see cref="AppDomain"/> to create.</param>
        /// <param name="evidence"><see cref="Evidence"/> that identifies the site of the host process or new <see cref="AppDomain"/>.</param>
        /// <param name="setup">Setup information used to initialize the <see cref="AppDomain"/>.</param>
        /// <returns></returns>
        public override AppDomain CreateDomain(string name, Evidence evidence, AppDomainSetup setup)
        {
            // Make sure we have setup information.
            if (null == setup)
            {
                setup = new AppDomainSetup();
            }

            // The application base is always the directory of the host process.
            setup.ApplicationBase = Path.GetDirectoryName(Process.GetCurrentProcess().MainModule.FileName);

            // Use the process name as the application name.
            setup.ApplicationName = Process.GetCurrentProcess().ProcessName;

            // All new AppDomains should use the specified mux.config application configuration file.
            setup.ConfigurationFile = Path.Combine(setup.ApplicationBase, "mux.config");

            return AppDomainManager.CreateDomainHelper(name, evidence, setup);
        }
    }
}
