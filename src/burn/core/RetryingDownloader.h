//-------------------------------------------------------------------------------------------------
// <copyright file="RetryingDownloader.h" company="Microsoft">
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

#include "WinHttpDownloader.h"
#include "BitsDownloader.h"
#include "UrlMonDownloader.h"
#include "common\ResultObserver.h"
#include "interfaces\IPerformer.h"
#include "CheckTrust.h"
#include "Ux\Ux.h"

namespace IronMan
{

template<typename HttpDownloader, typename BitsDownloader, typename UrlMonDownloader, typename FileAuthenticityT = FileAuthenticity, typename Ux = Ux> // for easy mocking
class RetryingDownloaderT : public AbortPerformerBase
{
    ILogger& m_logger;
    int m_currentRetry;
    int m_retries, m_secondsToWait;
    CUrl* m_src; // held by pointer to overcome PREfast error 6262 ("function uses 18278 bytes of stack")
    ULONGLONG m_itemSize;
    CString m_hash;
    CPath m_dst;
    CString m_friendlyName;
    IPerformer* m_currentPerformer;
    Ux& m_uxLogger;
    CString m_packageId;
    IBurnView *m_pBurnView;
    UxEnum::spigotDownloadProtocolEnum& m_protocolCurrent;  //Keep to pass by reference as it is also the last known good.

public:
    RetryingDownloaderT(const CUrl& src
                        , const CString& friendlyName
                        , const CString& hash
                        , const ULONGLONG itemSize
                        , const CPath& dst
                        , int retries
                        , int secondsToWait
                        , UxEnum::spigotDownloadProtocolEnum& protocol
                        , ILogger& logger
                        , Ux& uxLogger
                        , const CString& packageId = L""
                        , IBurnView *pBurnView = NULL)

        : m_logger(logger)
        , m_retries(retries)
        , m_secondsToWait(secondsToWait)
        , m_src(new CUrl(src))
        , m_friendlyName(friendlyName)
        , m_hash(hash)
        , m_itemSize(itemSize)
        , m_dst(dst)
        , m_protocolCurrent(protocol)
        , m_currentPerformer(&NullPerformer::GetNullPerformer())
        , m_uxLogger(uxLogger)
        , m_currentRetry(0)
        , m_packageId(packageId)
        , m_pBurnView(pBurnView)
    {}
    virtual ~RetryingDownloaderT()
    {
        delete m_src;
    }

    //Expose the current retry count so that cartman can use it to report status
    int GetCurrentRetry()
    {
        int currentRetry = m_currentRetry;
        if (currentRetry == (m_retries + 1))
        {
            --currentRetry;
        }

        return currentRetry;
    }

public: // IPerformer
    virtual void PerformAction(IProgressObserver& observer)
    {
        HRESULT hr = E_FAIL;
        ResultObserver resultObserver(observer, hr);

        CString fullUrl;
        DWORD dwUrlLength = ATL_URL_MAX_URL_LENGTH;
        fullUrl.Preallocate(ATL_URL_MAX_URL_LENGTH+2);
        m_src->CreateUrl(fullUrl.GetBuffer(),&dwUrlLength);

        CString section = L" complete";
        PUSHLOGSECTIONPOP(m_logger, L"Action", L"Downloading Item " + fullUrl, section);

        //Each retry means try all 3 protocols
        for(m_currentRetry = 0; (m_pBurnView && S_OK != hr && !HasAborted() && !HRESULT_IS_NETPATH_NOT_FOUND(hr)) || (m_currentRetry < m_retries + 1 && S_OK != hr && !HasAborted() && !HRESULT_IS_NETPATH_NOT_FOUND(hr)); ++m_currentRetry)
        {
            for (int iprotocol = 0; iprotocol < UxEnum::spdLAST_PROTOCOL && !HasAborted(); ++iprotocol)
            {
                //Delay before next retry.  Checking here as we don't want to wait on the last error.
                Wait(iprotocol);

                //Determine protocol
                if (!(0 == m_currentRetry && 0 == iprotocol))  //The first try, we don't have to increment.
                {
                    m_protocolCurrent = static_cast<UxEnum::spigotDownloadProtocolEnum>
                                                ( ( ( static_cast<int>(m_protocolCurrent) + 1 ) % UxEnum::spdNUM_PROTOCOLS) );
                }

                LOGEX(  m_logger
                        , ILogger::Information
                        , L"Starting download attempt %d of %d for %s using %s"
                        , m_currentRetry + 1
                        , m_retries + 1
                        , fullUrl
                        , UxEnum::GetProtocolString(m_protocolCurrent));

                DownloadInformation downloadInformation(*m_src, m_dst, m_friendlyName, (m_currentRetry * UxEnum::spdLAST_PROTOCOL) + iprotocol);
                switch(m_protocolCurrent)
                {
                //None is equivalent to BITS here.
                case UxEnum::sdpNone:
                    m_protocolCurrent = UxEnum::sdpBits;
                case UxEnum::sdpBits:
                    {
                        //Don't even bother trying bits if it is not available.
                        if (!BitsDownloader::IsAvailable(m_logger, m_uxLogger))
                        {
                            LOG(m_logger, ILogger::Information, L"BITS service not available");
                            continue;
                        }
                        BitsDownloader bd(downloadInformation, m_logger, m_uxLogger);
                        m_currentPerformer = &bd;
                        bd.PerformAction(resultObserver);
                        break;
                    }
                case UxEnum::sdpHttp:
                    {
                        HttpDownloader hd(downloadInformation, m_logger, m_uxLogger);
                        m_currentPerformer = &hd;
                        hd.PerformAction(resultObserver);
                        break;
                    }
                case UxEnum::sdpUrlMon:
                    {
                        UrlMonDownloader ud(downloadInformation, m_logger, m_uxLogger);
                        m_currentPerformer = &ud;
                        ud.PerformAction(resultObserver);
                    }
                }
                m_currentPerformer = &NullPerformer::GetNullPerformer();

                //Verify the payload file
                if (SUCCEEDED(hr) && !HasAborted())
                {
                    hr = VerifyPayload();
                }

                if (m_pBurnView && !((SUCCEEDED(hr) || HRESULT_IS_NETPATH_NOT_FOUND(hr)) || HasAborted()))
                {
                    int nResult = m_pBurnView->OnError(m_packageId, hr, NULL, MB_ICONERROR | MB_RETRYCANCEL);
                    if (nResult != IDRETRY)
                    {
                        Abort();
                    }
                }

                if (HasAborted())
                {
                    LOGEX(m_logger
                        , ILogger::Information
                        , L"User cancelled download attempt %d of %d for %s using %s"
                        , m_currentRetry + 1
                        , m_retries + 1
                        , fullUrl
                        , UxEnum::GetProtocolString(m_protocolCurrent));
                    break;
                }
                else if (SUCCEEDED(hr))
                {
                    LOGEX(m_logger
                        , ILogger::Information
                        , L"Download succeeded at attempt %d of %d for %s using %s"
                        , m_currentRetry + 1
                        , m_retries + 1
                        , fullUrl
                        , UxEnum::GetProtocolString(m_protocolCurrent));
                    break;
                }
                else if (HRESULT_IS_NETPATH_NOT_FOUND(hr))
                {
                    LOGEX(m_logger
                        , ILogger::Information
                        , L"Download path not found at attempt %d of %d for %s using %s"
                        , m_currentRetry + 1
                        , m_retries + 1
                        , fullUrl
                        , UxEnum::GetProtocolString(m_protocolCurrent));
                    break;
                }
                else
                {
                    LOGEX(m_logger
                        , ILogger::Information
                        , L"Download failed at attempt %d of %d for %s using %s"
                        , m_currentRetry + 1
                        , m_retries + 1
                        , fullUrl
                        , UxEnum::GetProtocolString(m_protocolCurrent));
                }
            }
        }
        observer.Finished(HasAborted() ? E_ABORT : hr);
    }

    virtual void Abort()
    {
        AbortPerformerBase::Abort();
        m_currentPerformer->Abort();
    }

    HRESULT VerifyPayload()
    {
        HRESULT hr = S_OK;

        m_uxLogger.StartRecordingItem(m_friendlyName 
                                        , UxEnum::pDownload
                                        , UxEnum::aVerify
                                        , UxEnum::tNone);

        FileAuthenticityT fileAuthenticity(m_friendlyName, m_dst, m_hash, m_itemSize, m_logger);
        hr = fileAuthenticity.Verify();
        m_uxLogger.StopRecordingItem(m_friendlyName
                                    , 0     // Size
                                    , hr    // Result
                                    , L""   // Detail
                                    , 0);   // Retry count
        if (FAILED(hr))
        {
            BOOL result = ::DeleteFile(m_dst);
            if (false == result)
            {
                DWORD err = ::GetLastError();
                LOG(m_logger, ILogger::Information, L"Failed to delete invalid file");
            }
        }

        return hr;
    }

private:
    void Wait(int iCounter)
    {
        if (0 != (iCounter + m_currentRetry))
        {
            // wait before retrying (polling for abort)
            for (int j = 0; j < m_secondsToWait * 1000 && !HasAborted(); j += 100)
            {
                // poll 10 times per second
                Sleep(100); // matches
            }
        }
    }

private:  // "test subclass" test hook
    virtual void Sleep(__in DWORD dwMilliseconds) { ::Sleep(dwMilliseconds); }
};

typedef RetryingDownloaderT<WinHttpDownloader, BitsDownloader, UrlMonDownloader, FileAuthenticity> RetryingDownloader; // test hook

}
