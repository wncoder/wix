//-----------------------------------------------------------------------
// <copyright file="Variable.cs" company="Microsoft">
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
// <summary>Variable element OM</summary>
//-----------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.ElementAttribute;

namespace Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnVariable
{
    # region Public enum

    public enum VariableDataType
    {
        String,
        numeric,
        version        
    }

    # endregion

    [BurnXmlElement("Variable")]
    public class Variable
    {
        # region Private members

        private string mID;
        private string mValue;
        private VariableDataType mType;

        # endregion        

        # region Public property

        [BurnXmlAttribute("Id")]
        public string Id
        {
            get
            {
                return mID;
            }

            set
            {
                mID = value;
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

        [BurnXmlAttribute("Type")]
        public VariableDataType Type
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

        public Variable()
        {
        }
    }
}
