//-------------------------------------------------------------------------------------------------
// <copyright file="RefreshGeneratedFile.cs" company="Microsoft">
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
// Build task to refresh the generated file that contains ComponentGroupRefs to harvested output.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Build.Tasks
{
    using System;
    using System.Collections.Generic;
    using System.Globalization;
    using System.IO;
    using System.Text.RegularExpressions;
    using System.Xml;
    using Microsoft.Build.Framework;
    using Microsoft.Build.Utilities;

    using Wix = Microsoft.Tools.WindowsInstallerXml.Serialize;

    /// <summary>
    /// This task refreshes the generated file that contains ComponentGroupRefs
    /// to harvested output.
    /// </summary>
    public class RefreshGeneratedFile : Task
    {
        private ITaskItem[] generatedFiles;
        private ITaskItem[] projectReferencePaths;

        /// <summary>
        /// The list of files to generate.
        /// </summary>
        [Required]
        public ITaskItem[] GeneratedFiles
        {
            get { return this.generatedFiles; }
            set { this.generatedFiles = value; }
        }

        /// <summary>
        /// All the project references in the project.
        /// </summary>
        [Required]
        public ITaskItem[] ProjectReferencePaths
        {
            get { return this.projectReferencePaths; }
            set { this.projectReferencePaths = value; }
        }

        /// <summary>
        /// Gets a complete list of external cabs referenced by the given installer database file.
        /// </summary>
        /// <returns>True upon completion of the task execution.</returns>
        public override bool Execute()
        {
            List<string> componentGroupRefs = new List<string>();
            for (int i = 0; i < this.ProjectReferencePaths.Length; i++)
            {
                ITaskItem item = this.ProjectReferencePaths[i];

                if (!String.IsNullOrEmpty(item.GetMetadata(Common.DoNotHarvest)))
                {
                    continue;
                }

                string projectPath = CreateProjectReferenceDefineConstants.GetProjectPath(this.ProjectReferencePaths, i);
                string projectName = Path.GetFileNameWithoutExtension(projectPath);
                string referenceName = Common.GetIdentifierFromName(CreateProjectReferenceDefineConstants.GetReferenceName(item, projectName));

                string[] pogs = item.GetMetadata("RefProjectOutputGroups").Split(';');
                foreach (string pog in pogs)
                {
                    if (!String.IsNullOrEmpty(pog))
                    {
                        componentGroupRefs.Add(String.Format(CultureInfo.InvariantCulture, "{0}.{1}", referenceName, pog));
                    }
                }
            }

            // Create Wix root element and 'xml' processing instruction.
            Wix.Wix wixElement = new Wix.Wix();

            // Create a fragment.
            Wix.Fragment fragmentElement = new Wix.Fragment();
            wixElement.AddChild(fragmentElement);

            // Create the main ComponentGroup to be referenced in authoring.
            Wix.ComponentGroup componentGroupElement = new Wix.ComponentGroup();
            fragmentElement.AddChild(componentGroupElement);

            // Add each harvested project output group to the main ComponentGroup.
            foreach (string componentGroupRef in componentGroupRefs)
            {
                Wix.ComponentGroupRef componentGroupRefElement = new Wix.ComponentGroupRef();
                componentGroupRefElement.Id = componentGroupRef;
                componentGroupElement.AddChild(componentGroupRefElement);
            }

            // Set up the XmlWriter to save the documents.
            XmlWriterSettings settings = new XmlWriterSettings();
            settings.CloseOutput = true;
            settings.Indent = true;

            foreach (ITaskItem item in this.GeneratedFiles)
            {
                // Get just the file name of the output file to use as the main ComponentGroup Id.
                string fullPath = item.GetMetadata("FullPath");
                componentGroupElement.Id = Path.GetFileNameWithoutExtension(fullPath);

                try
                {
                    using (XmlWriter writer = XmlWriter.Create(fullPath, settings))
                    {
                        wixElement.OutputXml(writer);
                    }
                }
                catch (Exception e)
                {
                    // e.Message will be something like: "Access to the path 'fullPath' is denied."
                    this.Log.LogMessage(MessageImportance.High, "Unable to save generated file to '{0}'. {1}", fullPath, e.Message);
                }
            }

            return true;
        }
    }
}
