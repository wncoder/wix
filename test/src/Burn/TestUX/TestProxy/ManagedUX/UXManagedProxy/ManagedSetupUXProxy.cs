//-----------------------------------------------------------------------
// <copyright file="ManagedSetupUXProxy.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>Acts as client server for pipe communication between Burn engine and mstest</summary>
//-----------------------------------------------------------------------

namespace ManagedBurnProxy
{
    using System;
    using System.Windows.Forms;
    using System.Threading;
    using System.Runtime.InteropServices;
    using System.Collections.Generic;
    using System.Xml;
    using System.IO;
    using System.Text;
    using System.Reflection;  
    using System.IO.Pipes;   
    using ParameterInfoConfigurator.BurnSearches;
    using Tests.Burn.Tests; 

    public static class ManagedSetupUXProxy
    {
        public static bool Initialize()
        {
            try
            {
                // Create client pipe           
                NamedPipeClientStream pipeClient = new NamedPipeClientStream
                    (".", "BurnCoreMethodCall", PipeDirection.In);

                using (StreamReader sr = new StreamReader(pipeClient))
                {
                    while (true)
                    {
                        if (!pipeClient.IsConnected)
                        {
                            pipeClient.Connect();
                        }

                        string temp;
                        while ((temp = sr.ReadLine()) != null && temp.Trim().ToLower() != "exit")
                        {
                            ParseMessage(temp);
                        }

                        if (string.IsNullOrEmpty(temp))
                            continue;

                        if (temp.Trim().ToLower() == "exit")
                        {
                            break;
                        }
                    }
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
                return false;
            }
            return true;
        }

        private static void ParseMessage(string message)
        {
            XmlDocument xmlDoc = new XmlDocument();
            xmlDoc.LoadXml(message);

            XmlNode node = xmlDoc.SelectSingleNode("./BurnCoreMethod/Method");
            string methodName = node.Attributes["Name"].Value;
            string variableName = string.Empty;

            if (node.Attributes["Variable"] != null)
                variableName = node.Attributes["Variable"].Value;

            if (methodName.ToLower().StartsWith("get"))
            {
                int stringLen = 0;
                if (node.Attributes["StringLen"] != null)
                {
                    stringLen =Convert.ToInt32(node.Attributes["StringLen"].Value);
                }

                CallGetVariableMethod(methodName, variableName, stringLen);
            }
            else if (methodName.ToLower().StartsWith("set"))
            {
                string value = node.Attributes["Value"].Value;
                CallSetVariableMethod(methodName, variableName, value);
            }
            else if (methodName == "EvaluateCondition")
            {
                string condStatement = node.Attributes["ConditionStatement"].Value;
                CallEvalulateCondition(condStatement);
            }
        }

        private static void CallEvalulateCondition(string conditionStatement)
        {
            try
            {
                int returnValue = -1;
                bool evalResult = false;
                conditionStatement = conditionStatement.Replace("&lt;", "<").Replace("&gt;", ">");
                returnValue = BurnCorePInvokeCalls.EvaluateCondition(conditionStatement, out evalResult);
                WriteEvalConditionResult("EvaluateCondition", evalResult.ToString(), conditionStatement, returnValue.ToString());
            }
            catch (Exception ex)
            {
                MessageBox.Show(string.Format("Error occured while evalulating condition statement: {0}. Error message: {1} "
                    , conditionStatement, ex.Message));
            }
        }

        private static void CallGetVariableMethod(string methodName, string variable, int stringValuelength)
        {
            try
            {
                int returnValue = 0;

                switch (methodName)
                {
                    case "GetVariableNumeric":
                        long pllValue = -1;
                        returnValue = BurnCorePInvokeCalls.GetVariableNumeric(variable, out pllValue);
                        WriteGetSetVariableResult("GetVariableNumeric", variable, pllValue.ToString(), returnValue.ToString());
                        break;

                    case "GetVariableString":
                        StringBuilder sb = new StringBuilder();
                        int pcchValue = stringValuelength;
                        returnValue = BurnCorePInvokeCalls.GetVariableString(variable, sb, ref pcchValue);
                        WriteGetSetVariableResult("GetVariableString", variable, sb.ToString(), returnValue.ToString());
                        break;

                    case "GetVariableVersion":
                        long pqwValue = 0;
                        returnValue = BurnCorePInvokeCalls.GetVariableVersion(variable, out pqwValue);
                        WriteGetSetVariableResult("GetVariableVersion", variable, pqwValue.ToString(), returnValue.ToString());
                        break;
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
        }

        private static void WriteGetSetVariableResult(string methodName, string variable, string value, string returnValue)
        {
            string resultFilePath = Environment.ExpandEnvironmentVariables(@"%temp%\BurnGetVariableResult.xml");

            if (File.Exists(resultFilePath))
            {
                File.Delete(resultFilePath);
            }

            string resultText = string.Format
                ("<BurnCoreMethodResult><Method methodname='{0}' variable='{1}' value='{2}' returnvalue='{3}' /></BurnCoreMethodResult>"
                , methodName, variable, value, returnValue);

            using (StreamWriter sw = new StreamWriter(resultFilePath))
            {
                sw.WriteLine(resultText);
            }
        }

        private static void WriteEvalConditionResult(string methodName, string evalResult, string evalCondition, string returnValue)
        {
            string resultFilePath = Environment.ExpandEnvironmentVariables(@"%temp%\BurnEvalResultResult.xml");

            if (File.Exists(resultFilePath))
            {
                File.Delete(resultFilePath);
            }
            evalCondition = evalCondition.Replace("<", "&lt;").Replace(">", "&gt;");
            string resultText = string.Format
                ("<BurnCoreMethodResult><Method methodname='{0}' Evalresult='{1}' EvalCondition='{2}' returnvalue='{3}' /></BurnCoreMethodResult>"
                , methodName, evalResult, evalCondition, returnValue);

            using (StreamWriter sw = new StreamWriter(resultFilePath))
            {
                sw.WriteLine(resultText);
            }
        }

        private static void CallSetVariableMethod(string methodName, string variable, string value)
        {
            int returnValue = 0;
            switch (methodName)
            {
                case "SetVariableNumeric":
                    returnValue = BurnCorePInvokeCalls.SetVariableNumeric(variable, Convert.ToInt64(value));
                    WriteGetSetVariableResult("SetVariableNumeric", variable, value, returnValue.ToString());
                    break;
                case "SetVariableString":
                    returnValue = BurnCorePInvokeCalls.SetVariableString(variable, value);
                    WriteGetSetVariableResult("SetVariableString", variable, value, returnValue.ToString());
                    break;
                case "SetVariableVersion":
                    returnValue = BurnCorePInvokeCalls.SetVariableVersion(variable, Convert.ToInt64(value));
                    WriteGetSetVariableResult("SetVariableVersion", variable, value, returnValue.ToString());
                    break;
            }
        }

    }
}