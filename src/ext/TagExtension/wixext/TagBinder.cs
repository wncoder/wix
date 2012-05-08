//-------------------------------------------------------------------------------------------------
// <copyright file="TagBinder.cs" company="Microsoft">
//    Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// 
// <summary>
// The Binder for the Windows Installer XML Toolset Software Id Tag Extension.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Extensions
{
    using System;
    using System.Collections.Generic;
    using System.IO;
    using System.Text;
    using System.Xml;
    using Microsoft.Tools.WindowsInstallerXml;

    /// <summary>
    /// The Binder for the Windows Installer XML Toolset Software Id Tag Extension.
    /// </summary>
    public sealed class TagBinder : BinderExtensionEx
    {
        private string overallRegid;
        private RowDictionary<Row> swidRows = new RowDictionary<Row>();

        /// <summary>
        /// Called before database binding occurs.
        /// </summary>
        public override void DatabaseInitialize(Output output)
        {
            this.overallRegid = null; // always reset overall regid on initialize.

            // Ensure the tag files are generated to be imported by the MSI.
            this.CreateTagFiles(output);
        }

        /// <summary>
        /// Called after database variable resolution occurs.
        /// </summary>
        public override void DatabaseAfterResolvedFields(Output output)
        {
            Table wixBindUpdateFilesTable = output.Tables["WixBindUpdatedFiles"];

            // We'll end up re-writing the tag files but this time we may have the ProductCode
            // now to use as the unique id.
            List<WixFileRow> updatedFileRows = this.CreateTagFiles(output);
            foreach (WixFileRow updateFileRow in updatedFileRows)
            {
                Row row = wixBindUpdateFilesTable.CreateRow(updateFileRow.SourceLineNumbers);
                row[0] = updateFileRow.File;
            }
        }

        private List<WixFileRow> CreateTagFiles(Output output)
        {
            List<WixFileRow> updatedFileRows = new List<WixFileRow>();
            SourceLineNumberCollection sourceLineNumbers = null;

            Table tagTable = output.Tables["WixTag"];
            if (null != tagTable)
            {
                string productCode = null;
                string productName = null;
                Version productVersion = new Version("0.0.0.0");
                string manufacturer = null;

                Table properties = output.Tables["Property"];
                foreach (Row property in properties.Rows)
                {
                    switch ((string)property[0])
                    {
                        case "ProductCode":
                            productCode = (string)property[1];
                            break;
                        case "ProductName":
                            productName = (string)property[1];
                            break;
                        case "ProductVersion":
                            productVersion = new Version((string)property[1]);
                            break;
                        case "Manufacturer":
                            manufacturer = (string)property[1];
                            break;
                    }
                }

                // If the ProductCode is available, only keep it if it is a GUID.
                if (!String.IsNullOrEmpty(productCode))
                {
                    try
                    {
                        Guid guid = new Guid(productCode);
                        productCode = guid.ToString("D").ToUpperInvariant();
                    }
                    catch // not a GUID, erase it.
                    {
                        productCode = null;
                    }
                }

                // Ensure version has 4 parts.
                productVersion = new Version(productVersion.Major,
                                             -1 < productVersion.Minor ? productVersion.Minor : 0,
                                             -1 < productVersion.Build ? productVersion.Build : 0,
                                             -1 < productVersion.Revision ? productVersion.Revision : 0);

                Table wixFileTable = output.Tables["WixFile"];
                foreach (Row tagRow in tagTable.Rows)
                {
                    string fileId = (string)tagRow[0];
                    string regid = (string)tagRow[1];
                    string name = (string)tagRow[2];
                    bool licensed = (null != tagRow[3] && 1 == (int)tagRow[3]);

                    if (String.IsNullOrEmpty(name))
                    {
                        name = productName;
                    }

                    string uniqueId = String.IsNullOrEmpty(productCode) ? name.Replace(" ", "-") : productCode;

                    if (String.IsNullOrEmpty(this.overallRegid))
                    {
                        this.overallRegid = regid;
                        sourceLineNumbers = tagRow.SourceLineNumbers;
                    }
                    else if (!this.overallRegid.Equals(regid, StringComparison.Ordinal))
                    {
                        // TODO: display error that only one regid supported.
                    }

                    // Find the WixFileRow that matches for this WixTag.
                    foreach (WixFileRow wixFileRow in wixFileTable.Rows)
                    {
                        if (fileId == wixFileRow.File)
                        {
                            updatedFileRows.Add(wixFileRow); // remember that we modified this file.

                            // Write the tag file.
                            wixFileRow.Source = Path.GetTempFileName();
                            using (XmlTextWriter writer = new XmlTextWriter(wixFileRow.Source, Encoding.UTF8))
                            {
                                writer.Formatting = Formatting.Indented;
                                writer.WriteStartDocument();
                                writer.WriteStartElement("software_identification_tag", "http://standards.iso.org/iso/19770/-2/2009/schema.xsd");
                                writer.WriteElementString("entitlement_required_indicator", licensed ? "true" : "false");

                                writer.WriteElementString("product_title", productName);

                                writer.WriteStartElement("product_version");
                                writer.WriteElementString("name", productVersion.ToString());
                                writer.WriteStartElement("numeric");
                                writer.WriteElementString("major", productVersion.Major.ToString());
                                writer.WriteElementString("minor", productVersion.Minor.ToString());
                                writer.WriteElementString("build", productVersion.Build.ToString());
                                writer.WriteElementString("review", productVersion.Revision.ToString());
                                writer.WriteEndElement();
                                writer.WriteEndElement();

                                writer.WriteStartElement("software_creator");
                                writer.WriteElementString("name", manufacturer);
                                writer.WriteElementString("regid", regid);
                                writer.WriteEndElement();

                                if (licensed)
                                {
                                    writer.WriteStartElement("software_licensor");
                                    writer.WriteElementString("name", manufacturer);
                                    writer.WriteElementString("regid", regid);
                                    writer.WriteEndElement();
                                }

                                writer.WriteStartElement("software_id");
                                writer.WriteElementString("unique_id", uniqueId);
                                writer.WriteElementString("tag_creator_regid", regid);
                                writer.WriteEndElement();

                                writer.WriteStartElement("tag_creator");
                                writer.WriteElementString("name", manufacturer);
                                writer.WriteElementString("regid", regid);
                                writer.WriteEndElement();

                                writer.WriteEndElement();
                            }

                            // Ensure the matching "SoftwareIdentificationTag" row exists and
                            // is populated correctly.
                            Row swidRow;
                            if (!this.swidRows.TryGet(fileId, out swidRow))
                            {
                                Table swid = output.Tables["SoftwareIdentificationTag"];
                                swidRow = swid.CreateRow(wixFileRow.SourceLineNumbers);
                                swidRow[0] = fileId;
                                swidRow[1] = this.overallRegid;

                                this.swidRows.Add(swidRow);
                            }

                            // Always rewrite.
                            swidRow[2] = uniqueId;
                            swidRow[3] = "Application"; // TODO: set this to the correct type when type's are supported.
                        }
                    }
                }
            }

            // If we remembered the source line number for the regid, then add
            // a WixVariable to map to the regid.
            if (null != sourceLineNumbers)
            {
                Table wixVariableTable = output.Tables.EnsureTable(output.EntrySection, this.Core.TableDefinitions["WixVariable"]);
                WixVariableRow wixVariableRow = (WixVariableRow)wixVariableTable.CreateRow(sourceLineNumbers);
                wixVariableRow.Id = "WixTagRegid";
                wixVariableRow.Value = this.overallRegid;
                wixVariableRow.Overridable = false;
            }

            return updatedFileRows;
        }
    }
}
