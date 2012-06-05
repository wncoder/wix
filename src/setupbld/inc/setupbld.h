//-------------------------------------------------------------------------------------------------
// <copyright file="setupbld.h" company="Microsoft Corporation">
//   Copyright (c) 2004, Microsoft Corporation.
//   This software is released under Common Public License Version 1.0 (CPL).
//   The license and further copyright text can be found in the file LICENSE.TXT
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
//
// <summary>
// Setup executable builder header for ClickThrough.
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once


typedef struct
{
    LPCWSTR wzSourcePath;

    BOOL fPrivileged;
    BOOL fCache;
    BOOL fShowUI;
    BOOL fIgnoreFailures;
    BOOL fMinorUpgrade;
    BOOL fLink;
    BOOL fUseTransform;
    BOOL fPatch;
    BOOL fPatchForceTarget;
} CREATE_SETUP_PACKAGE;

typedef struct
{
    LPCWSTR wzSourcePath;
    DWORD dwLocaleId;
} CREATE_SETUP_TRANSFORMS;
