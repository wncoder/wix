//-------------------------------------------------------------------------------------------------
// <copyright file="ResolveReferencesCommand.cs" company="Outercurve Foundation">
//   Copyright (c) 2004, Outercurve Foundation.
//   This software is released under Microsoft Reciprocal License (MS-RL).
//   The license and further copyright text can be found in the file
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
//-------------------------------------------------------------------------------------------------

namespace WixToolset.Link
{
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Text;
    using WixToolset.Data;
    using WixToolset.Data.Rows;


    internal class ResolveReferencesCommand
    {
        private Section entrySection;
        private IDictionary<string, Symbol> symbols;
        private HashSet<Symbol> referencedSymbols;
        private HashSet<Section> resolvedSections;
        private HashSet<SimpleReferenceSection> unresolvedReferences;


        public ResolveReferencesCommand(Section entrySection, IDictionary<string, Symbol> symbols)
        {
            this.entrySection = entrySection;
            this.symbols = symbols;
        }

        public bool BuildingMergeModule { get; set; }

        public IEnumerable<Symbol> ReferencedSymbols { get { return this.referencedSymbols; } }

        public IEnumerable<Section> ResolvedSections { get { return this.resolvedSections; } }

        public IEnumerable<SimpleReferenceSection> UnresolvedReferences { get { return this.unresolvedReferences; } }

        /// <summary>
        /// Resolves all the simple references in a section.
        /// </summary>
        public void Execute()
        {
            this.resolvedSections = new HashSet<Section>();
            this.referencedSymbols = new HashSet<Symbol>();
            this.unresolvedReferences = new HashSet<SimpleReferenceSection>();

            this.RecursivelyResolveReferences(this.entrySection);
        }

        /// <summary>
        /// Recursive helper function to resolve all references of passed in section.
        /// </summary>
        /// <param name="section">Section with references to resolve.</param>
        /// <remarks>Note: recursive function.</remarks>
        private void RecursivelyResolveReferences(Section section)
        {
            // If we already resolved this section, move on to the next.
            if (!this.resolvedSections.Add(section))
            {
                return;
            }

            // Process all of the references contained in this section using the collection of
            // symbols provided.  Then recursively call this method to process the
            // located symbol's section.  All in all this is a very simple depth-first
            // search of the references per-section.
            Table wixSimpleReferenceTable = section.Tables["WixSimpleReference"];
            if (null != wixSimpleReferenceTable)
            {
                foreach (WixSimpleReferenceRow wixSimpleReferenceRow in wixSimpleReferenceTable.Rows)
                {
                    // If we're building a Merge Module, ignore all references to the Media table
                    // because Merge Modules don't have Media tables.
                    if (this.BuildingMergeModule && "Media" == wixSimpleReferenceRow.TableName)
                    {
                        continue;
                    }

                    Symbol symbol;
                    if (!this.symbols.TryGetValue(wixSimpleReferenceRow.SymbolicName, out symbol))
                    {
                        this.unresolvedReferences.Add(new SimpleReferenceSection() { Section = section, WixSimpleReferenceRow = wixSimpleReferenceRow });
                    }
                    else
                    {
                        this.referencedSymbols.Add(symbol);

                        if (null != symbol.Section)
                        {
                            RecursivelyResolveReferences(symbol.Section);
                        }
                    }
                }
            }
        }

        /// <summary>
        /// Helper class to keep track of simple references in their section.
        /// </summary>
        public class SimpleReferenceSection
        {
            public Section Section { get; set; }

            public WixSimpleReferenceRow WixSimpleReferenceRow { get; set; }
        }
    }
}
