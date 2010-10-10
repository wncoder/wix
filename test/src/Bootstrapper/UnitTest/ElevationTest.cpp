//-------------------------------------------------------------------------------------------------
// <copyright file="ElevationTest.cpp" company="Microsoft">
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
//    Unit tests for Burn elevation.
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"


const DWORD TEST_CHILD_SENT_MESSAGE_ID = 0xFFFE;
const DWORD TEST_PARENT_SENT_MESSAGE_ID = 0xFFFF;
const char TEST_MESSAGE_DATA[] = "{94949868-7EAE-4ac5-BEAC-AFCA2821DE01}";


static BOOL STDAPICALLTYPE ElevateTest_ShellExecuteExW(
    __inout LPSHELLEXECUTEINFOW lpExecInfo
    );
static DWORD CALLBACK ElevateTest_ThreadProc(
    __in LPVOID lpThreadParameter
    );
static HRESULT ProcessParentMessages(
    __in BURN_PIPE_MESSAGE* pMsg,
    __in_opt LPVOID pvContext,
    __out DWORD* pdwResult
    );
static HRESULT ProcessChildMessages(
    __in BURN_PIPE_MESSAGE* pMsg,
    __in_opt LPVOID pvContext,
    __out DWORD* pdwResult
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
            HANDLE hPipe = NULL;
            LPWSTR sczPipeName = NULL;
            LPWSTR sczPipeToken = NULL;
            HANDLE hProcess = NULL;
            DWORD dwResult = S_OK;
            try
            {
                vpfnShellExecuteExW = ElevateTest_ShellExecuteExW;

                //
                // per-user side setup
                //
                hr = PipeCreatePipeNameAndToken(&hPipe, &sczPipeName, &sczPipeToken);
                TestThrowOnFailure(hr, L"Failed to create elevated pipe.");

                hr = PipeLaunchChildProcess(FALSE, sczPipeName, sczPipeToken, NULL, &hProcess);
                TestThrowOnFailure(hr, L"Failed to create elevated process.");

                hr = PipeWaitForChildConnect(hPipe, sczPipeToken, hProcess);
                TestThrowOnFailure(hr, L"Failed to wait for child process to connect.");

                // post execute message
                hr = PipeSendMessage(hPipe, TEST_PARENT_SENT_MESSAGE_ID, NULL, 0, ProcessParentMessages, NULL, &dwResult);
                TestThrowOnFailure(hr, "Failed to post execute message to per-machine process.");

                //
                // initiate termination
                //
                hr = PipeTerminateChildProcess(hProcess, hPipe);
                TestThrowOnFailure(hr, L"Failed to termiate elevated process.");

                // check flags
                hr = static_cast<HRESULT>(dwResult);
                TestThrowOnFailure(hr, L"Expected success returned from child process pipe.");
            }
            finally
            {
                ReleaseStr(sczPipeToken);
                ReleaseStr(sczPipeName);

                if (hProcess)
                {
                    ::CloseHandle(hProcess);
                }
                if (hPipe)
                {
                    ::CloseHandle(hPipe);
                }
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
    WCHAR wzPipeName[MAX_PATH] = { };
    WCHAR wzToken[MAX_PATH] = { };

    // parse command line arguments
    if (2 != swscanf_s(sczArguments, L"-q -burn.elevated %s %s", wzPipeName, countof(wzPipeName), wzToken, countof(wzToken)))
    {
        hr = E_INVALIDARG;
        ExitOnFailure(hr, L"Failed to parse argument string.");
    }

    // set up connection with per-user process
    hr = PipeChildConnect(wzPipeName, wzToken, &hPipe);
    ExitOnFailure(hr, L"Failed to connect to per-user process.");

    hr = PipeChildConnected(hPipe);
    ExitOnFailure(hr, L"Failed to pass connected message to per-user process.");

    // pump messages
    hr = PipePumpMessages(hPipe, ProcessChildMessages, static_cast<LPVOID>(hPipe), &dwResult);
    ExitOnFailure(hr, L"Failed while pumping messages in child 'process'.");

LExit:
    if (hPipe)
    {
        ::CloseHandle(hPipe);
    }

    ReleaseStr(sczArguments);

    return (DWORD)hr;
}

static HRESULT ProcessParentMessages(
    __in BURN_PIPE_MESSAGE* pMsg,
    __in_opt LPVOID pvContext,
    __out DWORD* pdwResult
    )
{
    HRESULT hr = S_OK;
    HRESULT hrResult = E_INVALIDDATA;

    // Process the message.
    switch (pMsg->dwMessage)
    {
    case TEST_CHILD_SENT_MESSAGE_ID:
        if (sizeof(TEST_MESSAGE_DATA) == pMsg->cbData && 0 == memcmp(TEST_MESSAGE_DATA, pMsg->pvData, sizeof(TEST_MESSAGE_DATA)))
        {
            hrResult = S_OK;
        }
        break;

    default:
        hr = E_INVALIDARG;
        ExitOnRootFailure1(hr, "Unexpected elevated message sent to parent process, msg: %u", pMsg->dwMessage);
    }

    *pdwResult = static_cast<DWORD>(hrResult);

LExit:
    return hr;
}

static HRESULT ProcessChildMessages(
    __in BURN_PIPE_MESSAGE* pMsg,
    __in_opt LPVOID pvContext,
    __out DWORD* pdwResult
    )
{
    HRESULT hr = S_OK;
    HANDLE hPipe = static_cast<HANDLE>(pvContext);
    DWORD dwResult = 0;

    // Process the message.
    switch (pMsg->dwMessage)
    {
    case TEST_PARENT_SENT_MESSAGE_ID:
        // send test message
        hr = PipeSendMessage(hPipe, TEST_CHILD_SENT_MESSAGE_ID, (LPVOID)TEST_MESSAGE_DATA, sizeof(TEST_MESSAGE_DATA), NULL, NULL, &dwResult);
        ExitOnFailure(hr, "Failed to send message to per-machine process.");
        break;

    default:
        hr = E_INVALIDARG;
        ExitOnRootFailure1(hr, "Unexpected elevated message sent to child process, msg: %u", pMsg->dwMessage);
    }

    *pdwResult = dwResult;

LExit:
    return hr;
}
