//-----------------------------------------------------------------------
// <copyright file="Program.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>Defines a Frost application.</summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test.Frost
{
    using System;
    using System.Collections.Generic;
    using System.Text;
    using System.Threading;
    using System.Runtime.InteropServices;

    using Microsoft.Tools.WindowsInstallerXml.Test.Frost.Core;

    public class Program
    {
        [STAThread]
        static void Main(string[] args)
        {
            AppDomain currentDomain = AppDomain.CurrentDomain;
            currentDomain.UnhandledException += new UnhandledExceptionEventHandler(Application_UnhandledException);

            try
            {
                Frost test = new Frost();

                test.StartUXInterface();
                test.StopUXInterface();
            }
            catch (Exception e)
            {
                Console.WriteLine(e.Message);
            }
        }

        static void Application_UnhandledException(object sender, UnhandledExceptionEventArgs args)
        {
            Exception e = (Exception)args.ExceptionObject;
            Console.WriteLine("Frost : " + e.Message);
        }
    }
}
