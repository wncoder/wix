#pragma once
//-------------------------------------------------------------------------------------------------
// <copyright file="mqqueueexec.h" company="Microsoft Corporation">
//   Copyright (c) 2004, Microsoft Corporation.
//   This software is released under Common Public License Version 1.0 (CPL).
//   The license and further copyright text can be found in the file LICENSE.TXT
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
// 
// <summary>
//    MSMQ functions for CustomActions
// </summary>
//-------------------------------------------------------------------------------------------------


// function declarations

HRESULT MqiInitialize();
void MqiUninitialize();
HRESULT MqiCreateMessageQueues(
    LPWSTR* ppwzData
    );
HRESULT MqiRollbackCreateMessageQueues(
    LPWSTR* ppwzData
    );
HRESULT MqiDeleteMessageQueues(
    LPWSTR* ppwzData
    );
HRESULT MqiRollbackDeleteMessageQueues(
    LPWSTR* ppwzData
    );
HRESULT MqiAddMessageQueuePermissions(
    LPWSTR* ppwzData
    );
HRESULT MqiRollbackAddMessageQueuePermissions(
    LPWSTR* ppwzData
    );
HRESULT MqiRemoveMessageQueuePermissions(
    LPWSTR* ppwzData
    );
HRESULT MqiRollbackRemoveMessageQueuePermissions(
    LPWSTR* ppwzData
    );
