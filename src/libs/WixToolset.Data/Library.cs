//-------------------------------------------------------------------------------------------------
// <copyright file="Library.cs" company="Outercurve Foundation">
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
    using System.Diagnostics;
    using System.IO;
    using System.Xml;

    /// <summary>
    /// Object that represents a library file.
    /// </summary>
    public sealed class Library
    {
        public const string XmlNamespaceUri = "http://wixtoolset.org/schemas/v4/wixlib";
        private static readonly Version currentVersion = new Version("4.0.0.0");

        private Dictionary<string, Localization> localizations;
        private List<Section> sections;

        /// <summary>
        /// Instantiates a new empty library which is only useful from static creating methods.
        /// </summary>
        private Library()
        {
            this.localizations = new Dictionary<string, Localization>();
            this.sections = new List<Section>();
        }

        /// <summary>
        /// Instantiate a new library populated with sections.
        /// </summary>
        /// <param name="sections">Sections to add to the library.</param>
        public Library(IEnumerable<Section> sections)
        {
            this.localizations = new Dictionary<string, Localization>();
            this.sections = new List<Section>(sections);
        }

        /// <summary>
        /// Get the sections contained in this library.
        /// </summary>
        /// <value>Sections contained in this library.</value>
        public IEnumerable<Section> Sections { get { return this.sections; } }

        /// <summary>
        /// Add a localization file to this library.
        /// </summary>
        /// <param name="localization">The localization file to add.</param>
        public void AddLocalization(Localization localization)
        {
            Localization existingCulture;
            if (this.localizations.TryGetValue(localization.Culture, out existingCulture))
            {
                existingCulture.Merge(localization);
            }
            else
            {
                this.localizations.Add(localization.Culture, localization);
            }
        }

        /// <summary>
        /// Gets localization files from this library that match the cultures passed in, in the order of the array of cultures.
        /// </summary>
        /// <param name="cultures">The list of cultures to get localizations for.</param>
        /// <returns>All localizations contained in this library that match the set of cultures provided, in the same order.</returns>
        public IEnumerable<Localization> GetLocalizations(string[] cultures)
        {
            foreach (string culture in cultures ?? new string[0])
            {
                Localization localization;
                if (this.localizations.TryGetValue(culture, out localization))
                {
                    yield return localization;
                }
            }
        }

        /// <summary>
        /// Loads a library from a path on disk.
        /// </summary>
        /// <param name="path">Path to library file saved on disk.</param>
        /// <param name="tableDefinitions">Collection containing TableDefinitions to use when reconstituting the intermediates.</param>
        /// <param name="suppressVersionCheck">Suppresses wix.dll version mismatch check.</param>
        /// <returns>Returns the loaded library.</returns>
        public static Library Load(string path, TableDefinitionCollection tableDefinitions, bool suppressVersionCheck)
        {
            using (FileStream stream = File.OpenRead(path))
            {
                return Load(stream, new Uri(Path.GetFullPath(path)), tableDefinitions, suppressVersionCheck);
            }
        }

        /// <summary>
        /// Loads a library from a stream.
        /// </summary>
        /// <param name="stream">Stream containing the library file.</param>
        /// <param name="uri">Uri for finding this stream.</param>
        /// <param name="tableDefinitions">Collection containing TableDefinitions to use when reconstituting the intermediates.</param>
        /// <param name="suppressVersionCheck">Suppresses wix.dll version mismatch check.</param>
        /// <returns>Returns the loaded library.</returns>
        public static Library Load(Stream stream, Uri uri, TableDefinitionCollection tableDefinitions, bool suppressVersionCheck)
        {
            using (FileStructure fs = FileStructure.Read(stream))
            using (XmlReader reader = XmlReader.Create(fs.GetDataStream(), null, uri.AbsoluteUri))
            {
                try
                {
                    reader.MoveToContent();
                    return Library.Read(reader, tableDefinitions, suppressVersionCheck);
                }
                catch (XmlException xe)
                {
                    throw new WixException(WixDataErrors.InvalidXml(SourceLineNumber.CreateFromUri(reader.BaseURI), "object", xe.Message));
                }
            }
        }

        /// <summary>
        /// Saves a library to a path on disk.
        /// </summary>
        /// <param name="path">Path to save library file to on disk.</param>
        /// <param name="resolver">The WiX path resolver.</param>
        public void Save(string path, ILibraryBinaryFileResolver resolver)
        {
            List<string> embedFilePaths = new List<string>();

            // resolve paths to files and create the library cabinet file
            foreach (Section section in this.sections)
            {
                foreach (Table table in section.Tables)
                {
                    foreach (Row row in table.Rows)
                    {
                        foreach (Field field in row.Fields)
                        {
                            ObjectField objectField = field as ObjectField;

                            if (null != objectField)
                            {
                                if (null != resolver && null != objectField.Data)
                                {
                                    string file = resolver.Resolve(row.SourceLineNumbers, table.Name, (string)objectField.Data);
                                    if (!String.IsNullOrEmpty(file))
                                    {
                                        // File was successfully resolved so track the embedded index as the cabinet file id.
                                        objectField.EmbeddedFileIndex = embedFilePaths.Count;
                                        embedFilePaths.Add(file);
                                    }
                                    else
                                    {
                                        Messaging.Instance.OnMessage(WixDataErrors.FileNotFound(row.SourceLineNumbers, (string)objectField.Data, table.Name));
                                    }
                                }
                                else // clear out a previous cabinet file id value
                                {
                                    objectField.EmbeddedFileIndex = null;
                                }
                            }
                        }
                    }
                }
            }

            // do not save the library if errors were found while resolving object paths
            if (Messaging.Instance.EncounteredError)
            {
                return;
            }

            // Ensure the location to output the lib exists
            Directory.CreateDirectory(Path.GetDirectoryName(Path.GetFullPath(path)));

            using (FileStream stream = File.Create(path))
            using (FileStructure fs = FileStructure.Create(stream, FileFormat.Wixlib, embedFilePaths))
            using (XmlWriter writer = XmlWriter.Create(fs.GetDataStream()))
            {
                writer.WriteStartDocument();

                this.Write(writer);

                writer.WriteEndDocument();
            }
        }

        /// <summary>
        /// Parse the root library element.
        /// </summary>
        /// <param name="reader">XmlReader with library persisted as Xml.</param>
        /// <param name="tableDefinitions">Collection containing TableDefinitions to use when reconstituting the intermediates.</param>
        /// <param name="suppressVersionCheck">Suppresses check for wix.dll version mismatch.</param>
        /// <returns>The parsed Library.</returns>
        private static Library Read(XmlReader reader, TableDefinitionCollection tableDefinitions, bool suppressVersionCheck)
        {
            if ("wixLibrary" != reader.LocalName)
            {
                throw new WixNotLibraryException(WixDataErrors.InvalidDocumentElement(SourceLineNumber.CreateFromUri(reader.BaseURI), reader.Name, "library", "wixLibrary"));
            }

            bool empty = reader.IsEmptyElement;
            Library library = new Library();
            Version version = null;

            while (reader.MoveToNextAttribute())
            {
                switch (reader.LocalName)
                {
                    case "version":
                        version = new Version(reader.Value);
                        break;
                }
            }

            if (!suppressVersionCheck && null != version && !currentVersion.Equals(version))
            {
                throw new WixException(WixDataErrors.VersionMismatch(SourceLineNumber.CreateFromUri(reader.BaseURI), "library", version.ToString(), currentVersion.ToString()));
            }

            if (!empty)
            {
                bool done = false;

                while (!done && (XmlNodeType.Element == reader.NodeType || reader.Read()))
                {
                    switch (reader.NodeType)
                    {
                        case XmlNodeType.Element:
                            switch (reader.LocalName)
                            {
                                case "section":
                                    library.sections.Add(Section.Read(reader, tableDefinitions));
                                    break;
                                case "WixLocalization":
                                    Localization localization = Localization.Parse(reader, tableDefinitions, true);
                                    library.localizations.Add(localization.Culture, localization);
                                    break;
                            }
                            break;
                        case XmlNodeType.EndElement:
                            done = true;
                            break;
                    }
                }

                Debug.Assert(done, "Expected end element at the end of the library.");
            }

            return library;
        }

        /// <summary>
        /// Persists a library in an XML format.
        /// </summary>
        /// <param name="writer">XmlWriter where the library should persist itself as XML.</param>
        private void Write(XmlWriter writer)
        {
            writer.WriteStartElement("wixLibrary", XmlNamespaceUri);

            writer.WriteAttributeString("version", currentVersion.ToString());

            foreach (Localization localization in this.localizations.Values)
            {
                localization.Write(writer);
            }

            foreach (Section section in this.sections)
            {
                section.Write(writer);
            }

            writer.WriteEndElement();
        }
    }
}
