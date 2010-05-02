//-------------------------------------------------------------------------------------------------
// <copyright file="VariableTest.cpp" company="Microsoft">
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
//    Variable management unit tests for Burn.
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"
#undef GetTempPath
#undef GetEnvironmentVariable


using namespace System;
using namespace Microsoft::VisualStudio::TestTools::UnitTesting;


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
    public ref class VariableTest
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
        void VariablesBasicTest()
        {
            HRESULT hr = S_OK;
            BURN_VARIABLES variables = { };
            try
            {
                // set variables
                VariableSetStringHelper(&variables, L"PROP1", L"VAL1");
                VariableSetNumericHelper(&variables, L"PROP2", 2);
                VariableSetStringHelper(&variables, L"PROP5", L"VAL5");
                VariableSetStringHelper(&variables, L"PROP3", L"VAL3");
                VariableSetStringHelper(&variables, L"PROP4", L"VAL4");
                VariableSetStringHelper(&variables, L"PROP6", L"VAL6");
                VariableSetStringHelper(&variables, L"PROP7", L"7");
                VariableSetVersionHelper(&variables, L"PROP8", MAKEQWORDVERSION(1,1,0,0));

                // get and verify variable values
                Assert::AreEqual(gcnew String(L"VAL1"), VariableGetStringHelper(&variables, L"PROP1"));
                Assert::AreEqual(2ll, VariableGetNumericHelper(&variables, L"PROP2"));
                Assert::AreEqual(gcnew String(L"2"), VariableGetStringHelper(&variables, L"PROP2"));
                Assert::AreEqual(gcnew String(L"VAL3"), VariableGetStringHelper(&variables, L"PROP3"));
                Assert::AreEqual(gcnew String(L"VAL4"), VariableGetStringHelper(&variables, L"PROP4"));
                Assert::AreEqual(gcnew String(L"VAL5"), VariableGetStringHelper(&variables, L"PROP5"));
                Assert::AreEqual(gcnew String(L"VAL6"), VariableGetStringHelper(&variables, L"PROP6"));
                Assert::AreEqual(7ll, VariableGetNumericHelper(&variables, L"PROP7"));
                Assert::AreEqual(MAKEQWORDVERSION(1,1,0,0), VariableGetVersionHelper(&variables, L"PROP8"));
                Assert::AreEqual(gcnew String(L"1.1.0.0"), VariableGetStringHelper(&variables, L"PROP8"));
            }
            finally
            {
                VariablesUninitialize(&variables);
            }
        }

        [TestMethod]
        void VariablesParseXmlTest()
        {
            HRESULT hr = S_OK;
            IXMLDOMElement* pixeBundle = NULL;
            BURN_VARIABLES variables = { };
            try
            {
                LPCWSTR wzDocument =
                    L"<Bundle>"
                    L"    <Variable Id='Var1' Type='numeric' Value='1' />"
                    L"    <Variable Id='Var2' Type='string' Value='String value.' />"
                    L"    <Variable Id='Var3' Type='version' Value='1.2.3.4' />"
                    L"</Bundle>";

                // load XML document
                LoadBundleXmlHelper(wzDocument, &pixeBundle);

                hr = VariablesParseFromXml(&variables, pixeBundle);
                TestThrowOnFailure(hr, L"Failed to parse searches from XML.");

                // get and verify variable values
                Assert::AreEqual((int)BURN_VARIANT_TYPE_NUMERIC, VariableGetTypeHelper(&variables, L"Var1"));
                Assert::AreEqual((int)BURN_VARIANT_TYPE_STRING, VariableGetTypeHelper(&variables, L"Var2"));
                Assert::AreEqual((int)BURN_VARIANT_TYPE_VERSION, VariableGetTypeHelper(&variables, L"Var3"));

                Assert::AreEqual(1ll, VariableGetNumericHelper(&variables, L"Var1"));
                Assert::AreEqual(gcnew String(L"String value."), VariableGetStringHelper(&variables, L"Var2"));
                Assert::AreEqual(MAKEQWORDVERSION(1,2,3,4), VariableGetVersionHelper(&variables, L"Var3"));
            }
            finally
            {
                ReleaseObject(pixeBundle);
                VariablesUninitialize(&variables);
            }
        }

        [TestMethod]
        void VariablesFormatTest()
        {
            HRESULT hr = S_OK;
            BURN_VARIABLES variables = { };
            LPWSTR scz = NULL;
            DWORD cch = 0;
            try
            {
                // set variables
                VariableSetStringHelper(&variables, L"PROP1", L"VAL1");
                VariableSetStringHelper(&variables, L"PROP2", L"VAL2");
                VariableSetNumericHelper(&variables, L"PROP3", 3);

                // test string formatting
                Assert::AreEqual(gcnew String(L"NOPROP"), VariableFormatStringHelper(&variables, L"NOPROP"));
                Assert::AreEqual(gcnew String(L"VAL1"), VariableFormatStringHelper(&variables, L"[PROP1]"));
                Assert::AreEqual(gcnew String(L" VAL1 "), VariableFormatStringHelper(&variables, L" [PROP1] "));
                Assert::AreEqual(gcnew String(L"PRE VAL1"), VariableFormatStringHelper(&variables, L"PRE [PROP1]"));
                Assert::AreEqual(gcnew String(L"VAL1 POST"), VariableFormatStringHelper(&variables, L"[PROP1] POST"));
                Assert::AreEqual(gcnew String(L"PRE VAL1 POST"), VariableFormatStringHelper(&variables, L"PRE [PROP1] POST"));
                Assert::AreEqual(gcnew String(L"VAL1 MID VAL2"), VariableFormatStringHelper(&variables, L"[PROP1] MID [PROP2]"));
                Assert::AreEqual(gcnew String(L""), VariableFormatStringHelper(&variables, L"[NONE]"));
                Assert::AreEqual(gcnew String(L""), VariableFormatStringHelper(&variables, L"[prop1]"));
                Assert::AreEqual(gcnew String(L"["), VariableFormatStringHelper(&variables, L"[\\[]"));
                Assert::AreEqual(gcnew String(L"]"), VariableFormatStringHelper(&variables, L"[\\]]"));
                Assert::AreEqual(gcnew String(L"[]"), VariableFormatStringHelper(&variables, L"[]"));
                Assert::AreEqual(gcnew String(L"[NONE"), VariableFormatStringHelper(&variables, L"[NONE"));
                Assert::AreEqual(gcnew String(L"VAL2"), VariableGetFormattedHelper(&variables, L"PROP2"));
                Assert::AreEqual(gcnew String(L"3"), VariableGetFormattedHelper(&variables, L"PROP3"));

                hr = VariableFormatString(&variables, L"PRE [PROP1] POST", &scz, &cch);
                TestThrowOnFailure(hr, L"Failed to format string");

                Assert::AreEqual((DWORD)lstrlenW(scz), cch);

                hr = VariableFormatString(&variables, L"PRE [PROP1] POST", NULL, &cch);
                TestThrowOnFailure(hr, L"Failed to format string");

                Assert::AreEqual((DWORD)lstrlenW(scz), cch);
            }
            finally
            {
                VariablesUninitialize(&variables);
                ReleaseStr(scz);
            }
        }

        [TestMethod]
        void VariablesEscapeTest()
        {
            // test string escaping
            Assert::AreEqual(gcnew String(L"[\\[]"), VariableEscapeStringHelper(L"["));
            Assert::AreEqual(gcnew String(L"[\\]]"), VariableEscapeStringHelper(L"]"));
            Assert::AreEqual(gcnew String(L" [\\[]TEXT[\\]] "), VariableEscapeStringHelper(L" [TEXT] "));
        }

        [TestMethod]
        void VariablesConditionTest()
        {
            HRESULT hr = S_OK;
            BURN_VARIABLES variables = { };
            try
            {
                // set variables
                VariableSetStringHelper(&variables, L"PROP1", L"VAL1");
                VariableSetStringHelper(&variables, L"PROP2", L"VAL2");
                VariableSetStringHelper(&variables, L"PROP3", L"VAL3");
                VariableSetStringHelper(&variables, L"PROP4", L"BEGIN MID END");
                VariableSetNumericHelper(&variables, L"PROP5", 5);
                VariableSetNumericHelper(&variables, L"PROP6", 6);
                VariableSetStringHelper(&variables, L"PROP7", L"");
                VariableSetNumericHelper(&variables, L"PROP8", 0);
                VariableSetStringHelper(&variables, L"_PROP9", L"VAL9");
                VariableSetNumericHelper(&variables, L"PROP10", -10);
                VariableSetNumericHelper(&variables, L"PROP11", 9223372036854775807ll);
                VariableSetNumericHelper(&variables, L"PROP12", -9223372036854775808ll);
                VariableSetNumericHelper(&variables, L"PROP13", 0x00010000);
                VariableSetNumericHelper(&variables, L"PROP14", 0x00000001);
                VariableSetNumericHelper(&variables, L"PROP15", 0x00010001);
                VariableSetVersionHelper(&variables, L"PROP16", MAKEQWORDVERSION(0,0,0,0));
                VariableSetVersionHelper(&variables, L"PROP17", MAKEQWORDVERSION(1,0,0,0));
                VariableSetVersionHelper(&variables, L"PROP18", MAKEQWORDVERSION(1,1,0,0));
                VariableSetVersionHelper(&variables, L"PROP19", MAKEQWORDVERSION(1,1,1,0));
                VariableSetVersionHelper(&variables, L"PROP20", MAKEQWORDVERSION(1,1,1,1));
                VariableSetNumericHelper(&variables, L"vPROP21", 1);
                VariableSetVersionHelper(&variables, L"PROP22", MAKEQWORDVERSION(65535,65535,65535,65535));
                VariableSetStringHelper(&variables, L"PROP23", L"1.1.1");

                // test conditions
                Assert::IsTrue(EvaluateConditionHelper(&variables, L"PROP1"));
                Assert::IsTrue(EvaluateConditionHelper(&variables, L"PROP5"));
                Assert::IsFalse(EvaluateConditionHelper(&variables, L"PROP7"));
                Assert::IsFalse(EvaluateConditionHelper(&variables, L"PROP8"));
                Assert::IsTrue(EvaluateConditionHelper(&variables, L"_PROP9"));
                Assert::IsFalse(EvaluateConditionHelper(&variables, L"PROP16"));
                Assert::IsTrue(EvaluateConditionHelper(&variables, L"PROP17"));

                Assert::IsTrue(EvaluateConditionHelper(&variables, L"PROP1 = \"VAL1\""));
                Assert::IsFalse(EvaluateConditionHelper(&variables, L"NONE = \"NOT\""));
                Assert::IsFalse(EvaluateConditionHelper(&variables, L"PROP1 <> \"VAL1\""));
                Assert::IsTrue(EvaluateConditionHelper(&variables, L"NONE <> \"NOT\""));

                Assert::IsTrue(EvaluateConditionHelper(&variables, L"PROP5 = 5"));
                Assert::IsFalse(EvaluateConditionHelper(&variables, L"PROP5 = 0"));
                Assert::IsFalse(EvaluateConditionHelper(&variables, L"PROP5 <> 5"));
                Assert::IsTrue(EvaluateConditionHelper(&variables, L"PROP5 <> 0"));

                Assert::IsTrue(EvaluateConditionHelper(&variables, L"PROP10 = -10"));
                Assert::IsFalse(EvaluateConditionHelper(&variables, L"PROP10 <> -10"));

                Assert::IsTrue(EvaluateConditionHelper(&variables, L"PROP17 = v1"));
                Assert::IsFalse(EvaluateConditionHelper(&variables, L"PROP17 = v0"));
                Assert::IsFalse(EvaluateConditionHelper(&variables, L"PROP17 <> v1"));
                Assert::IsTrue(EvaluateConditionHelper(&variables, L"PROP17 <> v0"));

                Assert::IsTrue(EvaluateConditionHelper(&variables, L"PROP16 = v0"));
                Assert::IsTrue(EvaluateConditionHelper(&variables, L"PROP17 = v1"));
                Assert::IsTrue(EvaluateConditionHelper(&variables, L"PROP18 = v1.1"));
                Assert::IsTrue(EvaluateConditionHelper(&variables, L"PROP19 = v1.1.1"));
                Assert::IsTrue(EvaluateConditionHelper(&variables, L"PROP20 = v1.1.1.1"));
                Assert::IsFalse(EvaluateConditionHelper(&variables, L"PROP20 = v1.1.1.1.0"));
                Assert::IsFalse(EvaluateConditionHelper(&variables, L"PROP20 = v1.1.1.1.1"));
                Assert::IsTrue(EvaluateConditionHelper(&variables, L"vPROP21 = 1"));
                Assert::IsTrue(EvaluateConditionHelper(&variables, L"PROP23 = v1.1.1"));
                Assert::IsTrue(EvaluateConditionHelper(&variables, L"v1.1.1 = PROP23"));
                Assert::IsTrue(EvaluateConditionHelper(&variables, L"PROP1 <> v1.1.1"));
                Assert::IsTrue(EvaluateConditionHelper(&variables, L"v1.1.1 <> PROP1"));

                Assert::IsFalse(EvaluateConditionHelper(&variables, L"PROP11 = 9223372036854775806"));
                Assert::IsTrue(EvaluateConditionHelper(&variables, L"PROP11 = 9223372036854775807"));
                Assert::IsFalse(EvaluateConditionHelper(&variables, L"PROP11 = 9223372036854775808"));
                Assert::IsFalse(EvaluateConditionHelper(&variables, L"PROP11 = 92233720368547758070000"));

                Assert::IsFalse(EvaluateConditionHelper(&variables, L"PROP12 = -9223372036854775807"));
                Assert::IsTrue(EvaluateConditionHelper(&variables, L"PROP12 = -9223372036854775808"));
                Assert::IsFalse(EvaluateConditionHelper(&variables, L"PROP12 = -9223372036854775809"));
                Assert::IsFalse(EvaluateConditionHelper(&variables, L"PROP12 = -92233720368547758080000"));

                Assert::IsTrue(EvaluateConditionHelper(&variables, L"PROP22 = v65535.65535.65535.65535"));
                Assert::IsFalse(EvaluateConditionHelper(&variables, L"PROP22 = v65536.65535.65535.65535"));
                Assert::IsFalse(EvaluateConditionHelper(&variables, L"PROP22 = v65535.655350000.65535.65535"));

                Assert::IsTrue(EvaluateConditionHelper(&variables, L"PROP5 < 6"));
                Assert::IsFalse(EvaluateConditionHelper(&variables, L"PROP5 < 5"));
                Assert::IsTrue(EvaluateConditionHelper(&variables, L"PROP5 > 4"));
                Assert::IsFalse(EvaluateConditionHelper(&variables, L"PROP5 > 5"));
                Assert::IsTrue(EvaluateConditionHelper(&variables, L"PROP5 <= 6"));
                Assert::IsTrue(EvaluateConditionHelper(&variables, L"PROP5 <= 5"));
                Assert::IsFalse(EvaluateConditionHelper(&variables, L"PROP5 <= 4"));
                Assert::IsTrue(EvaluateConditionHelper(&variables, L"PROP5 >= 4"));
                Assert::IsTrue(EvaluateConditionHelper(&variables, L"PROP5 >= 5"));
                Assert::IsFalse(EvaluateConditionHelper(&variables, L"PROP5 >= 6"));

                Assert::IsTrue(EvaluateConditionHelper(&variables, L"PROP4 << \"BEGIN\""));
                Assert::IsFalse(EvaluateConditionHelper(&variables, L"PROP4 << \"END\""));
                Assert::IsTrue(EvaluateConditionHelper(&variables, L"PROP4 >> \"END\""));
                Assert::IsFalse(EvaluateConditionHelper(&variables, L"PROP4 >> \"BEGIN\""));
                Assert::IsTrue(EvaluateConditionHelper(&variables, L"PROP4 >< \"MID\""));
                Assert::IsFalse(EvaluateConditionHelper(&variables, L"PROP4 >< \"NONE\""));

                Assert::IsTrue(EvaluateConditionHelper(&variables, L"PROP16 < v1.1"));
                Assert::IsFalse(EvaluateConditionHelper(&variables, L"PROP16 < v0"));
                Assert::IsTrue(EvaluateConditionHelper(&variables, L"PROP17 > v0.12"));
                Assert::IsFalse(EvaluateConditionHelper(&variables, L"PROP17 > v1"));
                Assert::IsTrue(EvaluateConditionHelper(&variables, L"PROP18 >= v1.0"));
                Assert::IsTrue(EvaluateConditionHelper(&variables, L"PROP18 >= v1.1"));
                Assert::IsFalse(EvaluateConditionHelper(&variables, L"PROP18 >= v2.1"));
                Assert::IsTrue(EvaluateConditionHelper(&variables, L"PROP19 <= v1.1234.1"));
                Assert::IsTrue(EvaluateConditionHelper(&variables, L"PROP19 <= v1.1.1"));
                Assert::IsFalse(EvaluateConditionHelper(&variables, L"PROP19 <= v1.0.123"));

                Assert::IsTrue(EvaluateConditionHelper(&variables, L"PROP6 = \"6\""));
                Assert::IsTrue(EvaluateConditionHelper(&variables, L"\"6\" = PROP6"));
                Assert::IsFalse(EvaluateConditionHelper(&variables, L"PROP6 = \"ABC\""));
                Assert::IsFalse(EvaluateConditionHelper(&variables, L"\"ABC\" = PROP6"));
                Assert::IsFalse(EvaluateConditionHelper(&variables, L"\"ABC\" = PROP6"));

                Assert::IsTrue(EvaluateConditionHelper(&variables, L"PROP13 << 1"));
                Assert::IsFalse(EvaluateConditionHelper(&variables, L"PROP13 << 0"));
                Assert::IsTrue(EvaluateConditionHelper(&variables, L"PROP14 >> 1"));
                Assert::IsFalse(EvaluateConditionHelper(&variables, L"PROP14 >> 0"));
                Assert::IsTrue(EvaluateConditionHelper(&variables, L"PROP15 >< 65537"));
                Assert::IsFalse(EvaluateConditionHelper(&variables, L"PROP15 >< 0"));

                Assert::IsFalse(EvaluateConditionHelper(&variables, L"NOT PROP1"));
                Assert::IsTrue(EvaluateConditionHelper(&variables, L"NOT (PROP1 <> \"VAL1\")"));

                Assert::IsTrue(EvaluateConditionHelper(&variables, L"PROP1 = \"VAL1\" AND PROP2 = \"VAL2\""));
                Assert::IsFalse(EvaluateConditionHelper(&variables, L"PROP1 = \"VAL1\" AND PROP2 = \"NOT\""));
                Assert::IsFalse(EvaluateConditionHelper(&variables, L"PROP1 = \"NOT\" AND PROP2 = \"VAL2\""));
                Assert::IsFalse(EvaluateConditionHelper(&variables, L"PROP1 = \"NOT\" AND PROP2 = \"NOT\""));

                Assert::IsTrue(EvaluateConditionHelper(&variables, L"PROP1 = \"VAL1\" OR PROP2 = \"VAL2\""));
                Assert::IsTrue(EvaluateConditionHelper(&variables, L"PROP1 = \"VAL1\" OR PROP2 = \"NOT\""));
                Assert::IsTrue(EvaluateConditionHelper(&variables, L"PROP1 = \"NOT\" OR PROP2 = \"VAL2\""));
                Assert::IsFalse(EvaluateConditionHelper(&variables, L"PROP1 = \"NOT\" OR PROP2 = \"NOT\""));

                Assert::IsTrue(EvaluateConditionHelper(&variables, L"PROP1 = \"VAL1\" AND PROP2 = \"VAL2\" OR PROP3 = \"NOT\""));
                Assert::IsTrue(EvaluateConditionHelper(&variables, L"PROP1 = \"VAL1\" AND PROP2 = \"NOT\" OR PROP3 = \"VAL3\""));
                Assert::IsFalse(EvaluateConditionHelper(&variables, L"PROP1 = \"VAL1\" AND PROP2 = \"NOT\" OR PROP3 = \"NOT\""));
                Assert::IsTrue(EvaluateConditionHelper(&variables, L"PROP1 = \"VAL1\" AND (PROP2 = \"NOT\" OR PROP3 = \"VAL3\")"));
                Assert::IsTrue(EvaluateConditionHelper(&variables, L"(PROP1 = \"VAL1\" AND PROP2 = \"VAL2\") OR PROP3 = \"NOT\""));

                Assert::IsTrue(EvaluateConditionHelper(&variables, L"PROP3 = \"NOT\" OR PROP1 = \"VAL1\" AND PROP2 = \"VAL2\""));
                Assert::IsTrue(EvaluateConditionHelper(&variables, L"PROP3 = \"VAL3\" OR PROP1 = \"VAL1\" AND PROP2 = \"NOT\""));
                Assert::IsFalse(EvaluateConditionHelper(&variables, L"PROP3 = \"NOT\" OR PROP1 = \"VAL1\" AND PROP2 = \"NOT\""));
                Assert::IsTrue(EvaluateConditionHelper(&variables, L"(PROP3 = \"NOT\" OR PROP1 = \"VAL1\") AND PROP2 = \"VAL2\""));
                Assert::IsTrue(EvaluateConditionHelper(&variables, L"PROP3 = \"NOT\" OR (PROP1 = \"VAL1\" AND PROP2 = \"VAL2\")"));

                Assert::IsFalse(EvaluateConditionHelper(&variables, L"="));
                Assert::IsFalse(EvaluateConditionHelper(&variables, L"(PROP1"));
                Assert::IsFalse(EvaluateConditionHelper(&variables, L"(PROP1 = \""));
                Assert::IsFalse(EvaluateConditionHelper(&variables, L"1A"));
                Assert::IsFalse(EvaluateConditionHelper(&variables, L"*"));
            }
            finally
            {
                VariablesUninitialize(&variables);
            }
        }

        [TestMethod]
        void VariablesSerializationTest()
        {
            HRESULT hr = S_OK;
            BYTE* pbBuffer = NULL;
            SIZE_T cbBuffer = 0;
            SIZE_T iBuffer = 0;
            BURN_VARIABLES variables1 = { };
            BURN_VARIABLES variables2 = { };
            try
            {
                // serialize
                hr = VariableInitializeBuiltIn(&variables1);
                TestThrowOnFailure(hr, L"Failed to initialize built-in variables.");

                VariableSetStringHelper(&variables1, L"PROP1", L"VAL1");
                VariableSetNumericHelper(&variables1, L"PROP2", 2);
                VariableSetVersionHelper(&variables1, L"PROP3", MAKEQWORDVERSION(1,1,1,1));
                VariableSetStringHelper(&variables1, L"PROP4", L"VAL4");

                hr = VariableSerialize(&variables1, &pbBuffer, &cbBuffer);
                TestThrowOnFailure(hr, L"Failed to serialize variables.");

                // deserialize
                hr = VariableInitializeBuiltIn(&variables2);
                TestThrowOnFailure(hr, L"Failed to initialize built-in variables.");

                hr = VariableDeserialize(&variables2, pbBuffer, cbBuffer, &iBuffer);
                TestThrowOnFailure(hr, L"Failed to deserialize variables.");

                Assert::AreEqual(gcnew String(L"VAL1"), VariableGetStringHelper(&variables2, L"PROP1"));
                Assert::AreEqual(2ll, VariableGetNumericHelper(&variables2, L"PROP2"));
                Assert::AreEqual(MAKEQWORDVERSION(1,1,1,1), VariableGetVersionHelper(&variables2, L"PROP3"));
                Assert::AreEqual(gcnew String(L"VAL4"), VariableGetStringHelper(&variables2, L"PROP4"));
            }
            finally
            {
                ReleaseBuffer(pbBuffer);
                VariablesUninitialize(&variables1);
                VariablesUninitialize(&variables2);
            }
        }

        [TestMethod]
        void VariablesBuiltInTest()
        {
            HRESULT hr = S_OK;
            BURN_VARIABLES variables = { };
            try
            {
                hr = VariableInitializeBuiltIn(&variables);
                TestThrowOnFailure(hr, L"Failed to initialize built-in variables.");

                // VersionMsi
                Assert::IsTrue(EvaluateConditionHelper(&variables, L"VersionMsi >= v1.1"));

                // VersionNT
                Version^ osVersion = Environment::OSVersion->Version;
                pin_ptr<const WCHAR> wzOsVersionCondition1 = PtrToStringChars(String::Format(L"VersionNT = v{0}.{1}", osVersion->Major, osVersion->Minor));
                Assert::IsTrue(EvaluateConditionHelper(&variables, wzOsVersionCondition1));
                pin_ptr<const WCHAR> wzOsVersionCondition2 = PtrToStringChars(String::Format(L"VersionNT <> v{0}.{1}", osVersion->Major, osVersion->Minor));
                Assert::IsFalse(EvaluateConditionHelper(&variables, wzOsVersionCondition2));

                // VersionNT64
                if (nullptr == Environment::GetEnvironmentVariable("ProgramFiles(x86)"))
                {
                    Assert::IsFalse(EvaluateConditionHelper(&variables, L"VersionNT64"));
                }
                else
                {
                    Assert::IsTrue(EvaluateConditionHelper(&variables, L"VersionNT64"));
                }

                // attempt to set a built-in property
                hr = VariableSetString(&variables, L"VersionNT", L"VAL");
                Assert::AreEqual(E_INVALIDARG, hr);
                Assert::IsFalse(EvaluateConditionHelper(&variables, L"VersionNT = \"VAL\""));

                VariableGetNumericHelper(&variables, L"NTProductType");
                VariableGetNumericHelper(&variables, L"NTSuiteBackOffice");
                VariableGetNumericHelper(&variables, L"NTSuiteDataCenter");
                VariableGetNumericHelper(&variables, L"NTSuiteEnterprise");
                VariableGetNumericHelper(&variables, L"NTSuitePersonal");
                VariableGetNumericHelper(&variables, L"NTSuiteSmallBusiness");
                VariableGetNumericHelper(&variables, L"NTSuiteSmallBusinessRestricted");
                VariableGetNumericHelper(&variables, L"NTSuiteWebServer");
                VariableGetNumericHelper(&variables, L"CompatibilityMode");
                VariableGetNumericHelper(&variables, L"Privileged");

                // known folders
                VariableGetStringHelper(&variables, L"AdminToolsFolder");
                Assert::AreEqual(VariableGetStringHelper(&variables, L"AppDataFolder"), Environment::GetFolderPath(Environment::SpecialFolder::ApplicationData));
                Assert::AreEqual(VariableGetStringHelper(&variables, L"CommonAppDataFolder"), Environment::GetFolderPath(Environment::SpecialFolder::CommonApplicationData));
                //VariableGetStringHelper(&variables, L"CommonFiles64Folder");
                Assert::AreEqual(VariableGetStringHelper(&variables, L"CommonFilesFolder"), Environment::GetFolderPath(Environment::SpecialFolder::CommonProgramFiles));
                Assert::AreEqual(VariableGetStringHelper(&variables, L"DesktopFolder"), Environment::GetFolderPath(Environment::SpecialFolder::DesktopDirectory));
                Assert::AreEqual(VariableGetStringHelper(&variables, L"FavoritesFolder"), Environment::GetFolderPath(Environment::SpecialFolder::Favorites));
                VariableGetStringHelper(&variables, L"FontsFolder");
                Assert::AreEqual(VariableGetStringHelper(&variables, L"LocalAppDataFolder"), Environment::GetFolderPath(Environment::SpecialFolder::LocalApplicationData));
                Assert::AreEqual(VariableGetStringHelper(&variables, L"MyPicturesFolder"), Environment::GetFolderPath(Environment::SpecialFolder::MyPictures));
                Assert::AreEqual(VariableGetStringHelper(&variables, L"PersonalFolder"), Environment::GetFolderPath(Environment::SpecialFolder::Personal));
                //VariableGetStringHelper(&variables, L"ProgramFiles64Folder");
                Assert::AreEqual(VariableGetStringHelper(&variables, L"ProgramFilesFolder"), Environment::GetFolderPath(Environment::SpecialFolder::ProgramFiles));
                Assert::AreEqual(VariableGetStringHelper(&variables, L"ProgramMenuFolder"), Environment::GetFolderPath(Environment::SpecialFolder::Programs));
                Assert::AreEqual(VariableGetStringHelper(&variables, L"SendToFolder"), Environment::GetFolderPath(Environment::SpecialFolder::SendTo));
                Assert::AreEqual(VariableGetStringHelper(&variables, L"StartMenuFolder"), Environment::GetFolderPath(Environment::SpecialFolder::StartMenu));
                Assert::AreEqual(VariableGetStringHelper(&variables, L"StartupFolder"), Environment::GetFolderPath(Environment::SpecialFolder::Startup));
                VariableGetStringHelper(&variables, L"SystemFolder");
                Assert::AreEqual(VariableGetStringHelper(&variables, L"TempFolder"), System::IO::Path::GetTempPath());
                Assert::AreEqual(VariableGetStringHelper(&variables, L"TemplateFolder"), Environment::GetFolderPath(Environment::SpecialFolder::Templates));
                VariableGetStringHelper(&variables, L"WindowsFolder");
                VariableGetStringHelper(&variables, L"WindowsVolume");
            }
            finally
            {
                VariablesUninitialize(&variables);
            }
        }
    };
}
}
}
}
}
