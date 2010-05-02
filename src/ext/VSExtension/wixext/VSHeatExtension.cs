//-------------------------------------------------------------------------------------------------
// <copyright file="VSHeatExtension.cs" company="Microsoft">
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
// VS-related extensions for the Windows Installer XML Toolset Harvester application.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Extensions
{
    using System;
    using System.Collections;
    using System.Globalization;
    using Microsoft.Tools.WindowsInstallerXml.Tools;

    using Wix = Microsoft.Tools.WindowsInstallerXml.Serialize;

    /// <summary>
    /// VS-related extensions for the Windows Installer XML Toolset Harvester application.
    /// </summary>
    public sealed class VSHeatExtension : HeatExtension
    {
        /// <summary>
        /// Gets a value indicating whether the extension is enabled. Because this
        /// extension requires MSBuild, it is only enabled on .NET v2.0 or later.
        /// </summary>
        public static bool Enabled
        {
            get
            {
                return Environment.Version.Major >= 2;
            }
        }

        /// <summary>
        /// Gets the supported command line types for this extension.
        /// </summary>
        /// <value>The supported command line types for this extension.</value>
        public override HeatCommandLineOption[] CommandLineTypes
        {
            get
            {
                if (VSHeatExtension.Enabled)
                {
                    return new HeatCommandLineOption[]
                    {
                        new HeatCommandLineOption("project", "harvest outputs of a VS project"),
                        new HeatCommandLineOption("-pog:<group>", Environment.NewLine +
                            "            specify output group of VS project, one of:" + Environment.NewLine +
                            "                " + String.Join(",", VSProjectHarvester.GetOutputGroupNames()) + Environment.NewLine +
                            "              This option may be repeated for multiple output groups."),
                    };
                }
                else
                {
                    return new HeatCommandLineOption[] { };
                }
            }
        }

        /// <summary>
        /// Parse the command line options for this extension.
        /// </summary>
        /// <param name="type">The active harvester type.</param>
        /// <param name="args">The option arguments.</param>
        public override void ParseOptions(string type, string[] args)
        {
            if (VSHeatExtension.Enabled && "project" == type)
            {
                string[] allOutputGroups = VSProjectHarvester.GetOutputGroupNames();
                bool suppressUniqueId = false;
                ArrayList outputGroups = new ArrayList();

                for (int i = 0; i < args.Length; i++)
                {
                    if (args[i].StartsWith("-pog:", StringComparison.Ordinal))
                    {
                        string pogName = args[i].Substring(5);
                        bool found = false;
                        foreach (string availableOutputGroup in allOutputGroups)
                        {
                            if (String.Equals(pogName, availableOutputGroup, StringComparison.Ordinal))
                            {
                                outputGroups.Add(availableOutputGroup);
                                found = true;
                                break;
                            }
                        }

                        if (!found)
                        {
                            throw new WixException(VSErrors.InvalidOutputGroup(pogName));
                        }
                    }
                    else if ("-suid" == args[i])
                    {
                        suppressUniqueId = true;
                    }
                }

                if (outputGroups.Count == 0)
                {
                    throw new WixException(VSErrors.NoOutputGroupSpecified());
                }

                VSProjectHarvester harvester = new VSProjectHarvester(
                    (string[]) outputGroups.ToArray(typeof(string)));

                harvester.SetUniqueIdentifiers = !suppressUniqueId;

                this.Core.Harvester.Extension = harvester;
            }
        }
    }
}
