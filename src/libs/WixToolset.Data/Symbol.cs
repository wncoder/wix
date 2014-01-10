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

    /// <summary>
    /// Symbol representing a single row in a database.
    /// </summary>
    public sealed class Symbol
    {
        /// <summary>
        /// Creates a symbol for a row.
        /// </summary>
        /// <param name="row">Row for the symbol</param>
        public Symbol(Row row)
        {
            this.Row = row;
            this.Name = String.Concat(this.Row.TableDefinition.Name, ":", this.Row.GetPrimaryKey('/'));
            this.Section = (null == this.Row.Table) ? null : this.Row.Table.Section;
        }

        /// <summary>
        /// Gets the name of the symbol.
        /// </summary>
        /// <value>Name of the symbol.</value>
        public string Name { get; private set; }

        /// <summary>
        /// Gets the section for the symbol.
        /// </summary>
        /// <value>Section for the symbol.</value>
        public Section Section { get; private set; }

        /// <summary>
        /// Gets the row for this symbol.
        /// </summary>
        /// <value>Row for this symbol.</value>
        public Row Row { get; private set; }
    }
}
