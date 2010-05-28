//-----------------------------------------------------------------------
// <copyright file="LayoutManager.cs" company="Microsoft">
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
// <summary>To create burn test layout</summary>
//-----------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text;
using System.Xml;
using Microsoft.Deployment.Compression.Cab;
using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.BurnManifestOM;
using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.BurnManifestOM.Chain;
using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.BurnManifestOM.Variables;

namespace Microsoft.Tools.WindowsInstallerXml.Test.Burn.LayoutManager
{
    public partial class LayoutManager
    {
        #region private member variables

        private Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnSetupElement.Setup m_ParameterInfo;
        private BurnManifestElement m_BurnManifest;
        private string m_LayoutFolder;
        private string m_SetupExeFilename;
        private string m_SetupBundleFilename;
        private Ux.UxBase m_Ux;

        #endregion

        #region public Properties

        public Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnSetupElement.Setup ParameterInfo
        {
            get
            {
                return m_ParameterInfo;
            }
            set
            {
                m_ParameterInfo = value;
            }
        }

        public BurnManifestElement BurnManifest
        {
            get
            {
                return m_BurnManifest;
            }
            set
            {
                m_BurnManifest = value;
            }
        }

        public string LayoutFolder
        {
            get
            {
                return m_LayoutFolder;
            }
            set
            {
                m_LayoutFolder = value;
            }
        }

        public string SetupExeFilename
        {
            get
            {
                if (String.IsNullOrEmpty(m_SetupExeFilename))
                {
                    m_SetupExeFilename = "Setup.exe";
                }
                return m_SetupExeFilename;
            }
            set
            {
                m_SetupExeFilename = value;
            }
        }

        public string SetupBundleFilename
        {
            get
            {
                if (String.IsNullOrEmpty(m_SetupBundleFilename))
                {
                    m_SetupBundleFilename = "BurnSetupBundle.exe";
                }
                return m_SetupBundleFilename;
            }
            set
            {
                m_SetupBundleFilename = value;
            }
        }

        public Ux.UxBase Ux
        {
            get
            {
                return m_Ux;
            }
            set
            {
                m_Ux = value;
            }
        }

        #endregion

        #region constructors

        public LayoutManager()
        {
            initializeProperties(null, null);
        }

        public LayoutManager(string layoutFolder)
        {
            initializeProperties(layoutFolder, null);
        }

        public LayoutManager(Ux.UxBase ux)
        {
            initializeProperties(null, ux);
        }

        public LayoutManager(string layoutFolder, Ux.UxBase ux)
        {
            initializeProperties(layoutFolder, ux);
        }

        private void initializeProperties(string layoutFolder, Ux.UxBase ux)
        {
            if (String.IsNullOrEmpty(layoutFolder))
            {
                LayoutFolder = System.Environment.ExpandEnvironmentVariables("%WIX_ROOT%\\test\\sandbox");
            }
            else
            {
                LayoutFolder = System.Environment.ExpandEnvironmentVariables(layoutFolder);
            }

            if (ux != null)
            {
                this.Ux = ux;
            }
            
            CreateAndInitializeParameterInfoConfiguratorWithMinimumDefaults();
            CreateAndInitializeBurnManifestWithMinimumDefaults();
        }

        #endregion

        public void CreateAndInitializeParameterInfoConfiguratorWithMinimumDefaults()
        {
            ParameterInfo = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnSetupElement.Setup(true);
            ParameterInfo.Configuration = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnConfigurationElement.BurnConfiguration();
            ParameterInfo.Configuration.UserExperienceDataCollection = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnConfigurationElement.BurnConfiguration.BurnUserExperienceDataCollection();
            ParameterInfo.Configuration.UserExperienceDataCollection.Policy = Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnConfigurationElement.BurnConfiguration.BurnUserExperienceDataCollection.UxPolicy.Disabled; // BUGBUG put this back to AlwaysUploaded once we have a null MetricLoader to use for testing
            ParameterInfo.Configuration.DownloadInstallSetting = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnConfigurationElement.BurnConfiguration.BurnDownloadInstallSetting();
            ParameterInfo.Configuration.DownloadInstallSetting.SerialDownload = false;
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnConfigurationElement.BurnConfiguration.BurnAcceptableCertificates.BurnCertificate cert1 = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnConfigurationElement.BurnConfiguration.BurnAcceptableCertificates.BurnCertificate();
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnConfigurationElement.BurnConfiguration.BurnAcceptableCertificates.BurnCertificate cert2 = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnConfigurationElement.BurnConfiguration.BurnAcceptableCertificates.BurnCertificate();
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnConfigurationElement.BurnConfiguration.BurnAcceptableCertificates.BurnCertificate cert3 = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnConfigurationElement.BurnConfiguration.BurnAcceptableCertificates.BurnCertificate();
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnConfigurationElement.BurnConfiguration.BurnAcceptableCertificates.BurnCertificate cert4 = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnConfigurationElement.BurnConfiguration.BurnAcceptableCertificates.BurnCertificate();
            cert1.Name = "Microsoft Root";
            cert1.AuthorityKeyIdentifier = "4A5C7522AA46BFA4089D39974EBDB4A360F7A01D";
            cert2.Name = "Microsoft Root Certificate Authority";
            cert2.AuthorityKeyIdentifier = "0EAC826040562797E52513FC2AE10A539559E4A4";
            cert3.Name = "Microsoft Test Root Authority";
            cert3.AuthorityKeyIdentifier = "22CD37F1B14750AE537C8C6A036747E2B71E17B7";
            cert4.Name = "Dotfuscator from Preemptive Solutions";
            cert4.AuthorityKeyIdentifier = "DAED6474149C143CABDD99A9BD5B284D8B3CC9D8";
            cert4.Thumbprint = "54E57DC08F1298601A22D6AC5278D472D3BACB56";
            ParameterInfo.Configuration.AcceptableCertificates = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnConfigurationElement.BurnConfiguration.BurnAcceptableCertificates();
            ParameterInfo.Configuration.AcceptableCertificates.Certificates.Add(cert1);
            ParameterInfo.Configuration.AcceptableCertificates.Certificates.Add(cert2);
            ParameterInfo.Configuration.AcceptableCertificates.Certificates.Add(cert3);
            ParameterInfo.Configuration.AcceptableCertificates.Certificates.Add(cert4);

            ParameterInfo.Configuration.BlockingMutex = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnConfigurationElement.BurnConfiguration.BurnBlockingMutex();
            ParameterInfo.Configuration.BlockingMutex.Name = "BurnTestBundleMutex";

            ParameterInfo.Registration = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnRegistrationElement.BurnRegistrationElement();
            ParameterInfo.Registration.Id = "{4C9C8A65-1159-414A-96D0-1815992D0A7F}";
            ParameterInfo.Registration.ExecutableName = "setup.exe";
            ParameterInfo.Registration.PerMachine = "yes";
            ParameterInfo.Registration.Family = "BurnTestFamily";
            ParameterInfo.Registration.Arp = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnRegistrationElement.BurnRegistrationElement.BurnArpElement();
            ParameterInfo.Registration.Arp.DisplayName = "BurnTestSKU";
            ParameterInfo.Registration.Arp.DisplayVersion = "1.0.0.0";

            ParameterInfo.UI.Name = "BurnTestSKU";
            ParameterInfo.UI.Version = "v0.1";
            ParameterInfo.Ux = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnUxElement.BurnUxElement();
            if (this.Ux != null)
            {
                ParameterInfo.UI.Dll = this.Ux.UxBinaryFilename;
                ParameterInfo.Ux.UxDllPayloadId = this.Ux.UxBinaryFilename.Replace(".", "_");
                Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnUxElement.BurnUxElement.BurnPayload payload = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnUxElement.BurnUxElement.BurnPayload();
                payload.Id = ParameterInfo.Ux.UxDllPayloadId;
                payload.FilePath = this.Ux.UxBinaryFilename;
                payload.Packaging = "embedded";  // BUGBUG what should this really be?
                payload.SourcePath = this.Ux.UxBinaryFilename;
                ParameterInfo.Ux.Payloads.Add(payload);
            }
            else
            {
                ParameterInfo.UI.Dll = "unknown";
            }
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnOperands.RegKeyValue regKeyVal = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnOperands.RegKeyValue("HKEY_CURRENT_USER\\Software\\Wix\\Burn\\Setup\\Repair");
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.Equals equalsExpression = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.Equals("1",
                Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnExpressions.ExpressionAttribute.BoolWhenNonExistentType.False,
                regKeyVal);
            ParameterInfo.EnterMaintenanceModeIf.Expression = equalsExpression;
            ParameterInfo.SystemCheck = null;
            ParameterInfo.Blockers = null;
        }

        public void CreateAndInitializeBurnManifestWithMinimumDefaults()
        {
            BurnManifest = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.BurnManifestOM.BurnManifestElement();
            BurnManifest.Xmlns = "http://schemas.microsoft.com/wix/2008/Burn";
            BurnManifest.Stub = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.BurnManifestOM.Stub.StubElement();
            BurnManifest.Stub.SourceFile = "burnstub.exe";
            if (null != this.Ux) BurnManifest.UX = this.Ux.GetBurnManifestUxElement();
            BurnManifest.Registration = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.BurnManifestOM.Registration.RegistrationElement();
            BurnManifest.Registration.Family = "Burn_Chainer_Test_Family";
            BurnManifest.Registration.Arp = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.BurnManifestOM.Registration.RegistrationElement.ArpElement();
            BurnManifest.Registration.Arp.Name = "Burn Chainer Test";
            BurnManifest.Registration.Arp.Version = "1.0.0.0";
            BurnManifest.Registration.Arp.Publisher = "BurnPublisher";
            BurnManifest.Registration.Arp.HelpLink = "http://burn/help.html";
            BurnManifest.Registration.Arp.HelpTelephone = "555-5555";
            BurnManifest.Registration.Arp.AboutUrl = "http://burn/about.html";
            BurnManifest.Registration.Arp.UpdateUrl = "http://burn/update.html";
            BurnManifest.Registration.Arp.DisableModify = "no";
            BurnManifest.Registration.Arp.DisableRepair = "no";
            BurnManifest.Registration.Arp.DisableRemove = "no";
            BurnManifest.Chain = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.BurnManifestOM.Chain.ChainElement();
        }

        /// <summary>
        /// Creates a burn loose file layout (copies engine and UX files, configures parameterinfo.xml, etc.)
        /// The layout's parameterinfo.xml is based on the data in the ParameterInfo object.
        /// Payload files are not copied.  You must copy them yourself or use the Add* methods here which will 
        /// copy the file and update the ParameterInfo object appropriately.
        /// </summary>
        public void GenerateLayout()
        {
            string xmlContent = Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.ParameterInfoConfiguratorEngine.XMLGenerator.GetXmlString(ParameterInfo);
            string parameterInfoFile = Path.Combine(this.LayoutFolder, "ParameterInfo.xml");

            // write the xml to Parameterinfo.xml
            XmlDocument xmlDoc = new XmlDocument();
            xmlDoc.LoadXml(xmlContent);
            //Save the parameterinfo.xml in Unicode.  Burn requires unicode, not ANSI.
            XmlTextWriter wrtr = new XmlTextWriter(parameterInfoFile, Encoding.Unicode);
            xmlDoc.WriteTo(wrtr);
            wrtr.Close();

            CopyEngineAndUxFiles();
        }

        private void CopyEngineAndUxFiles()
        {
            // copy all of the Burn engine files
            string srcBinDir = Microsoft.Tools.WindowsInstallerXml.Test.Settings.WixToolDirectory;
            string srcBurnEngineFile = Path.Combine(srcBinDir, "burnstub.exe");
            string destBurnEngineFile = Path.Combine(this.LayoutFolder, this.SetupExeFilename);
            CopyFile(srcBurnEngineFile, destBurnEngineFile);

            string srcDataDir = System.Environment.ExpandEnvironmentVariables("%WIX_ROOT%\\src\\burn\\core\\schema");
            string srcBurnDhtmlHeaderFile = Path.Combine(srcDataDir, "DHtmlHeader.html");
            string destBurnDhtmlHeaderFile = Path.Combine(this.LayoutFolder, "DHtmlHeader.html");
            CopyFile(srcBurnDhtmlHeaderFile, destBurnDhtmlHeaderFile);

            // BUGBUG this can be removed once the Burn engine doesn't require this.  It loads it but doesn't use it at this time.  The UX will own localized resources.
            string srcBurnLocalizedDataFile = Path.Combine(srcDataDir, "LocalizedData.xml");
            string destBurnLocalizedDataFile = Path.Combine(Path.Combine(this.LayoutFolder, "1033"), "LocalizedData.xml");
            CopyFile(srcBurnLocalizedDataFile, destBurnLocalizedDataFile);

            // TODO: Figure out if we should copy the PDBs too.  Might be nice to have them in the layout...

            // copy the Burn UX files.  
            if (null != this.Ux) this.Ux.CopyAndConfigureUx(this.LayoutFolder);

        }

        /// <summary>
        /// Creates a burn bundle layout by running the burn.exe build tool.
        /// Dynamically generates parameterinfo.xml from BurnManifest, cabs and attaches the UX files, parameterinfo.xml, embedded payloads in an attached container.
        /// </summary>
        public void GenerateBundle()
        {
            GenerateBundle(true);
        }

        /// <summary>
        /// Creates a burn bundle layout by running the burn.exe build tool.
        /// Dynamically generates parameterinfo.xml from BurnManifest, cabs and attaches the UX files, parameterinfo.xml, embedded payloads in an attached container.
        /// </summary>
        /// <param name="removeAllOtherFiles">determines if everything except the bundle is deleted from the bundle folder after the bundle is created.  It is a good idea to delete it if you want to verify stuff isn't picked up from this folder and is downloaded.</param>
        public void GenerateBundle(bool removeAllOtherFiles)
        {
            string xmlManifestContent = Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.ParameterInfoConfiguratorEngine.XMLGenerator.GetXmlString(BurnManifest);

            string burnManifestFile = Path.Combine(this.LayoutFolder, "burnManifest.xml");
            if (!Directory.Exists(this.LayoutFolder))
            {
                Directory.CreateDirectory(this.LayoutFolder);
            }
            // write the xml to burnManifest.xml
            XmlDocument xmlDoc = new XmlDocument();
            xmlDoc.LoadXml(xmlManifestContent);
            //Save the burnManifest.xml in Encoding.UTF8 since that is what the xml says is the encoding.
            XmlTextWriter wrtr = new XmlTextWriter(burnManifestFile, Encoding.UTF8);
            xmlDoc.WriteTo(wrtr);
            wrtr.Close();

            CopyEngineAndUxFiles();

            //// Run burn.exe to build a bundle
            //// %WIX_ROOT%\build\%BurnFlavour%\x86\burn.exe burnManifestFile -b %WIX_ROOT%\build\%BurnFlavour%\x86 -out %WIX_ROOT%\src\burn\demo\Bundle\BurnSetup.exe
            Microsoft.Tools.WindowsInstallerXml.Test.BurnExe burn = new Microsoft.Tools.WindowsInstallerXml.Test.BurnExe(this.LayoutFolder);  // be sure the working directory is the layout folder
            string outputFile = Path.Combine(this.LayoutFolder, this.SetupBundleFilename);
            if (File.Exists(outputFile))
            {
                File.Delete(outputFile);  // remove it so if the burn.exe fails, we don't acidentally have an old one laying around.
            }
            string burnArgs = string.Format("{0} -b {1} -out {2}", burnManifestFile, Settings.WixToolDirectory, outputFile);
            burn.Run(burnArgs);

            // TODO This should be replaced with a 'de-serialized' object of the entire parameterinfo.xml.  'de-serialization' isn't working 100% correctly though.
            this.ParameterInfo.Registration.Id = GetRegistrationId();
            this.ParameterInfo.Registration.PerMachine = GetRegistrationPerMachine();

            if (removeAllOtherFiles)
            {
                List<string> bundleFilesToKeep = new List<string>();
                bundleFilesToKeep.Add(Path.Combine(this.LayoutFolder, this.SetupBundleFilename));
                // BUGBUG TODO: read thru the burn manifest and keep all the "external" files too.  This could be UX resources files and/or Payload files and Payload resources in the Chain

                foreach (string file in Directory.GetFiles(this.LayoutFolder, "*", SearchOption.AllDirectories))
                {
                    if (!bundleFilesToKeep.Contains(file))
                    {
                        File.SetAttributes(file, System.IO.FileAttributes.Normal);
                        File.Delete(file);
                    }
                }

            }

        }

        private string GetRegistrationId()
        {
            string retVal = null;

            ExtractEmbeddedManifest();

            string extParameterinfo = Path.Combine(this.LayoutFolder, "my_extracted_parameterinfo.xml");

            XmlDocument xmlDoc2 = new XmlDocument();
            xmlDoc2.Load(extParameterinfo);
            string xpath2 = @"//burncore:Registration";

            XmlNamespaceManager xmlNamespaceManager = new XmlNamespaceManager(new NameTable());
            xmlNamespaceManager.AddNamespace("burn", "http://schemas.microsoft.com/wix/2008/Burn");
            xmlNamespaceManager.AddNamespace("burncore", "http://schemas.microsoft.com/Setup/2008/01/im");

            XmlNode Datapoint2 = xmlDoc2.DocumentElement.SelectSingleNode(xpath2, xmlNamespaceManager);

            retVal = Datapoint2.Attributes["Id"].Value;

            return retVal;
        }

        private string GetRegistrationPerMachine()
        {
            string retVal = null;

            ExtractEmbeddedManifest();

            string extParameterinfo = Path.Combine(this.LayoutFolder, "my_extracted_parameterinfo.xml");

            XmlDocument xmlDoc2 = new XmlDocument();
            xmlDoc2.Load(extParameterinfo);
            string xpath2 = @"//burncore:Registration";

            XmlNamespaceManager xmlNamespaceManager = new XmlNamespaceManager(new NameTable());
            xmlNamespaceManager.AddNamespace("burn", "http://schemas.microsoft.com/wix/2008/Burn");
            xmlNamespaceManager.AddNamespace("burncore", "http://schemas.microsoft.com/Setup/2008/01/im");

            XmlNode Datapoint2 = xmlDoc2.DocumentElement.SelectSingleNode(xpath2, xmlNamespaceManager);

            retVal = Datapoint2.Attributes["PerMachine"].Value;

            return retVal;
        }

        private void ExtractEmbeddedManifest()
        {
            try
            {
                // Run burn.exe to extract the UX cab from a bundle (the UX cab includes parameterinfo.xml) 
                //   burn.exe setup_to_extract.exe -x ux -o my_ux_container.cab
                Microsoft.Tools.WindowsInstallerXml.Test.BurnExe burn = new Microsoft.Tools.WindowsInstallerXml.Test.BurnExe(this.LayoutFolder);  // be sure the working directory is the layout folder
                string burnArgs = Path.Combine(this.LayoutFolder, this.SetupBundleFilename) + " -x ux -o my_ux_container.cab";
                burn.Run(burnArgs);

                // Now extract the parameterinfo.xml from the cab file that was extracted above

                CabInfo ci = new CabInfo(Path.Combine(this.LayoutFolder, "my_ux_container.cab"));

                // since the filenames in the cab aren't exactly what they were originally, we have to map them from data in the burn manifest. 
                // extract the burn manifest so we can map which file in the cab is really parameterinfo.xml
                string extManifest = Path.Combine(this.LayoutFolder, "my_extracted_manifest.xml");
                ci.UnpackFile("0", extManifest);  // The burn manifest is always a file in the cab named "0"
                XmlNamespaceManager xmlNamespaceManager = new XmlNamespaceManager(new NameTable());
                xmlNamespaceManager.AddNamespace("burn", "http://schemas.microsoft.com/wix/2008/Burn");
                xmlNamespaceManager.AddNamespace("burncore", "http://schemas.microsoft.com/Setup/2008/01/im");
                XmlDocument xmlDoc = new XmlDocument();
                xmlDoc.Load(extManifest);
                string xpath1 = @"//burn:UX/burn:Resource[@FileName='ParameterInfo.xml']";
                XmlNode Datapoint = xmlDoc.DocumentElement.SelectSingleNode(xpath1, xmlNamespaceManager);
                string idParameterinfo = Datapoint.Attributes["EmbeddedId"].Value;  // this is the real filename of parameterinfo.xml in the cab

                // get the bundle Id from parameterinfo's <Registration Id=''> attribute

                // extract parameterinfo.xml from cab
                string extParameterinfo = Path.Combine(this.LayoutFolder, "my_extracted_parameterinfo.xml");
                ci.UnpackFile(idParameterinfo, extParameterinfo);
            }
            catch
            {
                // If anything goes wrong with the extraction of the Bundle Id, don't throw.
                // However, some cleanup might not work.
            }

        }

        public static void CopyFile(string srcFile, string destFile)
        {
            string destDir = Path.GetDirectoryName(destFile);

            if (!Directory.Exists(destDir))
            {
                Directory.CreateDirectory(destDir);
            }

            if (File.Exists(destFile))
            {
                // remove read-only attributes from files that exist so they can be over-written.
                File.SetAttributes(destFile, System.IO.FileAttributes.Normal);
            }

            File.Copy(srcFile, destFile, true);
        }

        public static void RemoveFile(string file)
        {
            if (File.Exists(file))
            {
                // remove read-only attributes from files that exist so they can be deleted.
                File.SetAttributes(file, System.IO.FileAttributes.Normal);

                File.Delete(file);
            }
        }

        public static void RemoveDirectory(string directory)
        {
            if (Directory.Exists(directory))
            {
                // remove read-only attributes from all files that exist so they can be deleted.
                foreach (string file in Directory.GetFiles(directory, "*", SearchOption.AllDirectories))
                {
                    File.SetAttributes(file, System.IO.FileAttributes.Normal);
                }
                Directory.Delete(directory, true);
            }
        }

        #region methods for adding Files, EXEs, MSIs, MSPs

        public void AddFile(string file, bool includeInLayout)
        {
            AddFile(file, null, null, includeInLayout);
        }

        public void AddFile(string file, string newFileName, string url, bool includeInLayout)
        {
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnNonInstallableItems.FileItem item = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnNonInstallableItems.FileItem();
            item.GenerateDefaults(file);
            if (!String.IsNullOrEmpty(newFileName))
            {
                item.Name = newFileName;
                item.Id = item.Name;
            }
            item.Id = item.Id.Replace("\\", "-");
            item.URL = url;
            ParameterInfo.Items.Items.Add(item);

            // BUGBUG handle this in a Burn Manifest too.

            CopyPayloadToLayout(file, newFileName, includeInLayout);
        }

        public void AddMsi(string file, bool includeInLayout)
        {
            AddMsi(file, null, null, includeInLayout, string.Empty, string.Empty, string.Empty, string.Empty);
        }

        public void AddMsi(string file, string newFileName, string url, bool includeInLayout)
        {
            AddMsi(file, newFileName, url, includeInLayout, string.Empty, string.Empty, string.Empty, string.Empty);
        }

        public void AddMsi(string file, string newFileName, string url, bool includeInLayout, string installCondition, string rollbackInstallCondition)
        {
            AddMsi(file, newFileName, url, includeInLayout, string.Empty, string.Empty, installCondition, rollbackInstallCondition);
        }

        public void AddMsi(string file, string newFileName, string url, bool includeInLayout, string msiPropertyId, string msiPropertyValue
            , string installCondition, string rollbackInstallCondition)
        {
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnInstallableItems.MsiItem item = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnInstallableItems.MsiItem();
            item.GenerateDefaults(file);
            if (!String.IsNullOrEmpty(newFileName))
            {
                item.Name = newFileName;
                item.Id = item.Name;
            }
            item.Id = item.Id.Replace("\\", "-");
            item.URL = url;
            ParameterInfo.Items.Items.Add(item);            

            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.BurnManifestOM.Chain.MsiPackageElement msi = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.BurnManifestOM.Chain.MsiPackageElement();
            msi.Id = "Id_" + item.Id;
            msi.Vital = "no";
            msi.Packaging = "download";
            if (includeInLayout) msi.Packaging = "embedded";
            msi.SourceFile = item.Name;
            msi.FileName = item.Name;
            msi.DownloadUrl = item.URL;

            if (!string.IsNullOrEmpty(msiPropertyId) && !string.IsNullOrEmpty(msiPropertyValue))
            {
                MsiPropertyElement msiProperty = new MsiPropertyElement();
                msiProperty.Id = msiPropertyId;
                msiProperty.Value = msiPropertyValue;

                msi.MsiProperty = msiProperty;
            }

            if (!string.IsNullOrEmpty(installCondition))
            {
                msi.InstallCondition = installCondition;                
            }

            if (!string.IsNullOrEmpty(rollbackInstallCondition))
            {
                msi.RollbackInstallCondition = rollbackInstallCondition;
            }

            BurnManifest.Chain.Packages.Add(msi);

            CopyPayloadToLayout(file, newFileName, includeInLayout);
        }

        public void AddVariable(string id, string value, VariableElement.VariableDataType type)
        {
            VariableElement varElement = new VariableElement();
            varElement.Id = id;
            varElement.Value = value;
            varElement.Type = type;

            BurnManifest.Variables.Add(varElement);
        }

        public class ExternalFile
        {
            public string File;
            public string Url;
        }

        public void AddMsiAndExternalFiles(string msiFile, string newMsiFileName, string url, bool includeInLayout, List<ExternalFile> extFiles)
        {
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnInstallableItems.MsiItem item = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnInstallableItems.MsiItem();
            item.GenerateDefaults(msiFile);
            if (!String.IsNullOrEmpty(newMsiFileName))
            {
                item.Name = newMsiFileName;
                item.Id = item.Name;
            }
            item.Id = item.Id.Replace("\\", "-");
            item.URL = url;

            foreach (ExternalFile extFile in extFiles)
            {
                AddSubFile(item, extFile.File, null, extFile.Url, includeInLayout);
            }
            ParameterInfo.Items.Items.Add(item);

            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.BurnManifestOM.Chain.MsiPackageElement msi = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.BurnManifestOM.Chain.MsiPackageElement();
            msi.Id = "Id_" + item.Id;
            msi.Vital = "no";
            msi.Packaging = "download";
            if (includeInLayout) msi.Packaging = "embedded";
            msi.SourceFile = item.Name;
            msi.FileName = item.Name;
            msi.DownloadUrl = item.URL;
            BurnManifest.Chain.Packages.Add(msi);

            CopyPayloadToLayout(msiFile, newMsiFileName, includeInLayout);
        }

        public void AddMsp(string file, bool includeInLayout)
        {
            AddMsp(file, null, null, includeInLayout);
        }

        public void AddMsp(string file, string newFileName, string url, bool includeInLayout)
        {
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnInstallableItems.MspItem item = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnInstallableItems.MspItem();
            item.GenerateDefaults(file);
            if (!String.IsNullOrEmpty(newFileName))
            {
                item.Name = newFileName;
                item.Id = item.Name;
            }
            item.Id = item.Id.Replace("\\", "-");
            item.URL = url;
            ParameterInfo.Items.Items.Add(item);

            // BUGBUG handle this in a Burn Manifest too.

            CopyPayloadToLayout(file, newFileName, includeInLayout);
        }

        public void AddExe(string file, bool includeInLayout)
        {
            AddExe(file, null, null, includeInLayout, string.Empty, string.Empty, string.Empty);
        }

        public void AddExe(string file, string newFileName, string url, bool includeInLayout)
        {
            AddExe(file, newFileName, url, includeInLayout, string.Empty, string.Empty, string.Empty);
        }

        public void AddExe(string file, string newFileName, string url, bool includeInLayout, string installCondition, string rollbackInstallCondition
            , string installArguments)
        {
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnInstallableItems.ExeItem item = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnInstallableItems.ExeItem();
            item.GenerateDefaults(file);
            if (!String.IsNullOrEmpty(newFileName))
            {
                item.Name = newFileName;
                item.Id = item.Name;
            }
            item.Id = item.Id.Replace("\\", "-");
            item.URL = url;
            ParameterInfo.Items.Items.Add(item);

            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.BurnManifestOM.Chain.ExePackageElement exe = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.BurnManifestOM.Chain.ExePackageElement();
            exe.Id = "Id_" + item.Id;
            exe.Vital = "no";
            exe.Packaging = "download";
            if (includeInLayout) exe.Packaging = "embedded";
            exe.SourceFile = item.Name;
            exe.FileName = item.Name;
            exe.PerMachine = "yes";
            exe.Cache = "yes";
            exe.InstallArguments = " ";
            exe.RepairArguments = " ";
            exe.UninstallArguments = " ";

            if (!string.IsNullOrEmpty(installArguments))
            {
                exe.InstallArguments = installArguments;
            }

            if (!string.IsNullOrEmpty(installCondition))
            {
                exe.InstallCondition = installCondition;                
            }

            if (!string.IsNullOrEmpty(rollbackInstallCondition))
            {
                exe.RollbackInstallCondition = rollbackInstallCondition;
            }

            BurnManifest.Chain.Packages.Add(exe);

            CopyPayloadToLayout(file, newFileName, includeInLayout);
        }

        public void AddSubFile(Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.InstallableItems.BurnInstallableItem itemToAddTo, string file, string newFileName, string url, bool includeInLayout)
        {
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.InstallableItems.SubFile item = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.InstallableItems.SubFile();
            item.GenerateDefaults(file);
            item.URL = url;

            if (!String.IsNullOrEmpty(newFileName))
            {
                item.Name = newFileName;
                item.Id = item.Name;
            }
            item.Id = item.Id.Replace("\\", "-");

            if (itemToAddTo.SubFiles == null) itemToAddTo.SubFiles = new List<Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.InstallableItems.SubFileType>();
            itemToAddTo.SubFiles.Add(item);

            // BUGBUG handle this in a Burn Manifest too.
            // Burn.exe will automatically calculate this for MSI's with external cabs

            CopyPayloadToLayout(file, newFileName, includeInLayout);
        }

        #endregion

        private void CopyPayloadToLayout(string srcFile, string newFileName, bool includeInLayout)
        {
            string destFile = Path.Combine(this.LayoutFolder, Path.GetFileName(srcFile));
            if (!String.IsNullOrEmpty(newFileName))
            {
                destFile = Path.Combine(this.LayoutFolder, newFileName);
            }

            if (includeInLayout)
            {
                CopyFile(srcFile, destFile);
            }
            else
            {
                RemoveFile(destFile);
            }
        }

        /// <summary>
        /// Gets a path to the download cache folder for the current layout and current user.
        /// </summary>
        /// <returns>path to the download cache folder</returns>
        public string GetDownloadCachePath()
        {
            return Path.Combine(System.Environment.ExpandEnvironmentVariables(@"%temp%"), GetDownloadCacheFolderName());
        }

        /// <summary>
        /// Gets a folder name of the download cache folder for the current layout and current user.
        /// </summary>
        /// <returns>path to the download cache folder</returns>
        public string GetDownloadCacheFolderName()
        {
            return this.ParameterInfo.UI.Name + "_" + this.ParameterInfo.UI.Version;
        }

        /// <summary>
        /// Gets a path to the download cache folder for the current layout and current user.
        /// </summary>
        /// <param name="username">username who's download cache path to return.</param>
        /// <returns>path to the download cache folder</returns>
        public string GetDownloadCachePath(string username)
        {
            string currentUserPath = GetDownloadCachePath();
            return currentUserPath.Replace(System.Environment.ExpandEnvironmentVariables("%USERNAME%"), username);
        }

    }
}
