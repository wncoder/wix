#pragma once
//-------------------------------------------------------------------------------------------------
// <copyright file="deputil.h" company="Microsoft">
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
//    Common function declarations for the dependency/ref-counting feature.
// </summary>
//-------------------------------------------------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif

#define ReleaseDependencyArray(rg, c) if (rg) { DependencyArrayFree(rg, c); }
#define ReleaseNullDependencyArray(rg, c) if (rg) { DependencyArrayFree(rg, c); rg = NULL; }

typedef struct _DEPENDENCY
{
    LPWSTR sczKey;
    LPWSTR sczName;

} DEPENDENCY, *PDEPENDENCY;


/***************************************************************************
 CheckDependencies - Checks that all dependencies are registered
  and within the proper version range.

 Note: Returns S_FALSE if the authored dependencies were not found.
***************************************************************************/
DAPI_(HRESULT) CheckDependencies(
    __in HKEY hkHive,
    __in_z LPCWSTR wzProviderKey,
    __in_z_opt LPCWSTR wzMinVersion,
    __in_z_opt LPCWSTR wzMaxVersion,
    __in int iAttributes,
    __in STRINGDICT_HANDLE sdDependencies,
    __deref_inout_ecount_opt(*pcDependencies) PDEPENDENCY *prgDependencies,
    __inout LPUINT pcDependencies
    );

/***************************************************************************
 CheckDependents - Checks if any dependents are still installed for the
  given provider key.

 Notes: Returns S_FALSE if no authored dependents were found.
***************************************************************************/
DAPI_(HRESULT) CheckDependents(
    __in HKEY hkHive,
    __in_z LPCWSTR wzProviderKey,
    __in int iAttributes,
    __in C_STRINGDICT_HANDLE sdIgnoredDependents,
    __deref_inout_ecount_opt(*pcDependents) PDEPENDENCY *prgDependents,
    __inout LPUINT pcDependents
    );

/***************************************************************************
 DependencyArrayFree - Frees an array of DEPENDENCY structs.

***************************************************************************/
DAPI_(void) DependencyArrayFree(
    __in_ecount(cDependencies) PDEPENDENCY rgDependencies,
    __in UINT cDependencies
    );

#ifdef __cplusplus
}
#endif
