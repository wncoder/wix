//-----------------------------------------------------------------------
// <copyright file="RetryHelper.cs" company="Microsoft">
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
// <summary>RetryHelper element OM</summary>
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
    [BurnXmlElement("RetryHelper")]
    public class RetryHelper : BurnDownloadableBaseItem
    {
        //These attributes come from BurnDownloadableBaseItem
        //Name
        //URL
        //DownloadSize
        //SystemDriveSize
        //InstalledProductSize
        //HashValue

        private string m_CommandLine;
        private string m_CanonicalTargetName;
        private string m_LogFileHint;
        private Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnInstallableItems.ExeItem.ExeTypeEnum? m_ExeType;

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

        [BurnXmlAttribute("LogFileHint")]
        public string LogFileHint
        {
            get
            {
                return m_LogFileHint;
            }
            set
            {
                m_LogFileHint = value;
            }
        }

        [BurnXmlAttribute("ExeType")]
        public Microsoft.Tools.WindowsInstallerXml.Test.Burn.OM.ParameterInfoOM.BurnInstallableItems.ExeItem.ExeTypeEnum? ExeType
        {
            get
            {
                return m_ExeType;
            }
            set
            {
                m_ExeType = value;
            }
        }

    }
}
