//-------------------------------------------------------------------------------------------------
// <copyright file="userexperience.h" company="Microsoft">
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

#pragma once


#if defined(__cplusplus)
extern "C" {
#endif


// structs

typedef struct _BURN_USER_EXPERIENCE
{
    BURN_PAYLOADS payloads;

    HMODULE hUxModule;
    IBurnUserExperience* pUserExperience;
    LPWSTR sczTempDirectory;
} BURN_USER_EXPERIENCE;


// functions

HRESULT UserExperienceParseFromXml(
    __in BURN_USER_EXPERIENCE* pUserExperience,
    __in IXMLDOMNode* pixnBundle
    );
void UserExperienceUninitialize(
    __in BURN_USER_EXPERIENCE* pUserExperience
    );
HRESULT UserExperienceLoad(
    __in BURN_USER_EXPERIENCE* pUserExperience,
    __in BURN_CONTAINER_CONTEXT* pContainerContext,
    __in BURN_COMMAND* pCommand
    );
HRESULT UserExperienceUnload(
    __in BURN_USER_EXPERIENCE* pUserExperience
    );


#if defined(__cplusplus)
}
#endif
