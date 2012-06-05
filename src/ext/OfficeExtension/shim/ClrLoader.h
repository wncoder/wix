// <copyright file="ClrLoader.h" company="Microsoft Corporation">
//   Copyright (c) 2004, Microsoft Corporation.
//   This software is released under Common Public License Version 1.0 (CPL).
//   The license and further copyright text can be found in the file LICENSE.TXT
//   LICENSE.TXT at the root directory of the distribution.
// </copyright>
//
// <summary>
//  CLR Loader header.
// </summary>
//
#pragma once

extern "C" void WINAPI ClrLoaderInitialize();

extern "C" void WINAPI ClrLoaderUninitialize();

extern "C" HRESULT WINAPI ClrLoaderCreateInstance(
    __in_opt LPCWSTR wzClrVersion,
    __in LPCWSTR wzAssemblyName,
    __in LPCWSTR wzClassName,
    __in const IID &riid,
    __in void ** ppvObject
    );

extern "C" void WINAPI ClrLoaderDestroyInstance();
