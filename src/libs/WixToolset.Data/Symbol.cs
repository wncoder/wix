//-------------------------------------------------------------------------------------------------
// <copyright file="Symbol.cs" company="Outercurve Foundation">
//   Copyright (c) 2004, Outercurve Foundation.
//   This software is released under Microsoft Reciprocal License (MS-RL).
//   The license and further copyright text can be found in the file
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
//-------------------------------------------------------------------------------------------------

namespace WixToolset.Data
{
    using System;
    using System.Collections.Generic;

    /// <summary>
    /// Symbol representing a single row in a database.
    /// </summary>
    public sealed class Symbol
    {
        private HashSet<Symbol> duplicates;

        /// <summary>
        /// Creates a symbol for a row.
        /// </summary>
        /// <param name="row">Row for the symbol</param>
        public Symbol(Row row)
        {
            this.Row = row;
            this.Name = String.Concat(this.Row.TableDefinition.Name, ":", this.Row.GetPrimaryKey());
        }

        /// <summary>
        /// Gets the accessibility of the symbol which is a direct reflection of the accessibility of the row's accessibility.
        /// </summary>
        /// <value>Accessbility of the symbol.</value>
        public AccessModifier Access { get { return this.Row.Access; } }

        /// <summary>
        /// Gets the name of the symbol.
        /// </summary>
        /// <value>Name of the symbol.</value>
        public string Name { get; private set; }

        /// <summary>
        /// Gets the row for this symbol.
        /// </summary>
        /// <value>Row for this symbol.</value>
        public Row Row { get; private set; }

        /// <summary>
        /// Gets the section for the symbol.
        /// </summary>
        /// <value>Section for the symbol.</value>
        public Section Section { get { return (null == this.Row.Table) ? null : this.Row.Table.Section; } }

        /// <summary>
        /// Gets any duplicates of this symbol when loaded or null if there are no duplicates.
        /// </summary>
        public IEnumerable<Symbol> Duplicates { get { return this.duplicates; } }

        /// <summary>
        /// Adds a duplicate symbol.
        /// </summary>
        /// <param name="symbol">Symbol that is duplicative of this symbol.</param>
        public void AddDuplicate(Symbol symbol)
        {
            if (null == this.duplicates)
            {
                this.duplicates = new HashSet<Symbol>();
            }

            this.duplicates.Add(symbol);
        }
    }
}
