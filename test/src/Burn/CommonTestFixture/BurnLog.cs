//-----------------------------------------------------------------------
// <copyright file="BurnLog.cs" company="Microsoft">
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
//     - Contains methods used to identify and verify a Burn log file.
// </summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests.Burn.CommonTestFixture
{
    using System;
    using System.Collections.Generic;
    using System.IO;
    using System.Text.RegularExpressions;

    public class BurnLog
    {
        #region Factories for creating lists of BurnLog objects
        
        public static List<BurnLog> GetLatestBurnLogFromEachTempFolder(List<string> logFileNameHelpers, DateTime Start, DateTime End)
        {
            List<BurnLog> burnLogs = new List<BurnLog>();

            string myTemp = System.Environment.ExpandEnvironmentVariables("%TEMP%");
            string myUsername = System.Environment.ExpandEnvironmentVariables("%USERNAME%");
            string userRoot = myTemp.Substring(0, myTemp.ToLower().IndexOf(myUsername.ToLower()));

            foreach (string directory in Directory.GetDirectories(userRoot))
            {
                string userTemp = myTemp.Replace(Path.Combine(userRoot, myUsername), directory);
                // look in each userTemp for the latest BurnLogs
                BurnLog bl = new BurnLog(logFileNameHelpers, Start, End);
                bl.LogPath = userTemp;
                bl.SetBurnLogFileName();
                if (!String.IsNullOrEmpty(bl.FileName))
                {
                    burnLogs.Add(bl);
                }
            }

            return burnLogs;
        }

        public static List<BurnLog> GetAllBurnLogsFromEachUsersTempFolder(List<string> logFileNameHelpers, DateTime Start, DateTime End)
        {
            List<BurnLog> burnLogs = new List<BurnLog>();

            string myTemp = System.Environment.ExpandEnvironmentVariables("%TEMP%");
            string myUsername = System.Environment.ExpandEnvironmentVariables("%USERNAME%");
            string userRoot = myTemp.Substring(0, myTemp.ToLower().IndexOf(myUsername.ToLower()));

            foreach (string directory in Directory.GetDirectories(userRoot))
            {
                string userTemp = myTemp.Replace(Path.Combine(userRoot, myUsername), directory);
                // look in each userTemp for all the logs that match the patern and fall within the given time range
                try
                {
                    DirectoryInfo dir = new DirectoryInfo(System.Environment.ExpandEnvironmentVariables(userTemp));
                    foreach (string logFileNameHelper in logFileNameHelpers)
                    {
                        foreach (FileInfo file in dir.GetFiles(System.Environment.ExpandEnvironmentVariables(logFileNameHelper), SearchOption.AllDirectories))
                        {
                            if ((file.LastWriteTime >= Start) && (file.LastWriteTime <= End))
                            {
                                BurnLog bl = new BurnLog(logFileNameHelpers, Start, End);
                                bl.LogPath = userTemp;
                                bl.fileName = file.FullName;
                                burnLogs.Add(bl);
                            }
                        }
                    }
                }
                // catch if the GetFiles method throws because the folder being searched doesn't exist or you don't have access
                catch (System.IO.DirectoryNotFoundException) { }
                catch (System.Security.SecurityException) { } 
            }

            return burnLogs;
        }

        #endregion


        public DateTime StartTime;
        public DateTime EndTime;
        public string LogPath = "%temp%";
        public List<string> LogFileNameHelpers;

        protected string fileName;
        public string FileName
        {
            get { return fileName; }
        }

        public BurnLog(List<string> logFileNameHelpers, DateTime Start, DateTime End)
        {
            this.LogFileNameHelpers = logFileNameHelpers;
            StartTime = Start;
            EndTime = End;
            this.SetBurnLogFileName();
        }

        /// <summary>
        /// Get the latest log
        /// </summary>
        /// <param name="path">path of dir to search</param>
        /// <param name="searchPatterns">list of file name search patterns</param>
        /// <param name="option">search option</param>
        /// <returns>full path to latest log file with timestamp later than the original "time"</returns>
        public string GetLatestLogFile(string path, List<string> searchPatterns, SearchOption option)
        {
            DateTime latestTime = StartTime;
            string latestFile = null;

            DirectoryInfo dir = new DirectoryInfo(System.Environment.ExpandEnvironmentVariables(path));
            try
            {
                foreach (string searchPattern in searchPatterns)
                {
                    foreach (FileInfo file in dir.GetFiles(System.Environment.ExpandEnvironmentVariables(searchPattern), option))
                    {
                        if ((file.LastWriteTime > latestTime) && (file.LastWriteTime < EndTime))
                        {
                            latestTime = file.LastWriteTime;
                            latestFile = file.FullName;
                        }
                    }
                }
            }
            // catch if the GetFiles method throws because the folder being searched doesn't exist or you don't have access
            catch (System.IO.DirectoryNotFoundException) { }
            catch (System.Security.SecurityException) { } 
            return latestFile;
        }

        /// <summary>
        /// Sets the FileName property to the latest log found in the LogPath that matches the LogFileNameHelper and falls between StartTime and EndTime.
        /// </summary>
        public void SetBurnLogFileName()
        {
            fileName = null;
            string logFile = GetLatestLogFile(this.LogPath, this.LogFileNameHelpers, SearchOption.AllDirectories);
            if (!String.IsNullOrEmpty(logFile))
            {
                fileName = logFile;
            }
        }

        /// <summary>
        /// Verify the log file contains specific text
        /// </summary>
        /// <param name="containedText">text we're looking for</param>
        /// <returns>true if the string was found</returns>
        public bool LogContains(string containedText)
        {
            bool retVal = false;
            try
            {
                TextReader reader = new StreamReader(this.FileName);
                if (reader.ReadToEnd().Contains(containedText))
                {
                    retVal = true;
                }
            }
            catch
            {
            }

            return retVal;
        }

        /// <summary>
        /// Verify the log file contains specific text matching a regular expression
        /// </summary>
        /// <param name="RegEx">regular expression to be tested for a match</param>
        /// <returns>true if there is a match</returns>
        public bool LogContainsRegExMatch(string RegEx, RegexOptions RegexOptions)
        {
            bool retVal = false;
            try
            {
                TextReader reader = new StreamReader(this.FileName);
                string logFileContent = reader.ReadToEnd();
                Match m = Regex.Match(logFileContent, RegEx, RegexOptions);
                if (m.Success)
                {
                    retVal = true;
                }
            }
            catch
            {
            }

            return retVal;
        }

        public class AppliedPackage
        {
            public int Position;
            public string PackageId;
            public string Action;
            public string Path;
            public string Arguments;
        }

        /// <summary>
        /// Parse the Applied Package data from the Burn log file
        /// </summary>
        /// <returns>orderd list of applied packages</returns>
        public List<AppliedPackage> GetAppliedPackages()
        {
            List<AppliedPackage> appliedPackages = new List<AppliedPackage>();
            char[] seperators = { '\n' };
            int n = 0;
            try
            {
                TextReader reader = new StreamReader(this.FileName);
                string logFileContent = reader.ReadToEnd();
                foreach (string logLine in logFileContent.Split(seperators))
                {
                    Match m = Regex.Match(
                        logLine,
                        ".*: Applying package: (?<packageId>.*), action: (?<action>.*), path: (?<path>.*), arguments: (?<arguments>.*)", 
                        System.Text.RegularExpressions.RegexOptions.None);
                    if (m.Success)
                    {
                        n++;
                        AppliedPackage pkg = new AppliedPackage();
                        pkg.Position = n;
                        pkg.PackageId = m.Groups["packageId"].Value;
                        pkg.Action = m.Groups["action"].Value;
                        pkg.Path = m.Groups["path"].Value;
                        pkg.Arguments = m.Groups["arguments"].Value;
                        appliedPackages.Add(pkg);
                    }
                }
            }
            catch
            {
            }

            return appliedPackages;
        }
    }
}
