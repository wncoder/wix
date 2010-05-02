//-------------------------------------------------------------------------------------------------
// <copyright file="pipe.cpp" company="Microsoft">
//    Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// 
// <summary>
//    Burn Client Server pipe communication handler.
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"

static HRESULT PipeBurnServerPumpMessages(
    __in HANDLE hPipe,
    __in BURN_VARIABLES* pVariables
    );

static HRESULT PipeOnVariableGetNumeric(
    __in HANDLE hPipe,
    __in BURN_VARIABLES* pVariables,
    __in BYTE* pbData,
    __in DWORD cbData
    );

static HRESULT PipeOnVariableGetString(
    __in HANDLE hPipe,
    __in BURN_VARIABLES* pVariables,
    __in BYTE* pbData,
    __in DWORD cbData
    );

static HRESULT PipeOnVariableGetVersion(
    __in HANDLE hPipe,
    __in BURN_VARIABLES* pVariables,
    __in BYTE* pbData,
    __in DWORD cbData
    );

// Listen and Delegate
extern "C" HRESULT PipeRunBurnServer(__in BURN_VARIABLES* pVariables)
{
    HRESULT hr = S_OK;
    HANDLE hPipe = INVALID_HANDLE_VALUE;

    // connect to client process
    hr = ElevationChildConnect(L"BurnServerPipe", L"{D3D8CA1B-045B-4632-BBE0-1083D6F05984}", &hPipe); // TODO: fix
    ExitOnFailure(hr, "Failed to connect to client process.");

    // pump messages from client process
    hr = PipeBurnServerPumpMessages(hPipe, pVariables);
    ExitOnFailure(hr, "Failed to pump messages from per-user process.");
    
LExit:
    return hr;
}

static HRESULT PipeBurnServerPumpMessages(
    __in HANDLE hPipe,
    __in BURN_VARIABLES* pVariables
    )
{
    HRESULT hr = S_OK;
    HRESULT hrResult = S_OK;
    BURN_ELEVATION_MESSAGE msg = { };

    hr = ElevationChildConnected(hPipe);
    ExitOnFailure(hr, "Failed to notify parent process that child is connected.");

    // Pump messages from parent process.
    while (S_OK == (hr = ElevationGetMessage(hPipe, &msg)))
    {
        // Process the message.
        switch (msg.dwMessage)
        {
        case BURN_PIPE_MESSAGE_TYPE_GETPRIORVARIABLENUMERIC:
            hr = PipeOnVariableGetNumeric(hPipe, pVariables, (BYTE*)msg.pvData, msg.cbData);
            break;

        case BURN_PIPE_MESSAGE_TYPE_GETPRIORVARIABLESTRING:
            hr = PipeOnVariableGetString(hPipe, pVariables, (BYTE*)msg.pvData, msg.cbData);
            break;

        case BURN_PIPE_MESSAGE_TYPE_GETPRIORVARIABLEVERSION:
            hr = PipeOnVariableGetVersion(hPipe, pVariables, (BYTE*)msg.pvData, msg.cbData);
            break;

        case BURN_PIPE_MESSAGE_TYPE_TERMINATE:
            ExitFunction1(hr = S_OK);

        default:
            hr = E_INVALIDARG;
            ExitOnRootFailure1(hr, "Unexpected message sent to server process, msg: %u", msg.dwMessage);
        }

        // post result message
        hr = ElevationPostMessage(hPipe, BURN_PIPE_MESSAGE_TYPE_COMPLETE, &hrResult, sizeof(hrResult));
        ExitOnFailure(hr, "Failed to post result message.");

        ElevationMessageUninitialize(&msg);
    }

    if (S_FALSE == hr)
    {
        hr = S_OK;
    }

LExit:
    ElevationMessageUninitialize(&msg);

    return hr;
}

static HRESULT PipeOnVariableGetNumeric(
    __in HANDLE hPipe,
    __in BURN_VARIABLES* pVariables,
    __in BYTE* pbData,
    __in DWORD cbData
    )
{
    HRESULT hr = S_OK;
    SIZE_T iData = 0;
    LONGLONG lValue = 0;
    LPWSTR wzVariable = NULL;

    // deserialize message data
    hr = BuffReadString(pbData, cbData, &iData, &wzVariable);

    // get variable
    hr = VariableGetNumeric(pVariables, wzVariable, &lValue);
    ExitOnFailure(hr, "Failed to get numeric variable value.");

    // communicate value
    hr = ElevationPostMessage(hPipe, BURN_PIPE_MESSAGE_TYPE_GETPRIORVARIABLE_RESULT, &lValue, sizeof(LONGLONG));
    ExitOnFailure(hr, "Failed to send variable value message.")

LExit:
    ReleaseStr(wzVariable);
    return hr;
}

static HRESULT PipeOnVariableGetString(
    __in HANDLE hPipe,
    __in BURN_VARIABLES* pVariables,
    __in BYTE* pbData,
    __in DWORD cbData
    )
{
    HRESULT hr = S_OK;
    SIZE_T iData = 0;
    BYTE* pbValue = NULL;
    SIZE_T cbValue = 0;
    LPWSTR wzVariable = NULL;
    LPWSTR wzValue = NULL;

    // deserialize message data
    hr = BuffReadString(pbData, cbData, &iData, &wzVariable);

    // get variable
    hr = VariableGetString(pVariables, wzVariable, &wzValue);
    ExitOnFailure(hr, "Failed to get string variable value.");

    hr = BuffWriteString(&pbValue, &cbValue, wzValue);
    ExitOnFailure(hr, "Failed to write variable value as string.");

    // communicate value
    hr = ElevationPostMessage(hPipe, BURN_PIPE_MESSAGE_TYPE_GETPRIORVARIABLE_RESULT, pbValue, cbValue);
    ExitOnFailure(hr, "Failed to send variable value message.")

LExit:
    ReleaseBuffer(pbValue);
    ReleaseStr(wzValue);
    ReleaseStr(wzVariable);


    return hr;
}

static HRESULT PipeOnVariableGetVersion(
    __in HANDLE hPipe,
    __in BURN_VARIABLES* pVariables,
    __in BYTE* pbData,
    __in DWORD cbData
    )
{
    HRESULT hr = S_OK;
    SIZE_T iData = 0;
    DWORD64 qwValue = 0;

    LPWSTR wzVariable = NULL;
    // deserialize message data
    hr = BuffReadString(pbData, cbData, &iData, &wzVariable);

    // suspend session in per-machine process
    hr = VariableGetVersion(pVariables, wzVariable, &qwValue);
    ExitOnFailure(hr, "Failed to suspend registration session.");

    hr = ElevationPostMessage(hPipe, BURN_PIPE_MESSAGE_TYPE_GETPRIORVARIABLE_RESULT, &qwValue, sizeof(DWORD64));
    ExitOnFailure(hr, "Failed to send variable value message.")

LExit:
    ReleaseStr(wzVariable);
    return hr;
}
