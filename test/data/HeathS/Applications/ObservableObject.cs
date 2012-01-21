//-------------------------------------------------------------------------------------------------
// <copyright file="ObservableObject.cs" company="Microsoft">
//    Copyright (c) Microsoft Corporation.  All rights reserved.
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
// Abstract base class for bindable objects.
// </summary>
//-------------------------------------------------------------------------------------------------

using System.ComponentModel;
using System.Diagnostics;
using System.Reflection;

public abstract class ObservableObject : INotifyPropertyChanging, INotifyPropertyChanged
{
    public event PropertyChangingEventHandler PropertyChanging;
    public event PropertyChangedEventHandler PropertyChanged;

    protected virtual void OnPropertyChanging(string propertyName)
    {
        this.ValidateProperty(propertyName);

        PropertyChangingEventHandler handler = this.PropertyChanging;
        if (null != handler)
        {
            handler(this, new PropertyChangingEventArgs(propertyName));
        }
    }

    protected virtual void OnPropertyChanged(string propertyName)
    {
        this.ValidateProperty(propertyName);

        PropertyChangedEventHandler handler = this.PropertyChanged;
        if (null != handler)
        {
            handler(this, new PropertyChangedEventArgs(propertyName));
        }
    }

    [Conditional("DEBUG")]
    private void ValidateProperty(string propertyName)
    {
        PropertyInfo property = this.GetType().GetProperty(propertyName, BindingFlags.Instance | BindingFlags.Public);
        Debug.Assert(null != property);
    }
}
