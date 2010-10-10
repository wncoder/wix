//-----------------------------------------------------------------------
// <copyright file="BurnMetricsXml.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
//    
//    The use and distribution terms for this software are covered by the
//    Common Public License 1.0 (http://opensource.org/licenses/cpl1.0.php)
//    which can be found in the file CPL.TXT at the root of this distribution.
//    By using this software in any fashion, you are agreeing to be bound by
//    the terms of this license.
//    
//    You must not remove this notice, or any other, from this software.
// </copyright>
// <summary>
//     - Contains methods used for getting Burn Metrics data.
// </summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.BurnFileMetrics
{
    using System;
    using System.Collections.Generic;
    using System.IO;
    using System.Xml;

    /// <summary>
    /// Handles retrieving data from the Burn Metrics XML file.
    /// </summary>
    public class BurnMetricsXml
    {

        public static string GetBoolValueFromId(string xmlFile, int id)
        {
            string xpath = @"/Data/Bool[@Id='" + id.ToString() + "']";
            return GetAttributeStringFromXpath(xmlFile, xpath, "Value");
        }

        public static string GetDwordValueFromId(string xmlFile, int id)
        {
            string xpath = @"/Data/Dword[@Id='" + id.ToString() + "']";
            return GetAttributeStringFromXpath(xmlFile, xpath, "Value");
        }

        public static string GetStringValueFromId(string xmlFile, int id)
        {
            string xpath = @"/Data/String[@Id='" + id.ToString() + "']";
            return GetAttributeStringFromXpath(xmlFile, xpath, "Value");
        }

        public static string GetAttributeStringFromXpath(string xmlFile, string xpath, string attribute)
        {
            string retVal = "";

            try
            {
                XmlDocument xmlDoc = new XmlDocument();
                xmlDoc.Load(xmlFile);
                XmlNode Datapoint = xmlDoc.DocumentElement.SelectSingleNode(xpath);
                retVal = Datapoint.Attributes[attribute].Value;
            }
            catch
            {
                // if the xpath fails to find data, just return an empty string
                retVal = "";
            }

            return retVal;
        }


        public static List<BurnMetricsData.Item> GetItemStream(string xmlFile)
        {
            List<BurnMetricsData.Item> Items = new List<BurnMetricsData.Item>();

            try
            {
                XmlDocument xmlDoc = new XmlDocument();
                xmlDoc.Load(xmlFile);
                XmlNodeList Streams = xmlDoc.DocumentElement.SelectNodes(@"/Data/Stream[@Id='" + BurnMetricsConstants.BurnMetricsIds.DP_Setup_ItemStream.ToString() + "']");

                foreach (XmlNode Stream in Streams)
                {
                    int valuesPerEntry = int.Parse(Stream.Attributes["NoOfColumns"].Value);
                    XmlNodeList nodeList = Stream.ChildNodes;
                    if (nodeList.Count != valuesPerEntry) throw new Exception("nodeList.Count != valuesPerEntry");
                    if (nodeList.Count != 9) throw new Exception("nodeList.Count != 9.  Expect 9 fields in the Item stream");

                    BurnMetricsData.Item item = new BurnMetricsData.Item();
                    item.Name = nodeList[0].Attributes["Value"].Value;
                    item.Phase = nodeList[1].Attributes["Value"].Value;
                    item.Action = nodeList[2].Attributes["Value"].Value;
                    item.Result = nodeList[3].Attributes["Value"].Value;
                    item.Size = nodeList[4].Attributes["Value"].Value;
                    item.Time = nodeList[5].Attributes["Value"].Value;
                    item.Technology = nodeList[6].Attributes["Value"].Value;
                    item.ResultDetail = nodeList[7].Attributes["Value"].Value;
                    item.AttemptNumber = nodeList[8].Attributes["Value"].Value;
                    Items.Add(item);
                }
            }
            catch
            {
                // if we get any error trying to read the xml, just return what we've got so far.
                // it just means no data exists for the elements we are querying in the xpath
            }

            return Items;
        }

        public static List<BurnMetricsData.Blocker> GetBlockerStream(string xmlFile)
        {
            List<BurnMetricsData.Blocker> Blockers = new List<BurnMetricsData.Blocker>();

            try
            {
                XmlDocument xmlDoc = new XmlDocument();
                xmlDoc.Load(xmlFile);
                XmlNodeList Streams = xmlDoc.DocumentElement.SelectNodes(@"/Data/Stream[@Id='" + BurnMetricsConstants.BurnMetricsIds.DP_Setup_BlockerStream.ToString() + "']");

                foreach (XmlNode Stream in Streams)
                {
                    int valuesPerEntry = int.Parse(Stream.Attributes["ValuesPerEntry"].Value);
                    XmlNodeList nodeList = Stream.ChildNodes;
                    if (nodeList.Count != valuesPerEntry) throw new Exception("nodeList.Count != valuesPerEntry");
                    if (nodeList.Count != 9) throw new Exception("nodeList.Count != 9.  Expect 9 fields in the Item stream");

                    BurnMetricsData.Blocker blocker = new BurnMetricsData.Blocker();
                    blocker.Type = nodeList[0].Attributes["Value"].Value;
                    blocker.Name = nodeList[1].Attributes["Value"].Value;
                    Blockers.Add(blocker);
                }
            }
            catch
            {
                // if we get any error trying to read the xml, just return what we've got so far.
                // it just means no data exists for the elements we are querying in the xpath
            }

            return Blockers;
        }

        public static List<BurnMetricsData.Patch> GetPatchStream(string xmlFile)
        {
            List<BurnMetricsData.Patch> Patches = new List<BurnMetricsData.Patch>();

            try
            {
                XmlDocument xmlDoc = new XmlDocument();
                xmlDoc.Load(xmlFile);
                XmlNodeList Streams = xmlDoc.DocumentElement.SelectNodes(@"/Data/Stream[@Id='" + BurnMetricsConstants.BurnMetricsIds.DP_Setup_PatchStream.ToString() + "']");

                foreach (XmlNode Stream in Streams)
                {
                    int valuesPerEntry = int.Parse(Stream.Attributes["ValuesPerEntry"].Value);
                    XmlNodeList nodeList = Stream.ChildNodes;
                    if (nodeList.Count != valuesPerEntry) throw new Exception("nodeList.Count != valuesPerEntry");
                    if (nodeList.Count != 9) throw new Exception("nodeList.Count != 9.  Expect 9 fields in the Item stream");

                    BurnMetricsData.Patch patch = new BurnMetricsData.Patch();
                    patch.MspDisplayName = nodeList[0].Attributes["Value"].Value;
                    patch.ProductName = nodeList[1].Attributes["Value"].Value;
                    patch.Train = nodeList[2].Attributes["Value"].Value;
                    Patches.Add(patch);
                }
            }
            catch
            {
                // if we get any error trying to read the xml, just return what we've got so far.
                // it just means no data exists for the elements we are querying in the xpath
            }

            return Patches;
        }

        public static List<BurnMetricsData.ApplicableSKU> GetApplicableSKUStream(string xmlFile)
        {
            List<BurnMetricsData.ApplicableSKU> ApplicableSKUs = new List<BurnMetricsData.ApplicableSKU>();

            try
            {
                XmlDocument xmlDoc = new XmlDocument();
                xmlDoc.Load(xmlFile);
                XmlNodeList Streams = xmlDoc.DocumentElement.SelectNodes(@"/Data/Stream[@Id='" + BurnMetricsConstants.BurnMetricsIds.DP_SPInstaller_ApplicableSKU.ToString() + "']");

                foreach (XmlNode Stream in Streams)
                {
                    int valuesPerEntry = int.Parse(Stream.Attributes["ValuesPerEntry"].Value);
                    XmlNodeList nodeList = Stream.ChildNodes;
                    if (nodeList.Count != valuesPerEntry) throw new Exception("nodeList.Count != valuesPerEntry");
                    if (nodeList.Count != 9) throw new Exception("nodeList.Count != 9.  Expect 9 fields in the Item stream");

                    BurnMetricsData.ApplicableSKU applicableSKU = new BurnMetricsData.ApplicableSKU();
                    applicableSKU.Guid = nodeList[0].Attributes["Value"].Value;
                    applicableSKU.Name = nodeList[1].Attributes["Value"].Value;
                    ApplicableSKUs.Add(applicableSKU);
                }
            }
            catch
            {
                // if we get any error trying to read the xml, just return what we've got so far.
                // it just means no data exists for the elements we are querying in the xpath
            }

            return ApplicableSKUs;
        }
    }
}
