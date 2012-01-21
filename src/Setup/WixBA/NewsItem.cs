//-------------------------------------------------------------------------------------------------
// <copyright file="NewsItem.cs" company="Microsoft">
// Copyright (c) Microsoft Corporation. All rights reserved.
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
