//-------------------------------------------------------------------------------------------------
// <copyright file="IMessageHandler.cs" company="Microsoft Corporation">
//   Copyright (c) 2004, Microsoft Corporation.
//   This software is released under Common Public License Version 1.0 (CPL).
//   The license and further copyright text can be found in the file LICENSE.TXT
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
// 
// <summary>
// Interface for handling messages (error/warning/verbose).
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml
{
    using System;

    /// <summary>
    /// Interface for handling messages (error/warning/verbose).
    /// </summary>
    public interface IMessageHandler
    {
        /// <summary>
        /// Sends a message with the given arguments.
        /// </summary>
        /// <param name="e">Message arguments.</param>
        void OnMessage(MessageEventArgs e);
    }
}