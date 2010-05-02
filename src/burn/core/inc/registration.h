//-------------------------------------------------------------------------------------------------
// <copyright file="registration.h" company="Microsoft">
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
//    Module: Core
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once


#if defined(__cplusplus)
extern "C" {
#endif


// constants

enum BURN_RESUME_MODE
{
    BURN_RESUME_MODE_NONE,
    BURN_RESUME_MODE_ACTIVE,
    BURN_RESUME_MODE_SUSPEND,
    BURN_RESUME_MODE_ARP,
    BURN_RESUME_MODE_REBOOT_PENDING,
};


// structs

typedef struct _BURN_REGISTRATION
{
    BOOL fPerMachine;
    BOOL fRegisterArp;
    LPWSTR sczId;
    LPWSTR sczExecutableName;

    // paths
    HKEY hkRoot;
    LPWSTR sczRegistrationKey;
    LPWSTR sczCacheDirectory;
    LPWSTR sczCacheExecutablePath;
    LPWSTR sczStateFile;

    // ARP registration
    LPWSTR sczDisplayName;
    LPWSTR sczDisplayVersion;
    LPWSTR sczPublisher;
    LPWSTR sczHelpLink;
    LPWSTR sczHelpTelephone;
    LPWSTR sczAboutUrl;
    LPWSTR sczUpdateUrl;
    LPWSTR sczComments;
    //LPWSTR sczReadme; // TODO: this would be a file path
    LPWSTR sczContact;
    //DWORD64 qwEstimatedSize; // TODO: size should come from disk cost calculation
    BOOL fNoModifyDefined;
    BOOL fNoModify;
    BOOL fNoRepairDefined;
    BOOL fNoRepair;
    BOOL fNoRemoveDefined;
    BOOL fNoRemove;
} BURN_REGISTRATION;


// functions

HRESULT RegistrationParseFromXml(
    __in BURN_REGISTRATION* pRegistration,
    __in IXMLDOMNode* pixnBundle
    );
HRESULT RegistrationSetPaths(
    __in BURN_REGISTRATION* pRegistration
    );
HRESULT RegistrationDetectResumeType(
    __in BURN_REGISTRATION* pRegistration,
    __out BURN_RESUME_TYPE* pResumeType
    );
HRESULT RegistrationSessionBegin(
    __in BURN_REGISTRATION* pRegistration,
    __in BURN_USER_EXPERIENCE* pUserExperience,
    __in BURN_ACTION action,
    __in DWORD64 qwEstimatedSize,
    __in BOOL fPerMachineProcess
    );
HRESULT RegistrationSessionSuspend(
    __in BURN_REGISTRATION* pRegistration,
    __in BURN_ACTION action,
    __in BOOL fReboot,
    __in BOOL fPerMachineProcess
    );
HRESULT RegistrationSessionResume(
    __in BURN_REGISTRATION* pRegistration,
    __in BURN_ACTION action,
    __in BOOL fPerMachineProcess
    );
HRESULT RegistrationSessionEnd(
    __in BURN_REGISTRATION* pRegistration,
    __in BURN_ACTION action,
    __in BOOL fRollback,
    __in BOOL fPerMachineProcess
    );
HRESULT RegistrationSaveState(
    __in BURN_REGISTRATION* pRegistration,
    __in_bcount_opt(cbBuffer) BYTE* pbBuffer,
    __in_opt DWORD cbBuffer
    );
HRESULT RegistrationLoadState(
    __in BURN_REGISTRATION* pRegistration,
    __out_bcount(*pcbBuffer) BYTE** ppbBuffer,
    __out DWORD* pcbBuffer
    );
void RegistrationUninitialize(
    __in BURN_REGISTRATION* pRegistration
    );


#if defined(__cplusplus)
}
#endif
