//-------------------------------------------------------------------------------------------------
// <copyright file="DependencyCompiler.cs" company="Microsoft">
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
// The Windows Installer XML toolset dependency extension compiler.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Extensions
{
    using System;
    using System.Globalization;
    using System.Reflection;
    using System.Text;
    using System.Xml;
    using System.Xml.Schema;
    using Microsoft.Tools.WindowsInstallerXml;
    using Microsoft.Tools.WindowsInstallerXml.Extensions.Serialize.Dependency;

    using Wix = Microsoft.Tools.WindowsInstallerXml;

    /// <summary>
    /// The compiler for the Windows Installer XML toolset dependency extension.
    /// </summary>
    public sealed class DependencyCompiler : CompilerExtension
    {
        private XmlSchema schema;

        public DependencyCompiler()
        {
            this.schema = LoadXmlSchemaHelper(Assembly.GetExecutingAssembly(), "Microsoft.Tools.WindowsInstallerXml.Extensions.Xsd.Dependency.xsd");
        }

        /// <summary>
        /// Gets the schema for this extension.
        /// </summary>
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
                case "Bundle":
                case "Fragment":
                case "Module":
                case "Product":
                    switch (element.LocalName)
                    {
                        case "Requires":
                            this.ParseRequiresElement(element, null);
                            break;
                        default:
                            this.Core.UnexpectedElement(parentElement, element);
                            break;
                    }
                    break;
                case "ExePackage":
                case "MsiPackage":
                case "MspPackage":
                case "MsuPackage":
                    string packageId = contextValues[0];

                    switch (element.LocalName)
                    {
                        case "Provides":
                            string keyPath = null;
                            this.ParseProvidesElement(element, true, ref keyPath, packageId);
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
        /// Processes a child element of a Component for the Compiler.
        /// </summary>
        /// <param name="sourceLineNumbers">Source line number for the parent element.</param>
        /// <param name="parentElement">Parent element of element to process.</param>
        /// <param name="element">Element to process.</param>
        /// <param name="keyPath">Explicit key path.</param>
        /// <param name="contextValues">Extra information about the context in which this element is being parsed.</param>
        /// <returns>The component key path type if set.</returns>
        public override CompilerExtension.ComponentKeypathType ParseElement(SourceLineNumberCollection sourceLineNumbers, XmlElement parentElement, XmlElement element, ref string keyPath, params string[] contextValues)
        {
            CompilerExtension.ComponentKeypathType keyPathType = CompilerExtension.ComponentKeypathType.None;

            switch (parentElement.LocalName)
            {
                case "Component":
                    string componentId = contextValues[0];

                    switch (element.LocalName)
                    {
                        case "Provides":
                            keyPathType = this.ParseProvidesElement(element, false, ref keyPath, componentId);
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

            return keyPathType;
        }

        /// <summary>
        /// Processes the Provides element.
        /// </summary>
        /// <param name="node">The XML node for the Provides element.</param>
        /// <param name="isPackage">True if authored for a package; otherwise, false if authored for a component.</param>
        /// <param name="keyPath">Explicit key path.</param>
        /// <param name="parentId">The identifier of the parent component or package.</param>
        /// <returns>The type of key path if set.</returns>
        private CompilerExtension.ComponentKeypathType ParseProvidesElement(XmlNode node, bool isPackage, ref string keyPath, string parentId)
        {
            SourceLineNumberCollection sourceLineNumbers = Preprocessor.GetSourceLineNumbers(node);
            CompilerExtension.ComponentKeypathType keyPathType = CompilerExtension.ComponentKeypathType.None;
            string id = null;
            string key = null;
            string displayKey = null;
            int attributes = 0;
            int versionGuarantee = CompilerCore.IntegerNotSet;

            foreach (XmlAttribute attrib in node.Attributes)
            {
                if (0 == attrib.NamespaceURI.Length || attrib.NamespaceURI == this.schema.TargetNamespace)
                {
                    switch (attrib.LocalName)
                    {
                        case "Id":
                            id = this.Core.GetAttributeIdentifierValue(sourceLineNumbers, attrib);
                            break;
                        case "Key":
                            key = this.Core.GetAttributeValue(sourceLineNumbers, attrib);
                            break;
                        case "VersionGuarantee":
                            string value = this.Core.GetAttributeValue(sourceLineNumbers, attrib);
                            Provides.VersionGuaranteeType guarantee = Provides.ParseVersionGuaranteeType(value);
                            switch (guarantee)
                            {
                                case Provides.VersionGuaranteeType.None:
                                    break;
                                case Provides.VersionGuaranteeType.Major:
                                    versionGuarantee = DependencyCommon.ProvidesAttributesVersionGuaranteeMajor;
                                    break;
                                case Provides.VersionGuaranteeType.Minor:
                                    versionGuarantee = DependencyCommon.ProvidesAttributesVersionGuaranteeMinor;
                                    break;
                                case Provides.VersionGuaranteeType.Build:
                                    versionGuarantee = DependencyCommon.ProvidesAttributesVersionGuaranteeBuild;
                                    break;
                                default:
                                    this.Core.OnMessage(WixErrors.IllegalAttributeValue(sourceLineNumbers, node.LocalName, attrib.LocalName, value, "None", "Major", "Minor", "Build"));
                                    break;
                            }
                            break;
                        case "DisplayKey":
                            displayKey = this.Core.GetAttributeValue(sourceLineNumbers, attrib);
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

            if (!String.IsNullOrEmpty(key))
            {
                if (CompilerCore.IntegerNotSet != versionGuarantee)
                {
                    this.Core.OnMessage(DependencyWarnings.AttributeIgnored(sourceLineNumbers, node.LocalName, "VersionGuarantee", "Key"));
                }

                // Make sure the key does not contain a semicolon.
                if (0 <= key.IndexOf(";", StringComparison.Ordinal))
                {
                    this.Core.OnMessage(DependencyErrors.IllegalCharactersInProvider(sourceLineNumbers, "Key", ";"));
                }
            }
            else if (isPackage)
            {
                // Must specify the provider key when authored for a package.
                if (String.IsNullOrEmpty(key))
                {
                    this.Core.OnMessage(WixErrors.ExpectedAttribute(sourceLineNumbers, node.LocalName, "Key"));
                }

                // VersionGuarantee not valid on a package element.
                if (CompilerCore.IntegerNotSet != versionGuarantee)
                {
                    this.Core.OnMessage(WixErrors.IllegalAttributeExceptOnElement(sourceLineNumbers, node.LocalName, "VersionGuarantee", "Component"));
                }
            }
            else
            {
                // Make sure the UpgradeCode is authored and set the key.
                this.Core.CreateWixSimpleReferenceRow(sourceLineNumbers, "Property", "UpgradeCode");

                // Generate a key based on the documented algorithm.
                StringBuilder sb = new StringBuilder("!(bind.property.UpgradeCode)");

                // Default to the major version field.
                if (CompilerCore.IntegerNotSet == versionGuarantee)
                {
                    versionGuarantee = DependencyCommon.ProvidesAttributesVersionGuaranteeMajor;
                }

                if (DependencyCommon.ProvidesAttributesVersionGuaranteeMajor <= versionGuarantee)
                {
                    sb.Append("v!(bind.property.ProductVersion.Major)");
                }

                if (DependencyCommon.ProvidesAttributesVersionGuaranteeMinor <= versionGuarantee)
                {
                    sb.Append(".!(bind.property.ProductVersion.Minor)");
                }

                if (DependencyCommon.ProvidesAttributesVersionGuaranteeBuild <= versionGuarantee)
                {
                    sb.Append(".!(bind.property.ProductVersion.Build)");
                }

                key = sb.ToString();
            }

            // Need the element ID for child element processing, so generate now if not authored.
            if (null == id)
            {
                id = this.Core.GenerateIdentifier("dep", node.LocalName, parentId, key);
            }

            foreach (XmlNode child in node.ChildNodes)
            {
                if (XmlNodeType.Element == child.NodeType)
                {
                    if (child.NamespaceURI == this.schema.TargetNamespace)
                    {
                        switch (child.LocalName)
                        {
                            case "Requires":
                                this.ParseRequiresElement(child, id);
                                break;
                            case "RequiresRef":
                                this.ParseRequiresRefElement(child, id);
                                break;
                            default:
                                this.Core.UnexpectedElement(node, child);
                                break;
                        }
                    }
                    else
                    {
                        this.Core.UnsupportedExtensionElement(node, child);
                    }
                }
            }

            if (!this.Core.EncounteredError)
            {
                // Reference the custom action to check for dependencies on the current provider.
                this.Core.CreateWixSimpleReferenceRow(sourceLineNumbers, "CustomAction", "WixDependencyCheck");

                // Create the row in the provider table.
                Row row = this.Core.CreateRow(sourceLineNumbers, "WixDependencyProvider");
                row[0] = id;
                row[1] = parentId;
                row[2] = key;

                if (null != displayKey)
                {
                    row[3] = displayKey;
                }

                if (0 != attributes)
                {
                    row[4] = attributes;
                }

                if (!isPackage)
                {
                    // Generate registry rows for the provider using binder properties.
                    string keyProvides = String.Concat(DependencyCommon.RegistryRoot, key);

                    row = this.Core.CreateRow(sourceLineNumbers, "Registry");
                    row[0] = this.Core.GenerateIdentifier("reg", id, "(Default)");
                    row[1] = -1;
                    row[2] = keyProvides;
                    row[3] = null;
                    row[4] = "[ProductCode]";
                    row[5] = parentId;

                    // Use the Version registry value as the key path if not already set.
                    string idVersion = this.Core.GenerateIdentifier("reg", id, "Version");
                    if (String.IsNullOrEmpty(keyPath))
                    {
                        keyPath = idVersion;
                        keyPathType = CompilerExtension.ComponentKeypathType.Registry;
                    }

                    row = this.Core.CreateRow(sourceLineNumbers, "Registry");
                    row[0] = idVersion;
                    row[1] = -1;
                    row[2] = keyProvides;
                    row[3] = "Version";
                    row[4] = "[ProductVersion]";
                    row[5] = parentId;

                    if (null != displayKey)
                    {
                        row = this.Core.CreateRow(sourceLineNumbers, "Registry");
                        row[0] = this.Core.GenerateIdentifier("reg", id, "DisplayKey");
                        row[1] = -1;
                        row[2] = keyProvides;
                        row[3] = "DisplayKey";
                        row[4] = displayKey;
                        row[5] = parentId;
                    }

                    if (0 != attributes)
                    {
                        row = this.Core.CreateRow(sourceLineNumbers, "Registry");
                        row[0] = this.Core.GenerateIdentifier("reg", id, "Attributes");
                        row[1] = -1;
                        row[2] = keyProvides;
                        row[3] = "Attributes";
                        row[4] = String.Concat("#", attributes.ToString(CultureInfo.InvariantCulture.NumberFormat));
                        row[5] = parentId;
                    }
                }
            }

            return keyPathType;
        }

        /// <summary>
        /// Processes the Requires element.
        /// </summary>
        /// <param name="node">The XML node for the Requires element.</param>
        /// <param name="providerId">The parent provider identifier.</param>
        private void ParseRequiresElement(XmlNode node, string providerId)
        {
            SourceLineNumberCollection sourceLineNumbers = Preprocessor.GetSourceLineNumbers(node);
            string id = null;
            string providerKey = null;
            string minVersion = null;
            string maxVersion = null;
            int attributes = 0;

            foreach (XmlAttribute attrib in node.Attributes)
            {
                if (0 == attrib.NamespaceURI.Length || attrib.NamespaceURI == this.schema.TargetNamespace)
                {
                    switch (attrib.LocalName)
                    {
                        case "Id":
                            id = this.Core.GetAttributeIdentifierValue(sourceLineNumbers, attrib);
                            break;
                        case "ProviderKey":
                            providerKey = this.Core.GetAttributeValue(sourceLineNumbers, attrib);
                            break;
                        case "Minimum":
                            minVersion = this.Core.GetAttributeVersionValue(sourceLineNumbers, attrib, true);
                            break;
                        case "Maximum":
                            maxVersion = this.Core.GetAttributeVersionValue(sourceLineNumbers, attrib, true);
                            break;
                        case "IncludeMinimum":
                            if (Wix.YesNoType.Yes == this.Core.GetAttributeYesNoValue(sourceLineNumbers, attrib))
                            {
                                attributes |= DependencyCommon.RequiresAttributesMinVersionInclusive;
                            }
                            break;
                        case "IncludeMaximum":
                            if (Wix.YesNoType.Yes == this.Core.GetAttributeYesNoValue(sourceLineNumbers, attrib))
                            {
                                attributes |= DependencyCommon.RequiresAttributesMaxVersionInclusive;
                            }
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
                    if (child.NamespaceURI == this.schema.TargetNamespace)
                    {
                        this.Core.UnexpectedElement(node, child);
                    }
                    else
                    {
                        this.Core.UnsupportedExtensionElement(node, child);
                    }
                }
            }

            if (null == id)
            {
                // Generate an ID only if this element is authored under a Provides element; otherwise, a RequiresRef
                // element will be necessary and the Id attribute will be required.
                if (null != providerId)
                {
                    id = this.Core.GenerateIdentifier("dep", node.LocalName, providerKey);
                }
                else
                {
                    this.Core.OnMessage(WixErrors.ExpectedAttributeWhenElementNotUnderElement(sourceLineNumbers, node.LocalName, "Id", "Provides"));
                }
            }

            if (null == providerKey)
            {
                this.Core.OnMessage(WixErrors.ExpectedAttribute(sourceLineNumbers, node.LocalName, "ProviderKey"));
            }
            // Make sure the key does not contain a semicolon.
            else if (0 <= providerKey.IndexOf(";", StringComparison.Ordinal))
            {
                this.Core.OnMessage(DependencyErrors.IllegalCharactersInProvider(sourceLineNumbers, "ProviderKey", ";"));
            }


            if (!this.Core.EncounteredError)
            {
                Row row = this.Core.CreateRow(sourceLineNumbers, "WixDependency");
                row[0] = id;
                row[1] = providerKey;
                row[2] = minVersion;
                row[3] = maxVersion;

                if (0 != attributes)
                {
                    row[4] = attributes;
                }

                // Create the relationship between this WixDependency row and the WixDependencyProvider row.
                if (null != providerId)
                {
                    // Create the relationship between the WixDependency row and the parent WixDependencyProvider row.
                    row = this.Core.CreateRow(sourceLineNumbers, "WixDependencyRef");
                    row[0] = providerId;
                    row[1] = id;
                }
            }
        }

        /// <summary>
        /// Processes the RequiresRef element.
        /// </summary>
        /// <param name="node">The XML node for the RequiresRef element.</param>
        /// <param name="providerId">The parent provider identifier.</param>
        private void ParseRequiresRefElement(XmlNode node, string providerId)
        {
            SourceLineNumberCollection sourceLineNumbers = Preprocessor.GetSourceLineNumbers(node);
            string id = null;

            foreach (XmlAttribute attrib in node.Attributes)
            {
                if (0 == attrib.NamespaceURI.Length || attrib.NamespaceURI == this.schema.TargetNamespace)
                {
                    switch (attrib.LocalName)
                    {
                        case "Id":
                            id = this.Core.GetAttributeIdentifierValue(sourceLineNumbers, attrib);
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
                    if (child.NamespaceURI == this.schema.TargetNamespace)
                    {
                        this.Core.UnexpectedElement(node, child);
                    }
                    else
                    {
                        this.Core.UnsupportedExtensionElement(node, child);
                    }
                }
            }

            if (null == id)
            {
                this.Core.OnMessage(WixErrors.ExpectedAttribute(sourceLineNumbers, node.LocalName, "Id"));
            }

            if (!this.Core.EncounteredError)
            {
                // Create a link dependency on the row that contains information we'll need during bind.
                this.Core.CreateWixSimpleReferenceRow(sourceLineNumbers, "WixDependency", id);

                // Create the relationship between the WixDependency row and the parent WixDependencyProvider row.
                Row row = this.Core.CreateRow(sourceLineNumbers, "WixDependencyRef");
                row[0] = providerId;
                row[1] = id;
            }
        }
    }
}
