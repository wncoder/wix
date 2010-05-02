//-------------------------------------------------------------------------------------------------
// <copyright file="BitsDownloader.h" company="Microsoft">
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
// This is the BITS Downloader for downloading using BITS technology
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once

#include "interfaces\ILogger.h"
#include "common\CoInitializer.h"
#include "common\ResultObserver.h"
#include "LogSignatureDecorator.h"
#include "common\SystemUtil.h"
#include "common\StringUtil.h"
#include "BaseDownloader.h"
#include "BitsLogger.h"

namespace IronMan
{
    template <typename Ux = Ux, int MaxRetryCount = 3, int NoProgressTimeoutPeriod = 2 * 60>
    class BitsDownloaderT : protected IBackgroundCopyCallback, public BaseDownloaderT<Ux>
    {        
        const CoInitializer ci;

        int m_iRetryCount;
        int m_iSleepCountMilliseconds;

        //BITS related variables
        CComPtr<IBackgroundCopyManager> m_pBitsManager;        
        CComPtr<IBackgroundCopyJob> m_pJob;

        LONG m_PendingJobModificationCount;
        LONG m_lRefCount;

        int m_iPercentDownloaded;  //The % amount of data downloaded

        BitsLogger m_bitsLogger;

        bool m_bJobCanceled;
        DWORD m_threadID;

    public:
        //Constructor
        BitsDownloaderT(const DownloadInformation& downloadInformation
                       , ILogger& logger
                       , Ux& uxLogger) 
            : BaseDownloaderT(downloadInformation, logger, uxLogger)
            , m_bitsLogger(logger)
            , m_bJobCanceled(false)
            , m_pBitsManager(NULL)
            , m_PendingJobModificationCount(0)
            , m_lRefCount(0)
            , m_threadID(::GetCurrentThreadId())
            , m_iPercentDownloaded(0)
            , m_iRetryCount(MaxRetryCount)  //Resume on all errors for 3 times.
            , m_iSleepCountMilliseconds(1000)  //Wait 1 second between retry
        {
        }

        //Destructor
        virtual ~BitsDownloaderT()
        {
            m_bitsLogger.Stop();
        }

    public:
        virtual void Abort()
        {
            AbortPerformerBase::Abort();
            ::PostThreadMessage(m_threadID, WM_QUIT, 0, 0);
        }

        HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, LPVOID* ppvObj) 
        {
            if (riid == __uuidof(IUnknown) || riid == __uuidof(IBackgroundCopyCallback)) 
            {
                *ppvObj = static_cast<IBackgroundCopyCallback *>(this);
            }
            else
            {
                *ppvObj = NULL;
                return E_NOINTERFACE;
            }

            AddRef();
            return NOERROR;
        }

        ULONG STDMETHODCALLTYPE AddRef() 
        {
            return InterlockedIncrement(&m_lRefCount);
        }

        ULONG STDMETHODCALLTYPE Release()
        {
            return InterlockedDecrement(&m_lRefCount);
        }

        HRESULT STDMETHODCALLTYPE JobTransferred(IBackgroundCopyJob* pJob)
        {
            HRESULT hr = S_OK;
            int nResult = IDOK;
            hr = Complete(pJob);    

            //Set max progress to ensure that the progress is 100% for this download.
            if (SUCCEEDED(hr))
            {
                nResult = m_observer->OnProgress(0xff);
                hr = HRESULT_FROM_VIEW(nResult);
                if (FAILED(hr))
                {
                    LOG(m_logger, ILogger::Error, L"User interface commanded engine to abort");
                }
            }

            //Set the bytes transferred as JobModification callback may not be called.
            //This allows us to log the right user experience data
            BG_JOB_PROGRESS Prog;
            if (SUCCEEDED(GetProgress( &Prog, pJob )))
            {   
                m_bytesTransferred = Prog.BytesTransferred;
                m_bytesTotal = Prog.BytesTotal;
            }

            FinishedDownloadingItem(hr);
            ::PostThreadMessage(m_threadID, WM_QUIT, 0, 0);

            //If you do not return S_OK, BITS continues to call this callback.
            return S_OK;
        }

        //This callback is called by BITS when there is an error.
        HRESULT STDMETHODCALLTYPE JobError(IBackgroundCopyJob* pJob, IBackgroundCopyError* pError)
        {
            HRESULT hrError = E_FAIL;
            BG_ERROR_CONTEXT Context = BG_ERROR_CONTEXT_NONE;

            //Retrieve the HRESULT associated with the error. The context tells you
            //where the error occurred, for example, in the transport, queue manager, the 
            //local file, or the remote file.
            CString msg;
            HRESULT hr = GetError(pError, &Context, &hrError);
            if (SUCCEEDED(hr))
            {
                LPWSTR pszErrorDescription;
                hr = GetErrorDescription(pError, LANGIDFROMLCID(GetThreadLocale()), &pszErrorDescription);

                if (SUCCEEDED(hr))
                {
                    msg.Format(L"Error from JobError Callback : hr= 0x%x Context=%i Description=%s", hrError, Context, pszErrorDescription);
                    CoTaskMemFree(pszErrorDescription);
                    pszErrorDescription = NULL;
                }
                else
                {
                    msg.Format(L"Error from JobError Callback : hr= 0x%x Context=%i", hrError, Context);
                }
            }
            else
            {
                msg = L"Unknown Error";
            }
            CString csPercentDownloaded;
            csPercentDownloaded.Format(L"Percentage downloaded = %i", m_iPercentDownloaded);
            msg = msg + L". " +  csPercentDownloaded;

            // Make this just a warning since we will retry WinHttpDownloader
            // Error will be logged if the WinHttpDownloader fails
            LOG(m_logger, ILogger::Warning, msg);

            if (!HasAborted())
            {
                CString strErrorResult = StringUtil::FromHresult(hrError);
                //When ERROR_WINHTTP_NAME_NOT_RESOLVED is hit, we have already waited for 2 minutes (no activity timeout)
                //Retrying in this case has not shown to help the problem, therefore, not retrying anymore.
                if (L"00002ee7" == strErrorResult || BG_E_NETWORK_DISCONNECTED == hrError)
                {
                    LOG(m_logger, ILogger::Warning, L"This error indicates that the server/proxy could not be resolved by BITS.");
                }
                else if (L"00000043" == strErrorResult || L"00000035" == strErrorResult || BG_E_HTTP_ERROR_404 == hrError)
                {
                    LOG(m_logger, ILogger::Warning, L"This error indicates that the file could not be found by BITS.");
                }
                else
                {
                    //Per the BITS team recommendation, we are resuming for all errors for N times with N delay.
                    if (0 < m_iRetryCount--)
                    {
                        //Start logging when we have an actual failure.
                        m_bitsLogger.Start();
                        ::Sleep(m_iSleepCountMilliseconds);  //sleep for a wehile
                        ResumeJob(pJob);
                        return S_OK;
                    }
                }
            }

            //Need to cancel the job otherwise it will stay in the system for 90 days.
            Cancel(pJob);

            //Update the progress.
            FinishedDownloadingItem(HasAborted() ? E_ABORT : hrError);

            ::PostThreadMessage(m_threadID, WM_QUIT, 0, 0);

            //Always return S_OK.
            return S_OK;
        }

        HRESULT STDMETHODCALLTYPE JobModification(IBackgroundCopyJob* pJob, DWORD dwReserved)
        {
            return JobModificationPrivate<S_OK>(pJob, dwReserved);
        }

    public:
        static bool IsAvailable(ILogger& logger, Ux& uxLogger)
        {
            DownloadInformation downloadInformation(CUrl(), NULL, L"", 0);
            BitsDownloaderT<Ux> bits(downloadInformation, logger, uxLogger);
            return bits.IsOkay();
        }

    protected:
        //This template method is created so that we can unit test the InterlockedCompareExchange code path.
        template<HRESULT H> LONG JobModificationPrivate( IBackgroundCopyJob* pJob, DWORD dwReserved)
        {
            //To ensure that we process on JobModification at any one time.
            if (InterlockedCompareExchange(&m_PendingJobModificationCount, 1, 0) == 1)
            {
                return H;
            }

            //After InterlockedCompareExchange call to ensure that it does not get called mutliple time.
            if (HasAborted())
            {
                Cancel(pJob);
                return S_OK;
            }

            BG_JOB_STATE state = GetState(pJob);
            if (BG_JOB_STATE_TRANSFERRING == state)
            {
                BG_JOB_PROGRESS Prog;
                if (SUCCEEDED(GetProgress( &Prog, pJob )))
                {
                    //To protect against Prog.BytesTotal being 0; 
                    m_bytesTotal = Prog.BytesTotal;
                    m_bytesTransferred = Prog.BytesTransferred;
                    m_iPercentDownloaded = static_cast<int>((Prog.BytesTransferred * 100.0) / (0 == Prog.BytesTotal ? 1 : Prog.BytesTotal));
                    m_observer->OnProgress(static_cast<unsigned char>((Prog.BytesTransferred * 255.0)/(0 == Prog.BytesTotal ? 1 : Prog.BytesTotal)));
                }
            }

            m_PendingJobModificationCount = 0;
            return S_OK;
        }

    public:
        //IPerformer Interface
        virtual void PerformAction(IProgressObserver& observer)
        {
            CString section = L" complete";
            PUSHLOGSECTIONPOP(m_logger, L"Action", L"Downloading " + StringUtil::FromUrl(m_srcPath) + L" using BITS ", section);

            m_uxLogger.StartRecordingItem(m_friendlyName 
                                        , UxEnum::pDownload
                                        , UxEnum::aDownload
                                        , UxEnum::tBITS);
            HRESULT hr;
            m_observer = &observer;
            if (SUCCEEDED(hr = Initialize()))
            {
                DWORD dwRet = WAIT_TIMEOUT;

                MSG msg;
                BOOL bRet = -1;

                // GetMessage() will return '0' if msg.message == WM_QUIT.
                while ((bRet = ::GetMessage(&msg, NULL, 0, 0)) != 0)
                {
                    if (bRet != -1)
                    {
                        TranslateMessage(&msg);
                        DispatchMessage(&msg);
                    }
                }

                m_bitsLogger.Stop();

                if (HasAborted())
                {
                    Cancel(m_pJob);
                    FinishedDownloadingItem(E_ABORT);
                }
            }
            else
            {
                //Need to cancel after the job has been created!
                Cancel(m_pJob);

                FinishedDownloadingItem(hr);
            }
        }

        //------------------------------------------------------------------------------
        // IsOkay
        //
        // Use to determone if it is OK to use BITS service.
        //-------------------------------------------------------------------------------
        bool IsOkay()
        {
            HRESULT hr = S_OK;

            if ( FAILED(InitializeBITS())) return false;
            if ( FAILED(CreateJob())) return false;
            if ( FAILED(SetCredentials())) return false;

            TCHAR szTempPath[_MAX_PATH] = {0};
            TCHAR szTempFile[_MAX_PATH] = {0};
            ::GetTempPath(_MAX_PATH, szTempPath);
            ::GetTempFileName(szTempPath, _T("bch"), 0, szTempFile);

            if (FAILED(AddFile(m_pJob, L"http://www.microsoft.com", szTempFile))) return false;
            if (FAILED(Cancel(m_pJob))) return false;

            return true;
        }

    private:

        //------------------------------------------------------------------------------
        // Initialize
        //
        // Mechanic steps needs request a download via the bits service.
        //-------------------------------------------------------------------------------
        HRESULT Initialize()
        {
            HRESULT hr = S_OK;

            if (FAILED(hr = InitializeBITS())) return hr;

            if (FAILED(hr = CreateJob())) return hr;

            if (FAILED(hr = SetCredentials())) return hr;

            if (FAILED(hr = AddFileToJob())) return hr;

            if (FAILED(hr = SetJobPriority(m_pJob))) return hr;

            //Need to set the Timeout because the default timeout period is 14 days.
            if (FAILED(hr = SetNoProgressTimeout(m_pJob, NoProgressTimeoutPeriod)))
            {
                CString msg;
                msg.Format(L"Unable to Set No Progress Timeout : hr= 0x%x", hr);
                LOG(m_logger, ILogger::Error, msg);  
                return hr;
            }

            if (FAILED(hr = SetNotifyInterface(m_pJob, this))) return hr;

            if (FAILED(hr = SetNotifyFlags(m_pJob, BG_NOTIFY_JOB_TRANSFERRED 
                | BG_NOTIFY_JOB_ERROR 
                | BG_NOTIFY_JOB_MODIFICATION ))) return hr;

            if (FAILED(hr = ResumeJob(m_pJob))) return hr;

            return hr;
        }

    protected:
        //Used for testing purpose only
        void SetErrorRetryCount(int iRetryCount, int iSleepCountMilliseconds)
        {
            m_iRetryCount = iRetryCount;
            m_iSleepCountMilliseconds = iSleepCountMilliseconds;
        }
        //------------------------------------------------------------------------------
        // Complete
        //
        // Complete the job.  If the file is partially downloaded when this
        // is called, it will be removed from the file system and will NOT
        // be available to the user.
        //-------------------------------------------------------------------------------
        HRESULT CompleteJob(IBackgroundCopyJob* pJob)
        {
            // Get the BITS job state
            const BG_JOB_STATE State = GetState(pJob);

            // Do nothing if it's already completed
            if ( BG_JOB_STATE_ACKNOWLEDGED == State )
            {
                return S_OK;
            }

            // try to complete the job s.t. the files become available 
            return Complete(pJob);
        }

    protected:
        //------------------------------------------------------------------------------
        // InitializeBITS
        //
        // Co-Create BITS.
        //-------------------------------------------------------------------------------
        virtual HRESULT InitializeBITS(void)
        {
            HRESULT hr;

            hr = ::CoCreateInstance( __uuidof(BackgroundCopyManager)
                , NULL
                , CLSCTX_ALL
                , __uuidof(IBackgroundCopyManager)
                , reinterpret_cast<void **>(& m_pBitsManager));

            if (FAILED(hr))
            {
                CString msg;
                msg.Format(L"Failed to COCreateInstance BackgroundCopyManager : hr= 0x%x", hr);
                LOG(m_logger, ILogger::Information, msg);
            }

            return hr;
        }

    protected:
        //------------------------------------------------------------------------------
        // AddFile
        //
        // Add a file to the job
        //-------------------------------------------------------------------------------
        virtual HRESULT AddFileToJob()
        {
            HRESULT hr = S_OK;

            //This should be in the base class.
            if (FAILED(hr = CSystemUtil::CreateDirectoryIfNeeded(m_destPath)))
            {
                CString msg;
                msg.Format(L"Failed to create destination directory: %s with hr=0x%x", m_destPath, hr);
                LOG(m_logger, ILogger::Error, msg);
                return hr;
            }

            CString srcPath = StringUtil::FromUrl(m_srcPath);
            if (srcPath.IsEmpty())
            {
                return E_INVALIDARG;
            }

            if (FAILED(hr = m_pJob->AddFile(srcPath, m_destPath )))
            {
                CString msg;
                msg.Format(L"Unable to add file to BITS : hr= 0x%x", hr);
                LOG(m_logger, ILogger::Error, msg);
            }

            return hr;
        }

    protected:
        virtual HRESULT CreateJob()
        {
            HRESULT hr = S_OK;
            GUID    JobId;
            if (FAILED(hr = m_pBitsManager->CreateJob( L"Setup Installer"
                , BG_JOB_TYPE_DOWNLOAD
                , & JobId
                , & m_pJob)))
            {
                CString msg;
                msg.Format(L"Failed to CreateJob : hr= 0x%x", hr);
                LOG(m_logger, ILogger::Error, msg);
            }
            return hr;
        }

    protected:
        virtual HRESULT SetJobCredential(CComPtr<IBackgroundCopyJob2>& pJob2, BG_AUTH_CREDENTIALS* ac)
        {
            HRESULT hr = S_OK;    
            if (FAILED(hr = pJob2->SetCredentials(ac)))
            {
                CString msg;
                msg.Format(L"Unable to SetCredentials(PROXY,*) : hr= 0x%x",hr);
                LOG(m_logger, ILogger::Error, msg);
            }
            return hr;
        }

    protected:
        virtual HRESULT SetCredentials()
        {
            HRESULT hr = S_OK;
            CComPtr<IBackgroundCopyJob2> pJob2 = NULL;
            if (FAILED(hr = m_pJob->QueryInterface(__uuidof(IBackgroundCopyJob2), reinterpret_cast<void **>(&pJob2))))
            {
                CString msg;
                msg.Format(L"Unable to QueryInterface IBackgroundCopyJob2 : hr= 0x%x",hr);
                LOG(m_logger, ILogger::Error, msg); 
                return hr;
            }

            BG_AUTH_CREDENTIALS ac;

            ac.Target = BG_AUTH_TARGET_PROXY;
            ac.Scheme = BG_AUTH_SCHEME_NTLM;
            ac.Credentials.Basic.UserName = NULL;
            ac.Credentials.Basic.Password = NULL;
            SetJobCredential(pJob2, &ac); //Ignore the result since most proxy does not need auth anyway;

            ac.Target = BG_AUTH_TARGET_PROXY;
            ac.Scheme = BG_AUTH_SCHEME_NEGOTIATE;
            ac.Credentials.Basic.UserName = NULL;
            ac.Credentials.Basic.Password = NULL;
            SetJobCredential(pJob2, &ac);  //Ignore the result since most proxy does not need auth anyway;

            return hr;
        }

    protected:
        //------------------------------------------------------------------------------
        // SetJobPriority
        //
        // Set the prority to FOREGROUND so that it gets process immediately.
        //-------------------------------------------------------------------------------
        virtual HRESULT SetJobPriority(IBackgroundCopyJob* pJob)
        {
            HRESULT hr = S_OK;
            if (FAILED(hr = pJob->SetPriority( BG_JOB_PRIORITY_FOREGROUND )))
            {
                CString msg;
                msg.Format(L"Unable to SetPriority : hr= 0x%x", hr);
                LOG(m_logger, ILogger::Error, msg);
            }
            return hr;
        }

    protected:
        //------------------------------------------------------------------------------
        // ResumeJob
        //
        // Need to resume the job after adding it and log any error.
        //-------------------------------------------------------------------------------
        virtual HRESULT ResumeJob(IBackgroundCopyJob* pJob)
        {
            HRESULT hr = S_OK;
            if (FAILED(hr = pJob->Resume()))
            {
                CString msg;
                msg.Format(L"Unable to Resume Job: hr= 0x%x", hr);
                LOG(m_logger, ILogger::Error, msg);
            }
            return hr;
        }

    protected:
        //------------------------------------------------------------------------------
        // GetProgress
        //
        // Get the current progress of the download and log any error.
        //-------------------------------------------------------------------------------
        virtual HRESULT GetProgress(BG_JOB_PROGRESS* pProgress, IBackgroundCopyJob* pJob)
        {
            HRESULT hr = S_OK;
            if (FAILED(hr = pJob->GetProgress(pProgress)))
            {
                CString msg;
                msg.Format(L"Failed GetProgress() call: hr= 0x%x", hr);
                LOG(m_logger, ILogger::Error, msg);
            }
            return hr;
        }

    protected:
        //------------------------------------------------------------------------------
        // Cancel
        //
        // Signal the job to be cancel and log any error.
        //-------------------------------------------------------------------------------
        virtual HRESULT Cancel(IBackgroundCopyJob* pJob)
        {
            HRESULT hr = S_OK;

            if (NULL != pJob && m_bJobCanceled == false)
            {
                pJob->SetNotifyInterface(NULL); // Tell BITS to stop sending messages.

                hr = pJob->Cancel();
                if (FAILED(hr))
                {
                    CString msg;
                    msg.Format(L"Unable to Cancel Job : hr= 0x%x", hr);
                    LOG(m_logger, ILogger::Error, msg);
                }
                else
                {
                    m_bJobCanceled = true;
                }
            }
            return hr;
        }

    protected:
        //------------------------------------------------------------------------------
        // GetState
        //
        // Get the BITS job state and log any error.
        //-------------------------------------------------------------------------------
        virtual BG_JOB_STATE GetState(IBackgroundCopyJob* pJob)        
        {
            BG_JOB_STATE State;
            HRESULT hr = pJob->GetState( &State );
            if (FAILED(hr))
            {
                CString msg;
                msg.Format(L"Unable to State of the Job : hr= 0x%x", hr);
                LOG(m_logger, ILogger::Error, msg);
                return BG_JOB_STATE_ERROR;
            }
            return State;
        }

    protected:
        //------------------------------------------------------------------------------
        // Complete
        //
        // Signal the job to be complete and log any error.
        //-------------------------------------------------------------------------------
        virtual HRESULT Complete(IBackgroundCopyJob* pJob)
        {
            // try to complete the job s.t. the files become available
            const HRESULT hr = pJob->Complete();
            if (FAILED(hr))
            {
                CString msg;
                msg.Format(L"Unable to complete job : hr= 0x%x", hr);
                LOG(m_logger, ILogger::Error, msg);
            }    
            return hr;
        }

    protected:
        virtual HRESULT AddFile(IBackgroundCopyJob* pJob,
            /* [in] */ __RPC__in LPCWSTR RemoteUrl,
            /* [in] */ __RPC__in LPCWSTR LocalName)
        {
            const HRESULT hr = pJob->AddFile(RemoteUrl, LocalName);
            if (FAILED(hr))
            {
                CString msg;
                msg.Format(L"AddFile failed with : hr= 0x%x", hr);
                LOG(m_logger, ILogger::Error, msg);
            }    
            return hr;
        }

    protected:
        virtual HRESULT SetNotifyInterface(IBackgroundCopyJob* pJob, /* [in] */ __RPC__in_opt IUnknown *Val)
        {
            const HRESULT hr = pJob->SetNotifyInterface(Val);
            if (FAILED(hr))
            {
                CString msg;
                msg.Format(L"SetNotifyInterface failed with : hr= 0x%x", hr);
                LOG(m_logger, ILogger::Error, msg);
            }
            return hr;
        }

    protected:
        virtual HRESULT SetNotifyFlags(IBackgroundCopyJob* pJob, /* [in] */ ULONG Val)
        {
            const HRESULT hr = pJob->SetNotifyFlags(Val);
            if (FAILED(hr))
            {
                CString msg;
                msg.Format(L"SetNotifyFlags failed with : hr= 0x%x", hr);
                LOG(m_logger, ILogger::Error, msg);
            }    
            return hr;
        }

        virtual BOOL SetEvent( __in HANDLE hEvent )
        {
            return ::SetEvent(hEvent);
        }

    protected:
        virtual HRESULT GetError( IBackgroundCopyError* pError,
            /* [ref][out] */ __RPC__out BG_ERROR_CONTEXT *pContext,
            /* [ref][out] */ __RPC__out HRESULT *pCode)
        {
            const HRESULT hr = pError->GetError(pContext, pCode);
            if (FAILED(hr))
            {
                CString msg;
                msg.Format(L"IBackgroundCopyError::GetError failed: hr= 0x%x", hr);
                LOG(m_logger, ILogger::Error, msg);
            }
            return hr;
        }

    protected:
        virtual HRESULT GetErrorDescription( IBackgroundCopyError* pError,
            /* [in] */ DWORD LanguageId,
            /* [ref][out] */ __RPC__deref_out_opt LPWSTR *pErrorDescription)
        {
            const HRESULT hr = pError->GetErrorDescription(LanguageId, pErrorDescription);
            if (FAILED(hr))
            {
                CString msg;
                msg.Format(L"IBackgroundCopyError::GetErrorDescription failed: hr= 0x%x", hr);
                LOG(m_logger, ILogger::Error, msg);
            }
            return hr;
        }

    private:
        virtual void Sleep(__in DWORD dwMilliseconds)
        {
            ::Sleep(dwMilliseconds);
        }

        virtual HRESULT STDMETHODCALLTYPE SetNoProgressTimeout(IBackgroundCopyJob* pJob, ULONG Seconds)
        {
            return pJob->SetNoProgressTimeout(Seconds);
        }

        virtual DWORD MsgWaitForMultipleObjects(
            __in DWORD nCount,
            __in_ecount_opt(nCount) CONST HANDLE *pHandles,
            __in BOOL fWaitAll,
            __in DWORD dwMilliseconds,
            __in DWORD dwWakeMask)
        {
            return ::MsgWaitForMultipleObjects(nCount, pHandles, fWaitAll, dwMilliseconds, dwWakeMask);
        }

        virtual BOOL PeekMessageW(
            __out LPMSG lpMsg,
            __in_opt HWND hWnd,
            __in UINT wMsgFilterMin,
            __in UINT wMsgFilterMax,
            __in UINT wRemoveMsg)
        {
            return ::PeekMessage(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg);
        }
    };

    typedef BitsDownloaderT<> BitsDownloader;
} // namespace IronMan
