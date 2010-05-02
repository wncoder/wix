//-------------------------------------------------------------------------------------------------
// <copyright file="Werapidl.h" company="Microsoft">
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
//    This file contains type and variable declarations to call the 
//    Windows Error Reporting (WER) dynamically
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once

#include "werapi.h"

typedef HRESULT (WINAPI *WerReportCreate_t)(
    __in  PCWSTR pwzEventType, 
    __in WER_REPORT_TYPE repType,
    __in_opt PWER_REPORT_INFORMATION pReportInformation,
    __out HREPORT *phReportHandle
    );

extern WerReportCreate_t g_pfnWerReportCreate;
#define WerReportCreate (*g_pfnWerReportCreate)
    
typedef HRESULT (WINAPI *WerReportSetParameter_t)(
    __in  HREPORT hReportHandle, 
    __in  DWORD dwparamID, 
    __in_opt  PCWSTR pwzName,
    __in  PCWSTR pwzValue
    );

extern WerReportSetParameter_t g_pfnWerReportSetParameter;
#define WerReportSetParameter (*g_pfnWerReportSetParameter)
    
typedef HRESULT (WINAPI *WerReportAddFile_t)(
    __in  HREPORT hReportHandle, 
    __in  PCWSTR pwzPath, 
    __in  WER_FILE_TYPE repFileType,
    __in  DWORD  dwFileFlags
    );

extern WerReportAddFile_t g_pfnWerReportAddFile;
#define WerReportAddFile (*g_pfnWerReportAddFile)

typedef HRESULT  (WINAPI *WerReportSetUIOption_t)(
    __in  HREPORT hReportHandle, 
    __in  WER_REPORT_UI repUITypeID, 
    __in  PCWSTR pwzValue
    );

extern WerReportSetUIOption_t g_pfnWerReportSetUIOption;
#define WerReportSetUIOption (*g_pfnWerReportSetUIOption)

typedef HRESULT (WINAPI *WerReportAddSecondaryParameter_t)(
    __in  HREPORT hReportHandle, 
    __in  PCWSTR pwzKey, 
    __in_opt PCWSTR pwzValue
    );

extern WerReportAddSecondaryParameter_t g_pfnWerReportAddSecondaryParameter;
#define WerReportAddSecondaryParameter (*g_pfnWerReportAddSecondaryParameter)

typedef HRESULT (WINAPI *WerReportSubmit_t)(
    __in HREPORT hReportHandle,
    __in WER_CONSENT consent,
    __in DWORD  dwFlags,
    __out_opt PWER_SUBMIT_RESULT pSubmitResult
    );

extern WerReportSubmit_t g_pfnWerReportSubmit;
#define WerReportSubmit (*g_pfnWerReportSubmit)

typedef HRESULT  (WINAPI *WerReportAddDump_t)(
    __in HREPORT hReportHandle, 
    __in HANDLE  hProcess,
    __in_opt HANDLE hThread,
    __in WER_DUMP_TYPE dumpType,
    __in_opt  PWER_EXCEPTION_INFORMATION pExceptionParam,
    __in_opt PWER_DUMP_CUSTOM_OPTIONS pDumpCustomOptions,
    __in DWORD dwFlags
    );

extern WerReportAddDump_t g_pfnWerReportAddDump;
#define WerReportAddDump (*g_pfnWerReportAddDump)

typedef HRESULT (WINAPI *WerReportCloseHandle_t)(
    __in HREPORT hReportHandle
    );

extern WerReportCloseHandle_t g_pfnWerReportCloseHandle;
#define WerReportCloseHandle (*g_pfnWerReportCloseHandle)
