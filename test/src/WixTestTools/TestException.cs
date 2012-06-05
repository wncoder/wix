//-----------------------------------------------------------------------
// <copyright file="TestException.cs" company="Microsoft Corporation">
//   Copyright (c) 2004, Microsoft Corporation.
//   This software is released under Common Public License Version 1.0 (CPL).
//   The license and further copyright text can be found in the file LICENSE.TXT
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
// <summary>Exception that occurs during a TestTool run</summary>
//-----------------------------------------------------------------------

namespace WixTest
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
