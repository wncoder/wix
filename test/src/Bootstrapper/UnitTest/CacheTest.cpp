//-------------------------------------------------------------------------------------------------
// <copyright file="CacheTest.cpp" company="Microsoft">
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
//    Cache unit tests for Burn.
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"


using namespace System;
using namespace System::IO;
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
    public ref class CacheTest : BurnUnitTest
    {
    public:
        [TestMethod]
        void CacheSignatureTest()
        {
            HRESULT hr = S_OK;
            BURN_PACKAGE package = { };
            BURN_PAYLOAD payload = { };
            LPWSTR sczPayloadPath = NULL;
            BYTE* pb = NULL;
            DWORD cb = NULL;

            try
            {
                hr = PathExpand(&sczPayloadPath, L"%WIX_ROOT%\\src\\Votive\\SDK\\Redist\\ProjectAggregator2.msi", PATH_EXPAND_ENVIRONMENT);
                Assert::AreEqual(S_OK, hr, "Failed to get path to project aggregator MSI.");

                hr = StrAllocHexDecode(L"4A5C7522AA46BFA4089D39974EBDB4A360F7A01D", &pb, &cb);
                Assert::AreEqual(S_OK, hr);

                package.fPerMachine = FALSE;
                package.sczCacheId = L"Bootstrapper.CacheTest.CacheSignatureTest";
                payload.sczFilePath = L"CacheSignatureTest.File";
                payload.pbCertificateRootPublicKeyIdentifier = pb;
                payload.cbCertificateRootPublicKeyIdentifier = cb;

                hr = CachePayload(&package, &payload, NULL, sczPayloadPath, FALSE);
                Assert::AreEqual(S_OK, hr, "Failed while verifying path.");
            }
            finally
            {
                ReleaseMem(pb);
                ReleaseStr(sczPayloadPath);

                String^ filePath = Path::Combine(Environment::GetFolderPath(Environment::SpecialFolder::LocalApplicationData), "Package Cache\\Bootstrapper.CacheTest.CacheSignatureTest\\CacheSignatureTest.File");
                if (File::Exists(filePath))
                {
                    File::SetAttributes(filePath, FileAttributes::Normal);
                    File::Delete(filePath);
                }
            }
        }
    };
}
}
}
}
}
