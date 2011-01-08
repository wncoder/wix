//-------------------------------------------------------------------------------------------------
// <copyright file="logging.h" company="Microsoft">
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
//    Module: Core
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once


#if defined(__cplusplus)
extern "C" {
#endif


// constants

enum BURN_LOGGING_STATE
{
    BURN_LOGGING_STATE_CLOSED,
    BURN_LOGGING_STATE_OPEN,
    BURN_LOGGING_STATE_DISABLED,
};


// structs

typedef struct _BURN_LOGGING
{
    BURN_LOGGING_STATE state;
    LPWSTR sczPathVariable;

    BOOL fAppend;
    LPWSTR sczPath;
    LPWSTR sczPrefix;
    LPWSTR sczExtension;
} BURN_LOGGING;



// function declarations

HRESULT LoggingOpen(
    __in BURN_LOGGING* pLog,
    __in_z_opt LPCWSTR wzCommandLine,
    __in BURN_VARIABLES* pVariables
    );

HRESULT LoggingSetPackageVariable(
    __in DWORD dwPackageSequence,
    __in BURN_PACKAGE* pPackage,
    __in BOOL fRollback,
    __in BURN_LOGGING* pLog,
    __in BURN_VARIABLES* pVariables,
    __out_opt LPWSTR* psczLogPath
    );

LPCSTR LoggingBurnActionToString(
    __in BOOTSTRAPPER_ACTION action
    );

LPCSTR LoggingActionStateToString(
    __in BOOTSTRAPPER_ACTION_STATE actionState
    );

LPCSTR LoggingBoolToString(
    __in BOOL f
    );

LPCSTR LoggingTrueFalseToString(
    __in BOOL f
    );

LPCSTR LoggingPackageStateToString(
    __in BOOTSTRAPPER_PACKAGE_STATE packageState
    );

LPCSTR LoggingPerMachineToString(
    __in BOOL fPerMachine
    );

LPCSTR LoggingRelatedOperationToString(
    __in BOOTSTRAPPER_RELATED_OPERATION operation
    );

LPCSTR LoggingRequestStateToString(
    __in BOOTSTRAPPER_REQUEST_STATE requestState
    );

// Note: this function is not thread safe.
LPCSTR LoggingVersionToString(
    __in DWORD64 dw64Version
    );


#if defined(__cplusplus)
}
#endif
