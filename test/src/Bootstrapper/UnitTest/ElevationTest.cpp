//-------------------------------------------------------------------------------------------------
// <copyright file="ElevationTest.cpp" company="Microsoft">
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
//    Unit tests for Burn elevation.
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"


const char TEST_MESSAGE_DATA[] = "{94949868-7EAE-4ac5-BEAC-AFCA2821DE01}";


static BOOL STDAPICALLTYPE ElevateTest_ShellExecuteExW(
    __inout LPSHELLEXECUTEINFOW lpExecInfo
    );
static DWORD CALLBACK ElevateTest_ThreadProc(
    __in LPVOID lpThreadParameter
    );


using namespace System;
using namespace System::IO;
using namespace System::Threading;
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
    public ref class ElevationTest : BurnUnitTest
    {
    public:
        [TestMethod]
        void ElevateTest()
        {
            HRESULT hr = S_OK;
            HRESULT hrResult = S_OK;
            HANDLE hProcess = NULL;
            HANDLE hPipe = NULL;
            BURN_ELEVATION_MESSAGE msg = { };
            bool fInvalidMessageData = FALSE;
            try
            {
                vpfnShellExecuteExW = ElevateTest_ShellExecuteExW;

                //
                // per-user side setup
                //
                hr = ElevationParentProcessConnect(NULL, &hProcess, &hPipe);
                TestThrowOnFailure(hr, L"Failed to create elevated process.");

                // post execute message
                hr = ElevationPostMessage(hPipe, BURN_ELEVATION_MESSAGE_TYPE_EXECUTE, NULL, 0);
                TestThrowOnFailure(hr, "Failed to post execute message to per-machine process.");

                // pump messages
                while (S_OK == (hr = ElevationGetMessage(hPipe, &msg)))
                {
                    // Process the message.
                    if (BURN_ELEVATION_MESSAGE_TYPE_COMPLETE == msg.dwMessage)
                    {
                        break;
                    }
                    switch (msg.dwMessage)
                    {
                    case BURN_ELEVATION_MESSAGE_TYPE_LOG:
                        if (sizeof(TEST_MESSAGE_DATA) != msg.cbData || 0 != memcmp(TEST_MESSAGE_DATA, msg.pvData, sizeof(TEST_MESSAGE_DATA)))
                        {
                            fInvalidMessageData = TRUE;
                        }
                        hrResult = S_FALSE;
                        break;

                    default:
                        hr = E_INVALIDARG;
                        TestThrowOnFailure(hr, "Unexpected elevated message sent to child process.");
                    }

                    // post result message
                    hr = ElevationPostMessage(hPipe, BURN_ELEVATION_MESSAGE_TYPE_COMPLETE, &hrResult, sizeof(hrResult));
                    TestThrowOnFailure(hr, "Failed to post result message.");

                    ElevationMessageUninitialize(&msg);
                }

                //
                // initiate termination
                //
                hr = ElevationParentProcessTerminate(hProcess, hPipe);
                TestThrowOnFailure(hr, L"Failed to termiate elevated process.");

                // check flags
                Assert::IsFalse(fInvalidMessageData);
            }
            finally
            {
                if (hProcess)
                {
                    ::CloseHandle(hProcess);
                }
                if (hPipe)
                {
                    ::CloseHandle(hPipe);
                }
                ElevationMessageUninitialize(&msg);
            }
        }
    };
}
}
}
}
}


static BOOL STDAPICALLTYPE ElevateTest_ShellExecuteExW(
    __inout LPSHELLEXECUTEINFOW lpExecInfo
    )
{
    HRESULT hr = S_OK;
    LPWSTR scz = NULL;

    hr = StrAllocString(&scz, lpExecInfo->lpParameters, 0);
    ExitOnFailure(hr, "Failed to copy arguments.");

    lpExecInfo->hProcess = ::CreateThread(NULL, 0, ElevateTest_ThreadProc, scz, 0, NULL);
    ExitOnNullWithLastError(lpExecInfo->hProcess, hr, "Failed to create thread.");
    scz = NULL;

LExit:
    ReleaseStr(scz);

    return SUCCEEDED(hr);
}

static DWORD CALLBACK ElevateTest_ThreadProc(
    __in LPVOID lpThreadParameter
    )
{
    HRESULT hr = S_OK;
    HANDLE hPipe = NULL;
    LPWSTR sczArguments = (LPWSTR)lpThreadParameter;
    DWORD dwResult = 0;
    BURN_ELEVATION_MESSAGE msg = { };
    WCHAR wzPipeName[MAX_PATH] = { };
    WCHAR wzToken[MAX_PATH] = { };

    // parse command line arguments
    if (2 != swscanf_s(sczArguments, L"-q -burn.elevated %s %s", wzPipeName, countof(wzPipeName), wzToken, countof(wzToken)))
    {
        hr = E_INVALIDARG;
        ExitOnFailure(hr, L"Failed to parse argument string.");
    }

    // set up connection with per-user process
    hr = ElevationChildConnect(wzPipeName, wzToken, &hPipe);
    ExitOnFailure(hr, L"Failed to connect to per-user process.");

    hr = ElevationChildConnected(hPipe);
    ExitOnFailure(hr, L"Failed to pass connected message to per-user process.");

    // pump messages
    while (S_OK == (hr = ElevationGetMessage(hPipe, &msg)))
    {
        // Process the message.
        switch (msg.dwMessage)
        {
        case BURN_ELEVATION_MESSAGE_TYPE_EXECUTE:
            // send log message
            hr = ElevationSendMessage(hPipe, BURN_ELEVATION_MESSAGE_TYPE_LOG, (LPVOID)TEST_MESSAGE_DATA, sizeof(TEST_MESSAGE_DATA), &dwResult);
            ExitOnFailure(hr, "Failed to send message to per-machine process.");

            if (S_FALSE != dwResult)
            {
                hr = E_FAIL;
                ExitOnFailure(hr, "Invalid result from per-user process.");
            }

            // post complete message
            hr = ElevationPostMessage(hPipe, BURN_ELEVATION_MESSAGE_TYPE_COMPLETE, NULL, 0);
            ExitOnFailure(hr, "Failed to post complete message to per-user process.");

            break;

        case BURN_ELEVATION_MESSAGE_TYPE_TERMINATE:
            ExitFunction1(hr = S_OK);

        default:
            hr = E_INVALIDARG;
            ExitOnRootFailure1(hr, "Unexpected elevated message sent to child process, msg: %u", msg.dwMessage);
        }

        ElevationMessageUninitialize(&msg);
    }

    if (S_FALSE == hr)
    {
        hr = S_OK;
    }

LExit:
    if (hPipe)
    {
        ::CloseHandle(hPipe);
    }

    ReleaseStr(sczArguments);

    ElevationMessageUninitialize(&msg);

    return (DWORD)hr;
}
