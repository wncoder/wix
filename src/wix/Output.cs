//-------------------------------------------------------------------------------------------------
// <copyright file="Output.cs" company="Microsoft">
//    Copyright (c) Microsoft Corporation.  All rights reserved.
//    
//    The use and distribution terms for this software are covered by the
//    Common Public License 1.0 (http://opensource.org/licenses/cpl1.0.php)
//    which can be found in the file CPL.TXT at the root of this distribution.
//    By using this software in any fashion, you are agreeing to be bound by
//    the terms of this license.
//    
//    You must not remove this notice, or any other, from this software.
// </copyright>
// 
// <summary>
// Output of linker before binding.
// TODO: Ensure that ignore modularization is correctly persisted to xml.
// TODO: Remove sections since they are not persisted.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml
{
    using System;
    using System.CodeDom.Compiler;
    using System.Collections;
    using System.Collections.Specialized;
    using System.Diagnostics;
    using System.Globalization;
    using System.IO;
    using System.Reflection;
    using System.Xml;
    using System.Xml.Schema;
    using Microsoft.Tools.WindowsInstallerXml.Cab;

    /// <summary>
    /// Various types of output.
    /// </summary>
    public enum OutputType
    {
        /// <summary>Unknown output type.</summary>
        Unknown,

        /// <summary>Bundle output type.</summary>
        Bundle,

        /// <summary>Module output type.</summary>
        Module,

        /// <summary>Patch output type.</summary>
        Patch,

        /// <summary>Patch Creation output type.</summary>
        PatchCreation,

        /// <summary>Product output type.</summary>
        Product,

        /// <summary>Transform output type.</summary>
        Transform
    }

    /// <summary>
    /// Output is generated by the linker.
    /// </summary>
    public sealed class Output
    {
        public const string XmlNamespaceUri = "http://schemas.microsoft.com/wix/2006/outputs";
        private static readonly Version currentVersion = new Version("3.0.2002.0");

        private static XmlSchemaCollection schemas;
        private static readonly object lockObject = new object();

        private Section entrySection;
        private OutputType type;
        private int codepage;

        private SectionCollection sections;
        private SourceLineNumberCollection sourceLineNumbers;

        private ArrayList subStorages;

        private TableCollection tables;

        private TempFileCollection tempFileCollection;
        private string cabPath;

        /// <summary>
        /// Creates a new empty output object.
        /// </summary>
        /// <param name="sourceLineNumbers">The source line information for the output.</param>
        internal Output(SourceLineNumberCollection sourceLineNumbers)
        {
            this.sections = new SectionCollection();
            this.sourceLineNumbers = sourceLineNumbers;
            this.subStorages = new ArrayList();
            this.tables = new TableCollection();
        }

        /// <summary>
        /// Gets the entry section for the output
        /// </summary>
        /// <value>Entry section for the output.</value>
        public Section EntrySection
        {
            get
            {
                return this.entrySection;
            }

            set
            {
                this.entrySection = value;
                this.codepage = value.Codepage;

                switch (this.entrySection.Type)
                {
                    case SectionType.Bundle:
                        this.type = OutputType.Bundle;
                        break;
                    case SectionType.Product:
                        this.type = OutputType.Product;
                        break;
                    case SectionType.Module:
                        this.type = OutputType.Module;
                        break;
                    case SectionType.PatchCreation:
                        this.type = OutputType.PatchCreation;
                        break;
                    case SectionType.Patch:
                        this.type = OutputType.Patch;
                        break;
                    default:
                        throw new InvalidOperationException(String.Format(CultureInfo.CurrentUICulture, WixStrings.EXP_UnexpectedEntrySectionType, this.entrySection.Type));
                }
            }
        }

        /// <summary>
        /// Gets the substorages in this output.
        /// </summary>
        /// <value>The substorages in this output.</value>
        public ArrayList SubStorages
        {
            get { return this.subStorages; }
        }

        /// <summary>
        /// Gets the type of the output.
        /// </summary>
        /// <value>Type of the output.</value>
        public OutputType Type
        {
            get { return this.type; }
            set { this.type = value; }
        }

        /// <summary>
        /// Gets or sets the codepage for this output.
        /// </summary>
        /// <value>Codepage of the output.</value>
        public int Codepage
        {
            get { return this.codepage; }
            set { this.codepage = value; }
        }

        /// <summary>
        /// Gets the sections contained in the output.
        /// </summary>
        /// <value>Sections in the output.</value>
        public SectionCollection Sections
        {
            get { return this.sections; }
        }

        /// <summary>
        /// Gets the source line information for this output.
        /// </summary>
        /// <value>The source line information for this output.</value>
        public SourceLineNumberCollection SourceLineNumbers
        {
            get { return this.sourceLineNumbers; }
        }

        /// <summary>
        /// Gets the tables contained in this output.
        /// </summary>
        /// <value>Collection of tables.</value>
        public TableCollection Tables
        {
            get { return this.tables; }
        }

        /// <summary>
        /// Gets the cabPath for that cab that was contained in this output.
        /// </summary>
        /// <value>Path to the extracted cabinet from this output.</value>
        public string CabPath
        {
            get { return this.cabPath; }
        }

        /// <summary>
        /// Gets the output type corresponding to a given output filename extension.
        /// </summary>
        /// <param name="extension">Case-insensitive output filename extension.</param>
        /// <returns>Output type for the extension.</returns>
        public static OutputType GetOutputType(string extension)
        {
            if (String.Equals(extension, ".exe", StringComparison.OrdinalIgnoreCase))
            {
                return OutputType.Bundle;
            }
            if (String.Equals(extension, ".msi", StringComparison.OrdinalIgnoreCase))
            {
                return OutputType.Product;
            }
            else if (String.Equals(extension, ".msm", StringComparison.OrdinalIgnoreCase))
            {
                return OutputType.Module;
            }
            else if (String.Equals(extension, ".msp", StringComparison.OrdinalIgnoreCase))
            {
                return OutputType.Patch;
            }
            else if (String.Equals(extension, ".mst", StringComparison.OrdinalIgnoreCase))
            {
                return OutputType.Transform;
            }
            else if (String.Equals(extension, ".pcp", StringComparison.OrdinalIgnoreCase))
            {
                return OutputType.PatchCreation;
            }
            else
            {
                return OutputType.Unknown;
            }
        }

        /// <summary>
        /// Gets the filename extension corresponding to a given output type.
        /// </summary>
        /// <param name="type">One of the WiX output types.</param>
        /// <returns>Filename extension for the output type, for example ".msi".</returns>
        public static string GetExtension(OutputType type)
        {
            switch (type)
            {
                case OutputType.Bundle:
                    return ".exe";
                case OutputType.Product:
                    return ".msi";
                case OutputType.Module:
                    return ".msm";
                case OutputType.Patch:
                    return ".msp";
                case OutputType.Transform:
                    return ".mst";
                case OutputType.PatchCreation:
                    return ".pcp";
                default:
                    return ".wix";
            }
        }

        /// <summary>
        /// Loads an output from a path on disk.
        /// </summary>
        /// <param name="path">Path to output file saved on disk.</param>
        /// <param name="suppressVersionCheck">Suppresses wix.dll version mismatch check.</param>
        /// <param name="suppressSchema">Suppress xml schema validation while loading.</param>
        /// <returns>Output object.</returns>
        public static Output Load(string path, bool suppressVersionCheck, bool suppressSchema)
        {
            try
            {
                using (FileStream stream = File.OpenRead(path))
                {
                    return Load(stream, new Uri(Path.GetFullPath(path)), suppressVersionCheck, suppressSchema);
                }
            }
            catch (FileNotFoundException)
            {
                throw new WixException(WixErrors.WixFileNotFound(path));
            }
        }

        /// <summary>
        /// Saves an outputs cab to a path on disk.
        /// </summary>
        /// <param name="path">Path to save outputs cab to on disk.</param>
        /// <param name="binderFileManager">If provided, the binder file manager is used to bind files into the outputs cab.</param>
        /// <param name="wixVariableResolver">The Wix variable resolver.</param>
        /// <param name="tempFilesLocation">Location for temporary files.</param>
        /// <returns>Returns true if a cabinet existed or was created, false otherwise.</returns>
        public bool SaveCab(string path, BinderFileManager binderFileManager, WixVariableResolver wixVariableResolver, string tempFilesLocation)
        {
            bool hasCab = false;
            // Check if there was a cab on the wixout when it was created
            if (null != this.cabPath)
            {
                // There was already a cab on the wixout when it was loaded. Reuse that one.
                File.Copy(this.cabPath, path, true);
                if (null != this.tempFileCollection)
                {
                    this.tempFileCollection.Delete();
                }
                hasCab = true;
            }
            else
            {
                int index = 0;
                Hashtable cabinets = new Hashtable();
                StringCollection fileIds = new StringCollection();
                StringCollection files = new StringCollection();

                if (null != tempFilesLocation)
                {
                    // resolve paths to files and create the output cabinet file
                    if (0 == this.sections.Count)
                    {
                        Output.ResolveSectionFiles(this.tables, binderFileManager, wixVariableResolver, tempFilesLocation, cabinets, fileIds, files, ref index);
                    }
                    else
                    {
                        foreach (Section section in this.sections)
                        {
                            Output.ResolveSectionFiles(section.Tables, binderFileManager, wixVariableResolver, tempFilesLocation, cabinets, fileIds, files, ref index);
                        }
                    }
                }

                // extract files that come from cabinet files
                if (0 < cabinets.Count)
                {
                    // ensure the temporary directory exists
                    Directory.CreateDirectory(tempFilesLocation);

                    foreach (DictionaryEntry cabinet in cabinets)
                    {
                        Uri baseUri = new Uri((string)cabinet.Key);
                        string localPath;

                        if ("embeddedresource" == baseUri.Scheme)
                        {
                            int bytesRead;
                            byte[] buffer = new byte[512];

                            string originalLocalPath = Path.GetFullPath(baseUri.LocalPath.Substring(1));
                            string resourceName = baseUri.Fragment.Substring(1);
                            Assembly assembly = Assembly.LoadFile(originalLocalPath);

                            localPath = String.Concat(cabinet.Value, ".cab");

                            using (FileStream fs = File.OpenWrite(localPath))
                            {
                                using (Stream resourceStream = assembly.GetManifestResourceStream(resourceName))
                                {
                                    while (0 < (bytesRead = resourceStream.Read(buffer, 0, buffer.Length)))
                                    {
                                        fs.Write(buffer, 0, bytesRead);
                                    }
                                }
                            }
                        }
                        else // normal file
                        {
                            localPath = baseUri.LocalPath;
                        }

                        // extract the cabinet's files into a temporary directory
                        Directory.CreateDirectory((string)cabinet.Value);

                        using (WixExtractCab extractCab = new WixExtractCab())
                        {
                            extractCab.Extract(localPath, (string)cabinet.Value);
                        }
                    }
                }

                // create the cabinet file
                if (0 < fileIds.Count)
                {
                    using (WixCreateCab cab = new WixCreateCab(Path.GetFileName(path), Path.GetDirectoryName(path), fileIds.Count, 0, 0, CompressionLevel.Mszip))
                    {
                        for (int i = 0; i < fileIds.Count; i++)
                        {
                            cab.AddFile(files[i], fileIds[i]);
                        }
                        cab.Complete();
                    }

                    // append the output xml to the end of the newly created cabinet file
                    hasCab = true;
                }
            }

            return hasCab;
        }

        /// <summary>
        /// Saves an output to a path on disk.
        /// </summary>
        /// <param name="path">Path to save output file to on disk.</param>
        /// <param name="binderFileManager">If provided, the binder file manager is used to bind files into the output.</param>
        /// <param name="wixVariableResolver">The Wix variable resolver.</param>
        /// <param name="tempFilesLocation">Location for temporary files.</param>
        public void Save(string path, BinderFileManager binderFileManager, WixVariableResolver wixVariableResolver, string tempFilesLocation)
        {
            FileMode fileMode = FileMode.Create;

            // Assure the location to output the xml exists
            Directory.CreateDirectory(Path.GetDirectoryName(Path.GetFullPath(path)));

            // Check if there was a cab on the output when it was created
            if (SaveCab(path, binderFileManager, wixVariableResolver, tempFilesLocation))
            {
                fileMode = FileMode.Append;
            }

            // save the xml
            using (FileStream fs = new FileStream(path, fileMode))
            {
                XmlWriter writer = null;

                try
                {
                    writer = new XmlTextWriter(fs, System.Text.Encoding.UTF8);

                    writer.WriteStartDocument();
                    this.Persist(writer);
                    writer.WriteEndDocument();
                }
                finally
                {
                    if (null != writer)
                    {
                        writer.Close();
                    }
                }
            }
        }

        /// <summary>
        /// Resolves paths to files.
        /// </summary>
        /// <param name="sectionTables">TableCollection of tables to process</param>
        /// <param name="binderFileManager">If provided, the binder file manager is used to bind files into the output.</param>
        /// <param name="wixVariableResolver">The Wix variable resolver.</param>
        /// <param name="tempFilesLocation">Location for temporary files.</param>
        /// <param name="cabinets">Hash of source cabinets.</param>
        /// <param name="fileIds">Collection of CabinetFileIds.</param>
        /// <param name="files">Collection of file paths from compressed files.</param>
        /// <param name="index">CabinetFileId generator.</param>
        public static void ResolveSectionFiles(TableCollection sectionTables, BinderFileManager binderFileManager, WixVariableResolver wixVariableResolver, string tempFilesLocation, Hashtable cabinets, StringCollection fileIds, StringCollection files, ref int index)
        {
            foreach (Table table in sectionTables)
            {
                foreach (Row row in table.Rows)
                {
                    foreach (Field field in row.Fields)
                    {
                        ObjectField objectField = field as ObjectField;

                        if (null != objectField && null != objectField.Data)
                        {
                            string file = null;
                            string previousFile = null;
                            bool isDefault = true;
                            bool isPreviousDefault = true;

                            // resolve localization and wix variables if there is a file manager that would use the value
                            // if it was different, otherwise we just don't care so skip the whole variable resolution thing.
                            if (null != wixVariableResolver && null != binderFileManager)
                            {
                                objectField.Data = wixVariableResolver.ResolveVariables(row.SourceLineNumbers, (string)objectField.Data, false, ref isDefault);

                                if (null != objectField.PreviousData)
                                {
                                    objectField.PreviousData = wixVariableResolver.ResolveVariables(row.SourceLineNumbers, objectField.PreviousData, false, ref isPreviousDefault);
                                }

                                // do not save the output if errors were found while resolving object paths
                                if (wixVariableResolver.EncounteredError)
                                {
                                    return;
                                }
                            }

                            // file is compressed in a cabinet (and not modified above)
                            if (null != objectField.CabinetFileId && isDefault)
                            {
                                // index cabinets that have not been previously encountered
                                if (!cabinets.ContainsKey(objectField.BaseUri))
                                {
                                    Uri baseUri = new Uri(objectField.BaseUri);
                                    string localFileNameWithoutExtension = Path.GetFileNameWithoutExtension(baseUri.LocalPath);
                                    string extractedDirectoryName = String.Format(CultureInfo.InvariantCulture, "cab_{0}_{1}", cabinets.Count, localFileNameWithoutExtension);

                                    // index the cabinet file's base URI (source location) and extracted directory
                                    cabinets.Add(objectField.BaseUri, Path.Combine(tempFilesLocation, extractedDirectoryName));
                                }

                                // set the path to the file once its extracted from the cabinet
                                file = Path.Combine((string)cabinets[objectField.BaseUri], objectField.CabinetFileId);
                            }
                            else if (null != binderFileManager)
                            {
                                file = binderFileManager.ResolveFile((string)objectField.Data);
                            }

                            // add the file to the list of files to go in the cabinet
                            if (null != file)
                            {
                                string cabinetFileId = (index++).ToString(CultureInfo.InvariantCulture);

                                objectField.CabinetFileId = cabinetFileId;
                                fileIds.Add(cabinetFileId);

                                files.Add(file);
                            }

                            // previous file is compressed in a cabinet (and not modified above)
                            if (null != objectField.PreviousCabinetFileId && isPreviousDefault)
                            {
                                // index cabinets that have not been previously encountered
                                if (!cabinets.ContainsKey(objectField.PreviousBaseUri))
                                {
                                    Uri baseUri = new Uri(objectField.PreviousBaseUri);
                                    string localFileNameWithoutExtension = Path.GetFileNameWithoutExtension(baseUri.LocalPath);
                                    string extractedDirectoryName = String.Format(CultureInfo.InvariantCulture, "cab_{0}_{1}", cabinets.Count, localFileNameWithoutExtension);

                                    // index the cabinet file's base URI (source location) and extracted directory
                                    cabinets.Add(objectField.PreviousBaseUri, Path.Combine(tempFilesLocation, extractedDirectoryName));
                                }

                                // set the path to the file once its extracted from the cabinet
                                previousFile = Path.Combine((string)cabinets[objectField.PreviousBaseUri], objectField.PreviousCabinetFileId);
                            }
                            else if (null != objectField.PreviousData && null != binderFileManager)
                            {
                                previousFile = binderFileManager.ResolveFile((string)objectField.PreviousData);
                            }

                            // add the file to the list of files to go in the cabinet
                            if (null != previousFile)
                            {
                                string cabinetFileId = (index++).ToString(CultureInfo.InvariantCulture);

                                objectField.PreviousCabinetFileId = cabinetFileId;
                                fileIds.Add(cabinetFileId);

                                files.Add(previousFile);
                            }
                        }
                    }
                }
            }
        }

        internal TempFileCollection TempFiles
        {
            get
            {
                return this.tempFileCollection;
            }
            set
            {
                this.tempFileCollection = value;
            }
        }

        /// <summary>
        /// Loads an output from a path on disk.
        /// </summary>
        /// <param name="stream">Stream containing the output file.</param>
        /// <param name="uri">Uri for finding this stream.</param>
        /// <param name="suppressVersionCheck">Suppresses wix.dll version mismatch check.</param>
        /// <param name="suppressSchema">Suppress xml schema validation while loading.</param>
        /// <returns>Returns the loaded output.</returns>
        /// <remarks>This method will set the Path and SourcePath properties to the appropriate values on successful load.</remarks>
        internal static Output Load(Stream stream, Uri uri, bool suppressVersionCheck, bool suppressSchema)
        {
            XmlReader reader = null;
            TempFileCollection tempFileCollection = null;
            string cabPath = null;

            // look for the Microsoft cabinet file header and save the cabinet data if found
            if ('M' == stream.ReadByte() && 'S' == stream.ReadByte() && 'C' == stream.ReadByte() && 'F' == stream.ReadByte())
            {
                long cabFileSize = 0;
                byte[] offsetBuffer = new byte[4];
                tempFileCollection = new TempFileCollection();
                cabPath = tempFileCollection.AddExtension("cab", false);

                // skip the header checksum
                stream.Seek(4, SeekOrigin.Current);

                // get the cabinet file size
                stream.Read(offsetBuffer, 0, 4);
                cabFileSize = BitConverter.ToInt32(offsetBuffer, 0);

                stream.Seek(0, SeekOrigin.Begin);

                // Create the cab file from stream
                using (FileStream fs = File.Create(cabPath))
                {
                    for (int i = 0; i < cabFileSize; i++)
                    {
                        fs.WriteByte((byte)stream.ReadByte());
                    }
                }
            }
            else // plain xml file - start reading xml at the beginning of the stream
            {
                stream.Seek(0, SeekOrigin.Begin);
            }

            // read the xml
            try
            {
                reader = new XmlTextReader(uri.AbsoluteUri, stream);

                if (!suppressSchema)
                {
                    reader = new XmlValidatingReader(reader);
                    ((XmlValidatingReader)reader).Schemas.Add(GetSchemas());
                }

                reader.MoveToContent();

                if ("wixOutput" != reader.LocalName)
                {
                    throw new WixNotOutputException(WixErrors.InvalidDocumentElement(SourceLineNumberCollection.FromUri(reader.BaseURI), reader.Name, "output", "wixOutput"));
                }

                Output output = Parse(reader, suppressVersionCheck);
                output.tempFileCollection = tempFileCollection;
                output.cabPath = cabPath;

                return output;
            }
            catch (XmlException xe)
            {
                throw new WixException(WixErrors.InvalidXml(SourceLineNumberCollection.FromUri(reader.BaseURI), "output", xe.Message));
            }
            catch (XmlSchemaException xse)
            {
                throw new WixException(WixErrors.SchemaValidationFailed(SourceLineNumberCollection.FromUri(reader.BaseURI), xse.Message, xse.LineNumber, xse.LinePosition));
            }
            finally
            {
                if (null != reader)
                {
                    reader.Close();
                }
            }
        }

        /// <summary>
        /// Get the schemas required to validate an object.
        /// </summary>
        /// <returns>The schemas required to validate an object.</returns>
        internal static XmlSchemaCollection GetSchemas()
        {
            lock (lockObject)
            {
                if (null == schemas)
                {
                    Assembly assembly = Assembly.GetExecutingAssembly();

                    using (Stream outputSchemaStream = assembly.GetManifestResourceStream("Microsoft.Tools.WindowsInstallerXml.Xsd.outputs.xsd"))
                    {
                        schemas = new XmlSchemaCollection();
                        schemas.Add(Intermediate.GetSchemas());
                        schemas.Add(TableDefinitionCollection.GetSchemas());
                        XmlSchema outputSchema = XmlSchema.Read(outputSchemaStream, null);
                        schemas.Add(outputSchema);
                    }
                }
            }

            return schemas;
        }

        /// <summary>
        /// Processes an XmlReader and builds up the output object.
        /// </summary>
        /// <param name="reader">Reader to get data from.</param>
        /// <param name="suppressVersionCheck">Suppresses wix.dll version mismatch check.</param>
        /// <returns>The Output represented by the Xml.</returns>
        internal static Output Parse(XmlReader reader, bool suppressVersionCheck)
        {
            Debug.Assert("wixOutput" == reader.LocalName);

            bool empty = reader.IsEmptyElement;
            Output output = new Output(SourceLineNumberCollection.FromUri(reader.BaseURI));
            SectionType sectionType = SectionType.Unknown;
            Version version = null;

            while (reader.MoveToNextAttribute())
            {
                switch (reader.LocalName)
                {
                    case "codepage":
                        output.codepage = Convert.ToInt32(reader.Value, CultureInfo.InvariantCulture.NumberFormat);
                        break;
                    case "type":
                        switch (reader.Value)
                        {
                            case "Bundle":
                                output.type = OutputType.Bundle;
                                sectionType = SectionType.Bundle;
                                break;
                            case "Module":
                                output.type = OutputType.Module;
                                sectionType = SectionType.Module;
                                break;
                            case "Patch":
                                output.type = OutputType.Patch;
                                break;
                            case "PatchCreation":
                                output.type = OutputType.PatchCreation;
                                sectionType = SectionType.PatchCreation;
                                break;
                            case "Product":
                                output.type = OutputType.Product;
                                sectionType = SectionType.Product;
                                break;
                            case "Transform":
                                output.type = OutputType.Transform;
                                break;
                            default:
                                throw new WixException(WixErrors.IllegalAttributeValue(SourceLineNumberCollection.FromUri(reader.BaseURI), "wixOutput", reader.Name, reader.Value, "Module", "Patch", "PatchCreation", "Product", "Transform"));
                        }
                        break;
                    case "version":
                        version = new Version(reader.Value);
                        break;
                    default:
                        if (!reader.NamespaceURI.StartsWith("http://www.w3.org/", StringComparison.Ordinal))
                        {
                            throw new WixException(WixErrors.UnexpectedAttribute(SourceLineNumberCollection.FromUri(reader.BaseURI), "wixOutput", reader.Name));
                        }
                        break;
                }
            }

            if (null != version && !suppressVersionCheck)
            {
                if (0 != currentVersion.CompareTo(version))
                {
                    throw new WixException(WixErrors.VersionMismatch(SourceLineNumberCollection.FromUri(reader.BaseURI), "wixOutput", version.ToString(), currentVersion.ToString()));
                }
            }

            // create a section for all the rows to belong to
            output.entrySection = new Section(null, sectionType, output.codepage);

            TableDefinitionCollection tableDefinitions = null;

            // loop through the rest of the xml building up the Output object
            if (!empty)
            {
                bool done = false;

                // loop through all the fields in a row
                while (!done && reader.Read())
                {
                    switch (reader.NodeType)
                    {
                        case XmlNodeType.Element:
                            switch (reader.LocalName)
                            {
                                case "subStorage":
                                    output.SubStorages.Add(SubStorage.Parse(reader));
                                    break;
                                case "table":
                                    if (null == tableDefinitions)
                                    {
                                        throw new WixException(WixErrors.ExpectedElement(SourceLineNumberCollection.FromUri(reader.BaseURI), "wixOutput", "tableDefinitions"));
                                    }
                                    output.Tables.Add(Table.Parse(reader, output.entrySection, tableDefinitions));
                                    break;
                                case "tableDefinitions":
                                    tableDefinitions = TableDefinitionCollection.Parse(reader);
                                    break;
                                default:
                                    throw new WixException(WixErrors.UnexpectedElement(SourceLineNumberCollection.FromUri(reader.BaseURI), "wixOutput", reader.Name));
                            }
                            break;
                        case XmlNodeType.EndElement:
                            done = true;
                            break;
                    }
                }

                if (!done)
                {
                    throw new WixException(WixErrors.ExpectedEndElement(SourceLineNumberCollection.FromUri(reader.BaseURI), "wixOutput"));
                }
            }

            return output;
        }

        /// <summary>
        /// Ensure this output contains a particular table.
        /// </summary>
        /// <param name="tableDefinition">Definition of the table that should exist.</param>
        /// <returns>The table in this output.</returns>
        internal Table EnsureTable(TableDefinition tableDefinition)
        {
            return this.tables.EnsureTable(this.entrySection, tableDefinition);
        }

        /// <summary>
        /// Persists an output in an XML format.
        /// </summary>
        /// <param name="writer">XmlWriter where the Output should persist itself as XML.</param>
        internal void Persist(XmlWriter writer)
        {
            writer.WriteStartElement("wixOutput", XmlNamespaceUri);

            writer.WriteAttributeString("type", this.type.ToString());

            if (0 != this.codepage)
            {
                writer.WriteAttributeString("codepage", this.codepage.ToString(CultureInfo.InvariantCulture));
            }

            writer.WriteAttributeString("version", currentVersion.ToString());

            // collect all the table defintitions and write them
            TableDefinitionCollection tableDefinitions = new TableDefinitionCollection();
            foreach (Table table in this.tables)
            {
                tableDefinitions.Add(table.Definition);
            }
            tableDefinitions.Persist(writer);

            foreach (Table table in this.tables)
            {
                table.Persist(writer);
            }

            foreach (SubStorage subStorage in this.subStorages)
            {
                subStorage.Persist(writer);
            }

            writer.WriteEndElement();
        }
    }
}
