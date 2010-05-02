// manifest1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "dutil.h"
#include "fileutil.h"
#include "inc\setup.h"
#include "core\inc\core.h"
#include "core\manifest.h"

#define CHECK(fact) if (!(fact)) { printf("Failed at %s(%d)\n", __FILE__, __LINE__); ExitOnFailure(hr = E_FAIL, "test failed"); }

HRESULT TestSimpleManifest(
    __in SETUP_PACKAGES* pSetupPackages
    )
{
    HRESULT hr = S_OK;
    WCHAR guid[39] = {0};
    ULARGE_INTEGER uliVersion = { 0 };

    CHECK(4 == pSetupPackages->cPackages);
    CHECK(0 == pSetupPackages->cProperties);
    CHECK(0 == pSetupPackages->cRegistrySearches);

    // first package
    SETUP_PACKAGE* pPackage = &pSetupPackages->rgPackages[0];
    CHECK(CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, pPackage->wzId, -1, L"Package1", -1));
    CHECK(160256 == pPackage->cbFileSize);
    CHECK(MSI_UI_LEVEL_DEFAULT == pPackage->Msi.uiLevel);

    ::StringFromGUID2(pPackage->Msi.guidProductCode, guid, countof(guid));
    CHECK(CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, guid, -1, L"{F1590F4F-EB9D-455C-B78D-CA7536137C4A}", -1));

    hr = FileVersionFromString(L"1.1.2.3", &uliVersion.HighPart, &uliVersion.LowPart);
    ExitOnFailure(hr, "Failed to parse version");
    CHECK(uliVersion.QuadPart == pPackage->Msi.qwVersion);

    // exe package
    pPackage = &pSetupPackages->rgPackages[1];
    CHECK(CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, pPackage->wzId, -1, L"Exe1", -1));
    CHECK(5632 == pPackage->cbFileSize);
    CHECK(TRUE == pPackage->Exe.fEnableRepair);
    CHECK(CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, pPackage->Exe.wzArguments, -1, L"-install", -1));
    CHECK(CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, pPackage->Exe.wzRepairArguments, -1, L"-repair", -1));
    CHECK(CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, pPackage->Exe.wzUninstallArguments, -1, L"-uninstall", -1));

    hr = FileVersionFromString(L"1.2", &uliVersion.HighPart, &uliVersion.LowPart);
    ExitOnFailure(hr, "Failed to parse version");
    CHECK(uliVersion.QuadPart == pPackage->Exe.qwVersion);

    // third package
    pPackage = &pSetupPackages->rgPackages[2];
    CHECK(CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, pPackage->wzId, -1, L"Package2", -1));
    CHECK(160256 == pPackage->cbFileSize);
    CHECK(MSI_UI_LEVEL_DEFAULT == pPackage->Msi.uiLevel);

    ::StringFromGUID2(pPackage->Msi.guidProductCode, guid, countof(guid));
    CHECK(CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, guid, -1, L"{2639D0F1-43E3-4C69-805F-5EABD1E6A786}", -1));

    hr = FileVersionFromString(L"2.1.2.3", &uliVersion.HighPart, &uliVersion.LowPart);
    ExitOnFailure(hr, "Failed to parse version");
    CHECK(uliVersion.QuadPart == pPackage->Msi.qwVersion);

    // fourth package
    pPackage = &pSetupPackages->rgPackages[3];
    CHECK(CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, pPackage->wzId, -1, L"Package3", -1));
    CHECK(160256 == pPackage->cbFileSize);
    CHECK(MSI_UI_LEVEL_DEFAULT == pPackage->Msi.uiLevel);

    ::StringFromGUID2(pPackage->Msi.guidProductCode, guid, countof(guid));
    CHECK(CSTR_EQUAL == ::CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, guid, -1, L"{73643F0B-5193-4220-8755-0F29145304AD}", -1));

    hr = FileVersionFromString(L"3.1.2.3", &uliVersion.HighPart, &uliVersion.LowPart);
    ExitOnFailure(hr, "Failed to parse version");
    CHECK(uliVersion.QuadPart == pPackage->Msi.qwVersion);

LExit:
    return hr;
}

int _tmain(int argc, _TCHAR* argv[])
{
    HRESULT hr = S_OK;
    SETUP_COMMAND_LINE SetupCommandLine = {0};
    SETUP_PACKAGES* pSetupPackages = NULL;

    hr = ManifestInitialize();
    ExitOnFailure(hr, "Failed to initial Burn Manifest system");

    // test the first: a simple string manifest
    const WCHAR manifest1[] =
        L"<?xml version='1.0' encoding='utf-8'?>"
        L"<BurnManifest>"
        L"   <Chain>"
        L"       <MsiPackage Id='Package1' Vital='yes' Packaging='embed' FileName='Package1.msi' FileSize='160256' ProductCode='{F1590F4F-EB9D-455C-B78D-CA7536137C4A}' ProductVersion='1.1.2.3' />"
        L"       <ExePackage Id='Exe1' Vital='no' Packaging='download' ProductVersion='1.2' InstallArguments='-install' RepairArguments='-repair' UninstallArguments='-uninstall' FileName='ExePkg.exe' FileSize='5632'>"
        L"           <DetectionCondition>1 AND 1</DetectionCondition>"
        L"       </ExePackage>"
        L"       <MsiPackage Id='Package2' Vital='yes' Packaging='embed' FileName='Package2.msi' FileSize='160256' ProductCode='{2639D0F1-43E3-4C69-805F-5EABD1E6A786}' ProductVersion='2.1.2.3' />"
        L"       <MsiPackage Id='Package3' Vital='yes' Packaging='embed' FileName='Package3.msi' FileSize='160256' ProductCode='{73643F0B-5193-4220-8755-0F29145304AD}' ProductVersion='3.1.2.3' />"
        L"   </Chain>"
        L"</BurnManifest>";
    hr = ManifestParseFromString(manifest1, &SetupCommandLine, &pSetupPackages);
    ExitOnFailure(hr, "Failed to parse simple manifest from string");

    hr = TestSimpleManifest(pSetupPackages);
    ExitOnFailure(hr, "Failed to sanity check parsing simple manifest from string");
    
    FreeSetupPackages(pSetupPackages);
    pSetupPackages = NULL;

    hr = ManifestParseFromFile(argv[1], &SetupCommandLine, &pSetupPackages);
    ExitOnFailure(hr, "Failed to parse simple manifest from file");

    hr = TestSimpleManifest(pSetupPackages);
    ExitOnFailure(hr, "Failed to sanity check parsing simple manifest from file");

LExit:
    FreeSetupPackages(pSetupPackages);
    ManifestUninitialize();

    if (FAILED(hr))
    {
        printf("test failed: HRESULT=0x%x", hr);
    }
    else
    {
        puts("test passed");
    }

    return FAILED(hr);
}

