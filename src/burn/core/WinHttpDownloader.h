//-------------------------------------------------------------------------------------------------
// <copyright file="WinHttpDownloader.h" company="Microsoft">
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
#include "interfaces\IProgressObserver.h"
#include "common\SystemUtil.h"
#include "common\MsgWaitForObject.h"
#include "common\CoInitializer.h"
#include "common\LogUtils.h"
#include "common\AutoClean.h"
#include "common\StringUtil.h"
#include "LogSignatureDecorator.h"
#include "BaseDownloader.h"

namespace IronMan
{
    template <typename Ux = Ux>
    class WinHttpDownloaderT : public BaseDownloaderT<Ux>
    {
        const CoInitializer ci;	

        //Declare them as class member so that I can clean them in my destructor.
        HINTERNET m_hSession;
        HINTERNET m_hConnect;
        HINTERNET m_hRequest;

        //After the setup, the thread will block unit this event is being raised.
        //This event is raised by the functions in the callback.
        HANDLE m_DoneEvent;
        DWORD m_dwSizeRead;     

        CRITICAL_SECTION m_CallBackCritSec;


        // Holds the HRESULT after Download has started
        HRESULT m_hrDownload;

        //The internal buffer size of the Http read block is 8k
        //but the MSDN recommend a bigger buffer size, so set it 
        // 32k.
#define DOWNLOADSIZE 32768 

        CAtlArray<char> m_fileDownloadBuffer;
        LPSTR m_pfileDownloadBuffer;  //A pointer to m_fileDownloadBuffer;			
        CAtlFile m_fileToDownloadInto;

    public:
        WinHttpDownloaderT(const DownloadInformation& downloaderInformation
                          , ILogger& logger
                          , Ux& uxLogger)                  
            : BaseDownloaderT(downloaderInformation, logger, uxLogger)
            , m_hSession(NULL)
            , m_hConnect(NULL)
            , m_hRequest(NULL)
            , m_DoneEvent(CreateEvent(NULL, FALSE, FALSE, NULL))							
            , m_dwSizeRead(0)	
            , m_hrDownload(S_OK)
        {
            m_fileDownloadBuffer.SetCount(DOWNLOADSIZE);
            m_pfileDownloadBuffer = m_fileDownloadBuffer.GetData();
            InitializeCriticalSection(&m_CallBackCritSec);
        }

        virtual ~WinHttpDownloaderT()
        {	
            if (NULL != m_DoneEvent)
            {
                CloseHandle(m_DoneEvent);
                m_DoneEvent = NULL;
            }

            // Close any open handles.
            if (m_hRequest) 
            {
                WinHttpSetStatusCallback(m_hRequest, NULL, NULL, NULL );
                WinHttpCloseHandle(m_hRequest);
                m_hRequest = NULL;
            }

            if (m_hConnect) 
            {
                WinHttpCloseHandle(m_hConnect);
                m_hConnect = NULL;
            }

            if (m_hSession) 
            {
                WinHttpCloseHandle(m_hSession);
                m_hConnect = NULL;
            }

            DeleteCriticalSection(&m_CallBackCritSec);
        }

        virtual void PerformAction(IProgressObserver& observer)
        {			
            CString server;
            CString object;
            HRESULT hr = S_OK;

            CString section = L" complete";
            PUSHLOGSECTIONPOP(m_logger, L"Action", L"Downloading " + StringUtil::FromUrl(m_srcPath) + L" using WinHttp ", section);

            m_observer = &observer;
            m_uxLogger.StartRecordingItem(m_friendlyName 
                                        , UxEnum::pDownload
                                        , UxEnum::aDownload                                        
                                        , UxEnum::tWinHttp);            

            if SUCCEEDED(hr = ProcessHttpUrl(server, object))
            {				
                if (SUCCEEDED(hr = StartDownload(server, object)))
                {
                    MsgWaitForObject::Wait(m_DoneEvent);				
                    CloseHandle(m_DoneEvent);
                    m_DoneEvent = NULL;
                    // m_hrDownload is set in Cleanup() method. Update the local hr with it.
                    hr = m_hrDownload;
                }
            }
            FinishedDownloadingItem(hr);			
        }

      

    public:
        //------------------------------------------------------------------------------
        // AsyncCallback
        //
        // This is the callback that WINHTTP calls.
        //
        //-------------------------------------------------------------------------------
        template <typename T>
        static void CALLBACK AsyncCallback(
            __in  HINTERNET hInternet,
            __in  DWORD_PTR dwContext,
            __in  DWORD dwInternetStatus,
            LPVOID lpvStatusInformation,
            __in  DWORD dwStatusInformationLength)
        {
            T *pCallbackContext = (T *)(dwContext);
            pCallbackContext->AsyncCallbackInternal(hInternet, dwInternetStatus, lpvStatusInformation, dwStatusInformationLength);
            return;
        }


    private:
        //------------------------------------------------------------------------------
        // AsyncCallback
        //
        // This is the real method that handles the Winhttp Notification.
        //
        //-------------------------------------------------------------------------------
        void AsyncCallbackInternal(
            __in  HINTERNET hInternet
            ,__in  DWORD dwInternetStatus
            ,LPCVOID lpvStatusInformation
            ,__in DWORD dwStatusInformationLength)
        {
            int nResult = IDOK;
            HRESULT hr = S_OK;

            switch (dwInternetStatus)
            {
            case WINHTTP_CALLBACK_STATUS_DATA_AVAILABLE:				
                m_dwSizeRead = *(static_cast<const DWORD *>(lpvStatusInformation));

                // If there is no data, the process is complete.
                if( 0 == m_dwSizeRead)
                {		
                    nResult = m_observer->OnProgress(0xff);
                    hr = HRESULT_FROM_VIEW(nResult);
                    if (FAILED(hr))
                    {
                        LOG(m_logger, ILogger::Error, L"User interface commanded engine to abort"); 
                        Abort();
                    }
                    else
                    {
                        hr = S_OK;
                    }

                    Cleanup(hr);					
                }
                else   
                    // Otherwise, if there is more data, read the next block
                {
                    if ( FAILED(hr = ReadData()))
                    {
                        Cleanup(hr);
                    }
                }
                break;

            case WINHTTP_CALLBACK_STATUS_READ_COMPLETE:
                // Copy the data and delete the buffers.
                if( 0 != dwStatusInformationLength)
                {					
                    //Write to disk
                    hr = m_fileToDownloadInto.Write(m_pfileDownloadBuffer, m_dwSizeRead);
                    if (FAILED(hr))
                    {
                        CString msg;
                        msg.Format(L"Error writing to local file: hr= 0x%x", hr);						
                        LOG(m_logger, ILogger::Error, msg); 
                        Cleanup(hr);
                    }
                    else
                    {
                        //Compute total size download so far
                        m_bytesTransferred += m_dwSizeRead;
                        
                        if (0 < m_bytesTransferred)
                        {
                            //Update progress
                            nResult = m_observer->OnProgress(static_cast<unsigned char>((m_bytesTransferred * 255.0)/m_bytesTotal));
                            hr = HRESULT_FROM_VIEW(nResult);
                            if (FAILED(hr))
                            {
                                LOG(m_logger, ILogger::Error, L"User interface commanded engine to abort"); 
                                Abort();
                            }
                        }

                        // Now check for more data. If there is no more,  close the context
                        if(FAILED(hr = QueryData()))
                        {
                            Cleanup(hr);
                        }
                    }
                }				
                break;

            case WINHTTP_CALLBACK_STATUS_REQUEST_ERROR:
                {
                    const WINHTTP_ASYNC_RESULT* pAR = static_cast<const WINHTTP_ASYNC_RESULT *>(lpvStatusInformation);

                    CString msg;				
                    msg.Format(L"WINHTTP_CALLBACK_STATUS_REQUEST_ERROR error: error=%d, result= %d. Percentage downloaded=%i", 
                        pAR->dwError, pAR->dwResult, static_cast<int>((m_bytesTransferred*100.0)/m_bytesTotal));
                    LOG(m_logger, ILogger::Error, msg);				
                    Cleanup(pAR->dwError ? HRESULT_FROM_WIN32(pAR->dwError) : E_FAIL );							
                    break;				
                }
                //Should receive this callback once WinHttpSendRequest() is successfully send
                //The callback function should call WinHttpReceiveResponse() to begin receiving the response.
            case WINHTTP_CALLBACK_STATUS_SENDREQUEST_COMPLETE:				
                if (!WinHttpReceiveResponse(m_hRequest, NULL)) 
                {
                    hr = LOGGETLASTERROR(m_logger, ILogger::Error, L"WinHttpReceiveResponse");	
                    Cleanup(hr);										
                }
                break;

            case WINHTTP_CALLBACK_STATUS_HEADERS_AVAILABLE:
                ProcessHeader();				
                break;
            }
        }

        //Use protected so that I can unit test it.
    protected:
        //------------------------------------------------------------------------------
        // ProcessHeader
        //
        // Ensure that the destination file is being created 
        // Send WinHttpQueryHeaders request
        // Query if there is any data.
        //
        // Remark
        // =======
        // This block of code is being refactored out so that it can be unit tested.
        //
        //-------------------------------------------------------------------------------
        virtual void ProcessHeader()
        {
            HRESULT hr = S_OK;
            //Since fileToDownloadInto.Create will fail when the file does not exist, 
            //ensure that the directory exists first before trying to create the file.
            CPath strExpandFilePath;
            CSystemUtil::ExpandEnvironmentVariables(m_destPath, strExpandFilePath);
            if (FAILED(hr = CSystemUtil::CreateDirectoryIfNeeded(strExpandFilePath)))
            {
                CString msg;
                msg.Format(L"Failed to create destination directory: %s with hr=0x%x", strExpandFilePath, hr);
                LOG(m_logger, ILogger::Error, msg);

                Cleanup(hr);
                return ;
            }

            //Note: I am using CREATE_ALWAYS, so if the file exists, it will overwrite the file.
            if (FAILED(hr = m_fileToDownloadInto.Create(strExpandFilePath, FILE_WRITE_DATA, 0, CREATE_ALWAYS)))
            {
                CString msg;
                msg.Format(L"Failed to create the destination file: %s withith hr=0x%x", strExpandFilePath, hr);
                LOG(m_logger, ILogger::Error, msg);  	

                Cleanup(hr );
                return;
            }	

            WCHAR szContentLength[32];
            DWORD dwSize = sizeof(szContentLength);
            bool bSuccess = WinHttpQueryHeaders(m_hRequest, 
                WINHTTP_QUERY_CONTENT_LENGTH, 
                NULL, 
                szContentLength, 
                &dwSize, 
                WINHTTP_NO_HEADER_INDEX);
            
            if (bSuccess)
            {
                szContentLength[31] = NULL;
                m_bytesTotal = _wtoi64(szContentLength);
            }

            // Begin downloading the resource.
            if( FAILED(hr = QueryData()))
            {
                Cleanup(hr);					
            }
        }

        //------------------------------------------------------------------------------
        // ReadData
        //
        // Check that we have not aborted
        // Call the WinHttpReadData api to trigger the next event.
        //
        //-------------------------------------------------------------------------------
        virtual HRESULT ReadData()
        {
            if (HasAborted()) 
            {
                return E_ABORT;
            }			

            // Read the available data.
            if(FALSE == WinHttpReadData( m_hRequest, 
                (LPVOID)m_pfileDownloadBuffer, 
                m_dwSizeRead, 
                NULL ) )
            {
                return LOGGETLASTERROR(m_logger, ILogger::Error, L"WinHttpReadData");	
            }

            // Otherwise, the read is successful or asynchronous.
            return S_OK;
        }

        //------------------------------------------------------------------------------
        // QueryDataT
        //
        // Determine if there is any more data to read by calling the WinHttpQueryDataAvailable API.		
        //
        //-------------------------------------------------------------------------------
        virtual HRESULT QueryData()
        {
            // Chech for available data.
            if(FALSE == WinHttpQueryDataAvailable(m_hRequest, NULL))
            {
                return LOGGETLASTERROR(m_logger, ILogger::Error, L"WinHttpQueryDataAvailable" );				
            }
            return S_OK;
        }

        //------------------------------------------------------------------------------
        // Cleanup
        //
        // Below are the cleanup it is doing
        // a.  Clean up Request Handle
        // b.  Clean up Connection Handle
        // c.  Send the finish method
        // d.  Signal all done event
        // e.  Close the handle to the file we are writing to.
        //
        //-------------------------------------------------------------------------------
        virtual void Cleanup(HRESULT hr )
        {
            if(m_hRequest )
            {
                //Not using the virtual override function.
                WinHttpSetStatusCallback( m_hRequest, NULL, NULL, NULL );
                WinHttpCloseHandle( m_hRequest );
                m_hRequest = NULL;
            }

            if( m_hConnect )
            {
                WinHttpCloseHandle( m_hConnect );
                m_hConnect = NULL;
            }

            // NOTE:  Because this function can be called concurrently by 
            //        different threads, a critical section is needed to
            //        protect the following references to global data.
            EnterCriticalSection( &m_CallBackCritSec);			

            if (NULL != m_DoneEvent) 
            {
                m_fileToDownloadInto.Close();
                m_hrDownload = hr;
                SetEvent(m_DoneEvent);  //Signal that we are done.
            }

            LeaveCriticalSection( &m_CallBackCritSec);
        }

        //Used by Unit Test only
        virtual ULONGLONG GetTotalBytesToRead()
        {
            return m_bytesTotal;
        }

        //------------------------------------------------------------------------------
        // ProcessHttpUrl
        //
        // Note
        // ====
        // 1. Sample: http://www.microsoft.com/downloads/details.aspx?FamilyID=06111A3A-A651-4745-88EF-3D48091A390B&displaylang=en#filelist
        //		- server = www.microsoft.com
        //		- object = /downloads/details.aspx?FamilyID=06111A3A-A651-4745-88EF-3D48091A390B&displaylang=en#filelist
        //-------------------------------------------------------------------------------
        virtual HRESULT ProcessHttpUrl(CString& server, CString& object)
        {		
            CString srcPath = StringUtil::FromUrl(m_srcPath);
            if (srcPath.IsEmpty())
            {
                return E_INVALIDARG;
            }			

            /* We should be able to support FILE too....
            if (ATL_URL_SCHEME_HTTP != m_srcPath.GetScheme())
            {
                LOG(m_logger, ILogger::Error, L"The provided URL scheme is not http");  				
                return E_INVALIDARG;
            }*/

            server = m_srcPath.GetHostName();
            object = m_srcPath.GetUrlPath() + CString(m_srcPath.GetExtraInfo());				
            return S_OK;
        }

        //------------------------------------------------------------------------------
        // StartDownload
        //
        // All the neccessary ground work to download a file. 
        //-------------------------------------------------------------------------------
        HRESULT StartDownload(const CString& server, const CString& object)
        {
            HRESULT hr = S_OK;		
            bool  bResults = false;

            // Use WinHttpOpen to obtain a session handle.
            if (FAILED(hr = CreateSession())) return hr;
            LOG(m_logger, ILogger::Verbose, L"Session Created");  

            // Specify an HTTP server.
            if (m_hSession)
            {
                if (FAILED(hr = CreateConnection(server))) return hr;
            }
            LOG(m_logger, ILogger::Verbose, L"Connection Created");  

            // Create an HTTP request handle.
            if (m_hConnect)
            {
                if (FAILED(hr = CreateRequest(object))) return hr;
            }
            LOG(m_logger, ILogger::Verbose, L"Request Created");  

            // Send a request.
            if (m_hRequest)
            {	
                // Install the status callback function.
                // NOTE:  On success, WinHttpSetStatusCallback returns a 
                //        pointer to the previous callback function.
                //        Here it should be NULL!				
                if (WINHTTP_INVALID_STATUS_CALLBACK == WinHttpSetStatusCallback(
                    m_hRequest,
                    AsyncCallback<WinHttpDownloaderT<Ux>>,
                    WINHTTP_CALLBACK_FLAG_ALL_COMPLETIONS, 
                    NULL ))				
                {
                    return LOGGETLASTERROR(m_logger, ILogger::Error, L"WinHttpSetStatusCallback");	
                }

                if (FAILED(hr = SendRequest())) return hr;				
            }
            return hr;
        }

        HRESULT GetProxySettingViaIE(HINTERNET& hTestSession, CString& csProxyInfo, CString& csProxyByPass, DWORD& dwAccessType)
        {
            LOG(m_logger, ILogger::Verbose, L"Retrieving proxy information using WinHttpGetIEProxyConfigForCurrentUser");
            csProxyByPass.Empty();

            WINHTTP_CURRENT_USER_IE_PROXY_CONFIG ie = {0};
            AutoGlobalFree<LPWSTR> freeProxy(ie.lpszProxy);
            AutoGlobalFree<LPWSTR> freeProxyBypass(ie.lpszProxyBypass);
            AutoGlobalFree<LPWSTR> freeAutoConfigUrl(ie.lpszAutoConfigUrl);

            if (FALSE == WinHttpGetIEProxyConfigForCurrentUser(&ie))
            {
                return LOGGETLASTERROR(m_logger, ILogger::Warning, L"WinHttpGetIEProxyConfigForCurrentUser");
            }

            dwAccessType = WINHTTP_ACCESS_TYPE_NAMED_PROXY;
            if (ie.lpszProxyBypass) 
            {   
                csProxyByPass = ie.lpszProxyBypass;
            }

            if (ie.fAutoDetect && ie.lpszAutoConfigUrl)
            {
                csProxyInfo = ie.lpszAutoConfigUrl;
            }
            else if (ie.lpszProxy) 
            {
                csProxyInfo = ie.lpszProxy;
            }
            else
            {
                LOG(m_logger, ILogger::Warning, L"Unable to retrieve Proxy information although WinHttpGetIEProxyConfigForCurrentUser called succeeded");
                return E_FAIL;
            }

            return S_OK;
        }

        //------------------------------------------------------------------------------
        // GetProxySettingViaAutoDetect
        //
        // Get the proxy setting
        //
        // Note:
        // 1.	WinHTTP does not currently support proxy configurations that specify 
        //		more than one proxy server. If WinHttpGetProxyForUrl returns a 
        //		WINHTTP_PROXY_INFO structure that contains a list of proxy servers, 
        //		which the application then sets on the request handle using the 
        //		WINHTTP_OPTION_PROXY option, WinHTTP uses only the first proxy server 
        //		in the list. If that proxy server is not accessible, WinHTTP does not 
        //		failover to any of the other proxy servers in the list. It is up to the 
        //		application to handle this case by setting the WINHTTP_OPTION_PROXY option 
        //		again with the next proxy server in the list, and resending the request.
        //-------------------------------------------------------------------------------
        HRESULT GetProxySettingViaAutoDetect(HINTERNET& hTestSession, CString& csProxyInfo, CString& csProxyByPass, DWORD& dwAccessType)
        {
            HRESULT hr = S_OK;
            LPWSTR url = NULL;
            
            LOG(m_logger, ILogger::Verbose, L"Auto detecting proxy information");
            if (FALSE == WinHttpDetectAutoProxyConfigUrl(WINHTTP_AUTO_DETECT_TYPE_DHCP 
                | WINHTTP_AUTO_DETECT_TYPE_DNS_A, 
                &url))
            {
                if (NULL != url) 
                {
                    GlobalFree(url);
                }				
                return LOGGETLASTERROR(m_logger, ILogger::Warning, L"WinHttpDetectAutoProxyConfigUrl");				
            }
            const bool fProxyDetected = (NULL != url);

            // Set proxy options
            WINHTTP_AUTOPROXY_OPTIONS proxyOptions = {0};
            proxyOptions.lpszAutoConfigUrl = fProxyDetected ? url : NULL;
            proxyOptions.dwFlags = WINHTTP_AUTOPROXY_AUTO_DETECT | (fProxyDetected ? WINHTTP_AUTOPROXY_CONFIG_URL : NULL);
            proxyOptions.dwAutoDetectFlags = WINHTTP_AUTO_DETECT_TYPE_DHCP | WINHTTP_AUTO_DETECT_TYPE_DNS_A;
            proxyOptions.dwReserved = NULL;
            proxyOptions.lpvReserved = 0;
            proxyOptions.fAutoLogonIfChallenged = TRUE;

            // Get proxy info
            CString srcPath = StringUtil::FromUrl(m_srcPath);
            WINHTTP_PROXY_INFO proxyInfo = {0};
            if (FALSE == WinHttpGetProxyForUrl(hTestSession , srcPath, &proxyOptions, &proxyInfo))
            {
                hr = LOGGETLASTERROR(m_logger, ILogger::Error, L"WinHttpGetProxyForUrl");	
            }
            else
            {
                dwAccessType = proxyInfo.dwAccessType;
                if (NULL != proxyInfo.lpszProxy) 
                {
                    csProxyInfo = proxyInfo.lpszProxy;
                    GlobalFree(proxyInfo.lpszProxy);
                }

                if (NULL != proxyInfo.lpszProxyBypass) 
                {
                    csProxyByPass = proxyInfo.lpszProxyBypass;
                    GlobalFree(proxyInfo.lpszProxyBypass);
                }
            }

            if (NULL != url) 
            {
                GlobalFree(url);
            }
            return hr;
        }

        //------------------------------------------------------------------------------
        // GetProxySetting
        //
        // This is the wrappeer for getting the proxy.  The stratgy is as follows:
        // a.  Use WinHttpDetectAutoProxyConfigUrl to discover the file
        //     - This is known to not work in the runas scenario where a
        //       non-admin runas an admin
        // b.  Use WinHttpGetIEProxyConfigForCurrentUser
        //     - This is actually the fallback if (a) failed.
        // c.  The 2nd fallback's code is to use WINHTTP_ACCESS_TYPE_DEFAULT_PROXY
        //     - This is implemented in CreateSession()
        //
        //------------------------------------------------------------------------------
        HRESULT GetProxySetting(CString& csProxyInfo, CString& csProxyByPass, DWORD& dwAccessType)
        {
            HRESULT hr = S_OK;
            HINTERNET hTestSession;
            hTestSession = WinHttpOpen(L"Setup Installer"
                ,WINHTTP_ACCESS_TYPE_NO_PROXY 
                ,WINHTTP_NO_PROXY_NAME 
                ,WINHTTP_NO_PROXY_BYPASS 
                ,0);

            if (NULL == hTestSession) 
            {
                hr = LOGGETLASTERROR(m_logger, ILogger::Error, L"WinHttpOpen");				
            }
            else
            {
                if (FAILED(GetProxySettingViaAutoDetect(hTestSession, csProxyInfo, csProxyByPass, dwAccessType)))
                {
                    //Fallback
                    LOG(m_logger, ILogger::Verbose, L"Auto detection of proxy failed, try to retrieve proxy information via IE.");
                    hr = GetProxySettingViaIE(hTestSession, csProxyInfo, csProxyByPass, dwAccessType);
                }
            }
            WinHttpCloseHandle(hTestSession);	
            return hr;
        }

        // Note:
        // If asynchronous mode is used, WinHTTP creates a number of threads for 
        // asynchronous selection and timer pooling. WinHTTP creates one thread 
        // per CPU, plus one extra (CPU + 1). If DNS timeout is specified using 
        // NAME_RESOLUTION_TIMEOUT, there is an overhead of one thread per request.
        HRESULT CreateSession()
        {		
            CString csProxyInfo;
            CString csProxyByPass;
            DWORD dwAccessType;
            HRESULT hr = S_OK;			
            if (SUCCEEDED(GetProxySetting(csProxyInfo, csProxyByPass, dwAccessType)))
            {
                switch(dwAccessType)
                {
                case WINHTTP_ACCESS_TYPE_NAMED_PROXY:
                    LOG(m_logger, ILogger::Verbose, L"Using WINHTTP_ACCESS_TYPE_NAMED_PROXY");  
                    m_hSession = WinHttpOpen(	L"Setup Installer"
                        , WINHTTP_ACCESS_TYPE_NAMED_PROXY 
                        , csProxyInfo  
                        , csProxyByPass 
                        , WINHTTP_FLAG_ASYNC);
                    break;
                default:
                    LOG(m_logger, ILogger::Verbose, L"Using WINHTTP_ACCESS_TYPE_NO_PROXY");
                    m_hSession = WinHttpOpen(	L"Setup Installer"
                        , WINHTTP_ACCESS_TYPE_NO_PROXY
                        , WINHTTP_NO_PROXY_NAME 
                        , WINHTTP_NO_PROXY_BYPASS
                        , WINHTTP_FLAG_ASYNC);
                    break;
                }				
            }
            else
            {
                LOG(m_logger, ILogger::Verbose, L"WINHTTP_ACCESS_TYPE_DEFAULT_PROXY");  
                m_hSession = WinHttpOpen(	L"Setup Installer"
                        , WINHTTP_ACCESS_TYPE_DEFAULT_PROXY
                        , WINHTTP_NO_PROXY_NAME 
                        , WINHTTP_NO_PROXY_BYPASS
                        , WINHTTP_FLAG_ASYNC);
            }

            if (NULL == m_hSession)
            {
                //Cannot return here because we need to ensure that the resources are freed.
                hr = LOGGETLASTERROR(m_logger, ILogger::Error, L"WinHttpOpen");	
            }

            return hr;
        }

        HRESULT CreateConnection(CString server)
        {			
            m_hConnect = WinHttpConnect(m_hSession
                ,server
                ,INTERNET_DEFAULT_HTTP_PORT
                ,0);

            if (NULL == m_hConnect)
            {
                return LOGGETLASTERROR(m_logger, ILogger::Error, L"WinHttpConnect");	
            }
            return S_OK;
        }

        //------------------------------------------------------------------------------
        // ChooseAuthScheme
        //
        // Return the supported scheme and 0 for non-supported scheme. 
        //-------------------------------------------------------------------------------
        DWORD ChooseAuthScheme( DWORD dwSupportedSchemes )
        {
            if( dwSupportedSchemes & WINHTTP_AUTH_SCHEME_NTLM )
            {
                return WINHTTP_AUTH_SCHEME_NTLM;	  
            }
            else if( dwSupportedSchemes & WINHTTP_AUTH_SCHEME_DIGEST )
            {
                return WINHTTP_AUTH_SCHEME_DIGEST;
            }
            else
            {
                CString msg;
                msg.Format(L"Not Supported authentication schemes selected: %u", dwSupportedSchemes);
                LOG(m_logger, ILogger::Error, msg);
                return 0;
            }			
        }
        /*
        bool AutenticateWinHTTP()
        {
        bool bDone = false;
        DWORD dwStatusCode = 0;
        bool  bResults = false;
        DWORD dwSupportedSchemes;
        DWORD dwProxyAuthScheme = WINHTTP_AUTH_SCHEME_NTLM;
        DWORD dwFirstScheme;
        DWORD dwSelectedScheme;
        DWORD dwTarget;		
        DWORD dwLastStatus = 0;
        DWORD dwSize = sizeof(DWORD);

        while( !bDone )
        {
        //  If a proxy authentication challenge was responded to, reset
        //  those credentials before each SendRequest, because the proxy  
        //  may require re-authentication after responding to a 401 or  
        //  to a redirect. If you don't, you can get into a 
        //  407-401-407-401- loop.
        WinHttpSetCredentials(	m_hRequest, 
        WINHTTP_AUTH_TARGET_PROXY, 
        dwProxyAuthScheme, 
        NULL,
        NULL,
        NULL );

        // Send a request.
        bResults = WinHttpSendRequest(	m_hRequest,
        WINHTTP_NO_ADDITIONAL_HEADERS,
        0,
        WINHTTP_NO_REQUEST_DATA,
        0, 
        0, 
        0 );

        // End the request.
        if( bResults )
        {
        bResults = WinHttpReceiveResponse( m_hRequest, NULL );
        }

        // Resend the request in case of 
        // ERROR_WINHTTP_RESEND_REQUEST error.
        if( !bResults)
        {
        if (GetLastError( ) == ERROR_WINHTTP_RESEND_REQUEST)
        {
        continue;
        }
        else
        {
        break;
        }
        }

        // Check the status code.
        if (false == WinHttpQueryHeaders( m_hRequest, 
        WINHTTP_QUERY_STATUS_CODE |
        WINHTTP_QUERY_FLAG_NUMBER,
        NULL, 
        &dwStatusCode, 
        &dwSize, 
        NULL ))
        {
        break;
        }

        switch(dwStatusCode)
        {
        case 200: 
        // The resource was successfully retrieved.
        // You can use WinHttpReadData to read the 
        // contents of the server's response.
        bDone = TRUE;
        break;

        case 401:
        // The server requires authentication.					

        // Obtain the supported and preferred schemes.
        bResults = WinHttpQueryAuthSchemes( m_hRequest, 
        &dwSupportedSchemes, 
        &dwFirstScheme, 
        &dwTarget );

        // Set the credentials before resending the request.
        if( bResults )
        {
        dwSelectedScheme = ChooseAuthScheme( dwSupportedSchemes);

        if( dwSelectedScheme == 0 )
        {
        bDone = TRUE;
        }
        else
        {
        bResults = WinHttpSetCredentials(	m_hRequest, 
        dwTarget, 
        dwSelectedScheme,
        NULL,
        NULL,
        NULL );
        }
        }

        // If the same credentials are requested twice, abort the
        // request.  For simplicity, this sample does not check
        // for a repeated sequence of status codes.
        if( dwLastStatus == 401 )
        {
        bDone = TRUE;
        }
        break;

        case 407:
        // The proxy requires authentication.
        // Obtain the supported and preferred schemes.
        if (false == WinHttpQueryAuthSchemes( m_hRequest, 
        &dwSupportedSchemes, 
        &dwFirstScheme, 
        &dwTarget )) break;

        // Set the credentials before resending the request.
        dwProxyAuthScheme = ChooseAuthScheme(dwSupportedSchemes);

        // If the same credentials are requested twice, abort the
        // request.  For simplicity, this sample does not check 
        // for a repeated sequence of status codes.
        if( 407 == dwLastStatus)
        bDone = TRUE;
        break;

        default:
        // The status code does not indicate success.																
        CString msg;
        msg.Format(L"Invalid status code: %d",dwStatusCode);
        LOG(m_logger, ILogger::Error, msg);
        bDone = TRUE;
        }
        }
        }

        // Keep track of the last status code.
        dwLastStatus = dwStatusCode;

        // If there are any errors, break out of the loop.
        if( !bResults ) 
        {
        bDone = TRUE;
        }
        return bDone;
        }
        */
        HRESULT CreateRequest(CString object)
        {
            HRESULT hr = S_OK;
            m_hRequest = WinHttpOpenRequest(m_hConnect
                ,L"GET"
                ,object   /* filepath and filename */
                ,NULL
                ,WINHTTP_NO_REFERER
                ,WINHTTP_DEFAULT_ACCEPT_TYPES
                ,WINHTTP_FLAG_REFRESH);	

            if (NULL == m_hRequest)
            {
                return LOGGETLASTERROR(m_logger, ILogger::Error, L"WinHttpOpenRequest");	
            }	

            /*if (!AutenticateWinHTTP())
            {
            return E_FAIL;
            }*/

            return hr;
        }

        HRESULT SendRequest()
        {			
            DWORD_PTR dwContextValue = reinterpret_cast<DWORD_PTR>(this);
            if (FALSE == WinHttpSendRequest(m_hRequest
                ,WINHTTP_NO_ADDITIONAL_HEADERS
                ,0
                ,WINHTTP_NO_REQUEST_DATA	
                ,0 
                ,0
                ,dwContextValue))	
            {
                return LOGGETLASTERROR(m_logger, ILogger::Error, L"WinHttpSendRequest");	
            }
            return S_OK;				
        }		

    private:
        virtual bool WinHttpSendRequest(IN HINTERNET hRequest,
            IN LPCWSTR pwszHeaders OPTIONAL,
            IN DWORD dwHeadersLength,
            IN LPVOID lpOptional OPTIONAL,
            IN DWORD dwOptionalLength,
            IN DWORD dwTotalLength,
            IN DWORD_PTR dwContext)
        {
            return ::WinHttpSendRequest(hRequest, 
                pwszHeaders, 
                dwHeadersLength, 
                lpOptional, 
                dwOptionalLength, 
                dwTotalLength, 
                dwContext);		
        }

        virtual HINTERNET
            WINAPI
            WinHttpOpenRequest(	IN HINTERNET hConnect,
            IN LPCWSTR pwszVerb,
            IN LPCWSTR pwszObjectName,
            IN LPCWSTR pwszVersion,
            IN LPCWSTR pwszReferrer OPTIONAL,
            IN LPCWSTR FAR * ppwszAcceptTypes OPTIONAL,
            IN DWORD dwFlags)
        {
            return ::WinHttpOpenRequest(	hConnect, 
                pwszVerb, 
                pwszObjectName, 
                pwszVersion, 
                pwszReferrer, 
                ppwszAcceptTypes, 
                dwFlags);
        }

        virtual HINTERNET
            WINAPI
            WinHttpConnect(	IN HINTERNET hSession,
            IN LPCWSTR pswzServerName,
            IN INTERNET_PORT nServerPort,
            IN DWORD dwReserved)
        {
            return ::WinHttpConnect(	hSession, 
                pswzServerName, 
                nServerPort, 
                dwReserved);
        }

        virtual HINTERNET
            WINAPI
            WinHttpOpen(__in_opt LPCWSTR pszAgentW,
            __in DWORD dwAccessType,
            __in_opt LPCWSTR pszProxyW,
            __in_opt LPCWSTR pszProxyBypassW,
            __in DWORD dwFlags)
        {			
            return ::WinHttpOpen(pszAgentW,
                dwAccessType,
                pszProxyW,
                pszProxyBypassW,
                dwFlags);	
        }

        virtual BOOL WinHttpDetectAutoProxyConfigUrl( __in  DWORD     dwAutoDetectFlags, 
            __out LPWSTR *  ppwszAutoConfigUrl)
        {
            return ::WinHttpDetectAutoProxyConfigUrl(dwAutoDetectFlags, ppwszAutoConfigUrl);
        }

        virtual BOOL WinHttpGetProxyForUrl(IN  HINTERNET                   hSession,
            IN  LPCWSTR                     lpcwszUrl,
            IN  WINHTTP_AUTOPROXY_OPTIONS * pAutoProxyOptions,
            OUT WINHTTP_PROXY_INFO *        pProxyInfo)
        {
            return ::WinHttpGetProxyForUrl(hSession, lpcwszUrl, pAutoProxyOptions, pProxyInfo);			
        }

        virtual BOOL WinHttpSetCredentials(
            __in  HINTERNET hRequest,
            __in  DWORD AuthTargets,
            __in  DWORD AuthScheme,
            __in  LPCWSTR pwszUserName,
            __in  LPCWSTR pwszPassword,
            __in  LPVOID pAuthParams
            )
        {  
            return ::WinHttpSetCredentials(hRequest, AuthTargets, AuthScheme, pwszUserName, pwszPassword, pAuthParams);			
        }

        virtual BOOL WinHttpQueryAuthSchemes(
            __in   HINTERNET hRequest,
            __out  LPDWORD lpdwSupportedSchemes,
            __out  LPDWORD lpdwFirstScheme,
            __out  LPDWORD pdwAuthTarget
            )
        {
            return ::WinHttpQueryAuthSchemes( hRequest, lpdwSupportedSchemes, lpdwFirstScheme, pdwAuthTarget);			
        }

        virtual WINHTTP_STATUS_CALLBACK WinHttpSetStatusCallback
            (
            IN HINTERNET hInternet,
            IN WINHTTP_STATUS_CALLBACK lpfnInternetCallback,
            IN DWORD dwNotificationFlags,
            IN DWORD_PTR dwReserved
            )
        {			
            return ::WinHttpSetStatusCallback(hInternet, lpfnInternetCallback, dwNotificationFlags, dwReserved);
        }

        virtual bool
            WINAPI
            WinHttpReceiveResponse
            (
            IN HINTERNET hRequest,
            IN LPVOID lpReserved
            )
        {
            return ::WinHttpReceiveResponse(hRequest, lpReserved);
        }

        virtual bool
            WinHttpReadData
            (
            IN HINTERNET hRequest,
            IN __out_data_source(NETWORK) LPVOID lpBuffer,
            IN DWORD dwNumberOfBytesToRead,
            OUT LPDWORD lpdwNumberOfBytesRead
            )
        {
            return ::WinHttpReadData(hRequest, lpBuffer, dwNumberOfBytesToRead, lpdwNumberOfBytesRead);
        }

        virtual bool WinHttpQueryDataAvailable(
            __in       HINTERNET hRequest,
            __out_opt  LPDWORD lpdwNumberOfBytesAvailable
            )
        {
            return ::WinHttpQueryDataAvailable(hRequest, lpdwNumberOfBytesAvailable);
        }

        virtual bool WinHttpCloseHandle(IN HINTERNET hInternet)
        {
            return ::WinHttpCloseHandle(hInternet);
        }

        virtual bool 
            WinHttpQueryHeaders
            (
            IN     HINTERNET hRequest,
            IN     DWORD     dwInfoLevel,
            IN     LPCWSTR   pwszName OPTIONAL, 
            OUT __out_data_source(NETWORK) LPVOID    lpBuffer OPTIONAL,
            IN OUT LPDWORD   lpdwBufferLength,
            IN OUT LPDWORD   lpdwIndex OPTIONAL
            )
        {
            return ::WinHttpQueryHeaders(hRequest, dwInfoLevel, pwszName, lpBuffer, lpdwBufferLength, lpdwIndex);
        }

        virtual BOOL WinHttpGetIEProxyConfigForCurrentUser(IN OUT WINHTTP_CURRENT_USER_IE_PROXY_CONFIG * pProxyConfig)
        {
            return ::WinHttpGetIEProxyConfigForCurrentUser(pProxyConfig);
        }

        virtual HGLOBAL GlobalFree( __deref HGLOBAL hMem) 
        {
            return ::GlobalFree(hMem);
        }

    };

    typedef WinHttpDownloaderT<> WinHttpDownloader;
} // namespace IronMan
