//-------------------------------------------------------------------------------------------------
// <copyright file="RelayCommand.cs" company="Microsoft">
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
// Simple ICommand implementation.
// </summary>
//-------------------------------------------------------------------------------------------------

using System;
using System.Windows.Input;

public sealed class RelayCommand : ICommand
{
    private readonly Action<object> execute;
    private readonly Predicate<object> canExecute;


    public RelayCommand(Action<object> execute)
        : this(execute, null)
    {
    }

    public RelayCommand(Action<object> execute, Predicate<object> canExecute)
    {
        if (null == execute)
        {
            throw new ArgumentNullException("execute");
        }

        this.execute = execute;
        this.canExecute = canExecute;
    }

    public bool CanExecute(object parameter)
    {
        return (null == this.canExecute) || this.canExecute(parameter);
    }

    public event EventHandler CanExecuteChanged
    {
        add { CommandManager.RequerySuggested += value; }
        remove { CommandManager.RequerySuggested -= value; }
    }

    public void Execute(object parameter)
    {
        this.execute(parameter);
    }
}
