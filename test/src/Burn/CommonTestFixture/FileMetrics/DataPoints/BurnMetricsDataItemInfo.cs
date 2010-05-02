//-----------------------------------------------------------------------
// <copyright file="BurnMetricsDataItemInfo.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>
//     - Contains methods used for getting Burn Metrics data.
// </summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.BurnFileMetrics
{
    using System;
    using System.Collections.Generic;

    public partial class BurnMetricsData
    {
        public enum ActionType
        {
            None = 0,
            Install,
            Repair,
            Uninstall,
            Download,
            Verify,
            Rollback,
            Decompress
        }
        
        public class Item
        {
            public string Name = "";
            public string Phase = "";
            public string Action = "";
            public string Result = "";
            public string Size = "";
            public string Time = "";
            public string Technology = "";
            public string ResultDetail = "";
            public string AttemptNumber = "";
            
            public override string ToString()
            {
                string retVal = "";

                if (!String.IsNullOrEmpty(this.Action))
                {
                    retVal += String.Format("Action: {0} ", this.Action);
                }
                if (!String.IsNullOrEmpty(this.AttemptNumber))
                {
                    retVal += String.Format("AttemptNumber: {0} ", this.AttemptNumber);
                }
                if (!String.IsNullOrEmpty(this.Name))
                {
                    retVal += String.Format("Name: {0} ", this.Name);
                }
                if (!String.IsNullOrEmpty(this.Phase))
                {
                    retVal += String.Format("Phase: {0} ", this.Phase);
                }
                if (!String.IsNullOrEmpty(this.Result))
                {
                    retVal += String.Format("Result: {0} ", this.Result);
                }
                if (!String.IsNullOrEmpty(this.ResultDetail))
                {
                    retVal += String.Format("ResultDetail: {0} ", this.ResultDetail);
                }
                if (!String.IsNullOrEmpty(this.Size))
                {
                    retVal += String.Format("Size: {0} ", this.Size);
                }
                if (!String.IsNullOrEmpty(this.Technology))
                {
                    retVal += String.Format("Technology: {0} ", this.Technology);
                }
                if (!String.IsNullOrEmpty(this.Time))
                {
                    retVal += String.Format("Time: {0} ", this.Time);
                }

                return retVal;
            }
        }

        public class BurnMetricsItemStream : IBurnMetricsDataVerification
        {
            /// <summary>
            /// All items in the ItemStream
            /// </summary>
            public List<Item> Items;

            /// <summary>
            /// Filtered list that just includes all non-downloaded items (install, repair, uninstall, verify, etc.) in the ItemStream
            /// </summary>
            public List<Item> NonDownloadItems
            {
                get
                {
                    List<Item> tempItems = new List<Item>();
                    foreach (Item item in this.Items)
                    {
                        if (int.Parse(item.Action) != (int)ActionType.Download)
                        {
                            tempItems.Add(item);
                        }
                    }

                    return tempItems;
                }
            }

            /// <summary>
            /// Filtered list that just includes the downloaded items in the ItemStream
            /// </summary>
            public List<Item> DownloadItems
            {
                get
                {
                    return GetItemsByActionType(ActionType.Download);
                }
            }

            /// <summary>
            /// Filtered list that just includes all install items in the ItemStream
            /// </summary>
            public List<Item> InstallItems
            {
                get
                {
                    return GetItemsByActionType(ActionType.Install);
                }
            }

            /// <summary>
            /// Filtered list that just includes all repair items in the ItemStream
            /// </summary>
            public List<Item> RepairItems
            {
                get
                {
                    return GetItemsByActionType(ActionType.Repair);
                }
            }

            /// <summary>
            /// Filtered list that just includes all uninstall items in the ItemStream
            /// </summary>
            public List<Item> UninstallItems
            {
                get
                {
                    return GetItemsByActionType(ActionType.Uninstall);
                }
            }

            /// <summary>
            /// Filtered list that just includes all verify items in the ItemStream
            /// </summary>
            public List<Item> VerifyItems
            {
                get
                {
                    return GetItemsByActionType(ActionType.Verify);
                }
            }

            /// <summary>
            /// Filtered list that just includes all verify items in the ItemStream
            /// </summary>
            public List<Item> DecompressItems
            {
                get
                {
                    return GetItemsByActionType(ActionType.Decompress);
                }
            }

            private List<Item> GetItemsByActionType(ActionType actionType)
            {
                List<Item> tempItems = new List<Item>();
                foreach (Item item in this.Items)
                {
                    if (int.Parse(item.Action) == (int)actionType)
                    {
                        tempItems.Add(item);
                    }
                }

                return tempItems;
            }

            public BurnMetricsItemStream(string BurnMetricsXmlFile)
            {
                Items = BurnMetricsXml.GetItemStream(BurnMetricsXmlFile);
            }

            public bool IsValidValue()
            {
                bool ItemsAreCorrect = true;
                if (Items.Count == 0) ItemsAreCorrect = false;
                foreach (Item item in Items)
                {
                    bool itemVerified = (
                        (!String.IsNullOrEmpty(item.Name)) &&
                        (!String.IsNullOrEmpty(item.Phase) && long.Parse(item.Phase) >= 1) &&
                        (!String.IsNullOrEmpty(item.Action) && long.Parse(item.Action) >= 1) &&
                        (!String.IsNullOrEmpty(item.Result)) &&
                        (!String.IsNullOrEmpty(item.Size) && long.Parse(item.Size) >= 0) &&
                        (!String.IsNullOrEmpty(item.Time) && long.Parse(item.Time) > 0) &&
                        (!String.IsNullOrEmpty(item.Technology) && long.Parse(item.Technology) >= 1) &&
                        (!String.IsNullOrEmpty(item.ResultDetail))
                        );
                    if (!itemVerified)
                    {
                        ItemsAreCorrect = false;
                        Console.WriteLine("BurnMetricsItemStream contains invalid data.");
                    }
                }
                return (ItemsAreCorrect);
            }

        }

    }
}
