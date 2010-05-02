﻿//-----------------------------------------------------------------------
// <copyright file="BurnSubFile.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>File and FileRef element OM</summary>
//-----------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems;
using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.ElementAttribute;

namespace Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.InstallableItems
{
    public class SubFileType
    {

        # region Private member variables

        private string m_Name;
        private string m_Id;
        //private bool? m_IgnoreDownloadFailure;
        private string m_Url;
        private uint? m_DownloadSize;
        private string m_Sha1HashValue;
        //private bool? m_Compressed;
        private uint m_SystemDriveSize;
        private uint m_InstalledProductSize;
        //private uint? m_CompressedDownloadSize;
        //private ParameterInfoConfigurator.BurnItems.BurnBaseItems.YesNoType? m_GenerateCompressedHash;
        private string m_SourceFilePath;

        #endregion

        # region Public Property

        /// <summary>
        /// The name of the Item
        /// </summary>
        [BurnXmlAttribute("Name")]
        public string Name
        {
            get
            {
                if (m_Name == null) m_Name = "";
                return m_Name;
            }

            set
            {
                m_Name = value;
            }
        }


        /// <summary>
        /// The name of the Item
        /// </summary>
        [BurnXmlAttribute("Id")]
        public string Id
        {
            get
            {
                if (m_Id == null) m_Id = "";
                return m_Id;
            }

            set
            {
                m_Id = value;
            }
        }

        ///// <summary>
        ///// Determine if we should ignore the return code and return S_OK.  Default = False
        ///// </summary>
        //[BurnXmlAttribute("IgnoreDownloadFailure", false)]
        //public bool? IgnoreDownloadFailure
        //{
        //    get
        //    {
        //        return m_IgnoreDownloadFailure;
        //    }

        //    set
        //    {
        //        m_IgnoreDownloadFailure = value;
        //    }
        //}

        /// <summary>
        /// URL of the file to download if necessary
        /// </summary>
        [BurnXmlAttribute("URL")]
        public string URL
        {
            get
            {
                return m_Url;
            }

            set
            {
                m_Url = value;
            }
        }

        /// <summary>
        /// The size of the item in bytes. This value is used to calibrate download progressbar and estimate download time
        /// </summary>
        [BurnXmlAttribute("DownloadSize")]
        public uint? DownloadSize
        {
            get
            {
                if (m_DownloadSize == null)
                    m_DownloadSize = Convert.ToUInt32(Utility.Utility.GetFileSize(this.SourceFilePath));

                return m_DownloadSize;
            }

            set
            {
                m_DownloadSize = value;
            }
        }

        /// <summary>
        /// The SHA256 hash value of the file being downloaded
        /// </summary>
        [BurnXmlAttribute("Sha1HashValue")]
        public string Sha1HashValue
        {
            get
            {
                if (string.IsNullOrEmpty(m_Sha1HashValue))
                    m_Sha1HashValue = Utility.Utility.GetSHA1HashValue(this.SourceFilePath);

                return m_Sha1HashValue;
            }

            set
            {
                m_Sha1HashValue = value;
            }
        }

        ///// <summary>
        ///// Indicates that payload is compressed
        ///// </summary>
        //[BurnXmlAttribute("Compressed", false)]
        //public bool? Compressed
        //{
        //    get
        //    {
        //        return m_Compressed;
        //    }

        //    set
        //    {
        //        m_Compressed = value;
        //    }
        //}

        /// <summary>
        /// The estimated size that the item will take on the system drive in bytes after install
        /// </summary>
        [BurnXmlAttribute("SystemDriveSize", 11111)]
        public uint SystemDriveSize
        {
            get
            {
                return m_SystemDriveSize;
            }

            set
            {
                m_SystemDriveSize = value;
            }
        }

        /// <summary>
        /// The estimated size that the item will take on the drive the Product is installed in bytes
        /// </summary>
        [BurnXmlAttribute("InstalledProductSize", 11111)]
        public uint InstalledProductSize
        {
            get
            {
                return m_InstalledProductSize;
            }

            set
            {
                m_InstalledProductSize = value;
            }
        }

        ///// <summary>
        ///// The size of the compressed item in bytes
        ///// </summary>
        //[BurnXmlAttribute("CompressedDownloadSize")]
        //public uint? CompressedDownloadSize
        //{
        //    get
        //    {
        //        return m_CompressedDownloadSize;
        //    }

        //    set
        //    {
        //        m_CompressedDownloadSize = value;
        //    }
        //}

        ///// <summary>
        ///// Serializer only attribute: If yes, then the HashValue attribute is calculated on CompressedSource and added to the item.
        ///// Default is no HashValue is added
        ///// </summary>
        //[BurnXmlAttribute("GenerateCompressedHash", ParameterInfoConfigurator.BurnItems.BurnBaseItems.YesNoType.no)]
        //public ParameterInfoConfigurator.BurnItems.BurnBaseItems.YesNoType? GenerateCompressedHash
        //{
        //    get
        //    {
        //        return m_GenerateCompressedHash;
        //    }

        //    set
        //    {
        //        m_GenerateCompressedHash = value;
        //    }
        //}

        /// <summary>
        /// To define the source file path of payload
        /// </summary>
        public string SourceFilePath
        {
            get
            {
                return m_SourceFilePath;
            }

            set
            {
                m_SourceFilePath = value;
            }
        }

        #endregion

        #region Methods for generating default values for all the required Properties

        public void GenerateDefaults(string file)
        {
            this.SourceFilePath = file;
            this.Name = System.IO.Path.GetFileName(file);
            this.Id = this.Name;
            this.DownloadSize = null;  // null value will cause it to be calculated
            this.InstalledProductSize = (uint)this.DownloadSize;
            this.Sha1HashValue = null;  // null value will cause it to be calculated
            this.SystemDriveSize = 0;
            this.URL = null;
        }

        #endregion
    }


    [BurnXmlElement("FileRef")]
    public class SubFileRef : SubFileType
    {

    }

    [BurnXmlElement("File")]
    public class SubFile : SubFileType
    {

    }
}
