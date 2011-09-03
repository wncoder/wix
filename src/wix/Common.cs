//-------------------------------------------------------------------------------------------------
// <copyright file="Common.cs" company="Microsoft">
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
// Common Wix utility methods and types.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml
{
    using System;
    using System.Diagnostics.CodeAnalysis;
    using System.Globalization;
    using System.IO;
    using System.Security.Cryptography;
    using System.Text;
    using System.Text.RegularExpressions;

    /// <summary>
    /// Common Wix utility methods and types.
    /// </summary>
    internal sealed class Common
    {
        //-------------------------------------------------------------------------------------------------
        // Layout of an Access Mask (from http://technet.microsoft.com/en-us/library/cc783530(WS.10).aspx)
        //
        //  -------------------------------------------------------------------------------------------------
        //  |31|30|29|28|27|26|25|24|23|22|21|20|19|18|17|16|15|14|13|12|11|10|09|08|07|06|05|04|03|02|01|00|
        //  -------------------------------------------------------------------------------------------------
        //  |GR|GW|GE|GA| Reserved  |AS|StandardAccessRights|        Object-Specific Access Rights          |
        //
        //  Key
        //  GR = Generic Read
        //  GW = Generic Write
        //  GE = Generic Execute
        //  GA = Generic All
        //  AS = Right to access SACL
        //
        // TODO: what is the expected decompile behavior if a bit is found that is not explicitly enumerated
        //
        //-------------------------------------------------------------------------------------------------
        // Generic Access Rights (per WinNT.h)
        // ---------------------
        // GENERIC_ALL                      (0x10000000L)
        // GENERIC_EXECUTE                  (0x20000000L)
        // GENERIC_WRITE                    (0x40000000L)
        // GENERIC_READ                     (0x80000000L)
        internal static readonly string[] GenericPermissions = { "GenericAll", "GenericExecute", "GenericWrite", "GenericRead" };

        // Standard Access Rights (per WinNT.h)
        // ----------------------
        // DELETE                           (0x00010000L)
        // READ_CONTROL                     (0x00020000L)
        // WRITE_DAC                        (0x00040000L)
        // WRITE_OWNER                      (0x00080000L)
        // SYNCHRONIZE                      (0x00100000L)
        internal static readonly string[] StandardPermissions = { "Delete", "ReadPermission", "ChangePermission", "TakeOwnership", "Synchronize" };

        // Object-Specific Access Rights
        // =============================
        // Directory Access Rights (per WinNT.h)
        // -----------------------
        // FILE_LIST_DIRECTORY       ( 0x0001 )
        // FILE_ADD_FILE             ( 0x0002 )
        // FILE_ADD_SUBDIRECTORY     ( 0x0004 )
        // FILE_READ_EA              ( 0x0008 )
        // FILE_WRITE_EA             ( 0x0010 )
        // FILE_TRAVERSE             ( 0x0020 )
        // FILE_DELETE_CHILD         ( 0x0040 )
        // FILE_READ_ATTRIBUTES      ( 0x0080 )
        // FILE_WRITE_ATTRIBUTES     ( 0x0100 )
        internal static readonly string[] FolderPermissions = { "Read", "CreateFile", "CreateChild", "ReadExtendedAttributes", "WriteExtendedAttributes", "Traverse", "DeleteChild", "ReadAttributes", "WriteAttributes" };

        // Registry Access Rights (per TODO)
        // ----------------------
        internal static readonly string[] RegistryPermissions = { "Read", "Write", "CreateSubkeys", "EnumerateSubkeys", "Notify", "CreateLink" };

        // File Access Rights (per WinNT.h)
        // ------------------
        // FILE_READ_DATA            ( 0x0001 )
        // FILE_WRITE_DATA           ( 0x0002 )
        // FILE_APPEND_DATA          ( 0x0004 )
        // FILE_READ_EA              ( 0x0008 )
        // FILE_WRITE_EA             ( 0x0010 )
        // FILE_EXECUTE              ( 0x0020 )
        // via mask FILE_ALL_ACCESS  ( 0x0040 )
        // FILE_READ_ATTRIBUTES      ( 0x0080 )
        // FILE_WRITE_ATTRIBUTES     ( 0x0100 )
        // 
        // STANDARD_RIGHTS_REQUIRED  (0x000F0000L)
        // FILE_ALL_ACCESS           (STANDARD_RIGHTS_REQUIRED | SYNCHRONIZE | 0x1FF)
        internal static readonly string[] FilePermissions = { "Read", "Write", "Append", "ReadExtendedAttributes", "WriteExtendedAttributes", "Execute", "FileAllRights", "ReadAttributes", "WriteAttributes" };

        internal static readonly Regex WixVariableRegex = new Regex(@"(\!|\$)\((?<namespace>loc|wix|bind|bindpath)\.(?<fullname>(?<name>[_A-Za-z][0-9A-Za-z_]+)(\.(?<scope>[_A-Za-z][0-9A-Za-z_\.]*))?)(\=(?<value>.+?))?\)", RegexOptions.Compiled | RegexOptions.Singleline | RegexOptions.ExplicitCapture);

        private static readonly Regex PropertySearch = new Regex(@"\[[#$!]?[a-zA-Z_][a-zA-Z0-9_\.]*]", RegexOptions.Singleline);
        private static readonly Regex AddPrefix = new Regex(@"^[^a-zA-Z_]", RegexOptions.Compiled);
        private static readonly Regex IllegalIdentifierCharacters = new Regex(@"[^A-Za-z0-9_\.]|\.{2,}", RegexOptions.Compiled); // non 'words' and assorted valid characters

        /// <summary>
        /// Protect the constructor.
        /// </summary>
        private Common()
        {
        }

        /// <summary>
        /// Cleans up the temp files.
        /// </summary>
        /// <param name="path">The temporary directory to delete.</param>
        /// <param name="messageHandler">The message handler.</param>
        /// <returns>True if all files were deleted, false otherwise.</returns>
        internal static bool DeleteTempFiles(string path, IMessageHandler messageHandler)
        {
            // try three times and give up with a warning if the temp files aren't gone by then
            int retryLimit = 3;
            bool removedReadOnly = false;

            for (int i = 0; i < retryLimit; i++)
            {
                try
                {
                    Directory.Delete(path, true);   // toast the whole temp directory
                    break; // no exception means we got success the first time
                }
                catch (UnauthorizedAccessException)
                {
                    if (!removedReadOnly) // should only need to unmark readonly once - there's no point in doing it again and again
                    {
                        removedReadOnly = true;
                        RecursiveFileAttributes(path, FileAttributes.ReadOnly, false); // toasting will fail if any files are read-only. Try changing them to not be.
                    }
                    else
                    {
                        messageHandler.OnMessage(WixWarnings.AccessDeniedForDeletion(null, path));
                        return false;
                    }
                }
                catch (DirectoryNotFoundException)
                {
                    // if the path doesn't exist, then there is nothing for us to worry about
                    break;
                }
                catch (IOException) // directory in use
                {
                    if (i == (retryLimit - 1)) // last try failed still, give up
                    {
                        messageHandler.OnMessage(WixWarnings.DirectoryInUse(null, path));
                        return false;
                    }
                    System.Threading.Thread.Sleep(300);  // sleep a bit before trying again
                }
            }

            return true;
        }

        /// <summary>
        /// Gets a valid code page from the given web name or integer value.
        /// </summary>
        /// <param name="value">A code page web name or integer value as a string.</param>
        /// <exception cref="ArgumentOutOfRangeException">The value is an integer less than 0 or greater than 65535.</exception>
        /// <exception cref="ArgumentNullException"><paramref name="value"/> is null.</exception>
        /// <exception cref="NotSupportedException">The value doesn't not represent a valid code page name or integer value.</exception>
        internal static int GetValidCodePage(string value)
        {
            return GetValidCodePage(value, false);
        }

        /// <summary>
        /// Gets a valid code page from the given web name or integer value.
        /// </summary>
        /// <param name="value">A code page web name or integer value as a string.</param>
        /// <param name="allowNoChange">Whether to allow -1 which does not change the database code pages. This may be the case with wxl files.</param>
        /// <returns>A valid code page number.</returns>
        /// <exception cref="ArgumentOutOfRangeException">The value is an integer less than 0 or greater than 65535.</exception>
        /// <exception cref="ArgumentNullException"><paramref name="value"/> is null.</exception>
        /// <exception cref="NotSupportedException">The value doesn't not represent a valid code page name or integer value.</exception>
        /// <exception cref="WixException">The code page is invalid for summary information.</exception>
        internal static int GetValidCodePage(string value, bool allowNoChange)
        {
            return GetValidCodePage(value, allowNoChange, false, null);
        }

        /// <summary>
        /// Gets a valid code page from the given web name or integer value.
        /// </summary>
        /// <param name="value">A code page web name or integer value as a string.</param>
        /// <param name="allowNoChange">Whether to allow -1 which does not change the database code pages. This may be the case with wxl files.</param>
        /// <param name="onlyAnsi">Whether to allow Unicode (UCS) or UTF code pages.</param>
        /// <param name="sourceLineNumbers">Source line information for the current authoring.</param>
        /// <returns>A valid code page number.</returns>
        /// <exception cref="ArgumentOutOfRangeException">The value is an integer less than 0 or greater than 65535.</exception>
        /// <exception cref="ArgumentNullException"><paramref name="value"/> is null.</exception>
        /// <exception cref="NotSupportedException">The value doesn't not represent a valid code page name or integer value.</exception>
        /// <exception cref="WixException">The code page is invalid for summary information.</exception>
        internal static int GetValidCodePage(string value, bool allowNoChange, bool onlyAnsi, SourceLineNumberCollection sourceLineNumbers)
        {
            int codePage;
            Encoding enc;

            if (null == value)
            {
                throw new ArgumentNullException("value");
            }

            try
            {
                // check if a integer as a string was passed
                if (int.TryParse(value, out codePage))
                {
                    if (0 == codePage)
                    {
                        // 0 represents a neutral database
                        return 0;
                    }
                    else if (allowNoChange && -1 == codePage)
                    {
                        // -1 means no change to the database code page
                        return -1;
                    }

                    enc = Encoding.GetEncoding(codePage);
                }
                else
                {
                    enc = Encoding.GetEncoding(value);
                }

                // Windows Installer parses some code page references
                // as unsigned shorts which fail to open the database.
                if (onlyAnsi)
                {
                    codePage = enc.CodePage;
                    if (0 > codePage || short.MaxValue < codePage)
                    {
                        throw new WixException(WixErrors.InvalidSummaryInfoCodePage(sourceLineNumbers, codePage));
                    }
                }

                return enc.CodePage;
            }
            catch (ArgumentException ex)
            {
                // rethrow as NotSupportedException since either can be thrown
                // if the system does not support the specified code page
                throw new NotSupportedException(ex.Message, ex);
            }
        }

        /// <summary>
        /// Verifies if an identifier is a valid binder variable name.
        /// </summary>
        /// <param name="name">Binder variable name to verify.</param>
        /// <returns>True if the identifier is a valid binder variable name.</returns>
        public static bool IsValidBinderVariable(string name)
        {
            if (String.IsNullOrEmpty(name))
            {
                return false;
            }

            Match match = Common.WixVariableRegex.Match(name);

            return (match.Success && ("bind" == match.Groups["namespace"].Value || "wix" == match.Groups["namespace"].Value) && 0 == match.Index && name.Length == match.Length);
        }

        /// <summary>
        /// Get the value of an attribute with type YesNoType.
        /// </summary>
        /// <param name="sourceLineNumbers">Source information for the value.</param>
        /// <param name="elementName">Name of the element for this attribute, used for a possible exception.</param>
        /// <param name="attributeName">Name of the attribute.</param>
        /// <param name="value">Value to process.</param>
        /// <returns>Returns true for a value of 'yes' and false for a value of 'no'.</returns>
        /// <exception cref="WixException">Thrown when the attribute's value is not 'yes' or 'no'.</exception>
        internal static bool IsYes(SourceLineNumberCollection sourceLineNumbers, string elementName, string attributeName, string value)
        {
            switch (value)
            {
                case "no":
                    return false;
                case "yes":
                    return true;
                default:
                    throw new WixException(WixErrors.IllegalAttributeValue(sourceLineNumbers, elementName, attributeName, value, "no", "yes"));
            }
        }

        /// <summary>
        /// Generate a new Windows Installer-friendly guid.
        /// </summary>
        /// <returns>A new guid.</returns>
        internal static string GenerateGuid()
        {
            return Guid.NewGuid().ToString("B").ToUpper(CultureInfo.InvariantCulture);
        }

        /// <summary>
        /// Generate an identifier by hashing data from the row.
        /// </summary>
        /// <param name="prefix">Three letter or less prefix for generated row identifier.</param>
        /// <param name="fipsCompliant">Tells the algorithm to hash with a FIPS compliant hash.</param>
        /// <param name="args">Information to hash.</param>
        /// <returns>The generated identifier.</returns>
        [SuppressMessage("Microsoft.Globalization", "CA1303:DoNotPassLiteralsAsLocalizedParameters", MessageId = "System.InvalidOperationException.#ctor(System.String)")]
        public static string GenerateIdentifier(string prefix, bool fipsCompliant, params string[] args)
        {
            string stringData = String.Join("|", args);
            byte[] data = Encoding.Unicode.GetBytes(stringData);

            // hash the data
            byte[] hash;

            if (fipsCompliant)
            {
                using (SHA1 sha1 = new SHA1CryptoServiceProvider())
                {
                    hash = sha1.ComputeHash(data);
                }
            }
            else
            {
                using (MD5 md5 = new MD5CryptoServiceProvider())
                {
                    hash = md5.ComputeHash(data);
                }
            }

            // build up the identifier
            StringBuilder identifier = new StringBuilder(35, 35);
            identifier.Append(prefix);

            // hard coded to 16 as that is the most bytes that can be used to meet the length requirements. SHA1 is 20 bytes.
            for (int i = 0; i < 16; i++)
            {
                identifier.Append(hash[i].ToString("X2", CultureInfo.InvariantCulture.NumberFormat));
            }

            return identifier.ToString();
        }

        /// <summary>
        /// Return an identifier based on passed file/directory name
        /// </summary>
        /// <param name="name">File/directory name to generate identifer from</param>
        /// <returns>A version of the name that is a legal identifier.</returns>
        internal static string GetIdentifierFromName(string name)
        {
            string result = IllegalIdentifierCharacters.Replace(name, "_"); // replace illegal characters with "_".

            // MSI identifiers must begin with an alphabetic character or an
            // underscore. Prefix all other values with an underscore.
            if (AddPrefix.IsMatch(name))
            {
                result = String.Concat("_", result);
            }

            return result;
        }

        /// <summary>
        /// Checks if the string contains a property (i.e. "foo[Property]bar")
        /// </summary>
        /// <param name="possibleProperty">String to evaluate for properties.</param>
        /// <returns>True if a property is found in the string.</returns>
        internal static bool ContainsProperty(string possibleProperty)
        {
            return PropertySearch.IsMatch(possibleProperty);
        }

        /// <summary>
        /// Recursively loops through a directory, changing an attribute on all of the underlying files.
        /// An example is to add/remove the ReadOnly flag from each file.
        /// </summary>
        /// <param name="path">The directory path to start deleting from.</param>
        /// <param name="fileAttribute">The FileAttribute to change on each file.</param>
        /// <param name="markAttribute">If true, add the attribute to each file. If false, remove it.</param>
        private static void RecursiveFileAttributes(string path, FileAttributes fileAttribute, bool markAttribute)
        {
            foreach (string subDirectory in Directory.GetDirectories(path))
            {
                RecursiveFileAttributes(subDirectory, fileAttribute, markAttribute);
            }

            foreach (string filePath in Directory.GetFiles(path))
            {
                FileAttributes attributes = File.GetAttributes(filePath);
                if (markAttribute)
                {
                    attributes = attributes | fileAttribute; // add to list of attributes
                }
                else if (fileAttribute == (attributes & fileAttribute)) // if attribute set
                {
                    attributes = attributes ^ fileAttribute; // remove from list of attributes
                }
                File.SetAttributes(filePath, attributes);
            }
        }

        internal static string GetFileHash(FileInfo fileInfo)
        {
            byte[] hashBytes;
            using (SHA1Managed managed = new SHA1Managed())
            {
                using (FileStream stream = fileInfo.OpenRead())
                {
                    hashBytes = managed.ComputeHash(stream);
                }
            }

            StringBuilder sb = new StringBuilder();
            for (int i = 0; i < hashBytes.Length; i++)
            {
                sb.AppendFormat("{0:X2}", hashBytes[i]);
            }

            return sb.ToString();
        }
    }
}
