//-------------------------------------------------------------------------------------------------
// <copyright file="CopyPerformer.h" company="Microsoft">
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
//    For copying a set of files.
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once

#include "interfaces\IPerformer.h"
#include "LogSignatureDecorator.h"
#include "FindFile.h"
#include "SmartLock.h"
#include "SourceLocation.h"

namespace IronMan
{
    class XCopy
    {
        DWORD m_error;
        int m_srcLength;
        const CString m_dst;
        unsigned int m_total, m_soFar;
        IProgressObserver& m_observer;
        const bool& m_aborted;

        static int GetSrcLengthWithoutTrailingBackslash(const CString& src)
        {
            int length = CString::StringLength(src);
            if (src[length-1] == L'\\')
            {
                --length;
            }
            return length;
        }

    protected:
        ILogger& m_logger;
        
    public:
        XCopy(const CString& src, const CString& dst, unsigned int totalFiles, IProgressObserver& observer, ILogger& logger, const bool& aborted)
            : m_srcLength(GetSrcLengthWithoutTrailingBackslash(src))
            , m_dst(dst)
            , m_total(totalFiles)
            , m_soFar(0)
            , m_observer(observer)
            , m_logger(logger)
            , m_aborted(aborted)
            , m_error(ERROR_SUCCESS)
        {}
        ~XCopy()
        {
            m_observer.Finished(GetResult());
        }
        HRESULT GetResult()
        {
            return m_error == ERROR_SUCCESS ? S_OK : HRESULT_FROM_WIN32(m_error);
        }
        void operator()(const CString& from)
        {
            int nResult = IDOK;
            HRESULT hr = S_OK;

            if (m_aborted)
            {
                SetFirstError(E_ABORT);
                return; // finish up fast by doing nothing
            }

            { // special check for sfxcab-generated hidden locked file
                CString lower(from);
                lower.MakeLower();
                if (lower.Find(L"$shtdwn$.req") != -1)
                    return;
            }

            // construct destination filename
            CString to(m_dst);
            to += from.Mid(m_srcLength);

            // create directory, if necessary
            CPath dst(to);
            dst.RemoveFileSpec();
            ::SHCreateDirectory(NULL, dst);

            LOG(m_logger, ILogger::Information, L"Copying " + from + L" to " + to);

            CPath fileBeingCopied(from);
            fileBeingCopied.StripPath();
            m_observer.OnStateChangeDetail(IProgressObserver::Copying, fileBeingCopied);

            BOOL b = ::CopyFile(from, to, FALSE);
            if (b == FALSE)
            {
                DWORD err = ::GetLastError();
                SetFirstError(err);
                // N.B.: not stopping on errors.  Trying to XCopy as much as possible

                CString log;
                log.Format(L"::CopyFile failed with last error: %i, when copying from %s to %s", err, from.GetString(), to.GetString());
                LOG(m_logger, ILogger::Error, log);
            }
            else
            {
                // If the file is read only or hidden, reset these 2 attributes otherwise CopyFile() will fail.
                PostCopy(to);
            }

            ++m_soFar;
            if (m_soFar >= m_total)
                nResult = m_observer.OnProgress(255);
            else
                nResult = m_observer.OnProgress(255*m_soFar/m_total);

            hr = HRESULT_FROM_VIEW(nResult);
            if (FAILED(hr))
            {
                LOG(m_logger, ILogger::Error, L"User interface commanded engine to abort");
                SetFirstError(hr);
            }
        }

    public:
        //Do nothing here but derive class may do something.
        virtual void PostCopy(CString& to)
        {
        }

    private:
        void SetFirstError(DWORD err)
        {
            if (m_error == ERROR_SUCCESS)
                m_error = err;
        }
    };

    //There is a need to hold on the handle of the files we have locked.
    //Using the Decorator pattern seems like the best way to pass the handle information back 
    //to the CompositeDownloader.
    class XCopyForDownloadLoggerDecorator : public ILogger
    {
        ILogger& m_logger;
        CSimpleArray<HANDLE>& m_hFiles;
    public:
        XCopyForDownloadLoggerDecorator(CSimpleArray<HANDLE>& hFiles, ILogger& logger)
            : m_hFiles(hFiles)
            , m_logger(logger)
        {}
        virtual ~XCopyForDownloadLoggerDecorator() {}
    public:
        void AddLockFileHandle(HANDLE& handle)
        {
            m_hFiles.Add(handle);
        }

    public: // ILogger interface
        virtual void Log(LoggingLevel lvl, LPCWSTR szLogMessage) { m_logger.Log(lvl, szLogMessage); }
        virtual void LogFinalResult(LPCWSTR szLogMessage) { m_logger.LogFinalResult(szLogMessage); }
        virtual void PopSection(LPCWSTR str) { m_logger.PopSection(str); }
        virtual void PushSection(LPCWSTR strAction, LPCWSTR strDescription) {  m_logger.PushSection(strAction, strDescription); }
        virtual CPath GetFilePath() {  return m_logger.GetFilePath(); }
        virtual bool RenameLog(const CPath & pthNewName) { return m_logger.RenameLog(pthNewName); }
        virtual void OpenForAppend() { m_logger.OpenForAppend(); }
        virtual void Close() { m_logger.Close(); }

        virtual void BeginLogAsIs(LoggingLevel lvl, CString szLogStart) { m_logger.BeginLogAsIs(lvl, szLogStart); }
        virtual void LogLine(LPCWSTR szLine) { m_logger.LogLine(szLine); }
        virtual void LogStartList() { m_logger.LogStartList(); }
        virtual void LogListItem(LPCWSTR item) { m_logger.LogListItem(item); }
        virtual void LogEndList() { m_logger.LogEndList(); }
        virtual void EndLogAsIs() { m_logger.EndLogAsIs(); }

        virtual ILogger* Fork() 
        { 
            try
            {
                return new XCopyForDownloadLoggerDecorator(m_hFiles, *m_logger.Fork());
            }
            catch (...)
            {
                // return Un Decorated Null logger
                return &IronMan::NullLogger::GetNullLogger();
            }
        }

        virtual void Merge(ILogger* fork) { m_logger.Merge(fork); }
        virtual void DeleteFork(ILogger* fork)
        {
            XCopyForDownloadLoggerDecorator* wld = dynamic_cast<XCopyForDownloadLoggerDecorator*>(fork);
            if (wld)
            {
                m_logger.DeleteFork(&wld->m_logger);
                delete wld;
            }
            else
            {
                // this is Un Decorated Watson log. Just delete the fork.
                m_logger.DeleteFork(fork);
            }
        }
        virtual CPath GetForkedName() { return m_logger.GetForkedName(); }
    };

    class XCopyForDownload : public XCopy
    {
    public:
        XCopyForDownload(const CString& src
                        , const CString& dst
                        , unsigned int totalFiles
                        , IProgressObserver& observer
                        , ILogger& logger
                        , const bool& aborted)
            : XCopy(src, dst, totalFiles, observer, logger, aborted)
        {
        }

    private:
        //Reset the file attribute if they are different.
        virtual void PostCopy(CString& to)
        {
            DWORD attrib = 0;
            attrib = GetFileAttributes( to );
            if(attrib != 0xFFFFFFFF)
            {
                attrib &= ~FILE_ATTRIBUTE_READONLY;
                SetFileAttributes( to, attrib );
            }

            //Lock the file so that no one can change it.
            //Not holding on the handle.  It will go away after the process terminated.
            HANDLE hFile = SmartLock::LockFile(CPath(to), m_logger);
            if (INVALID_HANDLE_VALUE != hFile)
            {
                XCopyForDownloadLoggerDecorator* log = dynamic_cast<XCopyForDownloadLoggerDecorator*>(&m_logger);
                if (NULL != log)
                {
                    log->AddLockFileHandle(hFile);
                }
            }
        }
    };

    struct DefCopyPredicate
    {
        bool operator() (const CString&)
        {
            return true;
        }
    };

    template <typename XCopy, typename CopyPredicate=DefCopyPredicate>
    class CopyPerformerT : public AbortPerformerBase
    {    
    //Needed by derived class
    protected:
        const CPath m_src;
        const CPath m_dst;
        ILogger& m_logger;
        CopyPredicate& m_copyPredicate;

    protected: // this ctor is for testing, only
        CopyPerformerT(const CPath& src, const CPath& dst, ILogger& logger)
            : m_src(src)
            , m_dst(dst)
            , m_logger(logger)
            , m_copyPredicate(DefCopyPredicate())
        {}
    public: // this is the real one
        CopyPerformerT(const CPath& dst, ILogger& logger)
            : m_src(SourceLocation::GetPath())
            , m_dst(dst)
            , m_logger(logger)
            , m_copyPredicate(DefCopyPredicate())
        {}   

        CopyPerformerT(const CPath& dst, CopyPredicate& cp, ILogger& logger)
            : m_src(SourceLocation::GetPath())
            , m_dst(dst)
            , m_logger(logger)
            , m_copyPredicate(cp)
        {}   

    public:
        class Counter
        {
            unsigned int m_count;
        public:
            Counter() : m_count(0) {}
            void operator()(const CString&) { ++m_count; }
            unsigned int GetCount() const { return m_count; }
        };


        virtual void PerformAction(IProgressObserver& observer)
        {
            CString section = L" complete";
            PUSHLOGSECTIONPOP(m_logger, L"Action", L"Copying Items", section);

            Counter counter;
            Recursor(m_src, counter, m_copyPredicate);

            XCopy xcopy(m_src, m_dst, counter.GetCount(), observer, m_logger, AbortFlagRef());
            Recursor(m_src, xcopy, m_copyPredicate);

            observer.OnStateChangeDetail(IProgressObserver::Copying, L"");
        }

    public:
        template <typename F, typename P> static void Recursor(const CString& src, F& functor, P& predicate, bool bIncludeDirectories=false)
        {
            ATLASSERT((CString::StringLength(src) > 0) && L"Forgot to set SourceLocation().");

            CFindFile ff;

            if (TRUE == ff.FindFile (src + _T("\\*.*")))
            {
                do {
                    if (TRUE != ff.IsDirectory())
                    {
                        CString filePath = ff.GetFilePath();
                        if (predicate(filePath))
                        {
                            functor(filePath);
                        }
                    }
                    else
                    {
                        if (TRUE == ff.IsDots())
                        {
                            continue;   // prevents infinite recursion
                        }

                        Recursor(ff.GetFilePath(), functor, bIncludeDirectories);
                        if( bIncludeDirectories )
                        {
                            functor(ff.GetFilePath());
                        }

                    }
                } while (TRUE == ff.FindNextFile());
            }
        }

        template <typename F> static void Recursor(const CString& src, F& functor, bool bIncludeDirectories=false)
        {
            Recursor(src, functor, DefCopyPredicate(), bIncludeDirectories);
        }
    };

    typedef CopyPerformerT<XCopy> CopyPerformer;
    typedef CopyPerformerT<XCopyForDownload> CopyPerformerForDownload;
  
    template<typename TCmdLineSwitches=CCmdLineSwitches>
    class CopyPackagePerfomer : public CopyPerformerForDownload
    {
    private:
        bool m_bIsThereAtLeastOneFileToDownload; 
        bool m_bShouldCopyPackageFiles;

    //Make it public for easy testing.
    public:
        class SizeAggreator
        {
            DWORD m_size;
            ILogger& m_logger;

        public:
            SizeAggreator(ILogger& logger) 
                : m_size(0) 
                , m_logger(logger)
            {}

            void operator()(const CString& filename) 
            {
                //Get size;
                DWORD size = 0;
                {
                    CAtlFile file;
                    //Open with FILE_SHARE_READ to prevent race condition failure.
                    HRESULT hr = file.Create(filename, GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING);
                    if (FAILED(hr))
                    {
                        //Just log the message, no point to fail here.
                        LOGEX(m_logger, ILogger::Warning, L"Cannot get the size of : %s", filename);
                    }
                    else
                    {
                        size = ::GetFileSize(file, NULL);
                    }
                }  
                m_size += size;
            }

            const DWORD GetSize() const 
            { 
                return m_size; 
            }
        };

    public:
        CopyPackagePerfomer(const IDownloadItems& items
                            , bool bIsThereAtLeastOneFileToDownload
                            , bool bShouldCopyPackageFiles
                            , ILogger& logger)
            : CopyPerformerForDownload(ComputeDestinationPath(items, logger), logger)
            , m_bIsThereAtLeastOneFileToDownload(bIsThereAtLeastOneFileToDownload)
            , m_bShouldCopyPackageFiles(bShouldCopyPackageFiles)
        {}

        CopyPackagePerfomer(const CPath& src
                            , const IDownloadItems& items
                            , bool bIsThereAtLeastOneFileToDownload
                            , bool bShouldCopyPackageFiles
                            , ILogger& logger)
            : CopyPerformerForDownload(src, ComputeDestinationPath(items, logger), logger)
            , m_bIsThereAtLeastOneFileToDownload(bIsThereAtLeastOneFileToDownload)
            , m_bShouldCopyPackageFiles(bShouldCopyPackageFiles)
        {}

        //Should not be copying in 3 conditions:
        // a. CreateLayout Mode 
        // b. if all items are in the package.
        // c. source == destination
        // d. CopyPackageFilesToDownloadLocation is false
        const bool IsCopying() const
        {
            //Layout swith
            TCmdLineSwitches switches;
            bool bInCreateLayoutMode = !switches.CreateLayout().IsEmpty();

            //Source == Destination
            bool bSameSourceAndDestination = (CString(m_src).CompareNoCase(CString(m_dst)) == 0);

            return (!bInCreateLayoutMode && !bSameSourceAndDestination && m_bShouldCopyPackageFiles && m_bIsThereAtLeastOneFileToDownload);
        }

        const DWORD GetTotalCopySize() const
        {
            DWORD dwSize = 0;
            if (IsCopying())
            {
                //Loop through source to find the actual size
                SizeAggreator size(m_logger);
                Recursor(CString(m_src), size);
                dwSize = size.GetSize();
            }
            return dwSize;
        }

        //For testing purpose
        CPath GetDestinationPath() const
        {
            return m_dst;
        }

    private:
        //Compute the destination path.
        //The logic is to use the last path gathered.
        static const CPath ComputeDestinationPath(const IDownloadItems& items, ILogger& logger)
        {
            CPath dstCurrentPath;

            for(unsigned int i=0; i<items.GetCount(); ++i)
            {
                CUrl src;
                CString hash;
                CPath dstPath;
                ULONGLONG itemSize = 0;
                bool bIgnoreDownloadFailure = false;
                CString itemName;
                CString itemId;
                items.GetItem(i, &src, hash, dstPath, itemSize, bIgnoreDownloadFailure, itemName, itemId);

                //If the same helper item is defined 5 times, the 1st one has state == AvailableUnVerified 
                //and the 4 others have state == Available.
                if (items.IsItemAvailableUnVerified(i) || items.IsItemAvailable(i))
                {
                    CPath path(ModuleUtils::GetDllPath());
                    path.Append(itemName);

                    if (CString(dstPath).CompareNoCase(path) == 0)
                    {
                        continue;
                    }
                }

                //Strip out the itemName
                CString path(dstPath);
                int itemNameLength = itemName.GetLength() + 1;
                if ('\\' == itemName[0])
                {
                    --itemNameLength;
                }
                dstCurrentPath = CPath(path.Left(path.GetLength() - itemNameLength));

            }
            return dstCurrentPath;
        }
    };

    //
    // Copies a single file
    //
    template <typename XCopy>
    class CopyFilePerformerT : public AbortPerformerBase
    {    
        CPath m_srcFile;
        CPath m_dstFile;
        ILogger& m_logger;

    public: 
        CopyFilePerformerT(const CPath& srcFile, const CPath& dstFile, ILogger& logger)
            : m_srcFile(srcFile)
            , m_dstFile(dstFile)
            , m_logger(logger)
        {}

    public:
        virtual void PerformAction(IProgressObserver& observer)
        {
            CString section = L" complete";
            PUSHLOGSECTIONPOP(m_logger, L"Action", L"Copying File", section);

            XCopy xcopy(m_srcFile, m_dstFile, 1, observer, m_logger, AbortFlagRef());
            xcopy(m_srcFile);

            observer.OnStateChangeDetail(IProgressObserver::Copying, L"");
        }
    };

    typedef CopyFilePerformerT<XCopy> CopyFilePerformer;
    
}
