//-------------------------------------------------------------------------------------------------
// <copyright file="shine.cs" company="Microsoft">
//    Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// 
// <summary>
// Windows Installer Xml toolset scanner tool.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml
{
    using System;
    using System.Collections.Generic;
    using System.IO;
    using System.Xml.Linq;

    public class Shine
    {
        public static readonly XNamespace XDgmlNamespace = "http://schemas.microsoft.com/vs/2009/dgml";

        [Flags]
        private enum GroupType
        {
            None = 0,
            Projects = 1,
            Files = 2,
        }

        [Flags]
        private enum ShowType
        {
            None = 0,
            Projects = 1,
            Files = 2,
            Symbols = 4,
            References = 8,
            All = 15,
        }

        public static void Main(string[] args)
        {
            AppCommon.PrepareConsoleForLocalization();

            string dgml = null;
            string dgmlTemplate = null;
            bool showHelp = false;
            bool showLogo = true;

            GroupType group = GroupType.None;
            ShowType show = ShowType.All;
            List<string> paths = new List<string>();

            Scanner scanner = new Scanner();
            scanner.RecurseProjects = true;

            for (int i = 0; i < args.Length; ++i)
            {
                if (showHelp)
                {
                    break;
                }

                string arg = args[i];
                if (arg.StartsWith("-") || arg.StartsWith("/"))
                {
                    switch (arg.Substring(1).ToLowerInvariant())
                    {
                        case "dgml":
                            ++i;
                            dgml = args[i];
                            break;

                        case "dgmltemplate":
                            ++i;
                            dgmlTemplate = args[i];
                            break;

                        case "excludepath":
                        case "xp":
                            ++i;
                            //scanner.ExcludePaths.Add(Path.GetFullPath(args[i]));
                            break;

                        case "excludesymbol":
                        case "xs":
                            ++i;
                            //scanner.ExcludeSymbols.Add(args[i]);
                            break;

                        case "includesymbol":
                        case "is":
                            ++i;
                            //scanner.IncludeSymbols.Add(args[i]);
                            break;

                        case "nologo":
                            showLogo = false;
                            break;

                        case "srp":
                            scanner.RecurseProjects = false;
                            break;

                        case "help":
                        case "?":
                            showHelp = true;
                            break;

                        case "group":
                            ++i;
                            string[] groupNames = args[i].ToLowerInvariant().Split(new char[] { ';' }, StringSplitOptions.RemoveEmptyEntries);
                            foreach (string groupName in groupNames)
                            {
                                switch (groupName)
                                {
                                    case "proj":
                                    case "projs":
                                    case "project":
                                    case "projects":
                                        group |= GroupType.Projects;
                                        break;

                                    case "file":
                                    case "files":
                                        group |= GroupType.Files;
                                        break;
                                }
                            }
                            break;

                        case "show":
                            ++i;
                            show = ShowType.None;
                            string[] showNames = args[i].ToLowerInvariant().Split(new char[] { ';' }, StringSplitOptions.RemoveEmptyEntries);
                            foreach (string showName in showNames)
                            {
                                switch (showName)
                                {
                                    case "all":
                                        show |= ShowType.All;
                                        break;

                                    case "proj":
                                    case "projs":
                                    case "project":
                                    case "projects":
                                        show |= ShowType.Projects;
                                        break;

                                    case "file":
                                    case "files":
                                        show |= ShowType.Files;
                                        break;

                                    case "sym":
                                    case "syms":
                                    case "symbol":
                                    case "symbols":
                                        show |= ShowType.Symbols;
                                        break;

                                    case "ref":
                                    case "refs":
                                    case "reference":
                                    case "references":
                                        show |= ShowType.References;
                                        break;
                                }
                            }
                            break;

                        default:
                            Console.WriteLine("Unknown command line parameter: {0}", arg);
                            break;
                    }
                }
                else if (Directory.Exists(arg) || File.Exists(arg))
                {
                    paths.Add(Path.GetFullPath(arg));
                }
                else
                {
                    Console.WriteLine("Unknown command line parameter: {0}", arg);
                }
            }

            if (showLogo)
            {
                AppCommon.DisplayToolHeader();
            }

            if (showHelp || paths.Count == 0)
            {
                Shine.ShowHelp();
                AppCommon.DisplayToolFooter();
                return;
            }

            // Execute the scan and display the results.
            ScanResult result = scanner.Scan(paths, null, null);
            if (String.IsNullOrEmpty(dgml))
            {
                Console.WriteLine("Displaying graph to console is not supported yet. Use the -dgml switch.");
            }
            else
            {
                Shine.SaveDgml(result, group, show, dgmlTemplate, dgml);
            }
        }

        private static void ShowHelp()
        {
            //                 12345678901234567890123456789012345678901234567890123456789012345678901234567890
            Console.WriteLine(" usage: shine.exe [options] path|*.wixproj|*.wixpdb|...");
            Console.WriteLine("   -dgml file               save scan as DGML file");
            Console.WriteLine("   -dgmlTemplate file       a valid DGML file populated with data from scan");
            //Console.WriteLine("   -excludePath file|dir  remove file or directory from scan");
            //Console.WriteLine("   -excludeSymbol symbol  remove symbol and its referenced symbols from scan");
            //Console.WriteLine("   -includeSymbol symbol  filter scan using symbol");
            //Console.WriteLine("                            by default all symbols are returned");
            //Console.WriteLine("   -p <name>=<value>        define a property when loading MSBuild projects");
            Console.WriteLine("   -show proj;file;sym;ref  displays only the specified items in the scan");
            Console.WriteLine("                              proj - project files");
            Console.WriteLine("                              file - source files");
            Console.WriteLine("                              sym  - symbols");
            Console.WriteLine("                              ref  - symbol references");
            Console.WriteLine("                              all  - all of the above [default]");
            Console.WriteLine("   -? | -help             this help information");
            Console.WriteLine();
            Console.WriteLine("shine.exe scans directories, .wixproj files and .wixpdbs for WiX items such as:");
            Console.WriteLine("Features, ComponentGroups, Components and the references between them.");
            //Console.WriteLine("The resulting graph can be filtered by including and/or excluding symbols before");
            //Console.WriteLine("displaying to the console or saving to a DGML file.");
            //Console.WriteLine();
            //Console.WriteLine("A \"symbol\" is specified by its \"type\" and \"id\" separated by a colon. For");
            //Console.WriteLine("example:");
            //Console.WriteLine("   -includeSymbol Feature:MyFeature");
            //Console.WriteLine("   -excludeSymbol Component:CompA -excludeSymbol Component:CompB");
            //Console.WriteLine("   -excludeSymbol ComponentGroup:ComponentGoup_$(var.PreprocVariable)");
        }

        private static void SaveDgml(ScanResult result, GroupType group, ShowType show, string templatePath, string outputPath)
        {
            XElement dg;
            XElement nodes;
            XElement links;
            if (String.IsNullOrEmpty(templatePath))
            {
                nodes = new XElement(XDgmlNamespace + "Nodes");
                links = new XElement(XDgmlNamespace + "Links");
                dg = new XElement(XDgmlNamespace + "DirectedGraph", nodes, links);
            }
            else // load from the provided template path.
            {
                dg = XElement.Load(templatePath);
                nodes = dg.Element(XDgmlNamespace + "Nodes");
                if (nodes == null)
                {
                    nodes = new XElement(XDgmlNamespace + "Nodes");
                    dg.Add(nodes);
                }

                links = dg.Element(XDgmlNamespace + "Links");
                if (links == null)
                {
                    links = new XElement(XDgmlNamespace + "Links");
                    dg.Add(links);
                }
            }

            // Draw the projects.
            if (ShowType.Projects == (show & ShowType.Projects))
            {
                Console.WriteLine("Graphing projects...");
                foreach (ScannedProject project in result.ProjectFiles.Values)
                {
                    XElement node = new XElement(XDgmlNamespace + "Node",
                                new XAttribute("Id", project.Key),
                                new XAttribute("Category", "ProjectFile"),
                                new XAttribute("Reference", project.Path),
                                new XElement(XDgmlNamespace + "Category",
                                    new XAttribute("Ref", String.Concat(project.Type, "Project"))
                                    )
                                );

                    if (GroupType.Projects == (group & GroupType.Projects))
                    {
                        node.Add(new XAttribute("Group", "collapsed"));
                    }

                    nodes.Add(node);
                }

                foreach (ScannedProjectProjectReference projectRef in result.ProjectToProjectReferences)
                {
                        links.Add(new XElement(XDgmlNamespace + "Link",
                                    new XAttribute("Category", "ProjectReference"),
                                    new XAttribute("Source", projectRef.SourceProject.Key),
                                    new XAttribute("Target", projectRef.TargetProject.Key)
                                    )
                            );
                }
            }

            // Draw the files.
            if (ShowType.Files == (show & ShowType.Files))
            {
                Console.WriteLine("Graphing files...");
                foreach (ScannedSourceFile file in result.SourceFiles.Values)
                {
                    XElement node = new XElement(XDgmlNamespace + "Node",
                                new XAttribute("Id", file.Key),
                                new XAttribute("Category", "SourceFile"),
                                new XAttribute("Reference", file.Path)
                                );

                    if (GroupType.Files == (group & GroupType.Files))
                    {
                        node.Add(new XAttribute("Group", "collapsed"));
                    }

                    nodes.Add(node);
                }

                if (ShowType.Projects == (show & ShowType.Projects))
                {
                    foreach (ScannedProjectSourceFileReference fileRef in result.ProjectToSourceFileReferences)
                    {
                        links.Add(new XElement(XDgmlNamespace + "Link",
                                    new XAttribute("Category", "CompilesFile"),
                                    new XAttribute("Source", fileRef.SourceProject.Key),
                                    new XAttribute("Target", fileRef.TargetSourceFile.Key),
                                    new XElement(XDgmlNamespace + "Category",
                                        new XAttribute("Ref", "Contains")
                                        )
                                    )
                            );
                    }
                }
            }

            // Draw the symbols.
            if (ShowType.Symbols == (show & ShowType.Symbols))
            {
                Console.WriteLine("Graphing symbols...");
                foreach (ScannedSymbol symbol in result.Symbols.Values)
                {
                    nodes.Add(new XElement(XDgmlNamespace + "Node",
                                new XAttribute("Id", symbol.Key),
                                new XAttribute("Category", symbol.Type),
                                new XAttribute("Reference", "TODO")
                                )
                        );
                }

                if (ShowType.References == (show & ShowType.References))
                {
                    Console.WriteLine("Graphing symbol references...");
                    foreach (ScannedSymbolSymbolReference symbolRef in result.SymbolToSymbolReference)
                    {
                        links.Add(new XElement(XDgmlNamespace + "Link",
                                    new XAttribute("Category", "SymbolReference"),
                                    new XAttribute("Source", symbolRef.SourceSymbol.Key),
                                    new XAttribute("Target", symbolRef.TargetSymbol.Key)
                                    )
                            );
                    }
                }

                if (ShowType.Files == (show & ShowType.Files))
                {
                    foreach (ScannedSourceFileSymbolReference fileSymbolRef in result.SourceFileToSymbolReference)
                    {
                        links.Add(new XElement(XDgmlNamespace + "Link",
                                    new XAttribute("Category", "DefinesSymbol"),
                                    new XAttribute("Source", fileSymbolRef.SourceSourceFile.Key),
                                    new XAttribute("Target", fileSymbolRef.TargetSymbol.Key),
                                    new XElement(XDgmlNamespace + "Category",
                                        new XAttribute("Ref", "Contains")
                                        )
                                    )
                            );
                    }
                }
            }

            dg.Save(outputPath, SaveOptions.None);
        }
    }
}
