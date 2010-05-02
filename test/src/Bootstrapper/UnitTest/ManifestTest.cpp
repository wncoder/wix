//-------------------------------------------------------------------------------------------------
// <copyright file="ManifestTest.cpp" company="Microsoft">
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
//    Manifest unit tests for Burn.
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"


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
    public ref class ManifestTest
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
        //[TestInitialize()]
        //void MyTestInitialize() {};
        //
        //Use TestCleanup to run code after each test has run
        //[TestCleanup()]
        //void MyTestCleanup() {};
        //
        #pragma endregion 

        [TestMethod]
        void ManifestLoadXmlTest()
        {
            HRESULT hr = S_OK;
            BURN_ENGINE_STATE engineState = { };
            try
            {
                LPCSTR szDocument =
                    "<Bundle>"
                    "    <Ux UxDllPayloadId='ux.dll'>"
                    "        <Payload Id='ux.dll' FilePath='ux.dll' Packaging='embedded' SourcePath='ux.dll' />"
                    "    </Ux>"
                    "    <Registration Id='{D54F896D-1952-43e6-9C67-B5652240618C}' ExecutableName='setup.exe' PerMachine='no' Family='{E56901C1-8FCD-4491-B44C-850CB60F511E}' />"
                    "    <Variable Id='Variable1' Type='numeric' Value='1' />"
                    "    <RegistrySearch Id='Search1' Type='exists' Root='HKLM' Key='SOFTWARE\\Microsoft' Variable='Variable1' Condition='0' />"
                    "</Bundle>";

                // load manifest from XML
                hr = ManifestLoadXmlFromBuffer((BYTE*)szDocument, lstrlenA(szDocument), &engineState);
                TestThrowOnFailure(hr, L"Failed to parse searches from XML.");

                // check variable values
                Assert::IsTrue(VariableExistsHelper(&engineState.variables, L"Variable1"));
            }
            finally
            {
                CoreUninitialize(&engineState);
            }
        }
    };
}
}
}
}
}
