//-------------------------------------------------------------------------------------------------
// <copyright file="IProvideDataToEngine.h" company="Microsoft">
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

#include "common\operation.h"

//This is used to provide data to Engine from UI.
namespace IronMan
{
struct IProvideDataToEngine
{
    enum UiStateType
    {
        uiNone				= 0,
        uiInitialization	= 1,
        uiWelcome			= 2,
        uiEula				= 3,
        uiPreReq			= 4,
        uiDownloading		= 5,
        uiInstalling		= 6,
        uiFinish			= 7,
        uiCrash				= 8
    };
        
    virtual DWORD RefreshCount() = 0;	
    virtual UiStateType CurrentState() = 0;
    virtual Operation::euiOperation GetMaintenanceModeOperation() = 0;
};
}
