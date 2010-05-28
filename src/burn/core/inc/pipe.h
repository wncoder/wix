//-------------------------------------------------------------------------------------------------
// <copyright file="pipe.h" company="Microsoft">
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
//    Burn Client Server pipe communication handler.
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef enum _BURN_PIPE_MESSAGE_TYPE
{
    BURN_PIPE_MESSAGE_TYPE_UNKNOWN,
    BURN_PIPE_MESSAGE_TYPE_GETPRIORVARIABLENUMERIC,
    BURN_PIPE_MESSAGE_TYPE_GETPRIORVARIABLESTRING,
    BURN_PIPE_MESSAGE_TYPE_GETPRIORVARIABLEVERSION,
    BURN_PIPE_MESSAGE_TYPE_GETPRIORVARIABLE_RESULT,
    BURN_PIPE_MESSAGE_TYPE_COMPLETE,
    BURN_PIPE_MESSAGE_TYPE_TERMINATE
} BURN_PIPE_MESSAGE_TYPE;


HRESULT PipeRunBurnServer(
    __in BURN_VARIABLES* pVariables
    );

#ifdef __cplusplus
}
#endif
