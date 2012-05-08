//-------------------------------------------------------------------------------------------------
// <copyright file="BundleInfo.cs" company="Microsoft">
//    Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// 
// <summary>
// Bundle info for binding Bundles.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml
{
    using System;

    /// <summary>
    /// Bundle info for binding Bundles.
    /// </summary>
    internal class BundleInfo
    {
        public BundleInfo(string bundleFile, Row row)
        {
            this.Id = Guid.NewGuid();
            this.Path = System.IO.Path.GetFullPath(bundleFile);
            this.PerMachine = true; // default to per-machine but the first-per user package would flip it.

            this.SourceLineNumbers = row.SourceLineNumbers;
            this.Version = (string)row[0];
            this.Copyright = (string)row[1];

            this.RegistrationInfo = new RegistrationInfo();
            this.RegistrationInfo.Name = (string)row[2];
            this.RegistrationInfo.AboutUrl = (string)row[3];
            this.RegistrationInfo.DisableModify = (null != row[4]) ? (int)row[4] : 0;
            this.RegistrationInfo.DisableRemove = (null != row[5] && 1 == (int)row[5]);
            // (deprecated) this.RegistrationInfo.DisableRepair = (null != row[6] && 1 == (int)row[6]);
            this.RegistrationInfo.HelpTelephone = (string)row[7];
            this.RegistrationInfo.HelpLink = (string)row[8];
            this.RegistrationInfo.Publisher = (string)row[9];
            this.RegistrationInfo.UpdateUrl = (string)row[10];

            if (null != row[11])
            {
                Compressed = (1 == (int)row[11]) ? YesNoDefaultType.Yes : YesNoDefaultType.No;
            }

            if (null != row[12])
            {
                string[] logVariableAndPrefixExtension = ((string)row[12]).Split(':');
                this.LogPathVariable = logVariableAndPrefixExtension[0];

                string logPrefixAndExtension = logVariableAndPrefixExtension[1];
                int extensionIndex = logPrefixAndExtension.LastIndexOf('.');
                this.LogPrefix = logPrefixAndExtension.Substring(0, extensionIndex);
                this.LogExtension = logPrefixAndExtension.Substring(extensionIndex + 1);
            }

            if (null != row[13])
            {
                this.IconPath = (string)row[13];
            }

            if (null != row[14])
            {
                this.SplashScreenBitmapPath = (string)row[14];
            }

            this.Condition = (string)row[15];
            this.Tag = (string)row[16];
            this.Platform = (Platform)Enum.Parse(typeof(Platform), (string)row[17]);

            this.RegistrationInfo.ParentName = (string)row[18];
            this.UpgradeCode = (string)row[19];

            // Default provider key is the Id.
            this.ProviderKey = this.Id.ToString("B");
        }

        public YesNoDefaultType Compressed = YesNoDefaultType.Default;
        public PackagingType DefaultPackagingType
        {
            get
            {
                return (this.Compressed == YesNoDefaultType.No) ? PackagingType.External : PackagingType.Embedded;
            }

            private set {}
        }
        public Guid Id { get; private set; }
        public string Condition { get; private set; }
        public string Copyright { get; private set; }
        public string IconPath { get; private set; }
        public string LogPathVariable { get; private set; }
        public string LogPrefix { get; private set; }
        public string LogExtension { get; private set; }
        public string Path { get; private set; }
        public bool PerMachine { get; set; }
        public RegistrationInfo RegistrationInfo { get; set; }
        public UpdateRegistrationInfo UpdateRegistrationInfo { get; set; }
        public string SplashScreenBitmapPath { get; private set; }
        public SourceLineNumberCollection SourceLineNumbers { get; private set; }
        public string Tag { get; private set; }
        public Platform Platform { get; private set; }
        public string Version { get; private set; }
        public string UpgradeCode { get; private set; }
        public string ProviderKey { get; internal set; }
    }
}
