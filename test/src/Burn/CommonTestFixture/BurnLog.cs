//-----------------------------------------------------------------------
// <copyright file="BurnLog.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
//    
//    The use and distribution terms for this software are covered by the
//    Common Public License 1.0 (http://opensource.org/licenses/cpl.php)
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
        
        public static List<BurnLog> GetLatestBurnLogFromEachTempFolder(string logFileNameHelper, DateTime Start, DateTime End)
        {
            List<BurnLog> burnLogs = new List<BurnLog>();

            string myTemp = System.Environment.ExpandEnvironmentVariables("%TEMP%");
            string myUsername = System.Environment.ExpandEnvironmentVariables("%USERNAME%");
            string userRoot = myTemp.Substring(0, myTemp.ToLower().IndexOf(myUsername.ToLower()));

            foreach (string directory in Directory.GetDirectories(userRoot))
            {
                string userTemp = myTemp.Replace(Path.Combine(userRoot, myUsername), directory);
                // look in each userTemp for the latest BurnLogs
                BurnLog bl = new BurnLog(logFileNameHelper, Start, End);
                bl.LogPath = userTemp;
                bl.SetBurnLogFileName();
                if (!String.IsNullOrEmpty(bl.FileName))
                {
                    burnLogs.Add(bl);
                }
            }

            return burnLogs;
        }

        public static List<BurnLog> GetAllBurnLogsFromEachUsersTempFolder(string logFileNameHelper, DateTime Start, DateTime End)
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
                    foreach (FileInfo file in dir.GetFiles(System.Environment.ExpandEnvironmentVariables(logFileNameHelper), SearchOption.AllDirectories))
                    {
                        if ((file.LastWriteTime > Start) && (file.LastWriteTime < End))
                        {
                            BurnLog bl = new BurnLog(logFileNameHelper, Start, End);
                            bl.LogPath = userTemp;
                            bl.fileName = file.FullName;
                            burnLogs.Add(bl);
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
        public string LogFileNameHelper = "*_*_*.html";

        protected string fileName;
        public string FileName
        {
            get { return fileName; }
        }

        public BurnLog()
        {
            StartTime = DateTime.MinValue;
            EndTime = DateTime.MaxValue;
            this.SetBurnLogFileName();
        }

        public BurnLog(DateTime Start)
        {
            StartTime = Start;
            EndTime = DateTime.MaxValue;
            this.SetBurnLogFileName();
        }

        public BurnLog(DateTime Start, DateTime End)
        {
            StartTime = Start;
            EndTime = End;
            this.SetBurnLogFileName();
        }

        public BurnLog(string logFileNameHelper, DateTime Start, DateTime End)
        {
            this.LogFileNameHelper = logFileNameHelper;
            StartTime = Start;
            EndTime = End;
            this.SetBurnLogFileName();
        }

        /// <summary>
        /// Get the latest log
        /// </summary>
        /// <param name="path">path of dir to search</param>
        /// <param name="searchPattern">file name search pattern</param>
        /// <param name="option">search option</param>
        /// <returns>full path to latest log file with timestamp later than the original "time"</returns>
        public string GetLatestLogFile(string path, string searchPattern, SearchOption option)
        {
            DateTime latestTime = StartTime;
            string latestFile = null;

            DirectoryInfo dir = new DirectoryInfo(System.Environment.ExpandEnvironmentVariables(path));
            try
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
            string logFile = GetLatestLogFile(this.LogPath, this.LogFileNameHelper, SearchOption.AllDirectories);
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
    }
}
