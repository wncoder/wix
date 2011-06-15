using System;

namespace Microsoft.Tools.WindowsInstallerXml
{
    public enum ScannedSymbolType
    {
        Bundle,
        PackageGroup,
        PayloadGroup,
        Payload,
        MsiPackage,
        ExePackage,
        MspPackage,
        MsuPackage,
        Product,
        Module,
        Feature,
        ComponentGroup,
        Component,
        File,
        Shortcut,
        ServiceInstall,
    }

    public class ScannedSymbol
    {
        public ScannedSymbol(string typeName, string id)
        {
            this.Id = id;
            this.Type = (ScannedSymbolType)Enum.Parse(typeof(ScannedSymbolType), typeName);

            this.Key = String.Concat(this.Type, ":", this.Id);
        }

        public string Key { get; private set;  }

        public string Id { get; private set; }

        public ScannedSymbolType Type { get; private set; }

        public static string CalculateKey(string typeName, string id)
        {
            ScannedSymbolType type = (ScannedSymbolType)Enum.Parse(typeof(ScannedSymbolType), typeName);
            return String.Concat(type, ":", id);
        }
    }
}
