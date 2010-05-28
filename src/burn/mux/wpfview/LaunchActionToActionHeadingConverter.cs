//-------------------------------------------------------------------------------------------------
// <copyright file="LaunchActionToActionHeadingConverter.cs" company="Microsoft">
// Copyright (c) Microsoft Corporation. All rights reserved.
//    
//    The use and distribution terms for this software are covered by the
//    Common Public License 1.0 (http://opensource.org/licenses/cpl.php)
//    which can be found in the file CPL.TXT at the root of this distribution.
//    By using this software in any fashion, you are agreeing to be bound by
//    the terms of this license.
//    
//    You must not remove this notice, or any other, from this software.
// </copyright>
// 
// <summary>
// Converter to change LaunchAction into a displayable action heading.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Bootstrapper
{
    using System;
    using System.Globalization;
    using System.Windows;
    using System.Windows.Data;

    /// <summary>
    /// Class representing a converter of LaunchAction enumeration to an action heading.
    /// </summary>
    public class LaunchActionToActionHeadingConverter : IValueConverter
    {
        /// <summary>
        /// Converts LaunchAction to an action heading string.
        /// </summary>
        /// <param name="value">The value produced by the binding source.</param>
        /// <param name="targetType">The type of the binding target property.</param>
        /// <param name="parameter">The converter parameter to use.</param>
        /// <param name="culture">The culture to use in the converter.</param>
        /// <returns>A converted value.</returns>
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            string converted;

            switch ((LaunchAction)value)
            {
                case LaunchAction.Install:
                    converted = UserInterfaceResources.InstallationHeading;
                    break;

                case LaunchAction.Uninstall:
                    converted = UserInterfaceResources.UninstallationHeading;
                    break;

                case LaunchAction.Repair:
                    converted = UserInterfaceResources.RepairHeading;
                    break;

                case LaunchAction.Modify:
                    converted = UserInterfaceResources.ChangeHeading;
                    break;

                default:
                    throw new InvalidOperationException("The source must be a supported LaunchAction");
            }

            return converted;
        }

        /// <summary>
        /// Converts an action heading string to a LaunchAction enumerated type.
        /// </summary>
        /// <param name="value">The value produced by the binding target.</param>
        /// <param name="targetType">The type to convert to.</param>
        /// <param name="parameter">The converter parameter to use.</param>
        /// <param name="culture">The culture to use in the converter.</param>
        /// <returns>A converted value.</returns>
        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            throw new NotSupportedException();
        }
    }
}
