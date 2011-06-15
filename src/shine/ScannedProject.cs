﻿using System;
using System.Collections.Generic;
using System.Text;

namespace Microsoft.Tools.WindowsInstallerXml
{
    public enum ScannedProjectType
    {
        Unknown,
        Bundle,
        Library,
        Module,
        Package,
    }

    public class ScannedProject
    {
        public ScannedProject(string typeName, string path)
            : this(typeName, path, null, null)
        {
        }

        public ScannedProject(string typeName, string path, IDictionary<string, string> properties, string condition)
        {
            try
            {
                this.Type = (ScannedProjectType)Enum.Parse(typeof(ScannedProjectType), typeName);
            }
            catch (ArgumentException)
            {
                this.Type = ScannedProjectType.Unknown;
            }

            this.Path = path;
            this.Properties = new Dictionary<string, string>(StringComparer.OrdinalIgnoreCase);
            if (null != properties)
            {
                foreach (KeyValuePair<string, string> kvp in properties)
                {
                    this.Properties.Add(kvp.Key, kvp.Value);
                }
            }

            if (!String.IsNullOrEmpty(condition))
            {
                this.Condition = condition;
            }

            this.Key = ScannedProject.CalculateKey(this.Path, this.Properties);
        }

        public string Key { get; private set; }

        public string Condition { get; private set; }

        public string Path { get; private set; }

        public IDictionary<string, string> Properties { get; private set; }

        public ScannedProjectType Type { get; private set; }

        public static string CalculateKey(string path, IDictionary<string, string> properties)
        {
            StringBuilder keyBuilder = new StringBuilder();
            keyBuilder.Append(path.ToLowerInvariant());

            if (null != properties)
            {
                foreach (KeyValuePair<string, string> kvp in properties)
                {
                    keyBuilder.AppendFormat(";{0}={1}", kvp.Key, kvp.Value);
                }
            }

            return keyBuilder.ToString();
        }
    }
}
