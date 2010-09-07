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


namespace Microsoft
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
    public ref class ManifestTest : BurnUnitTest
    {
    public:
        [TestMethod]
        void ManifestLoadXmlTest()
        {
            HRESULT hr = S_OK;
            BURN_ENGINE_STATE engineState = { };
            try
            {
                LPCSTR szDocument =
                    "<Bundle>"
                    "    <UX UxDllPayloadId='ux.dll'>"
                    "        <Payload Id='ux.dll' FilePath='ux.dll' Packaging='embedded' SourcePath='ux.dll' Hash='000000000000' />"
                    "    </UX>"
                    "    <Registration Id='{D54F896D-1952-43e6-9C67-B5652240618C}' UpgradeCode='{D54F896D-1952-43e6-9C67-B5652240618C}' Version='1.0.0.0' ExecutableName='setup.exe' PerMachine='no' />"
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
