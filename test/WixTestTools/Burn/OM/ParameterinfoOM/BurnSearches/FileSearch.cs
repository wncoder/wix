//-----------------------------------------------------------------------
// <copyright file="FileSearch.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>FileSearch element OM</summary>
//-----------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.ElementAttribute;

namespace Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnSearches
{
    [BurnXmlElement("FileSearch")]
    public class FileSearch
    {
        public enum FileSearchType
        {
            exists,
            version
        }

        # region Private member

        private string mId;
        private FileSearch.FileSearchType mType;
        private string mPath;
        private string mVariableId;

        # endregion

        # region Public property

        [BurnXmlAttribute("Id")]
        public string Id
        {
            get
            {
                return mId;
            }
            set
            {
                mId = value;
            }
        }

        [BurnXmlAttribute("Type")]
        public FileSearch.FileSearchType Type
        {
            get
            {
                return mType;
            }
            set
            {
                mType = value;
            }
        }

        [BurnXmlAttribute("Path")]
        public string Path
        {
            get
            {
                return mPath;
            }
            set
            {
                mPath = value;
            }
        }

        [BurnXmlAttribute("Variable")]
        public string Variable
        {
            get
            {
                return mVariableId;
            }
            set
            {
                mVariableId = value;
            }
        } 

        # endregion
    }
}
