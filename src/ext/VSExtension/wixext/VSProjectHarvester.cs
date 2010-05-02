//-------------------------------------------------------------------------------------------------
// <copyright file="VSProjectHarvester.cs" company="Microsoft">
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
// Harvest WiX authoring for outputs of a VS project.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Extensions
{
    using System;
    using System.IO;
    using System.Reflection;
    using System.Collections;
    using System.Globalization;
    using System.Runtime.InteropServices;

    // Instead of directly referencing this .NET 2.0 assembly,
    // use reflection to allow building against .NET 1.1.
    // (Successful runtime use of this class still requires .NET 2.0.)

    ////using Microsoft.Build.BuildEngine;

    using Wix = Microsoft.Tools.WindowsInstallerXml.Serialize;

    /// <summary>
    /// Harvest WiX authoring for the outputs of a VS project.
    /// </summary>
    public sealed class VSProjectHarvester : HarvesterExtension
    {
        // These format strings are used for generated element identifiers.
        //   {0} = project name
        //   {1} = POG name
        //   {2} = file name
        private static readonly string DirectoryIdFormat = "{0}.{1}";
        private static readonly string ComponentIdFormat = "{0}.{1}.{2}";
        private static readonly string FileIdFormat = "{0}.{1}.{2}";
        private static readonly string VariableFormat = "$(var.{0}.{1})";

        private static readonly string ComponentPrefix = "cmp";
        private static readonly string DirectoryPrefix = "dir";
        private static readonly string FilePrefix = "fil";

        private string projectGUID;
        private string directoryRefSeed;
        private bool setUniqueIdentifiers;


        private static readonly ProjectOutputGroup[] allOutputGroups = new ProjectOutputGroup[]
        {
            new ProjectOutputGroup("Binaries",   "BuiltProjectOutputGroup",         "TargetDir"),
            new ProjectOutputGroup("Symbols",    "DebugSymbolsProjectOutputGroup",  "TargetDir"),
            new ProjectOutputGroup("Documents",  "DocumentationProjectOutputGroup", "ProjectDir"),
            new ProjectOutputGroup("Satellites", "SatelliteDllsProjectOutputGroup", "TargetDir"),
            new ProjectOutputGroup("Sources",    "SourceFilesProjectOutputGroup",   "ProjectDir"),
            new ProjectOutputGroup("Content",    "ContentFilesProjectOutputGroup",  "ProjectDir"),
        };

        private string[] outputGroups;

        /// <summary>
        /// Instantiate a new VSProjectHarvester.
        /// </summary>
        /// <param name="outputGroups">List of project output groups to harvest.</param>
        public VSProjectHarvester(string[] outputGroups)
        {
            if (outputGroups == null)
            {
                throw new ArgumentNullException("outputGroups");
            }

            this.outputGroups = outputGroups;
        }

        /// <summary>
        /// Gets of sets the option to set unique identifiers.
        /// </summary>
        /// <value>The option to set unique identifiers.</value>
        public bool SetUniqueIdentifiers
        {
            get { return this.setUniqueIdentifiers; }
            set { this.setUniqueIdentifiers = value; }
        }

        /// <summary>
        /// Gets a list of friendly output group names that will be recognized on the command-line.
        /// </summary>
        /// <returns>Array of output group names.</returns>
        public static string[] GetOutputGroupNames()
        {
            string[] names = new string[VSProjectHarvester.allOutputGroups.Length];
            for (int i = 0; i < names.Length; i++)
            {
                names[i] = VSProjectHarvester.allOutputGroups[i].Name;
            }
            return names;
        }

        /// <summary>
        /// Harvest a VS project.
        /// </summary>
        /// <param name="argument">The path of the VS project file.</param>
        /// <returns>The harvested directory.</returns>
        public override Wix.Fragment[] Harvest(string argument)
        {
            if (null == argument)
            {
                throw new ArgumentNullException("argument");
            }

            if (!File.Exists(argument))
            {
                throw new FileNotFoundException(argument);
            }

            // Match specified output group names to available POG structures
            // and collect list of build output groups to pass to MSBuild.
            ProjectOutputGroup[] pogs = new ProjectOutputGroup[this.outputGroups.Length];
            string[] buildOutputGroups = new string[this.outputGroups.Length];
            for (int i = 0; i < this.outputGroups.Length; i++)
            {
                foreach (ProjectOutputGroup pog in VSProjectHarvester.allOutputGroups)
                {
                    if (pog.Name == this.outputGroups[i])
                    {
                        pogs[i] = pog;
                        buildOutputGroups[i] = pog.BuildOutputGroup;
                    }
                }

                if (buildOutputGroups[i] == null)
                {
                    throw new WixException(VSErrors.InvalidOutputGroup(this.outputGroups[i]));
                }
            }

            string projectFile = Path.GetFullPath(argument);

            IDictionary buildOutputs = this.GetProjectBuildOutputs(projectFile, buildOutputGroups);

            ArrayList fragmentList = new ArrayList();

            for (int i = 0; i < pogs.Length; i++)
            {
                this.HarvestProjectOutputGroup(projectFile, buildOutputs, pogs[i], fragmentList);
            }

            return (Wix.Fragment[]) fragmentList.ToArray(typeof(Wix.Fragment));
        }

        /// <summary>
        /// Runs MSBuild on a project file to get the list of filenames for the specified output groups.
        /// </summary>
        /// <param name="projectFile">VS MSBuild project file to load.</param>
        /// <param name="buildOutputGroups">List of MSBuild output group names.</param>
        /// <returns>Dictionary mapping output group names to lists of filenames in the group.</returns>
        private IDictionary GetProjectBuildOutputs(string projectFile, string[] buildOutputGroups)
        {
            MSBuildProject project = GetMsbuildProject(projectFile);

            project.Load(projectFile);

            IDictionary buildOutputs = new Hashtable();

            bool buildSuccess = project.Build(projectFile, buildOutputGroups, buildOutputs);

            if (!buildSuccess)
            {
                throw new WixException(VSErrors.BuildFailed());
            }

            this.projectGUID = project.GetEvaluatedProperty("ProjectGuid");

            if (null == this.projectGUID)
            {
                throw new WixException(VSErrors.BuildFailed());
            }

            return buildOutputs;
        }

        /// <summary>
        /// Creates WiX fragments for files in one output group.
        /// </summary>
        /// <param name="projectFile">VS MSBuild project file.</param>
        /// <param name="buildOutputs">Dictionary of build outputs retrieved from an MSBuild run on the project file.</param>
        /// <param name="pog">Project output group parameters.</param>
        /// <param name="fragmentList">List to which generated fragments will be added.</param>
        /// <returns>Count of harvested files.</returns>
        private int HarvestProjectOutputGroup(string projectFile, IDictionary buildOutputs, ProjectOutputGroup pog, IList fragmentList)
        {
            string projectName = Path.GetFileNameWithoutExtension(projectFile);
            string projectBaseDir = null;

            Wix.DirectoryRef directoryRef = new Wix.DirectoryRef();

            if (this.setUniqueIdentifiers)
            {
                directoryRef.Id = String.Format(CultureInfo.InvariantCulture, DirectoryIdFormat, projectName, pog.Name);
            }
            else
            {
                directoryRef.Id = HarvesterCore.GetIdentifierFromName(String.Format(CultureInfo.InvariantCulture, VSProjectHarvester.DirectoryIdFormat, projectName, pog.Name));
            }

            this.directoryRefSeed = this.Core.GenerateIdentifier(DirectoryPrefix, this.projectGUID, pog.Name);

            IEnumerable pogFiles = buildOutputs[pog.BuildOutputGroup] as IEnumerable;
            if (pogFiles == null)
            {
                throw new WixException(VSErrors.MissingProjectOutputGroup(
                    projectFile, pog.BuildOutputGroup));
            }

            if (pog.FileSource == "ProjectDir")
            {
                projectBaseDir = Path.GetDirectoryName(projectFile) + "\\";
            }

            int harvestCount = this.HarvestProjectOutputGroupFiles(projectBaseDir, projectName, pog.Name, pog.FileSource, pogFiles, directoryRef);

            if (harvestCount > 0)
            {
                Wix.Fragment drf = new Wix.Fragment();
                drf.AddChild(directoryRef);
                fragmentList.Add(drf);

                Wix.ComponentGroup cg = new Wix.ComponentGroup();

                if (this.setUniqueIdentifiers)
                {
                    cg.Id = String.Format(CultureInfo.InvariantCulture, DirectoryIdFormat, projectName, pog.Name);
                }
                else
                {
                    cg.Id = directoryRef.Id;
                }

                this.AddComponentsToComponentGroup(directoryRef, cg);

                Wix.Fragment cgf = new Wix.Fragment();
                cgf.AddChild(cg);
                fragmentList.Add(cgf);
            }

            return harvestCount;
        }

        /// <summary>
        /// Add all Components in an element tree to a ComponentGroup.
        /// </summary>
        /// <param name="parent">Parent of an element tree that will be searched for Components.</param>
        /// <param name="cg">The ComponentGroup the Components will be added to.</param>
        private void AddComponentsToComponentGroup(Wix.IParentElement parent, Wix.ComponentGroup cg)
        {
            foreach (Wix.ISchemaElement childElement in parent.Children)
            {
                Wix.Component c = childElement as Wix.Component;
                if (c != null)
                {
                    Wix.ComponentRef cr = new Wix.ComponentRef();
                    cr.Id = c.Id;
                    cg.AddChild(cr);
                }
                else
                {
                    Wix.IParentElement p = childElement as Wix.IParentElement;
                    if (p != null)
                    {
                        this.AddComponentsToComponentGroup(p, cg);
                    }
                }
            }
        }

        /// <summary>
        /// Harvest files from one output group of a VS project.
        /// </summary>
        /// <param name="baseDir">The base directory of the files.</param>
        /// <param name="projectName">Name of the project, to be used as a prefix for generated identifiers.</param>
        /// <param name="pogName">Name of the project output group, used for generating identifiers for WiX elements.</param>
        /// <param name="pogFileSource">The ProjectOutputGroup file source.</param>
        /// <param name="outputGroupFiles">The files from one output group to harvest.</param>
        /// <param name="parent">The parent element that will contain the components of the harvested files.</param>
        /// <returns>The number of files harvested.</returns>
        private int HarvestProjectOutputGroupFiles(string baseDir, string projectName, string pogName, string pogFileSource, IEnumerable outputGroupFiles, Wix.DirectoryRef parent)
        {
            int fileCount = 0;

            Wix.File exeFile = null;
            Wix.File appConfigFile = null;
            
            foreach (object output in outputGroupFiles)
            {
                string filePath = output.ToString();
                string fileName = Path.GetFileName(filePath);
                string fileDir = Path.GetDirectoryName(filePath);
                string link = null;

                MethodInfo getMetadataMethod = output.GetType().GetMethod("GetMetadata");
                if (getMetadataMethod != null)
                {
                    link = (string)getMetadataMethod.Invoke(output, new object[] { "Link" });
                    if (!String.IsNullOrEmpty(link))
                    {
                        fileDir = Path.GetDirectoryName(Path.Combine(baseDir, link));
                    }
                }

                Wix.IParentElement parentDir = parent;
                if (baseDir != null && !String.Equals(Path.GetDirectoryName(baseDir), fileDir, StringComparison.OrdinalIgnoreCase))
                {
                    Uri baseUri = new Uri(baseDir);
                    Uri relativeUri = baseUri.MakeRelativeUri(new Uri(fileDir));
                    parentDir = this.GetSubDirElement(parentDir, relativeUri);
                }

                string parentDirId = null;

                if (parentDir is Wix.DirectoryRef)
                {
                    parentDirId = this.directoryRefSeed;
                }
                else
                {
                    parentDirId = ((Wix.Directory)parentDir).Id;
                }

                Wix.Component component = new Wix.Component();
                Wix.File file = new Wix.File();

                if (pogName.Equals("Satellites", StringComparison.OrdinalIgnoreCase))
                {
                    Wix.Directory locDirectory = new Wix.Directory();
 
                    locDirectory.Name = Path.GetFileName(Path.GetDirectoryName(Path.GetFullPath(filePath)));
                    file.Source = String.Concat(String.Format(CultureInfo.InvariantCulture, VariableFormat, projectName, pogFileSource), "\\", locDirectory.Name, "\\", Path.GetFileName(filePath));

                    parentDir.AddChild(locDirectory);
                    locDirectory.AddChild(component);
                    component.AddChild(file);

                    if (this.setUniqueIdentifiers)
                    {
                        locDirectory.Id = this.Core.GenerateIdentifier(DirectoryPrefix, parentDirId, locDirectory.Name);
                        file.Id = this.Core.GenerateIdentifier(FilePrefix, locDirectory.Id, fileName);
                        component.Id = this.Core.GenerateIdentifier(ComponentPrefix, locDirectory.Id, file.Id);
                    }
                    else
                    {
                        locDirectory.Id = HarvesterCore.GetIdentifierFromName(String.Format(DirectoryIdFormat, (parentDir is Wix.DirectoryRef) ? ((Wix.DirectoryRef)parentDir).Id : parentDirId, locDirectory.Name));
                        file.Id = HarvesterCore.GetIdentifierFromName(String.Format(CultureInfo.InvariantCulture, VSProjectHarvester.FileIdFormat, projectName, pogName, String.Concat(locDirectory.Name, ".", fileName)));
                        component.Id = HarvesterCore.GetIdentifierFromName(String.Format(CultureInfo.InvariantCulture, VSProjectHarvester.ComponentIdFormat, projectName, pogName, String.Concat(locDirectory.Name, ".", fileName)));
                    }
                }
                else
                {
                    if (!String.IsNullOrEmpty(link))
                    {
                        // This needs to be the absolute path as a link can be located anywhere.
                        file.Source = filePath;
                    }
                    else if (null == baseDir)
                    {
                        file.Source = String.Concat(String.Format(CultureInfo.InvariantCulture, "$(var.{0}.{1})", projectName, pogFileSource), "\\", Path.GetFileName(filePath));
                    }
                    else
                    {
                        file.Source = String.Concat(String.Format(CultureInfo.InvariantCulture, "$(var.{0}.{1})", projectName, pogFileSource), "\\", filePath.Substring(baseDir.Length));
                    }

                    component.AddChild(file);
                    parentDir.AddChild(component);

                    if (this.setUniqueIdentifiers)
                    {
                        file.Id = this.Core.GenerateIdentifier(FilePrefix, parentDirId, fileName);
                        component.Id = this.Core.GenerateIdentifier(ComponentPrefix, parentDirId, file.Id);
                    }
                    else
                    {
                        file.Id = HarvesterCore.GetIdentifierFromName(String.Format(CultureInfo.InvariantCulture, VSProjectHarvester.FileIdFormat, projectName, pogName, fileName));
                        component.Id = HarvesterCore.GetIdentifierFromName(String.Format(CultureInfo.InvariantCulture, VSProjectHarvester.ComponentIdFormat, projectName, pogName, fileName));
                    }
                }

                if (String.Equals(Path.GetExtension(file.Source), ".exe", StringComparison.OrdinalIgnoreCase))
                {
                    exeFile = file;
                }
                else if (file.Source.EndsWith("app.config", StringComparison.OrdinalIgnoreCase))
                {
                    appConfigFile = file;
                }

                fileCount++;
            }

            // Special case for the app.config file in the Binaries POG...
            // The POG refers to the files in the OBJ directory, while the
            // generated WiX code references them in the bin directory.
            // The app.config file gets renamed to match the exe name.
            if ("Binaries" == pogName && null != exeFile && null != appConfigFile)
            {
                appConfigFile.Source = appConfigFile.Source.Replace("app.config", Path.GetFileName(exeFile.Source) + ".config");
            }

            return fileCount;
        }

        /// <summary>
        /// Gets a Directory element corresponding to a relative subdirectory within the project,
        /// either by locating a suitable existing Directory or creating a new one.
        /// </summary>
        /// <param name="parentDir">The parent element which the subdirectory is relative to.</param>
        /// <param name="relativeUri">Relative path of the subdirectory.</param>
        /// <returns>Directory element for the relative path.</returns>
        private Wix.Directory GetSubDirElement(Wix.IParentElement parentDir, Uri relativeUri)
        {
            string[] segments = relativeUri.ToString().Split('\\', '/');
            string firstSubDirName = segments[0];
            Wix.Directory subDir = null;

            foreach (Wix.ISchemaElement childElement in parentDir.Children)
            {
                Wix.Directory childDir = childElement as Wix.Directory;
                if (childDir != null && String.Equals(childDir.Name, firstSubDirName, StringComparison.OrdinalIgnoreCase))
                {
                    subDir = childDir;
                    break;
                }
            }

            if (subDir == null)
            {
                string parentId = null;
                Wix.Directory parentDirectory = parentDir as Wix.Directory;
                Wix.DirectoryRef parentDirectoryRef = parentDir as Wix.DirectoryRef;

                if (parentDirectory != null)
                {
                    parentId = parentDirectory.Id;
                }
                else if (parentDirectoryRef != null)
                {
                    if (this.setUniqueIdentifiers)
                    {
                        //Use the GUID of the project instead of the project name to help keep things stable.
                        parentId = this.directoryRefSeed;
                    }
                    else
                    {
                        parentId = parentDirectoryRef.Id;
                    }
                }

                subDir = new Wix.Directory();

                if (this.setUniqueIdentifiers)
                {
                    subDir.Id = this.Core.GenerateIdentifier(DirectoryPrefix, parentId, firstSubDirName);
                }
                else
                {
                    subDir.Id = String.Format(DirectoryIdFormat, parentId, firstSubDirName);
                }

                subDir.Name = firstSubDirName;

                parentDir.AddChild(subDir);
            }

            if (segments.Length == 1)
            {
                return subDir;
            }
            else
            {
                Uri nextRelativeUri = new Uri(relativeUri.ToString().Substring(firstSubDirName.Length + 1), UriKind.Relative);
                return GetSubDirElement(subDir, nextRelativeUri);
            }
        }

        private static MSBuildProject GetMsbuildProject(string projectFile)
        {
            MSBuildProject project = ConstructMsbuildProject(projectFile);

            if ("2.0.0.0" != project.LoadVersion)
            {
                project.Load(projectFile);

                string version = project.GetDefaultToolsVersion();

                if ("2.0" == version)
                {
                    project = ConstructMsbuildProject(projectFile, "2.0.0.0");
                }
            }

            return project;
        }

        private static MSBuildProject ConstructMsbuildProject(string projectFile)
        {
            return ConstructMsbuildProject(projectFile, null);
        }

        private static MSBuildProject ConstructMsbuildProject(string projectFile, string loadVersion)
        {
            const string MSBuildEngineAssemblyName = "Microsoft.Build.Engine, Version={0}, Culture=neutral, PublicKeyToken=b03f5f7f11d50a3a";
            Assembly msbuildAssembly = null;

            loadVersion = loadVersion ?? "3.5.0.0";

            try
            {
                try
                {
                    msbuildAssembly = Assembly.Load(String.Format(MSBuildEngineAssemblyName, loadVersion));
                }
                catch (FileNotFoundException)
                {
                    loadVersion = "2.0.0.0";
                    msbuildAssembly = Assembly.Load(String.Format(MSBuildEngineAssemblyName, loadVersion));
                }
            }
            catch (Exception e)
            {
                throw new WixException(VSErrors.CannotLoadMSBuildAssembly(e.Message));
            }

            Type engineType;
            Type projectType;
            object engine;
            object project;

            try
            {
                engineType = msbuildAssembly.GetType("Microsoft.Build.BuildEngine.Engine");
                if (msbuildAssembly.GetName().Version.Major >= 3)
                {
                    // MSBuild v3.5 uses this constructor which automatically sets the tool path.
                    engine = engineType.GetConstructor(new Type[] { }).Invoke(null);
                }
                else
                {
                    //MSBuild v2.0 uses this constructor which requires specifying the MSBuild bin path.
                    string msbuildBinPath = RuntimeEnvironment.GetRuntimeDirectory();
                    engine = engineType.GetConstructor(new Type[] { typeof(string) }).Invoke(new object[] { msbuildBinPath });
                }

                projectType = msbuildAssembly.GetType("Microsoft.Build.BuildEngine.Project");
                project = projectType.GetConstructor(new Type[] { engineType }).Invoke(new object[] { engine });
            }
            catch (TargetInvocationException tie)
            {
                throw new WixException(VSErrors.CannotLoadMSBuildEngine(tie.InnerException.Message));
            }
            catch (Exception e)
            {
                throw new WixException(VSErrors.CannotLoadMSBuildEngine(e.Message));
            }

            return new MSBuildProject(project, projectType, loadVersion);
        }

        private class MSBuildProject
        {
            Type projectType;
            object project;
            string loadVersion;

            public MSBuildProject(object project, Type projectType, string loadVersion)
            {
                this.project = project;
                this.projectType = projectType;
                this.loadVersion = loadVersion;
            }

            public string LoadVersion
            {
                get
                { return this.loadVersion; }
            }

            public bool Build(string projectFileName, string[] targetNames, IDictionary targetOutputs)
            {
                try
                {
                    MethodInfo buildMethod = projectType.GetMethod("Build", new Type[] { typeof(string[]), typeof(IDictionary) });
                    return (bool)buildMethod.Invoke(project, new object[] { targetNames, targetOutputs });
                }
                catch (Exception e)
                {
                    throw new WixException(VSErrors.CannotBuildProject(projectFileName, e.Message));
                }
            }

            public string GetDefaultToolsVersion()
            {
                string version = null;

                try
                {
                    PropertyInfo pi = projectType.GetProperty("DefaultToolsVersion");
                    version = (string)pi.GetValue(this.project, null);
                }
                catch
                {
                    version = "2.0";
                }

                return version;
            }

            public string GetEvaluatedProperty(string propertyName)
            {
                MethodInfo getProjectGuid = projectType.GetMethod("GetEvaluatedProperty", new Type[] { typeof(string) });
                return (string)getProjectGuid.Invoke(project, new object[] { "ProjectGuid" });
            }

            public void Load(string projectFileName)
            {
                try
                {
                    projectType.GetMethod("Load", new Type[] { typeof(string) }).Invoke(project, new object[] { projectFileName });
                }
                catch (TargetInvocationException tie)
                {
                    throw new WixException(VSErrors.CannotLoadProject(projectFileName, tie.InnerException.Message));
                }
                catch (Exception e)
                {
                    throw new WixException(VSErrors.CannotLoadProject(projectFileName, e.Message));
                }
            }
        }

        /// <summary>
        /// Used internally in the VSProjectHarvester class to encapsulate
        /// the settings for a particular MSBuild "project output group".
        /// </summary>
        private struct ProjectOutputGroup
        {
            public readonly string Name;
            public readonly string BuildOutputGroup;
            public readonly string FileSource;

            /// <summary>
            /// Creates a new project output group.
            /// </summary>
            /// <param name="name">Friendly name used by heat.</param>
            /// <param name="buildOutputGroup">MSBuild's name of the project output group.</param>
            /// <param name="fileSource">VS directory token containing the files of the POG.</param>
            public ProjectOutputGroup(string name, string buildOutputGroup, string fileSource)
            {
                this.Name = name;
                this.BuildOutputGroup = buildOutputGroup;
                this.FileSource = fileSource;
            }
        }
    }
}
