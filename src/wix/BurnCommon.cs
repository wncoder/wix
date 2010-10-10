//-------------------------------------------------------------------------------------------------
// <copyright file="BurnCommon.cs" company="Microsoft">
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
// Burn PE writer for the Windows Installer Xml toolset.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml
{
    using System;
    using System.Diagnostics;
    using System.IO;

    /// <summary>
    /// Common functionality for Burn PE Writer & Reader for the Windows Installer Xml toolset.
    /// </summary>
    /// <remarks>This class encapsulates common functionality related to 
    /// bundled/chained setup packages.</remarks>
    /// <example>
    /// </example>
    class BurnCommon : IDisposable
    {
        public const string BurnNamespace = "http://schemas.microsoft.com/wix/2008/Burn";
        public const string BurnUXContainerEmbeddedIdFormat = "u{0}";
        public const string BurnUXContainerPayloadIdFormat = "p{0}";
        public const string BurnAttachedContainerEmbeddedIdFormat = "a{0}";

        // See WinNT.h for details about the PE format, including the
        // structure and offsets for IMAGE_DOS_HEADER, IMAGE_NT_HEADERS32,
        // IMAGE_FILE_HEADER, etc.
        protected const UInt32 IMAGE_DOS_HEADER_SIZE = 64;
        protected const UInt32 IMAGE_DOS_HEADER_OFFSET_MAGIC = 0;
        protected const UInt32 IMAGE_DOS_HEADER_OFFSET_NTHEADER = 60;

        protected const UInt32 IMAGE_NT_HEADER_SIZE = 24; // signature DWORD (4) + IMAGE_FILE_HEADER (20)
        protected const UInt32 IMAGE_NT_HEADER_OFFSET_SIGNATURE = 0;
        protected const UInt32 IMAGE_NT_HEADER_OFFSET_NUMBEROFSECTIONS = 6;
        protected const UInt32 IMAGE_NT_HEADER_OFFSET_SIZEOFOPTIONALHEADER = 20;

        protected const UInt32 IMAGE_SECTION_HEADER_SIZE = 40;
        protected const UInt32 IMAGE_SECTION_HEADER_OFFSET_NAME = 0;
        protected const UInt32 IMAGE_SECTION_HEADER_OFFSET_VIRTUALSIZE = 8;
        protected const UInt32 IMAGE_SECTION_HEADER_OFFSET_SIZEOFRAWDATA = 16;
        protected const UInt32 IMAGE_SECTION_HEADER_OFFSET_POINTERTORAWDATA = 20;

        protected const UInt16 IMAGE_DOS_SIGNATURE = 0x5A4D;
        protected const UInt32 IMAGE_NT_SIGNATURE = 0x00004550;
        protected const UInt64 IMAGE_SECTION_WIXBURN_NAME = 0x6E7275627869772E; // ".wixburn", as a qword.

        // The ".wixburn" section contains:
        //    0- 3:  magic number
        //    4- 7:  version
        //    8-11:  container count
        //   12-15:  container type (1 = CAB)
        //   16-23:  offset to start of manifest + UX container
        //   24-31:  byte count of manifest + UX container
        //   32-39:  offset to start of attached container
        //   40-47:  byte count of attached container
        //   48-63:  bundle GUID
        protected const UInt32 BURN_SECTION_OFFSET_MAGIC = 0;
        protected const UInt32 BURN_SECTION_OFFSET_VERSION = 4;
        protected const UInt32 BURN_SECTION_OFFSET_COUNT = 8;
        protected const UInt32 BURN_SECTION_OFFSET_FORMAT = 12;
        protected const UInt32 BURN_SECTION_OFFSET_UXADDRESS = 16;
        protected const UInt32 BURN_SECTION_OFFSET_UXSIZE = 24;
        protected const UInt32 BURN_SECTION_OFFSET_ATTACHEDCONTAINERADDRESS = 32;
        protected const UInt32 BURN_SECTION_OFFSET_ATTACHEDCONTAINERSIZE = 40;
        protected const UInt32 BURN_SECTION_OFFSET_BUNDLEGUID = 48;
        protected const UInt32 BURN_SECTION_SIZE = BURN_SECTION_OFFSET_BUNDLEGUID + 16 /*sizeof(GUID) - last field */;

        protected const UInt32 BURN_SECTION_MAGIC = 0x00f14300;
        protected const UInt32 BURN_SECTION_VERSION = 0x00000001;

        protected string fileExe;
        protected IMessageHandler messageHandler;
        protected FileStream stream;
        protected UInt32 peOffset = UInt32.MaxValue;
        protected UInt16 sections = UInt16.MaxValue;
        protected UInt32 firstSectionOffset = UInt32.MaxValue;
        protected UInt32 wixburnDataOffset = UInt32.MaxValue;

        // TODO: does this enum exist in another form somewhere?
        /// <summary>
        /// The types of attached containers that BurnWriter supports.
        /// </summary>
        public enum Container
        {
            Nothing = 0,
            UX,
            Attached
        }

        /// <summary>
        /// Creates a BurnCommon for re-writing a PE file.
        /// </summary>
        /// <param name="fileExe">File to modify in-place.</param>
        /// <param name="messageHandler">The messagehandler to report warnings/errors to.</param>
        /// <param name="bundleGuid">GUID for the bundle.</param>
        public BurnCommon(string fileExe, IMessageHandler messageHandler)
        {
            this.fileExe = fileExe;
            this.messageHandler = messageHandler;
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
        /// Checks for a valid Windows PE signature (IMAGE_NT_SIGNATURE) in the current exe.
        /// </summary>
        /// <returns>true if the exe is a Windows executable; false otherwise</returns>
        protected bool EnsureNTHeader()
        {
            if (UInt32.MaxValue == this.firstSectionOffset)
            {
                if (!EnsureDosHeader())
                {
                    return false;
                }

                byte[] bytes = this.ReadBytes(this.peOffset, IMAGE_NT_HEADER_SIZE);

                // Verify the NT signature...
                if (IMAGE_NT_SIGNATURE != BurnCommon.ReadUInt32(bytes, IMAGE_NT_HEADER_OFFSET_SIGNATURE))
                {
                    this.messageHandler.OnMessage(WixErrors.InvalidStubExe(this.fileExe));
                    return false;
                }

                this.sections = BurnCommon.ReadUInt16(bytes, IMAGE_NT_HEADER_OFFSET_NUMBEROFSECTIONS);
                this.firstSectionOffset = this.peOffset + IMAGE_NT_HEADER_SIZE + BurnCommon.ReadUInt16(bytes, IMAGE_NT_HEADER_OFFSET_SIZEOFOPTIONALHEADER);
            }

            return true;
        }

        /// <summary>
        /// Checks for a valid DOS header in the current exe.
        /// </summary>
        /// <returns>true if the exe starts with a DOS stub; false otherwise</returns>
        protected bool EnsureDosHeader()
        {
            if (UInt32.MaxValue == this.peOffset)
            {
                byte[] bytes = this.ReadBytes(0, IMAGE_DOS_HEADER_SIZE);

                // Verify the DOS 'MZ' signature.
                if (IMAGE_DOS_SIGNATURE != BurnCommon.ReadUInt16(bytes, IMAGE_DOS_HEADER_OFFSET_MAGIC))
                {
                    this.messageHandler.OnMessage(WixErrors.InvalidStubExe(this.fileExe));
                    return false;
                }

                this.peOffset = BurnCommon.ReadUInt32(bytes, IMAGE_DOS_HEADER_OFFSET_NTHEADER);
            }

            return true;
        }

        /// <summary>
        /// Reads bytes from a particular offset.
        /// </summary>
        /// <param name="offset">Offset from which to read.</param>
        /// <param name="size">Number of bytes to read.</param>
        /// <returns></returns>
        protected byte[] ReadBytes(UInt32 offset, UInt32 size)
        {
            byte[] bytes = new byte[size];
            this.stream.Seek(offset, SeekOrigin.Begin);
            this.stream.Read(bytes, 0, bytes.Length);
            return bytes;
        }

        /// <summary>
        /// Reads bytes from a particular offset (64-bit version).
        /// </summary>
        /// <param name="offset">Offset from which to read.</param>
        /// <param name="size">Number of bytes to read.</param>
        /// <returns></returns>
        protected byte[] ReadBytes(UInt64 offset, UInt64 size)
        {
            byte[] bytes = new byte[size];
            this.stream.Seek(Convert.ToInt64(offset), SeekOrigin.Begin);
            this.stream.Read(bytes, 0, bytes.Length);
            return bytes;
        }

        /// <summary>
        /// Writes bytes to a particular offset.  All of the bytes in the
        /// array are written.
        /// </summary>
        /// <param name="offset">Offset to write to.</param>
        /// <param name="bytes">Bytes to write.</param>
        protected void WriteBytes(UInt32 offset, byte[] bytes)
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
        protected static UInt16 ReadUInt16(byte[] bytes, UInt32 offset)
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
        protected static UInt32 ReadUInt32(byte[] bytes, UInt32 offset)
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
        protected static UInt64 ReadUInt64(byte[] bytes, UInt32 offset)
        {
            Debug.Assert(offset + 8 <= bytes.Length);
            return BurnCommon.ReadUInt32(bytes, offset) + ((UInt64)(BurnCommon.ReadUInt32(bytes, offset + 4)) << 32);
        }

        /// <summary>
        /// Writes a UInt32 value to an offset in an array of bytes.
        /// </summary>
        /// <param name="bytes">Array to write to.</param>
        /// <param name="offset">Beginning offset at which to write.</param>
        /// <param name="value">value to write</param>
        protected static void WriteUInt32(byte[] bytes, UInt32 offset, UInt32 value)
        {
            Debug.Assert(offset + 4 <= bytes.Length);
            bytes[offset] = (byte)(value & 0xff);
            bytes[offset + 1] = (byte)((value >> 8) & 0xff);
            bytes[offset + 2] = (byte)((value >> 16) & 0xff);
            bytes[offset + 3] = (byte)((value >> 24) & 0xff);
        }

        /// <summary>
        /// Writes a UInt64 value to an offset in an array of bytes.
        /// </summary>
        /// <param name="bytes">Array to write to.</param>
        /// <param name="offset">Beginning offset at which to write.</param>
        /// <param name="value">value to write</param>
        protected static void WriteUInt64(byte[] bytes, UInt32 offset, UInt64 value)
        {
            WriteUInt32(bytes, offset, (UInt32)(value & 0xFFFFFFFF));
            WriteUInt32(bytes, offset + 4, (UInt32)((value >> 32) & 0xFFFFFFFF));
        }
    }
}
