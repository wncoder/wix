#pragma once
//-------------------------------------------------------------------------------------------------
// <copyright file="scawebprop7.h" company="Microsoft Corporation">
//   Copyright (c) 2004, Microsoft Corporation.
//   This software is released under Common Public License Version 1.0 (CPL).
//   The license and further copyright text can be found in the file LICENSE.TXT
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
//
// <summary>
//    IIS Web Directory Property functions for CustomActions
// </summary>
//-------------------------------------------------------------------------------------------------

#include "scauser.h"

HRESULT ScaWriteWebDirProperties7(
    __in_z LPCWSTR wzwWebName,
    __in_z LPCWSTR wzRootOfWeb,
    const SCA_WEB_PROPERTIES* pswp
    );

