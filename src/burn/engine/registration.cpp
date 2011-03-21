//-------------------------------------------------------------------------------------------------
// <copyright file="registration.cpp" company="Microsoft">
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

#include "precomp.h"


// constants

const LPCWSTR REGISTRY_UNINSTALL_KEY = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall";
const LPCWSTR REGISTRY_RUN_KEY = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run";
const LPCWSTR REGISTRY_BUNDLE_CACHE_PATH = L"BundleCachePath";
const LPCWSTR REGISTRY_BUNDLE_UPGRADE_CODE = L"BundleUpgradeCode";
const LPCWSTR REGISTRY_BUNDLE_ADDON_CODE = L"BundleAddonCode";
const LPCWSTR REGISTRY_BUNDLE_DETECT_CODE = L"BundleDetectCode";
const LPCWSTR REGISTRY_BUNDLE_VERSION = L"BundleVersion";

// internal function declarations

static HRESULT UpdateResumeMode(
    __in BURN_REGISTRATION* pRegistration,
    __in HKEY hkRegistration,
    __in BURN_RESUME_MODE resumeMode,
    __in BOOL fPerMachineProcess
    );
static HRESULT ParseRelatedCodes(
    __in BURN_REGISTRATION* pRegistration,
    __in IXMLDOMNode* pixnBundle
    );
static HRESULT InitializeRelatedBundleFromKey(
    __in_z LPCWSTR wzBundleId,
    __in HKEY hkBundleId,
    __in BOOL fPerMachine,
    __inout BURN_RELATED_BUNDLE *pRelatedBundle
    );
static HRESULT FindMatchingStringBetweenArrays(
    __in_ecount(cValues) LPCWSTR *rgwzStringArray1,
    __in DWORD cStringArray1,
    __in_ecount(cValues) LPCWSTR *rgwzStringArray2,
    __in DWORD cStringArray2
    );
static HRESULT RegistrationDetectRelatedBundlesForKey(
    __in_opt BURN_USER_EXPERIENCE* pUX,
    __in BURN_REGISTRATION* pRegistration,
    __in_opt BOOTSTRAPPER_COMMAND* pCommand,
    __in HKEY hkRoot
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

    hr = ParseRelatedCodes(pRegistration, pixnBundle);
    ExitOnFailure(hr, "Failed to parse related bundles");

    // @Version
    hr = XmlGetAttributeEx(pixnRegistrationNode, L"Version", &scz);
    ExitOnFailure(hr, "Failed to get @Version.");

    hr = FileVersionFromStringEx(scz, 0, &pRegistration->qwVersion);
    ExitOnFailure1(hr, "Failed to parse @Version: %ls", scz);

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

        // @DisableModify
        hr = XmlGetYesNoAttribute(pixnArpNode, L"DisableModify", &pRegistration->fNoModify);
        if (E_NOTFOUND != hr)
        {
            ExitOnFailure(hr, "Failed to get @DisableModify.");
            pRegistration->fNoModifyDefined = TRUE;
        }

        // @DisableRepair
        hr = XmlGetYesNoAttribute(pixnArpNode, L"DisableRepair", &pRegistration->fNoRepair);
        if (E_NOTFOUND != hr)
        {
            ExitOnFailure(hr, "Failed to get @DisableRepair.");
            pRegistration->fNoRepairDefined = TRUE;
        }

        // @DisableRemove
        hr = XmlGetYesNoAttribute(pixnArpNode, L"DisableRemove", &pRegistration->fNoRemove);
        if (E_NOTFOUND != hr)
        {
            ExitOnFailure(hr, "Failed to get @DisableRemove.");
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
 RegistrationUninitialize - 

*******************************************************************/
extern "C" void RegistrationUninitialize(
    __in BURN_REGISTRATION* pRegistration
    )
{
    ReleaseStr(pRegistration->sczId);

    for (DWORD i = 0; i < pRegistration->cDetectCodes; ++i)
    {
        ReleaseStr(pRegistration->rgsczDetectCodes[i]);
    }
    ReleaseMem(pRegistration->rgsczDetectCodes);

    for (DWORD i = 0; i < pRegistration->cUpgradeCodes; ++i)
    {
        ReleaseStr(pRegistration->rgsczUpgradeCodes[i]);
    }
    ReleaseMem(pRegistration->rgsczUpgradeCodes);

    for (DWORD i = 0; i < pRegistration->cAddonCodes; ++i)
    {
        ReleaseStr(pRegistration->rgsczAddonCodes[i]);
    }
    ReleaseMem(pRegistration->rgsczAddonCodes);

    ReleaseStr(pRegistration->sczExecutableName);

    ReleaseStr(pRegistration->sczRegistrationKey);
    ReleaseStr(pRegistration->sczCacheDirectory);
    ReleaseStr(pRegistration->sczCacheExecutablePath);
    ReleaseStr(pRegistration->sczResumeCommandLine);
    ReleaseStr(pRegistration->sczStateFile);

    ReleaseStr(pRegistration->sczDisplayName);
    ReleaseStr(pRegistration->sczDisplayVersion);
    ReleaseStr(pRegistration->sczPublisher);
    ReleaseStr(pRegistration->sczHelpLink);
    ReleaseStr(pRegistration->sczHelpTelephone);
    ReleaseStr(pRegistration->sczAboutUrl);
    ReleaseStr(pRegistration->sczUpdateUrl);
    ReleaseStr(pRegistration->sczComments);
    ReleaseStr(pRegistration->sczContact);

    if (pRegistration->rgRelatedBundles)
    {
        for (DWORD i = 0; i < pRegistration->cRelatedBundles; ++i)
        {
            PackageUninitialize(&pRegistration->rgRelatedBundles[i].package);
        }

        MemFree(pRegistration->rgRelatedBundles);
    }

    // clear struct
    memset(pRegistration, 0, sizeof(BURN_REGISTRATION));
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
 RegistrationSetResumeCommand - Initializes resume command string

*******************************************************************/
extern "C" HRESULT RegistrationSetResumeCommand(
    __in BURN_REGISTRATION* pRegistration,
    __in BOOTSTRAPPER_COMMAND* pCommand,
    __in BURN_LOGGING* pLog
    )
{
    HRESULT hr = S_OK;

    // build the resume command-line.
    hr = StrAllocFormatted(&pRegistration->sczResumeCommandLine, L"\"%ls\"", pRegistration->sczCacheExecutablePath);
    ExitOnFailure(hr, "Failed to copy executable path to resume command-line.");

    if (pLog->sczPath)
    {
        hr = StrAllocConcat(&pRegistration->sczResumeCommandLine, L" /burn.log.append ", 0);
        ExitOnFailure(hr, "Failed to append burn.log.append commandline to resume command-line");

        hr = StrAllocConcat(&pRegistration->sczResumeCommandLine, pLog->sczPath, 0);
        ExitOnFailure(hr, "Failed to append logfile path to resume command-line");
    }

    switch (pCommand->action)
    {
    case BOOTSTRAPPER_ACTION_REPAIR:
        hr = StrAllocConcat(&pRegistration->sczResumeCommandLine, L" /repair", 0);
        break;
    case BOOTSTRAPPER_ACTION_UNINSTALL:
        hr = StrAllocConcat(&pRegistration->sczResumeCommandLine, L" /uninstall", 0);
        break;
    }
    ExitOnFailure(hr, "Failed to append action state to resume command-line");

    switch (pCommand->display)
    {
    case BOOTSTRAPPER_DISPLAY_NONE:
        hr = StrAllocConcat(&pRegistration->sczResumeCommandLine, L" /quiet", 0);
        break;
    case BOOTSTRAPPER_DISPLAY_PASSIVE:
        hr = StrAllocConcat(&pRegistration->sczResumeCommandLine, L" /passive", 0);
        break;
    }
    ExitOnFailure(hr, "Failed to append display state to resume command-line");

    switch (pCommand->restart)
    {
    case BOOTSTRAPPER_RESTART_ALWAYS:
        hr = StrAllocConcat(&pRegistration->sczResumeCommandLine, L" /forcerestart", 0);
        break;
    case BOOTSTRAPPER_RESTART_NEVER:
        hr = StrAllocConcat(&pRegistration->sczResumeCommandLine, L" /norestart", 0);
        break;
    }
    ExitOnFailure(hr, "Failed to append restart state to resume command-line");

    if (pCommand->wzCommandLine && *pCommand->wzCommandLine)
    {
        hr = StrAllocConcat(&pRegistration->sczResumeCommandLine, L" ", 0);
        ExitOnFailure(hr, "Failed to append space to resume command-line.");

        hr = StrAllocConcat(&pRegistration->sczResumeCommandLine, pCommand->wzCommandLine, 0);
        ExitOnFailure(hr, "Failed to append command-line to resume command-line.");
    }

LExit:
    return hr;
}

/*******************************************************************
 RegistrationDetectResumeMode - Detects registration information onthe system
                                to determine if a resume is taking place.

*******************************************************************/
extern "C" HRESULT RegistrationDetectResumeType(
    __in BURN_REGISTRATION* pRegistration,
    __out BOOTSTRAPPER_RESUME_TYPE* pResumeType
    )
{
    HRESULT hr = S_OK;
    HKEY hkRegistration = NULL;
    HKEY hkRebootRequired = NULL;
    DWORD dwResume = 0;

    // open registration key
    hr = RegOpen(pRegistration->hkRoot, pRegistration->sczRegistrationKey, KEY_QUERY_VALUE, &hkRegistration);
    if (E_FILENOTFOUND == hr || E_PATHNOTFOUND == hr)
    {
        *pResumeType = BOOTSTRAPPER_RESUME_TYPE_NONE;
        ExitFunction1(hr = S_OK);
    }
    ExitOnFailure(hr, "Failed to open registration key.");

    // read Resume value
    hr = RegReadNumber(hkRegistration, L"Resume", &dwResume);
    if (E_FILENOTFOUND == hr)
    {
        *pResumeType = BOOTSTRAPPER_RESUME_TYPE_NONE;
        ExitFunction1(hr = S_OK);
    }
    ExitOnFailure(hr, "Failed to read Resume value.");

    switch (dwResume)
    {
    case BURN_RESUME_MODE_ACTIVE:
        // a previous run was interrupted
        *pResumeType = BOOTSTRAPPER_RESUME_TYPE_UNEXPECTED;
        break;

    case BURN_RESUME_MODE_SUSPEND:
        *pResumeType = BOOTSTRAPPER_RESUME_TYPE_SUSPEND;
        break;

    case BURN_RESUME_MODE_ARP:
        *pResumeType = BOOTSTRAPPER_RESUME_TYPE_ARP;
        break;

    case BURN_RESUME_MODE_REBOOT_PENDING:
        // open RebootRequired key
        hr = RegOpen(hkRegistration, L"RebootRequired", KEY_QUERY_VALUE, &hkRebootRequired);
        if (E_FILENOTFOUND == hr)
        {
            // if key was not found, the system has been rebooted successfully
            *pResumeType = BOOTSTRAPPER_RESUME_TYPE_REBOOT;
            hr = S_OK;
        }
        else
        {
            ExitOnFailure(hr, "Failed to open RebootRequired key.");

            // if the key was opened successfully we are still pending a reboot
            *pResumeType = BOOTSTRAPPER_RESUME_TYPE_REBOOT_PENDING;
        }
        break;

    default:
        // the value stored in the registry is not valid
        *pResumeType = BOOTSTRAPPER_RESUME_TYPE_INVALID;
        break;
    }

LExit:
    ReleaseRegKey(hkRegistration);
    ReleaseRegKey(hkRebootRequired);

    return hr;
}

/*******************************************************************
 RegistrationDetectRelatedBundles - finds the bundles with same upgrade code.

*******************************************************************/
extern "C" HRESULT RegistrationDetectRelatedBundles(
    __in BURN_MODE mode,
    __in BURN_USER_EXPERIENCE* pUX,
    __in BURN_REGISTRATION* pRegistration,
    __in_opt BOOTSTRAPPER_COMMAND* pCommand
    )
{
    HRESULT hr = S_OK;

    if (BURN_MODE_NORMAL == mode)
    {
        hr = RegistrationDetectRelatedBundlesForKey(pUX, pRegistration, pCommand, HKEY_CURRENT_USER);
        ExitOnFailure(hr, "Failed to detect related bundles in HKCU for non-elevated process");

        hr = RegistrationDetectRelatedBundlesForKey(pUX, pRegistration, pCommand, HKEY_LOCAL_MACHINE);
        ExitOnFailure(hr, "Failed to detect related bundles in HKLM for non-elevated process");
    }
    else if (BURN_MODE_ELEVATED == mode || BURN_MODE_EMBEDDED == mode)
    {
        hr = RegistrationDetectRelatedBundlesForKey(pUX, pRegistration, pCommand, HKEY_LOCAL_MACHINE);
        ExitOnFailure(hr, "Failed to detect related bundles in HKLM for elevated process");
    }
    else
    {
        hr = E_UNEXPECTED;
        ExitOnFailure1(hr, "Detect related bundles called for unexpected burn mode: %d", mode);
    }

LExit:
    return hr;
}

extern "C" HRESULT RegistrationLoadRelatedBundle(
    __in BURN_REGISTRATION* pRegistration,
    __in_z LPCWSTR sczBundleId,
    __out BURN_RELATION_TYPE *pRelationType
    )
{
    HRESULT hr = S_OK;
    BOOL fRelated = FALSE;
    LPWSTR sczBundleKey = NULL;
    HKEY hkBundleId = NULL;
    LPWSTR *rgsczUpgradeCodes = NULL;
    DWORD cUpgradeCodes = 0;
    LPWSTR *rgsczAddonCodes = NULL;
    DWORD cAddonCodes = 0;
    LPWSTR *rgsczDetectCodes = NULL;
    DWORD cDetectCodes = 0;
    LPWSTR sczCachePath = NULL;
    DWORD64 dw64Version = 0;

    hr = StrAllocFormatted(&sczBundleKey, L"%ls\\%ls", REGISTRY_UNINSTALL_KEY, sczBundleId);
    ExitOnFailure(hr, "Failed to allocate path to bundle registry key.");

    hr = RegOpen(pRegistration->hkRoot, sczBundleKey, KEY_READ, &hkBundleId);
    ExitOnFailure1(hr, "Failed to open bundle registry key: %ls", sczBundleKey);

    hr = RegReadStringArray(hkBundleId, REGISTRY_BUNDLE_UPGRADE_CODE, &rgsczUpgradeCodes, &cUpgradeCodes);
    if (HRESULT_FROM_WIN32(ERROR_INVALID_DATATYPE) == hr)
    {
        TraceError(hr, "Failed to read upgrade code as REG_MULTI_SZ - trying again as REG_SZ in case of older products");

        rgsczUpgradeCodes = reinterpret_cast<LPWSTR *>(MemAlloc(sizeof(LPWSTR), TRUE));
        ExitOnNull(rgsczUpgradeCodes, hr, E_OUTOFMEMORY, "Failed to allocate list for a single upgrade code from older registry format");

        hr = RegReadString(hkBundleId, REGISTRY_BUNDLE_UPGRADE_CODE, &rgsczUpgradeCodes[0]);
        if (SUCCEEDED(hr))
        {
            cUpgradeCodes = 1;
        }
    }
    if (SUCCEEDED(hr))
    {
        // Both them and us must have the same upgrade code to cause an upgrade relation
        hr = FindMatchingStringBetweenArrays(const_cast<LPCWSTR *>(rgsczUpgradeCodes), cUpgradeCodes, const_cast<LPCWSTR *>(pRegistration->rgsczUpgradeCodes), pRegistration->cUpgradeCodes);
        if (HRESULT_FROM_WIN32(ERROR_NO_MATCH) == hr)
        {
            hr = S_OK;
        }
        else
        {
            ExitOnFailure(hr, "Failed to do array search for upgrade code match");

            fRelated = TRUE;
            *pRelationType = BURN_RELATION_UPGRADE;
            goto Finish;
        }
    }

    hr = RegReadStringArray(hkBundleId, REGISTRY_BUNDLE_ADDON_CODE, &rgsczAddonCodes, &cAddonCodes);
    if (SUCCEEDED(hr))
    {
        // Addon relation only occurs when their addoncode matches our detect code
        hr = FindMatchingStringBetweenArrays(const_cast<LPCWSTR *>(rgsczAddonCodes), cAddonCodes, const_cast<LPCWSTR *>(pRegistration->rgsczDetectCodes), pRegistration->cDetectCodes);
        if (HRESULT_FROM_WIN32(ERROR_NO_MATCH) == hr)
        {
            hr = S_OK;
        }
        else
        {
            ExitOnFailure(hr, "Failed to do array search for addon code match");

            fRelated = TRUE;
            *pRelationType = BURN_RELATION_ADDON;
            goto Finish;
        }
    }

    // Ignore failure
    RegReadStringArray(hkBundleId, REGISTRY_BUNDLE_DETECT_CODE, &rgsczDetectCodes, &cDetectCodes);

    // Detect relation occurs when one of our detect codes matches any of their codes.
    // Since we already matched their addon codes to our detect codes, we just have to check the other two
    // detection types: their detect codes and their upgrade codes
    hr = FindMatchingStringBetweenArrays(const_cast<LPCWSTR *>(rgsczDetectCodes), cDetectCodes, const_cast<LPCWSTR *>(pRegistration->rgsczDetectCodes), pRegistration->cDetectCodes);
    if (HRESULT_FROM_WIN32(ERROR_NO_MATCH) == hr)
    {
        hr = S_OK;
    }
    else
    {
        ExitOnFailure(hr, "Failed to do array search for detect code match");

        fRelated = TRUE;
        *pRelationType = BURN_RELATION_DETECT;
        goto Finish;
    }

    // Here we check against their upgrades, as described in the larger comment above
    hr = FindMatchingStringBetweenArrays(const_cast<LPCWSTR *>(rgsczUpgradeCodes), cUpgradeCodes, const_cast<LPCWSTR *>(pRegistration->rgsczDetectCodes), pRegistration->cDetectCodes);
    if (HRESULT_FROM_WIN32(ERROR_NO_MATCH) == hr)
    {
        hr = S_OK;
    }
    else
    {
        ExitOnFailure(hr, "Failed to do array search for detect code match");

        fRelated = TRUE;
        *pRelationType = BURN_RELATION_DETECT;
        goto Finish;
    }

Finish:
    if (fRelated)
    {
        hr = MemEnsureArraySize(reinterpret_cast<LPVOID*>(&pRegistration->rgRelatedBundles), pRegistration->cRelatedBundles + 1, sizeof(BURN_RELATED_BUNDLE), 5);
        ExitOnFailure(hr, "Failed to ensure there is space for related bundles.");

        BURN_RELATED_BUNDLE* pRelatedBundle = pRegistration->rgRelatedBundles + pRegistration->cRelatedBundles;
        ++pRegistration->cRelatedBundles;

        pRelatedBundle->package.fPerMachine = pRegistration->fPerMachine;
        hr = StrAllocString(&pRelatedBundle->package.sczId, sczBundleId, 0);
        ExitOnFailure(hr, "Failed to copy related bundle id.");

        hr = InitializeRelatedBundleFromKey(sczBundleId, hkBundleId, pRegistration->fPerMachine, pRelatedBundle);
        ExitOnFailure1(hr, "Failed to initialize package from bundle id: %ls", sczBundleId);
    }
    else
    {
        hr = E_NOTFOUND;
    }

LExit:
    ReleaseStr(sczCachePath);
    ReleaseStrArray(rgsczUpgradeCodes, cUpgradeCodes);
    ReleaseStrArray(rgsczAddonCodes, cAddonCodes);
    ReleaseStrArray(rgsczDetectCodes, cDetectCodes);
    ReleaseRegKey(hkBundleId);
    ReleaseStr(sczBundleKey);

    return hr;
}

/*******************************************************************
 RegistrationSessionBegin - Registers a run session on the system.

*******************************************************************/
extern "C" HRESULT RegistrationSessionBegin(
    __in BURN_REGISTRATION* pRegistration,
    __in BURN_USER_EXPERIENCE* pUserExperience,
    __in BOOTSTRAPPER_ACTION action,
    __in DWORD64 /* qwEstimatedSize */,
    __in BOOL fPerMachineProcess
    )
{
    HRESULT hr = S_OK;
    HKEY hkRegistration = NULL;
    LPWSTR sczExecutablePath = NULL;
    LPWSTR sczExecutableDirectory = NULL;
    LPWSTR sczPayloadSourcePath = NULL;
    LPWSTR sczPayloadTargetPath = NULL;

    // alter registration in the correct process
    if (pRegistration->fPerMachine == fPerMachineProcess)
    {
        // on install, cache executable
        if (BOOTSTRAPPER_ACTION_INSTALL == action)
        {
            // build cached executable path
            hr = PathForCurrentProcess(&sczExecutablePath, NULL);
            ExitOnFailure(hr, "Failed to get path for current executing process.");

            hr = LogStringLine(REPORT_STANDARD, "Caching executable from: %ls to: %ls", sczExecutablePath, pRegistration->sczCacheExecutablePath);
            ExitOnFailure(hr, "Failed to log 'caching executable' message");

            // create cache directory
            hr = DirEnsureExists(pRegistration->sczCacheDirectory, NULL);
            ExitOnFailure(hr, "Failed to ensure bundle cache directory exists.");

            // TODO: replace this copy with the more intelligent copy of only
            // the burnstub executable and manifest data with fix-up for the
            // signature.
            hr = FileEnsureCopy(sczExecutablePath, pRegistration->sczCacheExecutablePath, TRUE);
            ExitOnFailure2(hr, "Failed to cache burn from: '%ls' to '%ls'", sczExecutablePath, pRegistration->sczCacheExecutablePath);

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
                    ExitOnFailure2(hr, "Failed to copy UX payload from: '%ls' to '%ls'", sczPayloadSourcePath, sczPayloadTargetPath);
                }
            }
        }

        // create registration key
        hr = RegCreate(pRegistration->hkRoot, pRegistration->sczRegistrationKey, KEY_WRITE, &hkRegistration);
        ExitOnFailure(hr, "Failed to create registration key.");

        // ARP registration
        if (pRegistration->fRegisterArp)
        {
            // on initial install, or repair, write any ARP values
            if (BOOTSTRAPPER_ACTION_INSTALL == action || BOOTSTRAPPER_ACTION_REPAIR == action)
            {
                // Upgrade information
                hr = RegWriteString(hkRegistration, REGISTRY_BUNDLE_CACHE_PATH, pRegistration->sczCacheExecutablePath);
                ExitOnFailure(hr, "Failed to write BundleUpgradeCommand value.");

                hr = RegWriteStringArray(hkRegistration, REGISTRY_BUNDLE_UPGRADE_CODE, pRegistration->rgsczUpgradeCodes, pRegistration->cUpgradeCodes);
                ExitOnFailure(hr, "Failed to write BundleUpgradeCode value.");

                hr = RegWriteStringArray(hkRegistration, REGISTRY_BUNDLE_ADDON_CODE, pRegistration->rgsczAddonCodes, pRegistration->cAddonCodes);
                ExitOnFailure(hr, "Failed to write BundleAddonCode value.");

                hr = RegWriteStringArray(hkRegistration, REGISTRY_BUNDLE_DETECT_CODE, pRegistration->rgsczDetectCodes, pRegistration->cDetectCodes);
                ExitOnFailure(hr, "Failed to write BundleDetectCode value.");

                hr = RegWriteStringFormatted(hkRegistration, REGISTRY_BUNDLE_VERSION, L"%hu.%hu.%hu.%hu", (WORD)(pRegistration->qwVersion >> 48), (WORD)(pRegistration->qwVersion >> 32), (WORD)(pRegistration->qwVersion >> 16), (WORD)(pRegistration->qwVersion));
                ExitOnFailure(hr, "Failed to write BundleVersion value.");

                // DisplayIcon: [path to exe] and ",0" to refer to the first icon in the executable.
                hr = RegWriteStringFormatted(hkRegistration, L"DisplayIcon", L"%s,0", pRegistration->sczCacheExecutablePath);
                ExitOnFailure(hr, "Failed to write DisplayIcon value.");

                // DisplayName: provided by UI
                hr = RegWriteString(hkRegistration, L"DisplayName", pRegistration->sczDisplayName);
                ExitOnFailure(hr, "Failed to write DisplayName value.");

                // DisplayVersion: provided by UI
                if (pRegistration->sczDisplayVersion)
                {
                    hr = RegWriteString(hkRegistration, L"DisplayVersion", pRegistration->sczDisplayVersion);
                    ExitOnFailure(hr, "Failed to write DisplayVersion value.");
                }

                // Publisher: provided by UI
                if (pRegistration->sczPublisher)
                {
                    hr = RegWriteString(hkRegistration, L"Publisher", pRegistration->sczPublisher);
                    ExitOnFailure(hr, "Failed to write Publisher value.");
                }

                // HelpLink: provided by UI
                if (pRegistration->sczHelpLink)
                {
                    hr = RegWriteString(hkRegistration, L"HelpLink", pRegistration->sczHelpLink);
                    ExitOnFailure(hr, "Failed to write HelpLink value.");
                }

                // HelpTelephone: provided by UI
                if (pRegistration->sczHelpTelephone)
                {
                    hr = RegWriteString(hkRegistration, L"HelpTelephone", pRegistration->sczHelpTelephone);
                    ExitOnFailure(hr, "Failed to write HelpTelephone value.");
                }

                // URLInfoAbout, provided by UI
                if (pRegistration->sczAboutUrl)
                {
                    hr = RegWriteString(hkRegistration, L"URLInfoAbout", pRegistration->sczAboutUrl);
                    ExitOnFailure(hr, "Failed to write URLInfoAbout value.");
                }

                // URLUpdateInfo, provided by UI
                if (pRegistration->sczUpdateUrl)
                {
                    hr = RegWriteString(hkRegistration, L"URLUpdateInfo", pRegistration->sczUpdateUrl);
                    ExitOnFailure(hr, "Failed to write URLUpdateInfo value.");
                }

                // Comments, provided by UI
                if (pRegistration->sczComments)
                {
                    hr = RegWriteString(hkRegistration, L"Comments", pRegistration->sczComments);
                    ExitOnFailure(hr, "Failed to write Comments value.");
                }

                // Contact, provided by UI
                if (pRegistration->sczContact)
                {
                    hr = RegWriteString(hkRegistration, L"Contact", pRegistration->sczContact);
                    ExitOnFailure(hr, "Failed to write Contact value.");
                }

                // InstallLocation: provided by UI
                // TODO: need to figure out what "InstallLocation" means in a chainer. <smile/>

                // NoModify
                if (pRegistration->fNoModifyDefined)
                {
                    hr = RegWriteNumber(hkRegistration, L"NoModify", (DWORD)pRegistration->fNoModify);
                    ExitOnFailure(hr, "Failed to set NoModify value.");
                }

                // If we support modify (aka: not no modify) then write the other supporting keys.
                if (!pRegistration->fNoModify)
                {
                    // ModifyPath: [path to exe] /modify
                    hr = RegWriteStringFormatted(hkRegistration, L"ModifyPath", L"\"%s\" /modify", pRegistration->sczCacheExecutablePath);
                    ExitOnFailure(hr, "Failed to write ModifyPath value.");
                }

                // NoElevateOnModify: 1
                hr = RegWriteNumber(hkRegistration, L"NoElevateOnModify", 1);
                ExitOnFailure(hr, "Failed to set NoElevateOnModify value.");

                // NoRepair
                if (pRegistration->fNoRepairDefined)
                {
                    hr = RegWriteNumber(hkRegistration, L"NoRepair", (DWORD)pRegistration->fNoRepair);
                    ExitOnFailure(hr, "Failed to set NoRepair value.");
                }

                // NoRemove: should this be allowed?
                if (pRegistration->fNoRemoveDefined)
                {
                    hr = RegWriteNumber(hkRegistration, L"NoRemove", (DWORD)pRegistration->fNoRemove);
                    ExitOnFailure(hr, "Failed to set NoRemove value.");
                }

                // QuietUninstallString: [path to exe] /uninstall /quiet
                hr = RegWriteStringFormatted(hkRegistration, L"QuietUninstallString", L"\"%s\" /uninstall /quiet", pRegistration->sczCacheExecutablePath);
                ExitOnFailure(hr, "Failed to write QuietUninstallString value.");

                // UninstallString, [path to exe] /uninstall /passive
                hr = RegWriteStringFormatted(hkRegistration, L"UninstallString", L"\"%s\" /uninstall /passive", pRegistration->sczCacheExecutablePath);
                ExitOnFailure(hr, "Failed to write UninstallString value.");
            }

            // TODO: if we are not uninstalling, update estimated size
            //if (BOOTSTRAPPER_ACTION_UNINSTALL != action)
            //{
            //}
        }
    }

    // update resume mode
    hr = UpdateResumeMode(pRegistration, hkRegistration, BURN_RESUME_MODE_ACTIVE, fPerMachineProcess);
    ExitOnFailure(hr, "Failed to update resume mode.");

LExit:
    ReleaseRegKey(hkRegistration);
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
    __in BOOTSTRAPPER_ACTION /* action */,
    __in BOOL fReboot,
    __in BOOL fPerMachineProcess
    )
{
    HRESULT hr = S_OK;
    HKEY hkRegistration = NULL;

    // alter registration in the correct process
    if (pRegistration->fPerMachine == fPerMachineProcess)
    {
        // open registration key
        hr = RegOpen(pRegistration->hkRoot, pRegistration->sczRegistrationKey, KEY_WRITE, &hkRegistration);
        ExitOnFailure(hr, "Failed to open registration key.");
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
    __in BOOTSTRAPPER_ACTION /* action */,
    __in BOOL fPerMachineProcess
    )
{
    HRESULT hr = S_OK;
    HKEY hkRegistration = NULL;

    // alter registration in the correct process
    if (pRegistration->fPerMachine == fPerMachineProcess)
    {
        // open registration key
        hr = RegOpen(pRegistration->hkRoot, pRegistration->sczRegistrationKey, KEY_WRITE, &hkRegistration);
        ExitOnFailure(hr, "Failed to open registration key.");
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
    __in BOOTSTRAPPER_ACTION action,
    __in BOOL fRollback,
    __in BOOL fPerMachineProcess,
    __out_opt BURN_RESUME_MODE* pResumeMode
    )
{
    HRESULT hr = S_OK;
    BURN_RESUME_MODE resumeMode = BURN_RESUME_MODE_NONE;
    HKEY hkRegistration = NULL;
    LPWSTR sczRootCacheDirectory = NULL;

    // If we are ARP registered, and not uninstalling, then set resume mode to "ARP".
    if (pRegistration->fRegisterArp && !((BOOTSTRAPPER_ACTION_INSTALL == action && fRollback) || (BOOTSTRAPPER_ACTION_UNINSTALL == action && !fRollback)))
    {
        resumeMode = BURN_RESUME_MODE_ARP;
    }

    // Alter registration in the correct process.
    if (pRegistration->fPerMachine == fPerMachineProcess)
    {
        // If resume mode is "ARP", then leave the cache behind.
        if (pRegistration->fRegisterArp && !((BOOTSTRAPPER_ACTION_INSTALL == action && fRollback) || (BOOTSTRAPPER_ACTION_UNINSTALL == action && !fRollback)))
        {
            resumeMode = BURN_RESUME_MODE_ARP;

            // Open registration key.
            hr = RegOpen(pRegistration->hkRoot, pRegistration->sczRegistrationKey, KEY_WRITE, &hkRegistration);
            ExitOnFailure(hr, "Failed to open registration key.");
        }
        else
        {
            // Delete registration key.
            hr = RegDelete(pRegistration->hkRoot, pRegistration->sczRegistrationKey, REG_KEY_DEFAULT, FALSE);
            if (E_FILENOTFOUND != hr)
            {
                ExitOnFailure1(hr, "Failed to delete registration key: %ls", pRegistration->sczRegistrationKey);
            }

            LogId(REPORT_STANDARD, MSG_UNCACHE_BUNDLE, pRegistration->sczId, pRegistration->sczCacheDirectory);

            // Delete cache directory.
            hr = DirEnsureDeleteEx(pRegistration->sczCacheDirectory, DIR_DELETE_FILES | DIR_DELETE_RECURSE | DIR_DELETE_SCHEDULE);
            ExitOnFailure1(hr, "Failed to remove bundle directory: %ls", pRegistration->sczCacheDirectory);

            // Try to remove root package cache in the off chance it is now empty.
            HRESULT hrIgnored = CacheGetCompletedPath(pRegistration->fPerMachine, L"", &sczRootCacheDirectory);
            if (SUCCEEDED(hrIgnored))
            {
                ::RemoveDirectoryW(sczRootCacheDirectory);
            }
        }
    }

    // Update resume mode.
    hr = UpdateResumeMode(pRegistration, hkRegistration, resumeMode, fPerMachineProcess);
    ExitOnFailure(hr, "Failed to update resume mode.");

    // Return resume mode.
    if (pResumeMode)
    {
        *pResumeMode = resumeMode;
    }

LExit:
    ReleaseStr(sczRootCacheDirectory);
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
    ExitOnFailure1(hr, "Failed to write state to file: %ls", pRegistration->sczStateFile);

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
    ExitOnFailure1(hr, "Failed to read state from file: %ls", pRegistration->sczStateFile);

LExit:
    return hr;
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
            hr = RegWriteNumber(hkRegistration, L"Resume", (DWORD)resumeMode);
            ExitOnFailure(hr, "Failed to write Resume value.");
        }

        // If we are entering reboot-pending mode, write a volatile
        // registry key to track when the reboot has taken place.
        if (BURN_RESUME_MODE_REBOOT_PENDING == resumeMode)
        {
            // create registry key
            hr = RegCreateEx(hkRegistration, L"RebootRequired", KEY_WRITE, TRUE, NULL, &hkRebootRequired, NULL);
            ExitOnFailure(hr, "Failed to create resume key.");
        }
    }

    // update run key, this always happens in the per-user process
    if (!fPerMachineProcess)
    {
        if (BURN_RESUME_MODE_SUSPEND == resumeMode || BURN_RESUME_MODE_ARP == resumeMode || BURN_RESUME_MODE_NONE == resumeMode)
        {
            // delete run key value
            hr = RegOpen(HKEY_CURRENT_USER, REGISTRY_RUN_KEY, KEY_WRITE, &hkRun);
            if (E_FILENOTFOUND == hr || E_PATHNOTFOUND == hr)
            {
                hr = S_OK;
            }
            else
            {
                ExitOnWin32Error(er, hr, "Failed to open run key.");

                er = ::RegDeleteValueW(hkRun, pRegistration->sczId);
                ExitOnWin32Error(er, hr, "Failed to delete run key value.");
            }
        }
        else
        {
            // write run key
            hr = RegCreate(HKEY_CURRENT_USER, REGISTRY_RUN_KEY, KEY_WRITE, &hkRun);
            ExitOnFailure(hr, "Failed to create run key.");

            hr = RegWriteString(hkRun, pRegistration->sczId, pRegistration->sczResumeCommandLine);
            ExitOnFailure(hr, "Failed to write run key value.");
        }
    }

LExit:
    ReleaseRegKey(hkRebootRequired);
    ReleaseRegKey(hkRun);

    return hr;
}

static HRESULT ParseRelatedCodes(
    __in BURN_REGISTRATION* pRegistration,
    __in IXMLDOMNode* pixnBundle
    )
{
    HRESULT hr = S_OK;
    IXMLDOMNodeList* pixnNodes = NULL;
    IXMLDOMNode* pixnElement = NULL;
    LPWSTR sczAction = NULL;
    LPWSTR sczId = NULL;
    DWORD dwIndex = 0;
    DWORD cElements = 0;

    hr = XmlSelectNodes(pixnBundle, L"RelatedBundle", &pixnNodes);
    ExitOnFailure(hr, "Failed to get RelatedBundle nodes");

    hr = pixnNodes->get_length((long*)&cElements);
    ExitOnFailure(hr, "Failed to get RelatedBundle element count.");

    for (DWORD i = 0; i < cElements; ++i)
    {
        hr = XmlNextElement(pixnNodes, &pixnElement, NULL);
        ExitOnFailure(hr, "Failed to get next RelatedBundle element.");

        hr = XmlGetAttributeEx(pixnElement, L"Action", &sczAction);
        ExitOnFailure(hr, "Failed to get @Action.");

        hr = XmlGetAttributeEx(pixnElement, L"Id", &sczId);
        ExitOnFailure(hr, "Failed to get @Id.");

        if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, 0, sczAction, -1, L"Detect", -1))
        {
            hr = MemEnsureArraySize(reinterpret_cast<LPVOID*>(&pRegistration->rgsczDetectCodes), pRegistration->cDetectCodes + 1, sizeof(LPWSTR), 5);
            ExitOnFailure(hr, "Failed to resize Detect code array in registration");

            pRegistration->rgsczDetectCodes[pRegistration->cDetectCodes] = sczId;
            sczId = NULL;
            ++pRegistration->cDetectCodes;
        }
        else if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, 0, sczAction, -1, L"Upgrade", -1))
        {
            hr = MemEnsureArraySize(reinterpret_cast<LPVOID*>(&pRegistration->rgsczUpgradeCodes), pRegistration->cUpgradeCodes + 1, sizeof(LPWSTR), 5);
            ExitOnFailure(hr, "Failed to resize Upgrade code array in registration");

            pRegistration->rgsczUpgradeCodes[pRegistration->cUpgradeCodes] = sczId;
            sczId = NULL;
            ++pRegistration->cUpgradeCodes;
        }
        else if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, 0, sczAction, -1, L"Addon", -1))
        {
            hr = MemEnsureArraySize(reinterpret_cast<LPVOID*>(&pRegistration->rgsczAddonCodes), pRegistration->cAddonCodes + 1, sizeof(LPWSTR), 5);
            ExitOnFailure(hr, "Failed to resize Addon code array in registration");

            pRegistration->rgsczAddonCodes[pRegistration->cAddonCodes] = sczId;
            sczId = NULL;
            ++pRegistration->cAddonCodes;
        }
        else
        {
            hr = E_INVALIDARG;
            ExitOnFailure1(hr, "Invalid value for @Action: %ls", sczAction);
        }
    }

LExit:
    ReleaseObject(pixnNodes);
    ReleaseObject(pixnElement);
    ReleaseStr(sczAction);
    ReleaseStr(sczId);

    return hr;
}

static HRESULT InitializeRelatedBundleFromKey(
    __in_z LPCWSTR wzBundleId,
    __in HKEY hkBundleId,
    __in BOOL fPerMachine,
    __inout BURN_RELATED_BUNDLE *pRelatedBundle
    )
{
    HRESULT hr = S_OK;
    LPWSTR sczCachePath = NULL;

    hr = RegReadVersion(hkBundleId, REGISTRY_BUNDLE_VERSION, &pRelatedBundle->qwVersion);
    ExitOnFailure1(hr, "Failed to read version from registry for bundle: %ls", wzBundleId);

    hr = RegReadString(hkBundleId, REGISTRY_BUNDLE_CACHE_PATH, &sczCachePath);
    ExitOnFailure1(hr, "Failed to read cache path from registry for bundle: %ls", wzBundleId);

    // Initialize the single payload, and fill out all the necessary fields
    pRelatedBundle->package.rgPayloads = (BURN_PACKAGE_PAYLOAD *)MemAlloc(sizeof(BURN_PACKAGE_PAYLOAD), TRUE); 
    ExitOnNull(pRelatedBundle->package.rgPayloads, hr, E_OUTOFMEMORY, "Failed to allocate space for burn package payload inside of related bundle struct");
    pRelatedBundle->package.cPayloads = 1;

    pRelatedBundle->package.rgPayloads->fCached = TRUE;

    pRelatedBundle->package.rgPayloads->pPayload = (BURN_PAYLOAD *)MemAlloc(sizeof(BURN_PAYLOAD), TRUE); 
    ExitOnNull(pRelatedBundle->package.rgPayloads, hr, E_OUTOFMEMORY, "Failed to allocate space for burn payload inside of related bundle struct");
    pRelatedBundle->package.rgPayloads->pPayload->packaging = BURN_PAYLOAD_PACKAGING_EXTERNAL;

    hr = FileSize(sczCachePath, reinterpret_cast<LONGLONG *>(&pRelatedBundle->package.rgPayloads->pPayload->qwFileSize));
    ExitOnFailure1(hr, "Failed to get size of related bundle: %ls", sczCachePath);

    pRelatedBundle->package.rgPayloads->pPayload->sczFilePath = sczCachePath;
    sczCachePath = NULL;

    pRelatedBundle->package.type = BURN_PACKAGE_TYPE_EXE;
    pRelatedBundle->package.fPerMachine = fPerMachine;
    pRelatedBundle->package.currentState = BOOTSTRAPPER_PACKAGE_STATE_PRESENT;
    pRelatedBundle->package.requested = BOOTSTRAPPER_REQUEST_STATE_ABSENT;
    pRelatedBundle->package.fUninstallable = TRUE;
    pRelatedBundle->package.fVital = FALSE;
    
    hr = StrAllocString(&pRelatedBundle->package.Exe.sczInstallArguments, L"/quiet", 0);
    ExitOnFailure(hr, "Failed to copy install arguments for related bundle package");

    hr = StrAllocString(&pRelatedBundle->package.Exe.sczRepairArguments, L"/repair /quiet", 0);
    ExitOnFailure(hr, "Failed to copy repair arguments for related bundle package");

    hr = StrAllocString(&pRelatedBundle->package.Exe.sczUninstallArguments, L"/uninstall /quiet", 0);
    ExitOnFailure(hr, "Failed to copy uninstall arguments for related bundle package");

    pRelatedBundle->package.Exe.fRepairable = TRUE;
    pRelatedBundle->package.Exe.protocol = BURN_EXE_PROTOCOL_TYPE_BURN;

LExit:
    ReleaseStr(sczCachePath);

    return hr;
}

static HRESULT FindMatchingStringBetweenArrays(
    __in_ecount(cValues) LPCWSTR *rgwzStringArray1,
    __in DWORD cStringArray1,
    __in_ecount(cValues) LPCWSTR *rgwzStringArray2,
    __in DWORD cStringArray2
    )
{
    HRESULT hr = S_OK;

    for (DWORD i = 0; i < cStringArray1; ++i)
    {
        for (DWORD j = 0; j < cStringArray2; ++j)
        {
            if (CSTR_EQUAL == ::CompareStringW(LOCALE_NEUTRAL, NORM_IGNORECASE, rgwzStringArray2[j], -1, rgwzStringArray1[i], -1))
            {
                ExitFunction1(hr = S_OK);
            }
        }
    }

    ExitFunction1(hr = HRESULT_FROM_WIN32(ERROR_NO_MATCH));

LExit:
    return hr;
}

static HRESULT RegistrationDetectRelatedBundlesForKey(
    __in_opt BURN_USER_EXPERIENCE* pUX,
    __in BURN_REGISTRATION* pRegistration,
    __in_opt BOOTSTRAPPER_COMMAND* pCommand,
    __in HKEY hkRoot
    )
{
    HRESULT hr = S_OK;
    BURN_RELATION_TYPE relationType = BURN_RELATION_NONE;
    HKEY hkUninstallKey = NULL;
    LPWSTR sczBundleId = NULL;

    hr = RegOpen(hkRoot, REGISTRY_UNINSTALL_KEY, KEY_READ, &hkUninstallKey);
    if (HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND) == hr || HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr)
    {
        ExitFunction1(hr = S_OK);
    }
    ExitOnFailure(hr, "Failed to open uninstall registry key.");

    for (DWORD dwIndex = 0; /* exit via break below */; ++dwIndex)
    {
        hr = RegKeyEnum(hkUninstallKey, dwIndex, &sczBundleId);
        if (E_NOMOREITEMS == hr)
        {
            hr = S_OK;
            break;
        }
        ExitOnFailure(hr, "Failed to enumerate uninstall key.");

        // If we did not find ourself, try to load the subkey as a related bundle.
        if (CSTR_EQUAL != ::CompareStringW(LOCALE_NEUTRAL, NORM_IGNORECASE, sczBundleId, -1, pRegistration->sczId, -1))
        {
            // Ignore failures here since we'll often find products that aren't actually
            // related bundles (or even bundles at all).
            relationType = BURN_RELATION_NONE;

            hr = RegistrationLoadRelatedBundle(pRegistration, sczBundleId, &relationType);
            if (SUCCEEDED(hr))
            {
                BURN_RELATED_BUNDLE* pRelatedBundle = pRegistration->rgRelatedBundles + pRegistration->cRelatedBundles - 1;

                pRelatedBundle->relationType = relationType;

                if (pUX)
                {
                    BOOTSTRAPPER_RELATED_OPERATION operation = BOOTSTRAPPER_RELATED_OPERATION_NONE;

                    switch (relationType)
                    {
                    case BURN_RELATION_UPGRADE:
                        if (pRegistration->qwVersion > pRelatedBundle->qwVersion)
                        {
                            operation = BOOTSTRAPPER_RELATED_OPERATION_MAJOR_UPGRADE;
                        }
                        else if (pRegistration->qwVersion < pRelatedBundle->qwVersion)
                        {
                            operation = BOOTSTRAPPER_RELATED_OPERATION_DOWNGRADE;
                        }
                        break;

                    case BURN_RELATION_ADDON:
                        if (pCommand && BOOTSTRAPPER_ACTION_UNINSTALL == pCommand->action)
                        {
                            operation = BOOTSTRAPPER_RELATED_OPERATION_REMOVE;
                        }
                        break;

                    default:
                        hr = E_FAIL;
                        ExitOnFailure1(hr, "Unexpected relation type encountered: %d", relationType);
                        break;
                    }

                    LogId(REPORT_STANDARD, MSG_DETECTED_RELATED_BUNDLE, pRelatedBundle->package.sczId, LoggingPerMachineToString(pRelatedBundle->package.fPerMachine), LoggingVersionToString(pRelatedBundle->qwVersion), LoggingRelatedOperationToString(operation));

                    int nResult = pUX->pUserExperience->OnDetectRelatedBundle(pRelatedBundle->package.sczId, pRelatedBundle->package.fPerMachine, pRelatedBundle->qwVersion, operation);
                    hr = HRESULT_FROM_VIEW(nResult);
                    ExitOnRootFailure(hr, "UX aborted detect related bundle.");
                }
            }
        }
    }

LExit:
    ReleaseStr(sczBundleId);
    ReleaseRegKey(hkUninstallKey);

    return hr;
}
