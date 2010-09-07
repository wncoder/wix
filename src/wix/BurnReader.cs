//-------------------------------------------------------------------------------------------------
// <copyright file="BurnReader.cs" company="Microsoft">
//    Copyright (c) Microsoft Corporation.  All rights reserved.
//    
//    The use and distribution terms for this software are covered by the
//    Common Public License 1.0 (http://opensource.org/licenses/cpl.php)
//    which can be found in the file CPL.TXT at the root of this distribution.
//    By using this software in any fashion, you are agreeing to be bound by
//    the terms of this license.
//    
//    You must not remove this notice, or any other, from this software.
// </copyright>
// 
// <summary>
// Burn PE reader for the Windows Installer Xml toolset.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml
{
    using Microsoft.Tools.WindowsInstallerXml.Cab;
    using System;
    using System.Collections;
    using System.Collections.Generic;
    using System.Diagnostics;
    using System.IO;
    using System.Xml;

    /// <summary>
    /// Burn PE reader for the Windows Installer Xml toolset.
    /// </summary>
    /// <remarks>This class encapsulates reading from a stub EXE with containers attached
    /// for dissecting bundled/chained setup packages.</remarks>
    /// <example>
    /// using (BurnReader reader = BurnReader.Open(fileExe, this.core, guid))
    /// {
    ///     reader.AppendContainer(file1, BurnReader.Container.UX);
    ///     reader.AppendContainer(file2, BurnReader.Container.Attached);
    /// }
    /// </example>
    class BurnReader : BurnCommon
    {
        private bool invalidBundle;
        private UInt32 version = 0;
        private UInt32 sectionCount = 0;
        private UInt64 uxAddress = 0;
        private UInt64 uxSize = 0;
        private UInt64 attachedContainerAddress = 0;
        private UInt64 attachedContainerSize = 0;

        List<DictionaryEntry> attachedContainerPayloadNames;

        /// <summary>
        /// Creates a BurnReader for reading a PE file.
        /// </summary>
        /// <param name="fileExe">File to read.</param>
        /// <param name="messageHandler">The messagehandler to report warnings/errors to.</param>
        public BurnReader(string fileExe, IMessageHandler messageHandler)
            : base(fileExe, messageHandler)
        {
            this.stream = File.Open(fileExe, FileMode.Open, FileAccess.Read, FileShare.Read);

            if (!GetWixburnSectionInfo())
            {
                // If we detect we're an invalid bundle,
                // remember that so we don't continue trying to extract if asked again
                invalidBundle = true;
                return;
            }

            if (!ReadWixburnData())
            {
                invalidBundle = true;
                return;
            }

            attachedContainerPayloadNames = new List<DictionaryEntry>();
        }

        /// <summary>
        /// Finds the ".wixburn" section in the current exe.
        /// </summary>
        /// <returns>true if the ".wixburn" section is successfully found; false otherwise</returns>
        private bool GetWixburnSectionInfo()
        {
            if (UInt32.MaxValue == this.wixburnDataOffset)
            {
                if (!EnsureNTHeader())
                {
                    return false;
                }

                byte[] bytes = new byte[IMAGE_SECTION_HEADER_SIZE];
                this.stream.Seek(this.firstSectionOffset, SeekOrigin.Begin);

                UInt32 wixburnSectionOffset = UInt32.MaxValue;

                for (UInt16 sectionIndex = 0; sectionIndex < this.sections; ++sectionIndex)
                {
                    this.stream.Read(bytes, 0, bytes.Length);

                    if (IMAGE_SECTION_WIXBURN_NAME == BurnCommon.ReadUInt64(bytes, IMAGE_SECTION_HEADER_OFFSET_NAME))
                    {
                        wixburnSectionOffset = this.firstSectionOffset + (IMAGE_SECTION_HEADER_SIZE * sectionIndex);
                        break;
                    }
                }

                if (UInt32.MaxValue == wixburnSectionOffset)
                {
                    this.messageHandler.OnMessage(WixErrors.StubMissingWixburnSection(this.fileExe));
                    return false;
                }

                // we need 32 bytes for the manifest header, which is always going to fit in 
                // the smallest alignment (512 bytes), but just to be paranoid...
                if (BURN_SECTION_SIZE > BurnCommon.ReadUInt32(bytes, IMAGE_SECTION_HEADER_OFFSET_SIZEOFRAWDATA))
                {
                    this.messageHandler.OnMessage(WixErrors.StubWixburnSectionTooSmall(this.fileExe));
                    return false;
                }

                this.wixburnDataOffset = BurnCommon.ReadUInt32(bytes, IMAGE_SECTION_HEADER_OFFSET_POINTERTORAWDATA);
            }

            return true;
        }

        /// <summary>
        /// Read the magic ".wixburn" section data and bundle GUID from the current exe.
        /// </summary>
        /// <returns>true if the ".wixburn" section data is successfully read; false otherwise</returns>
        private bool ReadWixburnData()
        {
            byte[] bytes = this.ReadBytes(this.wixburnDataOffset, BURN_SECTION_SIZE);
            UInt32 uint32 = 0;

            uint32 = BurnCommon.ReadUInt32(bytes, BURN_SECTION_OFFSET_MAGIC);
            if (BURN_SECTION_MAGIC != uint32)
            {
                this.messageHandler.OnMessage(WixErrors.InvalidBundle(this.fileExe));
                return false;
            }
            version = BurnCommon.ReadUInt32(bytes, BURN_SECTION_OFFSET_VERSION);
            if (BURN_SECTION_VERSION != version)
            {
                this.messageHandler.OnMessage(WixErrors.BundleTooNew(this.fileExe, version));
                return false;
            }

            sectionCount = BurnCommon.ReadUInt32(bytes, BURN_SECTION_OFFSET_COUNT);
            uint32 = BurnCommon.ReadUInt32(bytes, BURN_SECTION_OFFSET_FORMAT); // We only know how to deal with CABs right now
            if (1 != uint32)
            {
                this.messageHandler.OnMessage(WixErrors.InvalidBundle(this.fileExe));
                return false;
            }

            uxAddress = BurnCommon.ReadUInt64(bytes, BURN_SECTION_OFFSET_UXADDRESS);
            uxSize = BurnCommon.ReadUInt64(bytes, BURN_SECTION_OFFSET_UXSIZE);
            attachedContainerAddress = BurnCommon.ReadUInt64(bytes, BURN_SECTION_OFFSET_ATTACHEDCONTAINERADDRESS);
            attachedContainerSize = BurnCommon.ReadUInt64(bytes, BURN_SECTION_OFFSET_ATTACHEDCONTAINERSIZE);

            return true;
        }

        /// <summary>
        /// Gets the UX container from the exe and extracts its contents to the output directory.
        /// </summary>
        /// <param name="outputDirectory">Directory to write extracted files to.</param>
        /// <returns>True if successful, false otherwise</returns>
        public bool ExtractUXContainer(string outputDirectory, string tempDirectory)
        {
            // No UX container to extract
            if (uxAddress == 0 || uxSize == 0)
            {
                return false;
            }
            if (invalidBundle)
            {
                return false;
            }

            Directory.CreateDirectory(outputDirectory);
            string tempCabPath = Path.Combine(tempDirectory, "ux.cab");
            string manifestOriginalPath = Path.Combine(outputDirectory, "0");
            string manifestPath = Path.Combine(outputDirectory, "manifest.xml");

            byte[] bytes = this.ReadBytes(uxAddress, uxSize);
            using (Stream tempCab = File.Open(tempCabPath, FileMode.Create, FileAccess.Write))
            {
                tempCab.Write(bytes, 0, bytes.Length);
                tempCab.Close();
            }

            using (WixExtractCab extract = new WixExtractCab())
            {
                extract.Extract(tempCabPath, outputDirectory);
            }

            Directory.CreateDirectory(Path.GetDirectoryName(manifestPath));
            File.Delete(manifestPath);
            File.Move(manifestOriginalPath, manifestPath);

            XmlDocument document = new XmlDocument();
            document.Load(manifestPath);
            XmlNamespaceManager namespaceManager = new XmlNamespaceManager(document.NameTable);
            namespaceManager.AddNamespace("burn", BurnCommon.BurnNamespace);
            XmlNodeList uxPayloads = document.SelectNodes("/burn:BurnManifest/burn:UX/burn:Payload", namespaceManager);
            XmlNodeList payloads = document.SelectNodes("/burn:BurnManifest/burn:Payload", namespaceManager);

            foreach (XmlNode uxPayload in uxPayloads)
            {
                XmlNode sourcePathNode = uxPayload.Attributes.GetNamedItem("SourcePath");
                XmlNode filePathNode = uxPayload.Attributes.GetNamedItem("FilePath");

                string sourcePath = Path.Combine(outputDirectory, sourcePathNode.Value);
                string destinationPath = Path.Combine(outputDirectory, filePathNode.Value);

                Directory.CreateDirectory(Path.GetDirectoryName(destinationPath));
                File.Delete(destinationPath);
                File.Move(sourcePath, destinationPath);
            }

            foreach (XmlNode payload in payloads)
            {
                XmlNode sourcePathNode = payload.Attributes.GetNamedItem("SourcePath");
                XmlNode filePathNode = payload.Attributes.GetNamedItem("FilePath");

                string sourcePath = sourcePathNode.Value;
                string destinationPath = filePathNode.Value;

                attachedContainerPayloadNames.Add(new DictionaryEntry(sourcePath, destinationPath));
            }

            return true;
        }

        /// <summary>
        /// Gets the attached container from the exe and extracts its contents to the output directory.
        /// </summary>
        /// <param name="outputDirectory">Directory to write extracted files to.</param>
        /// <returns>True if successful, false otherwise</returns>
        public bool ExtractAttachedContainer(string outputDirectory, string tempDirectory)
        {
            // No attached container to extract
            if (attachedContainerAddress == 0 || attachedContainerSize == 0)
            {
                return false;
            }
            if (invalidBundle)
            {
                return false;
            }

            Directory.CreateDirectory(outputDirectory);
            string tempCabPath = Path.Combine(tempDirectory, "attached.cab");

            byte[] bytes = this.ReadBytes(attachedContainerAddress, attachedContainerSize);
            using (Stream tempCab = File.Open(tempCabPath, FileMode.Create, FileAccess.Write))
            {
                tempCab.Write(bytes, 0, bytes.Length);
                tempCab.Close();
            }

            using (WixExtractCab extract = new WixExtractCab())
            {
                extract.Extract(tempCabPath, outputDirectory);
            }

            foreach (DictionaryEntry entry in this.attachedContainerPayloadNames)
            {
                string sourcePath = Path.Combine(outputDirectory, (string)entry.Key);
                string destinationPath = Path.Combine(outputDirectory, (string)entry.Value);

                Directory.CreateDirectory(Path.GetDirectoryName(destinationPath));
                File.Delete(destinationPath);
                File.Move(sourcePath, destinationPath);
            }

            return true;
        }
    }
}
