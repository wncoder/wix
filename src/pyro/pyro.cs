//-------------------------------------------------------------------------------------------------
// <copyright file="pyro.cs" company="Microsoft">
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
// The pyro patch builder application.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Tools
{
    using System;
    using System.Collections;
    using System.Collections.Generic;
    using System.Collections.Specialized;
    using System.Diagnostics;
    using System.CodeDom.Compiler;
    using System.IO;
    using System.Globalization;
    using System.Reflection;
    using System.Runtime.InteropServices;
    using System.Xml;

    /// <summary>
    /// The pyro patch builder application.
    /// </summary>
    public sealed class Pyro
    {
        private Microsoft.Tools.WindowsInstallerXml.Binder binder;
        private string cabCachePath;
        private bool delta;
        private StringCollection extensions;
        private StringCollection invalidArgs;
        private string inputFile;
        private Dictionary<string, string> inputTransforms;
        private List<string> inputTransformsOrdered;
        private ConsoleMessageHandler messageHandler;
        private string outputFile;
        private string pdbFile;
        private bool reuseCabinets;
        private bool showHelp;
        private bool showLogo;
        private bool suppressAssemblies;
        private bool suppressFileHashAndInfo;
        private bool suppressFiles;
        private bool suppressWixPdb;
        private bool tidy;
        private WixVariableResolver wixVariableResolver;

        /// <summary>
        /// Instantiate a new Pyro class.
        /// </summary>
        private Pyro()
        {
            this.extensions = new StringCollection();
            this.invalidArgs = new StringCollection();
            this.messageHandler = new ConsoleMessageHandler("PYRO", "pyro.exe");
            this.showLogo = true;
            this.tidy = true;
            this.delta = false;
            this.inputTransforms = new Dictionary<string, string>();
            this.inputTransformsOrdered = new List<string>();

            // set the message handler
            this.Message += new MessageEventHandler(this.messageHandler.Display);

            this.wixVariableResolver = new WixVariableResolver();
            this.wixVariableResolver.Message += new MessageEventHandler(this.messageHandler.Display);
        }

        /// <summary>
        /// Event for messages.
        /// </summary>
        private event MessageEventHandler Message;

        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        /// <param name="args">Arguments to pyro.</param>
        /// <returns>0 if sucessful, otherwise 1.</returns>
        public static int Main(string[] args)
        {
            AppCommon.PrepareConsoleForLocalization();
            Pyro pyro = new Pyro();
            return pyro.Run(args);
        }

        /// <summary>
        /// Main running method for the application.
        /// </summary>
        /// <param name="args">Commandline arguments to the application.</param>
        /// <returns>Returns the application error code.</returns>
        private int Run(string[] args)
        {
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
                    Console.WriteLine(PyroStrings.HelpMessage);
                    AppCommon.DisplayToolFooter();
                    return this.messageHandler.LastErrorNumber;
                }

                foreach (string parameter in this.invalidArgs)
                {
                    this.messageHandler.Display(this, WixWarnings.UnsupportedCommandLineArgument(parameter));
                }
                this.invalidArgs = null;

                // Load in transforms
                ArrayList transforms = new ArrayList();
                foreach (string inputTransform in inputTransformsOrdered)
                {
                    PatchTransform patchTransform = new PatchTransform(inputTransform, inputTransforms[inputTransform]);
                    patchTransform.Message += new MessageEventHandler(this.messageHandler.Display);
                    transforms.Add(patchTransform);
                }

                // Create and configure the patch
                Patch patch = new Patch();
                patch.Message += new MessageEventHandler(this.messageHandler.Display);

                // Create and configure the binder
                binder = new Microsoft.Tools.WindowsInstallerXml.Binder();
                binder.TempFilesLocation = Environment.GetEnvironmentVariable("WIX_TEMP");
                binder.WixVariableResolver = this.wixVariableResolver;
                binder.Message += new MessageEventHandler(this.messageHandler.Display);
                binder.SuppressAssemblies = this.suppressAssemblies;
                binder.SuppressFileHashAndInfo = this.suppressFileHashAndInfo;

                // Load the extensions
                bool binderFileManagerLoaded = false;
                foreach (String extension in this.extensions)
                {
                    WixExtension wixExtension = WixExtension.Load(extension);
                    binder.AddExtension(wixExtension);
                    patch.AddExtension(wixExtension);

                    if (null != wixExtension.BinderFileManager)
                    {
                        if (binderFileManagerLoaded)
                        {
                            throw new ArgumentException(String.Format(CultureInfo.CurrentUICulture, PyroStrings.EXP_CannotLoadBinderFileManager, wixExtension.BinderFileManager.GetType().ToString(), binder.FileManager.ToString()), "ext");
                        }

                        binder.FileManager = wixExtension.BinderFileManager;
                        binderFileManagerLoaded = true;
                    }
                }

                // Load the patch
                patch.Load(this.inputFile);

                // Copy transforms into output
                if (0 < transforms.Count)
                {
                    patch.AttachTransforms(transforms);
                }

                if (this.messageHandler.EncounteredError)
                {
                    return this.messageHandler.LastErrorNumber;
                }

                if (null == this.pdbFile && null != this.outputFile)
                {
                    this.pdbFile = Path.ChangeExtension(this.outputFile, ".wixpdb");
                }

                binder.PdbFile = suppressWixPdb ? null : this.pdbFile;

                if (this.suppressFiles)
                {
                    binder.SuppressAssemblies = true;
                    binder.SuppressFileHashAndInfo = true;
                }

                if (null != this.cabCachePath || this.reuseCabinets)
                {
                    // ensure the cabinet cache path exists if we are going to use it
                    if (null != this.cabCachePath && !Directory.Exists(this.cabCachePath))
                    {
                        Directory.CreateDirectory(this.cabCachePath);
                    }
                }

                binder.FileManager.ReuseCabinets = this.reuseCabinets;
                binder.FileManager.CabCachePath = this.cabCachePath;
                binder.FileManager.Output = patch.PatchOutput;
                binder.FileManager.DeltaBinaryPatch = this.delta;

                // Bind the patch to an msp.
                binder.Bind(patch.PatchOutput, this.outputFile);
            }
            catch (WixException we)
            {
                this.OnMessage(we.Error);
            }
            catch (Exception e)
            {
                this.OnMessage(WixErrors.UnexpectedException(e.Message, e.GetType().ToString(), e.StackTrace));
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
                            Console.WriteLine(PyroStrings.WAR_FailedToDeleteTempDir, binder.TempFilesLocation);
                        }
                    }
                    else
                    {
                        Console.WriteLine(PyroStrings.INF_TempDirLocatedAt, binder.TempFilesLocation);
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
                    
                    if ("cc" == parameter)
                    {
                        this.cabCachePath = CommandLine.GetDirectory(parameter, this.messageHandler, args, ++i);

                        if (String.IsNullOrEmpty(this.cabCachePath))
                        {
                            return;
                        }
                    }
                    else if ("delta" == parameter)
                    {
                        this.delta = true;
                    }
                    else if ("ext" == parameter)
                    {
                        if (!CommandLine.IsValidArg(args, ++i))
                        {
                            this.messageHandler.Display(this, WixErrors.TypeSpecificationForExtensionRequired("-ext"));
                            return;
                        }

                        this.extensions.Add(args[i]);
                    }
                    else if ("nologo" == parameter)
                    {
                        this.showLogo = false;
                    }
                    else if ("notidy" == parameter)
                    {
                        this.tidy = false;
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
                    else if ("o" == parameter || "out" == parameter)
                    {
                        this.outputFile = CommandLine.GetFile(parameter, this.messageHandler, args, ++i);

                        if (String.IsNullOrEmpty(this.outputFile))
                        {
                            return;
                        }
                    }
                    else if ("pdbout" == parameter)
                    {
                        this.pdbFile = CommandLine.GetFile(parameter, this.messageHandler, args, ++i);

                        if (String.IsNullOrEmpty(this.pdbFile))
                        {
                            return;
                        }
                    }
                    else if ("reusecab" == parameter)
                    {
                        this.reuseCabinets = true;
                    }
                    else if ("sa" == parameter)
                    {
                        this.suppressAssemblies = true;
                    }
                    else if ("sf" == parameter)
                    {
                        this.suppressFiles = true;
                    }
                    else if ("sh" == parameter)
                    {
                        this.suppressFileHashAndInfo = true;
                    }
                    else if ("spdb" == parameter)
                    {
                        this.suppressWixPdb = true;
                    }
                    else if ("t" == parameter)
                    {
                        string transform = null;
                        string baseline = null;

                        if (!CommandLine.IsValidArg(args, ++i))
                        {
                            this.messageHandler.Display(this, WixErrors.BaselineRequired());
                            return;
                        }

                        baseline = args[i];

                        transform = CommandLine.GetFile(parameter, this.messageHandler, args, ++i);

                        if (String.IsNullOrEmpty(transform))
                        {
                            return;
                        }

                        // Verify the transform hasnt already been added.
                        if (this.inputTransforms.ContainsKey(transform))
                        {
                            this.messageHandler.Display(this, WixErrors.DuplicateTransform(transform));
                            return;
                        }

                        this.inputTransforms.Add(transform, baseline);
                        this.inputTransformsOrdered.Add(transform);
                    }
                    else if ("v" == parameter)
                    {
                        this.messageHandler.ShowVerboseMessages = true;
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
                    if (null == this.inputFile)
                    {
                        this.inputFile = CommandLine.VerifyPath(this.messageHandler, arg);

                        if (String.IsNullOrEmpty(this.inputFile))
                        {
                            return;
                        }
                    }
                    else
                    {
                        this.OnMessage(WixErrors.AdditionalArgumentUnexpected(arg));
                    }
                }
            }
        }

        /// <summary>
        /// Sends a message to the message delegate if there is one.
        /// </summary>
        /// <param name="mea">Message event arguments.</param>
        private void OnMessage(MessageEventArgs mea)
        {
            if (null != this.Message)
            {
                this.Message(this, mea);
            }
        }
    }
}
