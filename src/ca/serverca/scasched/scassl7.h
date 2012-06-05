#pragma once
//-------------------------------------------------------------------------------------------------
// <copyright file="scassl7.h" company="Microsoft Corporation">
//   Copyright (c) 2004, Microsoft Corporation.
//   This software is released under Common Public License Version 1.0 (CPL).
//   The license and further copyright text can be found in the file LICENSE.TXT
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
//
// <summary>
//    SSL functions for CustomActions
// </summary>
//-------------------------------------------------------------------------------------------------

// prototypes
HRESULT ScaSslCertificateWrite7(
    __in_z LPCWSTR wzWebBase,
    __in SCA_WEB_SSL_CERTIFICATE* pswscList
    );
