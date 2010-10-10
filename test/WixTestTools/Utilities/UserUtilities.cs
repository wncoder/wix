//-----------------------------------------------------------------------
// <copyright file="UserUtils.cs" company="Microsoft">
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
// <summary>
//     - Contains methods used for getting values from another users environment.
// </summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Utilities
{
    using System.Collections.Generic;
    using System.IO;

    public class UserUtilities
    {
        /// <summary>
        /// To get the temp paths of all users in given system
        /// </summary>
        /// <returns>List containing user temp paths</returns>
        public static List<string> GetAllUserTempPaths()
        {
            List<string> tempDirs = new List<string>();

            string tempPath = System.Environment.ExpandEnvironmentVariables("%TEMP%");
            string userName = System.Environment.ExpandEnvironmentVariables("%USERNAME%");
            string userRoot = tempPath.Substring(0, tempPath.ToLower().IndexOf(userName.ToLower()));

            foreach (string directory in Directory.GetDirectories(userRoot))
            {
                string userTemp = tempPath.Replace(Path.Combine(userRoot, userName), directory);
                if (Directory.Exists(userTemp))
                {
                    tempDirs.Add(userTemp);
                }
            }

            return tempDirs;
        }

        /// <summary>
        /// To get LocalAppData path of all users
        /// </summary>
        /// <returns>List containing user LocalAppData path</returns>
        public static List<string> GetAllUserLocalAppDataPaths()
        {
            List<string> localAddDataDirs = new List<string>();

            string localAppData = System.Environment.ExpandEnvironmentVariables("%LOCALAPPDATA%");
            string userName = System.Environment.ExpandEnvironmentVariables("%USERNAME%");
            string userRoot = localAppData.Substring(0, localAppData.ToLower().IndexOf(userName.ToLower()));

            foreach (string directory in Directory.GetDirectories(userRoot))
            {
                string userLocalAppData = localAppData.Replace(Path.Combine(userRoot, userName), directory);
                if (Directory.Exists(userLocalAppData))
                {
                    localAddDataDirs.Add(userLocalAppData);
                }
            }

            return localAddDataDirs;

        }

    }
}
