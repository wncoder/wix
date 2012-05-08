//-------------------------------------------------------------------------------------------------
// <copyright file="TagDecompiler.cs" company="Microsoft">
//    Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// 
// <summary>
// The decompiler for the Windows Installer XML Toolset Software Id Tag Extension.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Extensions
{
    using System;
    using Microsoft.Tools.WindowsInstallerXml;
    using Tag = Microsoft.Tools.WindowsInstallerXml.Extensions.Serialize.Tag;

    /// <summary>
    /// The Binder for the Windows Installer XML Toolset Software Id Tag Extension.
    /// </summary>
    public sealed class TagDecompiler : DecompilerExtension
    {
        /// <summary>
        /// Decompiles an extension table.
        /// </summary>
        /// <param name="table">The table to decompile.</param>
        public override void DecompileTable(Table table)
        {
            switch (table.Name)
            {
                case "SoftwareIdentificationTag":
                    this.DecompileSoftwareIdentificationTag(table);
                    break;
                default:
                    base.DecompileTable(table);
                    break;
            }
        }

        /// <summary>
        /// Decompile the SoftwareIdentificationTag table.
        /// </summary>
        /// <param name="table">The table to decompile.</param>
        private void DecompileSoftwareIdentificationTag(Table table)
        {
            foreach (Row row in table.Rows)
            {
                Tag.Tag tag= new Tag.Tag();

                tag.Regid = (string)row[1];
                tag.Name = (string)row[2];
                tag.Licensed = null == row[3] ? Tag.YesNoType.NotSet : 1 == (int)row[3] ? Tag.YesNoType.yes : Tag.YesNoType.no;

                this.Core.RootElement.AddChild(tag);
            }
        }
    }
}
