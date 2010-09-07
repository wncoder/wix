//-----------------------------------------------------------------------
// <copyright file="FrostException.cs" company="Microsoft">
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
// <summary>Defines a Frost exception</summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Frost.Core
{
    using System;

    public class FrostException : Exception
    {
        public FrostException() : base(){ }
        public FrostException(string Reason) : base(Reason) { }
        public FrostException(params object[] ReasonStrings) : base(String.Concat(ReasonStrings)) { }
    }

    public class FrostNonExistentVariableException : FrostException
    {
        public FrostNonExistentVariableException(string VariableName) : base("Variable ", VariableName, " was not defined in the environment") { }
    }

    public class FrostConfigException : FrostException
    {
        public FrostConfigException(string Reason) : base(Reason) { }
        public FrostConfigException(params object[] ReasonStrings) : base(ReasonStrings) { }
    }
}
