//-------------------------------------------------------------------------------------------------
// <copyright file="Intermediate.cs" company="Outercurve Foundation">
//   Copyright (c) 2004, Outercurve Foundation.
//   This software is released under Microsoft Reciprocal License (MS-RL).
//   The license and further copyright text can be found in the file
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
// 
// <summary>
// Container class for an intermediate object.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace WixToolset.Data
{
    using System;
    using System.IO;
    using System.Xml;
    using System.Xml.Schema;

    /// <summary>
    /// Container class for an intermediate object.
    /// </summary>
    public sealed class Intermediate
    {
        public const string XmlNamespaceUri = "http://wixtoolset.org/schemas/v4/wixobj";
        private static readonly Version currentVersion = new Version("4.0.0.0");

        private SectionCollection sections;

        /// <summary>
        /// Instantiate a new Intermediate.
        /// </summary>
        public Intermediate()
        {
            this.sections = new SectionCollection();
        }

        /// <summary>
        /// Get the sections contained in this intermediate.
        /// </summary>
        /// <value>Sections contained in this intermediate.</value>
        public SectionCollection Sections
        {
            get { return this.sections; }
        }

        /// <summary>
        /// Loads an intermediate from a path on disk.
        /// </summary>
        /// <param name="path">Path to intermediate file saved on disk.</param>
        /// <param name="tableDefinitions">Collection containing TableDefinitions to use when reconstituting the intermediate.</param>
        /// <param name="suppressVersionCheck">Suppress checking for wix.dll version mismatches.</param>
        /// <returns>Returns the loaded intermediate.</returns>
        public static Intermediate Load(string path, TableDefinitionCollection tableDefinitions, bool suppressVersionCheck)
        {
            try
            {
                using (XmlReader reader = XmlReader.Create(path))
                {
                    reader.MoveToContent();
                    return Intermediate.Read(reader, tableDefinitions, suppressVersionCheck);
                }
            }
            catch (XmlException xe)
            {
                throw new WixNotIntermediateException(WixDataErrors.InvalidXml(new SourceLineNumber(path), "object", xe.Message));
            }
            catch (XmlSchemaException xse)
            {
                throw new WixNotIntermediateException(WixDataErrors.SchemaValidationFailed(new SourceLineNumber(path), xse.Message, xse.LineNumber, xse.LinePosition));
            }
        }

        /// <summary>
        /// Saves an intermediate to a path on disk.
        /// </summary>
        /// <param name="path">Path to save intermediate file to disk.</param>
        public void Save(string path)
        {
            try
            {
                // Ensure the location to output the xml exists.
                Directory.CreateDirectory(Path.GetDirectoryName(Path.GetFullPath(path)));

                using (XmlWriter writer = XmlWriter.Create(path))
                {
                    writer.WriteStartDocument();
                    this.Write(writer);
                    writer.WriteEndDocument();
                }
            }
            catch (UnauthorizedAccessException)
            {
                throw new WixException(WixDataErrors.UnauthorizedAccess(path));
            }
        }

        /// <summary>
        /// Parse an intermediate from an XML format.
        /// </summary>
        /// <param name="reader">XmlReader where the intermediate is persisted.</param>
        /// <param name="tableDefinitions">TableDefinitions to use in the intermediate.</param>
        /// <param name="suppressVersionCheck">Suppress checking for wix.dll version mismatch.</param>
        /// <returns>The parsed Intermediate.</returns>
        private static Intermediate Read(XmlReader reader, TableDefinitionCollection tableDefinitions, bool suppressVersionCheck)
        {
            if ("wixObject" != reader.LocalName)
            {
                throw new WixNotIntermediateException(WixDataErrors.InvalidDocumentElement(SourceLineNumber.CreateFromUri(reader.BaseURI), reader.Name, "object", "wixObject"));
            }

            bool empty = reader.IsEmptyElement;
            Version objVersion = null;

            while (reader.MoveToNextAttribute())
            {
                switch (reader.LocalName)
                {
                    case "version":
                        objVersion = new Version(reader.Value);
                        break;
                    default:
                        if (!reader.NamespaceURI.StartsWith("http://www.w3.org/", StringComparison.Ordinal))
                        {
                            throw new WixException(WixDataErrors.UnexpectedAttribute(SourceLineNumber.CreateFromUri(reader.BaseURI), "wixObject", reader.Name));
                        }
                        break;
                }
            }

            if (null != objVersion && !suppressVersionCheck)
            {
                if (0 != currentVersion.CompareTo(objVersion))
                {
                    throw new WixException(WixDataErrors.VersionMismatch(SourceLineNumber.CreateFromUri(reader.BaseURI), "object", objVersion.ToString(), currentVersion.ToString()));
                }
            }

            Intermediate intermediate = new Intermediate();

            // loop through the rest of the xml building up the SectionCollection
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
                                case "section":
                                    intermediate.sections.Add(Section.Read(reader, tableDefinitions));
                                    break;
                                default:
                                    throw new WixException(WixDataErrors.UnexpectedElement(SourceLineNumber.CreateFromUri(reader.BaseURI), "wixObject", reader.Name));
                            }
                            break;
                        case XmlNodeType.EndElement:
                            done = true;
                            break;
                    }
                }

                if (!done)
                {
                    throw new WixException(WixDataErrors.ExpectedEndElement(SourceLineNumber.CreateFromUri(reader.BaseURI), "wixObject"));
                }
            }

            return intermediate;
        }

        /// <summary>
        /// Persists an intermediate in an XML format.
        /// </summary>
        /// <param name="writer">XmlWriter where the Intermediate should persist itself as XML.</param>
        private void Write(XmlWriter writer)
        {
            writer.WriteStartElement("wixObject", XmlNamespaceUri);

            writer.WriteAttributeString("version", currentVersion.ToString());

            foreach (Section section in this.sections)
            {
                section.Write(writer);
            }

            writer.WriteEndElement();
        }
    }
}
