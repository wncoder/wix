namespace ExePkg
{
    using System;
    using Microsoft.Win32;

    class Program
    {
        static readonly string RegRoot = "HKEY_LOCAL_MACHINE";
        static readonly string RegKey = "SOFTWARE\\Microsoft\\WiX_Burn\\ExePackages";

        static int Main(string[] args)
        {
            int returnCode = 0;
            int wait = 0;

            if (0 == args.Length)
            {
                Console.WriteLine("usage: ExePkg.exe {-install|-uninstall|-repair}");
                return 1;
            }

            for (int i = 0; i < args.Length; ++i)
            {
                string arg = args[i];
                Console.Write(arg);

                switch (arg.ToLower())
                {
                    case "-install":
                        Registry.SetValue(RegRoot + "\\" + RegKey, "Exe1", "true", RegistryValueKind.String);
                        Console.WriteLine(": complete");
                        break;

                    case "-uninstall":
                        Registry.LocalMachine.DeleteSubKey(RegKey, false);
                        Console.WriteLine(": complete");
                        break;

                    case "-repair":
                        Registry.SetValue(RegRoot + "\\" + RegKey, "Exe1", "true", RegistryValueKind.String);
                        Console.WriteLine(": complete");
                        break;

                    case "-returnCode":
                        returnCode = int.Parse(args[++i]);
                        break;

                    case "-wait":
                        wait = int.Parse(args[++i]);
                        break;

                    default:
                        Console.WriteLine(": invalid argument");
                        return 2;
                }
            }

            if (0 < wait)
            {
                System.Threading.Thread.Sleep(wait);
            }

            return returnCode;
        }
    }
}
