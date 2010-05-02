//-------------------------------------------------------------------------------------------------
// <copyright file="CompositeDownloader.h" company="Microsoft">
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
//      The controller that handles downloading.
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once

#include "WeightedProgressObserver.h"
#include "RetryingDownloader.h"
#include "UberCoordinator.h"
#include "LogSignatureDecorator.h"
#include "Ux\Ux.h"
#include "InTestEnvironment.h"
#include "FirstError.h"
#include "FileCompression.h"
#include "CopyPerformer.h"
#include "ICacheManager.h"

namespace IronMan
{

struct Redirector
{
    static void Redirect(CUrl& src, bool bIsTest = false)
    {
        if (InTestEnvironment::IsTestEnvironment() || bIsTest)
        {
            RedirectFromKey(src);
        }
    }

private:
    static void RedirectFromKey(CUrl& src)
    {
        CRegKey rk;
        LONG lres = rk.Open(HKEY_CURRENT_USER, L"Software\\Microsoft\\VisualStudio\\Setup");
        if (lres == ERROR_SUCCESS)
        {
            ULONG ul = 0;
            lres = rk.QueryStringValue(L"DownloadServer", NULL, &ul);
            if (lres == ERROR_SUCCESS)
            {
                CString cs;
                rk.QueryStringValue(L"DownloadServer", cs.GetBuffer(ul), &ul);
                cs._ReleaseBuffer();

                // use protocol
                CUrl url;
                url.CrackUrl(cs);
                if (!CString(url.GetSchemeName()).IsEmpty())
                {
                    src.SetSchemeName(url.GetSchemeName());

                    // strip off protocol and use remaining as hostname
                    url.SetSchemeName(L"http");
                    DWORD dwMaxLength = url.GetUrlLength() + 1;
                    url.CreateUrl(cs.GetBuffer(dwMaxLength), &dwMaxLength);
                    cs._ReleaseBuffer();
                    src.SetHostName(cs.Mid(7)); // http:// == 7 characters
                }
            }
        }
    }
};

template<typename RetryingDownloader, typename Ux = Ux> // test hook
class CompositeDownloaderT : public AbortStopPerformerBase
{
    ILogger& m_logger;
    Ux& m_uxLogger;
    const int m_retries, m_secondsToWait;
    ICoordinator& m_coordinator;
    IPerformer* m_currentPerformer;
    FirstError m_firstError;
    bool m_shouldCopyPackageFile;
    CSimpleArray<HANDLE> m_hFiles;  //Hold the handles of the files we are locking
    ICacheManager& m_cacheManager;
    IBurnView *m_pBurnView;

    //This class to is used to determine if this is a local or Web package.
    class Local
    {
        bool m_bIsLocal;

    public:
        //Constructor
        Local(const IDownloadItems& items)
            : m_bIsLocal(ComputeLocal(items))
        {
        }

        //Destructor
        ~Local()
        {
        }

        const bool IsLocal() const
        {
            return m_bIsLocal;
        }
    private:
         //Return false when either of the conditions are true
        // a. There is 1 file to be download
        // b. The file that needs to be downloaded is already in the temp location.
        static bool ComputeLocal(const IDownloadItems& items)
        {
            bool bResult = true;
            for(unsigned int i=0; i<items.GetCount() && true == bResult; ++i)
            {
                if (items.IsItemAvailableUnVerified(i) || items.IsItemAvailable(i))
                {
                    CUrl src;
                    CString hash;
                    CPath dstPath;
                    ULONGLONG itemSize = 0;
                    bool bIgnoreDownloadFailure = false;
                    CString itemName;
                    CString itemId;
                    items.GetItem(i, &src, hash, dstPath, itemSize, bIgnoreDownloadFailure, itemName, itemId);

                    CPath path(ModuleUtils::GetDllPath());
                    path.Append(itemName);

                    if (0 != CString(dstPath).CompareNoCase(path))
                    {
                        bResult = false;
                    }
                }
                else
                {
                    bResult = false;
                }
            }

            return bResult;
        }
    };

public:
    CompositeDownloaderT(ICoordinator& coordinator
                        , ICacheManager& cacheManager
                        , int retries
                        , int secondsToWait
                        , bool shouldCopyPackageFile
                        , ILogger& logger
                        , Ux& uxLogger
                        , IBurnView *pBurnView = NULL)
        : m_logger(logger)
        , m_coordinator(coordinator)
        , m_cacheManager(cacheManager)
        , m_retries(retries)
        , m_secondsToWait(secondsToWait)
        , m_shouldCopyPackageFile(shouldCopyPackageFile)
        , m_currentPerformer(&NullPerformer::GetNullPerformer())
        , m_uxLogger(uxLogger)
        , m_firstError(logger)
        , m_pBurnView(pBurnView)
    {}

    ~CompositeDownloaderT()
    {
        for(int iIndex = 0; iIndex < m_hFiles.GetSize(); ++iIndex)
        {
            ::CloseHandle(m_hFiles[iIndex]);
        }
    }

    void SetBurnView(IBurnView *pBurnView)
    {
        m_pBurnView = pBurnView;
    }

public: // IPerformer
    virtual void PerformAction(IProgressObserver& observer)
    {
        int nResult = IDOK;

        CString section = L" complete";
        PUSHLOGSECTIONPOP(m_logger, L"Action", L"Downloading and/or Verifying Items", section);

        IDownloadItems& items = m_coordinator.GetDownloadItems();
        Local local(items);
        XCopyForDownloadLoggerDecorator copyLogger(m_hFiles, m_logger);
        CopyPackagePerfomer<> copy(items, !local.IsLocal(), m_shouldCopyPackageFile, copyLogger);
        m_uxLogger.RecordSKU(local.IsLocal() ? UxEnum::sLocal : UxEnum::sWeb);

        CSimpleArray<ULONGLONG> weights;

        //Need to compute total size of the file to be copied over and to the weight.
        //Need to be here so the weight is on the correct position.  
        if (copy.IsCopying())
        {            
            weights.Add(copy.GetTotalCopySize());
        }

        // Add files that need to be downloaded
        for(unsigned int i=0; i<items.GetCount(); ++i)
        {
            if (!items.IsItemAvailableUnVerified(i))
            {
                CUrl src;
                CString hash;
                CPath dst;
                ULONGLONG itemSize = 0;
                bool bIgnoreDownloadFailure = false;
                CString itemName;
                CString itemId;
                items.GetItem(i, &src, hash, dst, itemSize, bIgnoreDownloadFailure, itemName, itemId);

                // There should be no item that needs to be downloaded with an item size of zero! 
                if (itemSize == 0)
                {
                    LOGEX(m_logger, ILogger::Verbose, L"Item %s's download size has not been set or is set to zero. This means no space will be allocated for this item's download on the the download progress bar.", dst); 
                }
                weights.Add(itemSize);
            }
        }
        WeightedProgressObserver weightedObserver(observer, weights);

        HRESULT hr = S_OK;
        ResultObserver resultObserver(weightedObserver, hr);

        class TempDownloadFile
        {
        private:
            const CString& m_friendlyName;
            CPath m_dst;
            CPath m_tempFile;
            FirstError& m_firstError;
            bool m_fileCompressed;
            ILogger& m_logger;
            Ux& m_uxLogger;

        public:
            TempDownloadFile(const CString& friendlyName, const CPath& dst, FirstError& firstError, bool fileCompressed, ILogger& logger, Ux& uxLogger)
                : m_friendlyName(friendlyName) // Used to log user experience data - should not contain any PI data
                 ,m_dst(dst)
                 ,m_logger(logger)
                 ,m_uxLogger(uxLogger)
                 ,m_fileCompressed(fileCompressed)
                 ,m_firstError(firstError)
            {}

            HRESULT SetTempFile()
            {
                 // Create a temp file to pass to  the downloader
                HRESULT hr = S_OK;
                CPath folder(m_dst);
                folder.RemoveFileSpec();

                if (FAILED(hr = CSystemUtil::CreateDirectoryIfNeeded(m_dst)))
                {
                    LOG(m_logger, ILogger::Error, L"Failed to create the folder: " + CString(folder));
                }
                else
                {
                    if (!GetTempFileName(folder, L"TMP", 0, m_tempFile.m_strPath.GetBuffer(MAX_PATH+1)))
                    {
                        hr = HRESULT_FROM_WIN32(GetLastError());
                        CString msg;
                        msg.Format(L"Failed to get a temp file name.  GetTempFileName call failed with 0x%x", hr);
                        LOG(m_logger, ILogger::Error, msg);
                    }
                    m_tempFile.m_strPath._ReleaseBuffer();
                    if (m_fileCompressed)
                    {
                        ::DeleteFile(m_tempFile); // Since we will be downloading to TMP<UUUU>.tmp.exe, delete TMP<UUUU>.tmp
                        m_tempFile.m_strPath += L".exe"; // Postfix .exe to the temp file name, TMP<UUUU>.tmp -> TMP<UUUU>.tmp.exe
                    }

                }
                return hr;
            }

            const CPath& GetTempFile() const
            {
                return m_tempFile;
            }

            HRESULT CopyTempFileToDestinationFile(bool hasAborted)
            {
                HRESULT hr = S_OK;
                if (!m_firstError.IsError() &&  (false == hasAborted))
                {
                    if (!m_fileCompressed)
                    {
                        if (!::MoveFileEx(m_tempFile, m_dst, MOVEFILE_REPLACE_EXISTING|MOVEFILE_COPY_ALLOWED))
                        {
                            hr = HRESULT_FROM_WIN32(GetLastError());  
                            CString msg;
                            msg.Format(L"Failed to move temp file to destination location.  MoveFileEx call failed with 0x%x", hr);
                            LOG(m_logger, ILogger::Error, msg);  
                        }
                    }
                    else
                    {
                        FileCompression compressedFile(m_tempFile, m_logger);
                        CString extractionFolder = m_tempFile + L".tmp"; // Extract to folder 'TMP<nnnn>.tmp.exe.tmp'
                        m_uxLogger.StartRecordingItem( m_friendlyName
                                                     , UxEnum::pDownload
                                                     , UxEnum::aDecompress
                                                     , UxEnum::tNone); 

                        hr = compressedFile.Decompress(extractionFolder);

                        m_uxLogger.StopRecordingItem(  m_friendlyName
                                                     , 0
                                                     , hr
                                                     , L""
                                                     , 0);
                        if (S_OK == hr)
                        {
                            CString extractedFile = m_dst;
                            if (compressedFile.FileExists(m_dst, extractedFile))
                            {
                                if (!::MoveFileEx(extractedFile, m_dst, MOVEFILE_REPLACE_EXISTING|MOVEFILE_COPY_ALLOWED))
                                {
                                    hr = HRESULT_FROM_WIN32(GetLastError());  
                                    CString msg;
                                    msg.Format(L"Failed to move temp file to destination location.  MoveFileEx call failed with 0x%x", hr);
                                    LOG(m_logger, ILogger::Error, msg);  
                                }
                            }
                            else
                            {
                                CString msg;
                                msg.Format(L"Failed to find file in the extracted folder: %s", m_dst);
                                LOG(m_logger, ILogger::Error, msg);
                                hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
                            }
                        }
                        else
                        {
                            CString msg;
                            msg.Format(L"Decompression of payload failed: %s", m_dst);
                            LOG(m_logger, ILogger::Error, msg);
                        }
                    }
                }
                return hr;
            }

            virtual ~TempDownloadFile()
            {
                if (m_tempFile.FileExists())
                {
                    ::DeleteFile(m_tempFile);
                }
            }
        };

        //Do the actual copy of the source here.
        if (copy.IsCopying())
        {
            nResult = weightedObserver.OnProgress(255); // in case, last time didn't finish up
            hr = HRESULT_FROM_VIEW(nResult);
            if (FAILED(hr))
            {
                LOG(m_logger, ILogger::Error, L"User interface commanded engine to abort");
                m_firstError.SetError(hr);
                Abort();
            }

            nResult = weightedObserver.SetNextPhase();
            hr = HRESULT_FROM_VIEW(nResult);
            if (FAILED(hr))
            {
                LOG(m_logger, ILogger::Error, L"User interface commanded engine to abort");
                m_firstError.SetError(hr);
                Abort();
            }

            observer.OnStateChangeDetail(IProgressObserver::Copying, L"");

            m_uxLogger.StartRecordingItem(L"Package Files"
                                            , UxEnum::pDownload
                                            , UxEnum::aDownload
                                            , UxEnum::tNone); 

            copy.PerformAction(resultObserver);

            m_uxLogger.StopRecordingItem(L"Package Files"
                                         , copy.GetTotalCopySize()
                                         , hr
                                         , L""
                                         , 0); 
            if (FAILED(hr))
            {
                CComBSTR bstrErrorString;
                CSystemUtil::GetErrorString(hr, bstrErrorString);
                LOGEX(m_logger, ILogger::Error, L"Copy of package file to download location failed with error code: 0x%x - %s ", hr, bstrErrorString);
            }
            m_firstError.SetError(hr);
        }

        UxEnum::spigotDownloadProtocolEnum protocol = UxEnum::sdpNone;
        CPath fileFirstFailureOccurredOn;
        
        for(unsigned int i=0; i<items.GetCount() && !HasAborted() && !IsStopPending() && !m_firstError.IsError(); ++i)
        {
            if (items.IsItemOnDownloadList(i) == false)
                continue;

            //
            // Check to see if the item is cached on the machine already and is Valid.
            // If it is cached but not valid, then it will be deleted and it will need to be cached again
            // If it is cached and valid, the state will change to Available and the LocalPath updated
            // UnElevatedController is not available, so passing NULL
            if (m_cacheManager.IsCached(items, i, NULL))
            {
                LOG(m_logger, ILogger::Verbose, L"Item (" + items.GetItemName(i) + L") is already cached on the machine.");
                continue;
            }

            nResult = weightedObserver.OnProgress(255); // in case, last time didn't finish up
            hr = HRESULT_FROM_VIEW(nResult);
            if (FAILED(hr))
            {
                LOG(m_logger, ILogger::Error, L"User interface commanded engine to abort");
                m_firstError.SetError(hr);
                Abort();
            }
            nResult = weightedObserver.SetNextPhase();
            hr = HRESULT_FROM_VIEW(nResult);
            if (FAILED(hr))
            {
                LOG(m_logger, ILogger::Error, L"User interface commanded engine to abort");
                m_firstError.SetError(hr);
                Abort();
            }

            CUrl src;
            CString hash;
            CPath dst;
            ULONGLONG itemSize = 0;
            bool bIgnoreDownloadFailure = false;
            CString itemName;
            CString itemId;
            items.GetItem(i, &src, hash, dst, itemSize, bIgnoreDownloadFailure, itemName, itemId);
            Redirector::Redirect(src);

            CString downloadItemName = CString(src.GetSchemeName()) + CString(L"://") + CString(src.GetHostName()) + CString(src.GetUrlPath());
            LOG(m_logger, ILogger::Information, L"Downloading " + downloadItemName + L" to " + CString(dst));

            CPath dstName(dst);
            dstName.StripPath();
            CString detailString;
            detailString.Format(L"%i;%s", i, dstName);
            observer.OnStateChangeDetail(IProgressObserver::Downloading, detailString);

            HRESULT downloadResult = S_OK;
            TempDownloadFile downloadFile(itemName, dst, m_firstError, items.IsItemCompressed(i), m_logger, m_uxLogger);
            if (m_firstError.SetError(downloadFile.SetTempFile(), bIgnoreDownloadFailure))
            {
                fileFirstFailureOccurredOn = dst;
            }
            if (!m_firstError.IsError())
            {
                RetryingDownloader downloader(src
                                              , itemName   //Friendly Name - used to log user experience data - should not contain any PI data
                                              , hash
                                              , itemSize
                                              , downloadFile.GetTempFile()
                                              , m_retries
                                              , m_secondsToWait
                                              , protocol
                                              , m_logger 
                                              , m_uxLogger
                                              , itemId
                                              , m_pBurnView);

                m_currentPerformer = &downloader;

                HRESULT hrLocal = S_OK;
                // avoid (very rare) race condition by checking abort flag, right after setting current performer
                if (!HasAborted() && !IsStopPending())
                {
                    m_currentPerformer->PerformAction(resultObserver);
                    downloadResult = hr;
                    // hr can be modified by first SetError() call. So, store it.
                    hrLocal = hr;

                    if (HRESULT_IS_NETPATH_NOT_FOUND(hrLocal))
                    {
                        // no burnview? no source resoultion.
                        if (m_pBurnView)
                        {
                            LOG(m_logger, ILogger::Warning, L"File not found while downloading. Asking user to resolve.");

                            CPath itemPath = dst;
                            CPath itemOriginalNameStripped = dstName;
                            itemOriginalNameStripped.StripPath();
                            int idRet = IDCANCEL;

                            // Since the loop below can potentially go into an infinite loop we limit it to maxRetries.
                            const int maxRetries = 100;
                            int retries = 0;

                            while (1)
                            {
                                if (!itemPath.FileExists())
                                {
                                    idRet = m_pBurnView->ResolveSource(itemId, itemPath);
                                    itemPath = CPath(SourceLocation::GetPath());
                                    itemPath.Append(itemOriginalNameStripped);
                                }
                                else
                                {
                                    idRet = IDOK;
                                    break;
                                }
                               
                                if (idRet == IDOK || idRet == IDRETRY)
                                {
                                    if (!itemPath.FileExists())
                                    {
                                        // Ensure number of retries do not exceed 100.
                                        if (++retries >= 100)
                                        {
                                            LOGEX(m_logger, ILogger::Error, + L"Number of retries to ResolveSource() for item %s exceeded preset limit of %d!", dstName, maxRetries);
                                            idRet = IDABORT;
                                            break;
                                        }

                                        // Ask again.
                                        continue;
                                    }
                                }

                                break;
                            }

                            if (idRet == IDOK)
                            {
                                LOGEX(m_logger, ILogger::Warning, + L"The user has resolved the path to %s.", itemPath);

                                // copy the file to the expected destination
                                if (!::CopyFileW(itemPath, downloadFile.GetTempFile(), FALSE))
                                {
                                    hr = HRESULT_FROM_WIN32(GetLastError());
                                    CString msg;
                                    msg.Format(L"Failed to copy user-resolved file to the download destination location.  CopyFile call failed with 0x%x", hr);
                                    LOG(m_logger, ILogger::Error, msg);
                                }

                                // verify the file
                                hr = downloader.VerifyPayload();
                                hrLocal = hr;
                                downloadResult = hr;
                                if ( SUCCEEDED(hr) )
                                {
                                    items.SetItemStateAsAvailable(i);
                                }
                                else
                                {
                                    LOG(m_logger, ILogger::Error, L"User-resolved file failed verification.");
                                }
                            }
                        }
                    }

                    if ( m_firstError.SetError(hr, bIgnoreDownloadFailure)
                        || ( SUCCEEDED(hrLocal) && m_firstError.SetError(downloadFile.CopyTempFileToDestinationFile(HasAborted()), bIgnoreDownloadFailure)) )
                    {
                        fileFirstFailureOccurredOn = dst;
                    }
                }

                m_currentPerformer = &NullPerformer::GetNullPerformer();

                // Use the saved hr to see if it failed and failure can be ignored.
                bool bDownloadFailureIgnored = FAILED(hrLocal) && bIgnoreDownloadFailure;
                items.UpdateItemState(i, bDownloadFailureIgnored);
            }

            CString completeDetailString;
            completeDetailString.Format(L"%i;%s;%i", i, dstName, downloadResult);
            observer.OnStateChangeDetail(IProgressObserver::DownloadItemComplete, completeDetailString);
            observer.OnStateChangeDetail(IProgressObserver::Downloading, L"");

        }

        if (!m_firstError.IsError())
        {
            nResult = observer.OnProgress(0xff);
            hr = HRESULT_FROM_VIEW(nResult);
            if (FAILED(hr))
            {
                LOG(m_logger, ILogger::Error, L"User interface commanded engine to abort");
                m_firstError.SetError(hr);
                Abort();
            }
        }
        else
        {
            if ( !(fileFirstFailureOccurredOn.m_strPath.IsEmpty()))
            {
                fileFirstFailureOccurredOn.StripPath();
                m_uxLogger.RecordPhaseAndCurrentItemNameOnError(UxEnum::pDownload, UxEnum::aDownload, fileFirstFailureOccurredOn);
            }
        }
        observer.Finished(m_firstError.GetError());
    }
    virtual void Abort()
    {
        AbortPerformerBase::Abort();
        m_currentPerformer->Abort();
        m_firstError.Abort();
    }
};

typedef CompositeDownloaderT<RetryingDownloader> CompositeDownloader;

}
