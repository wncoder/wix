//-------------------------------------------------------------------------------------------------
// <copyright file="UrlMonDownLoader.h" company="Microsoft">
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
#include "common\CoInitializer.h"
#include "interfaces\IProgressObserver.h"
#include "LogSignatureDecorator.h"
#include "common\SystemUtil.h"
#include "common\StringUtil.h"
#include "BaseDownloader.h"

namespace IronMan
{
template <typename Ux = Ux>
class UrlMonDownloaderT : protected IBindStatusCallback, public BaseDownloaderT<Ux>
{   

public:
    UrlMonDownloaderT(const DownloadInformation& downloaderInformation
                     , ILogger& logger
                     , Ux& uxLogger) 
        : BaseDownloaderT(downloaderInformation, logger, uxLogger) 		 
    {
    }

    virtual ~UrlMonDownloaderT()
    {}

    virtual void PerformAction(IProgressObserver& observer)
    {
        m_observer = &observer;
        HRESULT hr = S_OK;

        m_uxLogger.StartRecordingItem(m_friendlyName 
                                        , UxEnum::pDownload
                                        , UxEnum::aDownload                                        
                                        , UxEnum::tUrlMon);  
        CString srcPath = StringUtil::FromUrl(m_srcPath);
        CString section = L" complete";
        PUSHLOGSECTIONPOP(m_logger, L"Action", L"Downloading " + srcPath + L" using UrlMon ", section);
        
        if (srcPath.IsEmpty())
        {
            FinishedDownloadingItem(HasAborted() ? E_ABORT : E_INVALIDARG);  
            return;
        }      

        if (FAILED(hr = CSystemUtil::CreateDirectoryIfNeeded(m_destPath)))
        {
            CString cs;
            cs.Format(L"Unable to create the destination directory: %s", m_destPath);
            LOG(m_logger, ILogger::Error, cs);
            FinishedDownloadingItem(HasAborted() ? E_ABORT : hr);  
            return;
        }

        CoInitializer ci;
        hr = URLDownloadToFile( NULL, 
                                srcPath, 
                                m_destPath,                                             
                                0, 
                                this);

        if (FAILED(hr) && !HasAborted())
        {
            //reset the progress bar - ignore result from OnProgress because we're already aborting
            observer.OnProgress(0);
            CString cs;
            cs.Format(L"UrlMon download failed with %x", hr);
            LOG(m_logger, ILogger::Error, cs); 
        }
        else
        //To test the following statement from MSDN:
        //"URLDownloadToFile returns S_OK even if the file cannot be created and the download is canceled"
        if (!::PathFileExists(m_destPath))
        {
            //reset the progress bar - ignore result from OnProgress because we're already aborting
            observer.OnProgress(0);            
            LOG(m_logger, ILogger::Error, L"UrlMon failed to create the destination file"); 
            hr = ERROR_FILE_NOT_FOUND;
        }

        FinishedDownloadingItem(hr);   
    }

private:   

// IUnknown
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID, void * *ppvObject) { return E_NOTIMPL; }
    virtual ULONG STDMETHODCALLTYPE AddRef (void) { return 2; }
    virtual ULONG STDMETHODCALLTYPE Release(void) { return 1; }

// IBindStatusCallback
    virtual HRESULT STDMETHODCALLTYPE OnStartBinding( DWORD dwReserved, IBinding __RPC_FAR *pib) { return E_NOTIMPL; }
    virtual HRESULT STDMETHODCALLTYPE GetPriority( LONG __RPC_FAR *pnPriority) { return E_NOTIMPL; }
    virtual HRESULT STDMETHODCALLTYPE OnLowResource( DWORD reserved) { return E_NOTIMPL; }
    virtual HRESULT STDMETHODCALLTYPE OnStopBinding( HRESULT hresult, LPCWSTR szError) { return E_NOTIMPL; }
    virtual HRESULT STDMETHODCALLTYPE GetBindInfo( DWORD __RPC_FAR *grfBINDF, BINDINFO __RPC_FAR *pbindinfo) { return E_NOTIMPL; }
    virtual HRESULT STDMETHODCALLTYPE OnDataAvailable( DWORD grfBSCF, DWORD dwSize, FORMATETC __RPC_FAR *pformatetc, STGMEDIUM __RPC_FAR *pstgmed) { return E_NOTIMPL; }
    virtual HRESULT STDMETHODCALLTYPE OnObjectAvailable( REFIID riid, IUnknown __RPC_FAR *punk) { return E_NOTIMPL; }
    virtual HRESULT STDMETHODCALLTYPE OnProgress( ULONG ulProgress, ULONG ulProgressMax, ULONG ulStatusCode, LPCWSTR szStatusText)
    {
        if (HasAborted())
        {
            return E_ABORT;
        }

        m_bytesTransferred = ulProgress;

        if (0 < ulProgressMax)
        {
            return m_observer->OnProgress(static_cast<unsigned char>((ulProgress * 255.0)/ulProgressMax));
        }

        return S_OK;
    }    
private:
    virtual HRESULT URLDownloadToFile(LPUNKNOWN pCaller, LPCWSTR szURL, LPCWSTR szFileName, DWORD dwReserved, LPBINDSTATUSCALLBACK lpfnCB)
    {
        return ::URLDownloadToFile(pCaller, szURL, szFileName, dwReserved, lpfnCB);
    }
};

typedef UrlMonDownloaderT<> UrlMonDownloader;
}
