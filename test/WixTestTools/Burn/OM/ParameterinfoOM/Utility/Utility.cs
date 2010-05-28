//-----------------------------------------------------------------------
// <copyright file="Utility.cs" company="Microsoft">
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
// <summary>ParameterInfo OM specific utility</summary>
//-----------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;

namespace Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.Utility
{
    /// <summary>
    /// Helper function required for generating ParameterInfo xml
    /// </summary>
    public class Utility
    {
        /// <summary>
        /// To generate SHA1 hash value for Burn items like msi, msp, exe etc
        /// </summary>
        /// <param name="FileName">filename</param>
        /// <returns>SHA1 hash value</returns>
        public static string GetSHA1HashValue(string FileName)
        {
            string retValue = string.Empty;
            if (!String.IsNullOrEmpty(FileName))
            {
                retValue = System.BitConverter.ToString(System.Security.Cryptography.SHA1.Create().ComputeHash(System.IO.File.OpenRead(System.Environment.ExpandEnvironmentVariables(FileName)))).Replace("-", "");
            }
            return retValue;
        }

        /// <summary>
        /// To get disk file size
        /// </summary>
        /// <param name="FileName">filename</param>
        /// <returns>file size</returns>
        public static string GetFileSize(string FileName)
        {
            string downloadSize = "0";

            if (!String.IsNullOrEmpty(FileName))
            {
                FileInfo f = new FileInfo(System.Environment.ExpandEnvironmentVariables(FileName));
                downloadSize = f.Length.ToString();
            }
            return downloadSize;
        }
    }
}
