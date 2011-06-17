//-------------------------------------------------------------------------------------------------
// <copyright file="NewsViewModel.cs" company="Microsoft">
// Copyright (c) Microsoft Corporation. All rights reserved.
//    
//    The use and distribution terms for this software are covered by the
//    Common Public License 1.0 (http://opensource.org/licenses/cpl1.0.php)
//    which can be found in the file CPL.TXT at the root of this distribution.
//    By using this software in any fashion, you are agreeing to be bound by
//    the terms of this license.
//    
//    You must not remove this notice, or any other, from this software.
// </copyright>
// 
// <summary>
// The model of the news view.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.UX
{
    using System;
    using System.Collections.Generic;
    using System.ComponentModel;
    using System.Linq;
    using System.Net;
    using System.ServiceModel.Syndication;
    using System.Text.RegularExpressions;
    using System.Windows.Input;
    using System.Xml;

    /// <summary>
    /// The states of the update view model.
    /// </summary>
    public enum NewsState
    {
        Initializing,
        Downloading,
        Complete,
        Failed,
    }

    /// <summary>
    /// The model of the news view.
    /// </summary>
    public class NewsViewModel : PropertyNotifyBase
    {
        private ICommand launchCommand;

        private IEnumerable<NewsItem> items;
        private NewsState state;
        private string title;

        private BackgroundWorker worker;

        public NewsViewModel()
        {
            this.title = "News about WiX";

            this.worker = new BackgroundWorker();
            this.worker.DoWork += new DoWorkEventHandler(worker_DoWork);
        }

        /// <summary>
        /// Gets and sets the command that launches the news items URLs.
        /// </summary>
        public ICommand LaunchCommand
        {
            get
            {
                if (this.launchCommand == null)
                {
                    this.launchCommand = new RelayCommand(param => WixUX.LaunchUrl((string)param), param => param is string);
                }

                return this.launchCommand;
            }
        }

        /// <summary>
        /// Gets and sets the items of the news view model.
        /// </summary>
        public IEnumerable<NewsItem> Items
        {
            get
            {
                return this.items;
            }

            set
            {
                if (this.items != value)
                {
                    this.items = value;
                    base.OnPropertyChanged("Items");
                }
            }
        }

        /// <summary>
        /// Gets and sets the state of the news view model.
        /// </summary>
        public NewsState State
        {
            get
            {
                return this.state;
            }

            set
            {
                if (this.state != value)
                {
                    this.state = value;
                    base.OnPropertyChanged("State");
                }
            }
        }

        /// <summary>
        /// Gets and sets the title of the news view model.
        /// </summary>
        public string Title
        {
            get
            {
                return this.title;
            }

            set
            {
                if (this.title != value)
                {
                    this.title = value;
                    base.OnPropertyChanged("Title");
                }
            }
        }

        /// <summary>
        /// Causes the news view to check entries again.
        /// </summary>
        public void Refresh()
        {
            // If we're already checking for entries, skip the refresh.
            if (this.State == NewsState.Downloading)
            {
                return;
            }

            this.State = NewsState.Downloading;
            this.worker.RunWorkerAsync();
        }

        /// <summary>
        /// Worker thread to check for updates.
        /// </summary>
        /// <param name="sender">Sender.</param>
        /// <param name="e">Arguments.</param>
        private void worker_DoWork(object sender, DoWorkEventArgs e)
        {
            bool succeeded = false;
            try
            {
                HttpWebRequest request = WixUX.Model.CreateWebRequest("http://wix.sourceforge.net/releases/wix3.6.feed");
                HttpWebResponse response = (HttpWebResponse)request.GetResponse();
                if (response.StatusCode == HttpStatusCode.OK)
                {
                    SyndicationFeed feed;
                    using (XmlReader reader = XmlReader.Create(response.GetResponseStream()))
                    {
                        feed = SyndicationFeed.Load(reader);
                    }

                    this.Items = this.GetNewsItems(feed);
                    this.State = NewsState.Complete;

                    succeeded = true;
                }
            }
            catch (ArgumentException)
            {
            }
            catch (FormatException)
            {
            }
            catch (OverflowException)
            {
            }
            catch (WebException)
            {
            }
            catch (XmlException)
            {
            }

            if (!succeeded)
            {
                this.State = NewsState.Failed;
                this.Items = null;
            }
        }

        /// <summary>
        /// Gets the news items from the feed.
        /// </summary>
        /// <param name="feed">Syndication feed.</param>
        /// <returns>New news items.</returns>
        private IEnumerable<NewsItem> GetNewsItems(SyndicationFeed feed)
        {
            List<NewsItem> items = new List<NewsItem>();
            SyndicationPerson feedAuthor = feed.Authors.FirstOrDefault();
            foreach (SyndicationItem entry in feed.Items)
            {
                NewsItem item = new NewsItem();

                SyndicationPerson author = entry.Authors.FirstOrDefault() ?? feedAuthor;
                if (author != null)
                {
                    item.Author = author.Name;
                }

                SyndicationLink link = entry.Links.Where(l => String.Equals(l.RelationshipType, "alternate", StringComparison.Ordinal)).FirstOrDefault();
                if (link != null)
                {
                    item.Url = link.Uri.AbsoluteUri;
                }

                item.Title = entry.Title.Text;
                item.Snippet = this.GetContentSnippet(entry.Content ?? entry.Summary);
                item.Updated = entry.LastUpdatedTime.ToLocalTime().DateTime;
                items.Add(item);
            }

            return items;
        }

        /// <summary>
        /// Gets a small bit of text from the syndication entry.
        /// </summary>
        /// <param name="content">Content to create snippet from.</param>
        /// <returns>Snippet or null if no snippet could be found.</returns>
        private string GetContentSnippet(SyndicationContent content)
        {
            string value = null;

            // Only text based content syndication is supported right now.
            if (content is TextSyndicationContent)
            {
                TextSyndicationContent text = (TextSyndicationContent)content;
                switch (text.Type)
                {
                    case "html": // strip out all the HTML goo.
                        value = Regex.Replace(text.Text, "<.*?>", String.Empty);
                        break;

                    case "text":
                    default:
                        value = text.Text;
                        break;
                }
            }

            // Trim to 200 characters.
            if (!String.IsNullOrEmpty(value) && value.Length > 197)
            {
                value = String.Concat(value.Substring(0, 197), "...");
            }

            return value;
        }
    }
}
