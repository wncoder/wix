//-----------------------------------------------------------------------
// <copyright file="BurnItemsGroup.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>Class to define item container object</summary>
//-----------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.ElementAttribute;

namespace Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems
{
    [BurnXmlElement("Items")]
    public class BurnItemsGroup
    {
        private List<BurnBaseItems> m_Items;

        public List<BurnBaseItems> Items
        {
            get 
            {
                if (m_Items == null) m_Items = new List<BurnBaseItems>();
                return m_Items; 
            }
            set 
            { 
                m_Items = value; 
            }
        }
        [BurnXmlChildElement()]
        public BurnBaseItems[] ItemsArray
        {
            get { return Items.ToArray(); }
        }
    }
}
