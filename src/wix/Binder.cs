//-------------------------------------------------------------------------------------------------
// <copyright file="Binder.cs" company="Microsoft">
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
// Binder core of the Windows Installer Xml toolset.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml
{
    using System;
    using System.Collections;
    using System.Collections.Generic;
    using System.Collections.Specialized;
    using System.ComponentModel;
    using System.Diagnostics;
    using System.Diagnostics.CodeAnalysis;
    using System.Globalization;
    using System.IO;
    using System.Reflection;
    using System.Runtime.InteropServices;
    using System.Security.Cryptography;
    using System.Text;
    using System.Xml;
    using System.Xml.XPath;
    using Microsoft.Tools.WindowsInstallerXml.Cab;
    using Microsoft.Tools.WindowsInstallerXml.CLR.Interop;
    using Microsoft.Tools.WindowsInstallerXml.MergeMod;
    using Microsoft.Tools.WindowsInstallerXml.Msi;
    using Microsoft.Tools.WindowsInstallerXml.Msi.Interop;

    // TODO: (4.0) Refactor so that these don't need to be copied.
    // Copied verbatim from ext\UtilExtension\wixext\UtilCompiler.cs
    [Flags]
    internal enum WixFileSearchAttributes
    {
        Default = 0,
        MinVersionInclusive = 0x1,
        MaxVersionInclusive = 0x2,
        MinSizeInclusive = 0x4,
        MaxSizeInclusive = 0x8,
        MinDateInclusive = 0x10,
        MaxDateInclusive = 0x20,
        WantVersion = 0x40,
        WantExists = 0x80,
        IsDirectory = 0x100,
    }

    [Flags]
    internal enum WixRegistrySearchAttributes
    {
        Raw = 0,
        Compatible = 0x1,
        ExpandEnvironmentVariables = 0x2,
        WantExists = 0x4,
    }

    [Flags]
    internal enum WixComponentSearchAttributes
    {
        KeyPath = 0,
        State = 0x1,
        WantDirectory = 0x2,
    }

    [Flags]
    internal enum WixProductSearchAttributes
    {
        Version = 0,
        Language = 0x1,
        State = 0x2,
        Assignment = 0x4,
    }


    /// <summary>
    /// Binder core of the Windows Installer Xml toolset.
    /// </summary>
    public sealed class Binder : WixBinder, IDisposable
    {
        private const int VisitedActionSentinel = -10;
        private static readonly char[] tabCharacter = "\t".ToCharArray();

        private const string BurnNamespace = "http://schemas.microsoft.com/wix/2008/Burn";
        private const string BurnUXContainerEmbeddedIdFormat = "u{0}";
        private const string BurnUXContainerPayloadIdFormat = "p{0}";
        private const string BurnAttachedContainerEmbeddedIdFormat = "a{0}";

        private string emptyFile;

        private bool backwardsCompatibleGuidGen;
        private int cabbingThreadCount;
        private string cabCachePath;
        private Cab.CompressionLevel defaultCompressionLevel;
        private bool exactAssemblyVersions;
        private bool setMsiAssemblyNameFileVersion;
        private string pdbFile;
        private bool reuseCabinets;
        private bool suppressAssemblies;
        private bool suppressAclReset;
        private bool suppressFileHashAndInfo;
        private StringCollection suppressICEs;
        private bool suppressLayout;
        private bool suppressWixPdb;
        private bool suppressValidation;

        private StringCollection ices;
        private StringCollection invalidArgs;
        private bool suppressAddingValidationRows;
        private Validator validator;

        /// <summary>
        /// Creates an MSI binder.
        /// </summary>
        public Binder()
        {
            this.defaultCompressionLevel = Cab.CompressionLevel.Mszip;
            this.suppressICEs = new StringCollection();

            this.ices = new StringCollection();
            this.invalidArgs = new StringCollection();
            this.validator = new Validator();
        }

        /// <summary>
        /// Gets or sets whether the GUID generation should use a backwards
        /// compatible version (i.e. MD5).
        /// </summary>
        public bool BackwardsCompatibleGuidGen
        {
            get { return this.backwardsCompatibleGuidGen; }
            set { this.backwardsCompatibleGuidGen = value; }
        }

        /// <summary>
        /// Gets or sets the number of threads to use for cabinet creation.
        /// </summary>
        /// <value>The number of threads to use for cabinet creation.</value>
        public int CabbingThreadCount
        {
            get { return this.cabbingThreadCount; }
            set { this.cabbingThreadCount = value; }
        }

        /// <summary>
        /// Gets or sets the default compression level to use for cabinets
        /// that don't have their compression level explicitly set.
        /// </summary>
        public Cab.CompressionLevel DefaultCompressionLevel
        {
            get { return this.defaultCompressionLevel; }
            set { this.defaultCompressionLevel = value; }
        }

        /// <summary>
        /// Gets or sets the exact assembly versions flag (see docs).
        /// </summary>
        public bool ExactAssemblyVersions
        {
            get { return this.exactAssemblyVersions; }
            set { this.exactAssemblyVersions = value; }
        }

        /// <summary>
        /// Gets and sets the location to save the WixPdb.
        /// </summary>
        /// <value>The location in which to save the WixPdb. Null if the the WixPdb should not be output.</value>
        public string PdbFile
        {
            get { return this.pdbFile; }
            set { this.pdbFile = value; }
        }

        /// <summary>
        /// Gets and sets the option to set the file version in the MsiAssemblyName table.
        /// </summary>
        /// <value>The option to set the file version in the MsiAssemblyName table.</value>
        public bool SetMsiAssemblyNameFileVersion
        {
            get { return this.setMsiAssemblyNameFileVersion; }
            set { this.setMsiAssemblyNameFileVersion = value; }
        }

        /// <summary>
        /// Gets and sets the option to suppress resetting ACLs by the binder.
        /// </summary>
        /// <value>The option to suppress resetting ACLs by the binder.</value>
        public bool SuppressAclReset
        {
            get { return this.suppressAclReset; }
            set { this.suppressAclReset = value; }
        }

        /// <summary>
        /// Gets and sets the option to suppress adding _Validation rows.
        /// </summary>
        /// <value>The option to suppress adding _Validation rows.</value>
        public bool SuppressAddingValidationRows
        {
            get { return this.suppressAddingValidationRows; }
            set { this.suppressAddingValidationRows = value; }
        }

        /// <summary>
        /// Gets and sets the option to suppress grabbing assembly name information from assemblies.
        /// </summary>
        /// <value>The option to suppress grabbing assembly name information from assemblies.</value>
        public bool SuppressAssemblies
        {
            get { return this.suppressAssemblies; }
            set { this.suppressAssemblies = value; }
        }

        /// <summary>
        /// Gets and sets the option to suppress grabbing the file hash, version and language at link time.
        /// </summary>
        /// <value>The option to suppress grabbing the file hash, version and language.</value>
        public bool SuppressFileHashAndInfo
        {
            get { return this.suppressFileHashAndInfo; }
            set { this.suppressFileHashAndInfo = value; }
        }

        /// <summary>
        /// Gets and sets the option to suppress creating an image for MSI/MSM.
        /// </summary>
        /// <value>The option to suppress creating an image for MSI/MSM.</value>
        public bool SuppressLayout
        {
            get { return this.suppressLayout; }
            set { this.suppressLayout = value; }
        }

        /// <summary>
        /// Gets and sets the option to suppress MSI/MSM validation.
        /// </summary>
        /// <value>The option to suppress MSI/MSM validation.</value>
        /// <remarks>This must be set before calling Bind.</remarks>
        public bool SuppressValidation
        {
            get
            {
                return this.suppressValidation;
            }

            set
            {
                // make a new validator if validation has been turned off and is now being turned back on
                if (!value && null == this.validator)
                {
                    this.validator = new Validator();
                }

                this.suppressValidation = value;
            }
        }

        /// <summary>
        /// Gets help for all the command line arguments for this binder.
        /// </summary>
        /// <returns>A string to be added to light's help string.</returns>
        public override string GetCommandLineArgumentsHelpString()
        {
            return WixStrings.BinderArguments;
        }

        /// <summary>
        /// Parse the commandline arguments.
        /// </summary>
        /// <param name="args">Commandline arguments.</param>
        /// <param name="consoleMessageHandler">The console message handler.</param>
        [SuppressMessage("Microsoft.Globalization", "CA1308:NormalizeStringsToUppercase", Justification = "These strings are not round tripped, and have no security impact")]
        public override StringCollection ParseCommandLine(string[] args, ConsoleMessageHandler consoleMessageHandler)
        {
            for (int i = 0; i < args.Length; ++i)
            {
                string arg = args[i];
                if (null == arg || 0 == arg.Length) // skip blank arguments
                {
                    continue;
                }

                if ('-' == arg[0] || '/' == arg[0])
                {
                    string parameter = arg.Substring(1);
                    if (parameter.Equals("bcgg", StringComparison.Ordinal))
                    {
                        this.backwardsCompatibleGuidGen = true;
                    }
                    else if (parameter.Equals("cc", StringComparison.Ordinal))
                    {
                        this.cabCachePath = CommandLine.GetDirectory(parameter, consoleMessageHandler, args, ++i);

                        if (String.IsNullOrEmpty(this.cabCachePath))
                        {
                            return this.invalidArgs;
                        }
                    }
                    else if (parameter.Equals("ct", StringComparison.Ordinal))
                    {
                        if (!CommandLine.IsValidArg(args, ++i))
                        {
                            consoleMessageHandler.Display(this, WixErrors.IllegalCabbingThreadCount(String.Empty));
                            return this.invalidArgs;
                        }

                        try
                        {
                            this.cabbingThreadCount = Convert.ToInt32(args[i], CultureInfo.InvariantCulture.NumberFormat);

                            if (0 >= this.cabbingThreadCount)
                            {
                                consoleMessageHandler.Display(this, WixErrors.IllegalCabbingThreadCount(args[i]));
                            }

                            consoleMessageHandler.Display(this, WixVerboses.SetCabbingThreadCount(this.cabbingThreadCount.ToString()));
                        }
                        catch (FormatException)
                        {
                            consoleMessageHandler.Display(this, WixErrors.IllegalCabbingThreadCount(args[i]));
                        }
                        catch (OverflowException)
                        {
                            consoleMessageHandler.Display(this, WixErrors.IllegalCabbingThreadCount(args[i]));
                        }
                    }
                    else if (parameter.Equals("cub", StringComparison.Ordinal))
                    {
                        string cubeFile = CommandLine.GetFile(parameter, consoleMessageHandler, args, ++i);

                        if (String.IsNullOrEmpty(cubeFile))
                        {
                            return this.invalidArgs;
                        }

                        this.validator.AddCubeFile(cubeFile);
                    }
                    else if (parameter.StartsWith("dcl:", StringComparison.Ordinal))
                    {
                        string defaultCompressionLevel = arg.Substring(5).ToLower(CultureInfo.InvariantCulture);

                        if (!String.IsNullOrEmpty(defaultCompressionLevel))
                        {
                            switch (defaultCompressionLevel)
                            {
                                case "low":
                                    this.defaultCompressionLevel = Cab.CompressionLevel.Low;
                                    break;
                                case "medium":
                                    this.defaultCompressionLevel = Cab.CompressionLevel.Medium;
                                    break;
                                case "high":
                                    this.defaultCompressionLevel = Cab.CompressionLevel.High;
                                    break;
                                case "none":
                                    this.defaultCompressionLevel = Cab.CompressionLevel.None;
                                    break;
                                case "mszip":
                                    this.defaultCompressionLevel = Cab.CompressionLevel.Mszip;
                                    break;
                                default:
                                    throw new WixException(WixErrors.IllegalCompressionLevel(defaultCompressionLevel));
                            }
                        }
                    }
                    else if (parameter.Equals("eav", StringComparison.Ordinal))
                    {
                        this.exactAssemblyVersions = true;
                    }
                    else if (parameter.Equals("fv", StringComparison.Ordinal))
                    {
                        this.setMsiAssemblyNameFileVersion = true;
                    }
                    else if (parameter.StartsWith("ice:", StringComparison.Ordinal))
                    {
                        this.ices.Add(parameter.Substring(4));
                    }
                    else if (parameter.Equals("O1", StringComparison.Ordinal))
                    {
                        consoleMessageHandler.Display(this, WixWarnings.DeprecatedCommandLineSwitch("O1"));
                    }
                    else if (parameter.Equals("O2", StringComparison.Ordinal))
                    {
                        consoleMessageHandler.Display(this, WixWarnings.DeprecatedCommandLineSwitch("O2"));
                    }
                    else if (parameter.Equals("pdbout", StringComparison.Ordinal))
                    {
                        this.pdbFile = CommandLine.GetFile(parameter, consoleMessageHandler, args, ++i);

                        if (String.IsNullOrEmpty(this.pdbFile))
                        {
                            return this.invalidArgs;
                        }
                    }
                    else if (parameter.Equals("reusecab", StringComparison.Ordinal))
                    {
                        this.reuseCabinets = true;
                    }
                    else if (parameter.Equals("sa", StringComparison.Ordinal))
                    {
                        this.suppressAssemblies = true;
                    }
                    else if (parameter.Equals("sacl", StringComparison.Ordinal))
                    {
                        this.suppressAclReset = true;
                    }
                    else if (parameter.Equals("sf", StringComparison.Ordinal))
                    {
                        this.suppressAssemblies = true;
                        this.suppressFileHashAndInfo = true;
                    }
                    else if (parameter.Equals("sh", StringComparison.Ordinal))
                    {
                        this.suppressFileHashAndInfo = true;
                    }
                    else if (parameter.StartsWith("sice:", StringComparison.Ordinal))
                    {
                        this.suppressICEs.Add(parameter.Substring(5));
                    }
                    else if (parameter.Equals("sl", StringComparison.Ordinal))
                    {
                        this.suppressLayout = true;
                    }
                    else if (parameter.Equals("spdb", StringComparison.Ordinal))
                    {
                        this.suppressWixPdb = true;
                    }
                    else if (parameter.Equals("sval", StringComparison.Ordinal))
                    {
                        this.suppressValidation = true;
                    }
                    else
                    {
                        this.invalidArgs.Add(arg);
                    }
                }
                else
                {
                    this.invalidArgs.Add(arg);
                }
            }

            this.pdbFile = this.suppressWixPdb ? null : this.pdbFile;

            return this.invalidArgs;
        }

        /// <summary>
        /// Do any setting up needed after all command line parsing.
        /// </summary>
        public override void PostParseCommandLine()
        {
            if (!this.suppressWixPdb && null == this.pdbFile && null != this.OutputFile)
            {
                this.pdbFile = Path.ChangeExtension(this.OutputFile, ".wixpdb");
            }
        }

        /// <summary>
        /// Adds an event handler.
        /// </summary>
        /// <param name="newHandler">The event handler to add.</param>
        public override void AddMessageEventHandler(MessageEventHandler newHandler)
        {
            base.AddMessageEventHandler(newHandler);
            validator.Extension.Message += newHandler;
        }

        /// <summary>
        /// Binds an output.
        /// </summary>
        /// <param name="output">The output to bind.</param>
        /// <param name="file">The Windows Installer file to create.</param>
        /// <remarks>The Binder.DeleteTempFiles method should be called after calling this method.</remarks>
        /// <returns>true if binding completed successfully; false otherwise</returns>
        public override bool Bind(Output output, string file)
        {
            // ensure the cabinet cache path exists if we are going to use it
            if (null != this.cabCachePath && !Directory.Exists(this.cabCachePath))
            {
                Directory.CreateDirectory(this.cabCachePath);
            }

            // tell the binder about the validator if validation isn't suppressed
            if (!this.suppressValidation && (OutputType.Module == output.Type || OutputType.Product == output.Type))
            {
                if (String.IsNullOrEmpty(this.validator.TempFilesLocation))
                {
                    this.validator.TempFilesLocation = Environment.GetEnvironmentVariable("WIX_TEMP");
                }

                // set the default cube file
                Assembly lightAssembly = Assembly.GetExecutingAssembly();
                string lightDirectory = Path.GetDirectoryName(lightAssembly.Location);
                if (OutputType.Module == output.Type)
                {
                    this.validator.AddCubeFile(Path.Combine(lightDirectory, "mergemod.cub"));
                }
                else // product
                {
                    this.validator.AddCubeFile(Path.Combine(lightDirectory, "darice.cub"));
                }

                // disable ICE33, ICE47 and ICE66 by default
                this.suppressICEs.Add("ICE33");
                this.suppressICEs.Add("ICE47");
                this.suppressICEs.Add("ICE66");

                // set the ICEs
                string[] iceArray = new string[this.ices.Count];
                this.ices.CopyTo(iceArray, 0);
                this.validator.ICEs = iceArray;

                // set the suppressed ICEs
                string[] suppressICEArray = new string[this.suppressICEs.Count];
                this.suppressICEs.CopyTo(suppressICEArray, 0);
                this.validator.SuppressedICEs = suppressICEArray;
            }
            else
            {
                this.validator = null;
            }

            this.core = new BinderCore(this.MessageHandler);

            foreach (BinderExtension extension in this.extensions)
            {
                extension.Core = this.core;
            }

            if (null == output)
            {
                throw new ArgumentNullException("output");
            }

            this.core.EncounteredError = false;

            switch (output.Type)
            {
                case OutputType.Bundle:
                    return this.BindBundle(output, file);
                case OutputType.Transform:
                    return this.BindTransform(output, file);
                default:
                    return this.BindDatabase(output, file);
            }
        }

        /// <summary>
        /// Does any housekeeping after Bind.
        /// </summary>
        /// <param name="tidy">Whether or not any actual tidying should be done.</param>
        public override void Cleanup(bool tidy)
        {
            // If Bind hasn't been called yet, core will be null. There will be
            // nothing to cleanup.
            if (this.core == null)
            {
                return;
            }

            if (tidy)
            {
                if (!this.DeleteTempFiles())
                {
                    this.core.OnMessage(WixWarnings.FailedToDeleteTempDir(this.TempFilesLocation));
                }
            }
            else
            {
                this.core.OnMessage(WixVerboses.BinderTempDirLocatedAt(this.TempFilesLocation));
            }

            if (null != this.validator && !String.IsNullOrEmpty(this.validator.TempFilesLocation))
            {
                if (tidy)
                {
                    if (!this.validator.DeleteTempFiles())
                    {
                        this.core.OnMessage(WixWarnings.FailedToDeleteTempDir(this.validator.TempFilesLocation));
                    }
                }
                else
                {
                    this.core.OnMessage(WixVerboses.ValidatorTempDirLocatedAt(this.validator.TempFilesLocation));
                }
            }
        }

        /// <summary>
        /// Cleans up the temp files used by the Binder.
        /// </summary>
        /// <returns>True if all files were deleted, false otherwise.</returns>
        public override bool DeleteTempFiles()
        {
            bool deleted = base.DeleteTempFiles();
            if (deleted)
            {
                this.emptyFile = null;
            }

            return deleted;
        }

        /// <summary>
        /// Process a list of loaded extensions.
        /// </summary>
        /// <param name="loadedExtensionList">The list of loaded extensions.</param>
        public override void ProcessExtensions(WixExtension[] loadedExtensionList)
        {
            bool binderFileManagerLoaded = false;
            bool validatorExtensionLoaded = false;

            foreach (WixExtension wixExtension in loadedExtensionList)
            {
                if (null != wixExtension.BinderFileManager)
                {
                    if (binderFileManagerLoaded)
                    {
                        core.OnMessage(WixErrors.CannotLoadBinderFileManager(wixExtension.BinderFileManager.GetType().ToString(), this.FileManager.ToString()));
                    }

                    this.FileManager = wixExtension.BinderFileManager;
                    binderFileManagerLoaded = true;
                }

                ValidatorExtension validatorExtension = wixExtension.ValidatorExtension;
                if (null != validatorExtension)
                {
                    if (validatorExtensionLoaded)
                    {
                        core.OnMessage(WixErrors.CannotLoadLinkerExtension(validatorExtension.GetType().ToString(), this.validator.Extension.ToString()));
                    }

                    this.validator.Extension = validatorExtension;
                    validatorExtensionLoaded = true;
                }
            }

            this.FileManager.CabCachePath = this.cabCachePath;
            this.FileManager.ReuseCabinets = this.reuseCabinets;
        }

        /// <summary>
        /// Creates the MSI/MSM/PCP database.
        /// </summary>
        /// <param name="output">Output to create database for.</param>
        /// <param name="databaseFile">The database file to create.</param>
        /// <param name="keepAddedColumns">Whether to keep columns added in a transform.</param>
        /// <param name="useSubdirectory">Whether to use a subdirectory based on the <paramref name="databaseFile"/> file name for intermediate files.</param>
        internal void GenerateDatabase(Output output, string databaseFile, bool keepAddedColumns, bool useSubdirectory)
        {
            // add the _Validation rows
            if (!this.suppressAddingValidationRows)
            {
                Table validationTable = output.EnsureTable(this.core.TableDefinitions["_Validation"]);

                foreach (Table table in output.Tables)
                {
                    if (!table.Definition.IsUnreal)
                    {
                        // add the validation rows for this table
                        table.Definition.AddValidationRows(validationTable);
                    }
                }
            }

            // set the base directory
            string baseDirectory = this.TempFilesLocation;
            if (useSubdirectory)
            {
                string filename = Path.GetFileNameWithoutExtension(databaseFile);
                baseDirectory = Path.Combine(baseDirectory, filename);

                // make sure the directory exists
                Directory.CreateDirectory(baseDirectory);
            }

            try
            {
                OpenDatabase type = OpenDatabase.CreateDirect;

                // set special flag for patch files
                if (OutputType.Patch == output.Type)
                {
                    type |= OpenDatabase.OpenPatchFile;
                }

                // try to create the database
                using (Database db = new Database(databaseFile, type))
                {
                    // localize the codepage if a value was specified by the localizer
                    if (null != this.Localizer && -1 != this.Localizer.Codepage)
                    {
                        output.Codepage = this.Localizer.Codepage;
                    }

                    // if we're not using the default codepage, import a new one into our
                    // database before we add any tables (or the tables would be added
                    // with the wrong codepage)
                    if (0 != output.Codepage)
                    {
                        this.SetDatabaseCodepage(db, output);
                    }

                    foreach (Table table in output.Tables)
                    {
                        Table importTable = table;
                        bool hasBinaryColumn = false;

                        // skip all unreal tables other than _Streams
                        if (table.Definition.IsUnreal && "_Streams" != table.Name)
                        {
                            continue;
                        }

                        // Do not put the _Validation table in patches, it is not needed
                        if (OutputType.Patch == output.Type && "_Validation" == table.Name)
                        {
                            continue;
                        }

                        // The only way to import binary data is to copy it to a local subdirectory first.
                        // To avoid this extra copying and perf hit, import an empty table with the same
                        // definition and later import the binary data from source using records.
                        foreach (ColumnDefinition columnDefinition in table.Definition.Columns)
                        {
                            if (ColumnType.Object == columnDefinition.Type)
                            {
                                importTable = new Table(table.Section, table.Definition);
                                hasBinaryColumn = true;
                                break;
                            }
                        }

                        // create the table via IDT import
                        if ("_Streams" != importTable.Name)
                        {
                            try
                            {
                                db.ImportTable(output.Codepage, this.core, importTable, baseDirectory, keepAddedColumns);
                            }
                            catch (WixInvalidIdtException)
                            {
                                // If ValidateRows finds anything it doesn't like, it throws
                                importTable.ValidateRows();

                                // Otherwise we rethrow the InvalidIdt
                                throw;
                            }
                        }

                        // insert the rows via SQL query if this table contains object fields
                        if (hasBinaryColumn)
                        {
                            StringBuilder query = new StringBuilder("SELECT ");

                            // build the query for the view
                            bool firstColumn = true;
                            foreach (ColumnDefinition columnDefinition in table.Definition.Columns)
                            {
                                if (!firstColumn)
                                {
                                    query.Append(",");
                                }
                                query.AppendFormat(" `{0}`", columnDefinition.Name);
                                firstColumn = false;
                            }
                            query.AppendFormat(" FROM `{0}`", table.Name);

                            using (View tableView = db.OpenExecuteView(query.ToString()))
                            {
                                // import each row containing a stream
                                foreach (Row row in table.Rows)
                                {
                                    using (Record record = new Record(table.Definition.Columns.Count))
                                    {
                                        StringBuilder streamName = new StringBuilder();

                                        // the _Streams table doesn't prepend the table name (or a period)
                                        if ("_Streams" != table.Name)
                                        {
                                            streamName.Append(table.Name);
                                        }

                                        for (int i = 0; i < table.Definition.Columns.Count; i++)
                                        {
                                            ColumnDefinition columnDefinition = table.Definition.Columns[i];

                                            switch (columnDefinition.Type)
                                            {
                                                case ColumnType.Localized:
                                                case ColumnType.Preserved:
                                                case ColumnType.String:
                                                    if (columnDefinition.IsPrimaryKey)
                                                    {
                                                        if (0 < streamName.Length)
                                                        {
                                                            streamName.Append(".");
                                                        }
                                                        streamName.Append((string)row[i]);
                                                    }

                                                    record.SetString(i + 1, (string)row[i]);
                                                    break;
                                                case ColumnType.Number:
                                                    record.SetInteger(i + 1, Convert.ToInt32(row[i], CultureInfo.InvariantCulture));
                                                    break;
                                                case ColumnType.Object:
                                                    if (null != row[i])
                                                    {
                                                        try
                                                        {
                                                            record.SetStream(i + 1, (string)row[i]);
                                                        }
                                                        catch (Win32Exception e)
                                                        {
                                                            if (0xA1 == e.NativeErrorCode) // ERROR_BAD_PATHNAME
                                                            {
                                                                throw new WixException(WixErrors.FileNotFound(row.SourceLineNumbers, (string)row[i]));
                                                            }
                                                            else
                                                            {
                                                                throw new WixException(WixErrors.Win32Exception(e.NativeErrorCode, e.Message));
                                                            }
                                                        }
                                                    }
                                                    break;
                                            }
                                        }

                                        // stream names are created by concatenating the name of the table with the values
                                        // of the primary key (delimited by periods)
                                        // check for a stream name that is more than 62 characters long (the maximum allowed length)
                                        if (MsiInterop.MsiMaxStreamNameLength < streamName.Length)
                                        {
                                            this.core.OnMessage(WixErrors.StreamNameTooLong(row.SourceLineNumbers, table.Name, streamName.ToString(), streamName.Length));
                                        }
                                        else // add the row to the database
                                        {
                                            tableView.Modify(ModifyView.Assign, record);
                                        }
                                    }
                                }
                            }

                            // Remove rows from the _Streams table for wixpdbs.
                            if ("_Streams" == table.Name)
                            {
                                table.Rows.Clear();
                            }
                        }
                    }

                    // insert substorages (like transforms inside a patch)
                    if (0 < output.SubStorages.Count)
                    {
                        using (View storagesView = new View(db, "SELECT `Name`, `Data` FROM `_Storages`"))
                        {
                            foreach (SubStorage subStorage in output.SubStorages)
                            {
                                string transformFile = Path.Combine(this.TempFilesLocation, String.Concat(subStorage.Name, ".mst"));

                                // bind the transform
                                if (this.BindTransform(subStorage.Data, transformFile))
                                {
                                    // add the storage
                                    using (Record record = new Record(2))
                                    {
                                        record.SetString(1, subStorage.Name);
                                        record.SetStream(2, transformFile);
                                        storagesView.Modify(ModifyView.Assign, record);
                                    }
                                }
                            }
                        }
                    }

                    // we're good, commit the changes to the new MSI
                    db.Commit();
                }
            }
            catch (IOException)
            {
                // TODO: this error message doesn't seem specific enough
                throw new WixFileNotFoundException(SourceLineNumberCollection.FromFileName(databaseFile), databaseFile);
            }
        }

        /// <summary>
        /// Get the source path of a directory.
        /// </summary>
        /// <param name="directories">All cached directories.</param>
        /// <param name="componentIdGenSeeds">Hash table of Component GUID generation seeds indexed by directory id.</param>
        /// <param name="directory">Directory identifier.</param>
        /// <param name="canonicalize">Canonicalize the path for standard directories.</param>
        /// <returns>Source path of a directory.</returns>
        [SuppressMessage("Microsoft.Globalization", "CA1308:NormalizeStringsToUppercase", Justification = "Changing the way this string normalizes would result " +
                         "in a change to the way autogenerated GUIDs are generated. Furthermore, there is no security hole here, as the strings won't need to " +
                         "make a round trip")]
        private static string GetDirectoryPath(Hashtable directories, Hashtable componentIdGenSeeds, string directory, bool canonicalize)
        {
            if (!directories.Contains(directory))
            {
                throw new WixException(WixErrors.ExpectedDirectory(directory));
            }
            ResolvedDirectory resolvedDirectory = (ResolvedDirectory)directories[directory];

            if (null == resolvedDirectory.Path)
            {
                if (null != componentIdGenSeeds && componentIdGenSeeds.Contains(directory))
                {
                    resolvedDirectory.Path = (string)componentIdGenSeeds[directory];
                }
                else if (canonicalize && Util.IsStandardDirectory(directory))
                {
                    // when canonicalization is on, standard directories are treated equally
                    resolvedDirectory.Path = directory;
                }
                else
                {
                    string name = resolvedDirectory.Name;

                    if (canonicalize && null != name)
                    {
                        name = name.ToLower(CultureInfo.InvariantCulture);
                    }

                    if (null == resolvedDirectory.DirectoryParent)
                    {
                        resolvedDirectory.Path = name;
                    }
                    else
                    {
                        string parentPath = GetDirectoryPath(directories, componentIdGenSeeds, resolvedDirectory.DirectoryParent, canonicalize);

                        if (null != resolvedDirectory.Name)
                        {
                            resolvedDirectory.Path = Path.Combine(parentPath, name);
                        }
                        else
                        {
                            resolvedDirectory.Path = parentPath;
                        }
                    }
                }
            }

            return resolvedDirectory.Path;
        }

        /// <summary>
        /// Set an MsiAssemblyName row.  If it was directly authored, override the value, otherwise
        /// create a new row.
        /// </summary>
        /// <param name="output">The output to bind.</param>
        /// <param name="assemblyNameTable">MsiAssemblyName table.</param>
        /// <param name="fileRow">FileRow containing the assembly read for the MsiAssemblyName row.</param>
        /// <param name="name">MsiAssemblyName name.</param>
        /// <param name="value">MsiAssemblyName value.</param>
        /// <param name="infoCache">Cache to populate with file information (optional).</param>
        /// <param name="modularizationGuid">The modularization GUID (in the case of merge modules).</param>
        [SuppressMessage("Microsoft.Globalization", "CA1308:NormalizeStringsToUppercase", Justification = "This string is not round tripped, and not used for any security decisions")]
        private void SetMsiAssemblyName(Output output, Table assemblyNameTable, FileRow fileRow, string name, string value, IDictionary<string, string> infoCache, string modularizationGuid)
        {
            // check for null value (this can occur when grabbing the file version from an assembly without one)
            if (null == value || 0 == value.Length)
            {
                this.core.OnMessage(WixWarnings.NullMsiAssemblyNameValue(fileRow.SourceLineNumbers, fileRow.Component, name));
            }
            else
            {
                Row assemblyNameRow = null;

                // override directly authored value
                foreach (Row row in assemblyNameTable.Rows)
                {
                    if ((string)row[0] == fileRow.Component && (string)row[1] == name)
                    {
                        assemblyNameRow = row;
                        break;
                    }
                }

                // if the assembly will be GAC'd and the name in the file table doesn't match the name in the MsiAssemblyName table, error because the install will fail.
                if ("name" == name && FileAssemblyType.DotNetAssembly == fileRow.AssemblyType && String.IsNullOrEmpty(fileRow.AssemblyApplication) && !String.Equals(Path.GetFileNameWithoutExtension(fileRow.LongFileName), value, StringComparison.OrdinalIgnoreCase))
                {
                    this.core.OnMessage(WixErrors.GACAssemblyIdentityWarning(fileRow.SourceLineNumbers, Path.GetFileNameWithoutExtension(fileRow.LongFileName), value));
                }

                if (null == assemblyNameRow)
                {
                    assemblyNameRow = assemblyNameTable.CreateRow(fileRow.SourceLineNumbers);
                    assemblyNameRow[0] = fileRow.Component;
                    assemblyNameRow[1] = name;
                    assemblyNameRow[2] = value;

                    // put the MsiAssemblyName row in the same section as the related File row
                    assemblyNameRow.SectionId = fileRow.SectionId;

                    if (null == fileRow.AssemblyNameRows)
                    {
                        fileRow.AssemblyNameRows = new RowCollection();
                    }
                    fileRow.AssemblyNameRows.Add(assemblyNameRow);
                }
                else
                {
                    assemblyNameRow[2] = value;
                }

                if (infoCache != null)
                {
                    string key = String.Format(CultureInfo.InvariantCulture, "assembly{0}.{1}", name, Demodularize(output, modularizationGuid, fileRow.File)).ToLower(CultureInfo.InvariantCulture);
                    infoCache[key] = (string)assemblyNameRow[2];
                }
            }
        }

        /// <summary>
        /// Merge data from a row in the WixPatchSymbolsPaths table into an associated FileRow.
        /// </summary>
        /// <param name="row">Row from the WixPatchSymbolsPaths table.</param>
        /// <param name="fileRow">FileRow into which to set symbol information.</param>
        /// <comment>This includes PreviousData as well.</comment>
        private static void MergeSymbolPaths(Row row, FileRow fileRow)
        {
            if (null == fileRow.Symbols)
            {
                fileRow.Symbols = (string)row[2];
            }
            else
            {
                fileRow.Symbols = String.Concat(fileRow.Symbols, ";", (string)row[2]);
            }

            Field field = row.Fields[2];
            if (null != field.PreviousData)
            {
                if (null == fileRow.PreviousSymbols)
                {
                    fileRow.PreviousSymbols = field.PreviousData;
                }
                else
                {
                    fileRow.PreviousSymbols = String.Concat(fileRow.PreviousSymbols, ";", field.PreviousData);
                }
            }
        }

        /// <summary>
        /// Merge data from the unreal tables into the real tables.
        /// </summary>
        /// <param name="tables">Collection of all tables.</param>
        private void MergeUnrealTables(TableCollection tables)
        {
            // merge data from the WixBBControl rows into the BBControl rows
            Table wixBBControlTable = tables["WixBBControl"];
            Table bbControlTable = tables["BBControl"];
            if (null != wixBBControlTable && null != bbControlTable)
            {
                // index all the BBControl rows by their identifier
                Hashtable indexedBBControlRows = new Hashtable(bbControlTable.Rows.Count);
                foreach (BBControlRow bbControlRow in bbControlTable.Rows)
                {
                    indexedBBControlRows.Add(bbControlRow.GetPrimaryKey('/'), bbControlRow);
                }

                foreach (Row row in wixBBControlTable.Rows)
                {
                    BBControlRow bbControlRow = (BBControlRow)indexedBBControlRows[row.GetPrimaryKey('/')];

                    bbControlRow.SourceFile = (string)row[2];
                }
            }

            // merge data from the WixControl rows into the Control rows
            Table wixControlTable = tables["WixControl"];
            Table controlTable = tables["Control"];
            if (null != wixControlTable && null != controlTable)
            {
                // index all the Control rows by their identifier
                Hashtable indexedControlRows = new Hashtable(controlTable.Rows.Count);
                foreach (ControlRow controlRow in controlTable.Rows)
                {
                    indexedControlRows.Add(controlRow.GetPrimaryKey('/'), controlRow);
                }

                foreach (Row row in wixControlTable.Rows)
                {
                    ControlRow controlRow = (ControlRow)indexedControlRows[row.GetPrimaryKey('/')];

                    controlRow.SourceFile = (string)row[2];
                }
            }

            // merge data from the WixFile rows into the File rows
            Table wixFileTable = tables["WixFile"];
            Table fileTable = tables["File"];
            if (null != wixFileTable && null != fileTable)
            {
                // index all the File rows by their identifier
                Hashtable indexedFileRows = new Hashtable(fileTable.Rows.Count, StringComparer.OrdinalIgnoreCase);

                foreach (FileRow fileRow in fileTable.Rows)
                {
                    try
                    {
                        indexedFileRows.Add(fileRow.File, fileRow);
                    }
                    catch (ArgumentException)
                    {
                        this.core.OnMessage(WixErrors.DuplicateFileId(fileRow.File));
                    }
                }

                if (this.core.EncounteredError)
                {
                    return;
                }

                foreach (WixFileRow row in wixFileTable.Rows)
                {
                    FileRow fileRow = (FileRow)indexedFileRows[row.File];

                    if (null != row[1])
                    {
                        fileRow.AssemblyType = (FileAssemblyType)Enum.ToObject(typeof(FileAssemblyType), row.AssemblyAttributes);
                    }
                    else
                    {
                        fileRow.AssemblyType = FileAssemblyType.NotAnAssembly;
                    }
                    fileRow.AssemblyApplication = row.AssemblyApplication;
                    fileRow.AssemblyManifest = row.AssemblyManifest;
                    fileRow.Directory = row.Directory;
                    fileRow.DiskId = row.DiskId;
                    fileRow.Source = row.Source;
                    fileRow.PreviousSource = row.PreviousSource;
                    fileRow.ProcessorArchitecture = row.ProcessorArchitecture;
                    fileRow.PatchGroup = row.PatchGroup;
                    fileRow.PatchAttributes = row.PatchAttributes;
                    fileRow.RetainLengths = row.RetainLengths;
                    fileRow.IgnoreOffsets = row.IgnoreOffsets;
                    fileRow.IgnoreLengths = row.IgnoreLengths;
                    fileRow.RetainOffsets = row.RetainOffsets;
                    fileRow.PreviousRetainLengths = row.PreviousRetainLengths;
                    fileRow.PreviousIgnoreOffsets = row.PreviousIgnoreOffsets;
                    fileRow.PreviousIgnoreLengths = row.PreviousIgnoreLengths;
                    fileRow.PreviousRetainOffsets = row.PreviousRetainOffsets;
                }
            }

            // merge data from the WixPatchSymbolPaths rows into the File rows
            Table wixPatchSymbolPathsTable = tables["WixPatchSymbolPaths"];
            Table mediaTable = tables["Media"];
            Table directoryTable = tables["Directory"];
            Table componentTable = tables["Component"];
            if (null != wixPatchSymbolPathsTable)
            {
                int fileRowNum = (null != fileTable) ? fileTable.Rows.Count : 0;
                int componentRowNum = (null != fileTable) ? componentTable.Rows.Count : 0;
                int directoryRowNum = (null != directoryTable) ? directoryTable.Rows.Count : 0;
                int mediaRowNum = (null != mediaTable) ? mediaTable.Rows.Count : 0;

                Hashtable fileRowsByFile = new Hashtable(fileRowNum);
                Hashtable fileRowsByComponent = new Hashtable(componentRowNum);
                Hashtable fileRowsByDirectory = new Hashtable(directoryRowNum);
                Hashtable fileRowsByDiskId = new Hashtable(mediaRowNum);

                // index all the File rows by their identifier
                if (null != fileTable)
                {
                    foreach (FileRow fileRow in fileTable.Rows)
                    {
                        fileRowsByFile.Add(fileRow.File, fileRow);

                        ArrayList fileRows = (ArrayList)fileRowsByComponent[fileRow.Component];
                        if (null == fileRows)
                        {
                            fileRows = new ArrayList();
                            fileRowsByComponent.Add(fileRow.Component, fileRows);
                        }
                        fileRows.Add(fileRow);

                        fileRows = (ArrayList)fileRowsByDirectory[fileRow.Directory];
                        if (null == fileRows)
                        {
                            fileRows = new ArrayList();
                            fileRowsByDirectory.Add(fileRow.Directory, fileRows);
                        }
                        fileRows.Add(fileRow);

                        fileRows = (ArrayList)fileRowsByDiskId[fileRow.DiskId];
                        if (null == fileRows)
                        {
                            fileRows = new ArrayList();
                            fileRowsByDiskId.Add(fileRow.DiskId, fileRows);
                        }
                        fileRows.Add(fileRow);
                    }
                }

                wixPatchSymbolPathsTable.Rows.Sort(new WixPatchSymbolPathsComparer());

                foreach (Row row in wixPatchSymbolPathsTable.Rows)
                {
                    switch ((string)row[0])
                    {
                        case "File":
                            MergeSymbolPaths(row, (FileRow)fileRowsByFile[row[1]]);
                            break;

                        case "Product":
                            foreach (FileRow fileRow in fileRowsByFile)
                            {
                                MergeSymbolPaths(row, fileRow);
                            }
                            break;

                        case "Component":
                            ArrayList fileRowsByThisComponent = (ArrayList)(fileRowsByComponent[row[1]]);
                            if (null != fileRowsByThisComponent)
                            {
                                foreach (FileRow fileRow in fileRowsByThisComponent)
                                {
                                    MergeSymbolPaths(row, fileRow);
                                }
                            }

                            break;

                        case "Directory":
                            ArrayList fileRowsByThisDirectory = (ArrayList)(fileRowsByDirectory[row[1]]);
                            if (null != fileRowsByThisDirectory)
                            {
                                foreach (FileRow fileRow in fileRowsByThisDirectory)
                                {
                                    MergeSymbolPaths(row, fileRow);
                                }
                            }
                            break;

                        case "Media":
                            ArrayList fileRowsByThisDiskId = (ArrayList)(fileRowsByDiskId[row[1]]);
                            if (null != fileRowsByThisDiskId)
                            {
                                foreach (FileRow fileRow in fileRowsByThisDiskId)
                                {
                                    MergeSymbolPaths(row, fileRow);
                                }
                            }
                            break;

                        default:
                            // error
                            break;
                    }
                }
            }

            // copy data from the WixMedia rows into the Media rows
            Table wixMediaTable = tables["WixMedia"];
            if (null != wixMediaTable && null != mediaTable)
            {
                // index all the Media rows by their identifier
                Hashtable indexedMediaRows = new Hashtable(mediaTable.Rows.Count);
                foreach (MediaRow mediaRow in mediaTable.Rows)
                {
                    indexedMediaRows.Add(mediaRow.DiskId, mediaRow);
                }

                foreach (Row row in wixMediaTable.Rows)
                {
                    MediaRow mediaRow = (MediaRow)indexedMediaRows[row[0]];

                    // compression level
                    if (null != row[1])
                    {
                        switch ((string)row[1])
                        {
                            case "low":
                                mediaRow.CompressionLevel = Cab.CompressionLevel.Low;
                                break;
                            case "medium":
                                mediaRow.CompressionLevel = Cab.CompressionLevel.Medium;
                                break;
                            case "high":
                                mediaRow.CompressionLevel = Cab.CompressionLevel.High;
                                break;
                            case "none":
                                mediaRow.CompressionLevel = Cab.CompressionLevel.None;
                                break;
                            case "mszip":
                                mediaRow.CompressionLevel = Cab.CompressionLevel.Mszip;
                                break;
                        }
                    }

                    // layout
                    if (null != row[2])
                    {
                        mediaRow.Layout = (string)row[2];
                    }
                }
            }
        }

        /// <summary>
        /// Signal a warning if a non-keypath file was changed in a patch without also changing the keypath file of the component.
        /// </summary>
        /// <param name="output">The output to validate.</param>
        private void ValidateFileRowChanges(Output transform)
        {
            Table componentTable = transform.Tables["Component"];
            Table fileTable = transform.Tables["File"];

            // There's no sense validating keypaths if the transform has no component or file table
            if (componentTable == null || fileTable == null)
            {
                return;
            }

            Dictionary<string, string> componentKeyPath = new Dictionary<string, string>();
            // Index Component table
            foreach (Row row in componentTable.Rows)
            {
                string keypath = (null == row.Fields[5].Data) ? String.Empty : row.Fields[5].Data.ToString();
                componentKeyPath.Add(row.Fields[0].Data.ToString(), keypath);
            }

            Dictionary<string, string> componentWithChangedKeyPath = new Dictionary<string, string>();
            Dictionary<string, string> componentWithNonKeyPathChanged = new Dictionary<string, string>();
            // Verify changes in the file table, now that file diffing has occurred
            foreach (FileRow row in fileTable.Rows)
            {
                string fileId = row.Fields[0].Data.ToString();
                string componentId = row.Fields[1].Data.ToString();

                if (RowOperation.Modify != row.Operation)
                {
                    continue;
                }

                // If this file is the keypath of a component
                if (componentKeyPath.ContainsValue(fileId))
                {
                    if (!componentWithChangedKeyPath.ContainsKey(componentId))
                    {
                        componentWithChangedKeyPath.Add(componentId, fileId);
                    }
                }
                else
                {
                    if (!componentWithNonKeyPathChanged.ContainsKey(componentId))
                    {
                        componentWithNonKeyPathChanged.Add(componentId, fileId);
                    }
                }
            }

            foreach (KeyValuePair<string, string> componentFile in componentWithNonKeyPathChanged)
            {
                // Make sure all changes to non keypath files also had a change in the keypath.
                if (!componentWithChangedKeyPath.ContainsKey(componentFile.Key))
                {
                    this.core.OnMessage(WixWarnings.UpdateOfNonKeyPathFile((string)componentFile.Value, (string)componentFile.Key, (string)componentKeyPath[componentFile.Key]));
                }
            }
        }

        /// <summary>
        /// Binds the summary information table of a database.
        /// </summary>
        /// <param name="output">The output to bind.</param>
        /// <param name="longNames">Returns a flag indicating if uncompressed files use long filenames.</param>
        /// <param name="compressed">Returns a flag indicating if files are compressed by default.</param>
        /// <returns>Modularization guid, or null if the output is not a module.</returns>
        private string BindDatabaseSummaryInfo(Output output, out bool longNames, out bool compressed)
        {
            longNames = false;
            compressed = false;
            string modularizationGuid = null;
            Table summaryInformationTable = output.Tables["_SummaryInformation"];
            if (null != summaryInformationTable)
            {
                bool foundCreateDataTime = false;
                bool foundLastSaveDataTime = false;
                bool foundCreatingApplication = false;
                string now = DateTime.Now.ToString("yyyy/MM/dd HH:mm:ss", CultureInfo.InvariantCulture);

                foreach (Row summaryInformationRow in summaryInformationTable.Rows)
                {
                    switch ((int)summaryInformationRow[0])
                    {
                        case 1: // PID_CODEPAGE
                            // make sure the code page is an int and not a web name or null
                            string codepage = (string)summaryInformationRow[1];

                            if (null == codepage)
                            {
                                codepage = "0";
                            }
                            else
                            {
                                summaryInformationRow[1] = Common.GetValidCodePage(codepage, false, true, summaryInformationRow.SourceLineNumbers).ToString(CultureInfo.InvariantCulture);
                            }
                            break;
                        case 9: // PID_REVNUMBER
                            string packageCode = (string)summaryInformationRow[1];

                            if (OutputType.Module == output.Type)
                            {
                                modularizationGuid = packageCode.Substring(1, 36).Replace('-', '_');
                            }
                            else if ("*" == packageCode)
                            {
                                // set the revision number (package/patch code) if it should be automatically generated
                                summaryInformationRow[1] = Common.GenerateGuid();
                            }
                            break;
                        case 12: // PID_CREATE_DTM
                            foundCreateDataTime = true;
                            break;
                        case 13: // PID_LASTSAVE_DTM
                            foundLastSaveDataTime = true;
                            break;
                        case 15: // PID_WORDCOUNT
                            if (OutputType.Patch == output.Type)
                            {
                                longNames = true;
                                compressed = true;
                            }
                            else
                            {
                                longNames = (0 == (Convert.ToInt32(summaryInformationRow[1], CultureInfo.InvariantCulture) & 1));
                                compressed = (2 == (Convert.ToInt32(summaryInformationRow[1], CultureInfo.InvariantCulture) & 2));
                            }
                            break;
                        case 18: // PID_APPNAME
                            foundCreatingApplication = true;
                            break;
                    }
                }

                // add a summary information row for the create time/date property if its not already set
                if (!foundCreateDataTime)
                {
                    Row createTimeDateRow = summaryInformationTable.CreateRow(null);
                    createTimeDateRow[0] = 12;
                    createTimeDateRow[1] = now;
                }

                // add a summary information row for the last save time/date property if its not already set
                if (!foundLastSaveDataTime)
                {
                    Row lastSaveTimeDateRow = summaryInformationTable.CreateRow(null);
                    lastSaveTimeDateRow[0] = 13;
                    lastSaveTimeDateRow[1] = now;
                }

                // add a summary information row for the creating application property if its not already set
                if (!foundCreatingApplication)
                {
                    Row creatingApplicationRow = summaryInformationTable.CreateRow(null);
                    creatingApplicationRow[0] = 18;
                    creatingApplicationRow[1] = String.Format(CultureInfo.InvariantCulture, AppCommon.GetCreatingApplicationString());
                }
            }

            return modularizationGuid;
        }

        /// <summary>
        /// Binds a databse.
        /// </summary>
        /// <param name="output">The output to bind.</param>
        /// <param name="databaseFile">The database file to create.</param>
        /// <returns>true if binding completed successfully; false otherwise</returns>
        [SuppressMessage("Microsoft.Globalization", "CA1308:NormalizeStringsToUppercase", Justification = "This string is not round tripped, and not used for any security decisions")]
        [SuppressMessage("Microsoft.Globalization", "CA1309:UseOrdinalStringComparison", Justification = "These strings need to be culture insensitive rather than ordinal because they are used for sorting")]
        private bool BindDatabase(Output output, string databaseFile)
        {
            foreach (BinderExtension extension in this.extensions)
            {
                extension.DatabaseInitialize(output);
            }

            Hashtable cabinets = new Hashtable();
            bool compressed = false;
            FileRowCollection fileRows = new FileRowCollection(OutputType.Patch == output.Type);
            ArrayList fileTransfers = new ArrayList();
            bool longNames = false;
            MediaRowCollection mediaRows = new MediaRowCollection();
            Hashtable suppressModularizationIdentifiers = null;
            StringCollection suppressedTableNames = new StringCollection();

            // gather all the wix variables
            Table wixVariableTable = output.Tables["WixVariable"];
            if (null != wixVariableTable)
            {
                foreach (WixVariableRow wixVariableRow in wixVariableTable.Rows)
                {
                    this.WixVariableResolver.AddVariable(wixVariableRow);
                }
            }

            // gather all the suppress modularization identifiers
            Table wixSuppressModularizationTable = output.Tables["WixSuppressModularization"];
            if (null != wixSuppressModularizationTable)
            {
                suppressModularizationIdentifiers = new Hashtable(wixSuppressModularizationTable.Rows.Count);

                foreach (Row row in wixSuppressModularizationTable.Rows)
                {
                    suppressModularizationIdentifiers[row[0]] = null;
                }
            }

            ArrayList delayedFields = new ArrayList();

            // localize fields, resolve wix variables, and resolve file paths
            this.ResolveFields(output.Tables, cabinets, delayedFields);

            // process the summary information table before the other tables
            string modularizationGuid = this.BindDatabaseSummaryInfo(output, out longNames, out compressed);

            // stop processing if an error previously occurred
            if (this.core.EncounteredError)
            {
                return false;
            }

            // set generated component guids
            // this must occur before modularization and after all variables have been resolved
            this.SetComponentGuids(output);

            // modularize identifiers and add tables with real streams to the import tables
            if (OutputType.Module == output.Type)
            {
                foreach (Table table in output.Tables)
                {
                    table.Modularize(modularizationGuid, suppressModularizationIdentifiers);
                }

                // Reset the special property lists after modularization. The linker creates these properties before modularization
                // so we have to reconstruct them for merge modules after modularization in the binder.
                Table wixPropertyTable = output.Tables["WixProperty"];
                if (null != wixPropertyTable)
                {
                    // Create lists of the properties that contribute to the special lists of properties.
                    SortedList adminProperties = new SortedList();
                    SortedList secureProperties = new SortedList();
                    SortedList hiddenProperties = new SortedList();

                    foreach (WixPropertyRow wixPropertyRow in wixPropertyTable.Rows)
                    {
                        if (wixPropertyRow.Admin)
                        {
                            adminProperties[wixPropertyRow.Id] = null;
                        }

                        if (wixPropertyRow.Hidden)
                        {
                            hiddenProperties[wixPropertyRow.Id] = null;
                        }

                        if (wixPropertyRow.Secure)
                        {
                            secureProperties[wixPropertyRow.Id] = null;
                        }
                    }

                    if (0 < adminProperties.Count || 0 < hiddenProperties.Count || 0 < secureProperties.Count)
                    {
                        Table table = output.Tables["Property"];
                        foreach (Row propertyRow in table.Rows)
                        {
                            if ("AdminProperties" == (string)propertyRow[0])
                            {
                                propertyRow[1] = GetPropertyListString(adminProperties);
                            }

                            if ("MsiHiddenProperties" == (string)propertyRow[0])
                            {
                                propertyRow[1] = GetPropertyListString(hiddenProperties);
                            }

                            if ("SecureCustomProperties" == (string)propertyRow[0])
                            {
                                propertyRow[1] = GetPropertyListString(secureProperties);
                            }
                        }
                    }
                }
            }

            // merge unreal table data into the real tables
            // this must occur after all variables and source paths have been resolved
            this.MergeUnrealTables(output.Tables);

            if (this.core.EncounteredError)
            {
                return false;
            }

            if (OutputType.Patch == output.Type)
            {
                foreach (SubStorage substorage in output.SubStorages)
                {
                    Output transform = (Output)substorage.Data;
                    this.ResolveFields(transform.Tables, cabinets, null);
                    this.MergeUnrealTables(transform.Tables);
                }
            }

            // stop processing if an error previously occurred
            if (this.core.EncounteredError)
            {
                return false;
            }

            // index the File table for quicker access later
            // this must occur after the unreal data has been merged in
            Table fileTable = output.Tables["File"];
            if (null != fileTable)
            {
                fileRows.AddRange(fileTable.Rows);
            }

            // index the media table
            Table mediaTable = output.Tables["Media"];
            if (null != mediaTable)
            {
                Dictionary<string, MediaRow> cabinetMediaRows = new Dictionary<string, MediaRow>(StringComparer.InvariantCultureIgnoreCase);
                foreach (MediaRow mediaRow in mediaTable.Rows)
                {
                    // If the Media row has a cabinet, make sure it is unique across all Media rows.
                    if (!String.IsNullOrEmpty(mediaRow.Cabinet))
                    {
                        MediaRow existingRow;
                        if (cabinetMediaRows.TryGetValue(mediaRow.Cabinet, out existingRow))
                        {
                            this.core.OnMessage(WixErrors.DuplicateCabinetName(mediaRow.SourceLineNumbers, mediaRow.Cabinet));
                            this.core.OnMessage(WixErrors.DuplicateCabinetName2(existingRow.SourceLineNumbers, existingRow.Cabinet));
                        }
                        else
                        {
                            cabinetMediaRows.Add(mediaRow.Cabinet, mediaRow);
                        }
                    }

                    mediaRows.Add(mediaRow);
                }
            }

            // stop processing if an error previously occurred
            if (this.core.EncounteredError)
            {
                return false;
            }

            // set the ProductCode if its generated
            Table propertyTable = output.Tables["Property"];
            if (null != propertyTable && OutputType.Product == output.Type)
            {
                foreach (Row propertyRow in propertyTable.Rows)
                {
                    if ("ProductCode" == propertyRow[0].ToString() && "*" == propertyRow[1].ToString())
                    {
                        propertyRow[1] = Common.GenerateGuid();

                        // Update the target ProductCode in any instance transforms
                        foreach (SubStorage subStorage in output.SubStorages)
                        {
                            Output subStorageOutput = (Output)subStorage.Data;
                            if (OutputType.Transform != subStorageOutput.Type)
                            {
                                continue;
                            }

                            Table instanceSummaryInformationTable = subStorageOutput.Tables["_SummaryInformation"];
                            foreach (Row row in instanceSummaryInformationTable.Rows)
                            {
                                if ((int)SummaryInformation.Transform.ProductCodes == (int)row[0])
                                {
                                    row[1] = ((string)row[1]).Replace("*", (string)propertyRow[1]);
                                    break;
                                }
                            }
                        }

                        break;
                    }
                }
            }

            // extract files that come from cabinet files (this does not extract files from merge modules)
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

            // retrieve files and their information from merge modules
            if (OutputType.Product == output.Type)
            {
                this.ProcessMergeModules(output, fileRows);
            }
            else if (OutputType.Patch == output.Type)
            {
                // merge transform data into the output object
                this.CopyTransformData(output, fileRows);
            }

            // stop processing if an error previously occurred
            if (this.core.EncounteredError)
            {
                return false;
            }

            IDictionary<string, string> fileInformationCache = null;
            if (delayedFields.Count != 0)
            {
                fileInformationCache = new Dictionary<string, string>(StringComparer.InvariantCultureIgnoreCase);
            }

            // update file version, hash, assembly, etc.. information
            this.core.OnMessage(WixVerboses.UpdatingFileInformation());
            this.UpdateFileInformation(output, fileRows, mediaRows, fileInformationCache, modularizationGuid);
            this.UpdateControlText(output);

            if (delayedFields.Count != 0)
            {
                this.ResolveDelayedFields(delayedFields, fileInformationCache);
            }

            // stop processing if an error previously occurred
            if (this.core.EncounteredError)
            {
                return false;
            }

            // create cabinet files and process uncompressed files
            string layoutDirectory = Path.GetDirectoryName(databaseFile);
            FileRowCollection uncompressedFileRows = null;
            if (!this.suppressLayout || OutputType.Module == output.Type)
            {
                this.core.OnMessage(WixVerboses.CreatingCabinetFiles());
                uncompressedFileRows = this.CreateCabinetFiles(output, fileRows, fileTransfers, mediaRows, layoutDirectory, compressed);
            }

            if (OutputType.Patch == output.Type)
            {
                // copy output data back into the transforms
                this.CopyTransformData(output, null);
            }

            // stop processing if an error previously occurred
            if (this.core.EncounteredError)
            {
                return false;
            }

            // add back suppressed tables which must be present prior to merging in modules
            if (OutputType.Product == output.Type)
            {
                Table wixMergeTable = output.Tables["WixMerge"];

                if (null != wixMergeTable && 0 < wixMergeTable.Rows.Count)
                {
                    foreach (SequenceTable sequence in Enum.GetValues(typeof(SequenceTable)))
                    {
                        string sequenceTableName = sequence.ToString();
                        Table sequenceTable = output.Tables[sequenceTableName];

                        if (null == sequenceTable)
                        {
                            sequenceTable = output.EnsureTable(this.core.TableDefinitions[sequenceTableName]);
                        }

                        if (0 == sequenceTable.Rows.Count)
                        {
                            suppressedTableNames.Add(sequenceTableName);
                        }
                    }
                }
            }

            foreach (BinderExtension extension in this.extensions)
            {
                extension.DatabaseFinalize(output);
            }

            // generate database file
            this.core.OnMessage(WixVerboses.GeneratingDatabase());
            string tempDatabaseFile = Path.Combine(this.TempFilesLocation, Path.GetFileName(databaseFile));
            this.GenerateDatabase(output, tempDatabaseFile, false, false);
            fileTransfers.Add(new FileTransfer(tempDatabaseFile, databaseFile, true)); // note where this database needs to move in the future

            // stop processing if an error previously occurred
            if (this.core.EncounteredError)
            {
                return false;
            }

            // Output the output to a file
            if (null != this.pdbFile)
            {
                Pdb pdb = new Pdb(null);
                pdb.Output = output;
                pdb.Save(this.pdbFile, null, this.WixVariableResolver, this.TempFilesLocation);
            }

            // merge modules
            if (OutputType.Product == output.Type)
            {
                this.core.OnMessage(WixVerboses.MergingModules());
                this.MergeModules(tempDatabaseFile, output, fileRows, suppressedTableNames);

                // stop processing if an error previously occurred
                if (this.core.EncounteredError)
                {
                    return false;
                }
            }

            // validate the output if there is an MSI validator
            if (null != this.validator)
            {
                // set the output file for source line information
                this.validator.Output = output;

                this.core.OnMessage(WixVerboses.ValidatingDatabase());
                this.core.EncounteredError = !this.validator.Validate(tempDatabaseFile);

                // stop processing if an error previously occurred
                if (this.core.EncounteredError)
                {
                    return false;
                }
            }

            // process uncompressed files
            if (!this.suppressLayout)
            {
                this.ProcessUncompressedFiles(tempDatabaseFile, uncompressedFileRows, fileTransfers, mediaRows, layoutDirectory, compressed, longNames);
            }

            // add LayoutDirectory
            ProcessLayoutDirectories(this.core, output, fileTransfers, layoutDirectory);

            // layout media
            this.core.OnMessage(WixVerboses.LayingOutMedia());
            this.LayoutMedia(fileTransfers, this.suppressAclReset);

            return !this.core.EncounteredError;
        }

        /// <summary>
        /// Get a sorted property list as a semicolon-delimited string.
        /// </summary>
        /// <param name="properties">SortedList of the properties.</param>
        /// <returns>Semicolon-delimited string representing the property list.</returns>
        private static string GetPropertyListString(SortedList properties)
        {
            bool first = true;
            StringBuilder propertiesString = new StringBuilder();

            foreach (string propertyName in properties.Keys)
            {
                if (first)
                {
                    first = false;
                }
                else
                {
                    propertiesString.Append(';');
                }
                propertiesString.Append(propertyName);
            }

            return propertiesString.ToString();
        }

        /// <summary>
        /// Resolve source fields in the tables included in the output
        /// </summary>
        /// <param name="tables">The tables to resolve.</param>
        /// <param name="cabinets">Cabinets containing files that need to be patched.</param>
        /// <param name="delayedFields">The collection of delayed fields.</param>
        private void ResolveFields(TableCollection tables, Hashtable cabinets, ArrayList delayedFields)
        {
            foreach (Table table in tables)
            {
                foreach (Row row in table.Rows)
                {
                    foreach (Field field in row.Fields)
                    {
                        bool isDefault = true;
                        bool delayedResolve = false;

                        // resolve localization and wix variables
                        if (field.Data is string)
                        {
                            field.Data = this.WixVariableResolver.ResolveVariables(row.SourceLineNumbers, (string)field.Data, false, ref isDefault, ref delayedResolve);
                            if (delayedResolve)
                            {
                                delayedFields.Add(new DelayedField(row, field));
                            }
                        }

                        // resolve file paths
                        if (!this.WixVariableResolver.EncounteredError && ColumnType.Object == field.Column.Type)
                        {
                            ObjectField objectField = (ObjectField)field;

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
                                    cabinets.Add(objectField.BaseUri, Path.Combine(this.TempFilesLocation, extractedDirectoryName));
                                }

                                // set the path to the file once its extracted from the cabinet
                                objectField.Data = Path.Combine((string)cabinets[objectField.BaseUri], objectField.CabinetFileId);
                            }
                            else if (null != objectField.Data) // non-compressed file (or localized value)
                            {
                                // when SuppressFileHasAndInfo is true do not resolve file paths
                                if (this.suppressFileHashAndInfo && table.Name == "WixFile")
                                {
                                    continue;
                                }

                                try
                                {
                                    // resolve the path to the file
                                    objectField.Data = this.FileManager.ResolveFile((string)objectField.Data);
                                }
                                catch (WixFileNotFoundException)
                                {
                                    // display the error with source line information
                                    this.core.OnMessage(WixErrors.FileNotFound(row.SourceLineNumbers, (string)objectField.Data));
                                }
                            }

                            isDefault = true;
                            if (null != objectField.PreviousData)
                            {
                                objectField.PreviousData = this.WixVariableResolver.ResolveVariables(row.SourceLineNumbers, objectField.PreviousData, false, ref isDefault);
                                if (!this.WixVariableResolver.EncounteredError)
                                {
                                    // file is compressed in a cabinet (and not modified above)
                                    if (null != objectField.PreviousCabinetFileId && isDefault)
                                    {
                                        // when loading transforms from disk, PreviousBaseUri may not have been set
                                        if (null == objectField.PreviousBaseUri)
                                        {
                                            objectField.PreviousBaseUri = objectField.BaseUri;
                                        }

                                        // index cabinets that have not been previously encountered
                                        if (!cabinets.ContainsKey(objectField.PreviousBaseUri))
                                        {
                                            Uri baseUri = new Uri(objectField.PreviousBaseUri);
                                            string localFileNameWithoutExtension = Path.GetFileNameWithoutExtension(baseUri.LocalPath);
                                            string extractedDirectoryName = String.Format(CultureInfo.InvariantCulture, "cab_{0}_{1}", cabinets.Count, localFileNameWithoutExtension);

                                            // index the cabinet file's base URI (source location) and extracted directory
                                            cabinets.Add(objectField.PreviousBaseUri, Path.Combine(this.TempFilesLocation, extractedDirectoryName));
                                        }

                                        // set the path to the file once its extracted from the cabinet
                                        objectField.PreviousData = Path.Combine((string)cabinets[objectField.PreviousBaseUri], objectField.PreviousCabinetFileId);
                                    }
                                    else if (null != objectField.PreviousData) // non-compressed file (or localized value)
                                    {
                                        // when SuppressFileHasAndInfo is true do not resolve file paths
                                        if (this.suppressFileHashAndInfo && table.Name == "WixFile")
                                        {
                                            continue;
                                        }

                                        try
                                        {
                                            // resolve the path to the file
                                            objectField.PreviousData = this.FileManager.ResolveFile((string)objectField.PreviousData);
                                        }
                                        catch (WixFileNotFoundException)
                                        {
                                            // display the error with source line information
                                            this.core.OnMessage(WixErrors.FileNotFound(row.SourceLineNumbers, (string)objectField.PreviousData));
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

            // remember if the variable resolver found an error
            if (this.WixVariableResolver.EncounteredError)
            {
                this.core.EncounteredError = true;
            }
        }

        /// <summary>
        /// Resolves the fields which had variables that needed to be resolved after the file information
        /// was loaded.
        /// </summary>
        /// <param name="delayedFields">The fields which had resolution delayed.</param>
        /// <param name="fileInformationCache">The file information to use when resolving variables.</param>
        private void ResolveDelayedFields(ArrayList delayedFields, IDictionary<string, string> fileInformationCache)
        {
            foreach (DelayedField delayedField in delayedFields)
            {
                try
                {
                    delayedField.Field.Data = WixVariableResolver.ResolveDelayedVariables(delayedField.Row.SourceLineNumbers, (string)delayedField.Field.Data, fileInformationCache);
                }
                catch (WixException we)
                {
                    this.core.OnMessage(we.Error);
                    continue;
                }
            }
        }

        /// <summary>
        /// Tests sequence table for PatchFiles and associated actions
        /// </summary>
        /// <param name="iesTable">The table to test.</param>
        /// <param name="hasPatchFilesAction">Set to true if PatchFiles action is found. Left unchanged otherwise.</param>
        /// <param name="seqInstallFiles">Set to sequence value of InstallFiles action if found. Left unchanged otherwise.</param>
        /// <param name="seqDuplicateFiles">Set to sequence value of DuplicateFiles action if found. Left unchanged otherwise.</param>
        private static void TestSequenceTableForPatchFilesAction(Table iesTable, ref bool hasPatchFilesAction, ref int seqInstallFiles, ref int seqDuplicateFiles)
        {
            if (null != iesTable)
            {
                foreach (Row iesRow in iesTable.Rows)
                {
                    if (String.Equals("PatchFiles", (string)iesRow[0], StringComparison.Ordinal))
                    {
                        hasPatchFilesAction = true;
                    }
                    if (String.Equals("InstallFiles", (string)iesRow[0], StringComparison.Ordinal))
                    {
                        seqInstallFiles = (int)iesRow.Fields[2].Data;
                    }
                    if (String.Equals("DuplicateFiles", (string)iesRow[0], StringComparison.Ordinal))
                    {
                        seqDuplicateFiles = (int)iesRow.Fields[2].Data;
                    }
                }
            }
        }

        /// <summary>
        /// Adds the PatchFiles action to the sequence table if it does not already exist.
        /// </summary>
        /// <param name="table">The sequence table to check or modify.</param>
        /// <param name="mainTransform">The primary authoring transform.</param>
        /// <param name="pairedTransform">The secondary patch transform.</param>
        /// <param name="mainFileRow">The file row that contains information about the patched file.</param>
        private void AddPatchFilesActionToSequenceTable(SequenceTable table, Output mainTransform, Output pairedTransform, Row mainFileRow)
        {
            // Find/add PatchFiles action (also determine sequence for it).
            // Search mainTransform first, then pairedTransform (pairedTransform overrides).
            bool hasPatchFilesAction = false;
            int seqInstallFiles = 0;
            int seqDuplicateFiles = 0;
            string tableName = table.ToString();

            TestSequenceTableForPatchFilesAction(
                    mainTransform.Tables[tableName],
                    ref hasPatchFilesAction,
                    ref seqInstallFiles,
                    ref seqDuplicateFiles);
            TestSequenceTableForPatchFilesAction(
                    pairedTransform.Tables[tableName],
                    ref hasPatchFilesAction,
                    ref seqInstallFiles,
                    ref seqDuplicateFiles);
            if (!hasPatchFilesAction)
            {
                Table iesTable = pairedTransform.EnsureTable(this.core.TableDefinitions[tableName]);
                if (0 == iesTable.Rows.Count)
                {
                    iesTable.Operation = TableOperation.Add;
                }
                Row patchAction = iesTable.CreateRow(null);
                WixActionRow wixPatchAction = Installer.GetStandardActions()[table, "PatchFiles"];
                int sequence = wixPatchAction.Sequence;
                // Test for default sequence value's appropriateness
                if (seqInstallFiles >= sequence || (0 != seqDuplicateFiles && seqDuplicateFiles <= sequence))
                {
                    if (0 != seqDuplicateFiles)
                    {
                        if (seqDuplicateFiles < seqInstallFiles)
                        {
                            throw new WixException(WixErrors.InsertInvalidSequenceActionOrder(mainFileRow.SourceLineNumbers, iesTable.Name, "InstallFiles", "DuplicateFiles", wixPatchAction.Action));
                        }
                        else
                        {
                            sequence = (seqDuplicateFiles + seqInstallFiles) / 2;
                            if (seqInstallFiles == sequence || seqDuplicateFiles == sequence)
                            {
                                throw new WixException(WixErrors.InsertSequenceNoSpace(mainFileRow.SourceLineNumbers, iesTable.Name, "InstallFiles", "DuplicateFiles", wixPatchAction.Action));
                            }
                        }
                    }
                    else
                    {
                        sequence = seqInstallFiles + 1;
                    }
                }
                patchAction[0] = wixPatchAction.Action;
                patchAction[1] = wixPatchAction.Condition;
                patchAction[2] = sequence;
                patchAction.Operation = RowOperation.Add;
            }
        }

        /// <summary>
        /// Copy file data between transform substorages and the patch output object
        /// </summary>
        /// <param name="output">The output to bind.</param>
        /// <param name="allFileRows">True if copying from transform to patch, false the other way.</param>
        /// <returns>true if binding completed successfully; false otherwise</returns>
        private bool CopyTransformData(Output output, FileRowCollection allFileRows)
        {
            bool copyToPatch = (allFileRows != null);
            bool copyFromPatch = !copyToPatch;
            if (OutputType.Patch != output.Type)
            {
                return true;
            }

            Hashtable patchMediaRows = new Hashtable();
            Hashtable patchMediaFileRows = new Hashtable();
            Table patchFileTable = output.EnsureTable(this.core.TableDefinitions["File"]);
            if (copyFromPatch)
            {
                // index patch files by diskId+fileId
                foreach (FileRow patchFileRow in patchFileTable.Rows)
                {
                    int diskId = patchFileRow.DiskId;
                    if (!patchMediaFileRows.Contains(diskId))
                    {
                        patchMediaFileRows[diskId] = new FileRowCollection();
                    }
                    FileRowCollection mediaFileRows = (FileRowCollection)patchMediaFileRows[diskId];
                    mediaFileRows.Add(patchFileRow);
                }

                Table patchMediaTable = output.EnsureTable(this.core.TableDefinitions["Media"]);
                foreach (MediaRow patchMediaRow in patchMediaTable.Rows)
                {
                    patchMediaRows[patchMediaRow.DiskId] = patchMediaRow;
                }
            }

            // index paired transforms
            Hashtable pairedTransforms = new Hashtable();
            foreach (SubStorage substorage in output.SubStorages)
            {
                if ("#" == substorage.Name.Substring(0, 1))
                {
                    pairedTransforms[substorage.Name.Substring(1)] = substorage.Data;
                }
            }

            try
            {
                // copy File bind data into substorages
                foreach (SubStorage substorage in output.SubStorages)
                {
                    if ("#" == substorage.Name.Substring(0, 1))
                    {
                        // no changes necessary for paired transforms
                        continue;
                    }

                    Output mainTransform = (Output)substorage.Data;
                    Table mainWixFileTable = mainTransform.Tables["WixFile"];
                    Table mainMsiFileHashTable = mainTransform.Tables["MsiFileHash"];

                    int numWixFileRows = (null != mainWixFileTable) ? mainWixFileTable.Rows.Count : 0;
                    int numMsiFileHashRows = (null != mainMsiFileHashTable) ? mainMsiFileHashTable.Rows.Count : 0;

                    this.FileManager.ActiveSubStorage = substorage;
                    Hashtable wixFiles = new Hashtable(numWixFileRows);
                    Hashtable mainMsiFileHashIndex = new Hashtable(numMsiFileHashRows);
                    Table mainFileTable = mainTransform.Tables["File"];
                    Output pairedTransform = (Output)pairedTransforms[substorage.Name];

                    if (null != mainWixFileTable)
                    {
                        // Index the WixFile table for later use.
                        foreach (WixFileRow row in mainWixFileTable.Rows)
                        {
                            wixFiles.Add(row.Fields[0].Data.ToString(), row);
                        }
                    }

                    // copy Media.LastSequence and index the MsiFileHash table if it exists.
                    if (copyFromPatch)
                    {
                        Table pairedMediaTable = pairedTransform.Tables["Media"];
                        foreach (MediaRow pairedMediaRow in pairedMediaTable.Rows)
                        {
                            MediaRow patchMediaRow = (MediaRow)patchMediaRows[pairedMediaRow.DiskId];
                            pairedMediaRow.Fields[1] = patchMediaRow.Fields[1];
                        }

                        if (null != mainMsiFileHashTable)
                        {
                            // Index the MsiFileHash table for later use.
                            foreach (Row row in mainMsiFileHashTable.Rows)
                            {
                                mainMsiFileHashIndex.Add(row[0], row);
                            }
                        }

                        // Validate file row changes for keypath-related issues
                        this.ValidateFileRowChanges(mainTransform);
                    }

                    // index File table of pairedTransform
                    FileRowCollection pairedFileRows = new FileRowCollection();
                    Table pairedFileTable = pairedTransform.Tables["File"];
                    if (null != pairedFileTable)
                    {
                        pairedFileRows.AddRange(pairedFileTable.Rows);
                    }

                    if (null != mainFileTable)
                    {
                        if (copyFromPatch)
                        {
                            // Remove the MsiFileHash table because it will be updated later with the final file hash for each file
                            mainTransform.Tables.Remove("MsiFileHash");
                        }

                        foreach (FileRow mainFileRow in mainFileTable.Rows)
                        {
                            if (mainFileRow.Operation == RowOperation.Delete)
                            {
                                continue;
                            }

                            if (!copyToPatch && mainFileRow.Operation == RowOperation.None)
                            {
                                continue;
                            }
                            // When copying to the patch, we need compare the underlying files and include all file changes.
                            else if (copyToPatch)
                            {
                                WixFileRow wixFileRow = (WixFileRow)wixFiles[mainFileRow.Fields[0].Data.ToString()];
                                ObjectField objectField = (ObjectField)wixFileRow.Fields[6];
                                FileRow pairedFileRow = pairedFileRows[mainFileRow.Fields[0].Data.ToString()];

                                // If the file is new, we always need to add it to the patch.
                                if (mainFileRow.Operation != RowOperation.Add)
                                {
                                    // If PreviousData doesn't exist, target and upgrade layout point to the same location. No need to compare.
                                    if (null == objectField.PreviousData)
                                    {
                                        if (mainFileRow.Operation == RowOperation.None)
                                        {
                                            continue;
                                        }
                                    }
                                    else
                                    {
                                        // TODO: should this entire condition be placed in the binder file manager?
                                        if ((0 == (PatchAttributeType.Ignore & wixFileRow.PatchAttributes)) &&
                                            !this.FileManager.CompareFiles(objectField.PreviousData.ToString(), objectField.Data.ToString()))
                                        {
                                            // If the file is different, we need to mark the mainFileRow and pairedFileRow as modified.
                                            mainFileRow.Operation = RowOperation.Modify;
                                            if (null != pairedFileRow)
                                            {
                                                // Always patch-added, but never non-compressed.
                                                pairedFileRow.Attributes |= MsiInterop.MsidbFileAttributesPatchAdded;
                                                pairedFileRow.Attributes &= ~MsiInterop.MsidbFileAttributesNoncompressed;
                                                pairedFileRow.Fields[6].Modified = true;
                                                pairedFileRow.Operation = RowOperation.Modify;
                                            }
                                        }
                                        else
                                        {
                                            // The File is same. We need mark all the attributes as unchanged.
                                            mainFileRow.Operation = RowOperation.None;
                                            foreach (Field field in mainFileRow.Fields)
                                            {
                                                field.Modified = false;
                                            }

                                            if (null != pairedFileRow)
                                            {
                                                pairedFileRow.Attributes &= ~MsiInterop.MsidbFileAttributesPatchAdded;
                                                pairedFileRow.Fields[6].Modified = false;
                                                pairedFileRow.Operation = RowOperation.None;
                                            }
                                            continue;
                                        }
                                    }
                                }
                                else if (null != pairedFileRow) // RowOperation.Add
                                {
                                    // Always patch-added, but never non-compressed.
                                    pairedFileRow.Attributes |= MsiInterop.MsidbFileAttributesPatchAdded;
                                    pairedFileRow.Attributes &= ~MsiInterop.MsidbFileAttributesNoncompressed;
                                    pairedFileRow.Fields[6].Modified = true;
                                    pairedFileRow.Operation = RowOperation.Add;
                                }
                            }

                            // index patch files by diskId+fileId
                            int diskId = mainFileRow.DiskId;
                            if (!patchMediaFileRows.Contains(diskId))
                            {
                                patchMediaFileRows[diskId] = new FileRowCollection();
                            }
                            FileRowCollection mediaFileRows = (FileRowCollection)patchMediaFileRows[diskId];

                            string fileId = mainFileRow.File;
                            FileRow patchFileRow = mediaFileRows[fileId];
                            if (copyToPatch)
                            {
                                if (null == patchFileRow)
                                {
                                    patchFileRow = (FileRow)patchFileTable.CreateRow(null);
                                    patchFileRow.CopyFrom(mainFileRow);
                                    mediaFileRows.Add(patchFileRow);
                                    allFileRows.Add(patchFileRow);
                                }
                                else
                                {
                                    // TODO: confirm the rest of data is identical?

                                    // make sure Source is same. Otherwise we are silently ignoring a file.
                                    if (0 != String.Compare(patchFileRow.Source, mainFileRow.Source, StringComparison.OrdinalIgnoreCase))
                                    {
                                        this.core.OnMessage(WixErrors.SameFileIdDifferentSource(mainFileRow.SourceLineNumbers, fileId, patchFileRow.Source, mainFileRow.Source));
                                    }
                                    // capture the previous file versions (and associated data) from this targeted instance of the baseline into the current filerow.
                                    patchFileRow.AppendPreviousDataFrom(mainFileRow);
                                }
                            }
                            else
                            {
                                // copy data from the patch back to the transform
                                if (null != patchFileRow)
                                {
                                    FileRow pairedFileRow = (FileRow)pairedFileRows[fileId];
                                    for (int i = 0; i < patchFileRow.Fields.Length; i++)
                                    {
                                        string patchValue = patchFileRow[i] == null ? "" : patchFileRow[i].ToString();
                                        string mainValue = mainFileRow[i] == null ? "" : mainFileRow[i].ToString();

                                        // File.Attributes should not changed for binary deltas
                                        if (6 == i)
                                        {
                                            if (null != patchFileRow.Patch)
                                            {
                                                // File.Attribute should not change for binary deltas
                                                pairedFileRow.Attributes = mainFileRow.Attributes;
                                                mainFileRow.Fields[i].Modified = false;
                                            }
                                        }
                                        // File.Sequence is updated in pairedTransform, not mainTransform
                                        else if (7 == i)
                                        {
                                            // file sequence is updated in Patch table instead of File table for delta patches
                                            if (null != patchFileRow.Patch)
                                            {
                                                pairedFileRow.Fields[i].Modified = false;
                                            }
                                            else
                                            {
                                                pairedFileRow[i] = patchFileRow[i];
                                                pairedFileRow.Fields[i].Modified = true;
                                            }
                                            mainFileRow.Fields[i].Modified = false;
                                        }
                                        else if (patchValue != mainValue)
                                        {
                                            mainFileRow[i] = patchFileRow[i];
                                            mainFileRow.Fields[i].Modified = true;
                                            if (mainFileRow.Operation == RowOperation.None)
                                            {
                                                mainFileRow.Operation = RowOperation.Modify;
                                            }
                                        }
                                    }

                                    // copy MsiFileHash row for this File
                                    Row patchHashRow;
                                    if (mainMsiFileHashIndex.ContainsKey(patchFileRow.File))
                                    {
                                        patchHashRow = (Row)mainMsiFileHashIndex[patchFileRow.File];
                                    }
                                    else
                                    {
                                        patchHashRow = patchFileRow.HashRow;
                                    }

                                    if (null != patchHashRow)
                                    {
                                        Table mainHashTable = mainTransform.EnsureTable(this.core.TableDefinitions["MsiFileHash"]);
                                        Row mainHashRow = mainHashTable.CreateRow(mainFileRow.SourceLineNumbers);
                                        for (int i = 0; i < patchHashRow.Fields.Length; i++)
                                        {
                                            mainHashRow[i] = patchHashRow[i];
                                            if (i > 1)
                                            {
                                                // assume all hash fields have been modified
                                                mainHashRow.Fields[i].Modified = true;
                                            }
                                        }

                                        // assume the MsiFileHash operation follows the File one
                                        mainHashRow.Operation = mainFileRow.Operation;
                                    }

                                    // copy MsiAssemblyName rows for this File
                                    RowCollection patchAssemblyNameRows = patchFileRow.AssemblyNameRows;
                                    if (null != patchAssemblyNameRows)
                                    {
                                        Table mainAssemblyNameTable = mainTransform.EnsureTable(this.core.TableDefinitions["MsiAssemblyName"]);
                                        foreach (Row patchAssemblyNameRow in patchAssemblyNameRows)
                                        {
                                            // Copy if there isn't an identical modified row already in the transform.
                                            bool foundMatchingModifiedRow = false;
                                            foreach (Row mainAssemblyNameRow in mainAssemblyNameTable.Rows)
                                            {
                                                if (mainAssemblyNameRow.IsIdentical(patchAssemblyNameRow) && RowOperation.None != mainAssemblyNameRow.Operation)
                                                {
                                                    foundMatchingModifiedRow = true;
                                                    break;
                                                }
                                            }

                                            if (!foundMatchingModifiedRow)
                                            {
                                                Row mainAssemblyNameRow = mainAssemblyNameTable.CreateRow(mainFileRow.SourceLineNumbers);
                                                for (int i = 0; i < patchAssemblyNameRow.Fields.Length; i++)
                                                {
                                                    mainAssemblyNameRow[i] = patchAssemblyNameRow[i];
                                                }

                                                // assume value field has been modified
                                                mainAssemblyNameRow.Fields[2].Modified = true;
                                                mainAssemblyNameRow.Operation = mainFileRow.Operation;
                                            }
                                        }
                                    }

                                    // Add patch header for this file
                                    if (null != patchFileRow.Patch)
                                    {
                                        // Add the PatchFiles action automatically to the AdminExecuteSequence and InstallExecuteSequence tables.
                                        AddPatchFilesActionToSequenceTable(SequenceTable.AdminExecuteSequence, mainTransform, pairedTransform, mainFileRow);
                                        AddPatchFilesActionToSequenceTable(SequenceTable.InstallExecuteSequence, mainTransform, pairedTransform, mainFileRow);

                                        // Add to Patch table
                                        Table patchTable = pairedTransform.EnsureTable(this.core.TableDefinitions["Patch"]);
                                        if (0 == patchTable.Rows.Count)
                                        {
                                            patchTable.Operation = TableOperation.Add;
                                        }
                                        Row patchRow = patchTable.CreateRow(mainFileRow.SourceLineNumbers);
                                        patchRow[0] = patchFileRow.File;
                                        patchRow[1] = patchFileRow.Sequence;
                                        FileInfo patchFile = new FileInfo(patchFileRow.Source);
                                        patchRow[2] = (int)patchFile.Length;
                                        patchRow[3] = 0 == (PatchAttributeType.AllowIgnoreOnError & patchFileRow.PatchAttributes) ? 0 : 1;
                                        string streamName = patchTable.Name + "." + patchRow[0] + "." + patchRow[1];
                                        if (MsiInterop.MsiMaxStreamNameLength < streamName.Length)
                                        {
                                            streamName = "_" + Guid.NewGuid().ToString("D").ToUpper(CultureInfo.InvariantCulture).Replace('-', '_');
                                            Table patchHeadersTable = pairedTransform.EnsureTable(this.core.TableDefinitions["MsiPatchHeaders"]);
                                            if (0 == patchHeadersTable.Rows.Count)
                                            {
                                                patchHeadersTable.Operation = TableOperation.Add;
                                            }
                                            Row patchHeadersRow = patchHeadersTable.CreateRow(mainFileRow.SourceLineNumbers);
                                            patchHeadersRow[0] = streamName;
                                            patchHeadersRow[1] = patchFileRow.Patch;
                                            patchRow[5] = streamName;
                                            patchHeadersRow.Operation = RowOperation.Add;
                                        }
                                        else
                                        {
                                            patchRow[4] = patchFileRow.Patch;
                                        }
                                        patchRow.Operation = RowOperation.Add;
                                    }
                                }
                                else
                                {
                                    // TODO: throw because all transform rows should have made it into the patch
                                }
                            }
                        }
                    }

                    if (copyFromPatch)
                    {
                        output.Tables.Remove("Media");
                        output.Tables.Remove("File");
                        output.Tables.Remove("MsiFileHash");
                        output.Tables.Remove("MsiAssemblyName");
                    }
                }
            }
            finally
            {
                this.FileManager.ActiveSubStorage = null;
            }

            return true;
        }

        /// <summary>
        /// Takes an id, and demodularizes it (if possible).
        /// </summary>
        /// <remarks>
        /// If the output type is a module, returns a demodularized version of an id. Otherwise, returns the id.
        /// </remarks>
        /// <param name="output">The output to bind.</param>
        /// <param name="modularizationGuid">The modularization GUID.</param>
        /// <param name="id">The id to demodularize.</param>
        /// <returns>The demodularized id.</returns>
        private static string Demodularize(Output output, string modularizationGuid, string id)
        {
            if (OutputType.Module == output.Type && id.EndsWith(String.Concat(".", modularizationGuid), StringComparison.Ordinal))
            {
                id = id.Substring(0, id.Length - 37);
            }

            return id;
        }

        /// <summary>
        /// Binds a bundle.
        /// </summary>
        /// <param name="bundle">The bundle to bind.</param>
        /// <param name="bundleFile">The bundle to create.</param>
        /// <returns>true if binding completed successfully; false otherwise</returns>
        private bool BindBundle(Output bundle, string bundleFile)
        {
            // First look for data we expect to find... Chain, WixGroups, etc.
            Table chainPackageTable = bundle.Tables["ChainPackage"];
            if (null == chainPackageTable || 0 == chainPackageTable.Rows.Count)
            {
                // We shouldn't really get past the linker phase if there are
                // no group items... that means that there's no UX, no Chain,
                // *and* no Containers!
                throw new WixException(WixErrors.MissingBundleInformation("ChainPackage"));
            }

            Table wixGroupTable = bundle.Tables["WixGroup"];
            if (null == wixGroupTable || 0 == wixGroupTable.Rows.Count)
            {
                // We shouldn't really get past the linker phase if there are
                // no group items... that means that there's no UX, no Chain,
                // *and* no Containers!
                throw new WixException(WixErrors.MissingBundleInformation("WixGroup"));
            }

            foreach (BinderExtension extension in this.extensions)
            {
                extension.BundleInitialize(bundle);
            }

            if (this.core.EncounteredError)
            {
                return false;
            }

            // To make lookups easier, we load the constituent data bottom-up, so
            // that we can index by ID.
            Table variableTable = bundle.Tables["Variable"];
            // TODO: Do we need a dictionary for variables?
            List<VariableInfo> allVariables = new List<VariableInfo>();
            if (null != variableTable && 0 < variableTable.Rows.Count)
            {
                foreach (Row row in variableTable.Rows)
                {
                    allVariables.Add(new VariableInfo(row));
                }
            }

            // TODO: Although the WixSearch tables are defined in the Util extension,
            // the Bundle Binder has to know all about them. We hope to revisit all
            // of this in the 4.0 timeframe.
            Dictionary<string, WixSearchInfo> allSearches = new Dictionary<string, WixSearchInfo>();
            Table wixFileSearchTable = bundle.Tables["WixFileSearch"];
            if (null != wixFileSearchTable && 0 < wixFileSearchTable.Rows.Count)
            {
                foreach (Row row in wixFileSearchTable.Rows)
                {
                    WixFileSearchInfo fileSearchInfo = new WixFileSearchInfo(row);
                    allSearches.Add(fileSearchInfo.Id, fileSearchInfo);
                }
            }

            Table wixRegistrySearchTable = bundle.Tables["WixRegistrySearch"];
            if (null != wixRegistrySearchTable && 0 < wixRegistrySearchTable.Rows.Count)
            {
                foreach (Row row in wixRegistrySearchTable.Rows)
                {
                    WixRegistrySearchInfo registrySearchInfo = new WixRegistrySearchInfo(row);
                    allSearches.Add(registrySearchInfo.Id, registrySearchInfo);
                }
            }

            Table wixComponentSearchTable = bundle.Tables["WixComponentSearch"];
            if (null != wixComponentSearchTable && 0 < wixComponentSearchTable.Rows.Count)
            {
                foreach (Row row in wixComponentSearchTable.Rows)
                {
                    WixComponentSearchInfo componentSearchInfo = new WixComponentSearchInfo(row);
                    allSearches.Add(componentSearchInfo.Id, componentSearchInfo);
                }
            }

            Table wixProductSearchTable = bundle.Tables["WixProductSearch"];
            if (null != wixProductSearchTable && 0 < wixProductSearchTable.Rows.Count)
            {
                foreach (Row row in wixProductSearchTable.Rows)
                {
                    WixProductSearchInfo productSearchInfo = new WixProductSearchInfo(row);
                    allSearches.Add(productSearchInfo.Id, productSearchInfo);
                }
            }

            // Merge in the variable/condition info and get the canonical ordering for
            // the searches.
            List<WixSearchInfo> orderedSearches = new List<WixSearchInfo>();
            Table wixSearchTable = bundle.Tables["WixSearch"];
            if (null != wixSearchTable && 0 < wixSearchTable.Rows.Count)
            {
                foreach (Row row in wixSearchTable.Rows)
                {
                    WixSearchInfo searchInfo = allSearches[(string)row[0]];
                    searchInfo.AddWixSearchRowInfo(row);
                    orderedSearches.Add(searchInfo);
                }
            }

            // For now, we have only UX payloads... this may need to get refactored
            // into more sensible parts as the Chain/MsiPackages and Containers
            // get implemented end-to-end.
            Table payloadTable = bundle.Tables["Payload"];
            Dictionary<string, PayloadInfo> allPayloads = new Dictionary<string, PayloadInfo>();
            foreach (Row row in payloadTable.Rows)
            {
                PayloadInfo payloadInfo = new PayloadInfo(row, this.FileManager);
                allPayloads.Add(payloadInfo.Id, payloadInfo);
            }

            // Get the UX payloads...
            List<PayloadInfo> uxPayloads = new List<PayloadInfo>();
            foreach (Row row in wixGroupTable.Rows)
            {
                string rowParentName = (string)row[0];
                string rowParentType = (string)row[1];
                string rowChildName = (string)row[2];
                string rowChildType = (string)row[3];

                if ("PayloadGroup" == rowParentType && "WixUXContainer" == rowParentName && "Payload" == rowChildType)
                {
                    uxPayloads.Add(allPayloads[rowChildName]);
                }
            }

            // If we didn't get any UX payloads, it's an error!
            if (0 == uxPayloads.Count)
            {
                throw new WixException(WixErrors.MissingBundleInformation("UX"));
            }

            // Give the UX payloads their embedded IDs...
            for (int uxPayloadIndex = 0; uxPayloadIndex < uxPayloads.Count; ++uxPayloadIndex)
            {
                PayloadInfo payload = uxPayloads[uxPayloadIndex];

                // In theory, UX payloads could be embedded in the UX CAB, external to the
                // bundle EXE, or even downloaded. The current engine requires the UX to be
                // fully present before any downloading starts, so that rules out one type.
                // Also, the burn engine does not currently copy external UX payloads into
                // the temporary UX directory correctly, so we don't allow external either.
                switch (payload.Packaging)
                {
                    case PayloadInfo.PackagingType.Unknown:
                    case PayloadInfo.PackagingType.Embedded:
                        // These are fine.
                        break;

                    default:
                        core.OnMessage(WixWarnings.UxPayloadsOnlySupportEmbedding(payload.SourceLineNumbers, payload.FileInfo.FullName));
                        break;
                }

                payload.Packaging = PayloadInfo.PackagingType.Embedded;
                payload.EmbeddedId = String.Format(CultureInfo.InvariantCulture, Binder.BurnUXContainerEmbeddedIdFormat, uxPayloadIndex);
            }

            // Get the chain packages...
            Dictionary<string, ChainPackageInfo> allPackages = new Dictionary<string, ChainPackageInfo>();
            foreach (Row row in chainPackageTable.Rows)
            {
                ChainPackageInfo packageInfo = new ChainPackageInfo(row, wixGroupTable, allPayloads, this.FileManager, this.core);
                allPackages.Add(packageInfo.Id, packageInfo);
            }

            if (this.core.EncounteredError)
            {
                return false;
            }

            List<ChainPackageInfo> chainPackages = new List<ChainPackageInfo>();
            foreach (Row row in wixGroupTable.Rows)
            {
                string rowParentName = (string)row[0];
                string rowParentType = (string)row[1];
                string rowChildName = (string)row[2];
                string rowChildType = (string)row[3];

                if ("PackageGroup" == rowParentType && "WixChain" == rowParentName && "Package" == rowChildType)
                {
                    chainPackages.Add(allPackages[rowChildName]);
                }
            }

            // Give the chain package payloads their embedded IDs...
            int payloadIndex = 0;
            foreach (ChainPackageInfo package in chainPackages)
            {
                PayloadInfo packagePayload = package.PackagePayload;
                if (PayloadInfo.PackagingType.Unknown == packagePayload.Packaging || PayloadInfo.PackagingType.Embedded == packagePayload.Packaging)
                {
                    packagePayload.Packaging = PayloadInfo.PackagingType.Embedded;
                    packagePayload.EmbeddedId = String.Format(CultureInfo.InvariantCulture, Binder.BurnAttachedContainerEmbeddedIdFormat, payloadIndex);
                    ++payloadIndex;
                }

                foreach (PayloadInfo payload in package.Payloads)
                {
                    if (PayloadInfo.PackagingType.Unknown == payload.Packaging || PayloadInfo.PackagingType.Embedded == payload.Packaging)
                    {
                        payload.Packaging = PayloadInfo.PackagingType.Embedded;
                        payload.EmbeddedId = String.Format(CultureInfo.InvariantCulture, Binder.BurnAttachedContainerEmbeddedIdFormat, payloadIndex);
                        ++payloadIndex;
                    }
                }
            }

            // Load the MsiProperty information...
            Table msiPropertyTable = bundle.Tables["MsiProperty"];
            if (null != msiPropertyTable && 0 < msiPropertyTable.Rows.Count)
            {
                foreach (Row row in msiPropertyTable.Rows)
                {
                    MsiPropertyInfo msiProperty = new MsiPropertyInfo(row);

                    ChainPackageInfo package;
                    if (allPackages.TryGetValue(msiProperty.PackageId, out package))
                    {
                        package.MsiProperties.Add(msiProperty);
                    }
                    else
                    {
                        core.OnMessage(WixErrors.IdentifierNotFound("Package", msiProperty.PackageId));
                    }
                }
            }

            // Generate the core-defined UX manifest tables...
            this.GenerateWixPackageProperties(bundle, chainPackages);

            foreach (BinderExtension extension in this.extensions)
            {
                extension.BundleFinalize(bundle);
            }

            // As more parts of the bundle get implemented, we may prefer to
            // make some of the local variables here into members of Binder
            // so that we don't have to pass them explicitly to the sub-methods.
            // For now, though, this isn't too bad.
            Guid bundleGuid = Guid.NewGuid();
            string stubFile = null;

            // default to burnstub.exe if the source manifest didn't say otherwise
            if (String.IsNullOrEmpty(stubFile))
            {
                string wixExeDirectory = Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location);
                stubFile = Path.Combine(wixExeDirectory, "burnstub.exe");
            }

            // start with a writable copy of the stub .exe
            this.core.OnMessage(WixVerboses.GeneratingBundle(bundleFile, stubFile));
            File.Copy(stubFile, bundleFile, true);
            File.SetAttributes(bundleFile, File.GetAttributes(bundleFile) & ~System.IO.FileAttributes.ReadOnly);

            // Create our manifests, CABs and final EXE...
            bool perMachineBundle = true;
            RegistrationInfo registrationInfo = null;
            this.GetChainInformation(chainPackages, ref perMachineBundle, ref registrationInfo);

            string uxManifestPath = this.CreateUxManifest(bundle, uxPayloads);

            string parameterInfoPath = this.CreateBurnParameterInfo(bundleFile, bundleGuid, allVariables, orderedSearches, uxPayloads, chainPackages, perMachineBundle, registrationInfo);
            string manifestPath = this.CreateBurnManifest(allVariables, orderedSearches, uxPayloads, chainPackages, perMachineBundle, registrationInfo);

            string manifestUXContainerPath = Path.Combine(this.TempFilesLocation, "bundle-ux.cab");
            string attachedContainerPath = null;

            this.CreateContainer(manifestUXContainerPath, uxPayloads, manifestPath);

            // create the attached container, if it is needed.
            // TODO: properly determine which packages/payloads go into the attached container...
            if (0 < chainPackages.Count)
            {
                attachedContainerPath = Path.Combine(this.TempFilesLocation, "bundle-attached.cab");
                // CreateContainer needs a list of PayloadInfo...
                List<PayloadInfo> embdeddedChainPayloads = new List<PayloadInfo>();
                foreach (ChainPackageInfo package in chainPackages)
                {
                    if (PayloadInfo.PackagingType.Embedded == package.PackagePayload.Packaging)
                    {
                        embdeddedChainPayloads.Add(package.PackagePayload);
                    }

                    foreach (PayloadInfo payload in package.Payloads)
                    {
                        if (PayloadInfo.PackagingType.Embedded == payload.Packaging)
                        {
                            embdeddedChainPayloads.Add(payload);
                        }
                    }
                }
                this.CreateContainer(attachedContainerPath, embdeddedChainPayloads, null);
            }

            // update the .wixburn section to point to at the UX and attached container(s) then attach the container(s).
            using (BurnWriter writer = new BurnWriter(bundleFile, this.core, bundleGuid))
            {
                writer.AppendContainer(manifestUXContainerPath, BurnWriter.Container.UX);

                if (!String.IsNullOrEmpty(attachedContainerPath))
                {
                    writer.AppendContainer(attachedContainerPath, BurnWriter.Container.Attached);
                }
            }

            // Output the bundle to a file
            if (null != this.pdbFile)
            {
                Pdb pdb = new Pdb(null);
                pdb.Output = bundle;
                pdb.Save(this.pdbFile, null, this.WixVariableResolver, this.TempFilesLocation);
            }

            // add any LayoutDirectory/LayoutFile content...
            ArrayList fileTransfers = new ArrayList();
            string layoutDirectory = Path.GetDirectoryName(bundleFile);
            ProcessLayoutDirectories(this.core, bundle, fileTransfers, layoutDirectory);

            // layout media
            this.core.OnMessage(WixVerboses.LayingOutMedia());
            this.LayoutMedia(fileTransfers, this.suppressAclReset);

            return !this.core.EncounteredError;
        }

        private void GenerateWixPackageProperties(Output bundle, List<ChainPackageInfo> chainPackages)
        {
            Table wixPackagePropertiesTable = bundle.EnsureTable(this.core.TableDefinitions["WixPackageProperties"]);

            foreach (ChainPackageInfo package in chainPackages)
            {
                string packageSize = package.PackagePayload.FileSize.ToString(CultureInfo.InvariantCulture);

                Row row = wixPackagePropertiesTable.CreateRow(null);
                row[0] = package.Id;
                row[1] = package.Vital ? "yes" : "no";
                row[2] = package.DisplayName;
                row[3] = package.Description;
                row[4] = packageSize; // TODO: DownloadSize (compressed) (what does this mean when it's embedded?)
                row[5] = packageSize; // PackageSize (uncompressed)
                row[6] = packageSize; // TODO: InstallSize (required disk space)
            }
        }

        private void GetChainInformation(List<ChainPackageInfo> chainPackages, ref bool perMachineBundle, ref RegistrationInfo registrationInfo)
        {
            foreach (ChainPackageInfo package in chainPackages)
            {
                if (perMachineBundle && !package.PerMachine)
                {
                    this.core.OnMessage(WixVerboses.SwitchingToPerUserPackage(package.PackagePayload.FileInfo.FullName));
                    perMachineBundle = false;
                }

                if (package.RegistrationInfo != null)
                {
                    registrationInfo = package.RegistrationInfo;
                }
            }
        }

        private void CreateContainer(string containerPath, List<PayloadInfo> payloads, string manifestFile)
        {
            int payloadCount = payloads.Count; // The number of embedded payloads
            if (!String.IsNullOrEmpty(manifestFile))
            {
                ++payloadCount;
            }

            using (WixCreateCab cab = new WixCreateCab(Path.GetFileName(containerPath), Path.GetDirectoryName(containerPath), payloadCount, 0, 0, this.defaultCompressionLevel))
            {
                // If a manifest was provided always add it as "payload 0" to the container.
                if (!String.IsNullOrEmpty(manifestFile))
                {
                    cab.AddFile(manifestFile, "0");
                }

                foreach (PayloadInfo payload in payloads)
                {
                    Debug.Assert(PayloadInfo.PackagingType.Embedded == payload.Packaging);
                    this.core.OnMessage(WixVerboses.LoadingPayload(payload.FileInfo.FullName));
                    cab.AddFile(payload.FileInfo.FullName, payload.EmbeddedId);
                }

                cab.Complete();
            }
        }

        private string CreateUxManifest(Output bundle, List<PayloadInfo> uxPayloads)
        {
            string uxManifestPath = Path.Combine(this.TempFilesLocation, "bundle-UxManifest.xml");

            using (XmlTextWriter writer = new XmlTextWriter(uxManifestPath, Encoding.Unicode))
            {
                writer.Formatting = Formatting.Indented;
                writer.WriteStartDocument();
                writer.WriteStartElement("UxManifest", "http://schemas.microsoft.com/wix/2010/UxManifest");

                foreach (Table table in bundle.Tables)
                {
                    if (table.Definition.IsUxManifest && null != table.Rows && 0 < table.Rows.Count)
                    {
                        // We simply assert that the table (and field) name is valid, because
                        // this is up to the extension developer to get right. An author will
                        // only affect the attribute value, and that will get properly escaped.
#if DEBUG
                        Debug.Assert(CompilerCore.IsIdentifier(table.Name));
                        foreach (ColumnDefinition column in table.Definition.Columns)
                        {
                            Debug.Assert(CompilerCore.IsIdentifier(column.Name));
                        }
#endif // DEBUG

                        foreach (Row row in table.Rows)
                        {
                            writer.WriteStartElement(table.Name);

                            foreach (Field field in row.Fields)
                            {
                                if (null != field.Data)
                                {
                                    writer.WriteAttributeString(field.Column.Name, field.Data.ToString());
                                }
                            }

                            writer.WriteEndElement();
                        }
                    }
                }

                writer.WriteEndElement();
                writer.WriteEndDocument();
            }

            // Add the UxManifest file to the set of UX payloads.
            PayloadInfo uxManifestPayload = new PayloadInfo(null, "UxManifest.xml", uxManifestPath, null, this.FileManager);
            uxManifestPayload.Packaging = PayloadInfo.PackagingType.Embedded;
            uxManifestPayload.EmbeddedId = string.Format(CultureInfo.InvariantCulture, Binder.BurnUXContainerEmbeddedIdFormat, uxPayloads.Count);
            uxPayloads.Add(uxManifestPayload);

            return uxManifestPath;
        }

        private string CreateBurnParameterInfo(string bundleFile, Guid bundleGuid, List<VariableInfo> allVariables, List<WixSearchInfo> orderedSearches, List<PayloadInfo> uxPayloads, List<ChainPackageInfo> chainPackages, bool perMachineBundle, RegistrationInfo registrationInfo)
        {
            string parameterInfoPath = Path.Combine(this.TempFilesLocation, "bundle-paraminfo.xml");
            using (XmlTextWriter writer = new XmlTextWriter(parameterInfoPath, Encoding.Unicode))
            {
                writer.Formatting = Formatting.Indented;
                writer.WriteStartDocument();
                writer.WriteStartElement("Setup", "http://schemas.microsoft.com/Setup/2008/01/im");
                writer.WriteAttributeString("SetupVersion", "1.0");

                writer.WriteStartElement("UI");
                // TODO: Write "Name" attribute (rather than "UX") if the bundle has a Name.
                writer.WriteAttributeString("Name", "UX");
                writer.WriteAttributeString("Version", "Ignored");
                writer.WriteAttributeString("Dll", uxPayloads[0].FileName);
                writer.WriteEndElement();

                writer.WriteStartElement("Configuration");
                writer.WriteStartElement("UserExperienceDataCollection");
                writer.WriteAttributeString("Policy", "Disabled");
                writer.WriteEndElement();

                // TODO: Write Certificates...
                ////XmlNodeList certificates = manifest.DocumentElement.SelectNodes("burn:Certificate", namespaceManager);
                ////if (certificates.Count > 0)
                ////{
                ////    writer.WriteStartElement("AcceptableCertificates");
                ////    foreach (XmlElement certificate in certificates)
                ////    {
                ////        Burn.CopyElement(certificate, writer);
                ////    }
                ////    writer.WriteEndElement();
                ////}
                writer.WriteEndElement();

                // Write the variables
                foreach (VariableInfo variable in allVariables)
                {
                    variable.WriteXml(writer);
                }

                // Write the searches
                foreach (WixSearchInfo searchinfo in orderedSearches)
                {
                    searchinfo.WriteXml(writer);
                }

                writer.WriteStartElement("EnterMaintenanceModeIf");
                writer.WriteStartElement("NeverTrue");
                writer.WriteEndElement();
                writer.WriteEndElement();

                // Create the Ux element.
                int uxPayloadIndex = 0;
                writer.WriteStartElement("Ux");
                writer.WriteAttributeString("UxDllPayloadId", String.Format(CultureInfo.InvariantCulture, Binder.BurnUXContainerPayloadIdFormat, uxPayloadIndex));

                for (uxPayloadIndex = 0; uxPayloadIndex < uxPayloads.Count; ++uxPayloadIndex)
                {
                    PayloadInfo payload = uxPayloads[uxPayloadIndex];
                    writer.WriteStartElement("Resource");
                    writer.WriteAttributeString("Id", String.Format(CultureInfo.InvariantCulture, Binder.BurnUXContainerPayloadIdFormat, uxPayloadIndex));
                    writer.WriteAttributeString("FilePath", payload.FileName);
                    writer.WriteAttributeString("FileSize", payload.FileSize.ToString(CultureInfo.InvariantCulture));
                    writer.WriteAttributeString("Sha1Hash", payload.Sha1);

                    Debug.Assert(PayloadInfo.PackagingType.Embedded == payload.Packaging);
                    writer.WriteAttributeString("Packaging", "embedded");
                    writer.WriteAttributeString("SourcePath", payload.EmbeddedId);

                    writer.WriteEndElement();
                }

                writer.WriteEndElement();

                // Create the Registration element and Arp element if appropriate.
                writer.WriteStartElement("Registration");
                writer.WriteAttributeString("Id", bundleGuid.ToString("B"));
                writer.WriteAttributeString("PerMachine", perMachineBundle ? "yes" : "no");
                writer.WriteAttributeString("ExecutableName", Path.GetFileName(bundleFile));

                // TODO: add this to Bundle schema so that it can be configured by users.
                // Setting the default here to a random GUID at least allows people to recover and use this later
                writer.WriteAttributeString("Family", Guid.NewGuid().ToString("B"));

                if (null != registrationInfo)
                {
                    writer.WriteStartElement("Arp");

                    // skip this attribute because it is on the registration element instead.
                    //writer.WriteAttributeString("PerMachine", perMachineBundle ? "yes" : "no");
                    writer.WriteAttributeString("DisplayName", registrationInfo.Name);
                    writer.WriteAttributeString("Publisher", registrationInfo.Publisher);
                    writer.WriteAttributeString("HelpLink", registrationInfo.HelpLink);
                    writer.WriteAttributeString("HelpTelephone", registrationInfo.HelpTelephone);
                    writer.WriteAttributeString("AboutUrl", registrationInfo.AboutUrl);
                    writer.WriteAttributeString("UpdateUrl", registrationInfo.UpdateUrl);

                    if (registrationInfo.DisableModify)
                    {
                        writer.WriteAttributeString("DisableModify", "yes");
                    }

                    if (registrationInfo.DisableRepair)
                    {
                        writer.WriteAttributeString("DisableRepair", "yes");
                    }

                    if (registrationInfo.DisableRemove)
                    {
                        writer.WriteAttributeString("DisableRemove", "yes");
                    }

                    writer.WriteEndElement();
                }
                writer.WriteEndElement();

                // Now add all of the packages.
                writer.WriteStartElement("Items");

                foreach (ChainPackageInfo package in chainPackages)
                {
                    Compiler.ChainPackageType packageType = package.ChainPackageType;

                    string packageTypeName = null;
                    switch (packageType)
                    {
                        case Compiler.ChainPackageType.Msi:
                            packageTypeName = "MSI";
                            break;
                        case Compiler.ChainPackageType.Msp:
                            packageTypeName = "MSP";
                            break;
                        case Compiler.ChainPackageType.Msu:
                            packageTypeName = "Exe";
                            break;
                        case Compiler.ChainPackageType.Exe:
                            packageTypeName = "Exe";
                            break;
                    }

                    Debug.Assert(null != packageTypeName);
                    writer.WriteStartElement(packageTypeName);

                    writer.WriteAttributeString("Id", package.Id);
                    writer.WriteAttributeString("Name", package.PackagePayload.FileName);
                    writer.WriteAttributeString("DownloadSize", package.PackagePayload.FileSize.ToString(CultureInfo.InvariantCulture));
                    writer.WriteAttributeString("Sha1HashValue", package.PackagePayload.Sha1);
                    writer.WriteAttributeString("InstalledProductSize", package.PackagePayload.FileSize.ToString(CultureInfo.InvariantCulture));
                    writer.WriteAttributeString("EstimatedInstallTime", "1");   // TODO: newburn uses @ProgressWeight, or 1 if not present
                    writer.WriteAttributeString("SystemDriveSize", "0");        // TODO: Burnexe hard-coded this to zero; should we have a value?
                    writer.WriteAttributeString("CanonicalTargetName", package.Id);
                    writer.WriteAttributeString("PerMachine", package.PerMachine ? "true" : "false");
                    writer.WriteAttributeString("Cache", package.Cache ? "true" : "false");
                    writer.WriteAttributeString("CacheId", package.CacheId);

                    // TODO: pass through "Rollback" package attribute, if present...

                    // TODO: Should we write this always, or only when the packaging is download?
                    if (!String.IsNullOrEmpty(package.PackagePayload.DownloadUrl))
                    {
                        writer.WriteAttributeString("URL", package.PackagePayload.DownloadUrl);
                    }

                    // TODO: Where does InstallCondition come in? I don't see this in the old
                    // burnexe code; it may be in the burn manifest only.
                    if (!String.IsNullOrEmpty(package.InstallCondition))
                    {
                        writer.WriteAttributeString("InstallCondition", package.InstallCondition);
                    }

                    switch (packageType)
                    {
                        // TODO: Is this really MSIs only, or MS*s?
                        case Compiler.ChainPackageType.Msi:
                            {
                                writer.WriteAttributeString("ProductCode", package.ProductCode);

                                StringBuilder sb = new StringBuilder();
                                ////foreach (XmlElement msiPropertyElement in packageElement.SelectNodes("burner:MsiProperty", namespaceManager))
                                ////{
                                ////    if (0 < sb.Length)
                                ////    {
                                ////        sb.Append(" ");
                                ////    }
                                ////
                                ////    sb.AppendFormat("{0}=\"{1}\"", msiPropertyElement.Attributes["Id"].Value, msiPropertyElement.Attributes["Value"].Value);
                                ////}

                                if (0 < sb.Length)
                                {
                                    writer.WriteAttributeString("MSIOptions", sb.ToString());
                                }
                            }
                            break;
                        case Compiler.ChainPackageType.Msp:
                            writer.WriteAttributeString("PatchCode", package.PatchCode);
                            break;
                        case Compiler.ChainPackageType.Msu:
                            writer.WriteAttributeString("ExeType", "MsuPackage");
                            break;
                        case Compiler.ChainPackageType.Exe:
                            writer.WriteAttributeString("InstallCommandLine", package.InstallCommand);
                            writer.WriteAttributeString("RepairCommandLine", package.RepairCommand);
                            writer.WriteAttributeString("UninstallCommandLine", package.UninstallCommand);
                            break;
                    }

                    // MsiProperty/MSIOptions really only apply to MsiPackages, but we trust the Compiler
                    // to ensure that this is the case. All other package types should have an empty
                    // MsiProperties list.
                    StringBuilder msiOptions = new StringBuilder();
                    foreach (MsiPropertyInfo msiProperty in package.MsiProperties)
                    {
                        if (0 < msiOptions.Length)
                        {
                            msiOptions.Append(" ");
                        }
                        // TODO: The original burnexe code didn't do anything to ensure that the name
                        // or value was escaped. (For example, what if the value already includes a
                        // quotation character?) We're keeping the same behavior for consistency, but
                        // this should probably be re-examined.
                        msiOptions.AppendFormat("{0}=\"{1}\"", msiProperty.Name, msiProperty.Value);
                    }

                    if (0 < msiOptions.Length)
                    {
                        writer.WriteAttributeString("MSIOptions", msiOptions.ToString());
                    }

                    writer.WriteStartElement("IsPresent");
                    if (Compiler.ChainPackageType.Msi == packageType)
                    {
                        writer.WriteStartElement("Exists");
                        writer.WriteStartElement("MsiProductVersion");
                        writer.WriteAttributeString("ProductCode", "Self");
                        writer.WriteEndElement();
                        writer.WriteEndElement();
                    }
                    else // all non-Msis (Msp & Exe)
                    {
                        // TODO: this will cause executables to never be "installed". Need to use the detect condition instead.
                        writer.WriteStartElement("NeverTrue");
                        writer.WriteEndElement();
                    }
                    writer.WriteEndElement();

                    writer.WriteStartElement("ApplicableIf");
                    if (Compiler.ChainPackageType.Msp == packageType)
                    {
                        // TODO: Convert this code from burnexe...
                        ////PayloadInfo payload = this.allPayloads[packageElement.GetAttribute("FileName")];
                        ////string patchXml = Installer.ExtractPatchXmlData(payload.FileInfo.FullName);

                        ////writer.WriteStartElement("Exists");
                        ////writer.WriteElementString("MsiXmlBlob", patchXml);
                        ////writer.WriteEndElement();
                    }
                    else
                    {
                        writer.WriteStartElement("AlwaysTrue");
                        writer.WriteEndElement();
                    }
                    writer.WriteEndElement();

                    writer.WriteStartElement("ActionTable");

                    writer.WriteStartElement("InstallAction");
                    if (package.Vital)
                    {
                        writer.WriteAttributeString("OnFailureBehavior", "Rollback");
                    }
                    writer.WriteAttributeString("IfPresent", "noop");
                    writer.WriteAttributeString("IfAbsent", "install");
                    writer.WriteEndElement();

                    writer.WriteStartElement("UninstallAction");
                    writer.WriteAttributeString("IfPresent", "uninstall");
                    writer.WriteAttributeString("IfAbsent", "noop");
                    writer.WriteEndElement();

                    writer.WriteStartElement("RepairAction");
                    writer.WriteAttributeString("IfPresent", "repair");
                    writer.WriteAttributeString("IfAbsent", "install");
                    writer.WriteEndElement();

                    writer.WriteEndElement();

                    // Include all "resource" payloads within the package.
                    int payloadCount = 0;
                    foreach (PayloadInfo payload in package.Payloads)
                    {
                        writer.WriteStartElement("File");
                        writer.WriteAttributeString("Id", String.Concat(package.Id, "_p", payloadCount));
                        writer.WriteAttributeString("Name", payload.FileName);
                        writer.WriteAttributeString("DownloadSize", payload.FileSize.ToString(CultureInfo.InvariantCulture));
                        writer.WriteAttributeString("Sha1HashValue", payload.Sha1);
                        writer.WriteAttributeString("InstalledProductSize", "0");
                        writer.WriteAttributeString("SystemDriveSize", "0");
                        writer.WriteAttributeString("Cache", package.Cache ? "true" : "false");

                        if (!String.IsNullOrEmpty(payload.DownloadUrl))
                        {
                            writer.WriteAttributeString("URL", payload.DownloadUrl);
                        }

                        writer.WriteEndElement();

                        ++payloadCount;
                    }

                    writer.WriteEndElement();
                }

                writer.WriteEndElement();

                writer.WriteEndDocument();
            }

            // Notice that the ParameterInfo.xml's "Ux" element above does not include a reference
            // for itself. It is impossible to get the SHA1 hash for the ParameterInfo.xml then update the
            // ParameterInfo.xml with data about itself. So only the burner.manifest will know about the
            // ParameterInfo.xml file.
            PayloadInfo parameterInfoPayload = new PayloadInfo(null, "ParameterInfo.xml", parameterInfoPath, null, this.FileManager);
            parameterInfoPayload.Packaging = PayloadInfo.PackagingType.Embedded;
            parameterInfoPayload.EmbeddedId = String.Format(CultureInfo.InvariantCulture, Binder.BurnUXContainerEmbeddedIdFormat, uxPayloads.Count);
            uxPayloads.Add(parameterInfoPayload);

            return parameterInfoPath;
        }

        private string CreateBurnManifest(List<VariableInfo> allVariables, List<WixSearchInfo> orderedSearches, List<PayloadInfo> uxPayloads, List<ChainPackageInfo> chainPackages, bool perMachineBundle, RegistrationInfo registrationInfo)
        {
            string manifestPath = Path.Combine(this.TempFilesLocation, "bundle-manifest.xml");
            this.core.OnMessage(WixVerboses.ResolvingManifest(manifestPath));
            using (XmlTextWriter writer = new XmlTextWriter(manifestPath, Encoding.UTF8))
            {
                writer.WriteStartDocument();

                writer.WriteStartElement("BurnManifest", Binder.BurnNamespace);

                // Write the variables
                foreach (VariableInfo variable in allVariables)
                {
                    variable.WriteXml(writer);
                }

                // Write the searches
                foreach (WixSearchInfo searchinfo in orderedSearches)
                {
                    searchinfo.WriteXml(writer);
                }

                // write the first UX payload / entrypoint
                int uxPayloadIndex = 0;
                writer.WriteStartElement("UX");
                WriteBurnManifestPayloadAttributes(writer, uxPayloads[uxPayloadIndex], true);
                // TODO: Write "Name" attribute to UX if the bundle has a Name.

                // write the remaining UX allPayloads...
                for (uxPayloadIndex = 1; uxPayloadIndex < uxPayloads.Count; ++uxPayloadIndex)
                {
                    writer.WriteStartElement("Resource");
                    WriteBurnManifestPayloadAttributes(writer, uxPayloads[uxPayloadIndex], true);
                    writer.WriteEndElement();
                }

                writer.WriteEndElement();

                // Write the registration information...
                if (registrationInfo != null)
                {
                    writer.WriteStartElement("Registration");

                    writer.WriteAttributeString("PerMachine", perMachineBundle ? "yes" : "no");
                    writer.WriteAttributeString("DisplayName", registrationInfo.Name);
                    writer.WriteAttributeString("Publisher", registrationInfo.Publisher);
                    writer.WriteAttributeString("HelpLink", registrationInfo.HelpLink);
                    writer.WriteAttributeString("HelpTelephone", registrationInfo.HelpTelephone);
                    writer.WriteAttributeString("AboutUrl", registrationInfo.AboutUrl);
                    writer.WriteAttributeString("UpdateUrl", registrationInfo.UpdateUrl);

                    if (registrationInfo.DisableModify)
                    {
                        writer.WriteAttributeString("DisableModify", "yes");
                    }

                    if (registrationInfo.DisableRepair)
                    {
                        writer.WriteAttributeString("DisableRepair", "yes");
                    }

                    if (registrationInfo.DisableRemove)
                    {
                        writer.WriteAttributeString("DisableRemove", "yes");
                    }

                    writer.WriteEndElement();
                }

                // write the Chain...
                writer.WriteStartElement("Chain");
                foreach (ChainPackageInfo package in chainPackages)
                {
                    writer.WriteStartElement(String.Format(CultureInfo.InvariantCulture, "{0}Package", package.ChainPackageType));

                    WriteBurnManifestPayloadAttributes(writer, package.PackagePayload, false);

                    writer.WriteAttributeString("Register", package.Register ? "yes" : "no");
                    writer.WriteAttributeString("PerMachine", package.PerMachine ? "yes" : "no");
                    writer.WriteAttributeString("ProductCode", package.ProductCode);
                    writer.WriteAttributeString("Cache", package.Cache ? "yes" : "no");
                    writer.WriteAttributeString("Uninstall", package.Uninstall ? "yes" : "no");

                    if (!String.IsNullOrEmpty(package.InstallCondition))
                    {
                        writer.WriteAttributeString("InstallCondition", package.InstallCondition);
                    }

                    switch (package.PackagePayload.Packaging)
                    {
                        case PayloadInfo.PackagingType.Embedded:
                            writer.WriteAttributeString("Packaging", "embedded");
                            writer.WriteAttributeString("SourcePath", package.PackagePayload.EmbeddedId);
                            break;
                        case PayloadInfo.PackagingType.Download:
                            writer.WriteAttributeString("Packaging", "download");
                            // DownloadUrl already written as a part of WriteBurnManifestPayloadAttributes().
                            break;
                        case PayloadInfo.PackagingType.External:
                            writer.WriteAttributeString("Packaging", "external");
                            writer.WriteAttributeString("SourcePath", package.PackagePayload.FileName);
                            break;
                    }

                    if (Compiler.ChainPackageType.Exe == package.ChainPackageType)
                    {
                        writer.WriteAttributeString("InstallArguments", package.InstallCommand);
                        writer.WriteAttributeString("RepairArguments", package.RepairCommand);
                        writer.WriteAttributeString("UninstallArguments", package.UninstallCommand);
                    }

                    foreach (MsiPropertyInfo msiProperty in package.MsiProperties)
                    {
                        writer.WriteStartElement("MsiProperty");
                        writer.WriteAttributeString("Id", msiProperty.Name);
                        writer.WriteAttributeString("Value", msiProperty.Value);
                        writer.WriteEndElement();
                    }

                    // Write any contained Payloads as "burn:Resource"...
                    foreach (PayloadInfo payload in package.Payloads)
                    {
                        writer.WriteStartElement("Resource");
                        WriteBurnManifestPayloadAttributes(writer, payload, false);
                        writer.WriteEndElement();
                    }

                    writer.WriteEndElement();
                }
                writer.WriteEndElement();

                writer.WriteEndDocument();
            }

            return manifestPath;
        }

        private static void WriteBurnManifestPayloadAttributes(XmlTextWriter writer, PayloadInfo payload, bool embeddedOnly)
        {
            writer.WriteAttributeString("FileName", payload.FileName);
            writer.WriteAttributeString("FileSize", payload.FileSize.ToString(CultureInfo.InvariantCulture));
            writer.WriteAttributeString("SHA1", payload.Sha1);

            Debug.Assert(!embeddedOnly || PayloadInfo.PackagingType.Embedded == payload.Packaging);

            if (PayloadInfo.PackagingType.Embedded == payload.Packaging)
            {
                writer.WriteAttributeString("EmbeddedId", payload.EmbeddedId);
            }

            // TODO: Should we only write the download URL if the packaging is "download"?
            if (!embeddedOnly && !String.IsNullOrEmpty(payload.DownloadUrl))
            {
                writer.WriteAttributeString("DownloadUrl", payload.DownloadUrl);
            }
        }

        /// <summary>
        /// Binds a transform.
        /// </summary>
        /// <param name="transform">The transform to bind.</param>
        /// <param name="transformFile">The transform to create.</param>
        /// <returns>true if binding completed successfully; false otherwise</returns>
        private bool BindTransform(Output transform, string transformFile)
        {
            foreach (BinderExtension extension in this.extensions)
            {
                extension.TransformInitialize(transform);
            }

            int transformFlags = 0;

            Output targetOutput = new Output(null);
            Output updatedOutput = new Output(null);

            // TODO: handle added columns

            // to generate a localized transform, both the target and updated
            // databases need to have the same code page. the only reason to
            // set different code pages is to support localized primary key
            // columns, but that would only support deleting rows. if this
            // becomes necessary, define a PreviousCodepage property on the
            // Output class and persist this throughout transform generation.
            targetOutput.Codepage = transform.Codepage;
            updatedOutput.Codepage = transform.Codepage;

            // remove certain Property rows which will be populated from summary information values
            string targetUpgradeCode = null;
            string updatedUpgradeCode = null;

            Table propertyTable = transform.Tables["Property"];
            if (null != propertyTable)
            {
                for (int i = propertyTable.Rows.Count - 1; i >= 0; i--)
                {
                    Row row = propertyTable.Rows[i];

                    if ("ProductCode" == (string)row[0] || "ProductLanguage" == (string)row[0] || "ProductVersion" == (string)row[0] || "UpgradeCode" == (string)row[0])
                    {
                        propertyTable.Rows.RemoveAt(i);

                        if ("UpgradeCode" == (string)row[0])
                        {
                            updatedUpgradeCode = (string)row[1];
                        }
                    }
                }
            }

            Table targetSummaryInfo = targetOutput.EnsureTable(this.core.TableDefinitions["_SummaryInformation"]);
            Table updatedSummaryInfo = updatedOutput.EnsureTable(this.core.TableDefinitions["_SummaryInformation"]);
            Table targetPropertyTable = targetOutput.EnsureTable(this.core.TableDefinitions["Property"]);
            Table updatedPropertyTable = updatedOutput.EnsureTable(this.core.TableDefinitions["Property"]);

            // process special summary information values
            foreach (Row row in transform.Tables["_SummaryInformation"].Rows)
            {
                if ((int)SummaryInformation.Transform.CodePage == (int)row[0])
                {
                    // convert from a web name if provided
                    string codePage = (string)row.Fields[1].Data;
                    if (null == codePage)
                    {
                        codePage = "0";
                    }
                    else
                    {
                        codePage = Common.GetValidCodePage(codePage).ToString(CultureInfo.InvariantCulture);
                    }

                    string previousCodePage = (string)row.Fields[1].PreviousData;
                    if (null == previousCodePage)
                    {
                        previousCodePage = "0";
                    }
                    else
                    {
                        previousCodePage = Common.GetValidCodePage(previousCodePage).ToString(CultureInfo.InvariantCulture);
                    }

                    Row targetCodePageRow = targetSummaryInfo.CreateRow(null);
                    targetCodePageRow[0] = 1; // PID_CODEPAGE
                    targetCodePageRow[1] = previousCodePage;

                    Row updatedCodePageRow = updatedSummaryInfo.CreateRow(null);
                    updatedCodePageRow[0] = 1; // PID_CODEPAGE
                    updatedCodePageRow[1] = codePage;
                }
                else if ((int)SummaryInformation.Transform.TargetPlatformAndLanguage == (int)row[0] ||
                         (int)SummaryInformation.Transform.UpdatedPlatformAndLanguage == (int)row[0])
                {
                    // the target language
                    string[] propertyData = ((string)row[1]).Split(';');
                    string lang = 2 == propertyData.Length ? propertyData[1] : "0";

                    Table tempSummaryInfo = (int)SummaryInformation.Transform.TargetPlatformAndLanguage == (int)row[0] ? targetSummaryInfo : updatedSummaryInfo;
                    Table tempPropertyTable = (int)SummaryInformation.Transform.TargetPlatformAndLanguage == (int)row[0] ? targetPropertyTable : updatedPropertyTable;

                    Row productLanguageRow = tempPropertyTable.CreateRow(null);
                    productLanguageRow[0] = "ProductLanguage";
                    productLanguageRow[1] = lang;

                    // set the platform;language on the MSI to be generated
                    Row templateRow = tempSummaryInfo.CreateRow(null);
                    templateRow[0] = 7; // PID_TEMPLATE
                    templateRow[1] = (string)row[1];
                }
                else if ((int)SummaryInformation.Transform.ProductCodes == (int)row[0])
                {
                    string[] propertyData = ((string)row[1]).Split(';');

                    Row targetProductCodeRow = targetPropertyTable.CreateRow(null);
                    targetProductCodeRow[0] = "ProductCode";
                    targetProductCodeRow[1] = propertyData[0].Substring(0, 38);

                    Row targetProductVersionRow = targetPropertyTable.CreateRow(null);
                    targetProductVersionRow[0] = "ProductVersion";
                    targetProductVersionRow[1] = propertyData[0].Substring(38);

                    Row updatedProductCodeRow = updatedPropertyTable.CreateRow(null);
                    updatedProductCodeRow[0] = "ProductCode";
                    updatedProductCodeRow[1] = propertyData[1].Substring(0, 38);

                    Row updatedProductVersionRow = updatedPropertyTable.CreateRow(null);
                    updatedProductVersionRow[0] = "ProductVersion";
                    updatedProductVersionRow[1] = propertyData[1].Substring(38);

                    // UpgradeCode is optional and may not exists in the target
                    // or upgraded databases, so do not include a null-valued
                    // UpgradeCode property.

                    targetUpgradeCode = propertyData[2];
                    if (!String.IsNullOrEmpty(targetUpgradeCode))
                    {
                        Row targetUpgradeCodeRow = targetPropertyTable.CreateRow(null);
                        targetUpgradeCodeRow[0] = "UpgradeCode";
                        targetUpgradeCodeRow[1] = targetUpgradeCode;

                        // If the target UpgradeCode is specified, an updated
                        // UpgradeCode is required.
                        if (String.IsNullOrEmpty(updatedUpgradeCode))
                        {
                            updatedUpgradeCode = targetUpgradeCode;
                        }
                    }

                    if (!String.IsNullOrEmpty(updatedUpgradeCode))
                    {
                        Row updatedUpgradeCodeRow = updatedPropertyTable.CreateRow(null);
                        updatedUpgradeCodeRow[0] = "UpgradeCode";
                        updatedUpgradeCodeRow[1] = updatedUpgradeCode;
                    }
                }
                else if ((int)SummaryInformation.Transform.ValidationFlags == (int)row[0])
                {
                    transformFlags = Convert.ToInt32(row[1], CultureInfo.InvariantCulture);
                }
                else
                {
                    // add everything else as is
                    Row targetRow = targetSummaryInfo.CreateRow(null);
                    targetRow[0] = row[0];
                    targetRow[1] = row[1];

                    Row updatedRow = updatedSummaryInfo.CreateRow(null);
                    updatedRow[0] = row[0];
                    updatedRow[1] = row[1];
                }
            }

            // Validate that both databases have an UpgradeCode if the
            // authoring transform will validate the UpgradeCode; otherwise,
            // MsiCreateTransformSummaryinfo() will fail with 1620.
            if (((int)TransformFlags.ValidateUpgradeCode & transformFlags) != 0 &&
                (String.IsNullOrEmpty(targetUpgradeCode) || String.IsNullOrEmpty(updatedUpgradeCode)))
            {
                this.core.OnMessage(WixErrors.BothUpgradeCodesRequired());
            }

            foreach (Table table in transform.Tables)
            {
                // Ignore unreal tables when building transforms except the _Stream table.
                // These tables are ignored when generating the database so there is no reason
                // to process them here.
                if (table.Definition.IsUnreal && "_Streams" != table.Name)
                {
                    continue;
                }

                // process table operations
                switch (table.Operation)
                {
                    case TableOperation.Add:
                        updatedOutput.EnsureTable(table.Definition);
                        break;
                    case TableOperation.Drop:
                        targetOutput.EnsureTable(table.Definition);
                        continue;
                    default:
                        targetOutput.EnsureTable(table.Definition);
                        updatedOutput.EnsureTable(table.Definition);
                        break;
                }

                // process row operations
                foreach (Row row in table.Rows)
                {
                    switch (row.Operation)
                    {
                        case RowOperation.Add:
                            Table updatedTable = updatedOutput.EnsureTable(table.Definition);
                            updatedTable.Rows.Add(row);
                            continue;
                        case RowOperation.Delete:
                            Table targetTable = targetOutput.EnsureTable(table.Definition);
                            targetTable.Rows.Add(row);

                            // fill-in non-primary key values
                            foreach (Field field in row.Fields)
                            {
                                if (!field.Column.IsPrimaryKey)
                                {
                                    if (ColumnType.Number == field.Column.Type && !field.Column.IsLocalizable)
                                    {
                                        field.Data = field.Column.MinValue;
                                    }
                                    else if (ColumnType.Object == field.Column.Type)
                                    {
                                        if (null == this.emptyFile)
                                        {
                                            this.emptyFile = this.tempFiles.AddExtension("empty");
                                            using (FileStream fileStream = File.Create(this.emptyFile))
                                            {
                                            }
                                        }

                                        field.Data = emptyFile;
                                    }
                                    else
                                    {
                                        field.Data = "0";
                                    }
                                }
                            }
                            continue;
                    }

                    // Assure that the file table's sequence is populated
                    if ("File" == table.Name)
                    {
                        foreach (Row fileRow in table.Rows)
                        {
                            if (null == fileRow[7])
                            {
                                if (RowOperation.Add == fileRow.Operation)
                                {
                                    this.core.OnMessage(WixErrors.InvalidAddedFileRowWithoutSequence(fileRow.SourceLineNumbers, (string)fileRow[0]));
                                    break;
                                }

                                // Set to 1 to prevent invalid IDT file from being generated
                                fileRow[7] = 1;
                            }
                        }
                    }

                    // process modified and unmodified rows
                    bool modifiedRow = false;
                    Row targetRow = new Row(null, table.Definition);
                    Row updatedRow = row;
                    for (int i = 0; i < row.Fields.Length; i++)
                    {
                        Field updatedField = row.Fields[i];

                        if (updatedField.Modified)
                        {
                            // set a different value in the target row to ensure this value will be modified during transform generation
                            if (ColumnType.Number == updatedField.Column.Type && !updatedField.Column.IsLocalizable)
                            {
                                if (null == updatedField.Data || 1 != (int)updatedField.Data)
                                {
                                    targetRow[i] = 1;
                                }
                                else
                                {
                                    targetRow[i] = 2;
                                }
                            }
                            else if (ColumnType.Object == updatedField.Column.Type)
                            {
                                if (null == emptyFile)
                                {
                                    emptyFile = this.tempFiles.AddExtension("empty");
                                    using (FileStream fileStream = File.Create(emptyFile))
                                    {
                                    }
                                }

                                targetRow[i] = emptyFile;
                            }
                            else
                            {
                                if ("0" != (string)updatedField.Data)
                                {
                                    targetRow[i] = "0";
                                }
                                else
                                {
                                    targetRow[i] = "1";
                                }
                            }

                            modifiedRow = true;
                        }
                        else if (ColumnType.Object == updatedField.Column.Type)
                        {
                            ObjectField objectField = (ObjectField)updatedField;

                            // create an empty file for comparing against
                            if (null == objectField.PreviousData)
                            {
                                if (null == emptyFile)
                                {
                                    emptyFile = this.tempFiles.AddExtension("empty");
                                    using (FileStream fileStream = File.Create(emptyFile))
                                    {
                                    }
                                }

                                targetRow[i] = emptyFile;
                                modifiedRow = true;
                            }
                            else if (!this.FileManager.CompareFiles(objectField.PreviousData, (string)objectField.Data))
                            {
                                targetRow[i] = objectField.PreviousData;
                                modifiedRow = true;
                            }
                        }
                        else // unmodified
                        {
                            if (null != updatedField.Data)
                            {
                                targetRow[i] = updatedField.Data;
                            }
                        }
                    }

                    // modified rows and certain special rows go in the target and updated msi databases
                    if (modifiedRow ||
                        ("Property" == table.Name &&
                            ("ProductCode" == (string)row[0] ||
                            "ProductLanguage" == (string)row[0] ||
                            "ProductVersion" == (string)row[0] ||
                            "UpgradeCode" == (string)row[0])))
                    {
                        Table targetTable = targetOutput.EnsureTable(table.Definition);
                        targetTable.Rows.Add(targetRow);

                        Table updatedTable = updatedOutput.EnsureTable(table.Definition);
                        updatedTable.Rows.Add(updatedRow);
                    }
                }
            }

            foreach (BinderExtension extension in this.extensions)
            {
                extension.TransformFinalize(transform);
            }

            // Any errors encountered up to this point can cause errors during generation.
            if (this.core.EncounteredError)
            {
                return false;
            }

            string transformFileName = Path.GetFileNameWithoutExtension(transformFile);
            string targetDatabaseFile = Path.Combine(this.TempFilesLocation, String.Concat(transformFileName, "_target.msi"));
            string updatedDatabaseFile = Path.Combine(this.TempFilesLocation, String.Concat(transformFileName, "_updated.msi"));

            this.suppressAddingValidationRows = true;
            this.GenerateDatabase(targetOutput, targetDatabaseFile, false, true);
            this.GenerateDatabase(updatedOutput, updatedDatabaseFile, true, true);

            // make sure the directory exists
            Directory.CreateDirectory(Path.GetDirectoryName(transformFile));

            // create the transform file
            using (Database targetDatabase = new Database(targetDatabaseFile, OpenDatabase.ReadOnly))
            {
                using (Database updatedDatabase = new Database(updatedDatabaseFile, OpenDatabase.ReadOnly))
                {
                    if (!updatedDatabase.GenerateTransform(targetDatabase, transformFile))
                    {
                        throw new WixException(WixErrors.NoDifferencesInTransform(transform.SourceLineNumbers));
                    }

                    updatedDatabase.CreateTransformSummaryInfo(targetDatabase, transformFile, (TransformErrorConditions)(transformFlags & 0xFFFF), (TransformValidations)((transformFlags >> 16) & 0xFFFF));
                }
            }

            return !this.core.EncounteredError;
        }

        /// <summary>
        /// Retrieve files and their information from merge modules.
        /// </summary>
        /// <param name="output">Internal representation of the msi database to operate upon.</param>
        /// <param name="fileRows">The indexed file rows.</param>
        private void ProcessMergeModules(Output output, FileRowCollection fileRows)
        {
            Table wixMergeTable = output.Tables["WixMerge"];
            if (null != wixMergeTable)
            {
                IMsmMerge2 merge = NativeMethods.GetMsmMerge();

                // Get the output's minimum installer version
                int outputInstallerVersion = int.MinValue;
                Table summaryInformationTable = output.Tables["_SummaryInformation"];
                if (null != summaryInformationTable)
                {
                    foreach (Row row in summaryInformationTable.Rows)
                    {
                        if (14 == (int)row[0])
                        {
                            outputInstallerVersion = Convert.ToInt32(row[1], CultureInfo.InvariantCulture);
                            break;
                        }
                    }
                }

                foreach (Row row in wixMergeTable.Rows)
                {
                    bool containsFiles = false;
                    WixMergeRow wixMergeRow = (WixMergeRow)row;

                    try
                    {
                        // read the module's File table to get its FileMediaInformation entries and gather any other information needed from the module.
                        using (Database db = new Database(wixMergeRow.SourceFile, OpenDatabase.ReadOnly))
                        {
                            if (db.TableExists("File") && db.TableExists("Component"))
                            {
                                Hashtable uniqueModuleFileIdentifiers = System.Collections.Specialized.CollectionsUtil.CreateCaseInsensitiveHashtable();

                                using (View view = db.OpenExecuteView("SELECT `File`, `Directory_` FROM `File`, `Component` WHERE `Component_`=`Component`"))
                                {
                                    Record record;

                                    // add each file row from the merge module into the file row collection (check for errors along the way)
                                    while (null != (record = view.Fetch()))
                                    {
                                        // NOTE: this is very tricky - the merge module file rows are not added to the
                                        // file table because they should not be created via idt import.  Instead, these
                                        // rows are created by merging in the actual modules
                                        FileRow fileRow = new FileRow(null, this.core.TableDefinitions["File"]);
                                        fileRow.File = record[1];
                                        fileRow.Compressed = wixMergeRow.FileCompression;
                                        fileRow.Directory = record[2];
                                        fileRow.DiskId = wixMergeRow.DiskId;
                                        fileRow.FromModule = true;
                                        fileRow.PatchGroup = -1;
                                        fileRow.Source = String.Concat(this.TempFilesLocation, Path.DirectorySeparatorChar, "MergeId.", wixMergeRow.Number.ToString(CultureInfo.InvariantCulture.NumberFormat), Path.DirectorySeparatorChar, record[1]);

                                        FileRow collidingFileRow = fileRows[fileRow.File];
                                        FileRow collidingModuleFileRow = (FileRow)uniqueModuleFileIdentifiers[fileRow.File];

                                        if (null == collidingFileRow && null == collidingModuleFileRow)
                                        {
                                            fileRows.Add(fileRow);

                                            // keep track of file identifiers in this merge module
                                            uniqueModuleFileIdentifiers.Add(fileRow.File, fileRow);
                                        }
                                        else // collision(s) detected
                                        {
                                            // case-sensitive collision with another merge module or a user-authored file identifier
                                            if (null != collidingFileRow)
                                            {
                                                this.core.OnMessage(WixErrors.DuplicateModuleFileIdentifier(wixMergeRow.SourceLineNumbers, wixMergeRow.Id, collidingFileRow.File));
                                            }

                                            // case-insensitive collision with another file identifier in the same merge module
                                            if (null != collidingModuleFileRow)
                                            {
                                                this.core.OnMessage(WixErrors.DuplicateModuleCaseInsensitiveFileIdentifier(wixMergeRow.SourceLineNumbers, wixMergeRow.Id, fileRow.File, collidingModuleFileRow.File));
                                            }
                                        }

                                        containsFiles = true;
                                    }
                                }
                            }

                            // Get the summary information to detect the Schema
                            using (SummaryInformation summaryInformation = new SummaryInformation(db))
                            {
                                int moduleInstallerVersion = Convert.ToInt32(summaryInformation.GetProperty(14), CultureInfo.InvariantCulture);
                                if (moduleInstallerVersion > outputInstallerVersion)
                                {
                                    this.core.OnMessage(WixWarnings.InvalidHigherInstallerVersionInModule(wixMergeRow.SourceLineNumbers, wixMergeRow.Id, moduleInstallerVersion, outputInstallerVersion));
                                }
                            }
                        }
                    }
                    catch (FileNotFoundException)
                    {
                        throw new WixException(WixErrors.FileNotFound(wixMergeRow.SourceLineNumbers, wixMergeRow.SourceFile));
                    }
                    catch (Win32Exception)
                    {
                        throw new WixException(WixErrors.CannotOpenMergeModule(wixMergeRow.SourceLineNumbers, wixMergeRow.Id, wixMergeRow.SourceFile));
                    }

                    // if the module has files and creating layout
                    if (containsFiles && !this.suppressLayout)
                    {
                        bool moduleOpen = false;
                        short mergeLanguage;

                        try
                        {
                            mergeLanguage = Convert.ToInt16(wixMergeRow.Language, CultureInfo.InvariantCulture);
                        }
                        catch (System.FormatException)
                        {
                            this.core.OnMessage(WixErrors.InvalidMergeLanguage(wixMergeRow.SourceLineNumbers, wixMergeRow.Id, wixMergeRow.Language));
                            continue;
                        }

                        try
                        {
                            merge.OpenModule(wixMergeRow.SourceFile, mergeLanguage);
                            moduleOpen = true;

                            string safeMergeId = wixMergeRow.Number.ToString(CultureInfo.InvariantCulture.NumberFormat);

                            // extract the module cabinet, then explode all of the files to a temp directory
                            string moduleCabPath = String.Concat(this.TempFilesLocation, Path.DirectorySeparatorChar, safeMergeId, ".module.cab");
                            merge.ExtractCAB(moduleCabPath);

                            string mergeIdPath = String.Concat(this.TempFilesLocation, Path.DirectorySeparatorChar, "MergeId.", safeMergeId);
                            Directory.CreateDirectory(mergeIdPath);

                            using (WixExtractCab extractCab = new WixExtractCab())
                            {
                                try
                                {
                                    extractCab.Extract(moduleCabPath, mergeIdPath);
                                }
                                catch (FileNotFoundException)
                                {
                                    throw new WixException(WixErrors.CabFileDoesNotExist(moduleCabPath, wixMergeRow.SourceFile, mergeIdPath));
                                }
                                catch
                                {
                                    throw new WixException(WixErrors.CabExtractionFailed(moduleCabPath, wixMergeRow.SourceFile, mergeIdPath));
                                }
                            }
                        }
                        catch (COMException ce)
                        {
                            throw new WixException(WixErrors.UnableToOpenModule(wixMergeRow.SourceLineNumbers, wixMergeRow.SourceFile, ce.Message));
                        }
                        finally
                        {
                            if (moduleOpen)
                            {
                                merge.CloseModule();
                            }
                        }
                    }
                }
            }
        }

        /// <summary>
        /// Set the guids for components with generatable guids.
        /// </summary>
        /// <param name="output">Internal representation of the database to operate on.</param>
        [SuppressMessage("Microsoft.Globalization", "CA1308:NormalizeStringsToUppercase", Justification = "Changing the way this string normalizes would result " +
                         "in a change to the way autogenerated GUIDs are generated. Furthermore, there is no security hole here, as the strings won't need to " +
                         "make a round trip")]
        private void SetComponentGuids(Output output)
        {
            // as outlined in RFC 4122, this is our namespace for generating name-based (version 3) UUIDs
            Guid wixComponentGuidNamespace = new Guid("{3064E5C6-FB63-4FE9-AC49-E446A792EFA5}");

            Table componentTable = output.Tables["Component"];
            if (null != componentTable)
            {
                Hashtable registryKeyRows = null;
                Hashtable directories = null;
                Hashtable componentIdGenSeeds = null;
                Hashtable fileRows = null;

                // find components with generatable guids
                foreach (ComponentRow componentRow in componentTable.Rows)
                {
                    // component guid will be generated
                    if ("*" == componentRow.Guid)
                    {
                        if (null == componentRow.KeyPath || componentRow.IsOdbcDataSourceKeyPath)
                        {
                            this.core.OnMessage(WixErrors.IllegalComponentWithAutoGeneratedGuid(componentRow.SourceLineNumbers));
                        }
                        else if (componentRow.IsRegistryKeyPath)
                        {
                            if (null == registryKeyRows)
                            {
                                Table registryTable = output.Tables["Registry"];

                                registryKeyRows = new Hashtable(registryTable.Rows.Count);

                                foreach (Row registryRow in registryTable.Rows)
                                {
                                    registryKeyRows.Add((string)registryRow[0], registryRow);
                                }
                            }

                            Row foundRow = registryKeyRows[componentRow.KeyPath] as Row;

                            string bitness = String.Empty;
                            if (!this.backwardsCompatibleGuidGen && componentRow.Is64Bit)
                            {
                                bitness = "64";
                            }

                            if (null != foundRow)
                            {
                                string regkey = String.Concat(bitness, foundRow[1], "\\", foundRow[2], "\\", foundRow[3]);
                                componentRow.Guid = Uuid.NewUuid(wixComponentGuidNamespace, regkey.ToLower(CultureInfo.InvariantCulture), this.backwardsCompatibleGuidGen).ToString("B").ToUpper(CultureInfo.InvariantCulture);
                            }
                        }
                        else // must be a File KeyPath
                        {
                            // if the directory table hasn't been loaded into an indexed hash
                            // of directory ids to target names do that now.
                            if (null == directories)
                            {
                                Table directoryTable = output.Tables["Directory"];

                                int numDirectoryTableRows = (null != directoryTable) ? directoryTable.Rows.Count : 0;

                                directories = new Hashtable(numDirectoryTableRows);

                                // get the target paths for all directories
                                if (null != directoryTable)
                                {
                                    foreach (Row row in directoryTable.Rows)
                                    {
                                        // if the directory Id already exists, we will skip it here since
                                        // checking for duplicate primary keys is done later when importing tables
                                        // into database
                                        if (directories.ContainsKey(row[0]))
                                        {
                                            continue;
                                        }

                                        string targetName = Installer.GetName((string)row[2], false, true);
                                        directories.Add(row[0], new ResolvedDirectory((string)row[1], targetName));
                                    }
                                }
                            }

                            // if the component id generation seeds have not been indexed
                            // from the WixDirectory table do that now.
                            if (null == componentIdGenSeeds)
                            {
                                Table wixDirectoryTable = output.Tables["WixDirectory"];

                                int numWixDirectoryRows = (null != wixDirectoryTable) ? wixDirectoryTable.Rows.Count : 0;

                                componentIdGenSeeds = new Hashtable(numWixDirectoryRows);

                                // if there are any WixDirectory rows, build up the Component Guid
                                // generation seeds indexed by Directory/@Id.
                                if (null != wixDirectoryTable)
                                {
                                    foreach (Row row in wixDirectoryTable.Rows)
                                    {
                                        componentIdGenSeeds.Add(row[0], (string)row[1]);
                                    }
                                }
                            }

                            // if the file rows have not been indexed by File.File yet
                            // then do that now
                            if (null == fileRows)
                            {
                                Table fileTable = output.Tables["File"];

                                int numFileRows = (null != fileTable) ? fileTable.Rows.Count : 0;

                                fileRows = new Hashtable(numFileRows);

                                if (null != fileTable)
                                {
                                    foreach (FileRow file in fileTable.Rows)
                                    {
                                        fileRows.Add(file.File, file);
                                    }
                                }
                            }

                            // calculate the canonical target path for the file key path
                            FileRow fileRow = fileRows[componentRow.KeyPath] as FileRow;
                            string directoryPath = GetDirectoryPath(directories, componentIdGenSeeds, componentRow.Directory, true);
                            string fileName = Installer.GetName(fileRow.FileName, false, true).ToLower(CultureInfo.InvariantCulture);

                            string path = Path.Combine(directoryPath, fileName);

                            // find paths that are not canonicalized
                            if (path.StartsWith(@"PersonalFolder\my pictures", StringComparison.Ordinal) ||
                                path.StartsWith(@"ProgramFilesFolder\common files", StringComparison.Ordinal) ||
                                path.StartsWith(@"ProgramMenuFolder\startup", StringComparison.Ordinal) ||
                                path.StartsWith("TARGETDIR", StringComparison.Ordinal) ||
                                path.StartsWith(@"StartMenuFolder\programs", StringComparison.Ordinal) ||
                                path.StartsWith(@"WindowsFolder\fonts", StringComparison.Ordinal))
                            {
                                this.core.OnMessage(WixErrors.IllegalPathForGeneratedComponentGuid(componentRow.SourceLineNumbers, fileRow.Component, path));
                            }
                            else // generate a guid
                            {
                                componentRow.Guid = Uuid.NewUuid(wixComponentGuidNamespace, path, this.backwardsCompatibleGuidGen).ToString("B").ToUpper(CultureInfo.InvariantCulture);
                            }
                        }
                    }
                }
            }
        }

        /// <summary>
        /// Update several msi tables with data contained in files references in the File table.
        /// </summary>
        /// <remarks>
        /// For versioned files, update the file version and language in the File table.  For
        /// unversioned files, add a row to the MsiFileHash table for the file.  For assembly
        /// files, add a row to the MsiAssembly table and add AssemblyName information by adding
        /// MsiAssemblyName rows.
        /// </remarks>
        /// <param name="output">Internal representation of the msi database to operate upon.</param>
        /// <param name="fileRows">The indexed file rows.</param>
        /// <param name="mediaRows">The indexed media rows.</param>
        /// <param name="infoCache">A hashtable to populate with the file information (optional).</param>
        /// <param name="modularizationGuid">The modularization guid (used in case of a merge module).</param>
        private void UpdateFileInformation(Output output, FileRowCollection fileRows, MediaRowCollection mediaRows, IDictionary<string, string> infoCache, string modularizationGuid)
        {
            // Index for all the fileId's
            // NOTE: When dealing with patches, there is a file table for each transform. In most cases, the data in these rows will be the same, however users of this index need to be aware of this.
            Hashtable fileRowIndex = new Hashtable(fileRows.Count);
            Table mediaTable = output.Tables["Media"];

            // calculate sequence numbers and media disk id layout for all file media information objects
            if (OutputType.Module == output.Type)
            {
                int lastSequence = 0;
                foreach (FileRow fileRow in fileRows)
                {
                    fileRow.Sequence = ++lastSequence;
                    fileRowIndex[fileRow.File] = fileRow;
                }
            }
            else if (null != mediaTable)
            {
                int lastSequence = 0;
                MediaRow mediaRow = null;
                SortedList patchGroups = new SortedList();

                // sequence the non-patch-added files
                foreach (FileRow fileRow in fileRows)
                {
                    fileRowIndex[fileRow.File] = fileRow;
                    if (null == mediaRow)
                    {
                        mediaRow = mediaRows[fileRow.DiskId];
                        if (OutputType.Patch == output.Type)
                        {
                            // patch Media cannot start at zero
                            lastSequence = mediaRow.LastSequence;
                        }
                    }
                    else if (mediaRow.DiskId != fileRow.DiskId)
                    {
                        mediaRow.LastSequence = lastSequence;
                        mediaRow = mediaRows[fileRow.DiskId];
                    }

                    if (0 < fileRow.PatchGroup)
                    {
                        ArrayList patchGroup = (ArrayList)patchGroups[fileRow.PatchGroup];

                        if (null == patchGroup)
                        {
                            patchGroup = new ArrayList();
                            patchGroups.Add(fileRow.PatchGroup, patchGroup);
                        }

                        patchGroup.Add(fileRow);
                    }
                    else
                    {
                        fileRow.Sequence = ++lastSequence;
                    }
                }

                if (null != mediaRow)
                {
                    mediaRow.LastSequence = lastSequence;
                    mediaRow = null;
                }

                // sequence the patch-added files
                foreach (ArrayList patchGroup in patchGroups.Values)
                {
                    foreach (FileRow fileRow in patchGroup)
                    {
                        if (null == mediaRow)
                        {
                            mediaRow = mediaRows[fileRow.DiskId];
                        }
                        else if (mediaRow.DiskId != fileRow.DiskId)
                        {
                            mediaRow.LastSequence = lastSequence;
                            mediaRow = mediaRows[fileRow.DiskId];
                        }

                        fileRow.Sequence = ++lastSequence;
                    }
                }

                if (null != mediaRow)
                {
                    mediaRow.LastSequence = lastSequence;
                }
            }
            else
            {
                foreach (FileRow fileRow in fileRows)
                {
                    fileRowIndex[fileRow.File] = fileRow;
                }
            }

            // no more work to do here if there are no file rows in the file table
            // note that this does not mean there are no files - the files from
            // merge modules are never put in the output's file table
            Table fileTable = output.Tables["File"];
            if (null == fileTable)
            {
                return;
            }

            // gather information about files that did not come from merge modules
            foreach (FileRow fileRow in fileTable.Rows)
            {
                FileInfo fileInfo = null;

                if (!this.suppressFileHashAndInfo || (!this.suppressAssemblies && FileAssemblyType.NotAnAssembly != fileRow.AssemblyType))
                {
                    try
                    {
                        fileInfo = new FileInfo(fileRow.Source);
                    }
                    catch (ArgumentException)
                    {
                        this.core.OnMessage(WixErrors.InvalidFileName(fileRow.SourceLineNumbers, fileRow.Source));
                        continue;
                    }
                    catch (PathTooLongException)
                    {
                        this.core.OnMessage(WixErrors.InvalidFileName(fileRow.SourceLineNumbers, fileRow.Source));
                        continue;
                    }
                    catch (NotSupportedException)
                    {
                        this.core.OnMessage(WixErrors.InvalidFileName(fileRow.SourceLineNumbers, fileRow.Source));
                        continue;
                    }
                }

                if (!this.suppressFileHashAndInfo)
                {
                    if (fileInfo.Exists)
                    {
                        string version;
                        string language;

                        if (fileInfo.Length > Int32.MaxValue)
                        {
                            throw new WixException(WixErrors.FileTooLarge(fileRow.SourceLineNumbers, fileRow.Source));
                        }

                        fileRow.FileSize = Convert.ToInt32(fileInfo.Length, CultureInfo.InvariantCulture);

                        try
                        {
                            Installer.GetFileVersion(fileInfo.FullName, out version, out language);
                        }
                        catch (Win32Exception e)
                        {
                            if (0x2 == e.NativeErrorCode) // ERROR_FILE_NOT_FOUND
                            {
                                throw new WixException(WixErrors.FileNotFound(fileRow.SourceLineNumbers, fileInfo.FullName));
                            }
                            else
                            {
                                throw new WixException(WixErrors.Win32Exception(e.NativeErrorCode, e.Message));
                            }
                        }

                        // If there is no version, it is assumed there is no language because it won't matter in the versioning of the install.
                        if (0 == version.Length)   // unversioned files have their hashes added to the MsiFileHash table
                        {
                            if (null != fileRow.Version)
                            {
                                // Check if this is a companion file. If its not, it is a default version.
                                if (!fileRowIndex.ContainsKey(fileRow.Version))
                                {
                                    this.core.OnMessage(WixWarnings.DefaultVersionUsedForUnversionedFile(fileRow.SourceLineNumbers, fileRow.Version, fileRow.File));
                                }
                            }
                            else
                            {
                                if (null != fileRow.Language)
                                {
                                    this.core.OnMessage(WixWarnings.DefaultLanguageUsedForUnversionedFile(fileRow.SourceLineNumbers, fileRow.Language, fileRow.File));
                                }

                                int[] hash;
                                try
                                {
                                    Installer.GetFileHash(fileInfo.FullName, 0, out hash);
                                }
                                catch (Win32Exception e)
                                {
                                    if (0x2 == e.NativeErrorCode) // ERROR_FILE_NOT_FOUND
                                    {
                                        throw new WixException(WixErrors.FileNotFound(fileRow.SourceLineNumbers, fileInfo.FullName));
                                    }
                                    else
                                    {
                                        throw new WixException(WixErrors.Win32Exception(e.NativeErrorCode, fileInfo.FullName, e.Message));
                                    }
                                }

                                Table msiFileHashTable = output.EnsureTable(this.core.TableDefinitions["MsiFileHash"]);
                                Row msiFileHashRow = msiFileHashTable.CreateRow(fileRow.SourceLineNumbers);
                                msiFileHashRow[0] = fileRow.File;
                                msiFileHashRow[1] = 0;
                                msiFileHashRow[2] = hash[0];
                                msiFileHashRow[3] = hash[1];
                                msiFileHashRow[4] = hash[2];
                                msiFileHashRow[5] = hash[3];
                                fileRow.HashRow = msiFileHashRow;
                            }
                        }
                        else // update the file row with the version and language information
                        {
                            // Check if the version field references a fileId because this would mean it has a companion file and the version should not be overwritten.
                            if (null == fileRow.Version || !fileRowIndex.ContainsKey(fileRow.Version))
                            {
                                fileRow.Version = version;
                            }

                            if (null != fileRow.Language && 0 == language.Length)
                            {
                                this.core.OnMessage(WixWarnings.DefaultLanguageUsedForVersionedFile(fileRow.SourceLineNumbers, fileRow.Language, fileRow.File));
                            }
                            else
                            {
                                fileRow.Language = language;
                            }

                            // Populate the binder variables for this file information if requested.
                            if (null != infoCache)
                            {
                                if (!String.IsNullOrEmpty(fileRow.Version))
                                {
                                    string key = String.Format(CultureInfo.InvariantCulture, "fileversion.{0}", Demodularize(output, modularizationGuid, fileRow.File));
                                    infoCache[key] = fileRow.Version;
                                }

                                if (!String.IsNullOrEmpty(fileRow.Language))
                                {
                                    string key = String.Format(CultureInfo.InvariantCulture, "filelanguage.{0}", Demodularize(output, modularizationGuid, fileRow.File));
                                    infoCache[key] = fileRow.Language;
                                }
                            }
                        }
                    }
                    else
                    {
                        this.core.OnMessage(WixErrors.CannotFindFile(fileRow.SourceLineNumbers, fileRow.File, fileRow.FileName, fileRow.Source));
                    }
                }

                // if we're not suppressing automagically grabbing assembly information and this is a
                // CLR assembly, load the assembly and get the assembly name information
                if (!this.suppressAssemblies)
                {
                    if (FileAssemblyType.DotNetAssembly == fileRow.AssemblyType)
                    {
                        StringDictionary assemblyNameValues = new StringDictionary();

                        CLRInterop.IReferenceIdentity referenceIdentity = null;
                        Guid referenceIdentityGuid = CLRInterop.ReferenceIdentityGuid;
                        uint result = CLRInterop.GetAssemblyIdentityFromFile(fileInfo.FullName, ref referenceIdentityGuid, out referenceIdentity);
                        if (0 == result && null != referenceIdentity)
                        {
                            string culture = referenceIdentity.GetAttribute(null, "Culture");
                            if (null != culture)
                            {
                                assemblyNameValues.Add("Culture", culture);
                            }
                            else
                            {
                                assemblyNameValues.Add("Culture", "neutral");
                            }

                            string name = referenceIdentity.GetAttribute(null, "Name");
                            if (null != name)
                            {
                                assemblyNameValues.Add("Name", name);
                            }

                            string processorArchitecture = referenceIdentity.GetAttribute(null, "ProcessorArchitecture");
                            if (null != processorArchitecture)
                            {
                                assemblyNameValues.Add("ProcessorArchitecture", processorArchitecture);
                            }

                            string publicKeyToken = referenceIdentity.GetAttribute(null, "PublicKeyToken");
                            if (null != publicKeyToken)
                            {
                                if (!String.Equals(publicKeyToken, "neutral", StringComparison.OrdinalIgnoreCase))
                                {
                                    publicKeyToken = publicKeyToken.ToUpperInvariant();
                                }
                                else
                                {
                                    // Managed code expects "null" instead of "neutral", and
                                    // this won't be installed to the GAC since it's not signed anyway.
                                    publicKeyToken = "null";
                                }

                                assemblyNameValues.Add("PublicKeyToken", publicKeyToken);
                            }
                            else if (fileRow.AssemblyApplication == null)
                            {
                                throw new WixException(WixErrors.GacAssemblyNoStrongName(fileRow.SourceLineNumbers, fileInfo.FullName, fileRow.Component));
                            }

                            string version = referenceIdentity.GetAttribute(null, "Version");
                            if (null != version)
                            {
                                assemblyNameValues.Add("Version", version);
                            }
                        }
                        else
                        {
                            this.core.OnMessage(WixErrors.InvalidAssemblyFile(fileRow.SourceLineNumbers, fileInfo.FullName, String.Format(CultureInfo.InvariantCulture, "HRESULT: 0x{0:x8}", result)));
                            continue;
                        }

                        Table assemblyNameTable = output.EnsureTable(this.core.TableDefinitions["MsiAssemblyName"]);
                        if (assemblyNameValues.ContainsKey("name"))
                        {
                            SetMsiAssemblyName(output, assemblyNameTable, fileRow, "name", assemblyNameValues["name"], infoCache, modularizationGuid);
                        }

                        string fileVersion = null;
                        if (this.setMsiAssemblyNameFileVersion)
                        {
                            string language;

                            Installer.GetFileVersion(fileInfo.FullName, out fileVersion, out language);
                            SetMsiAssemblyName(output, assemblyNameTable, fileRow, "fileVersion", fileVersion, infoCache, modularizationGuid);
                        }

                        if (assemblyNameValues.ContainsKey("version"))
                        {
                            string assemblyVersion = assemblyNameValues["version"];

                            if (!this.exactAssemblyVersions)
                            {
                                // there is a bug in fusion that requires the assembly's "version" attribute
                                // to be equal to or longer than the "fileVersion" in length when its present;
                                // the workaround is to prepend zeroes to the last version number in the assembly version
                                if (this.setMsiAssemblyNameFileVersion && null != fileVersion && fileVersion.Length > assemblyVersion.Length)
                                {
                                    string padding = new string('0', fileVersion.Length - assemblyVersion.Length);
                                    string[] assemblyVersionNumbers = assemblyVersion.Split('.');

                                    if (assemblyVersionNumbers.Length > 0)
                                    {
                                        assemblyVersionNumbers[assemblyVersionNumbers.Length - 1] = String.Concat(padding, assemblyVersionNumbers[assemblyVersionNumbers.Length - 1]);
                                        assemblyVersion = String.Join(".", assemblyVersionNumbers);
                                    }
                                }
                            }

                            SetMsiAssemblyName(output, assemblyNameTable, fileRow, "version", assemblyVersion, infoCache, modularizationGuid);
                        }

                        if (assemblyNameValues.ContainsKey("culture"))
                        {
                            SetMsiAssemblyName(output, assemblyNameTable, fileRow, "culture", assemblyNameValues["culture"], infoCache, modularizationGuid);
                        }

                        if (assemblyNameValues.ContainsKey("publicKeyToken"))
                        {
                            SetMsiAssemblyName(output, assemblyNameTable, fileRow, "publicKeyToken", assemblyNameValues["publicKeyToken"], infoCache, modularizationGuid);
                        }

                        if (null != fileRow.ProcessorArchitecture && 0 < fileRow.ProcessorArchitecture.Length)
                        {
                            SetMsiAssemblyName(output, assemblyNameTable, fileRow, "processorArchitecture", fileRow.ProcessorArchitecture, infoCache, modularizationGuid);
                        }

                        if (assemblyNameValues.ContainsKey("processorArchitecture"))
                        {
                            SetMsiAssemblyName(output, assemblyNameTable, fileRow, "processorArchitecture", assemblyNameValues["processorArchitecture"], infoCache, modularizationGuid);
                        }

                        // add the assembly name to the information cache
                        if (null != infoCache)
                        {
                            string key = String.Concat("assemblyfullname.", Demodularize(output, modularizationGuid, fileRow.File));
                            string assemblyName = String.Concat(assemblyNameValues["name"], ", version=", assemblyNameValues["version"], ", culture=", assemblyNameValues["culture"], ", publicKeyToken=", String.IsNullOrEmpty(assemblyNameValues["publicKeyToken"]) ? "null" : assemblyNameValues["publicKeyToken"]);
                            if (assemblyNameValues.ContainsKey("processorArchitecture"))
                            {
                                assemblyName = String.Concat(assemblyName, ", processorArchitecture=", assemblyNameValues["processorArchitecture"]);
                            }

                            infoCache[key] = assemblyName;
                        }
                    }
                    else if (FileAssemblyType.Win32Assembly == fileRow.AssemblyType)
                    {
                        // Able to use the index because only the Source field is used and it is used only for more complete error messages.
                        FileRow fileManifestRow = (FileRow)fileRowIndex[fileRow.AssemblyManifest];

                        if (null == fileManifestRow)
                        {
                            this.core.OnMessage(WixErrors.MissingManifestForWin32Assembly(fileRow.SourceLineNumbers, fileRow.File, fileRow.AssemblyManifest));
                        }

                        string type = null;
                        string name = null;
                        string version = null;
                        string processorArchitecture = null;
                        string publicKeyToken = null;

                        // loading the dom is expensive we want more performant APIs than the DOM
                        // Navigator is cheaper than dom.  Perhaps there is a cheaper API still.
                        try
                        {
                            XPathDocument doc = new XPathDocument(fileManifestRow.Source);
                            XPathNavigator nav = doc.CreateNavigator();
                            nav.MoveToRoot();

                            // this assumes a particular schema for a win32 manifest and does not
                            // provide error checking if the file does not conform to schema.
                            // The fallback case here is that nothing is added to the MsiAssemblyName
                            // table for an out of tolerance Win32 manifest.  Perhaps warnings needed.
                            if (nav.MoveToFirstChild())
                            {
                                while (nav.NodeType != XPathNodeType.Element || nav.Name != "assembly")
                                {
                                    nav.MoveToNext();
                                }

                                if (nav.MoveToFirstChild())
                                {
                                    bool hasNextSibling = true;
                                    while (nav.NodeType != XPathNodeType.Element || nav.Name != "assemblyIdentity" && hasNextSibling)
                                    {
                                        hasNextSibling = nav.MoveToNext();
                                    }
                                    if (!hasNextSibling)
                                    {
                                        this.core.OnMessage(WixErrors.InvalidManifestContent(fileRow.SourceLineNumbers, fileManifestRow.Source));
                                        continue;
                                    }

                                    if (nav.MoveToAttribute("type", String.Empty))
                                    {
                                        type = nav.Value;
                                        nav.MoveToParent();
                                    }

                                    if (nav.MoveToAttribute("name", String.Empty))
                                    {
                                        name = nav.Value;
                                        nav.MoveToParent();
                                    }

                                    if (nav.MoveToAttribute("version", String.Empty))
                                    {
                                        version = nav.Value;
                                        nav.MoveToParent();
                                    }

                                    if (nav.MoveToAttribute("processorArchitecture", String.Empty))
                                    {
                                        processorArchitecture = nav.Value;
                                        nav.MoveToParent();
                                    }

                                    if (nav.MoveToAttribute("publicKeyToken", String.Empty))
                                    {
                                        publicKeyToken = nav.Value;
                                        nav.MoveToParent();
                                    }
                                }
                            }
                        }
                        catch (FileNotFoundException fe)
                        {
                            this.core.OnMessage(WixErrors.FileNotFound(SourceLineNumberCollection.FromFileName(fileManifestRow.Source), fe.FileName, "AssemblyManifest"));
                        }
                        catch (XmlException xe)
                        {
                            this.core.OnMessage(WixErrors.InvalidXml(SourceLineNumberCollection.FromFileName(fileManifestRow.Source), "manifest", xe.Message));
                        }

                        Table assemblyNameTable = output.EnsureTable(this.core.TableDefinitions["MsiAssemblyName"]);
                        if (null != name && 0 < name.Length)
                        {
                            SetMsiAssemblyName(output, assemblyNameTable, fileRow, "name", name, infoCache, modularizationGuid);
                        }

                        if (null != version && 0 < version.Length)
                        {
                            SetMsiAssemblyName(output, assemblyNameTable, fileRow, "version", version, infoCache, modularizationGuid);
                        }

                        if (null != type && 0 < type.Length)
                        {
                            SetMsiAssemblyName(output, assemblyNameTable, fileRow, "type", type, infoCache, modularizationGuid);
                        }

                        if (null != processorArchitecture && 0 < processorArchitecture.Length)
                        {
                            SetMsiAssemblyName(output, assemblyNameTable, fileRow, "processorArchitecture", processorArchitecture, infoCache, modularizationGuid);
                        }

                        if (null != publicKeyToken && 0 < publicKeyToken.Length)
                        {
                            SetMsiAssemblyName(output, assemblyNameTable, fileRow, "publicKeyToken", publicKeyToken, infoCache, modularizationGuid);
                        }
                    }
                }
            }
        }

        /// <summary>
        /// Update Control and BBControl text by reading from files when necessary.
        /// </summary>
        /// <param name="output">Internal representation of the msi database to operate upon.</param>
        private void UpdateControlText(Output output)
        {
            // Control table
            Table controlTable = output.Tables["Control"];
            if (null != controlTable)
            {
                foreach (ControlRow controlRow in controlTable.Rows)
                {
                    if (null != controlRow.SourceFile)
                    {
                        controlRow.Text = this.ReadTextFile(controlRow.SourceLineNumbers, controlRow.SourceFile);
                    }
                }
            }

            // BBControl table
            Table bbcontrolTable = output.Tables["BBControl"];
            if (null != bbcontrolTable)
            {
                foreach (BBControlRow bbcontrolRow in bbcontrolTable.Rows)
                {
                    if (null != bbcontrolRow.SourceFile)
                    {
                        bbcontrolRow.Text = this.ReadTextFile(bbcontrolRow.SourceLineNumbers, bbcontrolRow.SourceFile);
                    }
                }
            }
        }

        /// <summary>
        /// Reads a text file and returns the contents.
        /// </summary>
        /// <param name="sourceLineNumbers">Source line numbers for row from source.</param>
        /// <param name="source">Source path to file to read.</param>
        /// <returns>Text string read from file.</returns>
        private string ReadTextFile(SourceLineNumberCollection sourceLineNumbers, string source)
        {
            string text = null;

            try
            {
                using (StreamReader reader = new StreamReader(source))
                {
                    text = reader.ReadToEnd();
                }
            }
            catch (DirectoryNotFoundException e)
            {
                this.core.OnMessage(WixErrors.BinderFileManagerMissingFile(sourceLineNumbers, e.Message));
            }
            catch (FileNotFoundException e)
            {
                this.core.OnMessage(WixErrors.BinderFileManagerMissingFile(sourceLineNumbers, e.Message));
            }
            catch (IOException e)
            {
                this.core.OnMessage(WixErrors.BinderFileManagerMissingFile(sourceLineNumbers, e.Message));
            }
            catch (NotSupportedException)
            {
                this.core.OnMessage(WixErrors.FileNotFound(sourceLineNumbers, source));
            }

            return text;
        }

        /// <summary>
        /// Merges in any modules to the output database.
        /// </summary>
        /// <param name="tempDatabaseFile">The temporary database file.</param>
        /// <param name="output">Output that specifies database and modules to merge.</param>
        /// <param name="fileRows">The indexed file rows.</param>
        /// <param name="suppressedTableNames">The names of tables that are suppressed.</param>
        /// <remarks>Expects that output's database has already been generated.</remarks>
        private void MergeModules(string tempDatabaseFile, Output output, FileRowCollection fileRows, StringCollection suppressedTableNames)
        {
            Debug.Assert(OutputType.Product == output.Type);

            Table wixMergeTable = output.Tables["WixMerge"];
            Table wixFeatureModulesTable = output.Tables["WixFeatureModules"];

            // check for merge rows to see if there is any work to do
            if (null == wixMergeTable || 0 == wixMergeTable.Rows.Count)
            {
                return;
            }

            IMsmMerge2 merge = null;
            bool commit = true;
            bool logOpen = false;
            bool databaseOpen = false;
            string logPath = null;
            try
            {
                merge = NativeMethods.GetMsmMerge();

                logPath = Path.Combine(this.TempFilesLocation, "merge.log");
                merge.OpenLog(logPath);
                logOpen = true;

                merge.OpenDatabase(tempDatabaseFile);
                databaseOpen = true;

                // process all the merge rows
                foreach (WixMergeRow wixMergeRow in wixMergeTable.Rows)
                {
                    bool moduleOpen = false;

                    try
                    {
                        short mergeLanguage;

                        try
                        {
                            mergeLanguage = Convert.ToInt16(wixMergeRow.Language, CultureInfo.InvariantCulture);
                        }
                        catch (System.FormatException)
                        {
                            this.core.OnMessage(WixErrors.InvalidMergeLanguage(wixMergeRow.SourceLineNumbers, wixMergeRow.Id, wixMergeRow.Language));
                            continue;
                        }

                        this.core.OnMessage(WixVerboses.OpeningMergeModule(wixMergeRow.SourceFile, mergeLanguage));
                        merge.OpenModule(wixMergeRow.SourceFile, mergeLanguage);
                        moduleOpen = true;

                        // If there is merge configuration data, create a callback object to contain it all.
                        ConfigurationCallback callback = null;
                        if (!String.IsNullOrEmpty(wixMergeRow.ConfigurationData))
                        {
                            callback = new ConfigurationCallback(wixMergeRow.ConfigurationData);
                        }

                        // merge the module into the database that's being built
                        this.core.OnMessage(WixVerboses.MergingMergeModule(wixMergeRow.SourceFile));
                        merge.MergeEx(wixMergeRow.Feature, wixMergeRow.Directory, callback);

                        // connect any non-primary features
                        if (null != wixFeatureModulesTable)
                        {
                            foreach (Row row in wixFeatureModulesTable.Rows)
                            {
                                if (wixMergeRow.Id == (string)row[1])
                                {
                                    this.core.OnMessage(WixVerboses.ConnectingMergeModule(wixMergeRow.SourceFile, (string)row[0]));
                                    merge.Connect((string)row[0]);
                                }
                            }
                        }
                    }
                    catch (COMException)
                    {
                        commit = false;
                    }
                    finally
                    {
                        IMsmErrors mergeErrors = merge.Errors;

                        // display all the errors encountered during the merge operations for this module
                        for (int i = 1; i <= mergeErrors.Count; i++)
                        {
                            IMsmError mergeError = mergeErrors[i];
                            StringBuilder databaseKeys = new StringBuilder();
                            StringBuilder moduleKeys = new StringBuilder();

                            // build a string of the database keys
                            for (int j = 1; j <= mergeError.DatabaseKeys.Count; j++)
                            {
                                if (1 != j)
                                {
                                    databaseKeys.Append(';');
                                }
                                databaseKeys.Append(mergeError.DatabaseKeys[j]);
                            }

                            // build a string of the module keys
                            for (int j = 1; j <= mergeError.ModuleKeys.Count; j++)
                            {
                                if (1 != j)
                                {
                                    moduleKeys.Append(';');
                                }
                                moduleKeys.Append(mergeError.ModuleKeys[j]);
                            }

                            // display the merge error based on the msm error type
                            switch (mergeError.Type)
                            {
                                case MsmErrorType.msmErrorExclusion:
                                    this.core.OnMessage(WixErrors.MergeExcludedModule(wixMergeRow.SourceLineNumbers, wixMergeRow.Id, moduleKeys.ToString()));
                                    break;
                                case MsmErrorType.msmErrorFeatureRequired:
                                    this.core.OnMessage(WixErrors.MergeFeatureRequired(wixMergeRow.SourceLineNumbers, mergeError.ModuleTable, moduleKeys.ToString(), wixMergeRow.SourceFile, wixMergeRow.Id));
                                    break;
                                case MsmErrorType.msmErrorLanguageFailed:
                                    this.core.OnMessage(WixErrors.MergeLanguageFailed(wixMergeRow.SourceLineNumbers, mergeError.Language, wixMergeRow.SourceFile));
                                    break;
                                case MsmErrorType.msmErrorLanguageUnsupported:
                                    this.core.OnMessage(WixErrors.MergeLanguageUnsupported(wixMergeRow.SourceLineNumbers, mergeError.Language, wixMergeRow.SourceFile));
                                    break;
                                case MsmErrorType.msmErrorResequenceMerge:
                                    this.core.OnMessage(WixWarnings.MergeRescheduledAction(wixMergeRow.SourceLineNumbers, mergeError.DatabaseTable, databaseKeys.ToString(), wixMergeRow.SourceFile));
                                    break;
                                case MsmErrorType.msmErrorTableMerge:
                                    if ("_Validation" != mergeError.DatabaseTable) // ignore merge errors in the _Validation table
                                    {
                                        this.core.OnMessage(WixWarnings.MergeTableFailed(wixMergeRow.SourceLineNumbers, mergeError.DatabaseTable, databaseKeys.ToString(), wixMergeRow.SourceFile));
                                    }
                                    break;
                                case MsmErrorType.msmErrorPlatformMismatch:
                                    this.core.OnMessage(WixErrors.MergePlatformMismatch(wixMergeRow.SourceLineNumbers, wixMergeRow.SourceFile));
                                    break;
                                default:
                                    this.core.OnMessage(WixErrors.UnexpectedException(String.Format(CultureInfo.CurrentUICulture, WixStrings.EXP_UnexpectedMergerErrorWithType, Enum.GetName(typeof(MsmErrorType), mergeError.Type), logPath), "InvalidOperationException", Environment.StackTrace));
                                    break;
                            }
                        }

                        if (0 >= mergeErrors.Count && !commit)
                        {
                            this.core.OnMessage(WixErrors.UnexpectedException(String.Format(CultureInfo.CurrentUICulture, WixStrings.EXP_UnexpectedMergerErrorInSourceFile, wixMergeRow.SourceFile, logPath), "InvalidOperationException", Environment.StackTrace));
                        }

                        if (moduleOpen)
                        {
                            merge.CloseModule();
                        }
                    }
                }
            }
            finally
            {
                if (databaseOpen)
                {
                    merge.CloseDatabase(commit);
                }

                if (logOpen)
                {
                    merge.CloseLog();
                }
            }

            // stop processing if an error previously occurred
            if (this.core.EncounteredError)
            {
                return;
            }

            using (Database db = new Database(tempDatabaseFile, OpenDatabase.Direct))
            {
                Table suppressActionTable = output.Tables["WixSuppressAction"];

                // suppress individual actions
                if (null != suppressActionTable)
                {
                    foreach (Row row in suppressActionTable.Rows)
                    {
                        if (db.TableExists((string)row[0]))
                        {
                            string query = String.Format(CultureInfo.InvariantCulture, "SELECT * FROM {0} WHERE `Action` = '{1}'", row[0].ToString(), (string)row[1]);

                            using (View view = db.OpenExecuteView(query))
                            {
                                Record record;

                                if (null != (record = view.Fetch()))
                                {
                                    this.core.OnMessage(WixWarnings.SuppressMergedAction((string)row[1], row[0].ToString()));
                                    view.Modify(ModifyView.Delete, record);
                                    record.Close();
                                }
                            }
                        }
                    }
                }

                // query for merge module actions in suppressed sequences and drop them
                foreach (string tableName in suppressedTableNames)
                {
                    if (!db.TableExists(tableName))
                    {
                        continue;
                    }

                    using (View view = db.OpenExecuteView(String.Concat("SELECT `Action` FROM ", tableName)))
                    {
                        Record resultRecord;
                        while (null != (resultRecord = view.Fetch()))
                        {
                            this.core.OnMessage(WixWarnings.SuppressMergedAction(resultRecord.GetString(1), tableName));
                            resultRecord.Close();
                        }
                    }

                    // drop suppressed sequences
                    using (View view = db.OpenExecuteView(String.Concat("DROP TABLE ", tableName)))
                    {
                    }

                    // delete the validation rows
                    using (View view = db.OpenView(String.Concat("DELETE FROM _Validation WHERE `Table` = ?")))
                    {
                        using (Record record = new Record(1))
                        {
                            record.SetString(1, tableName);
                            view.Execute(record);
                        }
                    }
                }

                // now update the Attributes column for the files from the Merge Modules
                this.core.OnMessage(WixVerboses.ResequencingMergeModuleFiles());
                using (View view = db.OpenView("SELECT `Sequence`, `Attributes` FROM `File` WHERE `File`=?"))
                {
                    foreach (FileRow fileRow in fileRows)
                    {
                        if (!fileRow.FromModule)
                        {
                            continue;
                        }

                        using (Record record = new Record(1))
                        {
                            record.SetString(1, fileRow.File);
                            view.Execute(record);
                        }

                        using (Record recordUpdate = view.Fetch())
                        {
                            if (null == recordUpdate)
                            {
                                throw new InvalidOperationException("Failed to fetch a File row from the database that was merged in from a module.");
                            }

                            recordUpdate.SetInteger(1, fileRow.Sequence);

                            // update the file attributes to match the compression specified
                            // on the Merge element or on the Package element
                            int attributes = 0;

                            // get the current value if its not null
                            if (!recordUpdate.IsNull(2))
                            {
                                attributes = recordUpdate.GetInteger(2);
                            }

                            if (YesNoType.Yes == fileRow.Compressed)
                            {
                                // these are mutually exclusive
                                attributes |= MsiInterop.MsidbFileAttributesCompressed;
                                attributes &= ~MsiInterop.MsidbFileAttributesNoncompressed;
                            }
                            else if (YesNoType.No == fileRow.Compressed)
                            {
                                // these are mutually exclusive
                                attributes |= MsiInterop.MsidbFileAttributesNoncompressed;
                                attributes &= ~MsiInterop.MsidbFileAttributesCompressed;
                            }
                            else // not specified
                            {
                                Debug.Assert(YesNoType.NotSet == fileRow.Compressed);

                                // clear any compression bits
                                attributes &= ~MsiInterop.MsidbFileAttributesCompressed;
                                attributes &= ~MsiInterop.MsidbFileAttributesNoncompressed;
                            }
                            recordUpdate.SetInteger(2, attributes);

                            view.Modify(ModifyView.Update, recordUpdate);
                        }
                    }
                }

                db.Commit();
            }
        }

        /// <summary>
        /// Creates cabinet files.
        /// </summary>
        /// <param name="output">Output to generate image for.</param>
        /// <param name="fileRows">The indexed file rows.</param>
        /// <param name="fileTransfers">Array of files to be transfered.</param>
        /// <param name="mediaRows">The indexed media rows.</param>
        /// <param name="layoutDirectory">The directory in which the image should be layed out.</param>
        /// <param name="compressed">Flag if source image should be compressed.</param>
        /// <returns>The uncompressed file rows.</returns>
        private FileRowCollection CreateCabinetFiles(Output output, FileRowCollection fileRows, ArrayList fileTransfers, MediaRowCollection mediaRows, string layoutDirectory, bool compressed)
        {
            Hashtable cabinets = new Hashtable();
            MediaRow mergeModuleMediaRow = null;
            FileRowCollection uncompressedFileRows = new FileRowCollection();

            if (OutputType.Module == output.Type)
            {
                Table mediaTable = new Table(null, this.core.TableDefinitions["Media"]);
                mergeModuleMediaRow = (MediaRow)mediaTable.CreateRow(null);
                mergeModuleMediaRow.Cabinet = "#MergeModule.CABinet";

                cabinets.Add(mergeModuleMediaRow, new FileRowCollection());
            }
            else
            {
                foreach (MediaRow mediaRow in mediaRows)
                {
                    if (null != mediaRow.Cabinet)
                    {
                        cabinets.Add(mediaRow, new FileRowCollection());
                    }
                }
            }

            foreach (FileRow fileRow in fileRows)
            {
                if (OutputType.Module == output.Type)
                {
                    ((FileRowCollection)cabinets[mergeModuleMediaRow]).Add(fileRow);
                }
                else
                {
                    MediaRow mediaRow = mediaRows[fileRow.DiskId];

                    if (OutputType.Product == output.Type &&
                        (YesNoType.No == fileRow.Compressed ||
                        (YesNoType.NotSet == fileRow.Compressed && !compressed)))
                    {
                        uncompressedFileRows.Add(fileRow);
                    }
                    else // file in a Module or marked compressed
                    {
                        FileRowCollection cabinetFileRow = (FileRowCollection)cabinets[mediaRow];

                        if (null != cabinetFileRow)
                        {
                            cabinetFileRow.Add(fileRow);
                        }
                        else
                        {
                            throw new WixException(WixErrors.ExpectedMediaCabinet(fileRow.SourceLineNumbers, fileRow.File, fileRow.DiskId));
                        }
                    }
                }
            }

            this.SetCabbingThreadCount();

            CabinetBuilder cabinetBuilder = new CabinetBuilder(this.cabbingThreadCount);
            if (null != this.MessageHandler)
            {
                cabinetBuilder.Message += new MessageEventHandler(this.MessageHandler);
            }

            foreach (DictionaryEntry entry in cabinets)
            {
                MediaRow mediaRow = (MediaRow)entry.Key;
                FileRowCollection files = (FileRowCollection)entry.Value;

                string cabinetDir = this.FileManager.ResolveMedia(mediaRow, layoutDirectory);

                CabinetWorkItem cabinetWorkItem = this.CreateCabinetWorkItem(output, cabinetDir, mediaRow, files, fileTransfers);
                if (null != cabinetWorkItem)
                {
                    cabinetBuilder.Enqueue(cabinetWorkItem);
                }
            }

            // stop processing if an error previously occurred
            if (this.core.EncounteredError)
            {
                return null;
            }

            // create queued cabinets with multiple threads
            int cabError = cabinetBuilder.CreateQueuedCabinets();
            if (0 != cabError)
            {
                this.core.EncounteredError = true;
                return null;
            }

            return uncompressedFileRows;
        }

        /// <summary>
        /// Final step in binding that transfers (moves/copies) all files generated into the appropriate
        /// location in the source image
        /// </summary>
        /// <param name="output">The output to bind.</param>
        /// <param name="fileTransfers">Array of files to transfer.</param>
        /// <param name="baseDirectory">Root of the relative LayoutDirectories.</param>
        public static void ProcessLayoutDirectories(BinderCore core, Output output, ArrayList fileTransfers, string baseDirectory)
        {
            if (core.EncounteredError)
            {
                return;
            }

            Table wixLayoutDirectory = output.Tables["WixLayoutDirectory"];
            if (null == wixLayoutDirectory)
            {
                return;
            }

            Table wixLayoutFiles = output.Tables["WixLayoutFile"];
            if (null == wixLayoutFiles)
            {
                return;
            }

            wixLayoutDirectory.ValidateRows();
            wixLayoutFiles.ValidateRows(); // just for completness sake. See comment in tables.xml for details

            ArrayList rootLayouts = new ArrayList();
            Table wixLayoutDirRef = output.Tables["WixLayoutDirRef"];
            if (null != wixLayoutDirRef)
            {
                wixLayoutDirRef.ValidateRows();
                foreach (Row rowLayout in wixLayoutDirectory.Rows)
                {
                    bool isRoot = true;
                    foreach (Row rowRef in wixLayoutDirRef.Rows)
                    {
                        if (0 == String.Compare(rowRef[0].ToString(), Guid.Empty.ToString(), StringComparison.Ordinal))
                        {
                            isRoot = true;
                            break;
                        }

                        if (0 == String.Compare(rowRef[1].ToString(), rowLayout[0].ToString(), StringComparison.Ordinal))
                        {
                            isRoot = false;
                            break;
                        }
                    }

                    if (isRoot)
                    {
                        rootLayouts.Add(rowLayout);
                    }
                }
            }
            else
            {
                foreach (Row rowLayout in wixLayoutDirectory.Rows)
                {
                    rootLayouts.Add(rowLayout);
                }
            }

            Hashtable fileKeys = new Hashtable(wixLayoutFiles.Rows.Count);
            foreach (Row row in wixLayoutFiles.Rows)
            {
                // Generate "Name" if needed
                if (null == row[2] || String.IsNullOrEmpty(row[2].ToString()))
                {
                    row[2] = Path.GetFileName(row[3].ToString());
                }

                // Check for collisions
                // Note that we "manufacture" what we really wanted our primary keys to be. See the entry for the table in tables.xml for details.
                string wantedKey = String.Concat(row[1].ToString(), "/", row[2].ToString());
                if (fileKeys.Contains(wantedKey))
                {
                    core.OnMessage(WixErrors.DuplicatePrimaryKey(row.SourceLineNumbers, wantedKey, "WixLayoutFile"));
                }
                else
                {
                    fileKeys.Add(wantedKey, row.SourceLineNumbers);
                }
            }

            if (core.EncounteredError)
            {
                return;
            }

            foreach (Row row in rootLayouts)
            {
                Stack stack = new Stack();
                stack.Push(row);
                ProcessLayoutDirectory(core, baseDirectory, stack, wixLayoutDirectory.Rows, null == wixLayoutDirRef ? null : wixLayoutDirRef.Rows, wixLayoutFiles.Rows, fileTransfers);
                stack.Pop();
            }
        }

        /// <summary>
        /// Final step in binding that transfers (moves/copies) all files generated into the appropriate
        /// location in the source image
        /// </summary>
        /// <param name="path">The root path for this instance of the LayoutDirectory.</param>
        /// <param name="parents">The stack of parents of the current instance.</param>
        /// <param name="dirs">WixLayoutDirectory table rows.</param>
        /// <param name="refs">WixLayoutDirRefs table rows.</param>
        /// <param name="files">WixLayoutFiles table rows.</param>
        /// <param name="fileTransfers">Array of files to transfer.</param>
        private static void ProcessLayoutDirectory(BinderCore core, string path, Stack parents, RowCollection dirs, RowCollection refs, RowCollection files, ArrayList fileTransfers)
        {
            Row thisDir = (Row)parents.Peek();
            string currentDir = Path.Combine(path, thisDir[1].ToString());
            foreach (Row row in files)
            {
                fileTransfers.Add(new FileTransfer(row[3].ToString(), Path.Combine(currentDir, row[2].ToString()), false));
            }

            if (null != refs)
            {
                // find children
                ArrayList children = new ArrayList();
                foreach (Row row in refs)
                {
                    if (0 == String.Compare(row[0].ToString(), thisDir[0].ToString(), StringComparison.Ordinal))
                    {
                        children.Add(row);
                    }
                }

                // find row from dirs that each child is, since children contains rows from refs and we need to add rows from dirs to parents
                foreach (Row rowRef in children)
                {
                    Row rowDir = null;
                    foreach (Row row in dirs)
                    {
                        if (0 == String.Compare(row[0].ToString(), rowRef[1].ToString(), StringComparison.Ordinal))
                        {
                            rowDir = row;
                            break;
                        }
                    }
                    // Test for circular directory loops
                    if (parents.Contains(rowDir))
                    {
                        core.OnMessage(WixErrors.IllegalNestedLayoutDirectory(rowDir.SourceLineNumbers));
                        return;
                    }

                    parents.Push(rowDir);
                    ProcessLayoutDirectory(core, currentDir, parents, dirs, refs, files, fileTransfers);
                    parents.Pop();
                }
            }
        }

        /// <summary>
        /// Sets the codepage of a database.
        /// </summary>
        /// <param name="db">Database to set codepage into.</param>
        /// <param name="output">Output with the codepage for the database.</param>
        private void SetDatabaseCodepage(Database db, Output output)
        {
            // write out the _ForceCodepage IDT file
            string idtPath = Path.Combine(this.TempFilesLocation, "_ForceCodepage.idt");
            using (StreamWriter idtFile = new StreamWriter(idtPath, false, Encoding.ASCII))
            {
                idtFile.WriteLine(); // dummy column name record
                idtFile.WriteLine(); // dummy column definition record
                idtFile.Write(output.Codepage);
                idtFile.WriteLine("\t_ForceCodepage");
            }

            // try to import the table into the MSI
            try
            {
                db.Import(Path.GetDirectoryName(idtPath), Path.GetFileName(idtPath));
            }
            catch (WixInvalidIdtException)
            {
                // the IDT should be valid, so an invalid code page was given
                throw new WixException(WixErrors.IllegalCodepage(output.Codepage));
            }
        }

        /// <summary>
        /// Creates a work item to create a cabinet.
        /// </summary>
        /// <param name="output">Output for the current database.</param>
        /// <param name="cabinetDir">Directory to create cabinet in.</param>
        /// <param name="mediaRow">MediaRow containing information about the cabinet.</param>
        /// <param name="fileRows">Collection of files in this cabinet.</param>
        /// <param name="fileTransfers">Array of files to be transfered.</param>
        /// <returns>created CabinetWorkItem object</returns>
        private CabinetWorkItem CreateCabinetWorkItem(Output output, string cabinetDir, MediaRow mediaRow, FileRowCollection fileRows, ArrayList fileTransfers)
        {
            CabinetWorkItem cabinetWorkItem = null;
            string tempCabinetFile = Path.Combine(this.TempFilesLocation, mediaRow.Cabinet);

            // check for an empty cabinet
            if (0 == fileRows.Count)
            {
                string cabinetName = mediaRow.Cabinet;

                // remove the leading '#' from the embedded cabinet name to make the warning easier to understand
                if (cabinetName.StartsWith("#", StringComparison.Ordinal))
                {
                    cabinetName = cabinetName.Substring(1);
                }

                this.core.OnMessage(WixWarnings.EmptyCabinet(mediaRow.SourceLineNumbers, cabinetName));
            }

            CabinetBuildOption cabinetBuildOption = this.FileManager.ResolveCabinet(fileRows, ref tempCabinetFile);

            // create a cabinet work item if it's not being skipped
            if (CabinetBuildOption.BuildAndCopy == cabinetBuildOption || CabinetBuildOption.BuildAndMove == cabinetBuildOption)
            {
                int maxThreshold = 0; // default to the threshold for best smartcabbing (makes smallest cabinet).
                Cab.CompressionLevel compressionLevel = this.defaultCompressionLevel;

                if (mediaRow.HasExplicitCompressionLevel)
                {
                    compressionLevel = mediaRow.CompressionLevel;
                }

                cabinetWorkItem = new CabinetWorkItem(fileRows, tempCabinetFile, maxThreshold, compressionLevel, this.FileManager);
            }

            if (mediaRow.Cabinet.StartsWith("#", StringComparison.Ordinal))
            {
                Table streamsTable = output.EnsureTable(this.core.TableDefinitions["_Streams"]);

                Row streamRow = streamsTable.CreateRow(null);
                streamRow[0] = mediaRow.Cabinet.Substring(1);
                streamRow[1] = tempCabinetFile;
            }
            else
            {
                string destinationPath = Path.Combine(cabinetDir, mediaRow.Cabinet);
                fileTransfers.Add(new FileTransfer(tempCabinetFile, destinationPath, CabinetBuildOption.BuildAndMove == cabinetBuildOption));
            }

            return cabinetWorkItem;
        }

        /// <summary>
        /// Process uncompressed files.
        /// </summary>
        /// <param name="tempDatabaseFile">The temporary database file.</param>
        /// <param name="fileRows">The collection of files to copy into the image.</param>
        /// <param name="fileTransfers">Array of files to be transfered.</param>
        /// <param name="mediaRows">The indexed media rows.</param>
        /// <param name="layoutDirectory">The directory in which the image should be layed out.</param>
        /// <param name="compressed">Flag if source image should be compressed.</param>
        /// <param name="longNamesInImage">Flag if long names should be used.</param>
        private void ProcessUncompressedFiles(string tempDatabaseFile, FileRowCollection fileRows, ArrayList fileTransfers, MediaRowCollection mediaRows, string layoutDirectory, bool compressed, bool longNamesInImage)
        {
            if (0 == fileRows.Count || this.core.EncounteredError)
            {
                return;
            }

            Hashtable directories = new Hashtable();
            using (Database db = new Database(tempDatabaseFile, OpenDatabase.ReadOnly))
            {
                using (View directoryView = db.OpenExecuteView("SELECT `Directory`, `Directory_Parent`, `DefaultDir` FROM `Directory`"))
                {
                    Record directoryRecord;

                    while (null != (directoryRecord = directoryView.Fetch()))
                    {
                        using (directoryRecord)
                        {
                            string sourceName = Installer.GetName(directoryRecord.GetString(3), true, longNamesInImage);

                            directories.Add(directoryRecord.GetString(1), new ResolvedDirectory(directoryRecord.GetString(2), sourceName));
                        }
                    }
                }

                using (View fileView = db.OpenView("SELECT `Directory_`, `FileName` FROM `Component`, `File` WHERE `Component`.`Component`=`File`.`Component_` AND `File`.`File`=?"))
                {
                    using (Record fileQueryRecord = new Record(1))
                    {
                        // for each file in the array of uncompressed files
                        foreach (FileRow fileRow in fileRows)
                        {
                            string relativeFileLayoutPath = null;

                            string mediaLayoutDirectory = this.FileManager.ResolveMedia(mediaRows[fileRow.DiskId], layoutDirectory);

                            // setup up the query record and find the appropriate file in the
                            // previously executed file view
                            fileQueryRecord[1] = fileRow.File;
                            fileView.Execute(fileQueryRecord);

                            using (Record fileRecord = fileView.Fetch())
                            {
                                if (null == fileRecord)
                                {
                                    throw new WixException(WixErrors.FileIdentifierNotFound(fileRow.SourceLineNumbers, fileRow.File));
                                }

                                string fileName = Installer.GetName(fileRecord[2], true, longNamesInImage);

                                if (compressed)
                                {
                                    // use just the file name of the file since all uncompressed files must appear
                                    // in the root of the image in a compressed package
                                    relativeFileLayoutPath = fileName;
                                }
                                else
                                {
                                    // get the relative path of where we want the file to be layed out as specified
                                    // in the Directory table
                                    string directoryPath = GetDirectoryPath(directories, null, fileRecord[1], false);
                                    relativeFileLayoutPath = Path.Combine(directoryPath, fileName);
                                }
                            }

                            // strip off "SourceDir" if it's still on there
                            if (relativeFileLayoutPath.StartsWith("SourceDir\\", StringComparison.Ordinal))
                            {
                                relativeFileLayoutPath = relativeFileLayoutPath.Substring(10);
                            }

                            // finally put together the base media layout path and the relative file layout path
                            string fileLayoutPath = Path.Combine(mediaLayoutDirectory, relativeFileLayoutPath);
                            string sourceFullPath = null;
                            string fileLayoutFullPath = null;

                            try
                            {
                                sourceFullPath = Path.GetFullPath(fileRow.Source);
                            }
                            catch (System.ArgumentException)
                            {
                                throw new WixException(WixErrors.InvalidFileName(fileRow.SourceLineNumbers, fileRow.Source));
                            }
                            catch (System.IO.PathTooLongException)
                            {
                                throw new WixException(WixErrors.PathTooLong(fileRow.SourceLineNumbers, fileRow.Source));
                            }

                            try
                            {
                                fileLayoutFullPath = Path.GetFullPath(fileLayoutPath);
                            }
                            catch (System.ArgumentException)
                            {
                                throw new WixException(WixErrors.InvalidFileName(fileRow.SourceLineNumbers, fileLayoutPath));
                            }
                            catch (System.IO.PathTooLongException)
                            {
                                throw new WixException(WixErrors.PathTooLong(fileRow.SourceLineNumbers, fileLayoutPath));
                            }

                            // if the current source path (where we know that the file already exists) and the resolved
                            // path as dictated by the Directory table are not the same, then propagate the file.  The
                            // image that we create may have already been done by some other process other than the linker, so 
                            // there is no reason to copy the files to the resolved source if they are already there.
                            if (!String.Equals(sourceFullPath, fileLayoutFullPath, StringComparison.OrdinalIgnoreCase))
                            {
                                // just put the file in the transfers array, how anti-climatic
                                fileTransfers.Add(new FileTransfer(fileRow.Source, fileLayoutPath, false));
                            }
                        }
                    }
                }
            }
        }

        /// <summary>
        /// Sets the thead count to the number of processors if the current thread count is set to 0.
        /// </summary>
        /// <remarks>The thread count value must be greater than 0 otherwise and exception will be thrown.</remarks>
        private void SetCabbingThreadCount()
        {
            // default the number of cabbing threads to the number of processors if it wasn't specified
            if (0 == this.cabbingThreadCount)
            {
                string numberOfProcessors = System.Environment.GetEnvironmentVariable("NUMBER_OF_PROCESSORS");

                try
                {
                    if (null != numberOfProcessors)
                    {
                        this.cabbingThreadCount = Convert.ToInt32(numberOfProcessors, CultureInfo.InvariantCulture.NumberFormat);

                        if (0 >= this.cabbingThreadCount)
                        {
                            throw new WixException(WixErrors.IllegalEnvironmentVariable("NUMBER_OF_PROCESSORS", numberOfProcessors));
                        }
                    }
                    else // default to 1 if the environment variable is not set
                    {
                        this.cabbingThreadCount = 1;
                    }

                    this.core.OnMessage(WixVerboses.SetCabbingThreadCount(this.cabbingThreadCount.ToString()));
                }
                catch (ArgumentException)
                {
                    throw new WixException(WixErrors.IllegalEnvironmentVariable("NUMBER_OF_PROCESSORS", numberOfProcessors));
                }
                catch (FormatException)
                {
                    throw new WixException(WixErrors.IllegalEnvironmentVariable("NUMBER_OF_PROCESSORS", numberOfProcessors));
                }
            }
        }

        /// <summary>
        /// Structure used to hold a row and field that contain binder variables, which need to be resolved
        /// later, once the files have been resolved.
        /// </summary>
        private struct DelayedField
        {
            /// <summary>
            /// The row containing the field.
            /// </summary>
            public Row Row;

            /// <summary>
            /// The field needing further resolving.
            /// </summary>
            public Field Field;

            /// <summary>
            /// Basic constructor for struct
            /// </summary>
            /// <param name="row">Row for the field.</param>
            /// <param name="field">Field needing further resolution.</param>
            public DelayedField(Row row, Field field)
            {
                this.Row = row;
                this.Field = field;
            }
        }

        /// <summary>
        /// Structure used for resolved directory information.
        /// </summary>
        private struct ResolvedDirectory
        {
            /// <summary>The directory parent.</summary>
            public string DirectoryParent;

            /// <summary>The name of this directory.</summary>
            public string Name;

            /// <summary>The path of this directory.</summary>
            public string Path;

            /// <summary>
            /// Constructor for ResolvedDirectory.
            /// </summary>
            /// <param name="directoryParent">Parent directory.</param>
            /// <param name="name">The directory name.</param>
            public ResolvedDirectory(string directoryParent, string name)
            {
                this.DirectoryParent = directoryParent;
                this.Name = name;
                this.Path = null;
            }
        }

        /// <summary>
        /// Callback object for configurable merge modules.
        /// </summary>
        private sealed class ConfigurationCallback : IMsmConfigureModule
        {
            private const int SOk = 0x0;
            private const int SFalse = 0x1;
            private Hashtable configurationData;

            /// <summary>
            /// Creates a ConfigurationCallback object.
            /// </summary>
            /// <param name="configData">String to break up into name/value pairs.</param>
            public ConfigurationCallback(string configData)
            {
                if (String.IsNullOrEmpty(configData))
                {
                    throw new ArgumentNullException("configData");
                }

                string[] pairs = configData.Split(',');
                this.configurationData = new Hashtable(pairs.Length);
                for (int i = 0; i < pairs.Length; ++i)
                {
                    string[] nameVal = pairs[i].Split('=');
                    string name = nameVal[0];
                    string value = nameVal[1];

                    name = name.Replace("%2C", ",");
                    name = name.Replace("%3D", "=");
                    name = name.Replace("%25", "%");

                    value = value.Replace("%2C", ",");
                    value = value.Replace("%3D", "=");
                    value = value.Replace("%25", "%");

                    this.configurationData[name] = value;
                }
            }

            /// <summary>
            /// Returns text data based on name.
            /// </summary>
            /// <param name="name">Name of value to return.</param>
            /// <param name="configData">Out param to put configuration data into.</param>
            /// <returns>S_OK if value provided, S_FALSE if not.</returns>
            public int ProvideTextData(string name, out string configData)
            {
                if (this.configurationData.Contains(name))
                {
                    configData = (string)this.configurationData[name];
                    return SOk;
                }
                else
                {
                    configData = null;
                    return SFalse;
                }
            }

            /// <summary>
            /// Returns integer data based on name.
            /// </summary>
            /// <param name="name">Name of value to return.</param>
            /// <param name="configData">Out param to put configuration data into.</param>
            /// <returns>S_OK if value provided, S_FALSE if not.</returns>
            public int ProvideIntegerData(string name, out int configData)
            {
                if (this.configurationData.Contains(name))
                {
                    string val = (string)this.configurationData[name];
                    configData = Convert.ToInt32(val, CultureInfo.InvariantCulture);
                    return SOk;
                }
                else
                {
                    configData = 0;
                    return SFalse;
                }
            }
        }

        /// <summary>
        /// The types that the WixPatchSymbolPaths table can hold (and that the WixPatchSymbolPathsComparer can sort).
        /// </summary>
        internal enum SymbolPathType
        {
            File,
            Component,
            Directory,
            Media,
            Product
        };

        /// <summary>
        /// Sorts the WixPatchSymbolPaths table for processing.
        /// </summary>
        internal sealed class WixPatchSymbolPathsComparer : IComparer
        {
            /// <summary>
            /// Compares two rows from the WixPatchSymbolPaths table.
            /// </summary>
            /// <param name="a">First row to compare.</param>
            /// <param name="b">Second row to compare.</param>
            /// <remarks>Only the File, Product, Component, Directory, and Media tables links are allowed by this method.</remarks>
            /// <returns>Less than zero if a is less than b; Zero if they are equal, and Greater than zero if a is greater than b</returns>
            public int Compare(Object a, Object b)
            {
                Row ra = (Row)a;
                Row rb = (Row)b;

                SymbolPathType ia = (SymbolPathType)Enum.Parse(typeof(SymbolPathType), ((Field)ra.Fields[0]).Data.ToString());
                SymbolPathType ib = (SymbolPathType)Enum.Parse(typeof(SymbolPathType), ((Field)rb.Fields[0]).Data.ToString());
                return (int)ib - (int)ia;
            }
        }

        /// <summary>
        /// Payload info for binding Bundles.
        /// </summary>
        private class PayloadInfo
        {
            public PayloadInfo(Row row, BinderFileManager fileManager)
                : this((string)row[0], (string)row[1], (string)row[2], (string)row[3], fileManager)
            {
                this.SourceLineNumbers = row.SourceLineNumbers;
            }

            public PayloadInfo(string id, string name, string sourceFile, string downloadUrl, BinderFileManager fileManager)
            {
                this.Id = id;
                this.FileInfo = new FileInfo(fileManager.ResolveFile(sourceFile));
                this.FileName = !String.IsNullOrEmpty(name) ? name : this.FileInfo.Name;
                this.DownloadUrl = downloadUrl;
                this.Packaging = !String.IsNullOrEmpty(downloadUrl) ? PackagingType.Download : PackagingType.Unknown;
            }

            public SourceLineNumberCollection SourceLineNumbers { get; private set; }
            public string Id { get; private set; }
            public FileInfo FileInfo { get; private set; }
            public string FileName { get; private set; }
            public long FileSize { get { return this.FileInfo.Length; } }
            public string DownloadUrl { get; private set; }
            public string EmbeddedId { get; set; }

            public enum PackagingType
            {
                Unknown,
                Embedded,
                Download,
                External,
            }

            public PackagingType Packaging { get; set; }

            // TODO: Added in newburn... ContainerInfo, TargetName, and SourceType.

            private string sha1;
            public string Sha1
            {
                get
                {
                    if (String.IsNullOrEmpty(this.sha1))
                    {
                        byte[] hashBytes;
                        using (SHA1Managed managed = new SHA1Managed())
                        {
                            using (FileStream stream = this.FileInfo.OpenRead())
                            {
                                hashBytes = managed.ComputeHash(stream);
                            }
                        }

                        StringBuilder sb = new StringBuilder();
                        for (int i = 0; i < hashBytes.Length; i++)
                        {
                            sb.AppendFormat("{0:X2}", hashBytes[i]);
                        }
                        this.sha1 = sb.ToString();
                    }

                    return this.sha1;
                }
            }
        }

        /// <summary>
        /// Chain package info for binding Bundles.
        /// </summary>
        private class ChainPackageInfo
        {
            private const string propertySqlFormat = "SELECT `Value` FROM `Property` WHERE `Property` = '{0}'";

            public ChainPackageInfo(Row row, Table wixGroupTable, Dictionary<string, PayloadInfo> allPayloads, BinderFileManager fileManager, BinderCore core)
                : this((string)row[0], (string)row[1], (string)row[2], (string)row[3],
                       (string)row[4], (string)row[5], (string)row[6],
                       row[7], (string)row[8], row[9],
                       wixGroupTable, allPayloads, fileManager, core)
            {
                this.SourceLineNumbers = row.SourceLineNumbers;
            }

            public ChainPackageInfo(string id, string packageType, string payloadId, string installCondition,
                                    string installCommand, string repairCommand, string uninstallCommand,
                                    object cacheData, string cacheId, object vitalData,
                                    Table wixGroupTable, Dictionary<string, PayloadInfo> allPayloads, BinderFileManager fileManager, BinderCore core)
            {
                YesNoType cache = YesNoType.NotSet;
                if (null != cacheData)
                {
                    cache = (1 == (int)cacheData) ? YesNoType.Yes : YesNoType.No;
                }

                YesNoType vital = YesNoType.NotSet;
                if (null != vitalData)
                {
                    vital = (1 == (int)vitalData) ? YesNoType.Yes : YesNoType.No;
                }

                this.Id = id;
                this.ChainPackageType = (Compiler.ChainPackageType)Enum.Parse(typeof(Compiler.ChainPackageType), packageType, true);
                PayloadInfo packagePayload;
                if (!allPayloads.TryGetValue(payloadId, out packagePayload))
                {
                    core.OnMessage(WixErrors.IdentifierNotFound("Payload", payloadId));
                    return;
                }
                this.PackagePayload = packagePayload;
                this.InstallCondition = installCondition;
                this.InstallCommand = installCommand;
                this.RepairCommand = repairCommand;
                this.UninstallCommand = uninstallCommand;

                this.PerMachine = true;
                this.ProductCode = null;
                this.Register = false;
                this.Cache = (YesNoType.No != cache); // true unless it's specifically prohibited.
                this.CacheId = cacheId;
                this.Vital = (YesNoType.Yes == vital); // true only when specifically requested.
                this.Uninstall = false;

                this.Payloads = new List<PayloadInfo>();
                this.MsiProperties = new List<MsiPropertyInfo>();

                // get all contained payloads...
                foreach (Row row in wixGroupTable.Rows)
                {
                    string rowParentName = (string)row[0];
                    string rowParentType = (string)row[1];
                    string rowChildName = (string)row[2];
                    string rowChildType = (string)row[3];

                    if ("Package" == rowParentType && this.Id == rowParentName &&
                        "Payload" == rowChildType && this.PackagePayload.Id != rowChildName)
                    {
                        this.Payloads.Add(allPayloads[rowChildName]);
                    }
                }

                switch (this.ChainPackageType)
                {
                    case Compiler.ChainPackageType.Msi:
                        this.ResolveMsiPackage(fileManager, core);
                        break;
                    case Compiler.ChainPackageType.Msp:
                        this.ResolveMspPackage(core);
                        break;
                    case Compiler.ChainPackageType.Msu:
                        this.ResolveMsuPackage(core);
                        break;
                    case Compiler.ChainPackageType.Exe:
                        this.ResolveExePackage(core);
                        break;
                }
            }

            public SourceLineNumberCollection SourceLineNumbers { get; private set; }
            public string Id { get; private set; }
            public Compiler.ChainPackageType ChainPackageType { get; private set; }
            public PayloadInfo PackagePayload { get; private set; }
            public string InstallCondition { get; private set; }
            public string InstallCommand { get; private set; }
            public string RepairCommand { get; private set; }
            public string UninstallCommand { get; private set; }

            public bool PerMachine { get; private set; }
            public string ProductCode { get; private set; }
            public string PatchCode { get; private set; }
            public bool Register { get; private set; }
            public bool Cache { get; private set; }
            public string CacheId { get; private set; }
            public bool Vital { get; private set; }
            public bool Uninstall { get; private set; }

            public string DisplayName { get; private set; }
            public string Description { get; private set; }

            public RegistrationInfo RegistrationInfo { get; private set; }
            public List<PayloadInfo> Payloads { get; private set; }
            public List<MsiPropertyInfo> MsiProperties { get; private set; }

            /// <summary>
            /// Initializes package state from the MSI contents.
            /// </summary>
            /// <param name="core">BinderCore for messages.</param>
            private void ResolveMsiPackage(BinderFileManager fileManager, BinderCore core)
            {
                string sourcePath = this.PackagePayload.FileInfo.FullName;

                    // Read data out of the msi database...
                using (Microsoft.Deployment.WindowsInstaller.SummaryInfo sumInfo = new Microsoft.Deployment.WindowsInstaller.SummaryInfo(sourcePath, false))
                {
                    // 8 is the Word Count summary information stream bit that means
                    // "Elevated privileges are not required to install this package."
                    // in MSI 4.5 and below, if this bit is 0, elevation is required.
                    this.PerMachine = 0 == (sumInfo.WordCount & 8);
                }

                using (Microsoft.Deployment.WindowsInstaller.Database db = new Microsoft.Deployment.WindowsInstaller.Database(sourcePath))
                {
                    try
                    {
                        this.ProductCode = ChainPackageInfo.GetProperty(db, "ProductCode");
                        string productVersion = ChainPackageInfo.GetProperty(db, "ProductVersion");

                        if (String.IsNullOrEmpty(this.CacheId))
                        {
                            this.CacheId = String.Format("{0}v{1}", this.ProductCode, productVersion);
                        }

                        this.Uninstall = true;

                        // TODO: Should we get the InstallSize from ARPSIZE?
                        this.DisplayName = ChainPackageInfo.GetProperty(db, "ProductName");
                        this.Description = ChainPackageInfo.GetProperty(db, "ARPCOMMENTS");

                        // TODO: Burn supported a "Register" attribute on a package element, which
                        // would set this.Register to true.  Do we need a similar construct in
                        // Bundle/Chain/MsiPackage, or can we rely on the "ARPSYSTEMCOMPONENT"
                        // property?  (Burn also had a note about warning when the two conditions
                        // were in conflict.
                        this.Register = ChainPackageInfo.HasProperty(db, "ARPSYSTEMCOMPONENT");

                        if (this.Register)
                        {
                            this.RegistrationInfo = new RegistrationInfo();
                            this.RegistrationInfo.Name = ChainPackageInfo.GetProperty(db, "ProductName");
                            this.RegistrationInfo.Publisher = ChainPackageInfo.GetProperty(db, "Manufacturer");
                            this.RegistrationInfo.AboutUrl = ChainPackageInfo.GetProperty(db, "ARPURLINFOABOUT");
                            this.RegistrationInfo.HelpLink = ChainPackageInfo.GetProperty(db, "ARPHELPLINK");
                            this.RegistrationInfo.HelpTelephone = ChainPackageInfo.GetProperty(db, "ARPHELPTELEPHONE");
                            this.RegistrationInfo.UpdateUrl = ChainPackageInfo.GetProperty(db, "ARPURLUPDATEINFO");
                            this.RegistrationInfo.DisableModify = ChainPackageInfo.HasProperty(db, "ARPNOMODIFY");
                            this.RegistrationInfo.DisableRemove = ChainPackageInfo.HasProperty(db, "ARPNOREMOVE");
                            this.RegistrationInfo.DisableRepair = ChainPackageInfo.HasProperty(db, "ARPNOREPAIR");
                        }

                        // add all external cabinets as package resources
                        // TODO: Do we need to do the same with loose files?

                        foreach (string cabinet in db.ExecuteStringQuery("SELECT `Cabinet` FROM `Media`"))
                        {
                            if (!String.IsNullOrEmpty(cabinet) && !cabinet.StartsWith("#", StringComparison.Ordinal))
                            {
                                // Before adding the external file as another payload, we have to check to
                                // see if it's already in the payload list. To do this, we have to match the
                                // expected relative location of the external file specified in the MSI with
                                // the destination @Name of the payload... the @SourceFile path on the payload
                                // may be something completely different!
                                bool foundPayload = false;
                                foreach (PayloadInfo payload in this.Payloads)
                                {
                                    if (String.Equals(cabinet, payload.FileName, StringComparison.OrdinalIgnoreCase))
                                    {
                                        foundPayload = true;
                                        break;
                                    }
                                }

                                // If we didn't find the Payload as an existing child of the package, we need to
                                // add it.  We expect the file to exist on-disk in the same relative location as
                                // the MSI expects to find it...
                                if (!foundPayload)
                                {
                                    string payloadSourceFile = Path.Combine(this.PackagePayload.FileInfo.DirectoryName, cabinet);
                                    PayloadInfo payloadNew = new PayloadInfo(null, cabinet, payloadSourceFile, null, fileManager);
                                    this.Payloads.Add(payloadNew);
                                }
                            }
                        }
                    }
                    catch (Microsoft.Deployment.WindowsInstaller.InstallerException e)
                    {
                        core.OnMessage(WixErrors.UnexpectedException(e.Message, "InstallerException", e.StackTrace));
                    }
                }
            }

            /// <summary>
            /// Initializes package state from the MSP contents.
            /// </summary>
            /// <param name="core">BinderCore for messages.</param>
            private void ResolveMspPackage(BinderCore core)
            {
                string sourcePath = this.PackagePayload.FileInfo.FullName;
                
                // Read data out of the msp database...
                using (Microsoft.Deployment.WindowsInstaller.SummaryInfo sumInfo = new Microsoft.Deployment.WindowsInstaller.SummaryInfo(sourcePath, false))
                {
                    this.PatchCode = sumInfo.RevisionNumber.Substring(0, 38);
                }

                if (String.IsNullOrEmpty(this.CacheId))
                {
                    this.CacheId = this.PatchCode;
                }
                this.PerMachine = true;
                this.Uninstall = true;
            }

            /// <summary>
            /// Initializes package state from the MSU contents.
            /// </summary>
            /// <param name="core">BinderCore for messages.</param>
            private void ResolveMsuPackage(BinderCore core)
            {
                // TODO: get MSU info?

                // MSUs are a variant of an Exe...
                this.ResolveExePackage(core);
            }

            /// <summary>
            /// Initializes package state from the EXE contents.
            /// </summary>
            /// <param name="core">BinderCore for messages.</param>
            private void ResolveExePackage(BinderCore core)
            {
                if (String.IsNullOrEmpty(this.CacheId))
                {
                    this.CacheId = this.PackagePayload.Sha1;
                }
                this.PerMachine = true;
                this.Uninstall = true;

                FileVersionInfo versionInfo = FileVersionInfo.GetVersionInfo(this.PackagePayload.FileInfo.FullName);
                this.DisplayName = versionInfo.ProductName;
                this.Description = versionInfo.FileDescription;
            }

            /// <summary>
            /// Queries a Windows Installer database to determine if one or more rows exist in the Property table.
            /// </summary>
            /// <param name="db">Database to query.</param>
            /// <param name="property">Property to examine.</param>
            /// <returns>True if query matches at least one result.</returns>
            private static bool HasProperty(Microsoft.Deployment.WindowsInstaller.Database db, string property)
            {
                try
                {
                    return 0 < db.ExecuteQuery(PropertyQuery(property)).Count;
                }
                catch (Microsoft.Deployment.WindowsInstaller.InstallerException)
                {
                }

                return false;
            }

            /// <summary>
            /// Queries a Windows Installer database for a Property value.
            /// </summary>
            /// <param name="db">Database to query.</param>
            /// <param name="property">Property to examine.</param>
            /// <returns>String value for result or null if query doesn't match a single result.</returns>
            private static string GetProperty(Microsoft.Deployment.WindowsInstaller.Database db, string property)
            {
                try
                {
                    return db.ExecuteScalar(PropertyQuery(property)).ToString();
                }
                catch (Microsoft.Deployment.WindowsInstaller.InstallerException)
                {
                }

                return null;
            }

            private static string PropertyQuery(string property)
            {
                // quick sanity check that we'll be creating a valid query...
                // TODO: Are there any other special characters we should be looking for?
                Debug.Assert(!property.Contains("'"));
                return String.Format(CultureInfo.InvariantCulture, propertySqlFormat, property);
            }
        }

        /// <summary>
        /// Add/Remove Programs registration for the bundle.
        /// </summary>
        private class RegistrationInfo
        {
            public string Name { get; set; }
            public string Publisher { get; set; }
            public string HelpLink { get; set; }
            public string HelpTelephone { get; set; }
            public string AboutUrl { get; set; }
            public string UpdateUrl { get; set; }
            public bool DisableModify { get; set; }
            public bool DisableRepair { get; set; }
            public bool DisableRemove { get; set; }
        }

        /// <summary>
        /// Utility class for Burn MsiProperty information.
        /// </summary>
        private class MsiPropertyInfo
        {
            public MsiPropertyInfo(Row row)
                : this((string)row[0], (string)row[1], (string)row[2])
            {
            }

            public MsiPropertyInfo(string packageId, string name, string value)
            {
                this.PackageId = packageId;
                this.Name = name;
                this.Value = value;
            }

            public string PackageId { get; private set; }
            public string Name { get; private set; }
            public string Value { get; private set; }
        }

        /// <summary>
        /// Utility class for Burn variable information.
        /// </summary>
        private class VariableInfo
        {
            public VariableInfo(Row row)
                : this((string)row[0], (string)row[1], (string)row[2])
            {
            }

            public VariableInfo(string id, string value, string type)
            {
                this.Id = id;
                this.Value = value;
                this.Type = type;
            }

            public string Id { get; private set; }
            public string Value { get; private set; }
            public string Type { get; private set; }

            /// <summary>
            /// Generates Burn manifest and ParameterInfo-style markup for a variable.
            /// </summary>
            /// <param name="writer"></param>
            public void WriteXml(XmlTextWriter writer)
            {
                writer.WriteStartElement("Variable");
                writer.WriteAttributeString("Id", this.Id);
                writer.WriteAttributeString("Value", this.Value);
                writer.WriteAttributeString("Type", this.Type);
                writer.WriteEndElement();
            }
        }

        /// <summary>
        /// Utility base class for all WixSearches
        /// </summary>
        private abstract class WixSearchInfo
        {
            public WixSearchInfo(string id)
            {
                this.Id = id;
            }

            public void AddWixSearchRowInfo(Row row)
            {
                Debug.Assert((string)row[0] == Id);
                Variable = (string)row[1];
                Condition = (string)row[2];
            }

            public string Id { get; private set; }
            public string Variable { get; private set; }
            public string Condition { get; private set; }

            /// <summary>
            /// Generates Burn manifest and ParameterInfo-style markup a search.
            /// </summary>
            /// <param name="writer"></param>
            public virtual void WriteXml(XmlTextWriter writer)
            {
            }

            /// <summary>
            /// Writes attributes common to all WixSearch elements.
            /// </summary>
            /// <param name="writer"></param>
            protected void WriteWixSearchAttributes(XmlTextWriter writer)
            {
                writer.WriteAttributeString("Id", this.Id);
                writer.WriteAttributeString("Variable", this.Variable);
                if (!String.IsNullOrEmpty(this.Condition))
                {
                    writer.WriteAttributeString("Condition", this.Condition);
                }
            }
        }

        /// <summary>
        /// Utility class for all WixFileSearches (file and directory searches)
        /// </summary>
        private class WixFileSearchInfo : WixSearchInfo
        {
            public WixFileSearchInfo(Row row)
                : this((string)row[0], (string)row[1], (int)row[9])
            {
            }

            public WixFileSearchInfo(string id, string path, int attributes)
                : base(id)
            {
                this.Path = path;
                this.Attributes = (WixFileSearchAttributes)attributes;
            }

            public string Path { get; private set; }
            public WixFileSearchAttributes Attributes { get; private set; }

            /// <summary>
            /// Generates Burn manifest and ParameterInfo-style markup for a file/directory search.
            /// </summary>
            /// <param name="writer"></param>
            public override void WriteXml(XmlTextWriter writer)
            {
                writer.WriteStartElement((0 == (this.Attributes & WixFileSearchAttributes.IsDirectory)) ? "FileSearch" : "DirectorySearch");
                this.WriteWixSearchAttributes(writer);
                writer.WriteAttributeString("Path", this.Path);
                writer.WriteAttributeString("Type", (0 == (this.Attributes & WixFileSearchAttributes.WantVersion)) ? "exists" : "version");
                writer.WriteEndElement();
            }
        }

        /// <summary>
        /// Utility class for all WixRegistrySearches
        /// </summary>
        private class WixRegistrySearchInfo : WixSearchInfo
        {
            public WixRegistrySearchInfo(Row row)
                : this((string)row[0], (int)row[1], (string)row[2], (string)row[3], (int)row[4])
            {
            }

            public WixRegistrySearchInfo(string id, int root, string key, string value, int attributes)
                : base(id)
            {
                this.Root = root;
                this.Key = key;
                this.Value = value;
                this.Attributes = (WixRegistrySearchAttributes)attributes;
            }

            public int Root { get; private set; }
            public string Key { get; private set; }
            public string Value { get; private set; }
            public WixRegistrySearchAttributes Attributes { get; private set; }

            /// <summary>
            /// Generates Burn manifest and ParameterInfo-style markup for a registry search.
            /// </summary>
            /// <param name="writer"></param>
            public override void WriteXml(XmlTextWriter writer)
            {
                writer.WriteStartElement("RegistrySearch");
                this.WriteWixSearchAttributes(writer);

                switch (this.Root)
                {
                    case Msi.Interop.MsiInterop.MsidbRegistryRootClassesRoot:
                        writer.WriteAttributeString("Root", "HKCR");
                        break;
                    case Msi.Interop.MsiInterop.MsidbRegistryRootCurrentUser:
                        writer.WriteAttributeString("Root", "HKCU");
                        break;
                    case Msi.Interop.MsiInterop.MsidbRegistryRootLocalMachine:
                        writer.WriteAttributeString("Root", "HKLM");
                        break;
                    case Msi.Interop.MsiInterop.MsidbRegistryRootUsers:
                        writer.WriteAttributeString("Root", "HKU");
                        break;
                }

                writer.WriteAttributeString("Key", this.Key);

                if (!String.IsNullOrEmpty(this.Value))
                {
                    writer.WriteAttributeString("Value", this.Value);
                }

                bool existenceOnly = 0 != (this.Attributes & WixRegistrySearchAttributes.WantExists);

                writer.WriteAttributeString("Type", existenceOnly ? "exists" : "value");

                if (!existenceOnly)
                {
                    if (0 != (this.Attributes & WixRegistrySearchAttributes.ExpandEnvironmentVariables))
                    {
                        writer.WriteAttributeString("ExpandEnvironment", "yes");
                    }

                    // We *always* say this is VariableType="string". If we end up
                    // needing to be more specific, we will have to expand the "Format"
                    // attribute to allow "number" and "version".

                    writer.WriteAttributeString("VariableType", "string");
                }

                writer.WriteEndElement();
            }
        }

        /// <summary>
        /// Utility class for all WixComponentSearches
        /// </summary>
        private class WixComponentSearchInfo : WixSearchInfo
        {
            public WixComponentSearchInfo(Row row)
                : this((string)row[0], (string)row[1], (string)row[2], (int)row[3])
            {
            }

            public WixComponentSearchInfo(string id, string guid, string productCode, int attributes)
                : base(id)
            {
                this.Guid = guid;
                this.ProductCode = productCode;
                this.Attributes = (WixComponentSearchAttributes)attributes;
            }

            public string Guid { get; private set; }
            public string ProductCode { get; private set; }
            public WixComponentSearchAttributes Attributes { get; private set; }

            /// <summary>
            /// Generates Burn manifest and ParameterInfo-style markup for a component search.
            /// </summary>
            /// <param name="writer"></param>
            public override void WriteXml(XmlTextWriter writer)
            {
                writer.WriteStartElement("MsiComponentSearch");
                this.WriteWixSearchAttributes(writer);

                writer.WriteAttributeString("ComponentId", this.Guid);

                if (!String.IsNullOrEmpty(this.ProductCode))
                {
                    writer.WriteAttributeString("ProductCode", this.ProductCode);
                }

                if (0 != (this.Attributes & WixComponentSearchAttributes.KeyPath))
                {
                    writer.WriteAttributeString("Type", "keyPath");
                }
                else if (0 != (this.Attributes & WixComponentSearchAttributes.State))
                {
                    writer.WriteAttributeString("Type", "state");
                }
                else if (0 != (this.Attributes & WixComponentSearchAttributes.WantDirectory))
                {
                    writer.WriteAttributeString("Type", "directory");
                }

                writer.WriteEndElement();
            }
        }


        /// <summary>
        /// Utility class for all WixProductSearches
        /// </summary>
        private class WixProductSearchInfo : WixSearchInfo
        {
            public WixProductSearchInfo(Row row)
                : this((string)row[0], (string)row[1], (int)row[2])
            {
            }

            public WixProductSearchInfo(string id, string productCode, int attributes)
                : base(id)
            {
                this.ProductCode = productCode;
                this.Attributes = (WixProductSearchAttributes)attributes;
            }

            public string ProductCode { get; private set; }
            public WixProductSearchAttributes Attributes { get; private set; }

            /// <summary>
            /// Generates Burn manifest and ParameterInfo-style markup for a product search.
            /// </summary>
            /// <param name="writer"></param>
            public override void WriteXml(XmlTextWriter writer)
            {
                writer.WriteStartElement("MsiProductSearch");
                this.WriteWixSearchAttributes(writer);

                writer.WriteAttributeString("ProductCode", this.ProductCode);

                if (0 != (this.Attributes & WixProductSearchAttributes.Version))
                {
                    writer.WriteAttributeString("Type", "version");
                }
                else if (0 != (this.Attributes & WixProductSearchAttributes.Language))
                {
                    writer.WriteAttributeString("Type", "language");
                }
                else if (0 != (this.Attributes & WixProductSearchAttributes.State))
                {
                    writer.WriteAttributeString("Type", "state");
                }
                else if (0 != (this.Attributes & WixProductSearchAttributes.Assignment))
                {
                    writer.WriteAttributeString("Type", "assignment");
                }

                writer.WriteEndElement();
            }
        }
    }
}
