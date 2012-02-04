//-----------------------------------------------------------------------
// <copyright file="FrostException.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
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
