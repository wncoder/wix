//-------------------------------------------------------------------------------------------------
// <copyright file="pipe.h" company="Microsoft">
//    Copyright (c) Microsoft Corporation.  All rights reserved.
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
