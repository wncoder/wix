//-----------------------------------------------------------------------
// <copyright file="CleanupBlock.cs" company="Microsoft">
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
// <summary>CleanupBlock element OM</summary>
//-----------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems;
using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.ElementAttribute;

namespace Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnNonInstallableItems
{
    [BurnXmlElement("CleanupBlock")]
    public class CleanupBlockItem : BurnBaseItems
    {
        [BurnXmlElement("RemovePatch")]
        public class RemovePatchElement
        {
            private string m_PatchCode;

            [BurnXmlAttribute("PatchCode")]
            public string PatchCode
            {
                get
                {
                    return m_PatchCode;
                }
                set
                {
                    m_PatchCode = value;
                }
            }

            public RemovePatchElement()
            {
            }

            public RemovePatchElement(string patchCode)
            {
                PatchCode = patchCode;
            }
        }

        public class ProductCodeElement
        {
            private string m_ProductCode;

            [BurnXmlAttribute("ProductCode")]
            public string ProductCode
            {
                get
                {
                    return m_ProductCode;
                }
                set
                {
                    m_ProductCode = value;
                }
            }

            public ProductCodeElement()
            {
            }
            public ProductCodeElement(string productCode)
            {
                ProductCode = productCode;
            }
        }

        [BurnXmlElement("UnAdvertiseFeatures")]
        public class UnAdvertiseFeaturesElement : ProductCodeElement
        {
            public UnAdvertiseFeaturesElement() : base() { }
            public UnAdvertiseFeaturesElement(string productCode) : base(productCode) { }
        }

        [BurnXmlElement("RemoveProduct")]
        public class RemoveProductElement : ProductCodeElement
        {
            public RemoveProductElement() : base() { }
            public RemoveProductElement(string productCode) : base(productCode) { }
        }

        # region Private member variables

        private List<RemovePatchElement> m_RemovePatch;

        private List<UnAdvertiseFeaturesElement> m_UnAdvertiseFeatures;

        private List<RemoveProductElement> m_RemoveProduct;

        private string m_Name;

        private string m_CanonicalTargetName;

        private uint m_InstalledProductSize;

        private bool? m_DoUnAdvertiseFeaturesOnRemovePatch;

        private bool? m_PerMachine;

        #endregion

        # region Public Property

        public List<RemovePatchElement> RemovePatch
        {
            get
            {
                if (m_RemovePatch == null) m_RemovePatch = new List<RemovePatchElement>();
                return m_RemovePatch;
            }

            set
            {
                m_RemovePatch = value;
            }
        }

        [BurnXmlChildElement()]
        public RemovePatchElement[] RemovePatchArray
        {
            get
            {
                if (null == m_RemovePatch) return null;
                return m_RemovePatch.ToArray();
            }
        }
      
        public List<UnAdvertiseFeaturesElement> UnAdvertiseFeatures
        {
            get
            {
                if (m_UnAdvertiseFeatures == null) m_UnAdvertiseFeatures = new List<UnAdvertiseFeaturesElement>();
                return m_UnAdvertiseFeatures;
            }

            set
            {
                m_UnAdvertiseFeatures = value;
            }
        }

        [BurnXmlChildElement()]
        public UnAdvertiseFeaturesElement[] UnAdvertiseFeaturesArray
        {
            get
            {
                if (null == m_UnAdvertiseFeatures) return null;
                return m_UnAdvertiseFeatures.ToArray();
            }
        }

        public List<RemoveProductElement> RemoveProduct
        {
            get
            {
                if (m_RemoveProduct == null) m_RemoveProduct = new List<RemoveProductElement>();
                return m_RemoveProduct;
            }

            set
            {
                m_RemoveProduct = value;
            }
        }

        [BurnXmlChildElement()]
        public RemoveProductElement[] RemoveProductArray
        {
            get
            {
                if (null == m_RemoveProduct) return null;
                return m_RemoveProduct.ToArray();
            }
        }

        [BurnXmlAttribute("Name")]
        public string Name
        {
            get
            {
                return m_Name;
            }

            set
            {
                m_Name = value;
            }
        }

        [BurnXmlAttribute("CanonicalTargetName")]
        public string CanonicalTargetName
        {
            get
            {
                return m_CanonicalTargetName;
            }

            set
            {
                m_CanonicalTargetName = value;
            }
        }

        [BurnXmlAttribute("InstalledProductSize")]
        public uint InstalledProductSize
        {
            get
            {
                return m_InstalledProductSize;
            }

            set
            {
                m_InstalledProductSize = value;
            }
        }

        [BurnXmlAttribute("DoUnAdvertiseFeaturesOnRemovePatch")]
        public bool? DoUnAdvertiseFeaturesOnRemovePatch
        {
            get
            {
                return m_DoUnAdvertiseFeaturesOnRemovePatch;
            }

            set
            {
                m_DoUnAdvertiseFeaturesOnRemovePatch = value;
            }
        }

        [BurnXmlAttribute("PerMachine")]
        public bool? PerMachine
        {
            get
            {
                return m_PerMachine;
            }

            set
            {
                m_PerMachine = value;
            }
        }

        #endregion


    }
}
