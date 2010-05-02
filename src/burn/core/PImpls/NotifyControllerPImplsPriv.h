//-------------------------------------------------------------------------------------------------
// <copyright file="NotifyControllerPImplsPriv.h" company="Microsoft">
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

#include "NotifyControllerPImpls.h"
#include "NotifyController.h"

namespace NotifyControllerPImplsPrivate
{
//------------------------------------------------------------------------------
// Class: NotifyControllerPImpl
//
// This class wraps the actual implementation of NotifyController
//------------------------------------------------------------------------------
    class CNotifyControllerPImpl : public NotifyControllerPImpls::NotifyControllerPImpl
    {
        IronMan::NotifyController* m_pImpl;
    public:
        CNotifyControllerPImpl(IronMan::IPerformer& performer) //, IronMan::IFilesInUse& fileInUse
            : m_pImpl(new IronMan::NotifyController(performer))
        {}

        //static class factory that returns an instance of the NotifyController Pimpl
        static NotifyControllerPImpls::NotifyControllerPImpl* MakePImpl(IronMan::IPerformer& performer)
        {
            return new CNotifyControllerPImpl(performer);  //, fileInUse
        }

        //delegate Abort to the real NotifyController
        virtual void Abort()
        {
            IronMan::INotifyController* controller = m_pImpl;
            controller->Abort();
        }

        //delegate MayBegin to the real NotifyController
        virtual void MayBegin(IronMan::IProgressObserver& observer)
        {
            IronMan::INotifyController* controller = m_pImpl;
            controller->MayBegin(observer);
        }

        virtual ~CNotifyControllerPImpl()
        {
            delete m_pImpl;
        }
    };
}
