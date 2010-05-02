//-------------------------------------------------------------------------------------------------
// <copyright file="registration.cpp" company="Microsoft">
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

#include "precomp.h"


// constants

const LPCWSTR REGISTRY_UNINSTALL_KEY = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall";
const LPCWSTR REGISTRY_RUN_KEY = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run";


// internal function declarations

static HRESULT UpdateResumeMode(
    __in BURN_REGISTRATION* pRegistration,
    __in HKEY hkRegistration,
    __in BURN_RESUME_MODE resumeMode,
    __in BOOL fPerMachineProcess
    );
static HRESULT WriteFormattedString(
    __in HKEY hk,
    __in_z LPCWSTR wzName,
    __in __format_string LPCWSTR szFormat,
    ...
    );
static HRESULT WriteString(
    __in HKEY hk,
    __in_z LPCWSTR wzName,
    __in_z_opt LPWSTR wzValue
    );
static HRESULT WriteNumber(
    __in HKEY hk,
    __in_z LPCWSTR wzName,
    __in DWORD dwValue
    );
static HRESULT ReadNumber(
    __in HKEY hk,
    __in_z LPCWSTR wzName,
    __out DWORD* pdwValue
    );


// function definitions

/*******************************************************************
 RegistrationParseFromXml - Parses registration information from manifest.

*******************************************************************/
extern "C" HRESULT RegistrationParseFromXml(
    __in BURN_REGISTRATION* pRegistration,
    __in IXMLDOMNode* pixnBundle
    )
{
    HRESULT hr = S_OK;
    IXMLDOMNode* pixnRegistrationNode = NULL;
    IXMLDOMNode* pixnArpNode = NULL;
    LPWSTR scz = NULL;

    // select registration node
    hr = XmlSelectSingleNode(pixnBundle, L"Registration", &pixnRegistrationNode);
    if (S_FALSE == hr)
    {
        hr = E_NOTFOUND;
    }
    ExitOnFailure(hr, "Failed to select registration node.");

    // @Id
    hr = XmlGetAttributeEx(pixnRegistrationNode, L"Id", &pRegistration->sczId);
    ExitOnFailure(hr, "Failed to get @Id.");

    // @ExecutableName
    hr = XmlGetAttributeEx(pixnRegistrationNode, L"ExecutableName", &pRegistration->sczExecutableName);
    ExitOnFailure(hr, "Failed to get @ExecutableName.");

    // @PerMachine
    hr = XmlGetYesNoAttribute(pixnRegistrationNode, L"PerMachine", &pRegistration->fPerMachine);
    ExitOnFailure(hr, "Failed to get @PerMachine.");

    // select ARP node
    hr = XmlSelectSingleNode(pixnRegistrationNode, L"Arp", &pixnArpNode);
    if (S_FALSE != hr)
    {
        ExitOnFailure(hr, "Failed to select ARP node.");

        pRegistration->fRegisterArp = TRUE;

        // @DisplayName
        hr = XmlGetAttributeEx(pixnArpNode, L"DisplayName", &pRegistration->sczDisplayName);
        if (E_NOTFOUND != hr)
        {
            ExitOnFailure(hr, "Failed to get @DisplayName.");
        }

        // @DisplayVersion
        hr = XmlGetAttributeEx(pixnArpNode, L"DisplayVersion", &pRegistration->sczDisplayVersion);
        if (E_NOTFOUND != hr)
        {
            ExitOnFailure(hr, "Failed to get @DisplayVersion.");
        }

        // @Publisher
        hr = XmlGetAttributeEx(pixnArpNode, L"Publisher", &pRegistration->sczPublisher);
        if (E_NOTFOUND != hr)
        {
            ExitOnFailure(hr, "Failed to get @Publisher.");
        }

        // @HelpLink
        hr = XmlGetAttributeEx(pixnArpNode, L"HelpLink", &pRegistration->sczHelpLink);
        if (E_NOTFOUND != hr)
        {
            ExitOnFailure(hr, "Failed to get @HelpLink.");
        }

        // @HelpTelephone
        hr = XmlGetAttributeEx(pixnArpNode, L"HelpTelephone", &pRegistration->sczHelpTelephone);
        if (E_NOTFOUND != hr)
        {
            ExitOnFailure(hr, "Failed to get @HelpTelephone.");
        }

        // @AboutUrl
        hr = XmlGetAttributeEx(pixnArpNode, L"AboutUrl", &pRegistration->sczAboutUrl);
        if (E_NOTFOUND != hr)
        {
            ExitOnFailure(hr, "Failed to get @AboutUrl.");
        }

        // @UpdateUrl
        hr = XmlGetAttributeEx(pixnArpNode, L"UpdateUrl", &pRegistration->sczUpdateUrl);
        if (E_NOTFOUND != hr)
        {
            ExitOnFailure(hr, "Failed to get @UpdateUrl.");
        }

        // @Comments
        hr = XmlGetAttributeEx(pixnArpNode, L"Comments", &pRegistration->sczComments);
        if (E_NOTFOUND != hr)
        {
            ExitOnFailure(hr, "Failed to get @Comments.");
        }

        // @Contact
        hr = XmlGetAttributeEx(pixnArpNode, L"Contact", &pRegistration->sczContact);
        if (E_NOTFOUND != hr)
        {
            ExitOnFailure(hr, "Failed to get @Contact.");
        }

        // @NoModify
        hr = XmlGetYesNoAttribute(pixnArpNode, L"NoModify", &pRegistration->fNoModify);
        if (E_NOTFOUND != hr)
        {
            ExitOnFailure(hr, "Failed to get @NoModify.");
            pRegistration->fNoModifyDefined = TRUE;
        }

        // @NoRepair
        hr = XmlGetYesNoAttribute(pixnArpNode, L"NoRepair", &pRegistration->fNoRepair);
        if (E_NOTFOUND != hr)
        {
            ExitOnFailure(hr, "Failed to get @NoRepair.");
            pRegistration->fNoRepairDefined = TRUE;
        }

        // @NoRemove
        hr = XmlGetYesNoAttribute(pixnArpNode, L"NoModify", &pRegistration->fNoRemove);
        if (E_NOTFOUND != hr)
        {
            ExitOnFailure(hr, "Failed to get @NoRemove.");
            pRegistration->fNoRemoveDefined = TRUE;
        }
    }

    hr = S_OK;

LExit:
    ReleaseObject(pixnRegistrationNode);
    ReleaseObject(pixnArpNode);
    ReleaseStr(scz);

    return hr;
}

/*******************************************************************
 RegistrationSetPaths - Initializes file system paths to registration entities.

*******************************************************************/
extern "C" HRESULT RegistrationSetPaths(
    __in BURN_REGISTRATION* pRegistration
    )
{
    HRESULT hr = S_OK;

    // save registration key root
    pRegistration->hkRoot = pRegistration->fPerMachine ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER;

    // build uninstall registry key path
    hr = StrAllocFormatted(&pRegistration->sczRegistrationKey, L"%s\\%s", REGISTRY_UNINSTALL_KEY, pRegistration->sczId);
    ExitOnFailure(hr, "Failed to build uninstall registry key path.");

    // build cache directory
    hr = CacheGetCompletedPath(pRegistration->fPerMachine, pRegistration->sczId, &pRegistration->sczCacheDirectory);
    ExitOnFailure(hr, "Failed to build cache directory.");

    // build cached executable path
    hr = PathConcat(pRegistration->sczCacheDirectory, pRegistration->sczExecutableName, &pRegistration->sczCacheExecutablePath);
    ExitOnFailure(hr, "Failed to build cached executable path.");

    // build state file path
    hr = StrAllocFormatted(&pRegistration->sczStateFile, L"%s\\state.rsm", pRegistration->sczCacheDirectory);
    ExitOnFailure(hr, "Failed to build state file path.");

LExit:
    return hr;
}

/*******************************************************************
 RegistrationDetectResumeMode - Detects registration information onthe system
                                to determine if a resume is taking place.

*******************************************************************/
extern "C" HRESULT RegistrationDetectResumeType(
    __in BURN_REGISTRATION* pRegistration,
    __out BURN_RESUME_TYPE* pResumeType
    )
{
    HRESULT hr = S_OK;
    DWORD er = ERROR_SUCCESS;
    HKEY hkRegistration = NULL;
    HKEY hkRebootRequired = NULL;
    DWORD dwResume = 0;

    // open registration key
    er = vpfnRegOpenKeyExW(pRegistration->hkRoot, pRegistration->sczRegistrationKey, 0, KEY_QUERY_VALUE, &hkRegistration);
    if (ERROR_PATH_NOT_FOUND == er || ERROR_FILE_NOT_FOUND == er)
    {
        *pResumeType = BURN_RESUME_TYPE_NONE;
        ExitFunction1(hr = S_OK);
    }
    ExitOnWin32Error(er, hr, "Failed to open registration key.");

    // read Resume value
    hr = ReadNumber(hkRegistration, L"Resume", &dwResume);
    if (E_FILENOTFOUND == hr)
    {
        *pResumeType = BURN_RESUME_TYPE_NONE;
        ExitFunction1(hr = S_OK);
    }
    ExitOnFailure(hr, "Failed to read Resume value.");

    switch (dwResume)
    {
    case BURN_RESUME_MODE_ACTIVE:
        // a previous run was interrupted
        *pResumeType = BURN_RESUME_TYPE_UNEXPECTED;
        break;

    case BURN_RESUME_MODE_SUSPEND:
        *pResumeType = BURN_RESUME_TYPE_SUSPEND;
        break;

    case BURN_RESUME_MODE_ARP:
        *pResumeType = BURN_RESUME_TYPE_ARP;
        break;

    case BURN_RESUME_MODE_REBOOT_PENDING:
        // open RebootRequired key
        er = vpfnRegOpenKeyExW(hkRegistration, L"RebootRequired", 0, KEY_QUERY_VALUE, &hkRebootRequired);
        if (ERROR_FILE_NOT_FOUND == er)
        {
            // if key was not found, the system has been rebooted successfully
            *pResumeType = BURN_RESUME_TYPE_REBOOT;
        }
        else
        {
            ExitOnWin32Error(er, hr, "Failed to open RebootRequired key.");

            // if the key was opened successfully we are still pending a reboot
            *pResumeType = BURN_RESUME_TYPE_REBOOT_PENDING;
        }
        break;

    default:
        // the value stored in the registry is not valid
        *pResumeType = BURN_RESUME_TYPE_INVALID;
        break;
    }

LExit:
    ReleaseRegKey(hkRegistration);
    ReleaseRegKey(hkRebootRequired);

    return hr;
}

/*******************************************************************
 RegistrationSessionBegin - Registers a run session on the system.

*******************************************************************/
extern "C" HRESULT RegistrationSessionBegin(
    __in BURN_REGISTRATION* pRegistration,
    __in BURN_USER_EXPERIENCE* pUserExperience,
    __in BURN_ACTION action,
    __in DWORD64 qwEstimatedSize,
    __in BOOL fPerMachineProcess
    )
{
    HRESULT hr = S_OK;
    DWORD er = ERROR_SUCCESS;
    HKEY hkRegistration = NULL;
    HKEY hkRun = NULL;
    LPWSTR sczExecutablePath = NULL;
    LPWSTR sczExecutableDirectory = NULL;
    LPWSTR sczPayloadSourcePath = NULL;
    LPWSTR sczPayloadTargetPath = NULL;

    // alter registration in the correct process
    if (pRegistration->fPerMachine == fPerMachineProcess)
    {
        // on install, cache executable
        if (BURN_ACTION_INSTALL == action)
        {
            // build cached executable path
            hr = PathForCurrentProcess(&sczExecutablePath, NULL);
            ExitOnFailure(hr, "Failed to get path for current executing process.");

            // create cache directory
            hr = DirEnsureExists(pRegistration->sczCacheDirectory, NULL);
            ExitOnFailure(hr, "Failed to ensure bundle cache directory exists.");

            // TODO: replace this copy with the more intelligent copy of only
            // the burnstub executable and manifest data with fix-up for the
            // signature.
            if (!::CopyFileW(sczExecutablePath, pRegistration->sczCacheExecutablePath, FALSE))
            {
                ExitWithLastError2(hr, "Failed to cache burn from: '%S' to '%S'", sczExecutablePath, pRegistration->sczCacheExecutablePath);
            }

            // get base directory for executable
            hr = PathGetDirectory(sczExecutablePath, &sczExecutableDirectory);
            ExitOnFailure(hr, "Failed to get base directory for executable.");

            // copy external UX payloads
            for (DWORD i = 0; i < pUserExperience->payloads.cPayloads; ++i)
            {
                BURN_PAYLOAD* pPayload = &pUserExperience->payloads.rgPayloads[i];

                if (BURN_PAYLOAD_PACKAGING_EXTERNAL == pPayload->packaging)
                {
                    hr = PathConcat(sczExecutableDirectory, pPayload->sczSourcePath, &sczPayloadSourcePath);
                    ExitOnFailure(hr, "Failed to build payload source path.");

                    hr = PathConcat(pRegistration->sczCacheDirectory, pPayload->sczFilePath, &sczPayloadTargetPath);
                    ExitOnFailure(hr, "Failed to build payload target path.");

                    // copy payload file
                    hr = FileEnsureCopy(sczPayloadSourcePath, sczPayloadTargetPath, TRUE);
                    ExitOnFailure2(hr, "Failed to UX payload from: '%S' to '%S'", sczPayloadSourcePath, sczPayloadTargetPath);
                }
            }
        }

        // create registration key
        er = vpfnRegCreateKeyExW(pRegistration->hkRoot, pRegistration->sczRegistrationKey, 0, NULL, 0, KEY_WRITE, NULL, &hkRegistration, NULL);
        ExitOnWin32Error(er, hr, "Failed to create registration key.");

        // ARP registration
        if (pRegistration->fRegisterArp)
        {
            // on initial install, or repair, write any ARP values
            if (BURN_ACTION_INSTALL == action || BURN_ACTION_REPAIR == action)
            {
                // DisplayIcon: [path to exe],1
                hr = WriteFormattedString(hkRegistration, L"DisplayIcon", L"%s,1", pRegistration->sczCacheExecutablePath);
                ExitOnFailure(hr, "Failed to write DisplayIcon value.");

                // DisplayName: provided by UI
                hr = WriteString(hkRegistration, L"DisplayName", pRegistration->sczDisplayName);
                ExitOnFailure(hr, "Failed to write DisplayName value.");

                // DisplayVersion: provided by UI
                if (pRegistration->sczDisplayVersion)
                {
                    hr = WriteString(hkRegistration, L"DisplayVersion", pRegistration->sczDisplayVersion);
                    ExitOnFailure(hr, "Failed to write DisplayVersion value.");
                }

                // Publisher: provided by UI
                if (pRegistration->sczPublisher)
                {
                    hr = WriteString(hkRegistration, L"Publisher", pRegistration->sczPublisher);
                    ExitOnFailure(hr, "Failed to write Publisher value.");
                }

                // HelpLink: provided by UI
                if (pRegistration->sczHelpLink)
                {
                    hr = WriteString(hkRegistration, L"HelpLink", pRegistration->sczHelpLink);
                    ExitOnFailure(hr, "Failed to write HelpLink value.");
                }

                // HelpTelephone: provided by UI
                if (pRegistration->sczHelpTelephone)
                {
                    hr = WriteString(hkRegistration, L"HelpTelephone", pRegistration->sczHelpTelephone);
                    ExitOnFailure(hr, "Failed to write HelpTelephone value.");
                }

                // URLInfoAbout, provided by UI
                if (pRegistration->sczAboutUrl)
                {
                    hr = WriteString(hkRegistration, L"URLInfoAbout", pRegistration->sczAboutUrl);
                    ExitOnFailure(hr, "Failed to write URLInfoAbout value.");
                }

                // URLUpdateInfo, provided by UI
                if (pRegistration->sczUpdateUrl)
                {
                    hr = WriteString(hkRegistration, L"URLUpdateInfo", pRegistration->sczUpdateUrl);
                    ExitOnFailure(hr, "Failed to write URLUpdateInfo value.");
                }

                // Comments, provided by UI
                if (pRegistration->sczComments)
                {
                    hr = WriteString(hkRegistration, L"Comments", pRegistration->sczComments);
                    ExitOnFailure(hr, "Failed to write Comments value.");
                }

                // Contact, provided by UI
                if (pRegistration->sczContact)
                {
                    hr = WriteString(hkRegistration, L"Contact", pRegistration->sczContact);
                    ExitOnFailure(hr, "Failed to write Contact value.");
                }

                // InstallLocation: provided by UI
                // TODO: need to figure out what "InstallLocation" means in a chainer. <smile/>

                // NoModify
                if (pRegistration->fNoModifyDefined)
                {
                    hr = WriteNumber(hkRegistration, L"NoModify", (DWORD)pRegistration->fNoModify);
                    ExitOnFailure(hr, "Failed to set NoModify value.");
                }

                // If we support modify (aka: not no modify) then write the other supporting keys.
                if (!pRegistration->fNoModify)
                {
                    // ModifyPath: [path to exe] /modify
                    hr = WriteFormattedString(hkRegistration, L"ModifyPath", L"\"%s\" /modify", pRegistration->sczCacheExecutablePath);
                    ExitOnFailure(hr, "Failed to write ModifyPath value.");
                }

                // NoElevateOnModify: 1
                hr = WriteNumber(hkRegistration, L"NoElevateOnModify", 1);
                ExitOnFailure(hr, "Failed to set NoElevateOnModify value.");

                // NoRepair
                if (pRegistration->fNoRepairDefined)
                {
                    hr = WriteNumber(hkRegistration, L"NoRepair", (DWORD)pRegistration->fNoRepair);
                    ExitOnFailure(hr, "Failed to set NoRepair value.");
                }

                // NoRemove: should this be allowed?
                if (pRegistration->fNoRemoveDefined)
                {
                    hr = WriteNumber(hkRegistration, L"NoRemove", (DWORD)pRegistration->fNoRemove);
                    ExitOnFailure(hr, "Failed to set NoRemove value.");
                }

                // QuietUninstallString: [path to exe] /uninstall /quiet
                hr = WriteFormattedString(hkRegistration, L"QuietUninstallString", L"\"%s\" /uninstall /quiet", pRegistration->sczCacheExecutablePath);
                ExitOnFailure(hr, "Failed to write QuietUninstallString value.");

                // UninstallString, [path to exe] /uninstall
                hr = WriteFormattedString(hkRegistration, L"UninstallString", L"\"%s\" /uninstall", pRegistration->sczCacheExecutablePath);
                ExitOnFailure(hr, "Failed to write UninstallString value.");
            }

            // TODO: if we are not uninstalling, update estimated size
            //if (BURN_ACTION_UNINSTALL != action)
            //{
            //}
        }
    }

    // update resume mode
    hr = UpdateResumeMode(pRegistration, hkRegistration, BURN_RESUME_MODE_ACTIVE, fPerMachineProcess);
    ExitOnFailure(hr, "Failed to update resume mode.");

LExit:
    ReleaseRegKey(hkRegistration);
    ReleaseRegKey(hkRun);
    ReleaseStr(sczExecutablePath);
    ReleaseStr(sczExecutableDirectory);
    ReleaseStr(sczPayloadSourcePath);
    ReleaseStr(sczPayloadTargetPath);

    return hr;
}

/*******************************************************************
 RegistrationSessionSuspend - Suspends a run session and writes resume mode to the system.

*******************************************************************/
extern "C" HRESULT RegistrationSessionSuspend(
    __in BURN_REGISTRATION* pRegistration,
    __in BURN_ACTION action,
    __in BOOL fReboot,
    __in BOOL fPerMachineProcess
    )
{
    HRESULT hr = S_OK;
    DWORD er = ERROR_SUCCESS;
    HKEY hkRegistration = NULL;

    // alter registration in the correct process
    if (pRegistration->fPerMachine == fPerMachineProcess)
    {
        // open registration key
        er = vpfnRegOpenKeyExW(pRegistration->hkRoot, pRegistration->sczRegistrationKey, 0, KEY_WRITE, &hkRegistration);
        ExitOnWin32Error(er, hr, "Failed to open registration key.");
    }

    // update resume mode
    hr = UpdateResumeMode(pRegistration, hkRegistration, fReboot ? BURN_RESUME_MODE_REBOOT_PENDING : BURN_RESUME_MODE_SUSPEND, fPerMachineProcess);
    ExitOnFailure(hr, "Failed to update resume mode.");

LExit:
    ReleaseRegKey(hkRegistration);

    return hr;
}


/*******************************************************************
 RegistrationSessionResume - Resumes a previous run session.

*******************************************************************/
extern "C" HRESULT RegistrationSessionResume(
    __in BURN_REGISTRATION* pRegistration,
    __in BURN_ACTION action,
    __in BOOL fPerMachineProcess
    )
{
    HRESULT hr = S_OK;
    DWORD er = ERROR_SUCCESS;
    HKEY hkRegistration = NULL;

    // alter registration in the correct process
    if (pRegistration->fPerMachine == fPerMachineProcess)
    {
        // open registration key
        er = vpfnRegOpenKeyExW(pRegistration->hkRoot, pRegistration->sczRegistrationKey, 0, KEY_WRITE, &hkRegistration);
        ExitOnWin32Error(er, hr, "Failed to open registration key.");
    }

    // update resume mode
    hr = UpdateResumeMode(pRegistration, hkRegistration, BURN_RESUME_MODE_ACTIVE, fPerMachineProcess);
    ExitOnFailure(hr, "Failed to update resume mode.");

LExit:
    ReleaseRegKey(hkRegistration);

    return hr;
}


/*******************************************************************
 RegistrationSessionEnd - Unregisters a run session from the system.

 *******************************************************************/
extern "C" HRESULT RegistrationSessionEnd(
    __in BURN_REGISTRATION* pRegistration,
    __in BURN_ACTION action,
    __in BOOL fRollback,
    __in BOOL fPerMachineProcess
    )
{
    HRESULT hr = S_OK;
    DWORD er = ERROR_SUCCESS;
    BURN_RESUME_MODE resumeMode = BURN_RESUME_MODE_NONE;
    HKEY hkRegistration = NULL;

    // alter registration in the correct process
    if (pRegistration->fPerMachine == fPerMachineProcess)
    {
        // if we are ARP registered, and not uninstalling, then leave the cache behind and set resume mode to "ARP"
        if (pRegistration->fRegisterArp && ((BURN_ACTION_UNINSTALL != action && !fRollback) || (BURN_ACTION_UNINSTALL == action && fRollback)))
        {
            resumeMode = BURN_RESUME_MODE_ARP;

            // open registration key
            er = vpfnRegOpenKeyExW(pRegistration->hkRoot, pRegistration->sczRegistrationKey, 0, KEY_WRITE, &hkRegistration);
            ExitOnWin32Error(er, hr, "Failed to open registration key.");
        }
        else
        {
            // delete registration key
            er = vpfnRegDeleteKeyW(pRegistration->hkRoot, pRegistration->sczRegistrationKey);
            if (ERROR_FILE_NOT_FOUND != er)
            {
                ExitOnWin32Error(er, hr, "Failed to delete registration key.");
            }

            // delete cache directory
            hr = CacheDeleteDirectory(pRegistration->sczCacheDirectory);
            ExitOnFailure1(hr, "Failed to remove bundle directory: %S", pRegistration->sczCacheDirectory);
        }
    }

    // update resume mode
    hr = UpdateResumeMode(pRegistration, hkRegistration, resumeMode, fPerMachineProcess);
    ExitOnFailure(hr, "Failed to update resume mode.");

LExit:
    ReleaseRegKey(hkRegistration);

    return hr;
}

/*******************************************************************
 RegistrationSaveState - Saves an engine state BLOB for retreval after a resume.

*******************************************************************/
extern "C" HRESULT RegistrationSaveState(
    __in BURN_REGISTRATION* pRegistration,
    __in_bcount(cbBuffer) BYTE* pbBuffer,
    __in SIZE_T cbBuffer
    )
{
    HRESULT hr = S_OK;

    // create cache directory
    hr = DirEnsureExists(pRegistration->sczCacheDirectory, NULL);
    ExitOnFailure(hr, "Failed to ensure bundle cache directory exists.");

    // write data to file
    hr = FileWrite(pRegistration->sczStateFile, FILE_ATTRIBUTE_NORMAL, pbBuffer, cbBuffer, NULL);
    ExitOnFailure1(hr, "Failed to write state to file: %S", pRegistration->sczStateFile);

LExit:
    return hr;
}

/*******************************************************************
 RegistrationLoadState - Loads a previously stored engine state BLOB.

*******************************************************************/
extern "C" HRESULT RegistrationLoadState(
    __in BURN_REGISTRATION* pRegistration,
    __out_bcount(*pcbBuffer) BYTE** ppbBuffer,
    __out DWORD* pcbBuffer
    )
{
    HRESULT hr = S_OK;

    // write data to file
    hr = FileRead(ppbBuffer, pcbBuffer, pRegistration->sczStateFile);
    ExitOnFailure1(hr, "Failed to read state from file: %S", pRegistration->sczStateFile);

LExit:
    return hr;
}

/*******************************************************************
 RegistrationUninitialize - 

*******************************************************************/
extern "C" void RegistrationUninitialize(
    __in BURN_REGISTRATION* pRegistration
    )
{
    ReleaseStr(pRegistration->sczExecutableName);

    ReleaseStr(pRegistration->sczRegistrationKey);
    ReleaseStr(pRegistration->sczCacheDirectory);
    ReleaseStr(pRegistration->sczCacheExecutablePath);
    ReleaseStr(pRegistration->sczStateFile);

    ReleaseStr(pRegistration->sczDisplayName);
    ReleaseStr(pRegistration->sczDisplayVersion);
    ReleaseStr(pRegistration->sczPublisher);
    ReleaseStr(pRegistration->sczHelpLink);
    ReleaseStr(pRegistration->sczHelpTelephone);
    ReleaseStr(pRegistration->sczAboutUrl);
    ReleaseStr(pRegistration->sczUpdateUrl);
}


// internal helper functions

static HRESULT UpdateResumeMode(
    __in BURN_REGISTRATION* pRegistration,
    __in HKEY hkRegistration,
    __in BURN_RESUME_MODE resumeMode,
    __in BOOL fPerMachineProcess
    )
{
    HRESULT hr = S_OK;
    DWORD er = ERROR_SUCCESS;
    HKEY hkRebootRequired = NULL;
    HKEY hkRun = NULL;

    // write resume information
    if (hkRegistration)
    {
        if (BURN_RESUME_MODE_NONE == resumeMode)
        {
            // delete Resume value
            er = ::RegDeleteValueW(hkRegistration, L"Resume");
            ExitOnWin32Error(er, hr, "Failed to delete Resume value.");
        }
        else
        {
            // write Resume value
            hr = WriteNumber(hkRegistration, L"Resume", (DWORD)resumeMode);
            ExitOnFailure(hr, "Failed to write Resume value.");
        }

        // If we are entering reboot-pending mode, write a volatile
        // registry key to track when the reboot has taken place.
        if (BURN_RESUME_MODE_REBOOT_PENDING == resumeMode)
        {
            // create registry key
            er = vpfnRegCreateKeyExW(hkRegistration, L"RebootRequired", 0, NULL, REG_OPTION_VOLATILE, KEY_WRITE, NULL, &hkRebootRequired, NULL);
            ExitOnWin32Error(er, hr, "Failed to create uninstall key.");
        }
    }

    // update run key, this always happens in the per-user process
    if (!fPerMachineProcess)
    {
        if (BURN_RESUME_MODE_SUSPEND == resumeMode || BURN_RESUME_MODE_ARP == resumeMode || BURN_RESUME_MODE_NONE == resumeMode)
        {
            // delete run key value
            er = vpfnRegOpenKeyExW(HKEY_CURRENT_USER, REGISTRY_RUN_KEY, 0, KEY_WRITE, &hkRun);
            ExitOnWin32Error(er, hr, "Failed to open run key.");

            er = ::RegDeleteValueW(hkRun, pRegistration->sczId);
            ExitOnWin32Error(er, hr, "Failed to delete run key value.");
        }
        else
        {
            // write run key
            er = vpfnRegCreateKeyExW(HKEY_CURRENT_USER, REGISTRY_RUN_KEY, 0, NULL, 0, KEY_WRITE, NULL, &hkRun, NULL);
            ExitOnWin32Error(er, hr, "Failed to create run key.");

            hr = WriteString(hkRun, pRegistration->sczId, pRegistration->sczCacheExecutablePath);
            ExitOnFailure(hr, "Failed to write run key value.");
        }
    }

LExit:
    ReleaseRegKey(hkRebootRequired);
    ReleaseRegKey(hkRun);

    return hr;
}

static HRESULT WriteFormattedString(
    __in HKEY hk,
    __in_z LPCWSTR wzName,
    __in __format_string LPCWSTR szFormat,
    ...
    )
{
    HRESULT hr = S_OK;
    LPWSTR sczValue = NULL;
    va_list args;

    va_start(args, szFormat);
    hr = StrAllocFormattedArgs(&sczValue, szFormat, args);
    va_end(args);
    ExitOnFailure1(hr, "Failed to allocate %S value.", wzName);

    hr = WriteString(hk, wzName, sczValue);

LExit:
    ReleaseStr(sczValue);
    return hr;
}

static HRESULT WriteString(
    __in HKEY hk,
    __in_z LPCWSTR wzName,
    __in_z_opt LPWSTR wzValue
    )
{
    HRESULT hr = S_OK;
    DWORD er = ERROR_SUCCESS;
    DWORD cbValue = 0;

    if (wzValue && *wzValue)
    {
        cbValue = (lstrlenW(wzValue) + 1) * sizeof(WCHAR);

        er = ::RegSetValueExW(hk, wzName, 0, REG_SZ, reinterpret_cast<PBYTE>(wzValue), cbValue);
        ExitOnWin32Error1(er, hr, "Failed to set %S value.", wzName);
    }
    else
    {
        er = ::RegDeleteValueW(hk, wzName);
        if (ERROR_FILE_NOT_FOUND == er || ERROR_PATH_NOT_FOUND == er)
        {
            er = ERROR_SUCCESS;
        }
        ExitOnWin32Error1(er, hr, "Failed to delete %S value.", wzName);
    }

LExit:
    return hr;
}

static HRESULT WriteNumber(
    __in HKEY hk,
    __in_z LPCWSTR wzName,
    __in DWORD dwValue
    )
{
    HRESULT hr = S_OK;
    DWORD er = ERROR_SUCCESS;

    er = ::RegSetValueExW(hk, wzName, 0, REG_DWORD, reinterpret_cast<PBYTE>(&dwValue), sizeof(dwValue));
    ExitOnWin32Error1(er, hr, "Failed to set %S value.", wzName);

LExit:
    return hr;
}

static HRESULT ReadNumber(
    __in HKEY hk,
    __in_z LPCWSTR wzName,
    __out DWORD* pdwValue
    )
{
    HRESULT hr = S_OK;
    DWORD er = ERROR_SUCCESS;
    DWORD dwType = 0;
    DWORD cbData = sizeof(DWORD);

    er = ::RegQueryValueExW(hk, wzName, NULL, &dwType, (LPBYTE)pdwValue, &cbData);
    ExitOnWin32Error(er, hr, "Failed to query registry key value.");

    if (REG_DWORD != dwType)
    {
        hr = ::HRESULT_FROM_WIN32(ERROR_INVALID_DATATYPE);
        ExitOnFailure(hr, "Invalid registry value type.");
    }

LExit:
    return hr;
}
