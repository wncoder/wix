//-------------------------------------------------------------------------------------------------
// <copyright file="Section.cs" company="Outercurve Foundation">
//   Copyright (c) 2004, Outercurve Foundation.
//   This software is released under Microsoft Reciprocal License (MS-RL).
//   The license and further copyright text can be found in the file
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
//-------------------------------------------------------------------------------------------------

namespace WixToolset.Data
{
    using System;
    using System.Collections;
    using System.Collections.Specialized;
    using System.Diagnostics;
    using System.Globalization;
    using System.Linq;
    using System.Text;
    using System.Xml;
    using WixToolset.Data.Rows;

    /// <summary>
    /// Section in an object file.
    /// </summary>
    public sealed class Section
    {
        private string id;
        private SectionType type;
        private int codepage;

        private TableCollection tables;

        private SourceLineNumber sourceLineNumbers;

        /// <summary>
        /// Creates a new section as part of an intermediate.
        /// </summary>
        /// <param name="id">Identifier for section.</param>
        /// <param name="type">Type of section.</param>
        /// <param name="codepage">Codepage for resulting database.</param>
        public Section(string id, SectionType type, int codepage)
        {
            this.id = id;
            this.type = type;
            this.codepage = codepage;

            this.tables = new TableCollection();
        }

        /// <summary>
        /// Gets the identifier for the section.
        /// </summary>
        /// <value>Section identifier.</value>
        public string Id
        {
            get { return this.id; }
        }

        /// <summary>
        /// Gets the type of the section.
        /// </summary>
        /// <value>Type of section.</value>
        public SectionType Type
        {
            get { return this.type; }
        }

        /// <summary>
        /// Gets the codepage for the section.
        /// </summary>
        /// <value>Codepage for the section.</value>
        public int Codepage
        {
            get { return this.codepage; }
        }

        /// <summary>
        /// Gets the tables in the section.
        /// </summary>
        /// <value>Tables in section.</value>
        public TableCollection Tables
        {
            get { return this.tables; }
        }

        /// <summary>
        /// Gets the source line information of the file containing this section.
        /// </summary>
        /// <value>The source line information of the file containing this section.</value>
        public SourceLineNumber SourceLineNumbers
        {
            get { return this.sourceLineNumbers; }
        }

        /// <summary>
        /// Parse a section from the xml.
        /// </summary>
        /// <param name="reader">XmlReader where the intermediate is persisted.</param>
        /// <param name="tableDefinitions">TableDefinitions to use in the intermediate.</param>
        /// <returns>The parsed Section.</returns>
        internal static Section Read(XmlReader reader, TableDefinitionCollection tableDefinitions)
        {
            Debug.Assert("section" == reader.LocalName);

            int codepage = 0;
            bool empty = reader.IsEmptyElement;
            string id = null;
            Section section = null;
            SectionType type = SectionType.Unknown;

            while (reader.MoveToNextAttribute())
            {
                switch (reader.Name)
                {
                    case "codepage":
                        codepage = Convert.ToInt32(reader.Value, CultureInfo.InvariantCulture);
                        break;
                    case "id":
                        id = reader.Value;
                        break;
                    case "type":
                        switch (reader.Value)
                        {
                            case "bundle":
                                type = SectionType.Bundle;
                                break;
                            case "fragment":
                                type = SectionType.Fragment;
                                break;
                            case "module":
                                type = SectionType.Module;
                                break;
                            case "patchCreation":
                                type = SectionType.PatchCreation;
                                break;
                            case "product":
                                type = SectionType.Product;
                                break;
                            case "patch":
                                type = SectionType.Patch;
                                break;
                            default:
                                throw new WixException(WixDataErrors.IllegalAttributeValue(SourceLineNumber.CreateFromUri(reader.BaseURI), "section", reader.Name, reader.Value, "fragment", "module", "patchCreation", "product", "patch"));
                        }
                        break;
                    default:
                        if (!reader.NamespaceURI.StartsWith("http://www.w3.org/", StringComparison.Ordinal))
                        {
                            throw new WixException(WixDataErrors.UnexpectedAttribute(SourceLineNumber.CreateFromUri(reader.BaseURI), "section", reader.Name));
                        }
                        break;
                }
            }

            if (null == id && (SectionType.Unknown != type && SectionType.Fragment != type))
            {
                throw new WixException(WixDataErrors.ExpectedAttribute(SourceLineNumber.CreateFromUri(reader.BaseURI), "section", "id", "type", type.ToString()));
            }

            if (SectionType.Unknown == type)
            {
                throw new WixException(WixDataErrors.ExpectedAttribute(SourceLineNumber.CreateFromUri(reader.BaseURI), "section", "type"));
            }

            section = new Section(id, type, codepage);
            section.sourceLineNumbers = SourceLineNumber.CreateFromUri(reader.BaseURI);

            if (!empty)
            {
                bool done = false;

                while (!done && reader.Read())
                {
                    switch (reader.NodeType)
                    {
                        case XmlNodeType.Element:
                            switch (reader.LocalName)
                            {
                                case "table":
                                    section.Tables.Add(Table.Read(reader, section, tableDefinitions));
                                    break;
                                default:
                                    throw new WixException(WixDataErrors.UnexpectedElement(SourceLineNumber.CreateFromUri(reader.BaseURI), "section", reader.Name));
                            }
                            break;
                        case XmlNodeType.EndElement:
                            done = true;
                            break;
                    }
                }

                if (!done)
                {
                    throw new WixException(WixDataErrors.ExpectedEndElement(SourceLineNumber.CreateFromUri(reader.BaseURI), "section"));
                }
            }

            return section;
        }

        /// <summary>
        /// Persist the Section to an XmlWriter.
        /// </summary>
        /// <param name="writer">XmlWriter which reference will be persisted to.</param>
        internal void Write(XmlWriter writer)
        {
            writer.WriteStartElement("section", Intermediate.XmlNamespaceUri);

            if (null != this.id)
            {
                writer.WriteAttributeString("id", this.id);
            }

            switch (this.type)
            {
                case SectionType.Bundle:
                    writer.WriteAttributeString("type", "bundle");
                    break;
                case SectionType.Fragment:
                    writer.WriteAttributeString("type", "fragment");
                    break;
                case SectionType.Module:
                    writer.WriteAttributeString("type", "module");
                    break;
                case SectionType.Product:
                    writer.WriteAttributeString("type", "product");
                    break;
                case SectionType.PatchCreation:
                    writer.WriteAttributeString("type", "patchCreation");
                    break;
                case SectionType.Patch:
                    writer.WriteAttributeString("type", "patch");
                    break;
            }

            if (0 != this.codepage)
            {
                writer.WriteAttributeString("codepage", this.codepage.ToString(CultureInfo.InvariantCulture));
            }

            // save the rows in table order
            foreach (Table table in this.tables)
            {
                table.Write(writer);
            }

            writer.WriteEndElement();
        }
    }
}
