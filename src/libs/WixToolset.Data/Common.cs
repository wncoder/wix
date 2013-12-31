//-------------------------------------------------------------------------------------------------
// <copyright file="Common.cs" company="Outercurve Foundation">
//   Copyright (c) 2004, Outercurve Foundation.
//   This software is released under Microsoft Reciprocal License (MS-RL).
//   The license and further copyright text can be found in the file
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
//-------------------------------------------------------------------------------------------------

namespace WixToolset.Data
{
    using System;
    using System.Diagnostics;
    using System.Globalization;
    using System.IO;
    using System.Linq;
    using System.Security.Cryptography;
    using System.Text;
    using System.Text.RegularExpressions;
    using System.Xml;
    using System.Xml.Linq;

    internal static class Common
    {
        public const int IntegerNotSet = int.MinValue;
        public const int IllegalInteger = int.MinValue + 1;
        public const long LongNotSet = long.MinValue;
        public const long IllegalLong = long.MinValue + 1;
        public const string IllegalGuid = "IllegalGuid";
        public static readonly Version IllegalVersion = new Version(Int32.MaxValue, Int32.MaxValue, Int32.MaxValue, Int32.MaxValue);

        internal static readonly XNamespace W3SchemaPrefix = "http://www.w3.org/";
        private static readonly Regex LegalIdentifierCharacters = new Regex(@"^[_A-Za-z][0-9A-Za-z_\.]*$", RegexOptions.Compiled);

        internal static string GetFileHash(FileInfo fileInfo)
        {
            byte[] hashBytes;
            using (SHA1Managed managed = new SHA1Managed())
            {
                using (FileStream stream = fileInfo.OpenRead())
                {
                    hashBytes = managed.ComputeHash(stream);
                }
            }

            StringBuilder sb = new StringBuilder();
            for (int i = 0; i < hashBytes.Length; i++)
            {
                sb.AppendFormat("{0:X2}", hashBytes[i]);
            }

            return sb.ToString();
        }

        public static bool IsIdentifier(string value)
        {
            if (!String.IsNullOrEmpty(value))
            {
                if (LegalIdentifierCharacters.IsMatch(value))
                {
                    return true;
                }
            }

            return false;
        }

        /// <summary>
        /// Gets a valid code page from the given web name or integer value.
        /// </summary>
        /// <param name="value">A code page web name or integer value as a string.</param>
        /// <param name="allowNoChange">Whether to allow -1 which does not change the database code pages. This may be the case with wxl files.</param>
        /// <param name="onlyAnsi">Whether to allow Unicode (UCS) or UTF code pages.</param>
        /// <param name="sourceLineNumbers">Source line information for the current authoring.</param>
        /// <returns>A valid code page number.</returns>
        /// <exception cref="ArgumentOutOfRangeException">The value is an integer less than 0 or greater than 65535.</exception>
        /// <exception cref="ArgumentNullException"><paramref name="value"/> is null.</exception>
        /// <exception cref="NotSupportedException">The value doesn't not represent a valid code page name or integer value.</exception>
        /// <exception cref="WixException">The code page is invalid for summary information.</exception>
        internal static int GetValidCodePage(string value, bool allowNoChange, bool onlyAnsi, SourceLineNumber sourceLineNumbers)
        {
            int codePage;
            Encoding enc;

            if (null == value)
            {
                throw new ArgumentNullException("value");
            }

            try
            {
                // check if a integer as a string was passed
                if (int.TryParse(value, out codePage))
                {
                    if (0 == codePage)
                    {
                        // 0 represents a neutral database
                        return 0;
                    }
                    else if (allowNoChange && -1 == codePage)
                    {
                        // -1 means no change to the database code page
                        return -1;
                    }

                    enc = Encoding.GetEncoding(codePage);
                }
                else
                {
                    enc = Encoding.GetEncoding(value);
                }

                // Windows Installer parses some code page references
                // as unsigned shorts which fail to open the database.
                if (onlyAnsi)
                {
                    codePage = enc.CodePage;
                    if (0 > codePage || short.MaxValue < codePage)
                    {
                        throw new WixException(WixDataErrors.InvalidSummaryInfoCodePage(sourceLineNumbers, codePage));
                    }
                }

                return enc.CodePage;
            }
            catch (ArgumentException ex)
            {
                // rethrow as NotSupportedException since either can be thrown
                // if the system does not support the specified code page
                throw new NotSupportedException(ex.Message, ex);
            }
        }

        internal static string GetAttributeIdentifierValue(SourceLineNumber sourceLineNumbers, XAttribute attribute, Action<MessageEventArgs> messageHandler)
        {
            string value = Common.GetAttributeValue(sourceLineNumbers, attribute, EmptyRule.CanBeWhitespaceOnly, messageHandler);

            if (Common.IsIdentifier(value))
            {
                if (72 < value.Length && null != messageHandler)
                {
                    messageHandler(WixDataWarnings.IdentifierTooLong(sourceLineNumbers, attribute.Parent.Name.LocalName, attribute.Name.LocalName, value));
                }

                return value;
            }
            else
            {
                if (value.StartsWith("[", StringComparison.Ordinal) && value.EndsWith("]", StringComparison.Ordinal) && null != messageHandler)
                {
                    messageHandler(WixDataErrors.IllegalIdentifierLooksLikeFormatted(sourceLineNumbers, attribute.Parent.Name.LocalName, attribute.Name.LocalName, value));
                }
                else if (null != messageHandler)
                {
                    messageHandler(WixDataErrors.IllegalIdentifier(sourceLineNumbers, attribute.Parent.Name.LocalName, attribute.Name.LocalName, value));
                }

                return String.Empty;
            }
        }

        public static int GetAttributeIntegerValue(SourceLineNumber sourceLineNumbers, XAttribute attribute, int minimum, int maximum, Action<MessageEventArgs> messageHandler)
        {
            Debug.Assert(minimum > Common.IntegerNotSet && minimum > Common.IllegalInteger, "The legal values for this attribute collide with at least one sentinel used during parsing.");

            string value = Common.GetAttributeValue(sourceLineNumbers, attribute, EmptyRule.CanBeWhitespaceOnly, messageHandler);
            int integer = Common.IllegalInteger;

            if (0 < value.Length)
            {
                try
                {
                    integer = Convert.ToInt32(value, CultureInfo.InvariantCulture.NumberFormat);

                    if (Common.IntegerNotSet == integer || Common.IllegalInteger == integer)
                    {
                        messageHandler(WixDataErrors.IntegralValueSentinelCollision(sourceLineNumbers, integer));
                    }
                    else if (minimum > integer || maximum < integer)
                    {
                        messageHandler(WixDataErrors.IntegralValueOutOfRange(sourceLineNumbers, attribute.Parent.Name.LocalName, attribute.Name.LocalName, integer, minimum, maximum));
                        integer = Common.IllegalInteger;
                    }
                }
                catch (FormatException)
                {
                    messageHandler(WixDataErrors.IllegalIntegerValue(sourceLineNumbers, attribute.Parent.Name.LocalName, attribute.Name.LocalName, value));
                }
                catch (OverflowException)
                {
                    messageHandler(WixDataErrors.IllegalIntegerValue(sourceLineNumbers, attribute.Parent.Name.LocalName, attribute.Name.LocalName, value));
                }
            }

            return integer;
        }

        internal static string GetAttributeValue(SourceLineNumber sourceLineNumbers, XAttribute attribute, EmptyRule emptyRule, Action<MessageEventArgs> messageHandler)
        {
            string value = attribute.Value;

            if ((emptyRule == EmptyRule.MustHaveNonWhitespaceCharacters && String.IsNullOrEmpty(value.Trim())) ||
                (emptyRule == EmptyRule.CanBeWhitespaceOnly && String.IsNullOrEmpty(value)))
            {
                if (null != messageHandler)
                {
                    messageHandler(WixDataErrors.IllegalEmptyAttributeValue(sourceLineNumbers, attribute.Parent.Name.LocalName, attribute.Name.LocalName));
                }

                return String.Empty;
            }

            return value;
        }

        internal static YesNoType GetAttributeYesNoValue(SourceLineNumber sourceLineNumbers, XAttribute attribute, Action<MessageEventArgs> messageHandler)
        {
            string value = Common.GetAttributeValue(sourceLineNumbers, attribute, EmptyRule.CanBeWhitespaceOnly, messageHandler);
            YesNoType yesNo = YesNoType.IllegalValue;

            if ("yes".Equals(value, StringComparison.Ordinal) || "true".Equals(value, StringComparison.Ordinal))
            {
                yesNo = YesNoType.Yes;
            }
            else if ("no".Equals(value, StringComparison.Ordinal) || "false".Equals(value, StringComparison.Ordinal))
            {
                yesNo = YesNoType.No;
            }
            else
            {
                if (null != messageHandler)
                {
                    messageHandler(WixDataErrors.IllegalYesNoValue(sourceLineNumbers, attribute.Parent.Name.LocalName, attribute.Name.LocalName, value));
                }
            }

            return yesNo;
        }

        /// <summary>
        /// Gets the text of an XElement.
        /// </summary>
        /// <param name="sourceLineNumbers">Source line information about the owner element.</param>
        /// <param name="attribute">The attribute containing the value to get.</param>
        /// <param name="messageHandler">A delegate that receives error messages.</param>
        /// <returns>The attribute's YesNoType value.</returns>
        internal static string GetInnerText(XElement node)
        {
            XText text = node.Nodes().Where(n => XmlNodeType.Text == n.NodeType || XmlNodeType.CDATA == n.NodeType).Cast<XText>().FirstOrDefault();
            return (null == text) ? null : text.Value;
        }

        public static void UnexpectedAttribute(SourceLineNumber sourceLineNumbers, XAttribute attribute, Action<MessageEventArgs> messageHandler)
        {
            // ignore elements defined by the W3C because we'll assume they are always right
            if (!((String.IsNullOrEmpty(attribute.Name.NamespaceName) && attribute.Name.LocalName.Equals("xmlns", StringComparison.Ordinal)) ||
                 attribute.Name.NamespaceName.StartsWith(Common.W3SchemaPrefix.NamespaceName, StringComparison.Ordinal)))
            {
                var mea = WixDataErrors.UnexpectedAttribute(sourceLineNumbers, attribute.Parent.Name.LocalName, attribute.Name.LocalName);
                if (null == messageHandler)
                {
                    throw new WixException(mea);
                }
                else
                {
                    messageHandler(mea);
                }
            }
        }

        /// <summary>
        /// Get the value of an attribute with type YesNoType.
        /// </summary>
        /// <param name="sourceLineNumbers">Source information for the value.</param>
        /// <param name="elementName">Name of the element for this attribute, used for a possible exception.</param>
        /// <param name="attributeName">Name of the attribute.</param>
        /// <param name="value">Value to process.</param>
        /// <returns>Returns true for a value of 'yes' and false for a value of 'no'.</returns>
        /// <exception cref="WixException">Thrown when the attribute's value is not 'yes' or 'no'.</exception>
        internal static bool IsYes(SourceLineNumber sourceLineNumbers, string elementName, string attributeName, string value)
        {
            switch (value)
            {
                case "no":
                    return false;
                case "yes":
                    return true;
                default:
                    throw new WixException(WixDataErrors.IllegalAttributeValue(sourceLineNumbers, elementName, attributeName, value, "no", "yes"));
            }
        }
    }
}
