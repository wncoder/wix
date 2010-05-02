//-------------------------------------------------------------------------------------------------
// <copyright file="light.cs" company="Microsoft">
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
// The light linker application.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Tools
{
    using System;
    using System.Collections;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
    using System.Collections.Specialized;
    using System.Diagnostics;
    using System.Diagnostics.CodeAnalysis;
    using System.Globalization;
    using System.IO;
    using System.Reflection;
    using System.Runtime.InteropServices;
    using System.Xml;
    using System.Xml.XPath;

    /// <summary>
    /// The main entry point for light.
    /// </summary>
    public sealed class Light
    {
        private string[] cultures;
        private bool allowIdenticalRows;
        private bool allowUnresolvedReferences;
        private bool backwardsCompatibleGuidGen;
        private bool bindFiles;
        private bool smartcabFastOverSmall;
        private string defaultCompressionLevel;
        private bool dropUnrealTables;
        private StringCollection ices;
        private StringCollection inputFiles;
        private StringCollection invalidArgs;
        private bool outputXml;
        private string pdbFile;
        private bool reuseCabinets;
        private bool sectionIdOnRows;
        private bool setMsiAssemblyNameFileVersion;
        private bool showHelp;
        private bool showLogo;
        private bool suppressAclReset;
        private bool suppressAdminSequence;
        private bool suppressAdvertiseSequence;
        private bool suppressAssemblies;
        private bool suppressFileHashAndInfo;
        private bool suppressFiles;
        private StringCollection suppressICEs;
        private bool suppressLayout;
        private bool suppressLocalization;
        private bool suppressMsiAssemblyTable;
        private bool suppressSchema;
        private bool suppressUISequence;
        private bool suppressValidation;
        private bool suppressVersionCheck;
        private bool suppressWixPdb;
        private bool tidy;
        private string outputFile;
        private ConsoleMessageHandler messageHandler;
        private string unreferencedSymbolsFile;
        private bool showPedanticMessages;
        private string cabCachePath;
        private StringCollection basePaths;
        private StringCollection extensionList;
        private StringCollection localizationFiles;
        private StringCollection sourcePaths;
        private WixVariableResolver wixVariableResolver;
        private int cabbingThreadCount;
        private Validator validator;

        /// <summary>
        /// Instantiate a new Light class.
        /// </summary>
        private Light()
        {
            this.basePaths = new StringCollection();
            this.extensionList = new StringCollection();
            this.localizationFiles = new StringCollection();
            this.messageHandler = new ConsoleMessageHandler("LGHT", "light.exe");
            this.ices = new StringCollection();
            this.inputFiles = new StringCollection();
            this.invalidArgs = new StringCollection();
            this.sourcePaths = new StringCollection();
            this.showLogo = true;
            this.suppressICEs = new StringCollection();
            this.tidy = true;
            this.sectionIdOnRows = true;

            this.wixVariableResolver = new WixVariableResolver();
            this.wixVariableResolver.Message += new MessageEventHandler(this.messageHandler.Display);

            this.validator = new Validator();
        }

        /// <summary>
        /// The main entry point for light.
        /// </summary>
        /// <param name="args">Commandline arguments for the application.</param>
        /// <returns>Returns the application error code.</returns>
        [MTAThread]
        public static int Main(string[] args)
        {
            AppCommon.PrepareConsoleForLocalization();
            Light light = new Light();
            return light.Run(args);
        }

        /// <summary>
        /// Main running method for the application.
        /// </summary>
        /// <param name="args">Commandline arguments to the application.</param>
        /// <returns>Returns the application error code.</returns>
        private int Run(string[] args)
        {
            Microsoft.Tools.WindowsInstallerXml.Binder binder = null;
            Linker linker = null;
            Localizer localizer = null;
            SectionCollection sections = new SectionCollection();
            ArrayList transforms = new ArrayList();

            try
            {
                // parse the command line
                this.ParseCommandLine(args);

                // exit if there was an error parsing the command line (otherwise the logo appears after error messages)
                if (this.messageHandler.EncounteredError)
                {
                    return this.messageHandler.LastErrorNumber;
                }

                if (0 == this.inputFiles.Count)
                {
                    this.showHelp = true;
                }
                else if (null == this.outputFile)
                {
                    if (1 < this.inputFiles.Count)
                    {
                        throw new WixException(WixErrors.MustSpecifyOutputWithMoreThanOneInput());
                    }

                    this.outputFile = Path.ChangeExtension(Path.GetFileName(this.inputFiles[0]), ".wix"); // we'll let the linker change the extension later
                }

                if (null == this.pdbFile && null != this.outputFile)
                {
                    this.pdbFile = Path.ChangeExtension(this.outputFile, ".wixpdb");
                }

                if (this.showLogo)
                {
                    AppCommon.DisplayToolHeader();
                }

                if (this.showHelp)
                {
                    Console.WriteLine(LightStrings.HelpMessage);
                    AppCommon.DisplayToolFooter();
                    return this.messageHandler.LastErrorNumber;
                }

                foreach (string parameter in this.invalidArgs)
                {
                    this.messageHandler.Display(this, WixWarnings.UnsupportedCommandLineArgument(parameter));
                }
                this.invalidArgs = null;

                // create the linker, binder, and validator
                linker = new Linker();
                binder = new Microsoft.Tools.WindowsInstallerXml.Binder();

                linker.AllowIdenticalRows = this.allowIdenticalRows;
                linker.AllowUnresolvedReferences = this.allowUnresolvedReferences;
                linker.Cultures = this.cultures;
                linker.UnreferencedSymbolsFile = this.unreferencedSymbolsFile;
                linker.ShowPedanticMessages = this.showPedanticMessages;
                linker.DropUnrealTables = this.dropUnrealTables;
                linker.SuppressLocalization = this.suppressLocalization;
                linker.SuppressMsiAssemblyTable = this.suppressMsiAssemblyTable;
                linker.WixVariableResolver = this.wixVariableResolver;

                // set the sequence suppression options
                linker.SuppressAdminSequence = this.suppressAdminSequence;
                linker.SuppressAdvertiseSequence = this.suppressAdvertiseSequence;
                linker.SuppressUISequence = this.suppressUISequence;

                linker.SectionIdOnRows = this.sectionIdOnRows;

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
                binder.BackwardsCompatibleGuidGen = this.backwardsCompatibleGuidGen;
                binder.SmartcabFastOverSmall = this.smartcabFastOverSmall;
                binder.CabbingThreadCount = this.cabbingThreadCount;

                binder.SuppressAclReset = this.suppressAclReset;
                binder.SetMsiAssemblyNameFileVersion = this.setMsiAssemblyNameFileVersion;
                binder.SuppressAssemblies = this.suppressAssemblies;
                binder.SuppressFileHashAndInfo = this.suppressFileHashAndInfo;

                if (this.suppressFiles)
                {
                    binder.SuppressAssemblies = true;
                    binder.SuppressFileHashAndInfo = true;
                }

                binder.PdbFile = this.suppressWixPdb ? null : this.pdbFile;

                binder.SuppressLayout = this.suppressLayout;
                binder.TempFilesLocation = Environment.GetEnvironmentVariable("WIX_TEMP");
                binder.WixVariableResolver = this.wixVariableResolver;

                if (!String.IsNullOrEmpty(this.defaultCompressionLevel))
                {
                    switch (this.defaultCompressionLevel)
                    {
                        case "low":
                            binder.DefaultCompressionLevel = Cab.CompressionLevel.Low;
                            break;
                        case "medium":
                            binder.DefaultCompressionLevel = Cab.CompressionLevel.Medium;
                            break;
                        case "high":
                            binder.DefaultCompressionLevel = Cab.CompressionLevel.High;
                            break;
                        case "none":
                            binder.DefaultCompressionLevel = Cab.CompressionLevel.None;
                            break;
                        case "mszip":
                            binder.DefaultCompressionLevel = Cab.CompressionLevel.Mszip;
                            break;
                        default:
                            throw new WixException(WixErrors.IllegalCompressionLevel(this.defaultCompressionLevel));
                    }
                }

                validator.TempFilesLocation = Environment.GetEnvironmentVariable("WIX_TEMP");

                if (null != this.cabCachePath || this.reuseCabinets)
                {
                    // ensure the cabinet cache path exists if we are going to use it
                    if (null != this.cabCachePath && !Directory.Exists(this.cabCachePath))
                    {
                        Directory.CreateDirectory(this.cabCachePath);
                    }
                }

                if (null != this.basePaths)
                {
                    foreach (string basePath in this.basePaths)
                    {
                        this.sourcePaths.Add(basePath);
                    }
                }

                // instantiate the localizer and load any localization files
                if (!this.suppressLocalization || 0 < this.localizationFiles.Count || null != this.cultures || !this.outputXml)
                {
                    List<Localization> localizations = new List<Localization>();
                    localizer = new Localizer();

                    localizer.Message += new MessageEventHandler(this.messageHandler.Display);

                    // load each localization file
                    foreach (string localizationFile in this.localizationFiles)
                    {
                        Localization localization = Localization.Load(localizationFile, linker.TableDefinitions, this.suppressSchema);
                        localizations.Add(localization);
                    }

                    if (null != this.cultures)
                    {
                        // add localizations in order specified in cultures
                        foreach (string culture in this.cultures)
                        {
                            foreach (Localization localization in localizations)
                            {
                                if (culture.Equals(localization.Culture, StringComparison.OrdinalIgnoreCase))
                                {
                                    localizer.AddLocalization(localization);
                                }
                            }
                        }
                    }
                    else 
                    {
                        bool neutralFound = false;
                        foreach (Localization localization in localizations)
                        {
                            if (0 == localization.Culture.Length)
                            {
                                // if a neutral wxl was provided use it
                                localizer.AddLocalization(localization);
                                neutralFound = true;
                            }
                        }

                        if (!neutralFound)
                        {
                            // cultures wasn't specified and no neutral wxl are available, include all of the files
                            foreach (Localization localization in localizations)
                            {
                                localizer.AddLocalization(localization);
                            }
                        }
                    }

                    // immediately stop processing if any errors were found
                    if (this.messageHandler.EncounteredError)
                    {
                        return this.messageHandler.LastErrorNumber;
                    }

                    // tell all of the objects about the localizer
                    linker.Localizer = localizer;
                    binder.Localizer = localizer;
                    this.wixVariableResolver.Localizer = localizer;
                }

                // load any extensions
                bool binderFileManagerLoaded = false;
                bool validatorExtensionLoaded = false;
                foreach (string extension in this.extensionList)
                {
                    WixExtension wixExtension = WixExtension.Load(extension);

                    linker.AddExtension(wixExtension);
                    binder.AddExtension(wixExtension);

                    // load the extension's localizations
                    Library library = wixExtension.GetLibrary(linker.TableDefinitions);
                    if (null != library)
                    {
                        // load the extension's default culture if it provides one and we don't specify any cultures
                        string[] extensionCultures = this.cultures;
                        if (null == extensionCultures && null != wixExtension.DefaultCulture)
                        {
                            extensionCultures = new string[] { wixExtension.DefaultCulture };
                        }

                        library.GetLocalizations(extensionCultures, localizer);
                    }

                    if (null != wixExtension.BinderFileManager)
                    {
                        if (binderFileManagerLoaded)
                        {
                            throw new ArgumentException(String.Format(CultureInfo.CurrentUICulture, LightStrings.EXP_CannotLoadBinderFileManager, wixExtension.BinderFileManager.GetType().ToString(), binder.FileManager.ToString()), "ext");
                        }

                        binder.FileManager = wixExtension.BinderFileManager;
                        binderFileManagerLoaded = true;
                    }

                    ValidatorExtension validatorExtension = wixExtension.ValidatorExtension;
                    if (null != validatorExtension)
                    {
                        if (validatorExtensionLoaded)
                        {
                            throw new ArgumentException(String.Format(CultureInfo.CurrentUICulture, LightStrings.EXP_CannotLoadLinkerExtension, validatorExtension.GetType().ToString(), validator.Extension.ToString()), "ext");
                        }

                        validator.Extension = validatorExtension;
                        validatorExtensionLoaded = true;
                    }
                }

                // set the message handlers
                linker.Message += new MessageEventHandler(this.messageHandler.Display);
                binder.Message += new MessageEventHandler(this.messageHandler.Display);
                validator.Extension.Message += new MessageEventHandler(this.messageHandler.Display);

                Output output = null;

                // loop through all the believed object files
                foreach (string inputFile in this.inputFiles)
                {
                    string dirName = Path.GetDirectoryName(inputFile);
                    string inputFileFullPath = Path.GetFullPath(inputFile);

                    if (!this.sourcePaths.Contains(dirName))
                    {
                        this.sourcePaths.Add(dirName);
                    }

                    // try loading as an object file
                    try
                    {
                        Intermediate intermediate = Intermediate.Load(inputFileFullPath, linker.TableDefinitions, this.suppressVersionCheck, this.suppressSchema);
                        sections.AddRange(intermediate.Sections);
                        continue; // next file
                    }
                    catch (WixNotIntermediateException)
                    {
                        // try another format
                    }

                    // try loading as a library file
                    try
                    {
                        Library library = Library.Load(inputFileFullPath, linker.TableDefinitions, this.suppressVersionCheck, this.suppressSchema);
                        library.GetLocalizations(this.cultures, localizer);
                        sections.AddRange(library.Sections);
                        continue; // next file
                    }
                    catch (WixNotLibraryException)
                    {
                        // try another format
                    }

                    // try loading as an output file
                    output = Output.Load(inputFileFullPath, this.suppressVersionCheck, this.suppressSchema);
                }

                // immediately stop processing if any errors were found
                if (this.messageHandler.EncounteredError)
                {
                    return this.messageHandler.LastErrorNumber;
                }

                // set the binder file manager information
                foreach (string basePath in this.basePaths)
                {
                    binder.FileManager.BasePaths.Add(basePath);
                }

                binder.FileManager.CabCachePath = this.cabCachePath;
                binder.FileManager.ReuseCabinets = this.reuseCabinets;

                foreach (string sourcePath in this.sourcePaths)
                {
                    binder.FileManager.SourcePaths.Add(sourcePath);
                }

                // and now for the fun part
                if (null == output)
                {
                    OutputType expectedOutputType = OutputType.Unknown;
                    if (this.outputFile != null)
                    {
                        expectedOutputType = Output.GetOutputType(Path.GetExtension(this.outputFile));
                    }

                    output = linker.Link(sections, transforms, expectedOutputType);

                    // if an error occurred during linking, stop processing
                    if (null == output)
                    {
                        return this.messageHandler.LastErrorNumber;
                    }
                }
                else if (0 != sections.Count)
                {
                    throw new InvalidOperationException(LightStrings.EXP_CannotLinkObjFilesWithOutpuFile);
                }

                // Now that the output object is either linked or loaded, tell the binder file manager about it.
                binder.FileManager.Output = output;

                // only output the xml if its a patch build or user specfied to only output wixout
                if (this.outputXml || OutputType.Patch == output.Type)
                {
                    string outputExtension = Path.GetExtension(this.outputFile);
                    if (null == outputExtension || 0 == outputExtension.Length || ".wix" == outputExtension)
                    {
                        if (OutputType.Patch == output.Type)
                        {
                            this.outputFile = Path.ChangeExtension(this.outputFile, ".wixmsp");
                        }
                        else
                        {
                            this.outputFile = Path.ChangeExtension(this.outputFile, ".wixout");
                        }
                    }

                    output.Save(this.outputFile, (this.bindFiles ? binder.FileManager : null), this.wixVariableResolver, binder.TempFilesLocation);
                }
                else // finish creating the MSI/MSM
                {
                    string outputExtension = Path.GetExtension(this.outputFile);
                    if (null == outputExtension || 0 == outputExtension.Length || ".wix" == outputExtension)
                    {
                        outputExtension = Output.GetExtension(output.Type);
                        this.outputFile = Path.ChangeExtension(this.outputFile, outputExtension);
                    }

                    // tell the binder about the validator if validation isn't suppressed
                    if (!this.suppressValidation && (OutputType.Module == output.Type || OutputType.Product == output.Type))
                    {
                        // set the default cube file
                        Assembly lightAssembly = Assembly.GetExecutingAssembly();
                        string lightDirectory = Path.GetDirectoryName(lightAssembly.Location);
                        if (OutputType.Module == output.Type)
                        {
                            validator.AddCubeFile(Path.Combine(lightDirectory, "mergemod.cub"));
                        }
                        else // product
                        {
                            validator.AddCubeFile(Path.Combine(lightDirectory, "darice.cub"));
                        }

                        // disable ICE33 and ICE66 by default
                        this.suppressICEs.Add("ICE33");
                        this.suppressICEs.Add("ICE66");

                        // set the ICEs
                        string[] iceArray = new string[this.ices.Count];
                        this.ices.CopyTo(iceArray, 0);
                        validator.ICEs = iceArray;

                        // set the suppressed ICEs
                        string[] suppressICEArray = new string[this.suppressICEs.Count];
                        this.suppressICEs.CopyTo(suppressICEArray, 0);
                        validator.SuppressedICEs = suppressICEArray;

                        binder.Validator = validator;
                    }

                    binder.Bind(output, this.outputFile);
                }
            }
            catch (WixException we)
            {
                if (we is WixInvalidIdtException)
                {
                    // make sure the IDT files stay around
                    this.tidy = false;
                }

                this.messageHandler.Display(this, we.Error);
            }
            catch (Exception e)
            {
                // make sure the files stay around for debugging
                this.tidy = false;

                this.messageHandler.Display(this, WixErrors.UnexpectedException(e.Message, e.GetType().ToString(), e.StackTrace));
                if (e is NullReferenceException || e is SEHException)
                {
                    throw;
                }
            }
            finally
            {
                if (null != binder)
                {
                    if (this.tidy)
                    {
                        if (!binder.DeleteTempFiles())
                        {
                            Console.WriteLine(LightStrings.WAR_FailedToDeleteTempDir, binder.TempFilesLocation);
                        }
                    }
                    else
                    {
                        Console.WriteLine(LightStrings.INF_BinderTempDirLocatedAt, binder.TempFilesLocation);
                    }
                }

                if (null != validator)
                {
                    if (this.tidy)
                    {
                        if (!validator.DeleteTempFiles())
                        {
                            Console.WriteLine(LightStrings.WAR_FailedToDeleteTempDir, validator.TempFilesLocation);
                        }
                    }
                    else
                    {
                        Console.WriteLine(LightStrings.INF_ValidatorTempDirLocatedAt, validator.TempFilesLocation);
                    }
                }
            }

            return this.messageHandler.LastErrorNumber;
        }

        /// <summary>
        /// Parse the commandline arguments.
        /// </summary>
        /// <param name="args">Commandline arguments.</param>
        [SuppressMessage("Microsoft.Globalization", "CA1308:NormalizeStringsToUppercase", Justification = "These strings are not round tripped, and have no security impact")]
        private void ParseCommandLine(string[] args)
        {
            for (int i = 0; i < args.Length; ++i)
            {
                string arg = args[i];
                if (null == arg || 0 == arg.Length) // skip blank arguments
                {
                    continue;
                }

                if (1 == arg.Length) // treat '-' and '@' as filenames when by themselves.
                {
                    this.inputFiles.AddRange(AppCommon.GetFiles(arg, "Source"));
                    continue;
                }

                if ('-' == arg[0] || '/' == arg[0])
                {
                    string parameter = arg.Substring(1);
                    if ("ai" == parameter)
                    {
                        this.messageHandler.Display(this, WixWarnings.DeprecatedCommandLineSwitch("ai"));
                        this.allowIdenticalRows = true;
                    }
                    else if ("au" == parameter)
                    {
                        this.messageHandler.Display(this, WixWarnings.DeprecatedCommandLineSwitch("au"));
                        this.allowUnresolvedReferences = true;
                    }
                    else if ("b" == parameter)
                    {
                        string path = CommandLine.GetDirectory(parameter, this.messageHandler, args, ++i);

                        if (String.IsNullOrEmpty(path))
                        {
                            return;
                        }

                        this.basePaths.Add(path);
                    }
                    else if ("bcgg" == parameter)
                    {
                        this.backwardsCompatibleGuidGen = true;
                    }
                    else if ("bf" == parameter)
                    {
                        this.bindFiles = true;
                    }
                    else if ("cc" == parameter)
                    {
                        this.cabCachePath = CommandLine.GetDirectory(parameter, this.messageHandler, args, ++i);

                        if (String.IsNullOrEmpty(this.cabCachePath))
                        {
                            return;
                        }
                    }
                    else if ("ct" == parameter)
                    {
                        if (!CommandLine.IsValidArg(args, ++i))
                        {
                            this.messageHandler.Display(this, WixErrors.IllegalCabbingThreadCount(String.Empty));
                            return;
                        }
                        try
                        {
                            this.cabbingThreadCount = Convert.ToInt32(args[i], CultureInfo.InvariantCulture.NumberFormat);

                            if (0 >= this.cabbingThreadCount)
                            {
                                this.messageHandler.Display(this, WixErrors.IllegalCabbingThreadCount(args[i]));
                            }
                        }
                        catch (FormatException)
                        {
                            this.messageHandler.Display(this, WixErrors.IllegalCabbingThreadCount(args[i]));
                        }
                        catch (OverflowException)
                        {
                            this.messageHandler.Display(this, WixErrors.IllegalCabbingThreadCount(args[i]));
                        }
                    }
                    else if (parameter.StartsWith("cultures:", StringComparison.Ordinal))
                    {
                        string culturesString = arg.Substring(10).ToLower(CultureInfo.InvariantCulture);
                        // When null is used treat it as if cultures wasn't specified.  
                        // This is needed for batching over the light task when using MSBuild 2.0 which doesn't 
                        // support empty items
                        if (culturesString.Equals("null", StringComparison.OrdinalIgnoreCase))
                        {
                            this.cultures = null;
                        }
                        else
                        {
                            this.cultures = culturesString.Split(';', ',');

                            for (int c = 0; c < this.cultures.Length; c++)
                            {
                                // Neutral is different from null. For neutral we still want to do WXL filtering.
                                // Set the culture to the empty string = identifier for the invariant culture
                                if (this.cultures[c].Equals("neutral", StringComparison.OrdinalIgnoreCase))
                                {
                                    this.cultures[c] = String.Empty;
                                }
                            }
                        }
                    }
                    else if ("cub" == parameter)
                    {
                        string cubeFile = CommandLine.GetFile(parameter, this.messageHandler, args, ++i);

                        if (String.IsNullOrEmpty(cubeFile))
                        {
                            return;
                        }

                        this.validator.AddCubeFile(cubeFile);
                    }
                    else if (parameter.StartsWith("dcl:", StringComparison.Ordinal))
                    {
                        this.defaultCompressionLevel = arg.Substring(5).ToLower(CultureInfo.InvariantCulture);
                    }
                    else if ("dut" == parameter)
                    {
                        this.dropUnrealTables = true;
                    }
                    else if (parameter.StartsWith("d", StringComparison.Ordinal))
                    {
                        parameter = arg.Substring(2);
                        string[] value = parameter.Split("=".ToCharArray(), 2);

                        if (1 == value.Length)
                        {
                            this.messageHandler.Display(this, WixErrors.ExpectedWixVariableValue(value[0]));
                        }
                        else
                        {
                            this.wixVariableResolver.AddVariable(value[0], value[1]);
                        }
                    }
                    else if ("ext" == parameter)
                    {
                        if (!CommandLine.IsValidArg(args, ++i))
                        {
                            this.messageHandler.Display(this, WixErrors.TypeSpecificationForExtensionRequired("-ext"));
                            return;
                        }

                        this.extensionList.Add(args[i]);
                    }
                    else if ("fv" == parameter)
                    {
                        this.setMsiAssemblyNameFileVersion = true;
                    }
                    else if (parameter.StartsWith("ice:", StringComparison.Ordinal))
                    {
                        this.ices.Add(parameter.Substring(4));
                    }
                    else if ("loc" == parameter)
                    {
                        string locFile = CommandLine.GetFile(parameter, this.messageHandler, args, ++i);

                        if (String.IsNullOrEmpty(locFile))
                        {
                            return;
                        }

                        this.localizationFiles.Add(locFile);
                    }
                    else if ("nologo" == parameter)
                    {
                        this.showLogo = false;
                    }
                    else if ("notidy" == parameter)
                    {
                        this.tidy = false;
                    }
                    else if ("o" == parameter || "out" == parameter)
                    {
                        this.outputFile = CommandLine.GetFile(parameter, this.messageHandler, args, ++i);

                        if (String.IsNullOrEmpty(this.outputFile))
                        {
                            return;
                        }
                    }
                    else if ("O1" == parameter)
                    {
                        this.smartcabFastOverSmall = false;
                    }
                    else if ("O2" == parameter)
                    {
                        this.smartcabFastOverSmall = true;
                    }
                    else if ("pdbout" == parameter)
                    {
                        this.pdbFile = CommandLine.GetFile(parameter, this.messageHandler, args, ++i);

                        if (String.IsNullOrEmpty(this.pdbFile))
                        {
                            return;
                        }
                    }
                    else if ("pedantic" == parameter)
                    {
                        this.showPedanticMessages = true;
                    }
                    else if ("reusecab" == parameter)
                    {
                        this.reuseCabinets = true;
                    }
                    else if ("sa" == parameter)
                    {
                        this.suppressAssemblies = true;
                    }
                    else if ("sacl" == parameter)
                    {
                        this.suppressAclReset = true;
                    }
                    else if ("sadmin" == parameter)
                    {
                        this.suppressAdminSequence = true;
                    }
                    else if ("sadv" == parameter)
                    {
                        this.suppressAdvertiseSequence = true;
                    }
                    else if ("sma" == parameter)
                    {
                        this.suppressMsiAssemblyTable = true;
                    }
                    else if ("sf" == parameter)
                    {
                        this.suppressFiles = true;
                    }
                    else if ("sh" == parameter)
                    {
                        this.suppressFileHashAndInfo = true;
                    }
                    else if (parameter.StartsWith("sice:", StringComparison.Ordinal))
                    {
                        this.suppressICEs.Add(parameter.Substring(5));
                    }
                    else if ("sl" == parameter)
                    {
                        this.suppressLayout = true;
                    }
                    else if ("sloc" == parameter)
                    {
                        this.suppressLocalization = true;
                    }
                    else if ("spdb" == parameter)
                    {
                        this.suppressWixPdb = true;
                    }
                    else if ("ss" == parameter)
                    {
                        this.suppressSchema = true;
                    }
                    else if ("sts" == parameter)
                    {
                        this.sectionIdOnRows = false;
                    }
                    else if ("sui" == parameter)
                    {
                        this.suppressUISequence = true;
                    }
                    else if ("sv" == parameter)
                    {
                        this.suppressVersionCheck = true;
                    }
                    else if ("sval" == parameter)
                    {
                        this.suppressValidation = true;
                    }
                    else if ("swall" == parameter)
                    {
                        this.messageHandler.Display(this, WixWarnings.DeprecatedCommandLineSwitch("swall", "sw"));
                        this.messageHandler.SuppressAllWarnings = true;
                    }
                    else if (parameter.StartsWith("sw", StringComparison.Ordinal))
                    {
                        string paramArg = parameter.Substring(2);
                        try
                        {
                            if (0 == paramArg.Length)
                            {
                                this.messageHandler.SuppressAllWarnings = true;
                            }
                            else
                            {
                                int suppressWarning = Convert.ToInt32(paramArg, CultureInfo.InvariantCulture.NumberFormat);
                                if (0 >= suppressWarning)
                                {
                                    this.messageHandler.Display(this, WixErrors.IllegalSuppressWarningId(paramArg));
                                }

                                this.messageHandler.SuppressWarningMessage(suppressWarning);
                            }
                        }
                        catch (FormatException)
                        {
                            this.messageHandler.Display(this, WixErrors.IllegalSuppressWarningId(paramArg));
                        }
                        catch (OverflowException)
                        {
                            this.messageHandler.Display(this, WixErrors.IllegalSuppressWarningId(paramArg));
                        }
                    }
                    else if ("usf" == parameter)
                    {
                        this.unreferencedSymbolsFile = CommandLine.GetFile(parameter, this.messageHandler, args, ++i);

                        if (String.IsNullOrEmpty(this.unreferencedSymbolsFile))
                        {
                            return;
                        }
                    }
                    else if ("wxall" == parameter)
                    {
                        this.messageHandler.Display(this, WixWarnings.DeprecatedCommandLineSwitch("wxall", "wx"));
                        this.messageHandler.WarningAsError = true;
                    }
                    else if (parameter.StartsWith("wx", StringComparison.Ordinal))
                    {
                        string paramArg = parameter.Substring(2);
                        try
                        {
                            if (0 == paramArg.Length)
                            {
                                this.messageHandler.WarningAsError = true;
                            }
                            else
                            {
                                int elevateWarning = Convert.ToInt32(paramArg, CultureInfo.InvariantCulture.NumberFormat);
                                if (0 >= elevateWarning)
                                {
                                    this.messageHandler.Display(this, WixErrors.IllegalWarningIdAsError(paramArg));
                                }

                                this.messageHandler.ElevateWarningMessage(elevateWarning);
                            }
                        }
                        catch (FormatException)
                        {
                            this.messageHandler.Display(this, WixErrors.IllegalWarningIdAsError(paramArg));
                        }
                        catch (OverflowException)
                        {
                            this.messageHandler.Display(this, WixErrors.IllegalWarningIdAsError(paramArg));
                        }
                    }
                    else if ("v" == parameter)
                    {
                        this.messageHandler.ShowVerboseMessages = true;
                    }
                    else if ("xo" == parameter)
                    {
                        this.outputXml = true;
                    }
                    else if ("?" == parameter || "help" == parameter)
                    {
                        this.showHelp = true;
                        return;
                    }
                    else
                    {
                        this.invalidArgs.Add(parameter);
                    }
                }
                else if ('@' == arg[0])
                {
                    this.ParseCommandLine(CommandLineResponseFile.Parse(arg.Substring(1)));
                }
                else
                {
                    this.inputFiles.AddRange(AppCommon.GetFiles(arg, "Source"));
                }
            }

            if (this.bindFiles && !this.outputXml)
            {
                throw new ArgumentException(LightStrings.EXP_BindFileOptionNotApplicable);
            }
        }
    }
}
