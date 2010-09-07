//-------------------------------------------------------------------------------------------------
// <copyright file="BurnWriter.cs" company="Microsoft">
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
// Burn PE writer for the Windows Installer Xml toolset.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml
{
    using System;
    using System.Diagnostics;
    using System.IO;

    /// <summary>
    /// Burn PE writer for the Windows Installer Xml toolset.
    /// </summary>
    /// <remarks>This class encapsulates reading/writing to a stub EXE for
    /// creating bundled/chained setup packages.</remarks>
    /// <example>
    /// using (BurnWriter writer = new BurnWriter(fileExe, this.core, guid))
    /// {
    ///     writer.AppendContainer(file1, BurnWriter.Container.UX);
    ///     writer.AppendContainer(file2, BurnWriter.Container.Attached);
    /// }
    /// </example>
    class BurnWriter : BurnCommon
    {
        /// <summary>
        /// Creates a BurnWriter for re-writing a PE file.
        /// </summary>
        /// <param name="fileExe">File to modify in-place.</param>
        /// <param name="messageHandler">The messagehandler to report warnings/errors to.</param>
        /// <param name="bundleGuid">GUID for the bundle.</param>
        public BurnWriter(string fileExe, IMessageHandler messageHandler, Guid bundleGuid)
            : base(fileExe, messageHandler)
        {
            this.stream = File.Open(fileExe, FileMode.Open, FileAccess.ReadWrite);

            WriteWixburnData(bundleGuid);
        }


        /// <summary>
        /// Finds the ".wixburn" section in the current exe.
        /// </summary>
        /// <returns>true if the ".wixburn" section is successfully found; false otherwise</returns>
        private bool EnsureWixburnSection()
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

                    if (IMAGE_SECTION_WIXBURN_NAME == BurnWriter.ReadUInt64(bytes, IMAGE_SECTION_HEADER_OFFSET_NAME))
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
                if (BURN_SECTION_SIZE > BurnWriter.ReadUInt32(bytes, IMAGE_SECTION_HEADER_OFFSET_SIZEOFRAWDATA))
                {
                    this.messageHandler.OnMessage(WixErrors.StubWixburnSectionTooSmall(this.fileExe));
                    return false;
                }

                this.wixburnDataOffset = BurnWriter.ReadUInt32(bytes, IMAGE_SECTION_HEADER_OFFSET_POINTERTORAWDATA);

                // Update the section information with the actual size of the ".wixburn" section data.
                BurnWriter.WriteUInt32(bytes, IMAGE_SECTION_HEADER_OFFSET_VIRTUALSIZE, BURN_SECTION_SIZE);
                this.WriteBytes(wixburnSectionOffset, bytes);
            }

            return true;
        }

        /// <summary>
        /// Writes the magic ".wixburn" section data and bundle GUID to the current exe.
        /// </summary>
        /// <param name="bundleGuid">GUID for the bundle.</param>
        /// <returns>true if the ".wixburn" section data is successfully updated; false otherwise</returns>
        private bool WriteWixburnData(Guid bundleGuid)
        {
            if (!EnsureWixburnSection())
            {
                return false;
            }

            byte[] bytes = this.ReadBytes(this.wixburnDataOffset, BURN_SECTION_SIZE);

            // Update the ".wixburn" section data.  See the comment above about the
            // fields and offsets.
            BurnWriter.WriteUInt32(bytes, BURN_SECTION_OFFSET_MAGIC, BURN_SECTION_MAGIC);
            BurnWriter.WriteUInt32(bytes, BURN_SECTION_OFFSET_VERSION, BURN_SECTION_VERSION);
            BurnWriter.WriteUInt32(bytes, BURN_SECTION_OFFSET_COUNT, 0);
            BurnWriter.WriteUInt32(bytes, BURN_SECTION_OFFSET_FORMAT, 1); // Hard-coded to CAB for now.
            // TODO: Would we want to catch cases where AppendContainer has already
            // been called?  If it was, we're zeroing out the ux/attached container
            // fields, which might be unexpected.
            BurnWriter.WriteUInt64(bytes, BURN_SECTION_OFFSET_UXADDRESS, 0);
            BurnWriter.WriteUInt64(bytes, BURN_SECTION_OFFSET_UXSIZE, 0);
            BurnWriter.WriteUInt64(bytes, BURN_SECTION_OFFSET_ATTACHEDCONTAINERADDRESS, 0);
            BurnWriter.WriteUInt64(bytes, BURN_SECTION_OFFSET_ATTACHEDCONTAINERSIZE, 0);

            this.messageHandler.OnMessage(WixVerboses.BundleGuid(bundleGuid.ToString("B")));
            byte[] guid = bundleGuid.ToByteArray();
            for (int byteIndex = 0; byteIndex < guid.Length; ++byteIndex)
            {
                bytes[BURN_SECTION_OFFSET_BUNDLEGUID + byteIndex] = guid[byteIndex];
            }

            this.WriteBytes(this.wixburnDataOffset, bytes);
            this.stream.Flush();

            return true;
        }

        /// <summary>
        /// Appends a UX or Attached container to the exe and updates the ".wixburn" section data to point to it.
        /// </summary>
        /// <param name="fileContainer">File to append to the current exe.</param>
        /// <param name="container">Container section represented by the fileContainer.</param>
        /// <returns>true if the container data is successfully appended; false otherwise</returns>
        public bool AppendContainer(string fileContainer, BurnCommon.Container container)
        {
            UInt32 burnSectionCount = 0;
            UInt32 burnSectionOffsetAddress = 0;
            UInt32 burnSectionOffsetSize = 0;

            switch (container)
            {
                case Container.UX:
                    burnSectionCount = 1;
                    burnSectionOffsetAddress = BURN_SECTION_OFFSET_UXADDRESS;
                    burnSectionOffsetSize = BURN_SECTION_OFFSET_UXSIZE;
                    break;

                case Container.Attached:
                    burnSectionCount = 2;
                    burnSectionOffsetAddress = BURN_SECTION_OFFSET_ATTACHEDCONTAINERADDRESS;
                    burnSectionOffsetSize = BURN_SECTION_OFFSET_ATTACHEDCONTAINERSIZE;
                    break;

                default:
                    Debug.Assert(false);
                    return false;
            }

            return AppendContainer(fileContainer, burnSectionOffsetAddress, burnSectionOffsetSize, burnSectionCount);
        }

        /// <summary>
        /// Appends a container to the exe and updates the ".wixburn" section data to point to it.
        /// </summary>
        /// <param name="fileContainer">File to append to the current exe.</param>
        /// <param name="burnSectionOffsetAddress">Offset of address field for this container in ".wixburn" section data.</param>
        /// <param name="burnSectionOffsetSize">Offset of size field for this container in ".wixburn" section data.</param>
        /// <returns>true if the container data is successfully appended; false otherwise</returns>
        private bool AppendContainer(string fileContainer, UInt32 burnSectionOffsetAddress, UInt32 burnSectionOffsetSize, UInt32 burnSectionCount)
        {
            if (!EnsureWixburnSection())
            {
                return false;
            }

            byte[] bytes = this.ReadBytes(this.wixburnDataOffset, BURN_SECTION_SIZE);

            // We expect not to append the same section more than once...
            Debug.Assert(0 == BurnWriter.ReadUInt32(bytes, burnSectionOffsetAddress));
            Debug.Assert(0 == BurnWriter.ReadUInt32(bytes, burnSectionOffsetSize));

            using (FileStream reader = File.OpenRead(fileContainer))
            {
                // Update the ".wixburn" section data
                BurnWriter.WriteUInt32(bytes, BURN_SECTION_OFFSET_COUNT, burnSectionCount);
                BurnWriter.WriteUInt64(bytes, burnSectionOffsetAddress, (UInt64)this.stream.Length);
                BurnWriter.WriteUInt64(bytes, burnSectionOffsetSize, (UInt64)reader.Length);
                this.WriteBytes(this.wixburnDataOffset, bytes);

                // append the container in 4096-byte chunks.
                this.stream.Seek(0, SeekOrigin.End);
                byte[] page = new byte[4096];
                int bytesRead = reader.Read(page, 0, page.Length);
                while (0 < bytesRead)
                {
                    this.stream.Write(page, 0, bytesRead);
                    bytesRead = reader.Read(page, 0, page.Length);
                }
            }

            this.stream.Flush();

            return true;
        }
    }
}
