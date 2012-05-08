//-------------------------------------------------------------------------------------------------
// <copyright file="TagCompiler.cs" company="Microsoft">
//    Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// 
// <summary>
// The compiler for the Windows Installer XML Toolset Software Id Tag Extension.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Extensions
{
    using System;
    using System.Reflection;
    using System.Xml;
    using System.Xml.Schema;
    using Microsoft.Tools.WindowsInstallerXml;

    /// <summary>
    /// The compiler for the Windows Installer XML Toolset Software Id Tag Extension.
    /// </summary>
    public sealed class TagCompiler : CompilerExtension
    {
        private XmlSchema schema;

        /// <summary>
        /// Instantiate a new GamingCompiler.
        /// </summary>
        public TagCompiler()
        {
            this.schema = LoadXmlSchemaHelper(Assembly.GetExecutingAssembly(), "Microsoft.Tools.WindowsInstallerXml.Extensions.Xsd.tag.xsd");
        }

        /// <summary>
        /// Gets the schema for this extension.
        /// </summary>
        /// <value>Schema for this extension.</value>
        public override XmlSchema Schema
        {
            get { return this.schema; }
        }

        /// <summary>
        /// Processes an element for the Compiler.
        /// </summary>
        /// <param name="sourceLineNumbers">Source line number for the parent element.</param>
        /// <param name="parentElement">Parent element of element to process.</param>
        /// <param name="element">Element to process.</param>
        /// <param name="contextValues">Extra information about the context in which this element is being parsed.</param>
        public override void ParseElement(SourceLineNumberCollection sourceLineNumbers, XmlElement parentElement, XmlElement element, params string[] contextValues)
        {
            switch (parentElement.LocalName)
            {
                case "Product":
                    switch (element.LocalName)
                    {
                        case "Tag":
                            this.ParseTagElement(element);
                            break;
                        default:
                            this.Core.UnexpectedElement(parentElement, element);
                            break;
                    }
                    break;
                default:
                    this.Core.UnexpectedElement(parentElement, element);
                    break;
            }
        }

        /// <summary>
        /// Parses a Tag element for Software Id Tag registration.
        /// </summary>
        /// <param name="node">The element to parse.</param>
        private void ParseTagElement(XmlNode node)
        {
            SourceLineNumberCollection sourceLineNumbers = Preprocessor.GetSourceLineNumbers(node);
            string name = null;
            string regid = null;
            string feature = "WixSwidTag";
            YesNoType licensed = YesNoType.NotSet;

            foreach (XmlAttribute attrib in node.Attributes)
            {
                if (0 == attrib.NamespaceURI.Length || attrib.NamespaceURI == this.schema.TargetNamespace)
                {
                    switch (attrib.LocalName)
                    {
                        case "Name":
                            name = this.Core.GetAttributeLongFilename(sourceLineNumbers, attrib, false);
                            break;
                        case "Regid":
                            regid = this.Core.GetAttributeValue(sourceLineNumbers, attrib);
                            break;
                        case "Feature":
                            feature = this.Core.GetAttributeIdentifierValue(sourceLineNumbers, attrib);
                            break;
                        case "Licensed":
                            licensed = this.Core.GetAttributeYesNoValue(sourceLineNumbers, attrib);
                            break;
                        default:
                            this.Core.UnexpectedAttribute(sourceLineNumbers, attrib);
                            break;
                    }
                }
                else
                {
                    this.Core.UnsupportedExtensionAttribute(sourceLineNumbers, attrib);
                }
            }

            foreach (XmlNode child in node.ChildNodes)
            {
                if (XmlNodeType.Element == child.NodeType)
                {
                    if (child.NamespaceURI == this.Schema.TargetNamespace)
                    {
                        this.Core.UnexpectedElement(node, child);
                    }
                    else
                    {
                        this.Core.UnsupportedExtensionElement(node, child);
                    }
                }
            }

            if (null == name)
            {
                XmlAttribute productNameAttribute = node.ParentNode.Attributes["Name"];
                if (null != productNameAttribute)
                {
                    name = productNameAttribute.Value;
                }
                else
                {
                    this.Core.OnMessage(WixErrors.ExpectedAttribute(sourceLineNumbers, node.Name, "Name"));
                }
            }

            if (null != name && !CompilerCore.IsValidLongFilename(name, false))
            {
                this.Core.OnMessage(TagErrors.IllegalName(sourceLineNumbers, node.ParentNode.LocalName, name));
            }

            if (null == regid)
            {
                this.Core.OnMessage(WixErrors.ExpectedAttribute(sourceLineNumbers, node.Name, "Regid"));
            }

            if (!this.Core.EncounteredError)
            {
                string directoryId = "WixTagRegidFolder";
                string fileId = this.Core.GenerateIdentifier("tag", regid, ".product.tag");
                string fileName = String.Concat(regid, " ", name, ".swidtag");
                string shortName = this.Core.GenerateShortName(fileName, false, false);

                this.Core.CreateWixSimpleReferenceRow(sourceLineNumbers, "Directory", directoryId);

                ComponentRow componentRow = (ComponentRow)this.Core.CreateRow(sourceLineNumbers, "Component");
                componentRow.Component = fileId;
                componentRow.Guid = "*";
                componentRow[3] = 0;
                componentRow.Directory = directoryId;
                componentRow.IsLocalOnly = true;
                componentRow.KeyPath = fileId;

                this.Core.CreateWixSimpleReferenceRow(sourceLineNumbers, "Feature", feature);
                this.Core.CreateComplexReference(sourceLineNumbers, ComplexReferenceParentType.Feature, feature, null, ComplexReferenceChildType.Component, fileId, true);

                FileRow fileRow = (FileRow)this.Core.CreateRow(sourceLineNumbers, "File");
                fileRow.File = fileId;
                fileRow.Component = fileId;
                fileRow.FileName = String.Concat(shortName, "|", fileName);

                WixFileRow wixFileRow = (WixFileRow)this.Core.CreateRow(sourceLineNumbers, "WixFile");
                wixFileRow.Directory = directoryId;
                wixFileRow.File = fileId;
                wixFileRow.DiskId = 1;
                wixFileRow.Attributes = 1;
                wixFileRow.Source = String.Concat("%TEMP%\\", fileName);

                this.Core.EnsureTable(sourceLineNumbers, "SoftwareIdentificationTag");
                Row row = this.Core.CreateRow(sourceLineNumbers, "WixTag");
                row[0] = fileId;
                row[1] = regid;
                row[2] = name;
                if (YesNoType.Yes == licensed)
                {
                    row[3] = 1;
                }

                this.Core.CreateWixSimpleReferenceRow(sourceLineNumbers, "File", fileId);
            }
        }
    }
}
