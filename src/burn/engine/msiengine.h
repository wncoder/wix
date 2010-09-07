//-------------------------------------------------------------------------------------------------
// <copyright file="msiengine.h" company="Microsoft">
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
//    Module: MSI Engine
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once


#if defined(__cplusplus)
extern "C" {
#endif


// constants

enum BURN_MSI_EXECUTE_MESSAGE_TYPE
{
    BURN_MSI_EXECUTE_MESSAGE_NONE,
    BURN_MSI_EXECUTE_MESSAGE_PROGRESS,
    BURN_MSI_EXECUTE_MESSAGE_ERROR,
    BURN_MSI_EXECUTE_MESSAGE_MSI_MESSAGE,
    BURN_MSI_EXECUTE_MESSAGE_MSI_FILES_IN_USE,
};


// structures

typedef struct _BURN_MSI_EXECUTE_MESSAGE
{
    BURN_MSI_EXECUTE_MESSAGE_TYPE type;
    union
    {
        struct
        {
            DWORD dwPercentage;
        } progress;
        struct
        {
            DWORD dwErrorCode;
            UINT uiFlags;
            LPCWSTR wzMessage;
        } error;
        struct
        {
            INSTALLMESSAGE mt;
            UINT uiFlags;
            LPCWSTR wzMessage;
        } msiMessage;
        struct
        {
            DWORD cFiles;
            LPCWSTR* rgwzFiles;
        } msiFilesInUse;
    };
} BURN_MSI_EXECUTE_MESSAGE;


// typedefs

typedef int (*PFN_MSIEXECUTEMESSAGEHANDLER)(
    __in LPVOID pvContext,
    __in BURN_MSI_EXECUTE_MESSAGE* pMessage
    );


// function declarations

HRESULT MsiEngineParsePackageFromXml(
    __in IXMLDOMNode* pixnBundle,
    __in BURN_PACKAGE* pPackage
    );
void MsiEnginePackageUninitialize(
    __in BURN_PACKAGE* pPackage
    );
HRESULT MsiEngineDetectPackage(
    __in BURN_PACKAGE* pPackage,
    __in BURN_USER_EXPERIENCE* pUserExperience
    );
HRESULT MsiEnginePlanPackage(
    __in DWORD dwPackageSequence,
    __in BURN_PACKAGE* pPackage,
    __in BURN_PLAN* pPlan,
    __in BURN_LOGGING* pLog,
    __in BURN_VARIABLES* pVariables,
    __in_opt HANDLE hCacheEvent,
    __in BURN_USER_EXPERIENCE* pUserExperience,
    __out BOOTSTRAPPER_ACTION_STATE* pExecuteAction,
    __out BOOTSTRAPPER_ACTION_STATE* pRollbackAction
    );
HRESULT MsiEngineExecutePackage(
    __in BURN_EXECUTE_ACTION* pExecuteAction,
    __in BURN_VARIABLES* pVariables,
    __in BOOL fRollback,
    __in PFN_MSIEXECUTEMESSAGEHANDLER pfnMessageHandler,
    __in LPVOID pvContext,
    __out BOOTSTRAPPER_APPLY_RESTART* pRestart
    );


#if defined(__cplusplus)
}
#endif
