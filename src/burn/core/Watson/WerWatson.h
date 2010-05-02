//-------------------------------------------------------------------------------------------------
// <copyright file="WerWatson.h" company="Microsoft">
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
//
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once

//We are including this until there is a standard library or common header is available.
#include "werapidl.h"

#define DW_DUPLICATE_ACCESS (STANDARD_RIGHTS_REQUIRED  | PROCESS_QUERY_INFORMATION | PROCESS_TERMINATE | PROCESS_VM_READ | SYNCHRONIZE) &  \
                (~(WRITE_DAC | WRITE_OWNER | DELETE | GENERIC_ALL | ACCESS_SYSTEM_SECURITY))

namespace IronMan
{
class WerWatson
{
private:
    HINSTANCE                        g_hWerDll;
    WerReportCreate_t                g_pfnWerReportCreate;
    WerReportSetParameter_t          g_pfnWerReportSetParameter;
    WerReportAddFile_t               g_pfnWerReportAddFile;
    WerReportSetUIOption_t           g_pfnWerReportSetUIOption;
    WerReportAddSecondaryParameter_t g_pfnWerReportAddSecondaryParameter;
    WerReportSubmit_t                g_pfnWerReportSubmit;
    WerReportAddDump_t               g_pfnWerReportAddDump;
    WerReportCloseHandle_t           g_pfnWerReportCloseHandle;

public:
    //Constructor
    WerWatson()
        : g_hWerDll(NULL)
        , g_pfnWerReportCreate(NULL)
        , g_pfnWerReportSetParameter(NULL)
        , g_pfnWerReportAddFile(NULL)
        , g_pfnWerReportSetUIOption(NULL)
        , g_pfnWerReportSubmit(NULL)
        , g_pfnWerReportAddDump(NULL)
        , g_pfnWerReportCloseHandle(NULL)
    {}

    //Destructor
    virtual ~WerWatson()
    {
        FreeWerDll();
    }

    //7 steps to prepare and send Watson Report
    // Step 1: Create the Report
    // Step 2: Add dump if needed
    // Step 3: Populate the bucketting parameters
    // Step 4: Add the files to upload if it is not empty
    // Setp 5: Set dialog text if not queued.
    // Step 6: Send the Report
    // Step 7: Close the Report
    HRESULT SendReport( const CString& csApplicationName
                        , bool bShouldQueue
                        , const CString& csFileToUpload
                        , CString m_arstrParams[WER_MAX_PARAM_COUNT]
                        , EXCEPTION_POINTERS* pExceptionPointers
                        , const CString& csWatsonBoldText = L"")
    {
        //if we cannot load wer functions, exit
        if (S_OK != LoadWerDll())
        {
            return E_FAIL;
        }

        HRESULT hr = S_OK;
        HREPORT hReport = NULL;

        //Step 1 - Create the Report
        WER_REPORT_INFORMATION reportInformation = {0};
        reportInformation.dwSize = sizeof(reportInformation);        
        StringCchCopy(reportInformation.wzApplicationName, _countof(reportInformation.wzApplicationName), csApplicationName);
        StringCchCopy(reportInformation.wzApplicationPath, _countof(reportInformation.wzApplicationPath), ModuleUtils::GetDllPath());

        hr = WerReportCreate_V(L"VSSetup"             //Name of the event
                                , WerReportCritical     //Type of report. Set as critical so that it will not always queued.
                                , NULL                  //More information for the report.  
                                , &hReport);            //A handle to the report

        if (S_OK == hr)
        {
            //Step 2: Add dump if it is a crash
            if (NULL != pExceptionPointers)
            {
                // NOTE:  The duplicated handle does not need to be closed
                HANDLE handleDup;
                if (!DuplicateHandle(GetCurrentProcess()
                                    , GetCurrentProcess()
                                    , GetCurrentProcess()
                                    , &handleDup
                                    , DW_DUPLICATE_ACCESS
                                    , TRUE
                                    , 0))
                {
                    hr = E_FAIL;
                }
                else
                {
                    WER_EXCEPTION_INFORMATION exceptionInformation = {0};
                    exceptionInformation.pExceptionPointers = pExceptionPointers;
                    exceptionInformation.bClientPointers = TRUE;

                    WerReportAddDump_V( hReport                 //A handle to the report.
                                        , handleDup             //A handle to the process for which the report is being generated.
                                        , NULL                  //A handle to the thread.  NULL since not using WerDumpTypeMicro
                                        , WerDumpTypeMiniDump   //The type of minidump. 
                                        , &exceptionInformation //Specify exception information
                                        , NULL                  //NULL means use standard minidump
                                        , 0);                   //Behaviour flag
                    //Note: Ignore WerReportAddDump failure since we want at least the report.
                }
            }

            //Step 3: Populate the bucketting parameters.  Set only 9 parameters.
            for (DWORD dwLoop = 0; dwLoop < WER_MAX_PARAM_COUNT - 1; ++dwLoop)
            {   
                hr = WerReportSetParameter_V(hReport                      //A handle to the report
                                            , dwLoop                    //The identifier of the parameter to be set
                                            , NULL                      //A pointer to a Unicode string that contains the name of the event
                                            , m_arstrParams[dwLoop] );  //The parameter value.

                //No point to continue if we cannot the bucketting parameters.
                if (S_OK != hr)
                {
                    break;
                }
            }

            if (S_OK == hr)
            {
                if (csFileToUpload.GetLength() > 0)
                {
                    //Step 4: Add the files to upload
                    //Ignore the failure since we should still upload even if we have problem adding files.
                    AddFilesToReport(  hReport               //A handle to the report
                                    , csFileToUpload
                                    , WerFileTypeOther
                                    , 0);
                }

                // Step 5: Set the UI Text if not queued and it has been set.
                if (!bShouldQueue && csWatsonBoldText.GetLength() > 0)
                {
                    WerReportSetUIOption_V( hReport
                                            , WerUIConsentDlgHeader
                                            , csWatsonBoldText);

                    WerReportSetUIOption_V( hReport
                                            , WerUICloseDlgHeader
                                            , csWatsonBoldText);
                }

                // Step 6: Send the Report
                DWORD dwReportingFlags = WER_SUBMIT_ADD_REGISTERED_DATA;
                WER_SUBMIT_RESULT submitResult = WerReportFailed;
                if (bShouldQueue)
                {
                    dwReportingFlags |= WER_SUBMIT_QUEUE;
                }

                hr = WerReportSubmit_V(hReport                    //A handle to the report.
                                     , WerConsentNotAsked       //The consent status.
                                     , dwReportingFlags         //Misc behaviour Flags.
                                     , &submitResult);          //The result of the submission.                    
            }
        }
        // Step 7: Close the Report
        if (hReport)
        {
            WerReportCloseHandle_V(hReport);
            hReport = NULL;
            FreeWerDll();
        }
        return hr;
    }

//Make it protected and virtual for testing purpose
protected:
    virtual DWORD LoadWerDll()
    {
        if (g_hWerDll != NULL)
        {
            return ERROR_SUCCESS;
        }

        g_hWerDll = LoadLibrary(L"wer.dll");
        if (NULL == g_hWerDll)
        {
            return GetLastError();
        }

        g_pfnWerReportCreate = reinterpret_cast<WerReportCreate_t>(GetProcAddress(g_hWerDll, "WerReportCreate"));
        if (NULL == g_pfnWerReportCreate)
        {
            return GetLastError();
        }

        g_pfnWerReportSetParameter = reinterpret_cast<WerReportSetParameter_t>(GetProcAddress(g_hWerDll, "WerReportSetParameter"));
        if (NULL == g_pfnWerReportSetParameter)
        {
            return GetLastError();
        }

        g_pfnWerReportAddFile = reinterpret_cast<WerReportAddFile_t>(GetProcAddress(g_hWerDll, "WerReportAddFile"));
        if (NULL == g_pfnWerReportAddFile)
        {
            return GetLastError();
        }

        g_pfnWerReportSetUIOption = reinterpret_cast<WerReportSetUIOption_t>(GetProcAddress(g_hWerDll, "WerReportSetUIOption"));
        if (NULL == g_pfnWerReportSetUIOption)
        {
            return GetLastError();
        }

        g_pfnWerReportSubmit = reinterpret_cast<WerReportSubmit_t>(GetProcAddress(g_hWerDll, "WerReportSubmit"));
        if (NULL == g_pfnWerReportSubmit)
        {
            return GetLastError();
        }

        g_pfnWerReportAddDump = reinterpret_cast<WerReportAddDump_t>(GetProcAddress(g_hWerDll, "WerReportAddDump"));
        if (NULL == g_pfnWerReportAddDump)
        {
            return GetLastError();
        }

        g_pfnWerReportCloseHandle = reinterpret_cast<WerReportCloseHandle_t>(GetProcAddress(g_hWerDll, "WerReportCloseHandle"));
        if (NULL == g_pfnWerReportCloseHandle)
        {
            return GetLastError();
        }

        return ERROR_SUCCESS;
    }

    virtual HRESULT WerReportCreate_V(PCWSTR pwzEventType
                                    , WER_REPORT_TYPE repType
                                    , PWER_REPORT_INFORMATION pReportInformation
                                    , HREPORT *phReportHandle)
    {
        return WerReportCreate(pwzEventType, repType, pReportInformation, phReportHandle);
    }

    virtual HRESULT WerReportSetParameter_V ( HREPORT hReportHandle
                                            , DWORD dwparamID
                                            , PCWSTR pwzName
                                            , PCWSTR pwzValue)
    {
        return WerReportSetParameter(hReportHandle, dwparamID, pwzName, pwzValue);
    }

    virtual HRESULT WerReportSubmit_V (HREPORT hReportHandle
                                        , WER_CONSENT consent
                                        , DWORD  dwFlags
                                        , PWER_SUBMIT_RESULT pSubmitResult)
    {
        return WerReportSubmit(hReportHandle, consent, dwFlags, pSubmitResult);
    }

    virtual HRESULT WerReportAddDump_V(HREPORT hReportHandle
                                        , HANDLE  hProcess
                                        , HANDLE hThread
                                        , WER_DUMP_TYPE dumpType
                                        , PWER_EXCEPTION_INFORMATION pExceptionParam
                                        , PWER_DUMP_CUSTOM_OPTIONS pDumpCustomOptions
                                        , DWORD dwFlags)
    {
        return WerReportAddDump(hReportHandle, hProcess, hThread, dumpType, pExceptionParam, pDumpCustomOptions, dwFlags);
    }

    virtual HRESULT WerReportCloseHandle_V(HREPORT hReportHandle)
    {
        return WerReportCloseHandle(hReportHandle);
    }

    virtual HRESULT WerReportAddFile_V(HREPORT hReportHandle
                                    , PCWSTR pwzPath
                                    , WER_FILE_TYPE repFileType
                                    , DWORD  dwFileFlags)
    {
        return WerReportAddFile(hReportHandle, pwzPath, repFileType, dwFileFlags);
    }

    virtual HRESULT  WerReportSetUIOption_V(HREPORT hReportHandle
                                            , WER_REPORT_UI repUITypeID
                                            , PCWSTR pwzValue)
    {
        return WerReportSetUIOption(hReportHandle, repUITypeID, pwzValue);
    }

    void FreeWerDll()
    {
        if (g_hWerDll != NULL)
        {
            FreeLibrary(g_hWerDll);
            g_hWerDll = NULL;
        }

        g_pfnWerReportCreate = NULL;
        g_pfnWerReportSetParameter = NULL;
        g_pfnWerReportAddFile = NULL;
        g_pfnWerReportSetUIOption = NULL;
        g_pfnWerReportAddSecondaryParameter = NULL;
        g_pfnWerReportSubmit = NULL;
        g_pfnWerReportAddDump = NULL;
        g_pfnWerReportCloseHandle = NULL;
    }

    //Add the files to be uploaded
    //We are going to ignore all failure to ensure 
    // a. we are still uploading Watson
    // b. we are getting at least something.
    HRESULT AddFilesToReport(HREPORT hReport
                            , const CString& pwszList
                            , WER_FILE_TYPE fileType 
                            , DWORD dwFileFlagIN)
    {
        //Note: Parameter checking has been assumed to be done by calling program.

        CString strFileName;
        PCWSTR pwszIndex = NULL, pwszNext = NULL;
        DWORD dwFileFlag = dwFileFlagIN;

        pwszIndex = pwszList;
        while((pwszNext = wcschr(pwszIndex, L'|')) != NULL)
        {
            //Ignore fialure, since this is a best effort 
            strFileName = CString (pwszIndex, (int)(pwszNext - pwszIndex));
            dwFileFlag = dwFileFlagIN;
            WerReportAddFile_V(hReport
                            , strFileName
                            , fileType
                            , dwFileFlag);
            pwszIndex = pwszNext + 1;
        }

        if (pwszIndex)
        {
            //Ignore fialure, since this is a best effort 
            WerReportAddFile_V(hReport
                            , pwszIndex
                            , fileType
                            , dwFileFlag);
        }
        return S_OK;
    }
};
} //namespace
