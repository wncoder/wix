//-----------------------------------------------------------------------
// <copyright file="RegistrySearch.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>RegistrySearch element OM</summary>
//-----------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.ElementAttribute;

namespace Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnSearches
{
    public enum Type
    {
        exists,
        value
    }

    [BurnXmlElement("RegistrySearch")]
    public class RegistrySearch
    {
        # region Public enum

        public enum RegRoot
        {
            HKCU,
            HKLM,
            HKCR,
            HKU
        }

        public enum YesNoType
        {
            yes,
            no
        }       

        # endregion

        # region Private member

        private string mId;
        private BurnSearches.Type mType;
        private RegRoot mRoot;
        private string mKey;
        private string mValue;
        private BurnVariable.VariableDataType mDataType;
        private YesNoType mExpandEnvironment;
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
        public BurnSearches.Type Type
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

        [BurnXmlAttribute("Root")]
        public RegRoot Root
        {
            get
            {
                return mRoot;
            }
            set
            {
                mRoot = value;
            }
        }

        [BurnXmlAttribute("Key")]
        public string Key
        {
            get
            {
                return mKey;
            }
            set
            {
                mKey = value;
            }
        }

        [BurnXmlAttribute("Value")]
        public string Value
        {
            get
            {
                return mValue;
            }
            set
            {
                mValue = value;
            }
        }

        [BurnXmlAttribute("VariableType")]
        public BurnVariable.VariableDataType VariableType
        {
            get
            {
                return mDataType;
            }
            set
            {
                mDataType = value;
            }
        }

        [BurnXmlAttribute("ExpandEnvironment")]
        public YesNoType ExpandEnvironment
        {
            get
            {
                return mExpandEnvironment;
            }
            set
            {
                mExpandEnvironment = value;
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
