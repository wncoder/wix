//-------------------------------------------------------------------------------------------------
// <copyright file="ScannedSourceFile.cs" company="Microsoft">
//    Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// 
// <summary>
// Source file found by the Windows Installer Xml toolset scanner.
// </summary>
//-------------------------------------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Text;

namespace Microsoft.Tools.WindowsInstallerXml
{
    public class ScannedSourceFile
    {
        public ScannedSourceFile(string path)
            : this(path, null)
        {
        }

        public ScannedSourceFile(string path, IDictionary<string, string> preprocessorDefines)
        {
            StringBuilder keyBuilder = new StringBuilder();
            keyBuilder.Append(path.ToLowerInvariant());

            this.Path = path;

            this.PreprocessorDefines = new Dictionary<string, string>(StringComparer.OrdinalIgnoreCase);
            if (null != preprocessorDefines)
            {
                foreach (KeyValuePair<string, string> kvp in preprocessorDefines)
                {
                    this.PreprocessorDefines.Add(kvp.Key, kvp.Value);
                }
            }

            this.Key = ScannedSourceFile.CalculateKey(this.Path, this.PreprocessorDefines);
            this.SourceProjects = new List<ScannedProject>();
            this.TargetSymbols = new List<ScannedSymbol>();
        }

        public string Key { get; private set; }

        public string Path { get; private set; }

        public IDictionary<string, string> PreprocessorDefines { get; private set; }

        public IList<ScannedProject> SourceProjects { get; private set; }

        public IList<ScannedSymbol> TargetSymbols { get; private set; }

        public static string CalculateKey(string path, IDictionary<string, string> preprocessorDefines)
        {
            StringBuilder keyBuilder = new StringBuilder();
            keyBuilder.Append(path.ToLowerInvariant());

            if (null != preprocessorDefines)
            {
                foreach (KeyValuePair<string, string> kvp in preprocessorDefines)
                {
                    keyBuilder.AppendFormat(";{0}={1}", kvp.Key, kvp.Value);
                }
            }

            return keyBuilder.ToString();
        }
    }
}
