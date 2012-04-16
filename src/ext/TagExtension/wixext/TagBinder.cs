//-------------------------------------------------------------------------------------------------
// <copyright file="TagBinder.cs" company="Microsoft">
//    Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// 
// <summary>
// The Binder for the Windows Installer XML Toolset Software Id Tag Extension.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Extensions
{
    using System;
    using System.IO;
    using System.Text;
    using System.Xml;
    using Microsoft.Tools.WindowsInstallerXml;

    /// <summary>
    /// The Binder for the Windows Installer XML Toolset Software Id Tag Extension.
    /// </summary>
    public sealed class TagBinder : BinderExtension
    {
        /// <summary>
        /// Called before database binding occurs.
        /// </summary>
        public override void DatabaseInitialize(Output output)
        {
            Table tagTable = output.Tables["WixTag"];
            if (null != tagTable)
            {
                string productName = null;
                Version productVersion = new Version("0.0");
                string manufacturer = null;

                Table properties = output.Tables["Property"];
                foreach (Row property in properties.Rows)
                {
                    switch ((string)property[0])
                    {
                        case "ProductName":
                            productName = (string)property[1];
                            break;
                        case "ProductVersion":
                            productVersion = new Version((string)property[1]);
                            break;
                        case "Manufacturer":
                            manufacturer = (string)property[1];
                            break;
                    }
                }

                Table wixFileTable = output.Tables["WixFile"];
                foreach (Row tagRow in tagTable.Rows)
                {
                    string fileId = (string)tagRow[0];
                    string regid = (string)tagRow[1];
                    string name = (string)tagRow[2];
                    bool licensed = (null != tagRow[3] && 1 == (int)tagRow[3]);

                    foreach (WixFileRow wixFileRow in wixFileTable.Rows)
                    {
                        if (fileId == wixFileRow.File)
                        {
                            wixFileRow.Source = Path.GetTempFileName();
                            using (XmlTextWriter writer = new XmlTextWriter(wixFileRow.Source, Encoding.UTF8))
                            {
                                writer.Formatting = Formatting.Indented;
                                writer.WriteStartDocument();
                                writer.WriteStartElement("software_identification_tag", "http://standards.iso.org/iso/19770/-2/2009/schema.xsd");
                                writer.WriteElementString("entitlement_required_indicator", licensed ? "true" : "false");

                                writer.WriteElementString("product_title", productName);

                                writer.WriteStartElement("product_version");
                                writer.WriteElementString("name", productVersion.ToString());
                                writer.WriteStartElement("numeric");
                                writer.WriteElementString("major", productVersion.Major.ToString());
                                if (-1 < productVersion.Minor)
                                {
                                    writer.WriteElementString("minor", productVersion.Minor.ToString());
                                }
                                if (-1 < productVersion.Build)
                                {
                                    writer.WriteElementString("build", productVersion.Build.ToString());
                                }
                                if (-1 < productVersion.Revision)
                                {
                                    writer.WriteElementString("review", productVersion.Revision.ToString());
                                }
                                writer.WriteEndElement();
                                writer.WriteEndElement();

                                writer.WriteStartElement("software_creator");
                                writer.WriteElementString("name", manufacturer);
                                writer.WriteElementString("regid", regid);
                                writer.WriteEndElement();

                                if (licensed)
                                {
                                    writer.WriteStartElement("software_licensor");
                                    writer.WriteElementString("name", manufacturer);
                                    writer.WriteElementString("regid", regid);
                                    writer.WriteEndElement();
                                }

                                writer.WriteStartElement("software_id");
                                writer.WriteElementString("unique_id", name.Replace(" ", "-"));
                                writer.WriteElementString("tag_creator_regid", regid);
                                writer.WriteEndElement();

                                writer.WriteStartElement("tag_creator");
                                writer.WriteElementString("name", manufacturer);
                                writer.WriteElementString("regid", regid);
                                writer.WriteEndElement();

                                writer.WriteEndElement();
                            }
                        }
                    }
                }
            }
        }
    }
}
