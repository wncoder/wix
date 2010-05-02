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
    /// using (BurnWriter writer = BurnWriter.Open(fileExe, this.core, guid))
    /// {
    ///     writer.AppendContainer(file1, BurnWriter.Container.UX);
    ///     writer.AppendContainer(file2, BurnWriter.Container.Attached);
    /// }
    /// </example>
    class BurnWriter : IDisposable
    {
        // See WinNT.h for details about the PE format, including the
        // structure and offsets for IMAGE_DOS_HEADER, IMAGE_NT_HEADERS32,
        // IMAGE_FILE_HEADER, etc.
        private const UInt32 IMAGE_DOS_HEADER_SIZE = 64;
        private const UInt32 IMAGE_DOS_HEADER_OFFSET_MAGIC = 0;
        private const UInt32 IMAGE_DOS_HEADER_OFFSET_NTHEADER = 60;

        private const UInt32 IMAGE_NT_HEADER_SIZE = 24; // signature DWORD (4) + IMAGE_FILE_HEADER (20)
        private const UInt32 IMAGE_NT_HEADER_OFFSET_SIGNATURE = 0;
        private const UInt32 IMAGE_NT_HEADER_OFFSET_NUMBEROFSECTIONS = 6;
        private const UInt32 IMAGE_NT_HEADER_OFFSET_SIZEOFOPTIONALHEADER = 20;

        private const UInt32 IMAGE_SECTION_HEADER_SIZE = 40;
        private const UInt32 IMAGE_SECTION_HEADER_OFFSET_NAME = 0;
        private const UInt32 IMAGE_SECTION_HEADER_OFFSET_VIRTUALSIZE = 8;
        private const UInt32 IMAGE_SECTION_HEADER_OFFSET_SIZEOFRAWDATA = 16;
        private const UInt32 IMAGE_SECTION_HEADER_OFFSET_POINTERTORAWDATA = 20;

        private const UInt16 IMAGE_DOS_SIGNATURE = 0x5A4D;
        private const UInt32 IMAGE_NT_SIGNATURE = 0x00004550;
        private const UInt64 IMAGE_SECTION_WIXBURN_NAME = 0x6E7275627869772E; // ".wixburn", as a qword.

        // The ".wixburn" section contains:
        //    0- 3:  magic number
        //    4- 7:  version
        //    8-11:  offset to start of manifest + UX container
        //   12-15:  byte count of manifest + UX container
        //   16-19:  offset to start of attached container
        //   20-23:  byte count of attached container
        //   24-39:  bundle GUID
        private const UInt32 BURN_SECTION_OFFSET_MAGIC = 0;
        private const UInt32 BURN_SECTION_OFFSET_VERSION = 4;
        private const UInt32 BURN_SECTION_OFFSET_UXADDRESS = 8;
        private const UInt32 BURN_SECTION_OFFSET_UXSIZE = 12;
        private const UInt32 BURN_SECTION_OFFSET_ATTACHEDCONTAINERADDRESS = 16;
        private const UInt32 BURN_SECTION_OFFSET_ATTACHEDCONTAINERSIZE = 20;
        private const UInt32 BURN_SECTION_OFFSET_BUNDLEGUID = 24;
        private const UInt32 BURN_SECTION_SIZE = BURN_SECTION_OFFSET_BUNDLEGUID + 16 /*sizeof(GUID) - last field */;

        private const UInt32 BURN_SECTION_MAGIC = 0x00f14300;
        private const UInt32 BURN_SECTION_VERSION = 0x00000001;

        private string fileExe;
        private IMessageHandler messageHandler;
        private FileStream stream;
        private UInt32 peOffset = UInt32.MaxValue;
        private UInt16 sections = UInt16.MaxValue;
        private UInt32 firstSectionOffset = UInt32.MaxValue;
        private UInt32 wixburnDataOffset = UInt32.MaxValue;

        // TODO: does this enum exist in another form somewhere?
        /// <summary>
        /// The types of attached containers that BurnWriter supports.
        /// </summary>
        public enum Container
        {
            UX,
            Attached
        }

        /// <summary>
        /// Creates a BurnWriter for re-writing a PE file.
        /// </summary>
        /// <param name="fileExe">File to modify in-place.</param>
        /// <param name="messageHandler">The messagehandler to report warnings/errors to.</param>
        /// <param name="bundleGuid">GUID for the bundle.</param>
        public BurnWriter(string fileExe, IMessageHandler messageHandler, Guid bundleGuid)
        {
            this.fileExe = fileExe;
            this.messageHandler = messageHandler;

            this.stream = File.Open(fileExe, FileMode.Open, FileAccess.ReadWrite);

            WriteWixburnData(bundleGuid);
        }

        public void Dispose()
        {
            this.stream.Close();
            this.stream.Dispose();
            // TODO: There's a FXCop warning about not calling:
            //GC.SuppressFinalize(this);
            // ... but I'm not sure what else this might imply.
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
            // TODO: Would we want to catch cases where AppendContainer has already
            // been called?  If it was, we're zeroing out the ux/attached container
            // fields, which might be unexpected.
            BurnWriter.WriteUInt32(bytes, BURN_SECTION_OFFSET_UXADDRESS, 0);
            BurnWriter.WriteUInt32(bytes, BURN_SECTION_OFFSET_UXSIZE, 0);
            BurnWriter.WriteUInt32(bytes, BURN_SECTION_OFFSET_ATTACHEDCONTAINERADDRESS, 0);
            BurnWriter.WriteUInt32(bytes, BURN_SECTION_OFFSET_ATTACHEDCONTAINERSIZE, 0);

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
        public bool AppendContainer(string fileContainer, Container container)
        {
            UInt32 burnSectionOffsetAddress = 0;
            UInt32 burnSectionOffsetSize = 0;

            switch (container)
            {
                case Container.UX:
                    burnSectionOffsetAddress = BURN_SECTION_OFFSET_UXADDRESS;
                    burnSectionOffsetSize = BURN_SECTION_OFFSET_UXSIZE;
                    break;

                case Container.Attached:
                    burnSectionOffsetAddress = BURN_SECTION_OFFSET_ATTACHEDCONTAINERADDRESS;
                    burnSectionOffsetSize = BURN_SECTION_OFFSET_ATTACHEDCONTAINERSIZE;
                    break;

                default:
                    Debug.Assert(false);
                    return false;
            }

            return AppendContainer(fileContainer, burnSectionOffsetAddress, burnSectionOffsetSize);
        }

        /// <summary>
        /// Appends a container to the exe and updates the ".wixburn" section data to point to it.
        /// </summary>
        /// <param name="fileContainer">File to append to the current exe.</param>
        /// <param name="burnSectionOffsetAddress">Offset of address field for this container in ".wixburn" section data.</param>
        /// <param name="burnSectionOffsetSize">Offset of size field for this container in ".wixburn" section data.</param>
        /// <returns>true if the container data is successfully appended; false otherwise</returns>
        private bool AppendContainer(string fileContainer, UInt32 burnSectionOffsetAddress, UInt32 burnSectionOffsetSize)
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
                UInt32 length = (UInt32)reader.Length;
                BurnWriter.WriteUInt32(bytes, burnSectionOffsetAddress, (UInt32)this.stream.Length);
                BurnWriter.WriteUInt32(bytes, burnSectionOffsetSize, length);
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
        /// Checks for a valid Windows PE signature (IMAGE_NT_SIGNATURE) in the current exe.
        /// </summary>
        /// <returns>true if the exe is a Windows executable; false otherwise</returns>
        private bool EnsureNTHeader()
        {
            if (UInt32.MaxValue == this.firstSectionOffset)
            {
                if (!EnsureDosHeader())
                {
                    return false;
                }

                byte[] bytes = this.ReadBytes(this.peOffset, IMAGE_NT_HEADER_SIZE);

                // Verify the NT signature...
                if (IMAGE_NT_SIGNATURE != BurnWriter.ReadUInt32(bytes, IMAGE_NT_HEADER_OFFSET_SIGNATURE))
                {
                    this.messageHandler.OnMessage(WixErrors.InvalidStubExe(this.fileExe));
                    return false;
                }

                this.sections = BurnWriter.ReadUInt16(bytes, IMAGE_NT_HEADER_OFFSET_NUMBEROFSECTIONS);
                this.firstSectionOffset = this.peOffset + IMAGE_NT_HEADER_SIZE + BurnWriter.ReadUInt16(bytes, IMAGE_NT_HEADER_OFFSET_SIZEOFOPTIONALHEADER);
            }

            return true;
        }

        /// <summary>
        /// Checks for a valid DOS header in the current exe.
        /// </summary>
        /// <returns>true if the exe starts with a DOS stub; false otherwise</returns>
        private bool EnsureDosHeader()
        {
            if (UInt32.MaxValue == this.peOffset)
            {
                byte[] bytes = this.ReadBytes(0, IMAGE_DOS_HEADER_SIZE);

                // Verify the DOS 'MZ' signature.
                if (IMAGE_DOS_SIGNATURE != BurnWriter.ReadUInt16(bytes, IMAGE_DOS_HEADER_OFFSET_MAGIC))
                {
                    this.messageHandler.OnMessage(WixErrors.InvalidStubExe(this.fileExe));
                    return false;
                }

                this.peOffset = BurnWriter.ReadUInt32(bytes, IMAGE_DOS_HEADER_OFFSET_NTHEADER);
            }

            return true;
        }

        /// <summary>
        /// Reads bytes from a particular offset.
        /// </summary>
        /// <param name="offset">Offset from which to read.</param>
        /// <param name="size">Number of bytes to read.</param>
        /// <returns></returns>
        private byte[] ReadBytes(UInt32 offset, UInt32 size)
        {
            byte[] bytes = new byte[size];
            this.stream.Seek(offset, SeekOrigin.Begin);
            this.stream.Read(bytes, 0, bytes.Length);
            return bytes;
        }

        /// <summary>
        /// Writes bytes to a particular offset.  All of the bytes in the
        /// array are written.
        /// </summary>
        /// <param name="offset">Offset to write to.</param>
        /// <param name="bytes">Bytes to write.</param>
        private void WriteBytes(UInt32 offset, byte[] bytes)
        {
            this.stream.Seek(offset, SeekOrigin.Begin);
            this.stream.Write(bytes, 0, bytes.Length);
        }

        /// <summary>
        /// Reads a UInt16 value in little-endian format from an offset in an array of bytes.
        /// </summary>
        /// <param name="bytes">Array from which to read.</param>
        /// <param name="offset">Beginning offset from which to read.</param>
        /// <returns>value at offset</returns>
        private static UInt16 ReadUInt16(byte[] bytes, UInt32 offset)
        {
            Debug.Assert(offset + 2 <= bytes.Length);
            return (UInt16)(bytes[offset] + (bytes[offset + 1] << 8));
        }

        /// <summary>
        /// Reads a UInt32 value in little-endian format from an offset in an array of bytes.
        /// </summary>
        /// <param name="bytes">Array from which to read.</param>
        /// <param name="offset">Beginning offset from which to read.</param>
        /// <returns>value at offset</returns>
        private static UInt32 ReadUInt32(byte[] bytes, UInt32 offset)
        {
            Debug.Assert(offset + 4 <= bytes.Length);
            return (UInt32)(bytes[offset] + (bytes[offset + 1] << 8) + (bytes[offset + 2] << 16) + (bytes[offset + 3] << 24));
        }

        /// <summary>
        /// Reads a UInt64 value in little-endian format from an offset in an array of bytes.
        /// </summary>
        /// <param name="bytes">Array from which to read.</param>
        /// <param name="offset">Beginning offset from which to read.</param>
        /// <returns>value at offset</returns>
        private static UInt64 ReadUInt64(byte[] bytes, UInt32 offset)
        {
            Debug.Assert(offset + 8 <= bytes.Length);
            return BurnWriter.ReadUInt32(bytes, offset) + ((UInt64)(BurnWriter.ReadUInt32(bytes, offset + 4)) << 32);
        }

        /// <summary>
        /// Writes a UInt32 value to an offset in an array of bytes.
        /// </summary>
        /// <param name="bytes">Array to write to.</param>
        /// <param name="offset">Beginning offset at which to write.</param>
        /// <param name="value">value to write</param>
        private static void WriteUInt32(byte[] bytes, UInt32 offset, UInt32 value)
        {
            Debug.Assert(offset + 4 <= bytes.Length);
            bytes[offset] = (byte)(value & 0xff);
            bytes[offset + 1] = (byte)((value >> 8) & 0xff);
            bytes[offset + 2] = (byte)((value >> 16) & 0xff);
            bytes[offset + 3] = (byte)((value >> 24) & 0xff);
        }
    }
}
