//-----------------------------------------------------------------------
// <copyright file="StartBurnstub.cs" company="Microsoft">
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
//     - Contains methods used for starting a Burnstub.exe process.
//     - Can launch as the current (admin) user or as a normal user (using a local test account)
// </summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests.Burn.CommonTestFixture
{
    using System;
    using System.Diagnostics;
    using System.IO;
    using Microsoft.Tools.WindowsInstallerXml.Test.Utilities;
    using Microsoft.Tools.WindowsInstallerXml.Test.Verifiers.Extensions;
    using Microsoft.Win32;

    // BUGBUG: Probably should move this to WixTestTools.  
    // But since this is a little different than other tools, I'm putting it here for now.  
    // Notably, this just launches a process, it doesn't wait for it.
    // Also, it can run as a differnt user (i.e. for testing normal-user vs elevated/admin).  
    // Works quite different than how all other wix build tools work.

    /// <summary>
    /// Class for starting Burn processes
    /// </summary>
    public class BurnstubLauncher
    {
        public const String NormalUserName = "NonAdminTestAcct";
        public const String NormalUserPassword = "Password!123";
        private const String GroupName = "Users";

        public struct RunAsUser
        {
            public static string UserName;
            public static string Password;
        }

        public BurnstubLauncher()
        {
            if (!UserVerifier.UserExists("", NormalUserName))
            {
                UserVerifier.CreateLocalUser(NormalUserName, NormalUserPassword);
                UserVerifier.AddUserToGroup(NormalUserName, GroupName);
            }
            RunAsUser.UserName = null;
            RunAsUser.Password = null;
        }

        private bool m_UseNormalUser;
        public bool UseNormalUser
        {
            get
            {
                return m_UseNormalUser;
            }
            set
            {
                m_UseNormalUser = value;
                if (m_UseNormalUser)
                {
                    RunAsUser.UserName = NormalUserName;
                    RunAsUser.Password = NormalUserPassword;
                }
            }
        }

        /// <summary>
        /// Creates a Burn process.  It will launch BurnSetupBundle.exe
        /// </summary>
        /// <param name="layout">layout to start</param>
        /// <param name="args">args to pass to the Burn chainer</param>
        /// <returns>Burn Process object</returns>
        public Process StartProcess(Microsoft.Tools.WindowsInstallerXml.Test.Burn.LayoutManager.LayoutManager layout, string args)
        {
            string exe = Path.Combine(layout.LayoutFolder, layout.SetupBundleFilename);
            
            Process proc = new Process();
            proc.StartInfo.Arguments = args;
            proc.StartInfo.FileName = exe;

            if (!String.IsNullOrEmpty(RunAsUser.UserName) && !String.IsNullOrEmpty(RunAsUser.Password))
            {
                proc.StartInfo.UserName = RunAsUser.UserName;
                proc.StartInfo.Password = new System.Security.SecureString();
                proc.StartInfo.Password.Clear(); // Make sure there is no junk
                foreach (char c in RunAsUser.Password.ToCharArray())
                {
                    proc.StartInfo.Password.AppendChar(c);
                }
                proc.StartInfo.Domain = Environment.MachineName.ToString();
                proc.StartInfo.UseShellExecute = false;
                proc.StartInfo.LoadUserProfile = true;
            }
            Console.WriteLine("Starting process FileName:\"{0}\" Arguments:\"{1}\" UserName:\"{2}\" WorkingDirectory:\"{3}\"", proc.StartInfo.FileName, proc.StartInfo.Arguments, proc.StartInfo.UserName, proc.StartInfo.WorkingDirectory);
            proc.Start();

            return proc;
        }

        /// <summary>
        /// Creates a Burn process.  It will launch the exe from the ARP entry.
        /// </summary>
        /// <param name="layout">layout to start (used to find the appropriate ARP entry)</param>
        /// <param name="args">additional args to pass to the Burn chainer</param>
        /// <returns>Burn Process object</returns>
        public Process StartArpProcess(Microsoft.Tools.WindowsInstallerXml.Test.Burn.LayoutManager.LayoutManager layout, string additionalArgs)
        {
            string exe = null; // Path.Combine(layout.LayoutFolder, layout.SetupExeFilename);

            string registrationId = layout.ActualBundleId;
            RegistryKey rkRegistrationId = null;

            if (String.IsNullOrEmpty(registrationId))
            {
                throw new Exception("No RegistrationId in layout.  Don't kow which ARP entry to start");
            }


            // get the path to the exe and args from the ARP entry
            if (UseNormalUser || !layout.Wix.Bundle.PerMachineT)

            {
                // Look in the USERS hive for this per-user ARP entry
                int found = 0;
                foreach (string sid in Registry.Users.GetSubKeyNames())
                {
                    string uninstallSubKey = sid + @"\Software\Microsoft\Windows\CurrentVersion\Uninstall\";
                    RegistryKey rkUserUninstall = Registry.Users.OpenSubKey(uninstallSubKey, true);
                    if (null != rkUserUninstall)
                    {
                        RegistryKey rkTempRegistrationId = Registry.Users.OpenSubKey(uninstallSubKey + registrationId, true);
                        if (null != rkTempRegistrationId)
                        {
                            found++;
                            rkRegistrationId = rkTempRegistrationId;
                        }
                    }
                }
                if (found > 1)
                {
                    // More that one ARP entry was found for this per-user bundle.
                    // We'll just use the last one found.
                }
            }
            else
            {
                // Look in the HKLM hive for this per-machine ARP entry
                string[] uninstallSubKeys = new string[] { 
                                             @"SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\", 
                                             @"SOFTWARE\Wow6432Node\Microsoft\Windows\CurrentVersion\Uninstall\" };

                foreach (string uninstallSubKey in uninstallSubKeys)
                {
                    // find the per-machine ARP entry for this RegistrationId if it exists
                    RegistryKey rkUninstall = Registry.LocalMachine.OpenSubKey(uninstallSubKey, true);
                    if (null != rkUninstall)
                    {
                        RegistryKey rkTempRegistrationId = Registry.LocalMachine.OpenSubKey(uninstallSubKey + registrationId, true);
                        if (null != rkTempRegistrationId)
                        {
                            rkRegistrationId = rkTempRegistrationId;
                        }

                    }
                }
            }

            if (rkRegistrationId == null)
            {
                throw new Exception("No Uninstall reg key for RegistrationId found.  Unable to start ARP entry.");
            }

            string t = (string)rkRegistrationId.GetValue("UninstallString");
            exe = t.Substring(0, t.IndexOf('/'));
            string arpArgs = t.Replace(exe, "");
            exe = exe.Replace("\"", "");


            // TODO, should verify DisplayName, DisplayIcon in the ARP entry
            // What do NoElevateOnModify, Resume do?
            // When are these called QuietUninstallString, UninstallString (i.e. what calls quiet?)
            // ModifyPath
            // Why no repair? MSI & ARP supports that too.  Though change can handle it in UI mode, a silent repair might be something we need to support from ARP.
            // What about: publisher, InstallSource, ProductId, RegOwner, RegCompany, HelpLing, HelpTelephone, URLUpdateInfo, URLInfoAbout, DisplayVersion, VersionMajor, VersionMinor
            // Are these supported? NoModify, NoRepair.  if so need to test them too.


            Process proc = new Process();
            proc.StartInfo.Arguments = arpArgs + " " + additionalArgs;
            proc.StartInfo.FileName = exe;

            if (!String.IsNullOrEmpty(RunAsUser.UserName) && !String.IsNullOrEmpty(RunAsUser.Password))
            {
                proc.StartInfo.UserName = RunAsUser.UserName;
                proc.StartInfo.Password = new System.Security.SecureString();
                proc.StartInfo.Password.Clear(); // Make sure there is no junk
                foreach (char c in RunAsUser.Password.ToCharArray())
                {
                    proc.StartInfo.Password.AppendChar(c);
                }
                proc.StartInfo.Domain = Environment.MachineName.ToString();
                proc.StartInfo.UseShellExecute = false;
                proc.StartInfo.LoadUserProfile = true;
            }

            Console.WriteLine("Starting process FileName:\"{0}\" Arguments:\"{1}\" UserName:\"{2}\" WorkingDirectory:\"{3}\"", proc.StartInfo.FileName, proc.StartInfo.Arguments, proc.StartInfo.UserName, proc.StartInfo.WorkingDirectory);
            proc.Start();

            return proc;
        }
    }
}
