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
const LPCWSTR REGISTRY_RUN_KEY = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnce";
const LPCWSTR REGISTRY_REBOOT_PENDING_FORMAT = L"%ls.RebootRequired";
const LPCWSTR REGISTRY_BUNDLE_CACHE_PATH = L"BundleCachePath";
const LPCWSTR REGISTRY_BUNDLE_UPGRADE_CODE = L"BundleUpgradeCode";
const LPCWSTR REGISTRY_BUNDLE_ADDON_CODE = L"BundleAddonCode";
const LPCWSTR REGISTRY_BUNDLE_DETECT_CODE = L"BundleDetectCode";
const LPCWSTR REGISTRY_BUNDLE_PATCH_CODE = L"BundlePatchCode";
const LPCWSTR REGISTRY_BUNDLE_TAG = L"BundleTag";
const LPCWSTR REGISTRY_BUNDLE_VERSION = L"BundleVersion";
const LPCWSTR REGISTRY_ENGINE_VERSION = L"EngineVersion";
const LPCWSTR REGISTRY_BUNDLE_INSTALLED = L"Installed";

// internal function declarations

static HRESULT SetPaths(
    __in BURN_REGISTRATION* pRegistration
    );
static HRESULT GetBundleName(
    __in BURN_REGISTRATION* pRegistration,
    __in BURN_VARIABLES* pVariables,
    __out LPWSTR* psczBundleName
    );
static HRESULT UpdateResumeMode(
    __in BURN_REGISTRATION* pRegistration,
    __in HKEY hkRegistration,
    __in BURN_RESUME_MODE resumeMode,
    __in BOOL fRestartInitiated,
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
    __inout BURN_RELATED_BUNDLE *pRelatedBundle,
    __out LPWSTR *psczTag
    );
static HRESULT FindMatchingStringBetweenArrays(
    __in_ecount(cValues) LPCWSTR *rgwzStringArray1,
    __in DWORD cStringArray1,
    __in_ecount(cValues) LPCWSTR *rgwzStringArray2,
    __in DWORD cStringArray2
    );
static HRESULT DetectRelatedBundlesForKey(
    __in_opt BURN_USER_EXPERIENCE* pUX,
    __in BURN_REGISTRATION* pRegistration,
    __in_opt BOOTSTRAPPER_COMMAND* pCommand,
    __in BOOL fPerMachine
    );
static HRESULT LoadRelatedBundle(
    __in BURN_REGISTRATION* pRegistration,
    __in HKEY hkUninstallKey,
    __in BOOL fPerMachine,
    __in_z LPCWSTR sczBundleId,
    __out BOOTSTRAPPER_RELATION_TYPE *pRelationType,
    __out LPWSTR *psczTag
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

    // @Tag
    hr = XmlGetAttributeEx(pixnRegistrationNode, L"Tag", &pRegistration->sczTag);
    ExitOnFailure(hr, "Failed to get @Tag.");

    hr = ParseRelatedCodes(pRegistration, pixnBundle);
    ExitOnFailure(hr, "Failed to parse related bundles");

    // @Version
    hr = XmlGetAttributeEx(pixnRegistrationNode, L"Version", &scz);
    ExitOnFailure(hr, "Failed to get @Version.");

    hr = FileVersionFromStringEx(scz, 0, &pRegistration->qwVersion);
    ExitOnFailure1(hr, "Failed to parse @Version: %ls", scz);

    // @ProviderKey
    hr = XmlGetAttributeEx(pixnRegistrationNode, L"ProviderKey", &pRegistration->sczProviderKey);
    ExitOnFailure(hr, "Failed to get @ProviderKey.");

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

        // @Register
        hr = XmlGetYesNoAttribute(pixnArpNode, L"Register", &pRegistration->fRegisterArp);
        ExitOnFailure(hr, "Failed to get @Register.");

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
        hr = XmlGetAttributeEx(pixnArpNode, L"DisableModify", &scz);
        if (SUCCEEDED(hr))
        {
            if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, 0, scz, -1, L"button", -1))
            {
                pRegistration->modify = BURN_REGISTRATION_MODIFY_DISABLE_BUTTON;
            }
            else if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, 0, scz, -1, L"yes", -1))
            {
                pRegistration->modify = BURN_REGISTRATION_MODIFY_DISABLE;
            }
            else if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, 0, scz, -1, L"no", -1))
            {
                pRegistration->modify = BURN_REGISTRATION_MODIFY_ENABLED;
            }
            else
            {
                hr = E_UNEXPECTED;
                ExitOnRootFailure1(hr, "Invalid modify disabled type: %ls", scz);
            }
        }
        else if (E_NOTFOUND == hr)
        {
            pRegistration->modify = BURN_REGISTRATION_MODIFY_ENABLED;
            hr = S_OK;
        }
        ExitOnFailure(hr, "Failed to get @DisableModify.");

        // @DisableRemove
        hr = XmlGetYesNoAttribute(pixnArpNode, L"DisableRemove", &pRegistration->fNoRemove);
        if (E_NOTFOUND != hr)
        {
            ExitOnFailure(hr, "Failed to get @DisableRemove.");
            pRegistration->fNoRemoveDefined = TRUE;
        }
    }

    hr = SetPaths(pRegistration);
    ExitOnFailure(hr, "Failed to set registration paths.");

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
    ReleaseStr(pRegistration->sczTag);

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

    for (DWORD i = 0; i < pRegistration->cPatchCodes; ++i)
    {
        ReleaseStr(pRegistration->rgsczPatchCodes[i]);
    }
    ReleaseMem(pRegistration->rgsczPatchCodes);

    ReleaseStr(pRegistration->sczProviderKey);
    ReleaseStr(pRegistration->sczExecutableName);

    ReleaseStr(pRegistration->sczRegistrationKey);
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

    if (pRegistration->relatedBundles.rgRelatedBundles)
    {
        for (DWORD i = 0; i < pRegistration->relatedBundles.cRelatedBundles; ++i)
        {
            PackageUninitialize(&pRegistration->relatedBundles.rgRelatedBundles[i].package);
        }

        MemFree(pRegistration->relatedBundles.rgRelatedBundles);
    }

    // clear struct
    memset(pRegistration, 0, sizeof(BURN_REGISTRATION));
}

/*******************************************************************
 RegistrationSetVariables - Initializes bundle variables that map to
                            registration entities.

*******************************************************************/
extern "C" HRESULT RegistrationSetVariables(
    __in BURN_REGISTRATION* pRegistration,
    __in BURN_VARIABLES* pVariables
    )
{
    HRESULT hr = S_OK;
    LPWSTR scz = NULL;

    // Ensure the registration bundle name is updated.
    hr = GetBundleName(pRegistration, pVariables, &scz);
    ExitOnFailure(hr, "Failed to intitialize bundle name.");

    hr = VariableSetString(pVariables, BURN_BUNDLE_PROVIDER_KEY, pRegistration->sczProviderKey, TRUE);
    ExitOnFailure(hr, "Failed to overwrite the bundle provider key built-in variable.");

    hr = VariableSetString(pVariables, BURN_BUNDLE_TAG, pRegistration->sczTag, TRUE);
    ExitOnFailure(hr, "Failed to overwrite the bundle tag built-in variable.");

LExit:
    ReleaseStr(scz);
    return hr;
}

extern "C" HRESULT RegistrationDetectInstalled(
    __in BURN_REGISTRATION* pRegistration,
    __out BOOL* pfInstalled
    )
{
    HRESULT hr = S_OK;
    HKEY hkRegistration = NULL;
    DWORD dwInstalled = 0;

    // open registration key
    hr = RegOpen(pRegistration->hkRoot, pRegistration->sczRegistrationKey, KEY_QUERY_VALUE, &hkRegistration);
    if (SUCCEEDED(hr))
    {
        hr = RegReadNumber(hkRegistration, REGISTRY_BUNDLE_INSTALLED, &dwInstalled);
    }

    // Not finding the key or value is okay.
    if (E_FILENOTFOUND == hr || E_PATHNOTFOUND == hr)
    {
        hr = S_OK;
    }

    *pfInstalled = (1 == dwInstalled);

    ReleaseRegKey(hkRegistration);
    return hr;
}

/*******************************************************************
 RegistrationDetectResumeMode - Detects registration information on the system
                                to determine if a resume is taking place.

*******************************************************************/
extern "C" HRESULT RegistrationDetectResumeType(
    __in BURN_REGISTRATION* pRegistration,
    __out BOOTSTRAPPER_RESUME_TYPE* pResumeType
    )
{
    HRESULT hr = S_OK;
    LPWSTR sczRebootRequiredKey = NULL;
    HKEY hkRebootRequired = NULL;
    HKEY hkRegistration = NULL;
    DWORD dwResume = 0;

    // Check to see if a restart is pending for this bundle.
    hr = StrAllocFormatted(&sczRebootRequiredKey, REGISTRY_REBOOT_PENDING_FORMAT, pRegistration->sczRegistrationKey);
    ExitOnFailure(hr, "Failed to format pending restart registry key to read.");

    hr = RegOpen(pRegistration->hkRoot, sczRebootRequiredKey, KEY_QUERY_VALUE, &hkRebootRequired);
    if (SUCCEEDED(hr))
    {
        *pResumeType = BOOTSTRAPPER_RESUME_TYPE_REBOOT_PENDING;
        ExitFunction1(hr = S_OK);
    }

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
        *pResumeType = BOOTSTRAPPER_RESUME_TYPE_INVALID;
        ExitFunction1(hr = S_OK);
    }
    ExitOnFailure(hr, "Failed to read Resume value.");

    switch (dwResume)
    {
    case BURN_RESUME_MODE_ACTIVE:
        // a previous run was interrupted
        *pResumeType = BOOTSTRAPPER_RESUME_TYPE_INTERRUPTED;
        break;

    case BURN_RESUME_MODE_SUSPEND:
        *pResumeType = BOOTSTRAPPER_RESUME_TYPE_SUSPEND;
        break;

    case BURN_RESUME_MODE_ARP:
        *pResumeType = BOOTSTRAPPER_RESUME_TYPE_ARP;
        break;

    case BURN_RESUME_MODE_REBOOT_PENDING:
        // The volatile pending registry doesn't exist (checked above) which means
        // the system was successfully restarted.
        *pResumeType = BOOTSTRAPPER_RESUME_TYPE_REBOOT;
        break;

    default:
        // the value stored in the registry is not valid
        *pResumeType = BOOTSTRAPPER_RESUME_TYPE_INVALID;
        break;
    }

LExit:
    ReleaseRegKey(hkRegistration);
    ReleaseRegKey(hkRebootRequired);
    ReleaseStr(sczRebootRequiredKey);

    return hr;
}

/*******************************************************************
 RegistrationDetectRelatedBundles - finds the bundles with same upgrade code.

*******************************************************************/
extern "C" HRESULT RegistrationDetectRelatedBundles(
    __in BOOL fElevated,
    __in BURN_USER_EXPERIENCE* pUX,
    __in BURN_REGISTRATION* pRegistration,
    __in_opt BOOTSTRAPPER_COMMAND* pCommand
    )
{
    HRESULT hr = S_OK;

    if (fElevated)
    {
        hr = DetectRelatedBundlesForKey(pUX, pRegistration, pCommand, TRUE);
        ExitOnFailure(hr, "Failed to detect related bundles in HKLM for elevated process");
    }
    else
    {
        hr = DetectRelatedBundlesForKey(pUX, pRegistration, pCommand, FALSE);
        ExitOnFailure(hr, "Failed to detect related bundles in HKCU for non-elevated process");

        hr = DetectRelatedBundlesForKey(pUX, pRegistration, pCommand, TRUE);
        ExitOnFailure(hr, "Failed to detect related bundles in HKLM for non-elevated process");
    }

LExit:
    return hr;
}

/*******************************************************************
 RegistrationSessionBegin - Registers a run session on the system.

*******************************************************************/
extern "C" HRESULT RegistrationSessionBegin(
    __in_z LPCWSTR wzEngineWorkingPath,
    __in BURN_REGISTRATION* pRegistration,
    __in BURN_VARIABLES* pVariables,
    __in BURN_USER_EXPERIENCE* pUserExperience,
    __in BOOTSTRAPPER_ACTION action,
    __in DWORD64 qwEstimatedSize,
    __in BOOL fPerMachineProcess
    )
{
    HRESULT hr = S_OK;
    DWORD dwSize = 0;
    HKEY hkRegistration = NULL;
    LPWSTR sczDisplayName = NULL;

    // alter registration in the correct process
    if (pRegistration->fPerMachine == fPerMachineProcess)
    {
        // On install, cache executable
        if (BOOTSTRAPPER_ACTION_INSTALL == action)
        {
            hr = CacheCompleteBundle(pRegistration->fPerMachine, pRegistration->sczExecutableName, pRegistration->sczId, &pUserExperience->payloads, wzEngineWorkingPath
#ifdef DEBUG
                            , pRegistration->sczCacheExecutablePath
#endif
                            );
            ExitOnFailure1(hr, "Failed to cache bundle from path: %ls", wzEngineWorkingPath);
        }

        // create registration key
        hr = RegCreate(pRegistration->hkRoot, pRegistration->sczRegistrationKey, KEY_WRITE, &hkRegistration);
        ExitOnFailure(hr, "Failed to create registration key.");

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

            hr = RegWriteStringArray(hkRegistration, REGISTRY_BUNDLE_PATCH_CODE, pRegistration->rgsczPatchCodes, pRegistration->cPatchCodes);
            ExitOnFailure(hr, "Failed to write BundlePatchCode value.");

            hr = RegWriteStringFormatted(hkRegistration, REGISTRY_BUNDLE_VERSION, L"%hu.%hu.%hu.%hu", (WORD)(pRegistration->qwVersion >> 48), (WORD)(pRegistration->qwVersion >> 32), (WORD)(pRegistration->qwVersion >> 16), (WORD)(pRegistration->qwVersion));
            ExitOnFailure(hr, "Failed to write BundleVersion value.");

            if (pRegistration->sczTag)
            {
                hr = RegWriteString(hkRegistration, REGISTRY_BUNDLE_TAG, pRegistration->sczTag);
                ExitOnFailure(hr, "Failed to write BundleTag value.");
            }

            hr = RegWriteStringFormatted(hkRegistration, REGISTRY_ENGINE_VERSION, L"%hs", szVerMajorMinorBuild);
            ExitOnFailure(hr, "Failed to write EngineVersion value.");

            // DisplayIcon: [path to exe] and ",0" to refer to the first icon in the executable.
            hr = RegWriteStringFormatted(hkRegistration, L"DisplayIcon", L"%s,0", pRegistration->sczCacheExecutablePath);
            ExitOnFailure(hr, "Failed to write DisplayIcon value.");

            // DisplayName: provided by UI
            hr = GetBundleName(pRegistration, pVariables, &sczDisplayName);
            hr = RegWriteString(hkRegistration, L"DisplayName", SUCCEEDED(hr) ? sczDisplayName : pRegistration->sczDisplayName);
            ExitOnFailure(hr, "Failed to write DisplayName value.");

            // EstimatedSize
            qwEstimatedSize /= 1024; // Convert bytes to KB
            if (0 < qwEstimatedSize)
            {
                // Convert bytes to KB as ARP expects
                if (DWORD_MAX < qwEstimatedSize)
                {
                    // ARP doesn't support QWORDs here
                    dwSize = DWORD_MAX;
                }
                else
                {
                    dwSize = static_cast<DWORD>(qwEstimatedSize);
                }

                hr = RegWriteNumber(hkRegistration, L"EstimatedSize", dwSize);
                ExitOnFailure(hr, "Failed to write EstimatedSize value.");
            }

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
            if (BURN_REGISTRATION_MODIFY_DISABLE == pRegistration->modify)
            {
                hr = RegWriteNumber(hkRegistration, L"NoModify", 1);
                ExitOnFailure(hr, "Failed to set NoModify value.");
            }
            else if (BURN_REGISTRATION_MODIFY_DISABLE_BUTTON != pRegistration->modify) // if support modify (aka: did not disable anything)
            {
                // ModifyPath: [path to exe] /modify
                hr = RegWriteStringFormatted(hkRegistration, L"ModifyPath", L"\"%ls\" /modify", pRegistration->sczCacheExecutablePath);
                ExitOnFailure(hr, "Failed to write ModifyPath value.");

                // NoElevateOnModify: 1
                hr = RegWriteNumber(hkRegistration, L"NoElevateOnModify", 1);
                ExitOnFailure(hr, "Failed to set NoElevateOnModify value.");
            }

            // NoRemove: should this be allowed?
            if (pRegistration->fNoRemoveDefined)
            {
                hr = RegWriteNumber(hkRegistration, L"NoRemove", (DWORD)pRegistration->fNoRemove);
                ExitOnFailure(hr, "Failed to set NoRemove value.");
            }

            // Conditionally hide the ARP entry.
            if (!pRegistration->fRegisterArp)
            {
                hr = RegWriteNumber(hkRegistration, L"SystemComponent", 1);
                ExitOnFailure(hr, "Failed to set SystemComponent value.");
            }

            // QuietUninstallString: [path to exe] /uninstall /quiet
            hr = RegWriteStringFormatted(hkRegistration, L"QuietUninstallString", L"\"%ls\" /uninstall /quiet", pRegistration->sczCacheExecutablePath);
            ExitOnFailure(hr, "Failed to write QuietUninstallString value.");

            // UninstallString, [path to exe]
            // If the modify button is to be disabled, we'll add "/modify" to the uninstall string because the button is "Uninstall/Change". Otherwise,
            // it's just the "Uninstall" button so we add "/uninstall" to make the program just go away.
            LPCWSTR wzUninstallParameters = (BURN_REGISTRATION_MODIFY_DISABLE_BUTTON == pRegistration->modify) ? L"/modify" : L" /uninstall";
            hr = RegWriteStringFormatted(hkRegistration, L"UninstallString", L"\"%ls\" %ls", pRegistration->sczCacheExecutablePath, wzUninstallParameters);
            ExitOnFailure(hr, "Failed to write UninstallString value.");
        }

        // TODO: if we are not uninstalling, update estimated size
        //if (BOOTSTRAPPER_ACTION_UNINSTALL != action)
        //{
        //}

        // Register the bundle dependency key.
        if (BOOTSTRAPPER_ACTION_INSTALL == action || BOOTSTRAPPER_ACTION_REPAIR == action)
        {
            hr = DependencyRegisterBundle(pRegistration);
            ExitOnFailure(hr, "Failed to register the bundle dependency key.");
        }

        // update resume mode
        hr = UpdateResumeMode(pRegistration, hkRegistration, BURN_RESUME_MODE_ACTIVE, FALSE, fPerMachineProcess);
        ExitOnFailure(hr, "Failed to update resume mode.");
    }

LExit:
    ReleaseStr(sczDisplayName);
    ReleaseRegKey(hkRegistration);

    return hr;
}


/*******************************************************************
 RegistrationSessionResume - Resumes a previous run session.

*******************************************************************/
extern "C" HRESULT RegistrationSessionResume(
    __in BURN_REGISTRATION* pRegistration,
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

        // update resume mode
        hr = UpdateResumeMode(pRegistration, hkRegistration, BURN_RESUME_MODE_ACTIVE, FALSE, fPerMachineProcess);
        ExitOnFailure(hr, "Failed to update resume mode.");
    }

LExit:
    ReleaseRegKey(hkRegistration);

    return hr;
}


/*******************************************************************
 RegistrationSessionEnd - Unregisters a run session from the system.

 *******************************************************************/
extern "C" HRESULT RegistrationSessionEnd(
    __in BURN_REGISTRATION* pRegistration,
    __in BOOL fKeepRegistration,
    __in BOOL fSuspend,
    __in BOOTSTRAPPER_APPLY_RESTART restart,
    __in BOOL fPerMachineProcess,
    __out_opt BURN_RESUME_MODE* pResumeMode
    )
{
    HRESULT hr = S_OK;
    BURN_RESUME_MODE resumeMode = BURN_RESUME_MODE_NONE;
    LPWSTR sczRebootRequiredKey = NULL;
    HKEY hkRebootRequired = NULL;
    HKEY hkRegistration = NULL;

    // Calculate the correct resume mode. If a restart has been initiated, that trumps all other
    // modes. If the user chose to suspend the install then we'll use that as the resume mode.
    // Barring those special cases, if it was determined that we should keep the registration
    // do that otherwise the resume mode was initialized to none and registration will be removed.
    if (BOOTSTRAPPER_APPLY_RESTART_INITIATED == restart)
    {
        resumeMode = BURN_RESUME_MODE_REBOOT_PENDING;
    }
    else if (fSuspend)
    {
        resumeMode = BURN_RESUME_MODE_SUSPEND;
    }
    else if (fKeepRegistration)
    {
        resumeMode = BURN_RESUME_MODE_ARP;
    }

    // Alter registration in the correct process.
    if (pRegistration->fPerMachine == fPerMachineProcess)
    {
        // If a restart is required for any reason, write a volatile registry key to track of
        // of that fact until the reboot has taken place.
        if (BOOTSTRAPPER_APPLY_RESTART_NONE != restart)
        {
            // We'll write the volatile registry key right next to the bundle ARP registry key
            // because that's easy. This is all best effort since the worst case just means in
            // the rare case the user launches the same install again before taking the restart
            // the BA won't know a restart was still required.
            hr = StrAllocFormatted(&sczRebootRequiredKey, REGISTRY_REBOOT_PENDING_FORMAT, pRegistration->sczRegistrationKey);
            if (SUCCEEDED(hr))
            {
                hr = RegCreateEx(pRegistration->hkRoot, sczRebootRequiredKey, KEY_WRITE, TRUE, NULL, &hkRebootRequired, NULL);
            }

            if (FAILED(hr))
            {
                ExitTrace(hr, "Failed to write volatile reboot required registry key.");
                hr = S_OK;
            }
        }

        // If no resume mode, then remove the bundle registration.
        if (BURN_RESUME_MODE_NONE == resumeMode)
        {
            // Remove the bundle dependency key.
            hr = DependencyUnregisterBundle(pRegistration);
            ExitOnFailure(hr, "Failed to remove the bundle dependency key.");

            // Delete registration key.
            hr = RegDelete(pRegistration->hkRoot, pRegistration->sczRegistrationKey, REG_KEY_DEFAULT, FALSE);
            if (E_FILENOTFOUND != hr)
            {
                ExitOnFailure1(hr, "Failed to delete registration key: %ls", pRegistration->sczRegistrationKey);
            }

            hr = CacheRemoveBundle(pRegistration->fPerMachine, pRegistration->sczId);
            ExitOnFailure(hr, "Failed to remove bundle from cache.");
        }
        else // the mode needs to be updated so open the registration key.
        {
            // Open registration key.
            hr = RegOpen(pRegistration->hkRoot, pRegistration->sczRegistrationKey, KEY_WRITE, &hkRegistration);
            ExitOnFailure(hr, "Failed to open registration key.");
        }

        // Update resume mode.
        hr = UpdateResumeMode(pRegistration, hkRegistration, resumeMode, BOOTSTRAPPER_APPLY_RESTART_INITIATED == restart, fPerMachineProcess);
        ExitOnFailure(hr, "Failed to update resume mode.");
    }

    // Return resume mode.
    if (pResumeMode)
    {
        *pResumeMode = resumeMode;
    }

LExit:
    ReleaseRegKey(hkRegistration);
    ReleaseRegKey(hkRebootRequired);
    ReleaseStr(sczRebootRequiredKey);

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

    // write data to file
    hr = FileWrite(pRegistration->sczStateFile, FILE_ATTRIBUTE_NORMAL, pbBuffer, cbBuffer, NULL);
    if (E_PATHNOTFOUND == hr)
    {
        // TODO: should we log that the bundle's cache folder was not present so the state file wasn't created either?
        hr = S_OK;
    }
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
    // read data from file
    HRESULT hr = FileRead(ppbBuffer, pcbBuffer, pRegistration->sczStateFile);
    return hr;
}


// internal helper functions

static HRESULT SetPaths(
    __in BURN_REGISTRATION* pRegistration
    )
{
    HRESULT hr = S_OK;
    LPWSTR sczCacheDirectory = NULL;

    // save registration key root
    pRegistration->hkRoot = pRegistration->fPerMachine ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER;

    // build uninstall registry key path
    hr = StrAllocFormatted(&pRegistration->sczRegistrationKey, L"%s\\%s", REGISTRY_UNINSTALL_KEY, pRegistration->sczId);
    ExitOnFailure(hr, "Failed to build uninstall registry key path.");

    // build cache directory
    hr = CacheGetCompletedPath(pRegistration->fPerMachine, pRegistration->sczId, &sczCacheDirectory);
    ExitOnFailure(hr, "Failed to build cache directory.");

    // build cached executable path
    hr = PathConcat(sczCacheDirectory, pRegistration->sczExecutableName, &pRegistration->sczCacheExecutablePath);
    ExitOnFailure(hr, "Failed to build cached executable path.");

    // build state file path
    hr = StrAllocFormatted(&pRegistration->sczStateFile, L"%s\\state.rsm", sczCacheDirectory);
    ExitOnFailure(hr, "Failed to build state file path.");

LExit:
    ReleaseStr(sczCacheDirectory);
    return hr;
}

static HRESULT GetBundleName(
    __in BURN_REGISTRATION* pRegistration,
    __in BURN_VARIABLES* pVariables,
    __out LPWSTR* psczBundleName
    )
{
    HRESULT hr = S_OK;

    hr = VariableGetString(pVariables, BURN_BUNDLE_NAME, psczBundleName);
    if (E_NOTFOUND == hr)
    {
        hr = VariableSetString(pVariables, BURN_BUNDLE_NAME, pRegistration->sczDisplayName, FALSE);
        ExitOnFailure(hr, "Failed to set bundle name.");

        hr = StrAllocString(psczBundleName, pRegistration->sczDisplayName, 0);
    }
    ExitOnFailure(hr, "Failed to get bundle name.");

LExit:
    return hr;
}

static HRESULT UpdateResumeMode(
    __in BURN_REGISTRATION* pRegistration,
    __in HKEY hkRegistration,
    __in BURN_RESUME_MODE resumeMode,
    __in BOOL fRestartInitiated,
    __in BOOL fPerMachineProcess
    )
{
    Assert(pRegistration->fPerMachine == fPerMachineProcess);

    HRESULT hr = S_OK;
    DWORD er = ERROR_SUCCESS;
    HKEY hkRebootRequired = NULL;
    HKEY hkRun = NULL;
    LPWSTR sczRunOnceCommandLine = NULL;

    // write resume information
    if (hkRegistration)
    {
        // write Resume value
        hr = RegWriteNumber(hkRegistration, L"Resume", (DWORD)resumeMode);
        ExitOnFailure(hr, "Failed to write Resume value.");

        // Write the Installed value *only* when the mode is ARP. This will tell us
        // that the bundle considers itself "installed" on the machine. Note that we
        // never change the value to "0" after that. The bundle will be considered
        // "uninstalled" when all of the registration is removed.
        if (BURN_RESUME_MODE_ARP == resumeMode)
        {
            // Write Installed value.
            hr = RegWriteNumber(hkRegistration, REGISTRY_BUNDLE_INSTALLED, 1);
            ExitOnFailure(hr, "Failed to write Installed value.");
        }
    }

    // If the engine is active write the run key so we resume if there is an unexpected
    // power loss. Also, if a restart was initiated in the middle of the chain then
    // ensure the run key exists (it should since going active would have written it).
    if (BURN_RESUME_MODE_ACTIVE == resumeMode || fRestartInitiated)
    {
        // append RunOnce switch
        hr = StrAllocFormatted(&sczRunOnceCommandLine, L"%ls /%ls", pRegistration->sczResumeCommandLine, BURN_COMMANDLINE_SWITCH_RUNONCE);
        ExitOnFailure(hr, "Failed to format resume command line for RunOnce.");

        // write run key
        hr = RegCreate(pRegistration->hkRoot, REGISTRY_RUN_KEY, KEY_WRITE, &hkRun);
        ExitOnFailure(hr, "Failed to create run key.");

        hr = RegWriteString(hkRun, pRegistration->sczId, sczRunOnceCommandLine);
        ExitOnFailure(hr, "Failed to write run key value.");
    }
    else // delete run key value
    {
        hr = RegOpen(pRegistration->hkRoot, REGISTRY_RUN_KEY, KEY_WRITE, &hkRun);
        if (E_FILENOTFOUND == hr || E_PATHNOTFOUND == hr)
        {
            hr = S_OK;
        }
        else
        {
            ExitOnWin32Error(er, hr, "Failed to open run key.");

            er = ::RegDeleteValueW(hkRun, pRegistration->sczId);
            if (ERROR_FILE_NOT_FOUND == er)
            {
                er = ERROR_SUCCESS;
            }
            ExitOnWin32Error(er, hr, "Failed to delete run key value.");
        }
    }

LExit:
    ReleaseStr(sczRunOnceCommandLine);
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
        else if (CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, 0, sczAction, -1, L"Patch", -1))
        {
            hr = MemEnsureArraySize(reinterpret_cast<LPVOID*>(&pRegistration->rgsczPatchCodes), pRegistration->cPatchCodes + 1, sizeof(LPWSTR), 5);
            ExitOnFailure(hr, "Failed to resize Patch code array in registration");

            pRegistration->rgsczPatchCodes[pRegistration->cPatchCodes] = sczId;
            sczId = NULL;
            ++pRegistration->cPatchCodes;
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
    __inout BURN_RELATED_BUNDLE *pRelatedBundle,
    __out LPWSTR *psczTag
    )
{
    HRESULT hr = S_OK;
    DWORD64 qwEngineVersion = 0;
    LPWSTR sczCachePath = NULL;
    LPCWSTR wzRelationTypeCommandLine = NULL;

    switch (pRelatedBundle->relationType)
    {
    case BOOTSTRAPPER_RELATION_DETECT:
        wzRelationTypeCommandLine = BURN_COMMANDLINE_SWITCH_RELATED_DETECT;
        break;
    case BOOTSTRAPPER_RELATION_UPGRADE:
        wzRelationTypeCommandLine = BURN_COMMANDLINE_SWITCH_RELATED_UPGRADE;
        break;
    case BOOTSTRAPPER_RELATION_ADDON:
        wzRelationTypeCommandLine = BURN_COMMANDLINE_SWITCH_RELATED_ADDON;
        break;
    case BOOTSTRAPPER_RELATION_PATCH:
        wzRelationTypeCommandLine = BURN_COMMANDLINE_SWITCH_RELATED_PATCH;
        break;
    case BOOTSTRAPPER_RELATION_NONE: __fallthrough;
    default:
        hr = HRESULT_FROM_WIN32(ERROR_BAD_FORMAT);
        ExitOnFailure2(hr, "Internal error: bad relation type %d while reading related bundle Id %ls", pRelatedBundle->relationType, wzBundleId);
        break;
    }

    hr = RegReadVersion(hkBundleId, REGISTRY_ENGINE_VERSION, &qwEngineVersion);
    if (FAILED(hr))
    {
        qwEngineVersion = 0;
        hr = S_OK;
    }

    hr = RegReadVersion(hkBundleId, REGISTRY_BUNDLE_VERSION, &pRelatedBundle->qwVersion);
    ExitOnFailure1(hr, "Failed to read version from registry for bundle: %ls", wzBundleId);

    hr = RegReadString(hkBundleId, REGISTRY_BUNDLE_CACHE_PATH, &sczCachePath);
    ExitOnFailure1(hr, "Failed to read cache path from registry for bundle: %ls", wzBundleId);

    hr = RegReadString(hkBundleId, REGISTRY_BUNDLE_TAG, psczTag);
    if (E_FILENOTFOUND == hr)
    {
        hr = S_OK;
    }
    ExitOnFailure1(hr, "Failed to read tag from registry for bundle: %ls", wzBundleId);

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
    pRelatedBundle->package.fUninstallable = TRUE;
    pRelatedBundle->package.fVital = FALSE;

    hr = StrAllocFormatted(&pRelatedBundle->package.Exe.sczInstallArguments, L"-quiet -%ls", wzRelationTypeCommandLine);
    ExitOnFailure(hr, "Failed to copy install arguments for related bundle package");

    hr = StrAllocFormatted(&pRelatedBundle->package.Exe.sczRepairArguments, L"-repair -quiet -%ls", wzRelationTypeCommandLine);
    ExitOnFailure(hr, "Failed to copy repair arguments for related bundle package");

    hr = StrAllocFormatted(&pRelatedBundle->package.Exe.sczUninstallArguments, L"-uninstall -quiet -%ls", wzRelationTypeCommandLine);
    ExitOnFailure(hr, "Failed to copy uninstall arguments for related bundle package");

    pRelatedBundle->package.Exe.fRepairable = TRUE;

    // Only support progress from engines that are compatible (aka: version greater than or equal to last protocol breaking change *and* versions that are older or the same as this engine).
    pRelatedBundle->package.Exe.protocol = (FILEMAKEVERSION(3, 6, 2221, 0) <= qwEngineVersion && qwEngineVersion <= FILEMAKEVERSION(rmj, rmm, rup, 0)) ? BURN_EXE_PROTOCOL_TYPE_BURN : BURN_EXE_PROTOCOL_TYPE_NONE;

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

static HRESULT DetectRelatedBundlesForKey(
    __in_opt BURN_USER_EXPERIENCE* pUX,
    __in BURN_REGISTRATION* pRegistration,
    __in_opt BOOTSTRAPPER_COMMAND* pCommand,
    __in BOOL fPerMachine
    )
{
    HRESULT hr = S_OK;
    BOOTSTRAPPER_RELATION_TYPE relationType = BOOTSTRAPPER_RELATION_NONE;
    HKEY hkUninstallKey = NULL;
    LPWSTR sczBundleId = NULL;
    LPWSTR sczTag = NULL;
    HKEY hkRoot = fPerMachine ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER;

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
            relationType = BOOTSTRAPPER_RELATION_NONE;

            hr = LoadRelatedBundle(pRegistration, hkUninstallKey, fPerMachine, sczBundleId, &relationType, &sczTag);
            if (SUCCEEDED(hr))
            {
                BURN_RELATED_BUNDLE* pRelatedBundle = pRegistration->relatedBundles.rgRelatedBundles + pRegistration->relatedBundles.cRelatedBundles - 1;

                if (pUX)
                {
                    BOOTSTRAPPER_RELATED_OPERATION operation = BOOTSTRAPPER_RELATED_OPERATION_NONE;

                    switch (relationType)
                    {
                    case BOOTSTRAPPER_RELATION_UPGRADE:
                        if (BOOTSTRAPPER_ACTION_INSTALL == pCommand->action)
                        {
                            if (pRegistration->qwVersion > pRelatedBundle->qwVersion)
                            {
                                operation = BOOTSTRAPPER_RELATED_OPERATION_MAJOR_UPGRADE;
                            }
                            else if (pRegistration->qwVersion < pRelatedBundle->qwVersion)
                            {
                                operation = BOOTSTRAPPER_RELATED_OPERATION_DOWNGRADE;
                            }
                        }
                        break;

                    case BOOTSTRAPPER_RELATION_PATCH: __fallthrough;
                    case BOOTSTRAPPER_RELATION_ADDON:
                        if (pCommand)
                        {
                            if (BOOTSTRAPPER_ACTION_UNINSTALL == pCommand->action)
                            {
                                operation = BOOTSTRAPPER_RELATED_OPERATION_REMOVE;
                            }
                            else if (BOOTSTRAPPER_ACTION_REPAIR == pCommand->action)
                            {
                                operation = BOOTSTRAPPER_RELATED_OPERATION_REPAIR;
                            }
                        }
                        break;

                    case BOOTSTRAPPER_RELATION_DETECT:
                        break;

                    default:
                        hr = E_FAIL;
                        ExitOnFailure1(hr, "Unexpected relation type encountered: %d", relationType);
                        break;
                    }

                    LogId(REPORT_STANDARD, MSG_DETECTED_RELATED_BUNDLE, pRelatedBundle->package.sczId, LoggingPerMachineToString(pRelatedBundle->package.fPerMachine), LoggingVersionToString(pRelatedBundle->qwVersion), LoggingRelatedOperationToString(operation));

                    int nResult = pUX->pUserExperience->OnDetectRelatedBundle(pRelatedBundle->package.sczId, sczTag, pRelatedBundle->package.fPerMachine, pRelatedBundle->qwVersion, operation);
                    hr = HRESULT_FROM_VIEW(nResult);
                    ExitOnRootFailure(hr, "UX aborted detect related bundle.");
                }
            }
        }
    }

LExit:
    ReleaseStr(sczBundleId);
    ReleaseStr(sczTag);
    ReleaseRegKey(hkUninstallKey);

    return hr;
}
static HRESULT LoadRelatedBundle(
    __in BURN_REGISTRATION* pRegistration,
    __in HKEY hkUninstallKey,
    __in BOOL fPerMachine,
    __in_z LPCWSTR sczBundleId,
    __out BOOTSTRAPPER_RELATION_TYPE *pRelationType,
    __out LPWSTR *psczTag
    )
{
    HRESULT hr = S_OK;
    BOOL fRelated = FALSE;
    LPWSTR sczBundleKey = NULL;
    LPWSTR sczId = NULL;
    HKEY hkBundleId = NULL;
    LPWSTR *rgsczUpgradeCodes = NULL;
    DWORD cUpgradeCodes = 0;
    LPWSTR *rgsczAddonCodes = NULL;
    DWORD cAddonCodes = 0;
    LPWSTR *rgsczDetectCodes = NULL;
    DWORD cDetectCodes = 0;
    LPWSTR *rgsczPatchCodes = NULL;
    DWORD cPatchCodes = 0;
    LPWSTR sczCachePath = NULL;
    LPWSTR sczTag = NULL;

    hr = RegOpen(hkUninstallKey, sczBundleId, KEY_READ, &hkBundleId);
    if (E_FILENOTFOUND == hr)
    {
        hr = S_OK;
    }
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
        // Both them and us must have one of the same upgrade code to cause an upgrade relation
        hr = FindMatchingStringBetweenArrays(const_cast<LPCWSTR *>(rgsczUpgradeCodes), cUpgradeCodes, const_cast<LPCWSTR *>(pRegistration->rgsczUpgradeCodes), pRegistration->cUpgradeCodes);
        if (HRESULT_FROM_WIN32(ERROR_NO_MATCH) == hr)
        {
            hr = S_OK;
        }
        else
        {
            ExitOnFailure(hr, "Failed to do array search for upgrade code match");

            fRelated = TRUE;
            *pRelationType = BOOTSTRAPPER_RELATION_UPGRADE;
            goto Finish;
        }
    }

    hr = RegReadStringArray(hkBundleId, REGISTRY_BUNDLE_ADDON_CODE, &rgsczAddonCodes, &cAddonCodes);
    if (SUCCEEDED(hr))
    {
        // Addon relation only occurs when their addon code matches one of our detect codes
        hr = FindMatchingStringBetweenArrays(const_cast<LPCWSTR *>(rgsczAddonCodes), cAddonCodes, const_cast<LPCWSTR *>(pRegistration->rgsczDetectCodes), pRegistration->cDetectCodes);
        if (HRESULT_FROM_WIN32(ERROR_NO_MATCH) == hr)
        {
            hr = S_OK;
        }
        else
        {
            ExitOnFailure(hr, "Failed to do array search for addon code match");

            fRelated = TRUE;
            *pRelationType = BOOTSTRAPPER_RELATION_ADDON;
            goto Finish;
        }
    }

    hr = RegReadStringArray(hkBundleId, REGISTRY_BUNDLE_PATCH_CODE, &rgsczPatchCodes, &cPatchCodes);
    if (SUCCEEDED(hr))
    {
        // Patch relation only occurs when their addon code matches one of our detect codes
        hr = FindMatchingStringBetweenArrays(const_cast<LPCWSTR *>(rgsczPatchCodes), cPatchCodes, const_cast<LPCWSTR *>(pRegistration->rgsczDetectCodes), pRegistration->cDetectCodes);
        if (HRESULT_FROM_WIN32(ERROR_NO_MATCH) == hr)
        {
            hr = S_OK;
        }
        else
        {
            ExitOnFailure(hr, "Failed to do array search for patch code match");

            fRelated = TRUE;
            *pRelationType = BOOTSTRAPPER_RELATION_PATCH;
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
        *pRelationType = BOOTSTRAPPER_RELATION_DETECT;
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
        *pRelationType = BOOTSTRAPPER_RELATION_DETECT;
        goto Finish;
    }

Finish:
    if (fRelated)
    {
        hr = MemEnsureArraySize(reinterpret_cast<LPVOID*>(&pRegistration->relatedBundles.rgRelatedBundles), pRegistration->relatedBundles.cRelatedBundles + 1, sizeof(BURN_RELATED_BUNDLE), 5);
        ExitOnFailure(hr, "Failed to ensure there is space for related bundles.");

        BURN_RELATED_BUNDLE* pRelatedBundle = pRegistration->relatedBundles.rgRelatedBundles + pRegistration->relatedBundles.cRelatedBundles;

        hr = StrAllocString(&sczId, sczBundleId, 0);
        ExitOnFailure(hr, "Failed to copy related bundle id.");

        pRelatedBundle->relationType = *pRelationType;

        hr = InitializeRelatedBundleFromKey(sczBundleId, hkBundleId, fPerMachine, pRelatedBundle, &sczTag);
        ExitOnFailure1(hr, "Failed to initialize package from bundle id: %ls", sczBundleId);

        pRelatedBundle->package.sczId = sczId;
        sczId = NULL;
        *psczTag = sczTag;
        sczTag = NULL;
        ++pRegistration->relatedBundles.cRelatedBundles;
    }
    else
    {
        hr = E_NOTFOUND;
    }

LExit:
    ReleaseStr(sczCachePath);
    ReleaseStr(sczId);
    ReleaseStrArray(rgsczUpgradeCodes, cUpgradeCodes);
    ReleaseStrArray(rgsczAddonCodes, cAddonCodes);
    ReleaseStrArray(rgsczDetectCodes, cDetectCodes);
    ReleaseStrArray(rgsczPatchCodes, cPatchCodes);
    ReleaseRegKey(hkBundleId);
    ReleaseStr(sczBundleKey);

    return hr;
}