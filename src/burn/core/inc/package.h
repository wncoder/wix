//-------------------------------------------------------------------------------------------------
// <copyright file="package.h" company="Microsoft">
//    Copyright (c) Microsoft Corporation.  All rights reserved.
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

enum BURN_EXE_EXIT_CODE_TYPE
{
    BURN_EXE_EXIT_CODE_TYPE_NONE,
    BURN_EXE_EXIT_CODE_TYPE_SUCCESS,
    BURN_EXE_EXIT_CODE_TYPE_ERROR,
    BURN_EXE_EXIT_CODE_TYPE_SCHEDULE_REBOOT,
    BURN_EXE_EXIT_CODE_TYPE_FORCE_REBOOT,
};

enum BURN_PACKAGE_TYPE
{
    BURN_PACKAGE_TYPE_NONE,
    BURN_PACKAGE_TYPE_EXE,
    BURN_PACKAGE_TYPE_MSI,
    BURN_PACKAGE_TYPE_MSP,
    BURN_PACKAGE_TYPE_MSU,
};

enum BURN_MSIFEATURE_STATE
{
    BURN_MSIFEATURE_STATE_UNKNOWN,
    BURN_MSIFEATURE_STATE_ABSENT,
    BURN_MSIFEATURE_STATE_ADVERTISED,
    BURN_MSIFEATURE_STATE_LOCAL,
    BURN_MSIFEATURE_STATE_SOURCE,
};

enum BURN_MSIFEATURE_ACTION
{
    BURN_MSIFEATURE_ACTION_NONE,
    BURN_MSIFEATURE_ACTION_ADDLOCAL,
    BURN_MSIFEATURE_ACTION_ADDSOURCE,
    BURN_MSIFEATURE_ACTION_ADDDEFAULT,
    BURN_MSIFEATURE_ACTION_REINSTALL,
    BURN_MSIFEATURE_ACTION_ADVERTISE,
    BURN_MSIFEATURE_ACTION_REMOVE,
};


// structs

typedef struct _BURN_EXE_EXIT_CODE
{
    BURN_EXE_EXIT_CODE_TYPE type;
    DWORD dwCode;
    BOOL fWildcard;
} BURN_EXE_EXIT_CODE;

typedef struct _BURN_MSIPROPERTY
{
    LPWSTR sczId;
    LPWSTR sczValue; // used during forward execution
    LPWSTR sczRollbackValue;  // used during rollback
} BURN_MSIPROPERTY;

typedef struct _BURN_MSIFEATURE
{
    LPWSTR sczId;
    LPWSTR sczAddLocalCondition;
    LPWSTR sczAddSourceCondition;
    LPWSTR sczAdvertiseCondition;
    LPWSTR sczRollbackAddLocalCondition;
    LPWSTR sczRollbackAddSourceCondition;
    LPWSTR sczRollbackAdvertiseCondition;
    BURN_MSIFEATURE_STATE currentState;
    //BURN_MSIFEATURE_STATE requestedState; // used during forward execution
    //BURN_MSIFEATURE_STATE expectedState;  // used during rollback
    BURN_MSIFEATURE_ACTION executeAction;
    BURN_MSIFEATURE_ACTION rollbackAction;
    BOOL fRepair;
} BURN_MSIFEATURE;

typedef struct _BURN_PACKAGE
{
    LPWSTR sczId;
    //DWORD64 qwVersion;
    //BURN_LOGGING_LEVEL logLevel;
    //LPWSTR sczLogFile;
    LPWSTR sczInstallCondition;
    LPWSTR sczRollbackInstallCondition;
    BOOL fPerMachine;
    BOOL fTransactionBoundary;
    BOOL fUninstallable;
    BOOL fVital;

    PACKAGE_STATE currentState;
    //REQUEST_STATE requested;
    ACTION_STATE executeAction;
    ACTION_STATE rollbackAction;
    LPWSTR sczExecutePath;

    BURN_PACKAGE_TYPE Type;
    union
    {
        struct
        {
            LPWSTR sczDetectCondition;
            LPWSTR sczInstallArguments;
            LPWSTR sczRepairArguments;
            LPWSTR sczUninstallArguments;
            //LPWSTR sczProgressSwitch;
            BOOL fAllowRepair;

            BURN_EXE_EXIT_CODE* rgExitCodes;
            DWORD cExitCodes;
        } Exe;
        struct
        {
            LPWSTR sczProductCode;
            DWORD64 qwVersion;
            DWORD64 qwInstalledVersion;

            BURN_MSIPROPERTY* rgProperties;
            DWORD cProperties;

            BURN_MSIFEATURE* rgFeatures;
            DWORD cFeatures;
        } Msi;
        struct
        {
        } Msp;
        struct
        {
        } Msu;
    };
} BURN_PACKAGE;

typedef struct _BURN_PACKAGES
{
    BURN_PACKAGE* rgPackages;
    DWORD cPackages;
} BURN_PACKAGES;


// function declarations

HRESULT PackagesParseFromXml(
    __in BURN_PACKAGES* pPackages,
    __in IXMLDOMNode* pixnBundle
    );
void PackagesUninitialize(
    __in BURN_PACKAGES* pPackages
    );


#if defined(__cplusplus)
}
#endif
