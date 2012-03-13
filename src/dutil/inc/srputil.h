//-------------------------------------------------------------------------------------------------
// <copyright file="srputil.h" company="Microsoft">
//    Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//
// <summary>
//  System restore point helper functions.
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once


#ifdef __cplusplus
extern "C" {
#endif


enum SRP_ACTION
{
    SRP_ACTION_UNKNOWN,
    SRP_ACTION_UNINSTALL,
    SRP_ACTION_INSTALL,
    SRP_ACTION_MODIFY,
};


/********************************************************************
 SrpInitialize - initializes system restore point functionality.

*******************************************************************/
DAPI_(HRESULT) SrpInitialize(
    __in BOOL fInitializeComSecurity
    );

/********************************************************************
 SrpUninitialize - uninitializes system restore point functionality.

*******************************************************************/
DAPI_(void) SrpUninitialize();

/********************************************************************
 SrpCreateRestorePoint - creates a system restore point.

*******************************************************************/
DAPI_(HRESULT) SrpCreateRestorePoint(
    __in_z LPCWSTR wzApplicationName,
    __in SRP_ACTION action
    );

#ifdef __cplusplus
}
#endif

