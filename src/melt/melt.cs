//-------------------------------------------------------------------------------------------------
// <copyright file="melt.cs" company="Microsoft">
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
// The merge module to ComponentGroup decompiler application.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Tools
{
    using System;
    using System.Collections.Specialized;
    using System.Diagnostics;
    using System.IO;
    using System.Globalization;
    using System.Reflection;
    using System.Runtime.InteropServices;
    using System.Xml;

    using Wix = Microsoft.Tools.WindowsInstallerXml.Serialize;

    /// <summary>
    /// Entry point for the melter
    /// </summary>
    public sealed class Melt
    {
        private string exportBasePath;
        private StringCollection extensionList;
        private StringCollection invalidArgs;
        private string id;
        private string inputFile;
        private ConsoleMessageHandler messageHandler;
        private string outputFile;
        private OutputType outputType;
        private bool showHelp;
        private bool showLogo;
        private bool tidy;

        /// <summary>
        /// Instantiate a new Melt class.
        /// </summary>
        private Melt()
        {
            this.extensionList = new StringCollection();
            this.invalidArgs = new StringCollection();
            this.messageHandler = new ConsoleMessageHandler("MELT", "melt.exe");
            this.showLogo = true;
            this.tidy = true;
            this.id = null;
        }

        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        /// <param name="args">Arguments to decompiler.</param>
        /// <returns>0 if sucessful, otherwise 1.</returns>
        public static int Main(string[] args)
        {
            AppCommon.PrepareConsoleForLocalization();
            Melt melt = new Melt();
            return melt.Run(args);
        }

        /// <summary>
        /// Main running method for the application.
        /// </summary>
        /// <param name="args">Commandline arguments to the application.</param>
        /// <returns>Returns the application error code.</returns>
        private int Run(string[] args)
        {
            Decompiler decompiler = null;
            Unbinder unbinder = null;
            Melter melter = null;

            try
            {
                // parse the command line
                this.ParseCommandLine(args);

                // exit if there was an error parsing the command line (otherwise the logo appears after error messages)
                if (this.messageHandler.EncounteredError)
                {
                    return this.messageHandler.LastErrorNumber;
                }

                if (null == this.inputFile || null == this.outputFile)
                {
                    this.showHelp = true;
                }

                if (this.showLogo)
                {
                    AppCommon.DisplayToolHeader();
                }

                if (this.showHelp)
                {
                    Console.WriteLine(MeltStrings.HelpMessage);
                    AppCommon.DisplayToolFooter();
                    return this.messageHandler.LastErrorNumber;
                }

                foreach (string parameter in this.invalidArgs)
                {
                    this.messageHandler.Display(this, WixWarnings.UnsupportedCommandLineArgument(parameter));
                }
                this.invalidArgs = null;

                // create the decompiler, unbinder, and melter
                decompiler = new Decompiler();
                unbinder = new Unbinder();
                melter = new Melter(decompiler, id);

                // read the configuration file (melt.exe.config)
                AppCommon.ReadConfiguration(this.extensionList);

                // load any extensions
                foreach (string extension in this.extensionList)
                {
                    WixExtension wixExtension = WixExtension.Load(extension);

                    decompiler.AddExtension(wixExtension);
                    unbinder.AddExtension(wixExtension);
                }

                // set options
                decompiler.TempFilesLocation = Environment.GetEnvironmentVariable("WIX_TEMP");

                unbinder.TempFilesLocation = Environment.GetEnvironmentVariable("WIX_TEMP");
                unbinder.SuppressDemodularization = true;

                decompiler.Message += new MessageEventHandler(this.messageHandler.Display);
                unbinder.Message += new MessageEventHandler(this.messageHandler.Display);
                melter.Message += new MessageEventHandler(this.messageHandler.Display);

                if (null == this.exportBasePath)
                {
                    this.exportBasePath = System.IO.Path.GetDirectoryName(this.outputFile);
                }

                // print friendly message saying what file is being decompiled
                Console.WriteLine(Path.GetFileName(this.inputFile));

                // unbind
                Output output = unbinder.Unbind(this.inputFile, this.outputType, this.exportBasePath);

                if (null != output)
                {
                    Wix.Wix wix = melter.Melt(output);
                    if (null != wix)
                    {
                        XmlTextWriter writer = null;

                        try
                        {
                            writer = new XmlTextWriter(this.outputFile, System.Text.Encoding.UTF8);

                            writer.Indentation = 4;
                            writer.IndentChar = ' ';
                            writer.QuoteChar = '"';
                            writer.Formatting = Formatting.Indented;

                            writer.WriteStartDocument();
                            wix.OutputXml(writer);
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
            }
            catch (WixException we)
            {
                this.messageHandler.Display(this, we.Error);
            }
            catch (Exception e)
            {
                this.messageHandler.Display(this, WixErrors.UnexpectedException(e.Message, e.GetType().ToString(), e.StackTrace));
                if (e is NullReferenceException || e is SEHException)
                {
                    throw;
                }
            }
            finally
            {
                if (null != decompiler)
                {
                    if (this.tidy)
                    {
                        if (!decompiler.DeleteTempFiles())
                        {
                            Console.WriteLine(MeltStrings.WAR_FailedToDeleteTempDir, decompiler.TempFilesLocation);
                        }
                    }
                    else
                    {
                        Console.WriteLine(MeltStrings.INF_TempDirLocatedAt, decompiler.TempFilesLocation);
                    }
                }

                if (null != unbinder)
                {
                    if (this.tidy)
                    {
                        if (!unbinder.DeleteTempFiles())
                        {
                            Console.WriteLine(MeltStrings.WAR_FailedToDeleteTempDir, unbinder.TempFilesLocation);
                        }
                    }
                    else
                    {
                        Console.WriteLine(MeltStrings.INF_TempDirLocatedAt, unbinder.TempFilesLocation);
                    }
                }
            }

            return this.messageHandler.LastErrorNumber;
        }

        /// <summary>
        /// Parse the commandline arguments.
        /// </summary>
        /// <param name="args">Commandline arguments.</param>
        private void ParseCommandLine(string[] args)
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

                    if ("ext" == parameter)
                    {
                        if (!CommandLine.IsValidArg(args, ++i))
                        {
                            this.messageHandler.Display(this, WixErrors.TypeSpecificationForExtensionRequired("-ext"));
                            return;
                        }

                        this.extensionList.Add(args[i]);
                    }
                    else if ("id" == parameter)
                    {
                        if (!CommandLine.IsValidArg(args, ++i))
                        {
                            this.messageHandler.Display(this, WixErrors.ExpectedArgument(String.Concat("-", parameter)));
                            return;
                        }

                        this.id = args[i];
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
                    else if ("x" == parameter)
                    {
                        this.exportBasePath = CommandLine.GetDirectory(parameter, this.messageHandler, args, ++i);

                        if (String.IsNullOrEmpty(this.exportBasePath))
                        {
                            return;
                        }
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
                else
                {
                    if (null == this.inputFile)
                    {
                        this.inputFile = CommandLine.VerifyPath(this.messageHandler, arg);

                        if (String.IsNullOrEmpty(this.inputFile))
                        {
                            return;
                        }

                        // guess the output type based on the extension of the input file
                        if (OutputType.Unknown == this.outputType)
                        {
                            string extension = Path.GetExtension(this.inputFile);

                            if (String.Equals(extension, ".msm", StringComparison.OrdinalIgnoreCase))
                            {
                                this.outputType = OutputType.Module;
                            }
                            else
                            {
                                this.messageHandler.Display(this, WixErrors.UnexpectedFileExtension(extension, ".msm"));
                                return;
                            }
                        }
                    }
                    else if (null == this.outputFile)
                    {
                        this.outputFile = CommandLine.VerifyPath(this.messageHandler, arg);

                        if (String.IsNullOrEmpty(this.outputFile))
                        {
                            return;
                        }
                    }
                    else
                    {
                        this.messageHandler.Display(this, WixErrors.AdditionalArgumentUnexpected(arg));
                    }
                }
            }
        }
    }
}
