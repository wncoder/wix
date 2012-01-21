//-----------------------------------------------------------------------
// <copyright file="TestException.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
//    
//    The use and distribution terms for this software are covered by the
//    Common Public License 1.0 (http://opensource.org/licenses/cpl1.0.php)
//    which can be found in the file CPL.TXT at the root of this distribution.
//    By using this software in any fashion, you are agreeing to be bound by
//    the terms of this license.
//    
//    You must not remove this notice, or any other, from this software.
// </copyright>
// <summary>Exception that occurs during a TestTool run</summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test
{
    using System;

    /// <summary>
    /// Exception that occurs during a TestTool run
    /// </summary>
    public class TestException : Exception
    {
        /// <summary>
        /// The result of executing the tool
        /// </summary>
        private Result result = null;

        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="message">The exception message</param>
        /// <param name="result">The result of executing the tool</param>
        public TestException(string message, Result result)
            : base(message)
        {
            this.result = result;
        }

        /// <summary>
        /// The result of executing the tool
        /// </summary>
        public Result Result
        {
            get { return this.result; }
        }
    }
}
