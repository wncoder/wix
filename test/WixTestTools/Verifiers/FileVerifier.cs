//-------------------------------------------------------------------------------------------------
// <copyright file="FileVerifier.cs" company="Microsoft">
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
//      Contains methods for verification for files and directories
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Verifiers
{
    using System;
    using System.IO;
    using System.Security.Cryptography;
    using Microsoft.VisualStudio.TestTools.UnitTesting;

    /// <summary>
    /// The FileVerifier contains methods for verification for files and directories
    /// </summary>
    public class FileVerifier
    {
        /// <summary>
        /// Computes the SHA1 hash for a file and returns the output formated as a string.
        /// </summary>
        /// <param name="filePath">File to compute the SHA1 hash for.</param>
        /// <returns>String representation of the SHA1 hash of the input file.</returns>
        public static string ComputeFileSHA1Hash(string filePath)
        {
            SHA1 sha = new SHA1CryptoServiceProvider();
            byte[] result = sha.ComputeHash(new FileStream(filePath, FileMode.Open,FileAccess.Read));

            // convert the byte array into string
            string hash = string.Empty;
            foreach (byte value in result)
            {
                hash += string.Format("{0:X2}", value);
            }

            return hash;
        }

        /// <summary>
        /// Verify two files are identical, though comparing hashes.
        /// </summary>
        /// <param name="file1Path">Path to the first file</param>
        /// <param name="file2Path">Path to the second file</param>
        public static void VerifyFilesAreIdentical(string file1Path, string file2Path)
        {
            string fileHash1 = ComputeFileSHA1Hash(file1Path);
            string fileHash2 = ComputeFileSHA1Hash(file2Path);

            Assert.AreEqual(fileHash1, fileHash2, "Files '{0}' and '{1}' have diffrent hash values. The files are not identical.", file1Path, file2Path);
        }
    }
}