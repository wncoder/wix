//-----------------------------------------------------------------------
// <copyright file="CustomErrorHandling.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>CustomErrorHandling element OM</summary>
//-----------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.ElementAttribute;

namespace Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnItems.BurnItemElements
{
    /// <summary>
    /// 
    /// </summary>
    [BurnXmlElement("CustomErrorHandling")]
    public class CustomErrorHandling
    {
        public abstract class CustomErrorActionType
        {
        }

        [BurnXmlElement("Success")]
        public class Success : CustomErrorActionType
        {
        }

        [BurnXmlElement("Failure")]
        public class Failure : CustomErrorActionType
        {
        }

        [BurnXmlElement("Retry")]
        public class Retry : CustomErrorActionType
        {
            private uint m_Delay;
            private uint m_Limit;
            private ItemRef m_ItemRef;

            [BurnXmlAttribute("Delay")]
            public uint Delay
            {
                get
                {
                    return m_Delay;
                }
                set
                {
                    m_Delay = value;
                }
            }

            [BurnXmlAttribute("Limit")]
            public uint Limit
            {
                get
                {
                    return m_Limit;
                }
                set
                {
                    m_Limit = value;
                }
            }

            [BurnXmlChildElement()]
            public ItemRef ItemRef
            {
                get
                {
                    return m_ItemRef;
                }
                set
                {
                    m_ItemRef = value;
                }
            }

        }

        [BurnXmlElement("ItemRef")]
        public class ItemRef
        {
            private string m_Name;
            private string m_CommandLine;
            private string m_LogFile;

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

            [BurnXmlAttribute("CommandLine")]
            public string CommandLine
            {
                get
                {
                    return m_CommandLine;
                }
                set
                {
                    m_CommandLine = value;
                }
            }

            [BurnXmlAttribute("LogFile")]
            public string LogFile
            {
                get
                {
                    return m_LogFile;
                }
                set
                {
                    m_LogFile = value;
                }
            }
        }

        [BurnXmlElement("CustomError")]
        public class CustomError
        {
            private string m_ReturnCode;
            private string m_MSIErrorMessage;
            private CustomErrorActionType m_CustomErrorAction;

            [BurnXmlAttribute("ReturnCode")]
            public string ReturnCode
            {
                get
                {
                    return m_ReturnCode;
                }
                set
                {
                    m_ReturnCode = value;
                }
            }

            [BurnXmlAttribute("MSIErrorMessage")]
            public string MSIErrorMessage
            {
                get
                {
                    return m_MSIErrorMessage;
                }
                set
                {
                    m_MSIErrorMessage = value;
                }
            }

            [BurnXmlChildElement()]
            public CustomErrorActionType CustomErrorAction
            {
                get
                {
                    return m_CustomErrorAction;
                }
                set
                {
                    m_CustomErrorAction = value;
                }
            }

        }

        private List<CustomError> m_CustomErrors;

        public List<CustomError> CustomErrors
        {
            get
            {
                if (m_CustomErrors == null) m_CustomErrors = new List<CustomError>();
                return m_CustomErrors;
            }
            set
            {
                m_CustomErrors = value;
            }
        }

        [BurnXmlChildElement()]
        public CustomError[] CustomErrorArray
        {
            get
            {
                if (null == m_CustomErrors) return null;
                return m_CustomErrors.ToArray();
            }
        }
    }
}
