//-------------------------------------------------------------------------------------------------
// <copyright file="FirstError.h" company="Microsoft">
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
//    The class for preserving the correct error.
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once

#include "interfaces\ILogger.h"
#include "LogSignatureDecorator.h"
#include "common\MsiUtils.h"

namespace IronMan
{

    //Below is how this class will proritize the error being set. 
    // 1.  Abort
    // 2.  All other error
    // 3.  ERROR_SUCCESS_REBOOT_REQUIRED
    // 4.  S_OK
    class FirstError
    {
        ILogger& m_logger;
        HRESULT m_hr;
        bool m_bAbort;

    public:
        FirstError(ILogger& logger)
            : m_hr(S_OK)
            , m_bAbort(false)
            , m_logger(logger)
        {}

        bool SetError(const HRESULT hr, const bool bIgnoreDownloadFailure)
        {
            bool bErrorWasSet = false;
            HRESULT hrLocal = hr;
            if (bIgnoreDownloadFailure && FAILED(hrLocal))
            {		
                CString csErrorMessage;
                csErrorMessage.Format(L"The original error is %u and has been over-written with S_OK because the IgnoreDownloadFailure attribute is set to true.", hr);
                LOG(m_logger, ILogger::Verbose, csErrorMessage);
                hrLocal = S_OK;
            }

            if (FAILED(hrLocal))
            {
                bErrorWasSet = SetError(hrLocal);
            }
            return bErrorWasSet;
        }

        //Limit this to be used by Custom Error Handler only.
        void OverwriteCurrentError(const HRESULT hr)
        {
            if (!m_bAbort)
            {
                m_hr = hr;
            }
        }

        // Used by CompositePerformer (Installer) to set error irrespective of abort
        // Did not combine with SetError() for the sake of clarity/redability
        bool SetErrorWithAbort(const HRESULT hr)
        {
            bool bErrorWasSet = false;
            if (S_OK != hr)
            {
                //Reboot Required is set only when the last error is S_OK since
                if ( (HRESULT_FROM_WIN32(ERROR_SUCCESS_REBOOT_REQUIRED) == hr) ||
                     (ERROR_SUCCESS_REBOOT_REQUIRED == HRESULT_CODE(hr)))
                {
                    RequestReboot();
                }
                else
                {
                    m_hr = hr;
                    bErrorWasSet = true;
                }
            }
            return bErrorWasSet;
        }

        // Called by Composite Performer (Installer and Downloader)
        bool SetError(const HRESULT hr)
        {
            bool bErrorWasSet = false;
            if ((MSIUtils::IsSuccess(HRESULT_FROM_WIN32(m_hr))) && (!IsAbort()))
            {
                if (S_OK != hr)
                {
                    //Reboot Required is set only when the last error is S_OK since
                    //we should not overwrite E_ABORT and other error 
                    if ( (HRESULT_FROM_WIN32(ERROR_SUCCESS_REBOOT_REQUIRED) == hr) ||
                         (ERROR_SUCCESS_REBOOT_REQUIRED == HRESULT_CODE(hr)))
                    {
                        RequestReboot();
                    }
                    else
                    {
                        m_hr = hr;
                        bErrorWasSet = true;
                    }
                }
            }
             return bErrorWasSet;
        }

        // Unless there is already Reboot pending, this will reset the code to S_OK
        void ClearError()
        {
            if (!m_bAbort && m_hr != HRESULT_FROM_WIN32(ERROR_SUCCESS_REBOOT_REQUIRED))
            {
                m_hr = S_OK;
            }
        }

        bool IsError() const
        {
            if ((MSIUtils::IsSuccess(HRESULT_FROM_WIN32(m_hr))) || (IsAbort()))
            {
                return false;
            }
            else
            {
                return true;
            }
        }

        bool IsReboot() const
        {
            return (m_hr == HRESULT_FROM_WIN32(ERROR_SUCCESS_REBOOT_REQUIRED));
        }

        HRESULT GetError() const
        {
            return m_hr;
        }
        
        void Abort()
        {
            m_hr = E_ABORT;
            m_bAbort = true;
        }

        const bool IsAbort() const
        {
            return (m_bAbort || m_hr == E_ABORT);
        }

        bool IsErrorCancelled()
        {
            return (HRESULT_FROM_WIN32(ERROR_INSTALL_USEREXIT) == m_hr);
        }

        void RequestReboot()
        {
            if (!m_bAbort)
            {
                m_hr = HRESULT_FROM_WIN32(ERROR_SUCCESS_REBOOT_REQUIRED);
            }
        }
    };
}
