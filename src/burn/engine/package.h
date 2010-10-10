//-------------------------------------------------------------------------------------------------
// <copyright file="package.h" company="Microsoft">
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

enum BURN_EXE_EXIT_CODE_TYPE
{
    BURN_EXE_EXIT_CODE_TYPE_NONE,
    BURN_EXE_EXIT_CODE_TYPE_SUCCESS,
    BURN_EXE_EXIT_CODE_TYPE_ERROR,
    BURN_EXE_EXIT_CODE_TYPE_SCHEDULE_REBOOT,
    BURN_EXE_EXIT_CODE_TYPE_FORCE_REBOOT,
};

enum BURN_EXE_PROTOCOL_TYPE
{
    BURN_EXE_PROTOCOL_TYPE_NONE,
    BURN_EXE_PROTOCOL_TYPE_BURN,
    BURN_EXE_PROTOCOL_TYPE_NETFX4,
};

enum BURN_PACKAGE_TYPE
{
    BURN_PACKAGE_TYPE_NONE,
    BURN_PACKAGE_TYPE_EXE,
    BURN_PACKAGE_TYPE_MSI,
    BURN_PACKAGE_TYPE_MSP,
    BURN_PACKAGE_TYPE_MSU,
};


// structs

typedef struct _BURN_EXE_EXIT_CODE
{
    BURN_EXE_EXIT_CODE_TYPE type;
    DWORD dwCode;
    BOOL fWildcard;
} BURN_EXE_EXIT_CODE;

typedef struct _BURN_MSPTARGETPRODUCT
{
    MSIINSTALLCONTEXT context;
    DWORD dwOrder;
    WCHAR wzTargetProductCode[39];

    BOOTSTRAPPER_PACKAGE_STATE patchPackageState;
} BURN_MSPTARGETPRODUCT;

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
    BOOTSTRAPPER_FEATURE_STATE currentState;
    BOOL fRepair;
} BURN_MSIFEATURE;

typedef struct _BURN_RELATED_MSI
{
    LPWSTR sczUpgradeCode;
    DWORD64 qwMinVersion;
    DWORD64 qwMaxVersion;
    BOOL fMinProvided;
    BOOL fMaxProvided;
    BOOL fMinInclusive;
    BOOL fMaxInclusive;
    BOOL fOnlyDetect;
    BOOL fLangInclusive;

    DWORD* rgdwLanguages;
    DWORD cLanguages;
} BURN_RELATED_MSI;

typedef struct _BURN_PACKAGE_PAYLOAD
{
    BURN_PAYLOAD* pPayload;
    BOOL fCached;
} BURN_PACKAGE_PAYLOAD;

typedef struct _BURN_ROLLBACK_BOUNDARY
{
    LPWSTR sczId;
    BOOL fVital;
} BURN_ROLLBACK_BOUNDARY;

typedef struct _BURN_PACKAGE
{
    LPWSTR sczId;

    LPWSTR sczLogPathVariable;          // name of the variable that will be set to the log path.
    LPWSTR sczRollbackLogPathVariable;  // name of the variable that will be set to the rollback path.

    LPWSTR sczInstallCondition;
    LPWSTR sczRollbackInstallCondition;
    BOOL fPerMachine;
    BOOL fUninstallable;
    BOOL fVital;

    BOOL fCache;
    LPWSTR sczCacheId;

    BURN_ROLLBACK_BOUNDARY* pRollbackBoundary;

    BOOTSTRAPPER_PACKAGE_STATE currentState;
    BOOL fCached;
    BOOTSTRAPPER_PACKAGE_STATE expected;
    BOOTSTRAPPER_REQUEST_STATE requested;

    BURN_PACKAGE_PAYLOAD* rgPayloads;
    DWORD cPayloads;

    BURN_PACKAGE_TYPE type;
    union
    {
        struct
        {
            LPWSTR sczDetectCondition;
            LPWSTR sczInstallArguments;
            LPWSTR sczRepairArguments;
            LPWSTR sczUninstallArguments;
            //LPWSTR sczProgressSwitch;
            BOOL fRepairable;
            BURN_EXE_PROTOCOL_TYPE protocol;

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

            BURN_RELATED_MSI* rgRelatedMsis;
            DWORD cRelatedMsis;
        } Msi;
        struct
        {
            LPWSTR sczPatchCode;
            LPWSTR sczApplicabilityXml;

            BURN_MSIPROPERTY* rgProperties;
            DWORD cProperties;

            BURN_MSPTARGETPRODUCT* rgTargetProducts;
            DWORD cTargetProductCodes;
        } Msp;
        struct
        {
            LPWSTR sczDetectCondition;
            LPWSTR sczKB;
        } Msu;
    };
} BURN_PACKAGE;

typedef struct _BURN_PACKAGES
{
    BURN_ROLLBACK_BOUNDARY* rgRollbackBoundaries;
    DWORD cRollbackBoundaries;

    BURN_PACKAGE* rgPackages;
    DWORD cPackages;

    MSIPATCHSEQUENCEINFOW* rgPatchInfo;
    BURN_PACKAGE** rgPatchInfoToPackage; // direct lookup from patch information to the (MSP) package it describes.
                                         // Thus this array is the exact same size as rgPatchInfo.
    DWORD cPatchInfo;
} BURN_PACKAGES;


// function declarations

HRESULT PackagesParseFromXml(
    __in BURN_PACKAGES* pPackages,
    __in BURN_PAYLOADS* pPayloads,
    __in IXMLDOMNode* pixnBundle
    );
void PackagesUninitialize(
    __in BURN_PACKAGES* pPackages
    );
HRESULT PackageFindById(
    __in BURN_PACKAGES* pPackages,
    __in_z LPCWSTR wzId,
    __out BURN_PACKAGE** ppPackage
    );


#if defined(__cplusplus)
}
#endif
