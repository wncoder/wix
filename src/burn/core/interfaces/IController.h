//-------------------------------------------------------------------------------------------------
// <copyright file="IController.h" company="Microsoft">
//    Copyright (c) Microsoft Corporation.  All rights reserved.
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
//
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once

#include "IProgressObserver.h"

namespace IronMan
{

class INotifyController
{
public:
    virtual bool MayBegin(IProgressObserver&) = 0;
    virtual void Abort() = 0;
    virtual void Stop() = 0;
};

class NullNotifyController : private INotifyController
{
    virtual bool MayBegin(IProgressObserver&) { return false; }
    virtual void Abort() {}
    virtual void Stop() {}
public:
    static INotifyController& GetNullNotifyController()
    {
        static NullNotifyController nnc;
        return nnc;
    }
};

}
