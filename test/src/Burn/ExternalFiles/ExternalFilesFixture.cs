//-----------------------------------------------------------------------
// <copyright file="ExternalFilesFixture.cs" company="Microsoft">
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
//     - Test fixture for Burn External Files feature.
// </summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Tests.Burn.ExternalFiles
{
    using System;
    using System.Diagnostics;
    using System.IO;
    using Microsoft.Tools.WindowsInstallerXml.Test.Tests.Burn.CommonTestFixture;

    class ExternalFilesFixture : BurnCommonTestFixture
    {
        public enum ExternalFileType
        {
            File,
            FileRef
        }

        public enum CacheState
        {
            InLayoutOnly,
            InLayoutAndInPayloadCache,
            ToBeDownloadedOnly,
            ToBeDownloadedAndInPayloadCache,
            ToBeDownloadedAndInDownloadCache,
            ToBeDownloadedAndInPayloadCacheAndInDownloadCache
        }

        public enum ItemNames
        {
            FileExt,
            PathFileExt,
            ManyPathFileExt
        }

        private PayloadType m_Item1PayloadType;
        private ItemNames m_Item1ItemName;
        private ExternalFileType m_Item1FileType;
        private ItemNames m_Item1FileName;
        private CacheState m_Item1ItemCacheState;
        private CacheState m_Item1FileCacheState;
        private PayloadType? m_Item2PayloadType;
        private ItemNames? m_Item2ItemName;
        private ExternalFileType? m_Item2FileType;
        private ItemNames? m_Item2FileName;
        private CacheState? m_Item2ItemCacheState;
        private CacheState? m_Item2FileCacheState;

        public ExternalFilesFixture()
            : base()
        {
            this.Layout = new Microsoft.Tools.WindowsInstallerXml.Test.Burn.LayoutManager.LayoutManager(new Microsoft.Tools.WindowsInstallerXml.Test.Burn.LayoutManager.Ux.SampleUx());
        }

        public override void CleanUp()
        {
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.Utility.MsiUtils.RemoveMSI(testMsiFile);
            Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.Utility.MsiUtils.RemoveMSI(testMsiPerUserFile);
            base.CleanUp();
        }

        public void RunTest(PayloadType item1PayloadType,
            ItemNames item1ItemName,
            ExternalFileType item1FileType,
            ItemNames item1FileName,
            CacheState item1ItemCacheState,
            CacheState item1FileCacheState,
            PayloadType? item2PayloadType,
            ItemNames? item2ItemName,
            ExternalFileType? item2FileType,
            ItemNames? item2FileName,
            CacheState? item2ItemCacheState,
            CacheState? item2FileCacheState,
            UserType userType
            )
        {

            m_Item1PayloadType = item1PayloadType;
            m_Item1ItemName = item1ItemName;
            m_Item1FileType = item1FileType;
            m_Item1FileName = item1FileName;
            m_Item1ItemCacheState = item1ItemCacheState;
            m_Item1FileCacheState = item1FileCacheState;
            m_Item2PayloadType = item2PayloadType;
            m_Item2ItemName = item2ItemName;
            m_Item2FileType = item2FileType;
            m_Item2FileName = item2FileName;
            m_Item2ItemCacheState = item2ItemCacheState;
            m_Item2FileCacheState = item2FileCacheState;
            UserTypeToUse = userType;

            BuildLayout();
            InitializeMachineState();
            VerifiableProperties = this.RunScenario(
                InstallMode.install,
                UiMode.Passive,
                userType);
        }

        private void BuildLayout()
        {
            // add item1 
            string item1Name;
            switch (m_Item1PayloadType)
            {
                case PayloadType.Exe:
                    item1Name = "item1.exe";
                    if (this.m_Item1ItemName == ItemNames.PathFileExt) item1Name = "Item1Subdir\\" + item1Name;
                    if (this.m_Item1ItemName == ItemNames.ManyPathFileExt) item1Name = "Item1 Subdir A\\sub dir B\\" + item1Name;
                    this.Layout.AddExe(testExeFile, item1Name, testExeUrl, false);
                    if (this.UserTypeToUse == UserType.NormalUser)
                    {
                        this.Layout.ParameterInfo.Registration.PerMachine = "no";
                        ((Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnInstallableItems.ExeItem)this.Layout.ParameterInfo.Items.Items[0]).PerMachine = false;
                        ((Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.BurnManifestOM.Chain.ExePackageElement)this.Layout.BurnManifest.Chain.Packages[0]).PerMachine = "no";
                    }
                    break;
                case PayloadType.MsiPerMachine:
                    item1Name = Path.GetFileName(testMsiFile);
                    if (this.m_Item1ItemName == ItemNames.PathFileExt) item1Name = "Item1Subdir\\" + item1Name;
                    if (this.m_Item1ItemName == ItemNames.ManyPathFileExt) item1Name = "Item1 Subdir A\\sub dir B\\" + item1Name;
                    this.Layout.AddMsi(testMsiFile, item1Name, testMsiUrl, false);
                    break;
                case PayloadType.MsiPerUser:
                    item1Name = Path.GetFileName(testMsiPerUserFile);
                    if (this.m_Item1ItemName == ItemNames.PathFileExt) item1Name = "Item1Subdir\\" + item1Name;
                    if (this.m_Item1ItemName == ItemNames.ManyPathFileExt) item1Name = "Item1 Subdir A\\sub dir B\\" + item1Name;
                    this.Layout.AddMsi(testMsiPerUserFile, item1Name, testMsiPerUserUrl, false);
                    this.Layout.ParameterInfo.Registration.PerMachine = "no";
                    ((Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnInstallableItems.MsiItem)this.Layout.ParameterInfo.Items.Items[0]).PerMachine = false;
                    break;
                case PayloadType.Msp:
                    item1Name = Path.GetFileName(testMspFile);
                    if (this.m_Item1ItemName == ItemNames.PathFileExt) item1Name = "Item1Subdir\\" + item1Name;
                    if (this.m_Item1ItemName == ItemNames.ManyPathFileExt) item1Name = "Item1 Subdir A\\sub dir B\\" + item1Name;
                    this.Layout.AddMsp(testMspFile, item1Name, testMspUrl, false);
                    break;
                default:
                    throw new NotImplementedException("That Item type is not supported by this fixture yet");
            }
            string item1FileName = "item1SubFile1.txt";
            if (this.m_Item1FileName == ItemNames.PathFileExt) item1FileName = "File1Subdir\\" + item1FileName;
            if (this.m_Item1FileName == ItemNames.ManyPathFileExt) item1FileName = "File1 Subdir A\\sub dir B\\" + item1FileName;
            this.Layout.AddSubFile((Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.InstallableItems.BurnInstallableItem)this.Layout.ParameterInfo.Items.Items[0],
                testFileFile,
                item1FileName,
                testFileUrl,
                (this.m_Item1FileCacheState == CacheState.InLayoutAndInPayloadCache) || (this.m_Item1FileCacheState == CacheState.InLayoutOnly));

            // add item2
            string item2Name;
            if (null != m_Item2PayloadType)
            {
                switch (m_Item2PayloadType)
                {
                    case PayloadType.Exe:
                        item2Name = "item2.exe";
                        if (this.m_Item2ItemName == ItemNames.PathFileExt) item2Name = "Item2Subdir\\" + item2Name;
                        if (this.m_Item2ItemName == ItemNames.ManyPathFileExt) item2Name = "Item2 Subdir A\\sub dir B\\" + item2Name;
                        this.Layout.AddExe(testExeFile, item2Name, testExeUrl, false);
                        if (this.UserTypeToUse == UserType.NormalUser)
                        {
                            this.Layout.ParameterInfo.Registration.PerMachine = "no";
                            ((Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnInstallableItems.ExeItem)this.Layout.ParameterInfo.Items.Items[1]).PerMachine = false;
                            ((Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.BurnManifestOM.Chain.ExePackageElement)this.Layout.BurnManifest.Chain.Packages[1]).PerMachine = "no";
                        }
                        break;
                    case PayloadType.MsiPerMachine:
                        item2Name = Path.GetFileName(testMsiFile);
                        if (this.m_Item2ItemName == ItemNames.PathFileExt) item2Name = "Item2Subdir\\" + item2Name;
                        if (this.m_Item2ItemName == ItemNames.ManyPathFileExt) item2Name = "Item2 Subdir A\\sub dir B\\" + item2Name;
                        this.Layout.AddMsi(testMsiFile, item2Name, testMsiUrl, false);
                        break;
                    case PayloadType.MsiPerUser:
                        item1Name = Path.GetFileName(testMsiPerUserFile);
                        if (this.m_Item1ItemName == ItemNames.PathFileExt) item1Name = "Item1Subdir\\" + item1Name;
                        if (this.m_Item1ItemName == ItemNames.ManyPathFileExt) item1Name = "Item1 Subdir A\\sub dir B\\" + item1Name;
                        this.Layout.AddMsi(testMsiPerUserFile, item1Name, testMsiPerUserUrl, false);
                        this.Layout.ParameterInfo.Registration.PerMachine = "no";
                        ((Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnInstallableItems.MsiItem)this.Layout.ParameterInfo.Items.Items[1]).PerMachine = false;
                        break;
                    case PayloadType.Msp:
                        item2Name = Path.GetFileName(testMspFile);
                        if (this.m_Item2ItemName == ItemNames.PathFileExt) item2Name = "Item2Subdir\\" + item2Name;
                        if (this.m_Item2ItemName == ItemNames.ManyPathFileExt) item2Name = "Item2 Subdir A\\sub dir B\\" + item2Name;
                        this.Layout.AddMsp(testMspFile, item2Name, testMspUrl, false);
                        break;
                    default:
                        throw new NotImplementedException("That Item type is not supported by this fixture yet");
                }
                string item2FileName = "item2SubFile1.txt";
                if (this.m_Item2FileName == ItemNames.PathFileExt) item2FileName = "File2Subdir\\" + item2FileName;
                if (this.m_Item2FileName == ItemNames.ManyPathFileExt) item2FileName = "File2 Subdir A\\sub dir B\\" + item2FileName;

                this.Layout.AddSubFile((Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.InstallableItems.BurnInstallableItem)this.Layout.ParameterInfo.Items.Items[1],
                    testFileFile,
                    item2FileName,
                    testFileUrl,
                    (this.m_Item1FileCacheState == CacheState.InLayoutAndInPayloadCache) || (this.m_Item1FileCacheState == CacheState.InLayoutOnly));
            }

            this.Layout.GenerateLayout();
        }

        private void InitializeMachineState()
        {
            // Configure Machine's Install State
            // install testMsiFile
            if (this.m_Item1PayloadType == PayloadType.Msp || this.m_Item2PayloadType == PayloadType.Msp)
            {
                Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.Utility.MsiUtils.InstallMSI(testMsiFile);
            }

            // Configure Machine's Download Cache State
            // BUGBUG TODO 

            //if (m_DownloadCacheState == DownloadCacheState.FilesExist)
            //{
            //    string dest = Path.Combine(this.Layout.GetDownloadCachePath(), fileName);

            //    Microsoft.Tools.WindowsInstallerXml.Test.Burn.LayoutManager.LayoutManager.CopyFile(testFileFile, dest);
            //}

            // Configure Machine's Payload Cache State
            // BUGBUG TODO 

        }

        /// <summary>
        /// Verifies the install succeeds and all the payload caches are accurate
        /// </summary>
        /// <returns>true if the test passes, false otherwise</returns>
        public override bool TestPasses()
        {
            bool retVal = true;
            string traceCategory = "Verify TestPasses";

            Trace.WriteLine("Start", traceCategory);
            Trace.Indent();

            retVal &= ExitCodeMatchesExpectation(0);
            retVal &= MetricsDataIsAccurate();
            retVal &= PayloadCacheIsAccurate();

            Trace.WriteLineIf(retVal, "PASS", traceCategory);
            Trace.WriteLineIf(!retVal, "FAIL", traceCategory);

            Trace.Unindent();
            Trace.WriteLine("End", traceCategory);

            return retVal;
        }
    }
}
