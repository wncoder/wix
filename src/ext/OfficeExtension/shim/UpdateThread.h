// <copyright file="UpdateThread.h" company="Microsoft Corporation">
//   Copyright (c) 2004, Microsoft Corporation.
//   This software is released under Common Public License Version 1.0 (CPL).
//   The license and further copyright text can be found in the file LICENSE.TXT
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
//
// <summary>
//  Background thread for doing update checks header.
// </summary>
//
#pragma once

extern "C" void WINAPI UpdateThreadInitialize();

extern "C" void WINAPI UpdateThreadUninitialize();

extern "C" HRESULT WINAPI UpdateThreadCheck(
    __in LPCWSTR wzAppId,
    __in BOOL fTryExecuteUpdate
    );
