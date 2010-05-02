#pragma once
//-------------------------------------------------------------------------------------------------
// <copyright file="CreateUserExperience.h" company="Microsoft">
//    Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// 
// <summary>
//    Definition of the global functions
// </summary>
//-------------------------------------------------------------------------------------------------

HRESULT CreateUserExperience(_In_ HMODULE hModule,
                             _In_ BURN_COMMAND* pCommand,
                             _Out_ IBurnUserExperience **ppUX);

#ifdef __cplusplus
extern "C" {
#endif

void SaveModuleHandle(_In_ HMODULE hModule);

#ifdef __cplusplus
}
#endif