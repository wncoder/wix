//-------------------------------------------------------------------------------------------------
// <copyright file="NotifyControllerPImpls.h" company="Microsoft">
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

#include "..\interfaces\IProgressObserver.h"
#include "CompositePerformerPImpls.h"

// ids
#define NOTIFYCONTROLLERPIMPL 3

namespace NotifyControllerPImpls
{

//------------------------------------------------------------------------------
// Class: NotifyControllerPImpl
//
// This class defines the interface used by the external client 
// and the wrapper of internal implementation
//------------------------------------------------------------------------------
    class NotifyControllerPImpl
    {
    public:
        virtual ~NotifyControllerPImpl(){}
        virtual void Abort() = 0;
        virtual void MayBegin(IronMan::IProgressObserver& observer) = 0;
    };
}
