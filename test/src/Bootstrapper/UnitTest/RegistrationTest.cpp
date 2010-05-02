//-------------------------------------------------------------------------------------------------
// <copyright file="RegistrationTest.cpp" company="Microsoft">
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
//    Unit tests for Burn registration.
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"


#define ROOT_PATH L"SOFTWARE\\Microsoft\\WiX_Burn_UnitTest"
#define HKLM_PATH L"SOFTWARE\\Microsoft\\WiX_Burn_UnitTest\\HKLM"
#define HKCU_PATH L"SOFTWARE\\Microsoft\\WiX_Burn_UnitTest\\HKCU"
#define REGISTRY_UNINSTALL_KEY L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall"
#define REGISTRY_RUN_KEY L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"

#define TEST_UNINSTALL_KEY L"HKEY_CURRENT_USER\\" HKCU_PATH L"\\" REGISTRY_UNINSTALL_KEY L"\\{D54F896D-1952-43e6-9C67-B5652240618C}"
#define TEST_RUN_KEY L"HKEY_CURRENT_USER\\" HKCU_PATH L"\\" REGISTRY_RUN_KEY


static LSTATUS APIENTRY RegistrationTest_RegCreateKeyExW(
    __in HKEY hKey,
    __in LPCWSTR lpSubKey,
    __reserved DWORD Reserved,
    __in_opt LPWSTR lpClass,
    __in DWORD dwOptions,
    __in REGSAM samDesired,
    __in_opt CONST LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    __out PHKEY phkResult,
    __out_opt LPDWORD lpdwDisposition
    );
static LSTATUS APIENTRY RegistrationTest_RegOpenKeyExW(
    __in HKEY hKey,
    __in_opt LPCWSTR lpSubKey,
    __reserved DWORD ulOptions,
    __in REGSAM samDesired,
    __out PHKEY phkResult
    );
static LSTATUS APIENTRY RegistrationTest_RegDeleteKeyW(
    __in HKEY hKey,
    __in LPCWSTR lpSubKey
    );


using namespace System;
using namespace System::IO;
using namespace Microsoft::VisualStudio::TestTools::UnitTesting;
using namespace Microsoft::Win32;


namespace MS
{
namespace Tools
{
namespace WindowsInstallerXml
{
namespace Test
{
namespace Bootstrapper
{
    [TestClass]
    public ref class RegistrationTest
    {
    private:
        TestContext^ testContextInstance;

    public: 
        /// <summary>
        ///Gets or sets the test context which provides
        ///information about and functionality for the current test run.
        ///</summary>
        property Microsoft::VisualStudio::TestTools::UnitTesting::TestContext^ TestContext
        {
            Microsoft::VisualStudio::TestTools::UnitTesting::TestContext^ get()
            {
                return testContextInstance;
            }
            System::Void set(Microsoft::VisualStudio::TestTools::UnitTesting::TestContext^ value)
            {
                testContextInstance = value;
            }
        };

        #pragma region Additional test attributes
        //
        //You can use the following additional attributes as you write your tests:
        //
        //Use ClassInitialize to run code before running the first test in the class
        [ClassInitialize()]
        static void MyClassInitialize(Microsoft::VisualStudio::TestTools::UnitTesting::TestContext^ testContext)
        {
            HRESULT hr = XmlInitialize();
            TestThrowOnFailure(hr, L"Failed to initialize XML support.");
        }

        //Use ClassCleanup to run code after all tests in a class have run
        [ClassCleanup()]
        static void MyClassCleanup()
        {
            XmlUninitialize();
        }

        //Use TestInitialize to run code before running each test
        [TestInitialize()]
        void MyTestInitialize()
        {
            PlatformInitialize();
        };

        //Use TestCleanup to run code after each test has run
        //[TestCleanup()]
        //void MyTestCleanup() {};
        //
        #pragma endregion

        [TestMethod]
        void RegisterBasicTest()
        {
            HRESULT hr = S_OK;
            IXMLDOMElement* pixeBundle = NULL;
            BURN_USER_EXPERIENCE userExperience = { };
            BURN_REGISTRATION registration = { };
            String^ cacheDirectory = Path::Combine(Environment::GetFolderPath(Environment::SpecialFolder::LocalApplicationData), "Apps");
			cacheDirectory = Path::Combine(cacheDirectory, "Cache");
			cacheDirectory = Path::Combine(cacheDirectory, gcnew String(L"{D54F896D-1952-43e6-9C67-B5652240618C}"));
			try
            {
                // set mock API's
                vpfnRegCreateKeyExW = RegistrationTest_RegCreateKeyExW;
                vpfnRegOpenKeyExW = RegistrationTest_RegOpenKeyExW;
                vpfnRegDeleteKeyW = RegistrationTest_RegDeleteKeyW;

                Registry::CurrentUser->CreateSubKey(gcnew String(HKCU_PATH));

                LPCWSTR wzDocument =
                    L"<Bundle>"
                    L"    <Ux UxDllPayloadId='ux.dll'>"
                    L"        <Payload Id='ux.dll' FilePath='ux.dll' Packaging='embedded' SourcePath='ux.dll' />"
                    L"    </Ux>"
                    L"    <Registration Id='{D54F896D-1952-43e6-9C67-B5652240618C}' ExecutableName='setup.exe' PerMachine='no' Family='{E56901C1-8FCD-4491-B44C-850CB60F511E}'/>"
                    L"</Bundle>";

                // load XML document
                LoadBundleXmlHelper(wzDocument, &pixeBundle);

                hr = UserExperienceParseFromXml(&userExperience, pixeBundle);
                TestThrowOnFailure(hr, L"Failed to parse UX from XML.");

                hr = RegistrationParseFromXml(&registration, pixeBundle);
                TestThrowOnFailure(hr, L"Failed to parse registration from XML.");

                hr = RegistrationSetPaths(&registration);
                TestThrowOnFailure(hr, L"Failed to set registration paths.");

                // write registration
                hr = RegistrationSessionBegin(&registration, &userExperience, BURN_ACTION_INSTALL, 0, FALSE);
                TestThrowOnFailure(hr, L"Failed to register bundle.");

                // verify that registration was created
                Assert::IsTrue(Directory::Exists(cacheDirectory));
                Assert::IsTrue(File::Exists(Path::Combine(cacheDirectory, gcnew String(L"setup.exe"))));

                Assert::AreEqual(Int32(BURN_RESUME_MODE_ACTIVE), Registry::GetValue(gcnew String(TEST_UNINSTALL_KEY), gcnew String(L"Resume"), nullptr));
                Assert::AreEqual(Path::Combine(cacheDirectory, gcnew String(L"setup.exe")), Registry::GetValue(gcnew String(TEST_RUN_KEY), gcnew String(L"{D54F896D-1952-43e6-9C67-B5652240618C}"), nullptr));

                // end session
                hr = RegistrationSessionEnd(&registration, BURN_ACTION_INSTALL, FALSE, FALSE, FALSE, NULL);
                TestThrowOnFailure(hr, L"Failed to unregister bundle.");

                // verify that registration was removed
                Assert::IsFalse(Directory::Exists(cacheDirectory));

                Assert::AreEqual(nullptr, Registry::GetValue(gcnew String(TEST_UNINSTALL_KEY), gcnew String(L"Resume"), nullptr));
                Assert::AreEqual(nullptr, Registry::GetValue(gcnew String(TEST_RUN_KEY), gcnew String(L"{D54F896D-1952-43e6-9C67-B5652240618C}"), nullptr));
            }
            finally
            {
                ReleaseObject(pixeBundle);
                UserExperienceUninitialize(&userExperience);
                RegistrationUninitialize(&registration);

                Registry::CurrentUser->DeleteSubKeyTree(gcnew String(ROOT_PATH));
                if (Directory::Exists(cacheDirectory))
                {
                    Directory::Delete(cacheDirectory, true);
                }
            }
        }

        [TestMethod]
        void RegisterArpMinimumTest()
        {
            HRESULT hr = S_OK;
            IXMLDOMElement* pixeBundle = NULL;
            BURN_USER_EXPERIENCE userExperience = { };
            BURN_REGISTRATION registration = { };
            String^ cacheDirectory = Path::Combine(Environment::GetFolderPath(Environment::SpecialFolder::LocalApplicationData), "Apps");
			cacheDirectory = Path::Combine(cacheDirectory, "Cache");
			cacheDirectory = Path::Combine(cacheDirectory, gcnew String(L"{D54F896D-1952-43e6-9C67-B5652240618C}"));
            try
            {
                // set mock API's
                vpfnRegCreateKeyExW = RegistrationTest_RegCreateKeyExW;
                vpfnRegOpenKeyExW = RegistrationTest_RegOpenKeyExW;
                vpfnRegDeleteKeyW = RegistrationTest_RegDeleteKeyW;

                Registry::CurrentUser->CreateSubKey(gcnew String(HKCU_PATH));

                LPCWSTR wzDocument =
                    L"<Bundle>"
                    L"    <Ux UxDllPayloadId='ux.dll'>"
                    L"        <Payload Id='ux.dll' FilePath='ux.dll' Packaging='embedded' SourcePath='ux.dll' />"
                    L"    </Ux>"
                    L"    <Registration Id='{D54F896D-1952-43e6-9C67-B5652240618C}' ExecutableName='setup.exe' PerMachine='no' Family='{E56901C1-8FCD-4491-B44C-850CB60F511E}'>"
                    L"        <Arp DisplayName='Product1' DisplayVersion='1.0.0.0'/>"
                    L"    </Registration>"
                    L"</Bundle>";

                // load XML document
                LoadBundleXmlHelper(wzDocument, &pixeBundle);

                hr = UserExperienceParseFromXml(&userExperience, pixeBundle);
                TestThrowOnFailure(hr, L"Failed to parse UX from XML.");

                hr = RegistrationParseFromXml(&registration, pixeBundle);
                TestThrowOnFailure(hr, L"Failed to parse registration from XML.");

                hr = RegistrationSetPaths(&registration);
                TestThrowOnFailure(hr, L"Failed to set registration paths.");

                //
                // install
                //

                // write registration
                hr = RegistrationSessionBegin(&registration, &userExperience, BURN_ACTION_INSTALL, 0, FALSE);
                TestThrowOnFailure(hr, L"Failed to register bundle.");

                // verify that registration was created
                Assert::AreEqual(Int32(BURN_RESUME_MODE_ACTIVE), Registry::GetValue(gcnew String(TEST_UNINSTALL_KEY), gcnew String(L"Resume"), nullptr));
                Assert::AreEqual(Path::Combine(cacheDirectory, gcnew String(L"setup.exe")), Registry::GetValue(gcnew String(TEST_RUN_KEY), gcnew String(L"{D54F896D-1952-43e6-9C67-B5652240618C}"), nullptr));

                // delete registration
                hr = RegistrationSessionEnd(&registration, BURN_ACTION_INSTALL, FALSE, FALSE, FALSE, NULL);
                TestThrowOnFailure(hr, L"Failed to unregister bundle.");

                // verify that registration was updated
                Assert::AreEqual(Int32(BURN_RESUME_MODE_ARP), Registry::GetValue(gcnew String(TEST_UNINSTALL_KEY), gcnew String(L"Resume"), nullptr));
                Assert::AreEqual(nullptr, Registry::GetValue(gcnew String(TEST_RUN_KEY), gcnew String(L"{D54F896D-1952-43e6-9C67-B5652240618C}"), nullptr));

                //
                // uninstall
                //

                // write registration
                hr = RegistrationSessionBegin(&registration, &userExperience, BURN_ACTION_UNINSTALL, 0, FALSE);
                TestThrowOnFailure(hr, L"Failed to register bundle.");

                // verify that registration was updated
                Assert::AreEqual(Int32(BURN_RESUME_MODE_ACTIVE), Registry::GetValue(gcnew String(TEST_UNINSTALL_KEY), gcnew String(L"Resume"), nullptr));
                Assert::AreEqual(Path::Combine(cacheDirectory, gcnew String(L"setup.exe")), Registry::GetValue(gcnew String(TEST_RUN_KEY), gcnew String(L"{D54F896D-1952-43e6-9C67-B5652240618C}"), nullptr));

                // delete registration
                hr = RegistrationSessionEnd(&registration, BURN_ACTION_UNINSTALL, FALSE, FALSE, FALSE, NULL);
                TestThrowOnFailure(hr, L"Failed to unregister bundle.");

                // verify that registration was removed
                Assert::AreEqual(nullptr, Registry::GetValue(gcnew String(TEST_UNINSTALL_KEY), gcnew String(L"Resume"), nullptr));
                Assert::AreEqual(nullptr, Registry::GetValue(gcnew String(TEST_RUN_KEY), gcnew String(L"{D54F896D-1952-43e6-9C67-B5652240618C}"), nullptr));
            }
            finally
            {
                ReleaseObject(pixeBundle);
                UserExperienceUninitialize(&userExperience);
                RegistrationUninitialize(&registration);

                Registry::CurrentUser->DeleteSubKeyTree(gcnew String(ROOT_PATH));
                if (Directory::Exists(cacheDirectory))
                {
                    Directory::Delete(cacheDirectory, true);
                }
            }
        }

        [TestMethod]
        void RegisterArpFullTest()
        {
            HRESULT hr = S_OK;
            IXMLDOMElement* pixeBundle = NULL;
            BURN_USER_EXPERIENCE userExperience = { };
            BURN_REGISTRATION registration = { };
            String^ cacheDirectory = Path::Combine(Environment::GetFolderPath(Environment::SpecialFolder::LocalApplicationData), "Apps");
			cacheDirectory = Path::Combine(cacheDirectory, "Cache");
			cacheDirectory = Path::Combine(cacheDirectory, gcnew String(L"{D54F896D-1952-43e6-9C67-B5652240618C}"));
            try
            {
                // set mock API's
                vpfnRegCreateKeyExW = RegistrationTest_RegCreateKeyExW;
                vpfnRegOpenKeyExW = RegistrationTest_RegOpenKeyExW;
                vpfnRegDeleteKeyW = RegistrationTest_RegDeleteKeyW;

                Registry::CurrentUser->CreateSubKey(gcnew String(HKCU_PATH));

                LPCWSTR wzDocument =
                    L"<Bundle>"
                    L"    <Ux UxDllPayloadId='ux.dll'>"
                    L"        <Payload Id='ux.dll' FilePath='ux.dll' Packaging='embedded' SourcePath='ux.dll' />"
                    L"    </Ux>"
                    L"    <Registration Id='{D54F896D-1952-43e6-9C67-B5652240618C}' ExecutableName='setup.exe' PerMachine='no' Family='{E56901C1-8FCD-4491-B44C-850CB60F511E}'>"
                    L"        <Arp DisplayName='DisplayName1' DisplayVersion='1.2.3.4' Publisher='Publisher1' HelpLink='http://www.microsoft.com/help'"
                    L"             HelpTelephone='555-555-5555' AboutUrl='http://www.microsoft.com/about' UpdateUrl='http://www.microsoft.com/update'"
                    L"             Comments='Comments1' Contact='Contact1' NoModify='yes' NoRepair='yes' NoRemove='yes' />"
                    L"    </Registration>"
                    L"</Bundle>";

                // load XML document
                LoadBundleXmlHelper(wzDocument, &pixeBundle);

                hr = UserExperienceParseFromXml(&userExperience, pixeBundle);
                TestThrowOnFailure(hr, L"Failed to parse UX from XML.");

                hr = RegistrationParseFromXml(&registration, pixeBundle);
                TestThrowOnFailure(hr, L"Failed to parse registration from XML.");

                hr = RegistrationSetPaths(&registration);
                TestThrowOnFailure(hr, L"Failed to set registration paths.");

                //
                // install
                //

                // write registration
                hr = RegistrationSessionBegin(&registration, &userExperience, BURN_ACTION_INSTALL, 0, FALSE);
                TestThrowOnFailure(hr, L"Failed to register bundle.");

                // verify that registration was created
                Assert::AreEqual(Int32(BURN_RESUME_MODE_ACTIVE), Registry::GetValue(gcnew String(TEST_UNINSTALL_KEY), gcnew String(L"Resume"), nullptr));
                Assert::AreEqual(Path::Combine(cacheDirectory, gcnew String(L"setup.exe")), Registry::GetValue(gcnew String(TEST_RUN_KEY), gcnew String(L"{D54F896D-1952-43e6-9C67-B5652240618C}"), nullptr));

                // delete registration
                hr = RegistrationSessionEnd(&registration, BURN_ACTION_INSTALL, FALSE, FALSE, FALSE, NULL);
                TestThrowOnFailure(hr, L"Failed to unregister bundle.");

                // verify that registration was updated
                Assert::AreEqual(Int32(BURN_RESUME_MODE_ARP), Registry::GetValue(gcnew String(TEST_UNINSTALL_KEY), gcnew String(L"Resume"), nullptr));
                Assert::AreEqual(nullptr, Registry::GetValue(gcnew String(TEST_RUN_KEY), gcnew String(L"{D54F896D-1952-43e6-9C67-B5652240618C}"), nullptr));

                Assert::AreEqual(gcnew String(L"DisplayName1"), Registry::GetValue(gcnew String(TEST_UNINSTALL_KEY), gcnew String(L"DisplayName"), nullptr));
                Assert::AreEqual(gcnew String(L"1.2.3.4"), Registry::GetValue(gcnew String(TEST_UNINSTALL_KEY), gcnew String(L"DisplayVersion"), nullptr));
                Assert::AreEqual(gcnew String(L"Publisher1"), Registry::GetValue(gcnew String(TEST_UNINSTALL_KEY), gcnew String(L"Publisher"), nullptr));
                Assert::AreEqual(gcnew String(L"http://www.microsoft.com/help"), Registry::GetValue(gcnew String(TEST_UNINSTALL_KEY), gcnew String(L"HelpLink"), nullptr));
                Assert::AreEqual(gcnew String(L"555-555-5555"), Registry::GetValue(gcnew String(TEST_UNINSTALL_KEY), gcnew String(L"HelpTelephone"), nullptr));
                Assert::AreEqual(gcnew String(L"http://www.microsoft.com/about"), Registry::GetValue(gcnew String(TEST_UNINSTALL_KEY), gcnew String(L"URLInfoAbout"), nullptr));
                Assert::AreEqual(gcnew String(L"http://www.microsoft.com/update"), Registry::GetValue(gcnew String(TEST_UNINSTALL_KEY), gcnew String(L"URLUpdateInfo"), nullptr));
                Assert::AreEqual(gcnew String(L"Comments1"), Registry::GetValue(gcnew String(TEST_UNINSTALL_KEY), gcnew String(L"Comments"), nullptr));
                Assert::AreEqual(gcnew String(L"Contact1"), Registry::GetValue(gcnew String(TEST_UNINSTALL_KEY), gcnew String(L"Contact"), nullptr));
                Assert::AreEqual(1, Registry::GetValue(gcnew String(TEST_UNINSTALL_KEY), gcnew String(L"NoModify"), nullptr));
                Assert::AreEqual(1, Registry::GetValue(gcnew String(TEST_UNINSTALL_KEY), gcnew String(L"NoRepair"), nullptr));
                Assert::AreEqual(1, Registry::GetValue(gcnew String(TEST_UNINSTALL_KEY), gcnew String(L"NoRemove"), nullptr));

                //
                // uninstall
                //

                // write registration
                hr = RegistrationSessionBegin(&registration, &userExperience, BURN_ACTION_UNINSTALL, 0, FALSE);
                TestThrowOnFailure(hr, L"Failed to register bundle.");

                // verify that registration was updated
                Assert::AreEqual(Int32(BURN_RESUME_MODE_ACTIVE), Registry::GetValue(gcnew String(TEST_UNINSTALL_KEY), gcnew String(L"Resume"), nullptr));
                Assert::AreEqual(Path::Combine(cacheDirectory, gcnew String(L"setup.exe")), Registry::GetValue(gcnew String(TEST_RUN_KEY), gcnew String(L"{D54F896D-1952-43e6-9C67-B5652240618C}"), nullptr));

                // delete registration
                hr = RegistrationSessionEnd(&registration, BURN_ACTION_UNINSTALL, FALSE, FALSE, FALSE, NULL);
                TestThrowOnFailure(hr, L"Failed to unregister bundle.");

                // verify that registration was removed
                Assert::AreEqual(nullptr, Registry::GetValue(gcnew String(TEST_UNINSTALL_KEY), gcnew String(L"Resume"), nullptr));
                Assert::AreEqual(nullptr, Registry::GetValue(gcnew String(TEST_RUN_KEY), gcnew String(L"{D54F896D-1952-43e6-9C67-B5652240618C}"), nullptr));
            }
            finally
            {
                ReleaseObject(pixeBundle);
                UserExperienceUninitialize(&userExperience);
                RegistrationUninitialize(&registration);

                Registry::CurrentUser->DeleteSubKeyTree(gcnew String(ROOT_PATH));
                if (Directory::Exists(cacheDirectory))
                {
                    Directory::Delete(cacheDirectory, true);
                }
            }
        }

        [TestMethod]
        void ResumeTest()
        {
            HRESULT hr = S_OK;
            IXMLDOMElement* pixeBundle = NULL;
            BURN_USER_EXPERIENCE userExperience = { };
            BURN_REGISTRATION registration = { };
            BYTE rgbData[256] = { };
            BURN_RESUME_TYPE resumeType = BURN_RESUME_TYPE_NONE;
            BYTE* pbBuffer = NULL;
            DWORD cbBuffer = 0;
            String^ cacheDirectory = Path::Combine(Environment::GetFolderPath(Environment::SpecialFolder::LocalApplicationData), "Apps");
			cacheDirectory = Path::Combine(cacheDirectory, "Cache");
			cacheDirectory = Path::Combine(cacheDirectory, gcnew String(L"{D54F896D-1952-43e6-9C67-B5652240618C}"));
            try
            {
                for (DWORD i = 0; i < 256; ++i)
                {
                    rgbData[i] = (BYTE)i;
                }

                // set mock API's
                vpfnRegCreateKeyExW = RegistrationTest_RegCreateKeyExW;
                vpfnRegOpenKeyExW = RegistrationTest_RegOpenKeyExW;
                vpfnRegDeleteKeyW = RegistrationTest_RegDeleteKeyW;

                Registry::CurrentUser->CreateSubKey(gcnew String(HKCU_PATH));

                LPCWSTR wzDocument =
                    L"<Bundle>"
                    L"    <Ux UxDllPayloadId='ux.dll'>"
                    L"        <Payload Id='ux.dll' FilePath='ux.dll' Packaging='embedded' SourcePath='ux.dll' />"
                    L"    </Ux>"
                    L"    <Registration Id='{D54F896D-1952-43e6-9C67-B5652240618C}' ExecutableName='setup.exe' PerMachine='no' Family='{E56901C1-8FCD-4491-B44C-850CB60F511E}'/>"
                    L"</Bundle>";

                // load XML document
                LoadBundleXmlHelper(wzDocument, &pixeBundle);

                hr = UserExperienceParseFromXml(&userExperience, pixeBundle);
                TestThrowOnFailure(hr, L"Failed to parse UX from XML.");

                hr = RegistrationParseFromXml(&registration, pixeBundle);
                TestThrowOnFailure(hr, L"Failed to parse registration from XML.");

                hr = RegistrationSetPaths(&registration);
                TestThrowOnFailure(hr, L"Failed to set registration paths.");

                // read resume type before session
                hr = RegistrationDetectResumeType(&registration, FALSE, &resumeType);
                TestThrowOnFailure(hr, L"Failed to read resume type.");

                Assert::AreEqual((int)BURN_RESUME_TYPE_NONE, (int)resumeType);

                // begin session
                hr = RegistrationSessionBegin(&registration, &userExperience, BURN_ACTION_INSTALL, 0, FALSE);
                TestThrowOnFailure(hr, L"Failed to register bundle.");

                hr = RegistrationSaveState(&registration, rgbData, sizeof(rgbData));
                TestThrowOnFailure(hr, L"Failed to save state.");

                // read interrupted resume type
                hr = RegistrationDetectResumeType(&registration, FALSE, &resumeType);
                TestThrowOnFailure(hr, L"Failed to read interrupted resume type.");

                Assert::AreEqual((int)BURN_RESUME_TYPE_UNEXPECTED, (int)resumeType);

                // suspend session
                hr = RegistrationSessionSuspend(&registration, BURN_ACTION_INSTALL, FALSE, FALSE);
                TestThrowOnFailure(hr, L"Failed to suspend session.");

                // verify that run key was removed
                Assert::AreEqual(nullptr, Registry::GetValue(gcnew String(TEST_RUN_KEY), gcnew String(L"{D54F896D-1952-43e6-9C67-B5652240618C}"), nullptr));

                // read suspend resume type
                hr = RegistrationDetectResumeType(&registration, FALSE, &resumeType);
                TestThrowOnFailure(hr, L"Failed to read suspend resume type.");

                Assert::AreEqual((int)BURN_RESUME_TYPE_SUSPEND, (int)resumeType);

                // read state back
                hr = RegistrationLoadState(&registration, &pbBuffer, &cbBuffer);
                TestThrowOnFailure(hr, L"Failed to load state.");

                Assert::AreEqual(sizeof(rgbData), cbBuffer);
                Assert::IsTrue(0 == memcmp(pbBuffer, rgbData, sizeof(rgbData)));

                // write active resume mode
                hr = RegistrationSessionResume(&registration, BURN_ACTION_INSTALL, FALSE);
                TestThrowOnFailure(hr, L"Failed to write active resume mode.");

                // verify that run key was put back
                Assert::AreNotEqual(nullptr, Registry::GetValue(gcnew String(TEST_RUN_KEY), gcnew String(L"{D54F896D-1952-43e6-9C67-B5652240618C}"), nullptr));

                // end session
                hr = RegistrationSessionEnd(&registration, BURN_ACTION_INSTALL, FALSE, FALSE, FALSE, NULL);
                TestThrowOnFailure(hr, L"Failed to unregister bundle.");

                // read resume type after session
                hr = RegistrationDetectResumeType(&registration, FALSE, &resumeType);
                TestThrowOnFailure(hr, L"Failed to read resume type.");

                Assert::AreEqual((int)BURN_RESUME_TYPE_NONE, (int)resumeType);
            }
            finally
            {
                ReleaseObject(pixeBundle);
                UserExperienceUninitialize(&userExperience);
                RegistrationUninitialize(&registration);

                Registry::CurrentUser->DeleteSubKeyTree(gcnew String(ROOT_PATH));
                if (Directory::Exists(cacheDirectory))
                {
                    Directory::Delete(cacheDirectory, true);
                }
            }
        }

    //BURN_RESUME_TYPE_NONE,
    //BURN_RESUME_TYPE_INVALID,        // resume information is present but invalid
    //BURN_RESUME_TYPE_UNEXPECTED,     // relaunched after an unexpected interruption
    //BURN_RESUME_TYPE_REBOOT_PENDING, // reboot has not taken place yet
    //BURN_RESUME_TYPE_REBOOT,         // relaunched after reboot
    //BURN_RESUME_TYPE_SUSPEND,        // relaunched after suspend
    //BURN_RESUME_TYPE_ARP,            // launched from ARP
    };
}
}
}
}
}


static LSTATUS APIENTRY RegistrationTest_RegCreateKeyExW(
    __in HKEY hKey,
    __in LPCWSTR lpSubKey,
    __reserved DWORD Reserved,
    __in_opt LPWSTR lpClass,
    __in DWORD dwOptions,
    __in REGSAM samDesired,
    __in_opt CONST LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    __out PHKEY phkResult,
    __out_opt LPDWORD lpdwDisposition
    )
{
    LSTATUS ls = ERROR_SUCCESS;
    LPCWSTR wzRoot = NULL;
    HKEY hkRoot = NULL;
    HKEY hk = NULL;

    if (HKEY_LOCAL_MACHINE == hKey)
    {
        wzRoot = HKLM_PATH;
    }
    else if (HKEY_CURRENT_USER == hKey)
    {
        wzRoot = HKCU_PATH;
    }
    else
    {
        hkRoot = hKey;
    }

    if (wzRoot)
    {
        ls = ::RegOpenKeyExW(HKEY_CURRENT_USER, wzRoot, 0, KEY_WRITE, &hkRoot);
        if (ERROR_SUCCESS != ls)
        {
            ExitFunction();
        }
    }

    ls = ::RegCreateKeyExW(hkRoot, lpSubKey, Reserved, lpClass, dwOptions, samDesired, lpSecurityAttributes, phkResult, lpdwDisposition);

LExit:
    ReleaseRegKey(hkRoot);

    return ls;
}

static LSTATUS APIENTRY RegistrationTest_RegOpenKeyExW(
    __in HKEY hKey,
    __in_opt LPCWSTR lpSubKey,
    __reserved DWORD ulOptions,
    __in REGSAM samDesired,
    __out PHKEY phkResult
    )
{
    LSTATUS ls = ERROR_SUCCESS;
    LPCWSTR wzRoot = NULL;
    HKEY hkRoot = NULL;
    HKEY hk = NULL;

    if (HKEY_LOCAL_MACHINE == hKey)
    {
        wzRoot = HKLM_PATH;
    }
    else if (HKEY_CURRENT_USER == hKey)
    {
        wzRoot = HKCU_PATH;
    }
    else
    {
        hkRoot = hKey;
    }

    if (wzRoot)
    {
        ls = ::RegOpenKeyExW(HKEY_CURRENT_USER, wzRoot, 0, KEY_WRITE, &hkRoot);
        if (ERROR_SUCCESS != ls)
        {
            ExitFunction();
        }
    }

    ls = ::RegOpenKeyExW(hkRoot, lpSubKey, ulOptions, samDesired, phkResult);

LExit:
    ReleaseRegKey(hkRoot);

    return ls;
}

static LSTATUS APIENTRY RegistrationTest_RegDeleteKeyW(
    __in HKEY hKey,
    __in LPCWSTR lpSubKey
    )
{
    LSTATUS ls = ERROR_SUCCESS;
    LPCWSTR wzRoot = NULL;
    HKEY hkRoot = NULL;
    HKEY hk = NULL;

    if (HKEY_LOCAL_MACHINE == hKey)
    {
        wzRoot = HKLM_PATH;
    }
    else if (HKEY_CURRENT_USER == hKey)
    {
        wzRoot = HKCU_PATH;
    }
    else
    {
        hkRoot = hKey;
    }

    if (wzRoot)
    {
        ls = ::RegOpenKeyExW(HKEY_CURRENT_USER, wzRoot, 0, KEY_WRITE, &hkRoot);
        if (ERROR_SUCCESS != ls)
        {
            ExitFunction();
        }
    }

    ls = ::RegDeleteKeyW(hkRoot, lpSubKey);

LExit:
    ReleaseRegKey(hkRoot);

    return ls;
}
