//-------------------------------------------------------------------------------------------------
// <copyright file="NewsItem.cs" company="Microsoft Corporation">
//   Copyright (c) 2004, Microsoft Corporation.
//   This software is released under Common Public License Version 1.0 (CPL).
//   The license and further copyright text can be found in the file LICENSE.TXT
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
// 
// <summary>
// The model for an individual news item.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.UX
{
    using System;

    /// <summary>
    /// The model for an individual news item.
    /// </summary>
    public class NewsItem
    {
        public string Author { get; set; }
        public string Title { get; set; }
        public string Url { get; set; }
        public string Snippet { get; set; }
        public DateTime Updated { get; set; }
    }
}
