//-------------------------------------------------------------------------------------------------
// <copyright file="core.cpp" company="Microsoft">
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
//    Module: Core
//
//    Setup chainer/bootstrapper core for WiX toolset.
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"


// function definitions

extern "C" HRESULT CoreInitialize(
    __in BURN_ENGINE_STATE* pEngineState
    )
{
    HRESULT hr = S_OK;
    LPWSTR sczStreamName = NULL;
    BYTE* pbBuffer = NULL;
    SIZE_T cbBuffer = 0;
    BURN_CONTAINER_CONTEXT containerContext = { };

    // initialize structure
    ::InitializeCriticalSection(&pEngineState->csActive);
    pEngineState->hElevatedPipe = INVALID_HANDLE_VALUE;

    // initialize built-in variables
    hr = VariableInitializeBuiltIn(&pEngineState->variables);
    ExitOnFailure(hr, "Failed to initialize built-in variables.");

    //// open attached UX container
    //hr = ContainerOpenUx(&containerContext);
    //ExitOnFailure(hr, "Failed to open attached UX container.");

    //// load manifest
    //hr = ContainerNextStream(&containerContext, &sczStreamName);
    //ExitOnFailure(hr, "Failed to open manifest stream.");

    //hr = ContainerStreamToBuffer(&containerContext, &pbBuffer, &cbBuffer);
    //ExitOnFailure(hr, "Failed to get manifest stream from container.");

    //hr = ManifestLoadXmlFromBuffer(pbBuffer, cbBuffer, pEngineState);
    //ExitOnFailure(hr, "Failed to load manifest.");

    //// load UX
    //hr = UserExperienceLoad(&pEngineState->userExperience, &containerContext, &pEngineState->command);
    //ExitOnFailure(hr, "Failed to load UX.");

LExit:
    //ContainerClose(&containerContext);
    ReleaseStr(sczStreamName);
    ReleaseMem(pbBuffer);

    return hr;
}

extern "C" void CoreUninitialize(
    __in BURN_ENGINE_STATE* pEngineState
    )
{
    // unload UX
    //UserExperienceUnload(&pEngineState->userExperience);

    ::DeleteCriticalSection(&pEngineState->csActive);

    VariablesUninitialize(&pEngineState->variables);
    SearchesUninitialize(&pEngineState->searches);
    UserExperienceUninitialize(&pEngineState->userExperience);
    RegistrationUninitialize(&pEngineState->registration);
    PayloadsUninitialize(&pEngineState->payloads);
    PackagesUninitialize(&pEngineState->packages);
}

extern "C" HRESULT CoreSerializeEngineState(
    __in BURN_ENGINE_STATE* pEngineState,
    __inout BYTE** ppbBuffer,
    __inout SIZE_T* piBuffer
    )
{
    HRESULT hr = S_OK;

    hr = VariableSerialize(&pEngineState->variables, ppbBuffer, piBuffer);
    ExitOnFailure(hr, "Failed to serialize variables.");

LExit:
    return hr;
}

extern "C" HRESULT CoreDeserializeEngineState(
    __in BURN_ENGINE_STATE* pEngineState,
    __in_bcount(cbBuffer) BYTE* pbBuffer,
    __in SIZE_T cbBuffer
    )
{
    HRESULT hr = S_OK;
    SIZE_T iBuffer = 0;

    hr = VariableDeserialize(&pEngineState->variables, pbBuffer, cbBuffer, &iBuffer);
    ExitOnFailure(hr, "Failed to deserialize variables.");

LExit:
    return hr;
}
