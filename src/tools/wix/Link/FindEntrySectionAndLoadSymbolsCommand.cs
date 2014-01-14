//-------------------------------------------------------------------------------------------------
// <copyright file="FindEntrySectionAndLoadSymbolsCommand.cs" company="Outercurve Foundation">
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

    internal class FindEntrySectionAndLoadSymbolsCommand
    {
        private IEnumerable<Section> sections;

        public FindEntrySectionAndLoadSymbolsCommand(IEnumerable<Section> sections)
        {
            this.sections = sections;
        }

        public bool AllowIdenticalRows { get; set; }

        public OutputType ExpectedOutputType { get; set;}

        public Section EntrySection { get; private set; }

        public IDictionary<string, Symbol> Symbols { get; private set; }

        public IEnumerable<Symbol> SymbolsWithDuplicates { get; private set; }

        /// <summary>
        /// Finds the entry section and loads the symbols from an array of intermediates.
        /// </summary>
        /// <param name="allowIdenticalRows">Flag specifying whether identical rows are allowed or not.</param>
        /// <param name="messageHandler">Message handler object to route all errors through.</param>
        /// <param name="expectedOutputType">Expected entry output type, based on output file extension provided to the linker.</param>
        /// <param name="entrySection">Located entry section.</param>
        /// <param name="allSymbols">Collection of symbols loaded.</param>
        public void Execute()
        {
            Dictionary<string, Symbol> symbols = new Dictionary<string, Symbol>();
            HashSet<Symbol> withDuplicates = new HashSet<Symbol>();

            SectionType expectedEntrySectionType;
            if (!Enum.TryParse<SectionType>(this.ExpectedOutputType.ToString(), out expectedEntrySectionType))
            {
                expectedEntrySectionType = SectionType.Unknown;
            }

            foreach (Section section in this.sections)
            {
                // Try to find the one and only entry section.
                if (SectionType.Product == section.Type || SectionType.Module == section.Type || SectionType.PatchCreation == section.Type || SectionType.Patch == section.Type || SectionType.Bundle == section.Type)
                {
                    if (SectionType.Unknown != expectedEntrySectionType && section.Type != expectedEntrySectionType)
                    {
                        string outputExtension = Output.GetExtension(this.ExpectedOutputType);
                        Messaging.Instance.OnMessage(WixDataWarnings.UnexpectedEntrySection(section.SourceLineNumbers, section.Type.ToString(), expectedEntrySectionType.ToString(), outputExtension));
                    }

                    if (null == this.EntrySection)
                    {
                        this.EntrySection = section;
                    }
                    else
                    {
                        Messaging.Instance.OnMessage(WixDataErrors.MultipleEntrySections(this.EntrySection.SourceLineNumbers, this.EntrySection.Id, section.Id));
                        Messaging.Instance.OnMessage(WixDataErrors.MultipleEntrySections2(section.SourceLineNumbers));
                    }
                }

                // Load all the symbols from the section's tables.
                foreach (Table table in section.Tables)
                {
                    // TODO: turn this into a LINQ query when TableCollection is gone/modernized.
                    if (!table.Definition.CreateSymbols)
                    {
                        continue;
                    }

                    foreach (Row row in table.Rows)
                    {
                        Symbol symbol = new Symbol(row);

                        Symbol existingSymbol;
                        if (!symbols.TryGetValue(symbol.Name, out existingSymbol))
                        {
                            symbols.Add(symbol.Name, symbol);
                        }
                        else if (this.AllowIdenticalRows && existingSymbol.Row.IsIdentical(symbol.Row))
                        {
                            Messaging.Instance.OnMessage(WixDataWarnings.IdenticalRowWarning(symbol.Row.SourceLineNumbers, existingSymbol.Name));
                            Messaging.Instance.OnMessage(WixDataWarnings.IdenticalRowWarning2(existingSymbol.Row.SourceLineNumbers));
                        }
                        else // uh-oh, duplicate symbols.
                        {
                            // TODO: can this be here? Supposedly tables like WixAction and WixVariable allow duplicate symbols so this would fail
                            //       them. Seems like this check must be later in the linker/librarian.
                            //Messaging.Instance.OnMessage(WixDataErrors.DuplicateSymbol(existingSymbol.Row.SourceLineNumbers, existingSymbol.Name));
                            //if (null != symbol.Row.SourceLineNumbers)
                            //{
                            //    Messaging.Instance.OnMessage(WixDataErrors.DuplicateSymbol2(symbol.Row.SourceLineNumbers));
                            //}

                            existingSymbol.AddDuplicate(symbol);
                            withDuplicates.Add(existingSymbol);
                        }
                    }
                }
            }

            this.Symbols = symbols;
            this.SymbolsWithDuplicates = withDuplicates;
        }
    }
}
