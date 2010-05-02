//-----------------------------------------------------------------------
// <copyright file="BurnMetricsDataDynamic.cs" company="Microsoft">
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

    // Dynamic BurnMetrics data whose values are based on user actions and events

    public partial class BurnMetricsData
    {
        public class BurnMetricsOperationRequested : IBurnMetricsDataVerification
        {
            public string Value = "";

            public BurnMetricsOperationRequested(string BurnMetricsXmlFile)
            {
                Value = BurnMetricsXml.GetDwordValueFromId(BurnMetricsXmlFile, BurnMetricsConstants.BurnMetricsIds.DP_SPInstaller_OperationRequested);
            }

            public bool IsValidValue()
            {
                if (!String.IsNullOrEmpty(Value))
                {
                    return true;
                }
                else
                {
                    Console.WriteLine("BurnMetricsOperationRequested Value doesn't exist or is out of range.");
                    return false;
                }
            }
        }

        public class BurnMetricsCurrentItem : IBurnMetricsDataVerification
        {
            public string Value = "";

            public BurnMetricsCurrentItem(string BurnMetricsXmlFile)
            {
                Value = BurnMetricsXml.GetStringValueFromId(BurnMetricsXmlFile, BurnMetricsConstants.BurnMetricsIds.DP_SPInstaller_CurrentItem);
            }

            public bool IsValidValue()
            {
                if (!String.IsNullOrEmpty(Value))
                {
                    return true;
                }
                else
                {
                    Console.WriteLine("BurnMetricsCurrentItem Value doesn't exist or is out of range.");
                    return false;
                }
            }
        }

        public class BurnMetricsCurrentFlags : IBurnMetricsDataVerification
        {
            public string Value = "";

            public BurnMetricsCurrentFlags(string BurnMetricsXmlFile)
            {
                Value = BurnMetricsXml.GetStringValueFromId(BurnMetricsXmlFile, BurnMetricsConstants.BurnMetricsIds.DP_Setup_CurrentFlags2);
            }

            public bool IsValidValue()
            {
                if (!String.IsNullOrEmpty(Value))
                {
                    return true;
                }
                else
                {
                    Console.WriteLine("BurnMetricsCurrentFlags Value doesn't exist or is out of range.");
                    return false;
                }
            }
        }

        public class BurnMetricsReturnCode : IBurnMetricsDataVerification
        {
            public string Value = "";

            public BurnMetricsReturnCode(string BurnMetricsXmlFile)
            {
                Value = BurnMetricsXml.GetStringValueFromId(BurnMetricsXmlFile, BurnMetricsConstants.BurnMetricsIds.DP_Setup_ResultCode);
            }

            public bool IsValidValue()
            {
                if (!String.IsNullOrEmpty(Value))
                {
                    return true;
                }
                else
                {
                    Console.WriteLine("BurnMetricsReturnCode Value doesn't exist or is out of range.");
                    return false;
                }
            }
        }

        public class BurnMetricsResultDetail : IBurnMetricsDataVerification
        {
            public string Value = "";

            public BurnMetricsResultDetail(string BurnMetricsXmlFile)
            {
                Value = BurnMetricsXml.GetStringValueFromId(BurnMetricsXmlFile, BurnMetricsConstants.BurnMetricsIds.DP_Setup_ResultDetail);
            }

            public bool IsValidValue()
            {
                if (!String.IsNullOrEmpty(Value))
                {
                    return true;
                }
                else
                {
                    Console.WriteLine("BurnMetricsResultDetail Value doesn't exist or is out of range.");
                    return false;
                }
            }
        }


        public class BurnMetricsCurrentItemStep : IBurnMetricsDataVerification
        {
            public string Value = "";

            public BurnMetricsCurrentItemStep(string BurnMetricsXmlFile)
            {
                Value = BurnMetricsXml.GetStringValueFromId(BurnMetricsXmlFile, BurnMetricsConstants.BurnMetricsIds.DP_CurrentItemStep);
            }

            public bool IsValidValue()
            {
                if (!String.IsNullOrEmpty(Value))
                {
                    return true;
                }
                else
                {
                    Console.WriteLine("BurnMetricsCurrentItemStep Value doesn't exist or is out of range.");
                    return false;
                }
            }
        }


        public class BurnMetricsOperationUI : IBurnMetricsDataVerification
        {
            public string Value = "";

            public BurnMetricsOperationUI(string BurnMetricsXmlFile)
            {
                Value = BurnMetricsXml.GetDwordValueFromId(BurnMetricsXmlFile, BurnMetricsConstants.BurnMetricsIds.DP_SPInstaller_OperationUI);
            }

            public bool IsValidValue()
            {
                if (!String.IsNullOrEmpty(Value))
                {
                    return true;
                }
                else
                {
                    Console.WriteLine("BurnMetricsOperationUI Value doesn't exist or is out of range.");
                    return false;
                }
            }
        }

        public class BurnMetricsDisplayLCID : IBurnMetricsDataVerification
        {
            public string Value = "";

            public BurnMetricsDisplayLCID(string BurnMetricsXmlFile)
            {
                Value = BurnMetricsXml.GetDwordValueFromId(BurnMetricsXmlFile, BurnMetricsConstants.BurnMetricsIds.DP_SPInstaller_DisplayedLcid2);
            }

            public bool IsValidValue()
            {
                if (!String.IsNullOrEmpty(Value))
                {
                    return true;
                }
                else
                {
                    Console.WriteLine("BurnMetricsDisplayLCID Value doesn't exist or is out of range.");
                    return false;
                }
            }
        }

        public class BurnMetricsUserType : IBurnMetricsDataVerification
        {
            public string Value = "";

            public BurnMetricsUserType(string BurnMetricsXmlFile)
            {
                Value = BurnMetricsXml.GetDwordValueFromId(BurnMetricsXmlFile, BurnMetricsConstants.BurnMetricsIds.DP_UserType);
            }

            public bool IsValidValue()
            {
                if (!String.IsNullOrEmpty(Value))
                {
                    return true;
                }
                else
                {
                    Console.WriteLine("BurnMetricsUserType Value doesn't exist or is out of range.");
                    return false;
                }
            }
        }

        public class BurnMetricsIsInternal : IBurnMetricsDataVerification
        {
            public string Value = "";

            public BurnMetricsIsInternal(string BurnMetricsXmlFile)
            {
                Value = BurnMetricsXml.GetDwordValueFromId(BurnMetricsXmlFile, BurnMetricsConstants.BurnMetricsIds.sdpIsInternal);
            }

            public bool IsValidValue()
            {
                if (!String.IsNullOrEmpty(Value))
                {
                    return true;
                }
                else
                {
                    Console.WriteLine("BurnMetricsIsInternal Value doesn't exist or is out of range.");
                    return false;
                }
            }
        }

        public class BurnMetricsIsAdmin : IBurnMetricsDataVerification
        {
            public string Value = "";

            public BurnMetricsIsAdmin(string BurnMetricsXmlFile)
            {
                Value = BurnMetricsXml.GetBoolValueFromId(BurnMetricsXmlFile, BurnMetricsConstants.BurnMetricsIds.DP_SPInstaller_IsAdmin);
            }

            public bool IsValidValue()
            {
                if (!String.IsNullOrEmpty(Value))
                {
                    return true;
                }
                else
                {
                    Console.WriteLine("BurnMetricsIsAdmin Value doesn't exist or is out of range.");
                    return false;
                }
            }
        }

        public class BurnMetricsChainingPackage : IBurnMetricsDataVerification
        {
            public string Value = "";

            public BurnMetricsChainingPackage(string BurnMetricsXmlFile)
            {
                Value = BurnMetricsXml.GetStringValueFromId(BurnMetricsXmlFile, BurnMetricsConstants.BurnMetricsIds.DP_Setup_ChainingPackage);
            }

            public bool IsValidValue()
            {
                if (!String.IsNullOrEmpty(Value))
                {
                    return true;
                }
                else
                {
                    Console.WriteLine("BurnMetricsChainingPackage Value doesn't exist or is out of range.");
                    return false;
                }
            }
        }

        public class BurnMetricsRebootCount : IBurnMetricsDataVerification
        {
            public string Value = "";

            public BurnMetricsRebootCount(string BurnMetricsXmlFile)
            {
                Value = BurnMetricsXml.GetDwordValueFromId(BurnMetricsXmlFile, BurnMetricsConstants.BurnMetricsIds.DP_RebootCount);
            }

            public bool IsValidValue()
            {
                if (!String.IsNullOrEmpty(Value))
                {
                    return true;
                }
                else
                {
                    Console.WriteLine("BurnMetricsRebootCount Value doesn't exist or is out of range.");
                    return false;
                }
            }
        }




        public class BurnMetricsTimeToFirstWindow : IBurnMetricsDataVerification
        {
            public string Value = "";

            public BurnMetricsTimeToFirstWindow(string BurnMetricsXmlFile)
            {
                Value = BurnMetricsXml.GetDwordValueFromId(BurnMetricsXmlFile, BurnMetricsConstants.BurnMetricsIds.DP_SPInstaller_TimeToFirstWindow);
            }

            public bool IsValidValue()
            {
                if (!String.IsNullOrEmpty(Value))
                {
                    return true;
                }
                else
                {
                    Console.WriteLine("BurnMetricsTimeToFirstWindow Value doesn't exist or is out of range.");
                    return false;
                }
            }
        }

        public class BurnMetricsApplicableIfTime : IBurnMetricsDataVerification
        {
            public string Value = "";

            public BurnMetricsApplicableIfTime(string BurnMetricsXmlFile)
            {
                Value = BurnMetricsXml.GetDwordValueFromId(BurnMetricsXmlFile, BurnMetricsConstants.BurnMetricsIds.DP_SPInstaller_ApplicableIfTime);
            }

            public bool IsValidValue()
            {
                if (!String.IsNullOrEmpty(Value))
                {
                    return true;
                }
                else
                {
                    Console.WriteLine("BurnMetricsApplicableIfTime Value doesn't exist or is out of range.");
                    return false;
                }
            }
        }

        public class BurnMetricsSystemRequirementCheckTime : IBurnMetricsDataVerification
        {
            public string Value = "";

            public BurnMetricsSystemRequirementCheckTime(string BurnMetricsXmlFile)
            {
                Value = BurnMetricsXml.GetDwordValueFromId(BurnMetricsXmlFile, BurnMetricsConstants.BurnMetricsIds.DP_SPInstaller_SystemRequirementCheckTime);
            }

            public bool IsValidValue()
            {
                if (!String.IsNullOrEmpty(Value))
                {
                    return true;
                }
                else
                {
                    Console.WriteLine("BurnMetricsSystemRequirementCheckTime Value doesn't exist or is out of range.");
                    return false;
                }
            }
        }

        public class BurnMetricsInstallTime : IBurnMetricsDataVerification
        {
            public string Value = "";

            public BurnMetricsInstallTime(string BurnMetricsXmlFile)
            {
                Value = BurnMetricsXml.GetDwordValueFromId(BurnMetricsXmlFile, BurnMetricsConstants.BurnMetricsIds.DP_SPInstaller_InstallTime);
            }

            public bool IsValidValue()
            {
                if (!String.IsNullOrEmpty(Value))
                {
                    return true;
                }
                else
                {
                    Console.WriteLine("BurnMetricsInstallTime Value doesn't exist or is out of range.");
                    return false;
                }
            }
        }

        public class BurnMetricsCancelPage : IBurnMetricsDataVerification
        {
            public string Value = "";

            public BurnMetricsCancelPage(string BurnMetricsXmlFile)
            {
                Value = BurnMetricsXml.GetStringValueFromId(BurnMetricsXmlFile, BurnMetricsConstants.BurnMetricsIds.DP_Setup_CancelPage);
            }

            public bool IsValidValue()
            {
                if (!String.IsNullOrEmpty(Value))
                {
                    return true;
                }
                else
                {
                    Console.WriteLine("CancelPage Value doesn't exist or is out of range.");
                    return false;
                }
            }
        }



        public class Blocker
        {
            public string Type = "";
            public string Name = "";
        }
        public class BurnMetricsBlockerStream : IBurnMetricsDataVerification
        {
            public List<Blocker> Blockers;

            public BurnMetricsBlockerStream(string BurnMetricsXmlFile)
            {
                Blockers = BurnMetricsXml.GetBlockerStream(BurnMetricsXmlFile);
            }

            public bool IsValidValue()
            {
                bool ItemsAreCorrect = true;
                if (Blockers.Count == 0) ItemsAreCorrect = false;
                foreach (Blocker item in Blockers)
                {
                    bool itemVerified = (
                        (!String.IsNullOrEmpty(item.Name)) &&
                        (!String.IsNullOrEmpty(item.Type))
                        );
                    if (!itemVerified)
                    {
                        ItemsAreCorrect = false;
                        Console.WriteLine("BurnMetricsBlockerStream contains invalid data.");
                    }
                }
                return (ItemsAreCorrect);
            }

        }



        public class ApplicableSKU : IComparable
        {
            public string Guid = "";
            public string Name = "";

            #region IComparable Members

            public int CompareTo(object obj)
            {
                // make sure the Name and Guid match
                int nameComparison = this.Name.CompareTo(((ApplicableSKU)obj).Name);
                int guidComparison = this.Guid.CompareTo(((ApplicableSKU)obj).Guid);
                if (nameComparison != 0) 
                {
                    return nameComparison;
                }
                else if (guidComparison != 0)
                {
                    return guidComparison;
                }
                else
                {
                    return (0);
                }
            }

            #endregion
        }
        public class BurnMetricsApplicableSKUStream : IBurnMetricsDataVerification
        {
            public List<ApplicableSKU> ApplicableSKUs;

            public BurnMetricsApplicableSKUStream(string BurnMetricsXmlFile)
            {
                ApplicableSKUs = BurnMetricsXml.GetApplicableSKUStream(BurnMetricsXmlFile);
            }

            public bool IsValidValue()
            {
                bool ItemsAreCorrect = true;
                if (ApplicableSKUs.Count == 0) ItemsAreCorrect = false;
                foreach (ApplicableSKU applicableSKU in ApplicableSKUs)
                {
                    bool itemVerified = (
                        (!String.IsNullOrEmpty(applicableSKU.Name)) &&
                        (!String.IsNullOrEmpty(applicableSKU.Guid))
                        );
                    if (!itemVerified)
                    {
                        ItemsAreCorrect = false;
                        Console.WriteLine("BurnMetricsApplicableSKUStream contains invalid data.");
                    }
                }
                return (ItemsAreCorrect);
            }

        }

        public class Patch
        {
            public string MspDisplayName = "";
            public string ProductName = "";
            public string Train = "";
        }
        public class BurnMetricsPatchStream : IBurnMetricsDataVerification
        {
            public List<Patch> Patches;

            public BurnMetricsPatchStream(string BurnMetricsXmlFile)
            {
                Patches = BurnMetricsXml.GetPatchStream(BurnMetricsXmlFile);
            }

            public bool IsValidValue()
            {
                bool ItemsAreCorrect = true;
                if (Patches.Count == 0) ItemsAreCorrect = false;
                foreach (Patch item in Patches)
                {
                    bool itemVerified = (
                        (!String.IsNullOrEmpty(item.MspDisplayName)) &&
                        (!String.IsNullOrEmpty(item.ProductName))
                        );
                    if (!itemVerified)
                    {
                        ItemsAreCorrect = false;
                        Console.WriteLine("BurnMetricsPatchStream contains invalid data.");
                    }
                }
                return (ItemsAreCorrect);
            }

        }

    }
}
