//-------------------------------------------------------------------------------------------------
// <copyright file="DecompilerExtension.cs" company="Outercurve Foundation">
//   Copyright (c) 2004, Outercurve Foundation.
//   This software is released under Microsoft Reciprocal License (MS-RL).
//   The license and further copyright text can be found in the file
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
//-------------------------------------------------------------------------------------------------

namespace WixToolset.Extensibility
{
    /// <summary>
    /// Base class for creating a decompiler extension.
    /// </summary>
    public abstract class DecompilerExtension : IDecompilerExtension
    {
        /// <summary>
        /// Gets or sets the decompiler core for the extension.
        /// </summary>
        /// <value>The decompiler core for the extension.</value>
        public IDecompilerCore Core { get; set; }

        /// <summary>
        /// Gets the option to remove the rows from this extension's library.
        /// </summary>
        /// <value>The option to remove the rows from this extension's library.</value>
        public virtual bool RemoveLibraryRows
        {
            get { return true; }
        }

        /// <summary>
        /// Called at the beginning of the decompilation of a database.
        /// </summary>
        /// <param name="tables">The collection of all tables.</param>
        public virtual void Initialize(TableCollection tables)
        {
        }

        /// <summary>
        /// Decompiles an extension table.
        /// </summary>
        /// <param name="table">The table to decompile.</param>
        public virtual void DecompileTable(Table table)
        {
            this.Core.OnMessage(WixErrors.TableDecompilationUnimplemented(table.Name));
        }

        /// <summary>
        /// Finalize decompilation.
        /// </summary>
        /// <param name="tables">The collection of all tables.</param>
        public virtual void Finish(TableCollection tables)
        {
        }
    }
}
