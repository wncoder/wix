//-----------------------------------------------------------------------
// <copyright file="BurnItemsGroup.cs" company="Microsoft">
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
