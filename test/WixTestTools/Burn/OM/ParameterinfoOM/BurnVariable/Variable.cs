//-----------------------------------------------------------------------
// <copyright file="Variable.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
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
