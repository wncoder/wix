//-------------------------------------------------------------------------------------------------
// <copyright file="test.cpp" company="Microsoft">
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
//    Code useful in testing custom actions.
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"


/******************************************************************
WixFailWhenDeferred - entry point for WixFailWhenDeferred
    custom action which always fails when running as a deferred
    custom action (otherwise it blindly succeeds). It's useful when
    testing the rollback of deferred custom actions: Schedule it
    immediately after the rollback/deferred CA pair you're testing
    and it will fail, causing your rollback CA to get invoked.
********************************************************************/
extern "C" UINT __stdcall WixFailWhenDeferred(
    __in MSIHANDLE hInstall
    )
{
    return ::MsiGetMode(hInstall, MSIRUNMODE_SCHEDULED) ? ERROR_INSTALL_FAILURE : ERROR_SUCCESS;
}
