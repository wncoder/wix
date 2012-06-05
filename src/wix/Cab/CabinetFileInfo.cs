//-------------------------------------------------------------------------------------------------
// <copyright file="CabinetFileInfo.cs" company="Microsoft Corporation">
//   Copyright (c) 2004, Microsoft Corporation.
//   This software is released under Common Public License Version 1.0 (CPL).
//   The license and further copyright text can be found in the file LICENSE.TXT
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
// 
// <summary>
// Contains properties for a file inside a cabinet
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml
{
    using System;

    /// <summary>
    /// Properties of a file in a cabinet.
    /// </summary>
    internal sealed class CabinetFileInfo
    {
        private string fileId;
        private ushort date;
        private ushort time;

        /// <summary>
        /// Constructs CabinetFileInfo
        /// </summary>
        /// <param name="fileId">File Id</param>
        /// <param name="date">Last modified date (MS-DOS time)</param>
        /// <param name="time">Last modified time (MS-DOS time)</param>
        public CabinetFileInfo(string fileId, ushort date, ushort time)
        {
            this.fileId = fileId;
            this.date = date;
            this.time = time;
        }

        /// <summary>
        /// Gets the file Id of the file.
        /// </summary>
        /// <value>file Id</value>
        public string FileId
        {
            get { return this.fileId; }
        }

        /// <summary>
        /// Gets modified date (DOS format).
        /// </summary>
        public ushort Date
        {
            get { return this.date; }
        }

        /// <summary>
        /// Gets modified time (DOS format).
        /// </summary>
        public ushort Time
        {
            get { return this.time; }
        }
    }
}
