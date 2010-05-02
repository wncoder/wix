//-----------------------------------------------------------------------
// <copyright file="EngineCleanup.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>to perform clean-up after test is complete. It removes any payloads installed as a part of test</summary>
//-----------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using Microsoft.Win32;
using System.Text;
using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.Utility;
using Microsoft.Tools.WindowsInstallerXml.Test.Utilities;

namespace Microsoft.Tools.WindowsInstallerXml.Test.Burn
{
    /// <summary>
    /// Uninstalls all packages/payloads in a given layout and deletes registry keys and files that the Burn Engine could leave behind.
    /// The Burn engine is not used to perform the uninstall (in case it is broken).
    /// This is to be used by test code that needs to ensure the machine is in a clean state after a test is performed.
    /// </summary>
    public class EngineCleanup
    {
        // contains all the data to know what to delete
        private Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnSetupElement.Setup m_ParameterInfo;
        private LayoutManager.LayoutManager m_Layout;
        private string RegistrationId;
        private List<string> UpdateIds;
        #region Constructors

        /// <summary>
        /// Creates an EngineCleanup object that can be used to clean a machine of all files and regkeys a Burn bundle may have installed.
        /// </summary>
        /// <param name="Layout">Layout to be removed from the machine</param>
        public EngineCleanup(LayoutManager.LayoutManager Layout)
        {
            UpdateIds = new List<string>();
            m_Layout = Layout;
            m_ParameterInfo = Layout.ParameterInfo;
            // BUGBUG need to get the Id from a bundle where the parameterinfo.xml is dynamically generated with a unique ID
            RegistrationId = m_ParameterInfo.Registration.Id;

            try
            {
                foreach (Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.BurnManifestOM.Registration.RegistrationElement.UpdateElement updateElement in m_Layout.BurnManifest.Registration.UpdateElements)
                {
                    UpdateIds.Add(updateElement.BundleId);
                }
            }
            catch
            {
                // don't die if this isn't a Bundle that Updates other bundles.
            }
        }

        /// <summary>
        /// Creates an EngineCleanup object that can be used to clean a machine of all files and regkeys a Burn bundle may have installed.
        /// ParameterInfo should contain sufficient info to know what to remove.  But pass in a LayoutManager if you have one.
        /// </summary>
        /// <param name="ParameterInfo">ParameterInfo from the layout to be removed from the machine</param>
        public EngineCleanup(Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnSetupElement.Setup ParameterInfo)
        {
            m_ParameterInfo = ParameterInfo;
            m_Layout = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.LayoutManager.LayoutManager();
            m_Layout.ParameterInfo = m_ParameterInfo;
        }

        #endregion

        public void CleanupEverything()
        {
            UninstallAllPayloads();
            DeleteEngineRunRegKeys();
            DeleteBurnArpEntry();
            DeleteEngineDownloadCache();
            DeleteBundleExtractionTempCache();
            DeleteEngineCache();
            DeletePayloadCache();
        }

        /// <summary>
        /// Deletes this regkey if it exists (this key causes setup to resume after a reboot):
        /// HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Run  guid (where guid = what is in parameterinfo.xml for  Registration Id="{4C9C8A65-1159-414A-96D0-1815992D0A7F}")
        /// </summary>
        public void DeleteEngineRunRegKeys()
        {
            // clean up the current user
            if (!String.IsNullOrEmpty(RegistrationId))
            {
                try
                {
                    Registry.CurrentUser.OpenSubKey(@"Software\Microsoft\Windows\CurrentVersion\Run", true).DeleteValue(RegistrationId);
                }
                catch
                {
                    // don't throw if the key couldn't be deleted.  It probably didn't exist.
                }
            }

            // try to remove it for all users on the machine cause another user may have installed this layout and left this key behind
            if (!String.IsNullOrEmpty(RegistrationId))
            {
                foreach (string sid in Registry.Users.GetSubKeyNames())
                {
                    try
                    {
                        Registry.Users.OpenSubKey(sid + @"\Software\Microsoft\Windows\CurrentVersion\Run", true).DeleteValue(RegistrationId);
                    }
                    catch
                    {
                        // don't throw if the key couldn't be deleted.  It probably didn't exist.
                    }
                }
            }
        }

        /// <summary>
        /// Deletes the ARP entry (if it exists) by deleting this regkey:
        /// HKLM\SOFTWARE\Wow6432Node\Microsoft\Windows\CurrentVersion\Uninstall\{RegistrationGUID} (where RegistrationGUID = what is in parameterinfo.xml for  Registration Id="{4C9C8A65-1159-414A-96D0-1815992D0A7F}")
        /// </summary>
        public void DeleteBurnArpEntry()
        {
            if (!String.IsNullOrEmpty(RegistrationId))
            {
                string[] uninstallSubKeys = new string[] { 
                                             @"SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\", 
                                             @"SOFTWARE\Wow6432Node\Microsoft\Windows\CurrentVersion\Uninstall\" };
                foreach (string uninstallSubKey in uninstallSubKeys)
                {
                    // delete the per-machine ARP entry for this RegistrationId if it exists
                    RegistryKey rkUninstall = Registry.LocalMachine.OpenSubKey(uninstallSubKey, true);
                    if (null != rkUninstall)
                    {
                        try
                        {
                            RegistryKey rkRegistrationId = Registry.LocalMachine.OpenSubKey(uninstallSubKey + RegistrationId, true);
                            if (null != rkRegistrationId)
                            {
                                rkUninstall.DeleteSubKeyTree(RegistrationId);
                            }
                        }
                        catch
                        {
                            // don't throw if the key couldn't be deleted.  It probably didn't exist.
                        }
                        foreach (string id in UpdateIds)
                        {
                            try
                            {
                                RegistryKey rkRegistrationId = Registry.LocalMachine.OpenSubKey(uninstallSubKey + RegistrationId + "_" + id, true);
                                if (null != rkRegistrationId)
                                {
                                    rkUninstall.DeleteSubKeyTree(RegistrationId + "_" + id);
                                }
                            }
                            catch
                            {
                                // don't throw if the key couldn't be deleted.  It probably didn't exist.
                            }
                        }
                    }
                }

                // delete the per-user ARP entry(s) for this RegistrationId if they exists
                // For example:
                // HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Uninstall\{b12dfdfb-9ef3-471e-b2a0-b960cc00106c}
                // HKEY_USERS\S-1-5-21-2127521184-1604012920-1887927527-1450143\Software\Microsoft\Windows\CurrentVersion\Uninstall\{b12dfdfb-9ef3-471e-b2a0-b960cc00106c}
                foreach (string sid in Registry.Users.GetSubKeyNames())
                {
                    string uninstallSubKey = sid + @"\Software\Microsoft\Windows\CurrentVersion\Uninstall\";
                    RegistryKey rkUserUninstall = Registry.Users.OpenSubKey(uninstallSubKey, true);
                    if (null != rkUserUninstall)
                    {
                        try
                        {
                            RegistryKey rkRegistrationId = Registry.Users.OpenSubKey(uninstallSubKey + RegistrationId, true);
                            if (null != rkRegistrationId)
                            {
                                rkUserUninstall.DeleteSubKeyTree(RegistrationId);
                            }
                        }
                        catch
                        {
                            // don't throw if the key couldn't be deleted.  It probably didn't exist.
                        }
                        foreach (string id in UpdateIds)
                        {
                            try
                            {
                                RegistryKey rkRegistrationId = Registry.Users.OpenSubKey(uninstallSubKey + RegistrationId + "_" + id, true);
                                if (null != rkRegistrationId)
                                {
                                    rkUserUninstall.DeleteSubKeyTree(RegistrationId + "_" + id);
                                }
                            }
                            catch
                            {
                                // don't throw if the key couldn't be deleted.  It probably didn't exist.
                            }
                        }
                    }
                }

            }

        }

        /// <summary>
        /// Deletes %temp%\PackageName_PackageVersion (i.e. %temp%\BurnTestSKU_v0.1) and all of it's contents.
        /// This will be done for all users temp folders on the machine cause another user may have installed this layout
        /// </summary>
        public void DeleteEngineDownloadCache()
        {
            string cacheFolder;

            // delete the cache for current user (local admin)
            cacheFolder = m_Layout.GetDownloadCachePath();
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.LayoutManager.LayoutManager.RemoveDirectory(cacheFolder);

            // delete the cache for all other users if it exists
            foreach (string directory in UserUtilities.GetAllUserTempPaths())
            {
                cacheFolder = Path.Combine(directory, m_Layout.GetDownloadCacheFolderName());
                Microsoft.Tools.WindowsInstallerXml.Test.Burn.LayoutManager.LayoutManager.RemoveDirectory(cacheFolder);
            }
        }

        /// <summary>
        /// Deletes %temp%\RegistrationId (i.e. %temp%\{00000000-0000-0000-4F85-B939234AE44F}) and all of it's contents.
        /// This will be done for all users temp folders on the machine cause another user may have installed this layout
        /// </summary>
        public void DeleteBundleExtractionTempCache()
        {
            string cacheFolder;

            // delete the cache for all users if it exists
            foreach (string directory in UserUtilities.GetAllUserTempPaths())
            {
                cacheFolder = Path.Combine(directory, this.RegistrationId);
                Microsoft.Tools.WindowsInstallerXml.Test.Burn.LayoutManager.LayoutManager.RemoveDirectory(cacheFolder);
            }
        }

        /// <summary>
        /// Deletes %ProgramData%\guid and all of it's contents if it exists. (where guid = what is in parameterinfo.xml for  Registration Id="{4C9C8A65-1159-414A-96D0-1815992D0A7F}") i.e.: C:\ProgramData\{4C9C8A65-1159-414A-96D0-1815992D0A7F}\setup.exe
        /// </summary>
        public void DeleteEngineCache()
        {
            // Delete the per-machine cached engine folder if it exists
            // per-machine installs need to be deleted from %ProgramData%\\RegistrationId
            string cacheFolder = System.Environment.ExpandEnvironmentVariables("%ProgramData%\\Apps\\Cache\\" + RegistrationId);
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.LayoutManager.LayoutManager.RemoveDirectory(cacheFolder);

            // Delete the per-user cached engine folder(s) if they exist (for all users)
            // per-user installs need to be deleted from %LOCALAPPDATA%\\RegistrationId
            foreach (string directory in UserUtilities.GetAllUserLocalAppDataPaths())
            {
                // delete the cache for all the other users non-admin user (normal user)
                cacheFolder = Path.Combine(directory, "Apps\\Cache\\" + RegistrationId);
                Microsoft.Tools.WindowsInstallerXml.Test.Burn.LayoutManager.LayoutManager.RemoveDirectory(cacheFolder);

            }
        }

        /// <summary>
        /// Removes the payload cache for all items in the layout.  Be sure to uninstall all the payload before removing the payload cache as it might be needed during uninstall.
        /// </summary>
        public void DeletePayloadCache()
        {
            // Files are cached in:
            // EXE:  // %ProgramData%\Applications\Cache\filehash\filename.exe
            // MSI:  // %ProgramData%\Applications\Cache\productCode_version\filename.msi
            // MSP:  // %ProgramData%\Applications\Cache\packageCode\filename.msp

            // BUGBUG
            // for now, just delete everything under %ProgramData%\Apps\Cache
            // We may want to be more selective and just delete the relative things.  
            // Once real products ship with Burn, we could be destroying their legitimate caches

            // delete payload cache for per-machine packages
            string cacheFolder = System.Environment.ExpandEnvironmentVariables("%ProgramData%\\Apps\\Cache");
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.LayoutManager.LayoutManager.RemoveDirectory(cacheFolder);

            // delete payload cache for per-user packages (for all users)
            // per-user payloads are cached under "%LOCALAPPDATA%\Apps\Cache" for the user
            foreach (string directory in UserUtilities.GetAllUserLocalAppDataPaths())
            {
                // delete the cache for all the other users non-admin user (normal user)
                cacheFolder = Path.Combine(directory, @"Apps\Cache");
                Microsoft.Tools.WindowsInstallerXml.Test.Burn.LayoutManager.LayoutManager.RemoveDirectory(cacheFolder);
            }

        }

        /// <summary>
        /// Uninstalls all the payload items in a layout
        /// </summary>
        public void UninstallAllPayloads()
        {
            // loop thru all the items in a layout and uninstall them.
            foreach (Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnBaseItems item in m_ParameterInfo.Items.Items)
            {
                Type t = item.GetType();
                switch (t.FullName)
                {
                    case "ParameterInfoConfigurator.BurnInstallableItems.ExeItem":
                        // run the exe with the UninstallCommandLine
                        // hopefully the Exe does the right thing
                        // per-user installs that were performed by another user will probably not be removed properly.
                        string exeFile = null;
                        if (!String.IsNullOrEmpty(item.SourceFilePath))
                        {
                            exeFile = item.SourceFilePath;
                        }
                        else if (m_Layout != null &&
                            (!String.IsNullOrEmpty(m_Layout.LayoutFolder)) &&
                            (!String.IsNullOrEmpty(((Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnInstallableItems.ExeItem)item).Name)))
                        {
                            exeFile = Path.Combine(m_Layout.LayoutFolder, ((Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnInstallableItems.ExeItem)item).Name);
                        }

                        if (!String.IsNullOrEmpty(exeFile))
                        {
                            Process proc = new Process();
                            proc.StartInfo.Arguments = ((Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnInstallableItems.ExeItem)item).UninstallCommandLine;
                            proc.StartInfo.FileName = exeFile;
                            proc.Start();
                            proc.WaitForExit();
                        }
                        else
                        {
                            System.Diagnostics.Trace.TraceError("Unable to find Exe to uninstall: {0}", ((Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnInstallableItems.ExeItem)item).Name);
                        }
                        break;
                    case "ParameterInfoConfigurator.BurnInstallableItems.MsiItem":
                        // unintall the Msi from the SourceFilePath if it is set
                        // otherwise, use the Msi in the layout if it can be found
                        string msiFile = null;
                        if (!String.IsNullOrEmpty(item.SourceFilePath))
                        {
                            msiFile = item.SourceFilePath;
                        }
                        else if (m_Layout != null &&
                            (!String.IsNullOrEmpty(m_Layout.LayoutFolder)) &&
                            (!String.IsNullOrEmpty(((Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnInstallableItems.MsiItem)item).Name)))
                        {
                            msiFile = Path.Combine(m_Layout.LayoutFolder, ((Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnInstallableItems.MsiItem)item).Name);
                        }

                        if (!String.IsNullOrEmpty(msiFile))
                        {
                            MsiUtils.RemoveMSI(msiFile);
                        }
                        else
                        {
                            System.Diagnostics.Trace.TraceError("Unable to find Msi to uninstall: {0}", ((Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnInstallableItems.MsiItem)item).Name);
                        }
                        break;
                    case "ParameterInfoConfigurator.BurnInstallableItems.MspItem":
                        // run the exe with the UninstallCommandLine
                        // hopefully it does the right thing
                        //((ParameterInfoConfigurator.BurnInstallableItems.MspItem)item).PatchCode;
                        string mspFile = null;
                        if (!String.IsNullOrEmpty(item.SourceFilePath))
                        {
                            mspFile = item.SourceFilePath;
                        }
                        else if (m_Layout != null &&
                            (!String.IsNullOrEmpty(m_Layout.LayoutFolder)) &&
                            (!String.IsNullOrEmpty(((Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnInstallableItems.MspItem)item).Name)))
                        {
                            mspFile = Path.Combine(m_Layout.LayoutFolder, ((Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnInstallableItems.MspItem)item).Name);
                        }

                        if (!String.IsNullOrEmpty(mspFile))
                        {
                            MsiUtils.RemovePatchFromProducts(MsiUtils.GetPatchCode(mspFile), MsiUtils.GetTargetProductCodes(mspFile));
                        }
                        else
                        {
                            System.Diagnostics.Trace.TraceError("Unable to find Msp file to uninstall: {0}", ((Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnInstallableItems.MspItem)item).Name);
                        }
                        break;
                    // BUGBUG TODO: handle Item Groups (i.e. Patches) once support for testing those is enabled in the LayoutManager
                    default:
                        System.Diagnostics.Trace.TraceError("Unknown item type to uninstall.  type = {0}", t.FullName);
                        break;
                }
            }
        }
    }
}
