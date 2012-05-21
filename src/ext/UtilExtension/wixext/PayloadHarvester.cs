//-------------------------------------------------------------------------------------------------
// <copyright file="FileHarvester.cs" company="Microsoft">
//    Copyright (c) Microsoft Corporation.  All rights reserved.
//    
//    The use and distribution terms for this software are covered by the
//    Common Public License 1.0 (http://opensource.org/licenses/cpl1.0.php)
//    which can be found in the file CPL.TXT at the root of this distribution.
//    By using this software in any fashion, you are agreeing to be bound by
//    the terms of this license.
//    
//    You must not remove this notice, or any other, from this software.
// </copyright>
// 
// <summary>
// Harvest WiX authoring for a file from the file system.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Extensions
{
    using System;
    using System.IO;

    using Wix = Microsoft.Tools.WindowsInstallerXml.Serialize;

    /// <summary>
    /// Harvest WiX authoring for a payload from the file system.
    /// </summary>
    public sealed class PayloadHarvester : HarvesterExtension
    {
        private bool setUniqueIdentifiers;

        /// <summary>
        /// Instantiate a new PayloadHarvester.
        /// </summary>
        public PayloadHarvester()
        {
            this.setUniqueIdentifiers = true;
        }

        /// <summary>
        /// Gets of sets the option to set unique identifiers.
        /// </summary>
        /// <value>The option to set unique identifiers.</value>
        public bool SetUniqueIdentifiers
        {
            get { return this.setUniqueIdentifiers; }
            set { this.setUniqueIdentifiers = value; }
        }

        /// <summary>
        /// Harvest a payload.
        /// </summary>
        /// <param name="argument">The path of the payload.</param>
        /// <returns>A harvested payload.</returns>
        public override Wix.Fragment[] Harvest(string argument)
        {
            if (null == argument)
            {
                throw new ArgumentNullException("argument");
            }
            
            string fullPath = Path.GetFullPath(argument);

            Wix.RemotePayload remotePayload = this.HarvestRemotePayload(fullPath);

            Wix.Fragment fragment = new Wix.Fragment();
            fragment.AddChild(remotePayload);

            return new Wix.Fragment[] { fragment };
        }

        /// <summary>
        /// Harvest a payload.
        /// </summary>
        /// <param name="path">The path of the payload.</param>
        /// <returns>A harvested payload.</returns>
        public Wix.RemotePayload HarvestRemotePayload(string path)
        {
            if (null == path)
            {
                throw new ArgumentNullException("path");
            }

            if (!File.Exists(path))
            {
                throw new WixException(UtilErrors.FileNotFound(path));
            }

            PayloadInfo payloadInfo = new PayloadInfo() 
            {
                SourceFile = Path.GetFullPath(path),
                SuppressSignatureValidation = false // assume signed, if its unsigned it won't get the certificate properties
            };

            PayloadInfoRow.ResolvePayloadInfo(payloadInfo);

            return payloadInfo;
        }

        /// <summary>
        /// An adapter for RemotePayload that exposes an IPayloadInfo
        /// </summary>
        private class PayloadInfo : Wix.RemotePayload, IPayloadInfo
        {
            public string SourceFile { get; set; }
            public bool SuppressSignatureValidation { get; set; }


            // renamed columns
            public int FileSize
            {
                get
                {
                    return this.Size;
                }
                set
                {
                    this.Size = value;
                }
            }

            public string PublicKey
            {
                get
                {
                    return this.CertificatePublicKey;
                }
                set
                {
                    this.CertificatePublicKey = value;
                }
            }

            public string Thumbprint
            {
                get
                {
                    return this.CertificateThumbprint;
                }
                set
                {
                    this.CertificateThumbprint = value;
                }
            }
        }
    }
}
