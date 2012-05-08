//-------------------------------------------------------------------------------------------------
// <copyright file="UpdateRegistrationInfo.cs" company="Microsoft">
//    Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// 
// <summary>
// Major upgrade detection information for related packages.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml
{
    using System;

    /// <summary>
    /// Major upgrade detection information for related packages.
    /// </summary>
    internal class UpdateRegistrationInfo
    {
        public UpdateRegistrationInfo(Row row)
        {
            this.Manufacturer = (string)row[0];
            this.Department = (string)row[1];
            this.ProductFamily = (string)row[2];
            this.Name = (string)row[3];
            this.Classification = (string)row[4];
        }

        public string Manufacturer { get; set; }
        public string Department { get; set; }
        public string ProductFamily { get; set; }
        public string Name { get; set; }
        public string Classification { get; set; }
    }
}
