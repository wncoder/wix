#pragma once
//-------------------------------------------------------------------------------------------------
// <copyright file="CreateUserExperience.h" company="Microsoft">
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