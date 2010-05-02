//-------------------------------------------------------------------------------------------------
// <copyright file="LogUtils.h" company="Microsoft">
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

#include "interfaces\ILogger.h"
#include "LogSignatureDecorator.h"
#include "SystemUtil.h"
#include "msiutils.h"

namespace IronMan
{

// These need to be used so the __FUNCTION__ is the calling function, not LogUtils::LogLastError
// Use this one if you want to pass the error in
#define LOGERROR(logger, loggingLevel, failedFunction, dwLastError) IronMan::LogUtils::LogLastError(logger, loggingLevel, failedFunction, __FUNCTION__, dwLastError)
// use this one if you want to log the return value of GetLastError()
#define LOGGETLASTERROR(logger, loggingLevel, failedFunction) IronMan::LogUtils::LogLastError(logger, loggingLevel, failedFunction, __FUNCTION__ )

//------------------------------------------------------------------------------
// Struct: IronMan::LogUtils
// Utility class to help with logging
//------------------------------------------------------------------------------
struct LogUtils
{
    //------------------------------------------------------------------------------
    // GenerateLogFileName
    //
    // Generates a log file name based on the first prefix name passed in.
    // with a date stamp of the form _YYYYMMDD_HHMMSSmmm e.g. _20080803_143510032
    // and without an extension, because diffrent types of logger can add their
    // own extension. The path part is set to %TEMP%\
    //-------------------------------------------------------------------------------
    static CString GenerateLogFileName(const CString & strName, const CString & strFolder = CString(L"%TEMP%\\") )
    {
        CString strGeneratedFileName;

        SYSTEMTIME systemTime = {0};
        ::GetLocalTime(&systemTime);

        strGeneratedFileName.FormatMessageW(L"%1!s!_%2!04d!%3!02d!%4!02d!_%5!02d!%6!02d!%7!02d!%8!03d!",
            strName, systemTime.wYear,systemTime.wMonth,systemTime.wDay,
            systemTime.wHour,systemTime.wMinute,systemTime.wSecond,systemTime.wMilliseconds);

        CPath pthGeneratedPath;
        pthGeneratedPath.Combine(strFolder,strGeneratedFileName);
        strGeneratedFileName = CString(pthGeneratedPath);

        return strGeneratedFileName;
    }

    // Don't use this directly, use the macro LOGLASTERROR to automatically get the calling function name
    static HRESULT LogLastError(ILogger& logger
                            , ILogger::LoggingLevel loggingLevel
                            , LPCTSTR failedFunction
                            , LPCSTR callingFunctionName
                            , DWORD dwLastError = GetLastError() )
    {
#ifdef IRONMAN_FUNCTIONNAMEINLOGFILE
            CString message( LogHelper::DecorateFunctionName(CString(callingFunctionName)) );
#else
            CString message = L"";
#endif
            message.AppendFormat(L"%s failed with error: %u", failedFunction, dwLastError);
            logger.Log(loggingLevel, message);
            return (dwLastError ? HRESULT_FROM_WIN32(dwLastError) : E_FAIL);  // in case GetLastError() returns 0, we still need to fail.
    }

    //------------------------------------------------------------------------------
    // LogFinalResult
    //
    // Log the final result for summary.
    //
    //------------------------------------------------------------------------------
    static void LogFinalResult(HRESULT hr, ILogger& logger, const CTime &startTime)
    {
        CString cs;
        //To log the final result.
        if (MSIUtils::IsSuccess(hr))
        {           
            CComBSTR bstrError;
            CSystemUtil::GetErrorString(hr, bstrError);
            if ( 0 == bstrError.Length() )
            {
                cs.Format(L"Final Result: Installation completed successfully with success code: (0x%08lX)", hr);           
            }
            else
            {
                cs.Format(L"Final Result: Installation completed successfully with success code: (0x%08lX), \"%s\"", hr, bstrError);           
            }

        }
        else if (E_ABORT == hr)
        {
            cs = L"Final Result: Installation aborted";
        }
        else
        {
            CComBSTR bstrError;
            CSystemUtil::GetErrorString(hr, bstrError);
            if ( 0 == bstrError.Length() )
            {
                // in case of an error that cannot be formatted, refer the user to the log
                cs.Format(L"Final Result: Installation failed with error code: (0x%08lX)", hr);           
            }
            else
            {
                cs.Format(L"Final Result: Installation failed with error code: (0x%08lX), \"%s\"", hr, bstrError);           
            }

        }

        // Record elapsed time if valid start time passed in
        CString strElapsedTime(L".");
        if (startTime.GetTime())
        {
            CTime endTime = CTime::GetCurrentTime();
            // Figure out the elapsed time 
            CTimeSpan elapsedTime = endTime - startTime;
            strElapsedTime = elapsedTime.Format(L" (Elapsed time: %D %H:%M:%S).");
        }

        logger.LogFinalResult(cs + strElapsedTime);
    }
};

}
