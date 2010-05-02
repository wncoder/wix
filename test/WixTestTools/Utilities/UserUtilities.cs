//-----------------------------------------------------------------------
// <copyright file="UserUtils.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
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
