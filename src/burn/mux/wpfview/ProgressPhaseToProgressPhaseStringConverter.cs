//-------------------------------------------------------------------------------------------------
// <copyright file="ProgressPhaseToProgressPhaseStringConverter.cs" company="Microsoft">
// Copyright (c) Microsoft Corporation. All rights reserved.
// </copyright>
// 
// <summary>
// Converter to change LaunchAction into a string to select user options
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.Bootstrapper
{
    using System;
    using System.Globalization;
    using System.Windows.Data;

    /// <summary>
    /// Class representing a converter of ProgressPhase enumeration to a progress phase string.
    /// </summary>
    public class ProgressPhaseToProgressPhaseStringConverter : IValueConverter
    {
        /// <summary>
        /// Converts ProgressPhase to a progress phase string.
        /// </summary>
        /// <param name="value">The value produced by the binding source.</param>
        /// <param name="targetType">The type of the binding target property.</param>
        /// <param name="parameter">The converter parameter to use.</param>
        /// <param name="culture">The culture to use in the converter.</param>
        /// <returns>A converted value.</returns>
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            // If progress phase hasn't started yet, don't display anything
            string progressPhase = String.Empty;
            string converted = String.Empty;

            switch ((ProgressPhase)value)
            {
                case ProgressPhase.Download:
                    progressPhase = UserInterfaceResources.DownloadingFeature;
                    break;

                case ProgressPhase.Install:
                    progressPhase = UserInterfaceResources.InstallingFeature;
                    break;

                case ProgressPhase.Uninstall:
                    progressPhase = UserInterfaceResources.UninstallingFeature;
                    break;

                case ProgressPhase.Repair:
                    progressPhase = UserInterfaceResources.RepairingFeature;
                    break;

                case ProgressPhase.Change:
                    progressPhase = UserInterfaceResources.ChangingFeature;
                    break;
            }

            if (!String.IsNullOrEmpty(progressPhase))
            {
                converted = String.Format(culture, UserInterfaceResources.FeatureProgress, progressPhase);
            }

            return converted;
        }

        /// <summary>
        /// Converts a progress phase string to a ProgressPhase enumerated type.
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
