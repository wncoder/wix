//-----------------------------------------------------------------------
// <copyright file="MsiComponentSearch.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
//    
//    The use and distribution terms for this software are covered by the
//    Common Public License 1.0 (http://opensource.org/licenses/cpl.php)
//    which can be found in the file CPL.TXT at the root of this distribution.
//    By using this software in any fashion, you are agreeing to be bound by
//    the terms of this license.
//    
//    You must not remove this notice, or any other, from this software.
// </copyright>
// <summary>MsiComponentSearch element OM</summary>
//-----------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.ElementAttribute;

namespace Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnSearches
{
    [BurnXmlElement("MsiComponentSearch")]
    public class MsiComponentSearch
    {
        public enum MsiComponentSearchType
        {
            keyPath,
            state,
            directory
        }

        # region Private member

        private string mId;
        private string mCondition;
        private string mVariableId;
        private string mComponentId;
        private string mProductCode;
        private MsiComponentSearchType mType;

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

        [BurnXmlAttribute("Condition")]
        public string Condition
        {
            get
            {
                return mCondition;
            }
            set
            {
                mCondition = value;
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

        [BurnXmlAttribute("ComponentId")]
        public string ComponentId
        {
            get
            {
                return mComponentId;
            }
            set
            {
                mComponentId = value;
            }
        }

        [BurnXmlAttribute("ProductCode")]
        public string ProductCode
        {
            get
            {
                return mProductCode;
            }
            set
            {
                mProductCode = value;
            }
        }

        [BurnXmlAttribute("Type")]
        public MsiComponentSearchType Type
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

        # endregion
    }
}
