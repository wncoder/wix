#pragma once
//-------------------------------------------------------------------------------------------------
// <copyright file="balretry.h" company="Microsoft">
//    Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//
// <summary>
// Bootstrapper Application Layer retry utility.
// </summary>
//-------------------------------------------------------------------------------------------------


#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************
 BalRetryInitialize - initialize the retry count and timeout between
                      retries (in milliseconds).
********************************************************************/
void BalRetryInitialize(
    __in DWORD dwMaxRetries,
    __in DWORD dwTimeout
    );

/*******************************************************************
 BalRetryUninitialize - call to cleanup any memory allocated during
                        use of the retry utility.
********************************************************************/
void BalRetryUninitialize();

/*******************************************************************
 BalRetryStartPackage - call when a package begins to be modified. If
                        the package is being retried, the function will
                        wait the specified timeout.
********************************************************************/
void BalRetryStartPackage(
    __in_z_opt LPCWSTR wzPackageId
    );

/*******************************************************************
 BalRetryOnError - call when an error occurs for the retry utility to
                   consider.
********************************************************************/
void BalRetryOnError(
    __in_z_opt LPCWSTR wzPackageId,
    __in DWORD dwError
    );

/*******************************************************************
 BalRetryEndPackage - returns IDRETRY is a retry is recommended or 
                      IDNOACTION if a retry is not recommended.
********************************************************************/
int BalRetryEndPackage(
    __in_z_opt LPCWSTR wzPackageId,
    __in HRESULT hrError
    );


#ifdef __cplusplus
}
#endif
