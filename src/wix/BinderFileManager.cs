//-------------------------------------------------------------------------------------------------
// <copyright file="BinderFileManager.cs" company="Microsoft">
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
// 
// <summary>
// The base binder file manager.  Any of these methods can be overridden to change
// the behavior of the binder.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml
{
    using System;
    using System.Collections;
    using System.Collections.Specialized;
    using System.Diagnostics.CodeAnalysis;
    using System.Globalization;
    using System.IO;

    /// <summary>
    /// Options for building the cabinet.
    /// </summary>
    public enum CabinetBuildOption
    {
        /// <summary>
        /// Build the cabinet and move it to the target location.
        /// </summary>
        BuildAndMove,

        /// <summary>
        /// Build the cabinet and copy it to the target location.
        /// </summary>
        BuildAndCopy,

        /// <summary>
        /// Just copy the cabinet to the target location.
        /// </summary>
        Copy
    }

    /// <summary>
    /// Base class for creating a binder file manager.
    /// </summary>
    public class BinderFileManager
    {
        private StringCollection bindPaths;
        private NameValueCollection namedBindPaths;
        private string cabCachePath;
        private Output output;
        private bool reuseCabinets;
        private StringCollection sourcePaths;
        private SubStorage activeSubstorage;
        private bool deltaBinaryPatch;
        private string tempFilesLocation;

        /// <summary>
        /// Instantiate a new BinderFileManager.
        /// </summary>
        public BinderFileManager()
        {
            this.bindPaths = new StringCollection();
            this.sourcePaths = new StringCollection();
            this.namedBindPaths = new NameValueCollection();
        }

        /// <summary>
        /// Gets the bind paths to locate files.
        /// </summary>
        /// <value>The bind paths to locate files.</value>
        public StringCollection BindPaths
        {
            get { return this.bindPaths; }
        }

        /// <summary>
        /// Gets the named paths to locate files.
        /// </summary>
        /// <value>The named bind paths to locate files.</value>
        public NameValueCollection NamedBindPaths
        {
            get { return this.namedBindPaths; }
        }

        /// <summary>
        /// Gets or sets the path to cabinet cache.
        /// </summary>
        /// <value>The path to cabinet cache.</value>
        public string CabCachePath
        {
            get { return this.cabCachePath; }
            set { this.cabCachePath = value; }
        }

        /// <summary>
        /// Gets or sets the output object used for binding.
        /// </summary>
        /// <value>The output object.</value>
        public Output Output
        {
            get { return this.output; }
            set { this.output = value; }
        }

        /// <summary>
        /// Gets or sets the option to reuse cabinets in the cache.
        /// </summary>
        /// <value>The option to reuse cabinets in the cache.</value>
        public bool ReuseCabinets
        {
            get { return this.reuseCabinets; }
            set { this.reuseCabinets = value; }
        }

        /// <summary>
        /// Gets the collection of all source paths to intermediate files.
        /// </summary>
        /// <value>The collection of all source paths to intermediate files.</value>
        public StringCollection SourcePaths
        {
            get { return this.sourcePaths; }
        }

        /// <summary>
        /// Gets or sets the active subStorage used for binding.
        /// </summary>
        /// <value>The subStorage object.</value>
        public SubStorage ActiveSubStorage
        {
            get { return this.activeSubstorage; }
            set { this.activeSubstorage = value; }
        }

        /// <summary>
        /// Gets or sets the option to enable building binary delta patches.
        /// </summary>
        /// <value>The option to enable building binary delta patches.</value>
        public bool DeltaBinaryPatch
        {
            get { return this.deltaBinaryPatch; }
            set { this.deltaBinaryPatch = value; }
        }

        /// <summary>
        /// Gets or sets the path to the temp files location.
        /// </summary>
        /// <value>The path to the temp files location.</value>
        public string TempFilesLocation
        {
            get { return this.tempFilesLocation; }
            set { this.tempFilesLocation = value; }
        }

        /// <summary>
        /// Compares two files to determine if they are equivalent.
        /// </summary>
        /// <param name="targetFile">The target file.</param>
        /// <param name="updatedFile">The updated file.</param>
        /// <returns>true if the files are equal; false otherwise.</returns>
        public virtual bool CompareFiles(string targetFile, string updatedFile)
        {
            FileInfo targetFileInfo = new FileInfo(targetFile);
            FileInfo updatedFileInfo = new FileInfo(updatedFile);

            if (targetFileInfo.Length != updatedFileInfo.Length)
            {
                return false;
            }

            using (FileStream targetStream = File.OpenRead(targetFile))
            {
                using (FileStream updatedStream = File.OpenRead(updatedFile))
                {
                    if (targetStream.Length != updatedStream.Length)
                    {
                        return false;
                    }

                    // Using a larger buffer than the default buffer of 4 * 1024 used by FileStream.ReadByte improves performance.
                    // The buffer size is based on user feedback. Based on performance results, a better buffer size may be determined.
                    byte[] targetBuffer = new byte[16 * 1024];
                    byte[] updatedBuffer = new byte[16 * 1024];
                    int targetReadLength;
                    int updatedReadLength;
                    do
                    {
                        targetReadLength = targetStream.Read(targetBuffer, 0, targetBuffer.Length);
                        updatedReadLength = updatedStream.Read(updatedBuffer, 0, updatedBuffer.Length);
                        
                        if (targetReadLength != updatedReadLength)
                        {
                            return false;
                        }

                        for (int i = 0; i < targetReadLength; ++i)
                        {
                            if (targetBuffer[i] != updatedBuffer[i])
                            {
                                return false;
                            }
                        }

                    } while (0 < targetReadLength);
                }
            }

            return true;
        }

        /// <summary>
        /// Resolves the source path of a file.
        /// </summary>
        /// <param name="source">Original source value.</param>
        /// <returns>Should return a valid path for the stream to be imported.</returns>
        public virtual string ResolveFile(string source)
        {
            if (String.IsNullOrEmpty(source))
            {
                throw new ArgumentNullException("source");
            }

            string filePath = null;
            const string bindPathOpenString = "!(bindpath.";

            if (source.StartsWith(bindPathOpenString, StringComparison.Ordinal) && source.IndexOf(')') != -1)
            {
                int bindpathSignatureLength = bindPathOpenString.Length;
                string name = source.Substring(bindpathSignatureLength, source.IndexOf(')') - bindpathSignatureLength);
                string[] values = this.namedBindPaths.GetValues(name);

                if (null != values)
                {
                    foreach (string bindPath in values)
                    {
                        // Parse out '\\' chars that separate the "bindpath" variable and the next part of the path, 
                        // because Path.Combine() thinks that rooted second paths don't need the first path.
                        string nameSection = string.Empty;
                        int nameStart = bindpathSignatureLength + 1 + name.Length;  // +1 for the closing bracket.

                        nameSection = source.Substring(nameStart).TrimStart('\\');
                        filePath = Path.Combine(bindPath, nameSection);

                        if (File.Exists(filePath))
                        {
                            return filePath;
                        }
                    }
                }
            }

            if (source.StartsWith("SourceDir\\", StringComparison.Ordinal) || source.StartsWith("SourceDir/", StringComparison.Ordinal))
            {
                foreach (string bindPath in this.bindPaths)
                {
                    filePath = Path.Combine(bindPath, source.Substring(10));
                    if (File.Exists(filePath))
                    {
                        return filePath;
                    }
                }
            }

            try
            {
                if (Path.IsPathRooted(source) || File.Exists(source))
                {
                    return source;
                }
            }
            catch (ArgumentException)
            {
                throw new WixException(WixErrors.IllegalCharactersInPath(source));
            }

            foreach (string path in this.sourcePaths)
            {
                filePath = Path.Combine(path, source);
                if (File.Exists(filePath))
                {
                    return filePath;
                }

                if (source.StartsWith("SourceDir\\", StringComparison.Ordinal) || source.StartsWith("SourceDir/", StringComparison.Ordinal))
                {
                    filePath = Path.Combine(path, source.Substring(10));
                    try
                    {
                        if (File.Exists(filePath))
                        {
                            return filePath;
                        }
                    }
                    catch (ArgumentException)
                    {
                        throw new WixException(WixErrors.IllegalCharactersInPath(source));
                    }
                }
            }

            throw new WixFileNotFoundException(source);
        }

        /// <summary>
        /// Resolves the source path of a cabinet file.
        /// </summary>
        /// <param name="fileRows">Collection of files in this cabinet.</param>
        /// <param name="cabinetPath">Path to cabinet to generate.  Path may be modified by delegate.</param>
        /// <returns>The CabinetBuildOption.  By default the cabinet is built and moved to its target location.</returns>
        [SuppressMessage("Microsoft.Design", "CA1045:DoNotPassTypesByReference")]
        public virtual CabinetBuildOption ResolveCabinet(FileRowCollection fileRows, ref string cabinetPath)
        {
            if (fileRows == null)
            {
                throw new ArgumentNullException("fileRows");
            }

            // no special behavior specified, use the default
            if (null == this.cabCachePath && !this.reuseCabinets)
            {
                return CabinetBuildOption.BuildAndMove;
            }

            // if a cabinet cache path was provided, change the location for the cabinet
            // to be built to
            if (null != this.cabCachePath)
            {
                string cabinetName = Path.GetFileName(cabinetPath);
                cabinetPath = Path.Combine(this.cabCachePath, cabinetName);
            }

            // if we still think we're going to reuse the cabinet check to see if the cabinet exists first
            if (this.reuseCabinets)
            {
                bool cabinetExists = false;

                if (File.Exists(cabinetPath))
                {
                    // check to see if
                    // 1. any files are added or removed
                    // 2. order of files changed or names changed
                    // 3. modified time changed
                    cabinetExists = true;

                    // Need to force garbage collection of WixEnumerateCab to ensure the handle
                    // associated with it is closed before it is reused.
                    using (Cab.WixEnumerateCab wixEnumerateCab = new Cab.WixEnumerateCab())
                    {
                        ArrayList fileList = wixEnumerateCab.Enumerate(cabinetPath);

                        if (fileRows.Count != fileList.Count)
                        {
                            cabinetExists = false;
                        }
                        else
                        {
                            int i = 0;
                            foreach (FileRow fileRow in fileRows)
                            {
                                CabinetFileInfo fileInfo = fileList[i] as CabinetFileInfo;
                                DateTime fileTime = File.GetLastWriteTime(fileRow.Source);

                                ushort cabDate;
                                ushort cabTime;
                                Cab.Interop.CabInterop.DateTimeToCabDateAndTime(fileTime, out cabDate, out cabTime);
                                if (fileRow.File != fileInfo.FileId || fileInfo.Date != cabDate || fileInfo.Time != cabTime)
                                {
                                    cabinetExists = false;
                                    break;
                                }
                                i++;
                            }
                        }
                    }
                }

                return (cabinetExists ? CabinetBuildOption.Copy : CabinetBuildOption.BuildAndCopy);
            }
            else // by default move the built cabinet
            {
                return CabinetBuildOption.BuildAndMove;
            }
        }

        /// <summary>
        /// Resolve the layout path of a media.
        /// </summary>
        /// <param name="mediaRow">The media's row.</param>
        /// <param name="layoutDirectory">The layout directory for the setup image.</param>
        /// <returns>The layout path for the media.</returns>
        public virtual string ResolveMedia(MediaRow mediaRow, string layoutDirectory)
        {
            if (mediaRow == null)
            {
                throw new ArgumentNullException("mediaRow");
            }

            string mediaLayoutDirectory = mediaRow.Layout;

            if (null == mediaLayoutDirectory)
            {
                mediaLayoutDirectory = layoutDirectory;
            }
            else if (!Path.IsPathRooted(mediaLayoutDirectory))
            {
                mediaLayoutDirectory = Path.Combine(layoutDirectory, mediaLayoutDirectory);
            }

            return mediaLayoutDirectory;
        }

        /// <summary>
        /// Resolves the URL to a file.
        /// </summary>
        /// <param name="url">URL that may be a format string for the id and fileName.</param>
        /// <param name="packageId">Identity of the package (if payload is not part of a package) the URL points to. NULL if not part of a package.</param>
        /// <param name="payloadId">Identity of the payload the URL points to.</param>
        /// <param name="fileName">File name the URL points at.</param>
        /// <param name="fallbackUrl">Optional URL to use if the URL provided is empty.</param>
        /// <returns>An absolute URL or null if no URL is provided.</returns>
        public virtual string ResolveUrl(string url, string fallbackUrl, string packageId, string payloadId, string fileName)
        {
            // If a URL was not specified but there is a fallback URL that has a format specifier in it
            // then use the fallback URL formatter for this URL.
            if (String.IsNullOrEmpty(url) && !String.IsNullOrEmpty(fallbackUrl))
            {
                string formattedFallbackUrl = String.Format(fallbackUrl, packageId, payloadId, fileName);
                if (!String.Equals(fallbackUrl, formattedFallbackUrl, StringComparison.OrdinalIgnoreCase))
                {
                    url = fallbackUrl;
                }
            }

            if (!String.IsNullOrEmpty(url))
            {
                string formattedUrl = String.Format(url, packageId, payloadId, fileName);

                Uri canonicalUri;
                if (Uri.TryCreate(formattedUrl, UriKind.Absolute, out canonicalUri))
                {
                    url = canonicalUri.AbsoluteUri;
                }
                else
                {
                    url = null;
                }
            }

            return url;
        }

        /// <summary>
        /// Copies a file.
        /// </summary>
        /// <param name="source">The file to copy.</param>
        /// <param name="destination">The destination file.</param>
        /// <param name="overwrite">true if the destination file can be overwritten; otherwise, false.</param>
        public virtual void CopyFile(string source, string destination, bool overwrite)
        {
            File.Copy(source, destination, overwrite);
        }

        /// <summary>
        /// Moves a file.
        /// </summary>
        /// <param name="source">The file to move.</param>
        /// <param name="destination">The destination file.</param>
        public virtual void MoveFile(string source, string destination)
        {
            File.Move(source, destination);
        }

        /// <summary>
        /// Create patch if needed. This runs in the cabinet building thread.
        /// </summary>
        /// <param name="fileRow">The FileRow of the file to create the delta for.</param>
        /// <param name="retainRangeWarning">true if the retain ranges were ignored to mismatches.</param>
        [SuppressMessage("Microsoft.Design", "CA1021:AvoidOutParameters")]
        public virtual void ResolvePatch(FileRow fileRow, out bool retainRangeWarning)
        {
            if (fileRow == null)
            {
                throw new ArgumentNullException("fileRow");
            }

            retainRangeWarning = false;
            if (this.deltaBinaryPatch && RowOperation.Modify == fileRow.Operation)
            {
                if (0 != (PatchAttributeType.IncludeWholeFile | fileRow.PatchAttributes))
                {
                    string deltaBase = String.Concat(fileRow.PatchGroup.ToString(CultureInfo.InvariantCulture), fileRow.File);
                    string deltaFile = Path.Combine(this.tempFilesLocation, String.Concat(deltaBase, ".dpf"));
                    string headerFile = Path.Combine(this.tempFilesLocation, String.Concat(deltaBase, ".phd"));
                    PatchAPI.PatchInterop.PatchSymbolFlagsType apiPatchingSymbolFlags = 0;
                    bool optimizePatchSizeForLargeFiles = false;

                    Table wixPatchIdTable = this.output.Tables["WixPatchId"];
                    if (null != wixPatchIdTable)
                    {
                        Row row = wixPatchIdTable.Rows[0];
                        if (null != row)
                        {
                            if (null != row[2])
                            {
                                optimizePatchSizeForLargeFiles = (1 == Convert.ToUInt32(row[2], CultureInfo.InvariantCulture));
                            }
                            if (null != row[3])
                            {
                                apiPatchingSymbolFlags = (PatchAPI.PatchInterop.PatchSymbolFlagsType)Convert.ToUInt32(row[3], CultureInfo.InvariantCulture);
                            }
                        }
                    }

                    if (PatchAPI.PatchInterop.CreateDelta(
                            deltaFile,
                            fileRow.Source,
                            fileRow.Symbols,
                            fileRow.RetainOffsets,
                            fileRow.PreviousSourceArray,
                            fileRow.PreviousSymbolsArray,
                            fileRow.PreviousIgnoreLengthsArray,
                            fileRow.PreviousIgnoreOffsetsArray,
                            fileRow.PreviousRetainLengthsArray,
                            fileRow.PreviousRetainOffsetsArray,
                            apiPatchingSymbolFlags,
                            optimizePatchSizeForLargeFiles,
                            out retainRangeWarning))
                    {
                        PatchAPI.PatchInterop.ExtractDeltaHeader(deltaFile, headerFile);
                        fileRow.Patch = headerFile;
                        fileRow.Source = deltaFile;
                    }
                }
            }
        }
    }
}
