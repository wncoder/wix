//-----------------------------------------------------------------------
// <copyright file="WixMessage.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
//    
//    The use and distribution terms for this software are covered by the
//    Common Public License 1.0 (http://opensource.org/licenses/cpl.php)
//    which can be found in the file CPL.TXT at the root of this distribution.
//    By using this software in any fashion, you are agreeing to be bound by
//    the terms of this license.
//    
//    You must not remove this notice, or any other, from this software.
// </copyright>
// <summary>
//     A class represents a WiX message. That is, a warning or an error.
// </summary>
//-----------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Test
{
    using System;
    using System.Collections.Generic;
    using System.Text;
    using System.Text.RegularExpressions;

    /// <summary>
    /// A WiX message. That is, a warning or an error.
    /// </summary>
    public class WixMessage
    {
        /// <summary>
        /// The message number
        /// </summary>
        private readonly int messageNumber;

        /// <summary>
        /// The message text
        /// </summary>
        private readonly string messageText;

        /// <summary>
        /// The message type
        /// </summary>
        private readonly MessageTypeEnum messageType;

        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="messageNumber">The message number</param>
        /// <param name="messageText">The message text</param>
        /// <param name="messageType">The message type</param>
        public WixMessage(int messageNumber, string messageText, MessageTypeEnum messageType)
        {
            this.messageNumber = messageNumber;
            this.messageText = messageText;
            this.messageType = messageType;
        }

        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="messageNumber">The message number</param>
        /// <param name="messageType">The message type</param>
        public WixMessage(int messageNumber, MessageTypeEnum messageType)
        {
            this.messageNumber = messageNumber;
            this.messageText = String.Empty;
            this.messageType = messageType;
        }

        /// <summary>
        /// The type of message
        /// </summary>
        public enum MessageTypeEnum
        {
            /// <summary>
            /// Error
            /// </summary>
            Error,

            /// <summary>
            /// Warning
            /// </summary>
            Warning
        }

        /// <summary>
        /// The message number
        /// </summary>
        public int MessageNumber
        {
            get { return this.messageNumber; }
        }

        /// <summary>
        /// The message text
        /// </summary>
        public string MessageText
        {
            get { return this.messageText; }
        }

        /// <summary>
        /// The message type
        /// </summary>
        public MessageTypeEnum MessageType
        {
            get { return this.messageType; }
        }

        /// <summary>
        /// Check if a line of text contains a WiX message
        /// </summary>
        /// <param name="text">The text to search</param>
        /// <returns>A WixMessage if one exists in the text. Otherwise, return null.</returns>
        public static WixMessage FindWixMessage(string text)
        {
            return WixMessage.FindWixMessage(text, WixTool.WixTools.Any);
        }

        /// <summary>
        /// Check if a line of text contains a WiX message
        /// </summary>
        /// <param name="text">The text to search</param>
        /// <param name="tool">Specifies which tool the message is expected to come from</param>
        /// <returns>A WixMessage if one exists in the text. Otherwise, return null.</returns>
        public static WixMessage FindWixMessage(string text, WixTool.WixTools tool)
        {
            Match messageMatch = WixMessage.GetToolWixMessageRegex(tool).Match(text);

            if (messageMatch.Success)
            {
                int messageNumber = Convert.ToInt32(messageMatch.Groups["messageNumber"].Value);
                string messageText = messageMatch.Groups["messageText"].Value;
                WixMessage.MessageTypeEnum messageType = WixMessage.ConvertToMessageTypeEnum(messageMatch.Groups["messageType"].Value);

                return new WixMessage(messageNumber, messageText, messageType);
            }
            else
            {
                return null;
            }
        }

        /// <summary>
        /// Returns a Regex that matches a Wix Message (warning or error) for a particular tool
        /// </summary>
        /// <param name="tool">A Wix Tool</param>
        /// <returns>A Regex that matches a Wix Message for the specified tool</returns>
        public static Regex GetToolWixMessageRegex(WixTool.WixTools tool)
        {
            string toolCode = String.Empty;

            switch (tool)
            {
                case WixTool.WixTools.Candle:
                    toolCode = "CNDL";
                    break;
                case WixTool.WixTools.Dark:
                    toolCode = "DARK";
                    break;
                case WixTool.WixTools.Light:
                    toolCode = "LGHT";
                    break;
                case WixTool.WixTools.Lit:
                    toolCode = "LIT";
                    break;
                case WixTool.WixTools.Melt:
                    toolCode = "MELT";
                    break;
                case WixTool.WixTools.Pyro:
                    toolCode = "PYRO";
                    break;
                case WixTool.WixTools.Smoke:
                    toolCode = "SMOK";
                    break;
                case WixTool.WixTools.Torch:
                    toolCode = "TRCH";
                    break;
                case WixTool.WixTools.Wixunit:
                    toolCode = "WUNT";
                    break;
                case WixTool.WixTools.Any:
                    // This string will match any toolCode
                    toolCode = @"[^:\d]*";
                    break;
                default:
                    throw new ArgumentException(String.Format("Unexpected argument {0}", tool.ToString()));
            }

            Regex wixMessageRegex = new Regex(String.Format(@"^.*: (?<messageType>error|warning) {0}(?<messageNumber>\d*) : (?<messageText>[^\n\r]*).*$", toolCode), RegexOptions.ExplicitCapture | RegexOptions.Singleline);
            return wixMessageRegex;
        }

        /// <summary>
        /// Determines equality between two WixMessage objects
        /// </summary>
        /// <param name="wm1">A WixMessage</param>
        /// <param name="wm2">A WixMessage</param>
        /// <returns>True if the WixMessages are equal and false if they are not equal</returns>
        public static bool operator ==(WixMessage wm1, WixMessage wm2)
        {
            if (0 == WixMessage.Compare(wm1, wm2))
            {
                return true;
            }
            else
            {
                return false;
            }
        }

        /// <summary>
        /// Determines inequality between two WixMessage objects
        /// </summary>
        /// <param name="wm1">A WixMessage</param>
        /// <param name="wm2">A WixMessage</param>
        /// <returns>True if the WixMessages are not equal and false if they are equal</returns>
        public static bool operator !=(WixMessage wm1, WixMessage wm2)
        {
            return !(wm1 == wm2);
        }

        /// <summary>
        /// Compares two specified WixMessage objects and returns an integer that indicates their relationship to one another in the sort order
        /// </summary>
        /// <param name="wm1">The first WixMessage</param>
        /// <param name="wm2">The second WixMessage</param>
        /// <returns>
        /// Less than zero if wm1 is less than wm2
        /// Zero if wm1 is equal to wm2
        /// Greater than zero if wm1 is greater than wm2
        /// </returns>
        public static int Compare(WixMessage wm1, WixMessage wm2)
        {
            return WixMessage.Compare(wm1, wm2, false);
        }

        /// <summary>
        /// Compares two specified WixMessage objects and returns an integer that indicates their relationship to one another in the sort order
        /// </summary>
        /// <param name="wm1">The first WixMessage</param>
        /// <param name="wm2">The second WixMessage</param>
        /// <param name="ignoreText">True if the message text should be ignored when comparing WixMessage objects</param>
        /// <returns>
        /// Less than zero if wm1 is less than wm2
        /// Zero if wm1 is equal to wm2
        /// Greater than zero if wm1 is greater than wm2
        /// </returns>
        public static int Compare(WixMessage wm1, WixMessage wm2, bool ignoreText)
        {
            // cast the WixMessages to Objects for null comparison, otherwise the == operator call would be recursive
            Object obj1 = (Object)wm1;
            Object obj2 = (Object)wm2;

            if (null == obj1 && null == obj2)
            {
                // both objects are null
                return 0;
            }
            else if (null == obj1 && null != obj2)
            {
                // obj1 is null and obj2 is not null
                return -1;
            }
            else if (null != obj1 && null == obj2)
            {
                // obj1 is not null and obj1 is null
                return 1;
            }

            // wm1 and wm2 are both not null, so compare their values

            if (wm1.MessageNumber < wm2.MessageNumber)
            {
                return -1;
            }
            else if (wm1.MessageNumber > wm2.MessageNumber)
            {
                return 1;
            }
            else
            {
                // MessageNumbers are equal so compare MessageTypes

                if (wm1.MessageType != wm2.MessageType)
                {
                    // MessageNumbers are equal but MessageTypes are not

                    if (wm1.MessageType == MessageTypeEnum.Warning)
                    {
                        // A warning is considered to be 'less' than an error

                        return -1;
                    }
                    else
                    {
                        return 1;
                    }
                }
                else
                {
                    // MessageNumbers and MessageTypes are equal so compare MessageText

                    if (ignoreText)
                    {
                        return 0;
                    }
                    else
                    {
                        return String.Compare(wm1.MessageText, wm2.MessageText, StringComparison.InvariantCulture);
                    }
                }
            }
        }

        /// <summary>
        /// Determines whether the specified WixMessage is equal to the current WixMessage
        /// </summary>
        /// <param name="obj">The WixMessage to compare</param>
        /// <returns>True if the message type, number and text are equal</returns>
        public override bool Equals(object obj)
        {
            if (null == obj || this.GetType() != obj.GetType())
            {
                return false;
            }
            else
            {
                return (this == (WixMessage)obj);
            }
        }

        /// <summary>
        /// Serves as a hash function for WixMessage
        /// </summary>
        /// <returns>A hash code for a WixMessage</returns>
        /// <remarks>
        /// This method should be overridden when the Equals method is overridden to ensure that equal objects return the same hash code
        /// </remarks>
        public override int GetHashCode()
        {
            int hashCode = this.MessageType.GetHashCode() ^ this.MessageNumber.GetHashCode() ^ this.MessageText.GetHashCode();
            return hashCode;
        }

        /// <summary>
        /// String representation of a WixMessage
        /// </summary>
        /// <returns>A string</returns>
        public override string ToString()
        {
            string messageType = Enum.GetName(this.MessageType.GetType(), this.MessageType);
            return String.Format("{0} {1} {2}", messageType, this.MessageNumber, this.messageText);
        }

        /// <summary>
        /// Converts a string message type to an enum message type
        /// </summary>
        /// <param name="messageType">A string message type, eg "warning" or "error"</param>
        /// <returns>An enum message type, eg. MessageTypeEnum.Warning</returns>
        private static MessageTypeEnum ConvertToMessageTypeEnum(string messageType)
        {
            if (messageType.Equals("error", StringComparison.InvariantCultureIgnoreCase))
            {
                return MessageTypeEnum.Error;
            }
            else if (messageType.Equals("warning", StringComparison.InvariantCultureIgnoreCase))
            {
                return MessageTypeEnum.Warning;
            }
            else
            {
                throw new ArgumentException(String.Format("The message type '{0}' is not valid"));
            }
        }
    }
}
