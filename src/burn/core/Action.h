//-------------------------------------------------------------------------------------------------
// <copyright file="Action.h" company="Microsoft">
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

#include "interfaces\IProgressObserver.h"
#include "ux\ux.h"

namespace IronMan
{
class Action
{	
private:
    IProgressObserver::State m_action;

protected:
    virtual ~Action()
    {}
    Action(IProgressObserver::State action)
        : m_action(action)		
    {}

    IProgressObserver::State GetAction() const 
    {
        return m_action; 
    }

    //Do the appropriate translation from Oberver's State Enum to Ux's actionEnum
    const UxEnum::actionEnum GetUxAction(const IProgressObserver::State enumVal) const
    {
        if (IProgressObserver::Installing ==  enumVal)
        {
            return UxEnum::aInstall;
        }
        if (IProgressObserver::Uninstalling ==  enumVal)
        {
            return UxEnum::aUninstall;
        } 
        if (IProgressObserver::Repairing ==  enumVal)
        {
            return UxEnum::aRepair;
        }
        
        if (IProgressObserver::Rollback ==  enumVal) 
        {
            return UxEnum::aRollback;
        }

        IMASSERT2(0, L"State to actionEnum not mapped");
        return UxEnum::aNone;
    }
};

}
