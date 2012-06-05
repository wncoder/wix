#pragma once
//-------------------------------------------------------------------------------------------------
// <copyright file="scaweberr7.h" company="Microsoft Corporation">
//   Copyright (c) 2004, Microsoft Corporation.
//   This software is released under Common Public License Version 1.0 (CPL).
//   The license and further copyright text can be found in the file LICENSE.TXT
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
//
// <summary>
//    IIS Web Error functions for CustomActions
// </summary>
//-------------------------------------------------------------------------------------------------

// prototypes

HRESULT ScaWriteWebError7(
    __in_z LPCWSTR wzWebName,
    __in_z LPCWSTR wzRoot,
    SCA_WEB_ERROR* psweList
    );

