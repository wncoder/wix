//-------------------------------------------------------------------------------------------------
// <copyright file="ScanResult.cs" company="Microsoft">
//    Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// 
// <summary>
// Windows Installer Xml toolset scanner result.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml
{
    using System;
    using System.Collections.Generic;

    public class ScanResult
    {
        public ScanResult()
        {
            this.Unresolved = new List<ScannedUnresolvedReference>();

            this.ProjectFiles = new Dictionary<string, ScannedProject>();
            this.SourceFiles = new Dictionary<string, ScannedSourceFile>();
            this.Symbols = new Dictionary<string, ScannedSymbol>();
            this.UnknownFiles = new SortedSet<string>(StringComparer.OrdinalIgnoreCase);

            this.ProjectToProjectReferences = new SortedSet<ScannedProjectProjectReference>();
            this.ProjectToSourceFileReferences = new SortedSet<ScannedProjectSourceFileReference>();
            this.SourceFileToSymbolReference = new SortedSet<ScannedSourceFileSymbolReference>();
            this.SymbolToSymbolReference = new SortedSet<ScannedSymbolSymbolReference>();
        }

        public IDictionary<string, ScannedProject> ProjectFiles { get; private set; }

        public IDictionary<string, ScannedSourceFile> SourceFiles { get; private set; }

        public IDictionary<string, ScannedSymbol> Symbols { get; private set; }

        public ISet<string> UnknownFiles { get; private set; }

        public List<ScannedUnresolvedReference> Unresolved { get; private set; }

        public SortedSet<ScannedProjectProjectReference> ProjectToProjectReferences { get; private set; }

        public SortedSet<ScannedProjectSourceFileReference> ProjectToSourceFileReferences { get; private set; }

        public SortedSet<ScannedSourceFileSymbolReference> SourceFileToSymbolReference { get; private set; }

        public SortedSet<ScannedSymbolSymbolReference> SymbolToSymbolReference { get; private set; }
    }
}
