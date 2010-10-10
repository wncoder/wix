//-------------------------------------------------------------------------------------------------
// <copyright file="InstallationStateToControlConverter.cs" company="Microsoft">
// Copyright (c) Microsoft Corporation. All rights reserved.
//    
//    The use and distribution terms for this software are covered by the
//    Common Public License 1.0 (http://opensource.org/licenses/cpl1.0.php)
//    which can be found in the file CPL.TXT at the root of this distribution.
//    By using this software in any fashion, you are agreeing to be bound by
//    the terms of this license.
//    
//    You must not remove this notice, or any other, from this software.
// </copyright>
//
// <summary>
// Converter from installation state to the appropriate control.
// </summary>
//-------------------------------------------------------------------------------------------------

namespace Microsoft.Tools.WindowsInstallerXml.UX
{
    using System;
    using System.Windows.Data;

    /// <summary>
    /// Converter from installation state to the appropriate control.
    /// </summary>
    public class InstallationStateToControlConverter : IValueConverter
    {
        private static InstallationDetectedControl CachedDetectedControl;
        private static InstallationApplyingControl CachedApplyingControl;
        private static InstallationCompleteControl CachedCompleteControl;

        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            switch ((InstallationState)value)
            {
                case InstallationState.Initializing:
                case InstallationState.DetectedAbsent:
                case InstallationState.DetectedNewer:
                case InstallationState.DetectedPresent:
                    if (InstallationStateToControlConverter.CachedDetectedControl == null)
                    {
                        InstallationStateToControlConverter.CachedDetectedControl = new InstallationDetectedControl();
                    }
                    return InstallationStateToControlConverter.CachedDetectedControl;

                case InstallationState.Applying:
                    if (InstallationStateToControlConverter.CachedApplyingControl == null)
                    {
                        InstallationStateToControlConverter.CachedApplyingControl = new InstallationApplyingControl();
                    }
                    return InstallationStateToControlConverter.CachedApplyingControl;

                case InstallationState.Applied:
                case InstallationState.Failed:
                    if (InstallationStateToControlConverter.CachedCompleteControl == null)
                    {
                        InstallationStateToControlConverter.CachedCompleteControl = new InstallationCompleteControl();
                    }
                    return InstallationStateToControlConverter.CachedCompleteControl;

                default:
                    throw new InvalidCastException();
            }
        }

        public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }
}
