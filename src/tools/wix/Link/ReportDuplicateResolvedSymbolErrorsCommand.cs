//-------------------------------------------------------------------------------------------------
// <copyright file="ReportDuplicateResolvedSymbolErrorsCommand.cs" company="Outercurve Foundation">
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
    using WixToolset.Data;

    public class ReportDuplicateResolvedSymbolErrorsCommand
    {
        private IEnumerable<Symbol> duplicateSymbols;
        private IEnumerable<Section> resolvedSections;

        public ReportDuplicateResolvedSymbolErrorsCommand(IEnumerable<Symbol> duplicateSymbols, IEnumerable<Section> resolvedSections)
        {
            this.duplicateSymbols = duplicateSymbols;
            this.resolvedSections = resolvedSections;
        }

        public void Execute()
        {
            // Do a quick check if there are any duplicate symbols that don't come from tables that allow overriding.
            // Hopefully the symbols with duplicates list is usually a relatively short (although there could be many
            // duplicates that are actually redundant private directories which can be ignored). If we find any matches,
            // we'll do a more costly check to see if the duplicate symbols are in sections we actually referenced.
            // From the resulting set, we'll ignore redundant private directoires and show an error for each remaining
            // duplicate symbol.
            List<Symbol> illegalDuplicates = duplicateSymbols.Where(s => "WixAction" != s.Row.Table.Name && "WixVariable" != s.Row.Table.Name).ToList();
            if (0 < illegalDuplicates.Count)
            {
                HashSet<Section> referencedSections = new HashSet<Section>(resolvedSections);
                foreach (Symbol referencedDuplicateSymbol in illegalDuplicates.Where(s => referencedSections.Contains(s.Section)))
                {
                    List<Symbol> actuallyReferencedDuplicateSymbols = referencedDuplicateSymbol.Duplicates.Where(s => referencedSections.Contains(s.Section)).ToList();

                    if (actuallyReferencedDuplicateSymbols.Any())
                    {
                        // If the referenced symbol with duplicates is a private directory, there is a chance that some or all of
                        // the duplicates are redundant and can be collapsed. If so, replace the collection of referenced duplicate
                        // symbols with only those symbols that are not redundant.
                        if (AccessModifier.Private == referencedDuplicateSymbol.Access && "Directory" == referencedDuplicateSymbol.Row.Table.Name)
                        {
                            List<Symbol> duplicates = new List<Symbol>(actuallyReferencedDuplicateSymbols.Count);
                            foreach (Symbol check in actuallyReferencedDuplicateSymbols)
                            {
                                if (AccessModifier.Private == check.Access && check.Row.IsIdentical(referencedDuplicateSymbol.Row))
                                {
                                    check.Row.Redundant = true; // note that the row is redundant so it does not end up causing problems during .idt generation.
                                }
                                else
                                {
                                    duplicates.Add(check);
                                }
                            }

                            actuallyReferencedDuplicateSymbols = duplicates;
                        }

                        // It is entirely likely that we ended up with no referenced duplicates after the redundant private
                        // directories were removed. Thus, only show errors if there are honest to goodness duplicates.
                        if (actuallyReferencedDuplicateSymbols.Any())
                        {
                            // TODO: Consider using a different error message. This error message doesn't explain that collisions can happen between private symbols
                            //       in different sections when the share the same primary key.
                            Messaging.Instance.OnMessage(WixErrors.DuplicateSymbol(referencedDuplicateSymbol.Row.SourceLineNumbers, referencedDuplicateSymbol.Name));

                            foreach (Symbol duplicate in actuallyReferencedDuplicateSymbols)
                            {
                                Messaging.Instance.OnMessage(WixErrors.DuplicateSymbol2(duplicate.Row.SourceLineNumbers));
                            }
                        }
                    }
                }
            }
        }
    }
}
