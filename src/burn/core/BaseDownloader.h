//-------------------------------------------------------------------------------------------------
// <copyright file="BaseDownloader.h" company="Microsoft">
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
// This class provides the base functionality required of the downloader
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once

#include "LogSignatureDecorator.h"
#include "interfaces\IPerformer.h"

namespace IronMan
{
    struct DownloadInformation
    {
        const CUrl& m_srcPath;
        const CPath& m_destPath;
        const CString& m_friendlyName;
        const DWORD m_retryCount;

        DownloadInformation(const CUrl& srcPath
                        , const CPath& destPath
                        , const CString& friendlyName
                        , const DWORD retryCount)
        : m_srcPath(srcPath)
          , m_destPath(destPath)
          , m_friendlyName(friendlyName)
          , m_retryCount(retryCount)          
        {}
    };     

    template <typename Ux = Ux>
    class BaseDownloaderT : public AbortPerformerBase
    {
    public:
        ILogger& m_logger;   
        Ux& m_uxLogger;  
        const CUrl m_srcPath;
        const CPath& m_destPath;
        const CString& m_friendlyName;
        const UINT m_retryCount;

    protected:
        IProgressObserver* m_observer;
        ULONGLONG m_bytesTransferred;
        ULONGLONG m_bytesTotal;

    public:
        // Constructor
        BaseDownloaderT(const DownloadInformation& downloaderInformation
                          , ILogger& logger
                          , Ux& uxLogger)
            : m_srcPath(downloaderInformation.m_srcPath)
            , m_destPath(downloaderInformation.m_destPath)
            , m_friendlyName(downloaderInformation.m_friendlyName)
            , m_retryCount(downloaderInformation.m_retryCount)
            , m_logger(logger)
            , m_uxLogger(uxLogger)
            , m_bytesTransferred(0)
            , m_bytesTotal(0)
            , m_observer(&NullProgressObserver::GetNullProgressObserver())            
        {

        }

        // Virtual Destructor
        virtual ~BaseDownloaderT()
        {
        }       

    protected:
        void FinishedDownloadingItem(HRESULT hr)
        {
            if (S_OK == hr) 
            {
                m_bytesTransferred = m_bytesTotal;
            }
            m_uxLogger.StopRecordingItem(m_friendlyName
                                         , m_bytesTransferred
                                         , HRESULT_FROM_WIN32(hr)
                                         , L""
                                         , m_retryCount); 
            m_observer->Finished(HasAborted() ? E_ABORT : hr);
        }
    };

    typedef BaseDownloaderT<> BaseDownloader;
}
