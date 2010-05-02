//-------------------------------------------------------------------------------------------------
// <copyright file="userexperience.cpp" company="Microsoft">
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
// </summary>
//-------------------------------------------------------------------------------------------------

#include "precomp.h"


// internal function declarations

static HRESULT ExtractUxPayloads(
    __in BURN_CONTAINER_CONTEXT* pContainerContext,
    __in BURN_PAYLOADS* pPayloads,
    __in LPCWSTR wzTempDirectory
    );


// function definitions

/*******************************************************************
 UserExperienceParseFromXml - 

*******************************************************************/
extern "C" HRESULT UserExperienceParseFromXml(
    __in BURN_USER_EXPERIENCE* pUserExperience,
    __in IXMLDOMNode* pixnBundle
    )
{
    HRESULT hr = S_OK;
    IXMLDOMNode* pixnUserExperienceNode = NULL;

    // select UX node
    hr = XmlSelectSingleNode(pixnBundle, L"UX", &pixnUserExperienceNode);
    if (S_FALSE == hr)
    {
        hr = E_NOTFOUND;
    }
    ExitOnFailure(hr, "Failed to select user experience node.");

    // parse payloads
    hr = PayloadsParseFromXml(&pUserExperience->payloads, pixnUserExperienceNode);
    ExitOnFailure(hr, "Failed to parse user experience payloads.");

    // make sure we have at least one payload
    if (0 == pUserExperience->payloads.cPayloads)
    {
        hr = E_UNEXPECTED;
        ExitOnFailure(hr, "Too few UX payloads.");
    }

LExit:
    ReleaseObject(pixnUserExperienceNode);

    return hr;
}

/*******************************************************************
 UserExperienceUninitialize - 

*******************************************************************/
extern "C" void UserExperienceUninitialize(
    __in BURN_USER_EXPERIENCE* pUserExperience
    )
{
    ReleaseStr(pUserExperience->sczTempDirectory);
    PayloadsUninitialize(&pUserExperience->payloads);
}

/*******************************************************************
 UserExperienceLoad - 

*******************************************************************/
extern "C" HRESULT UserExperienceLoad(
    __in BURN_USER_EXPERIENCE* pUserExperience,
    __in BURN_CONTAINER_CONTEXT* pContainerContext,
    __in BURN_COMMAND* pCommand
    )
{
    HRESULT hr = S_OK;
    LPWSTR sczTempDirectory = NULL;

    // create temp directory path
    hr = PathCreateTempDirectory(NULL, L"UX%d", 999999, &pUserExperience->sczTempDirectory);
    ExitOnFailure(hr, "Failed to get unique temporary folder for UX.");

    // extract payloads to temp directory
    hr = ExtractUxPayloads(pContainerContext, &pUserExperience->payloads, pUserExperience->sczTempDirectory);
    ExitOnFailure(hr, "Failed to extract UX payloads.");

    // load UX DLL
    pUserExperience->hUxModule = ::LoadLibraryW(pUserExperience->payloads.rgPayloads[0].sczLocalFilePath);
    ExitOnNullWithLastError(pUserExperience->hUxModule, hr, "Failed to load UX DLL.");

    // get SetupUXCreate entry-point
    PFN_CREATE_USER_EXPERIENCE pfnSetupUXCreate = (PFN_CREATE_USER_EXPERIENCE)::GetProcAddress(pUserExperience->hUxModule, "SetupUXCreate");
    ExitOnNullWithLastError(pfnSetupUXCreate, hr, "Failed to get SetupUXCreate entry-point");

    // create UX
    hr = pfnSetupUXCreate(pCommand, &pUserExperience->pUserExperience);
    ExitOnFailure(hr, "Failed to create UX.");

LExit:
    return hr;
}

/*******************************************************************
 UserExperienceUnload - 

*******************************************************************/
extern "C" HRESULT UserExperienceUnload(
    __in BURN_USER_EXPERIENCE* pUserExperience
    )
{
    HRESULT hr = S_OK;

    if (pUserExperience->hUxModule)
    {
        // get SetupUXDestroy entry-point
        PFN_DESTROY_USER_EXPERIENCE pfnSetupUXDestroy = (PFN_DESTROY_USER_EXPERIENCE)::GetProcAddress(pUserExperience->hUxModule, "SetupUXDestroy");

        // destroy UX
        if (pfnSetupUXDestroy)
        {
            pfnSetupUXDestroy();
        }

        // free UX DLL
        ::FreeLibrary(pUserExperience->hUxModule);
    }

    ReleaseStr(pUserExperience->sczTempDirectory);

LExit:
    return hr;
}


// internal function definitions

static HRESULT ExtractUxPayloads(
    __in BURN_CONTAINER_CONTEXT* pContainerContext,
    __in BURN_PAYLOADS* pPayloads,
    __in LPCWSTR wzTempDirectory
    )
{
    HRESULT hr = S_OK;
    LPWSTR sczStreamName = NULL;
    LPWSTR sczFilePath = NULL;
    BURN_PAYLOAD* pPayload = NULL;

    // extract all payloads
    for (;;)
    {
        // get next stream
        hr = ContainerNextStream(pContainerContext, &sczStreamName);
        if (E_NOTFOUND == hr)
        {
            break;
        }
        ExitOnFailure(hr, "Failed to get next stream.");

        // find payload by stream name
        hr = PayloadFindEmbeddedBySourcePath(pPayloads, sczStreamName, &pPayload);
        ExitOnFailure1(hr, "Failed to find embedded payload: %S", pPayload->sczKey);

        // make file path
        hr = PathConcat(wzTempDirectory, pPayload->sczFilePath, &sczFilePath);
        ExitOnFailure(hr, "Failed to make file path.");

        // extract file
        hr = ContainerStreamToFile(pContainerContext, sczFilePath);
        ExitOnFailure(hr, "Failed to extract file.");
    }

    hr = S_OK;

LExit:
    ReleaseStr(sczStreamName);
    ReleaseStr(sczFilePath);

    return hr;
}
