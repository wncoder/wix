//-----------------------------------------------------------------------
// <copyright file="Settings.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
//    
//    The use and distribution terms for this software are covered by the
//    Common Public License 1.0 (http://opensource.org/licenses/cpl1.0.php)
//    which can be found in the file CPL.TXT at the root of this distribution.
//    By using this software in any fashion, you are agreeing to be bound by
//    the terms of this license.
//    
//    You must not remove this notice, or any other, from this software.
// </copyright>
// <summary>Contains settings such as default directories</summary>
//-----------------------------------------------------------------------

namespace WixTest
{
    using System;

    /// <summary>
    /// Contains settings such as default directories
    /// </summary>
    public static class Settings
    {

        /// <summary>
        /// The build flavor to run tests against
        /// </summary>
        private static string flavor = String.Empty;

        /// <summary>
        /// The default location for MSBuild
        /// </summary>
        private static string msBuildDirectory = String.Empty;

        /// <summary>
        /// The location of the wix.targets file
        /// </summary>
        private static string wixTargetsPath = String.Empty;

        /// <summary>
        /// The location of the WixTasks.dll file
        /// </summary>
        private static string wixTasksPath = String.Empty;

        /// <summary>
        /// The default location for the WiX tools
        /// </summary>
        private static string wixToolDirectory = String.Empty;

        /// <summary>
        /// Folder for extracted files
        /// </summary>
        public const string ExtractedFilesFolder = "extracted";

        /// <summary>
        /// Folder for .msi file types
        /// </summary>
        public const string MSIFolder = "msis";

        /// <summary>
        /// Folder for .msm file types
        /// </summary>
        public const string MSMFolder = "msms";

        /// <summary>
        /// Folder for .msp file types
        /// </summary>
        public const string MSPFolder = "msps";

        /// <summary>
        /// Folder for .mst file types
        /// </summary>
        public const string MSTFolder = "msts";

        /// <summary>
        /// Folder for .wixobj file types
        /// </summary>
        public const string WixobjFolder = "wixobjs";

        /// <summary>
        /// Folder for .wixout file types
        /// </summary>
        public const string WixoutFolder = "wixouts";

        /// <summary>
        /// The build flavor to run tests against
        /// </summary>
        public static string Flavor
        {
            get
            {
                return Settings.flavor;
            }

            set
            {
                Settings.flavor = value;
            }
        }

        /// <summary>
        /// The default location for MSBuild
        /// </summary>
        public static string MSBuildDirectory
        {
            get
            {
                return Settings.msBuildDirectory;
            }

            set
            {
                Settings.msBuildDirectory = value;
            }
        }

        /// <summary>
        /// The location for the WiX build output
        /// </summary>
        public static string WixBuildDirectory { get; set; }

        /// <summary>
        /// The location of the wix.targets file
        /// </summary>
        public static string WixTargetsPath
        {
            get
            {
                return Settings.wixTargetsPath;
            }

            set
            {
                Settings.wixTargetsPath = value;
            }
        }

        /// <summary>
        /// The location of the WixTasks.dll file
        /// </summary>
        public static string WixTasksPath
        {
            get
            {
                return Settings.wixTasksPath;
            }

            set
            {
                Settings.wixTasksPath = value;
            }
        }

        /// <summary>
        /// The location for the WiX tools
        /// </summary>
        public static string WixToolDirectory
        {
            get
            {
                return Settings.wixToolDirectory;
            }

            set
            {
                Settings.wixToolDirectory = value;
            }
        }
    }
}
