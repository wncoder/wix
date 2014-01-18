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
            // Do a quick check if there are any illegal duplicate symbols. Hopefully the symbols with duplicates
            // is usually a very short (almost always empty) list so that goes fast. If we find any matches, we'll
            // do a more costly check to see if the duplicate symbols are in sections we actually referenced. From
            // the resulting set, we'll show an error for each duplicate symbol.
            List<Symbol> illegalDuplicates = duplicateSymbols.Where(s => "WixAction" != s.Row.Table.Name && "WixVariable" != s.Row.Table.Name).ToList();
            if (0 < illegalDuplicates.Count)
            {
                HashSet<Section> referencedSections = new HashSet<Section>(resolvedSections);
                foreach (Symbol referencedDuplicateSymbol in illegalDuplicates.Where(s => referencedSections.Contains(s.Section)))
                {
                    List<Symbol> actuallyReferencedDuplicateSymbols = referencedDuplicateSymbol.Duplicates.Where(s => referencedSections.Contains(s.Section)).ToList();

                    if (actuallyReferencedDuplicateSymbols.Any())
                    {
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
