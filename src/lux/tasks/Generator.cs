//-------------------------------------------------------------------------------------------------
// <copyright file="Generator.cs" company="Microsoft">
//    Copyright (c) Microsoft Corporation.  All rights reserved.
//    
//    The use and distribution terms for this software are covered by the
//    Common Public License 1.0 (http://opensource.org/licenses/cpl.php)
//    which can be found in the file CPL.TXT at the root of this distribution.
//    By using this software in any fashion, you are agreeing to be bound by
//    the terms of this license.
//    
//    You must not remove this notice, or any other, from this software.
// </copyright>
// 
// <summary>
// Helper class to scan object files for unit tests.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Lux
{
    using System;
    using System.Collections.Generic;
    using System.Globalization;
    using System.IO;
    using System.Xml;
    using Microsoft.Tools.WindowsInstallerXml;

    using Wix = Microsoft.Tools.WindowsInstallerXml.Serialize;
    using WixLux = Microsoft.Tools.WindowsInstallerXml.Extensions.Serialize.Lux;

    /// <summary>
    /// Helper class to scan objects for unit tests.
    /// </summary>
    public sealed class Generator : IMessageHandler
    {
        private List<string> extensions = new List<string>();
        private List<string> inputFiles = new List<string>();
        private List<string> inputFilesWithUnitTests;
        private string outputFile;

        public List<string> Extensions
        {
            set
            {
                this.extensions = value;
            }
        }

        public List<string> InputFiles
        {
            get
            {
                return this.inputFiles;
            }

            set
            {
                this.inputFiles = value;
            }
        }

        public List<string> InputFilesWithUnitTests
        {
            get
            {
                return this.inputFilesWithUnitTests;
            }
        }

        public string OutputFile
        {
            set
            {
                this.outputFile = value;
            }
        }

        /// <summary>
        /// Event for messages.
        /// </summary>
        public event MessageEventHandler Message;

        public static bool GenerateTestProductSourceFile(List<string> extensions, List<string> inputFiles, string outputFile, MessageEventHandler message, out List<string> inputFilesWithUnitTests)
        {
            Generator generator = new Generator();
            generator.Extensions = extensions;
            generator.InputFiles = inputFiles;
            generator.OutputFile = outputFile;
            generator.Message += message;

            bool success = generator.GenerateTestProductSourceFile();
            inputFilesWithUnitTests = generator.InputFilesWithUnitTests;
            return success;
        }

        public bool GenerateTestProductSourceFile()
        {
            // get the unit tests included in all the objects
            List<string> unitTestIds = this.FindUnitTests();
            if (null == unitTestIds || 0 == unitTestIds.Count)
            {
                return false;
            }

            // and write the WiX source that consumes them all
            this.GenerateTestSource(unitTestIds);
            return true;
        }

        /// <summary>
        /// Find all the unit tests from the WixUnitTest tables in all the input files' sections.
        /// </summary>
        /// <returns>Returns a list of unit test ids.</returns>
        private List<string> FindUnitTests()
        {
            // get the primary keys for every row from every WixUnitTest table in our sections:
            // voila, we have our unit test ids
            this.inputFilesWithUnitTests = new List<string>();
            List<string> unitTestIds = new List<string>();
            Dictionary<Section, string> sections = this.LoadSections();

            if (null != sections && 0 < sections.Count)
            {
                foreach (Section section in sections.Keys)
                {
                    string file = sections[section];
                    Table unitTestTable = section.Tables["WixUnitTest"];
                    if (null == unitTestTable)
                    {
                        continue; // nothing to see here
                    }

                    if (!this.inputFilesWithUnitTests.Contains(file))
                    {
                        this.inputFilesWithUnitTests.Add(file);
                    }

                    foreach (Row row in unitTestTable.Rows)
                    {
                        unitTestIds.Add(row.GetPrimaryKey('/'));

                        string customAction = (string)row[1];
                        string property = (string)row[2];
                        string value = (string)row[4];
                        string expression = (string)row[5];
                        string index = (string)row[9];

                        if (!String.IsNullOrEmpty(expression))
                        {
                            this.OnMessage(LuxVerboses.FoundExpressionTest(customAction, expression));
                        }
                        else if (!String.IsNullOrEmpty(index))
                        {
                            this.OnMessage(LuxVerboses.FoundMultiValueTest(customAction, property, index, value));
                        }
                        else
                        {
                            this.OnMessage(LuxVerboses.FoundSimpleTest(customAction, property, value));
                        }
                    }
                }
            }

            return unitTestIds;
        }

        /// <summary>
        /// Generates a WiX serialization object tree for a product that consumes the
        /// given unit tests.
        /// </summary>
        /// <param name="unitTestIds">List of unit test ids.</param>
        private void GenerateTestSource(List<string> unitTestIds)
        {
            Wix.Product product = new Wix.Product();
            product.Id = "*";
            product.Language = "1033";
            product.Manufacturer = "Lux";
            product.Name = Path.GetFileNameWithoutExtension(this.outputFile) + " Lux test project";
            product.Version = "1.0";
            product.UpgradeCode = Guid.NewGuid().ToString("B").ToUpper(CultureInfo.InvariantCulture);

            Wix.Package package = new Wix.Package();
            package.Compressed = Wix.YesNoType.yes;
            package.InstallScope = Wix.Package.InstallScopeType.perUser;
            product.AddChild(package);

            Wix.Media media = new Wix.Media();
            media.Id = 1;
            media.Cabinet = "luxtest.cab";
            media.EmbedCab = Wix.YesNoType.yes;
            product.AddChild(media);

            Wix.Directory targetDir = new Wix.Directory();
            targetDir.Id = "TARGETDIR";
            targetDir.Name = "SourceDir";
            product.AddChild(targetDir);

            foreach (string unitTestId in unitTestIds)
            {
                WixLux.UnitTestRef unitTestRef = new WixLux.UnitTestRef();
                unitTestRef.Id = unitTestId;
                product.AddChild(unitTestRef);
            }

            Wix.Wix wix = new Wix.Wix();
            wix.AddChild(product);

            // now write to the file
            XmlWriterSettings settings = new XmlWriterSettings();
            settings.Indent = true;

            this.OnMessage(LuxVerboses.GeneratingConsumer(this.outputFile, unitTestIds.Count));
            using (XmlWriter writer = XmlWriter.Create(this.outputFile, settings))
            {
                writer.WriteStartDocument();
                wix.OutputXml(writer);
                writer.WriteEndDocument();
            }
        }

        /// <summary>
        /// Load sections from the input files.
        /// </summary>
        /// <returns>Returns a section collection.</returns>
        private Dictionary<Section, string> LoadSections()
        {
            // we need a Linker and the extensions for their table definitions
            Linker linker = new Linker();
            linker.Message += new MessageEventHandler(this.Message);

            if (null != this.extensions)
            {
                foreach (string extension in this.extensions)
                {
                    WixExtension wixExtension = WixExtension.Load(extension);
                    linker.AddExtension(wixExtension);
                }
            }

            // load each intermediate and library file and get their sections
            Dictionary<Section, string> sectionFiles = new Dictionary<Section, string>();

            if (null != this.inputFiles)
            {
                foreach (string inputFile in this.inputFiles)
                {
                    string inputFileFullPath = Path.GetFullPath(inputFile);

                    // try loading as an object file
                    try
                    {
                        Intermediate intermediate = Intermediate.Load(inputFileFullPath, linker.TableDefinitions, false, false);
                        foreach (Section section in intermediate.Sections)
                        {
                            sectionFiles[section] = inputFileFullPath;
                        }
                        continue; // next file
                    }
                    catch (WixNotIntermediateException)
                    {
                        // try another format
                    }

                    // try loading as a library file
                    try
                    {
                        Library library = Library.Load(inputFileFullPath, linker.TableDefinitions, false, false);
                        foreach (Section section in library.Sections)
                        {
                            sectionFiles[section] = inputFileFullPath;
                        }
                        continue; // next file
                    }
                    catch (WixNotLibraryException)
                    {
                        this.OnMessage(LuxErrors.CouldntLoadInput(inputFile));
                    }
                }
            }

            return sectionFiles;
        }

        /// <summary>
        /// Sends a message to the message delegate if there is one.
        /// </summary>
        /// <param name="mea">Message event arguments.</param>
        public void OnMessage(MessageEventArgs mea)
        {
            WixErrorEventArgs errorEventArgs = mea as WixErrorEventArgs;

            if (null != this.Message)
            {
                this.Message(this, mea);
            }
            else if (null != errorEventArgs)
            {
                throw new WixException(errorEventArgs);
            }
        }
    }
}
