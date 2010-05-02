//-------------------------------------------------------------------------------------------------
// <copyright file="CacheManager.h" company="Microsoft">
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
//      Cache manager implementation
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once

#include "ICacheManager.h"

namespace IronMan
{

//---------------------------------------------------------------------------------------------------------
// CacheManager class
// class for caching packages
//---------------------------------------------------------------------------------------------------------
class CacheManager : public ICacheManager
{
    CPath m_pathCacheCommonRoot;
    CPath m_pathCacheLocalRoot;
    CString m_csBundleId;
    ILogger& m_logger;
    CRITICAL_SECTION m_cs;
public:
    //---------------------------------------------------------------------------------------------------------
    // Constructor
    //---------------------------------------------------------------------------------------------------------
    CacheManager(const CString& csBundleId, ILogger& logger)
        : m_csBundleId(csBundleId)
        , m_logger(logger)
    {
        InitializeCriticalSection(&m_cs);
        CString csCacheCommonRoot;
        SHGetFolderPath(NULL, CSIDL_COMMON_APPDATA, NULL, SHGFP_TYPE_CURRENT, csCacheCommonRoot.GetBuffer(MAX_PATH));
        csCacheCommonRoot._ReleaseBuffer();
        m_pathCacheCommonRoot = CPath(csCacheCommonRoot);

        CString csCacheLocalRoot;
        SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, SHGFP_TYPE_CURRENT, csCacheLocalRoot.GetBuffer(MAX_PATH));
        csCacheLocalRoot._ReleaseBuffer();
        m_pathCacheLocalRoot = CPath(csCacheLocalRoot);
    }


    //---------------------------------------------------------------------------------------------------------
    // Destructor
    //---------------------------------------------------------------------------------------------------------
    virtual ~CacheManager()
    {
        ::DeleteCriticalSection(&m_cs);
    }

public:

    //---------------------------------------------------------------------------------------------------------
    // MoveFileToCached
    // Move file to cache
    //---------------------------------------------------------------------------------------------------------
    static HRESULT MoveFileToCached(
        __in LPCWSTR wzSourceFile
        , const CPath&  pathCachedFile
        , ILogger& logger)
    {
        HRESULT hr = S_OK;
        // Make sure Cache directory Exites
        CPath pathCacheDir(pathCachedFile);
        pathCacheDir.RemoveFileSpec();
        hr = DirEnsureExists(pathCacheDir, NULL);
        if ( !SUCCEEDED(hr) )
        {
            LOGEX(logger, ILogger::Error, L"Failed create payload cache directory %s", wzSourceFile, pathCacheDir);
            return hr;
        }

        // The file should be moved only if it is under the temp directory. Otherwise copy
        CString tempPath;
        ::GetTempPath(_MAX_DIR, tempPath.GetBuffer(_MAX_DIR));
        tempPath._ReleaseBuffer();
        CString csSourceFile(wzSourceFile);
        csSourceFile.MakeLower();
        if ( -1 != csSourceFile.Find(tempPath.MakeLower()) )
        {
            // Under temp directory.  Move Payload to Cache Directory
            hr = FileEnsureMove(wzSourceFile, pathCachedFile, TRUE, TRUE);
            if ( !SUCCEEDED(hr) )
            {
                LOGEX(logger, ILogger::Error, L"Failed to move file %s to %s", wzSourceFile, pathCachedFile);
                return hr;
            }
        }
        else
        {
            // not under temp directory. Copy Payload to Cache Directory
            hr = FileEnsureCopy(wzSourceFile, pathCachedFile, TRUE);
            if ( !SUCCEEDED(hr) )
            {
                LOGEX(logger, ILogger::Error, L"Failed to copy file %s to %s", wzSourceFile, pathCachedFile);
                return hr;
            }
        }
        return hr;
    }


    //---------------------------------------------------------------------------------------------------------
    // DeleteCachedPackage
    // Attempt to delete cached package.  If cannot delete, then move it to the temp directory
    // Also deletes the directory that the cached package is in
    //---------------------------------------------------------------------------------------------------------
    virtual HRESULT DeleteCachedPackage(IItems& items, unsigned int nIndex, UnElevatedController* pUnElevatedController)
    {
        CPath pathCacheFile;
        bool bPerMachine;
        bool bCacheItem;
        GetCachedPath(items, nIndex, pathCacheFile, bPerMachine, bCacheItem);
        return DeleteCachedPackage(pathCacheFile, bPerMachine, pUnElevatedController, m_logger);
    }


    //---------------------------------------------------------------------------------------------------------
    // DeleteCachedPackage
    // Attempt to delete cached package.  If cannot delete, then move it to the temp directory
    // Also deletes the directory that the cached package is in
    //---------------------------------------------------------------------------------------------------------
    static HRESULT DeleteCachedPackage(
        __in LPCWSTR wzFile
        , bool bPerMachine
        , UnElevatedController* pUnElevatedController
        , ILogger& logger)
    {
        if ( NULL != pUnElevatedController && bPerMachine )
        {
            return pUnElevatedController->DeleteCachedPackage(wzFile);
        }
        else
        {
            // Delete cached file
            HRESULT hr = CacheManager::DeleteCachedFile(wzFile, logger);

            // Now attempt to delete the directory the package is in
            CPath pathPackageCachedDir(wzFile);
            if ( pathPackageCachedDir.RemoveFileSpec() && pathPackageCachedDir.IsDirectory() )
            {
                pathPackageCachedDir.AddBackslash();
                hr = DirEnsureDelete(pathPackageCachedDir, true, true);
                if ( FAILED(hr) )
                {
                    LOGEX(logger, ILogger::Verbose, L"Failed to delete package cache directory " + pathPackageCachedDir + L".  %u.", hr);
                }
            }
            return hr;
        }
    }

    //---------------------------------------------------------------------------------------------------------
    // DeleteCachedFile
    // Attempt to delete cached file.  If cannot delete, then move it to the temp directory
    //---------------------------------------------------------------------------------------------------------
    static HRESULT DeleteCachedFile(
        __in LPCWSTR wzFile
        , ILogger& logger)
    {
        HRESULT hr = S_OK;
        if ( FAILED(FileEnsureDelete(wzFile)) )
        {
            // Delete file failed, attempt to move it to the temp directory with a temp file name
            // FileEnsureDelete already cleared any attributes that might get in the way
            WCHAR wzTempDirectory[MAX_PATH];
            WCHAR wzTempPath[MAX_PATH];
            // Get temp directory
            if (!::GetTempPathW(countof(wzTempDirectory), wzTempDirectory))
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
                LOGEX(logger, ILogger::Verbose, L"Failed to get temp directory.  %u.", hr);
            }
            else
            {
                // Get full path to Temp file
                if (!::GetTempFileNameW(wzTempDirectory, L"BRN", 0, wzTempPath))
                {
                    hr = HRESULT_FROM_WIN32(GetLastError());
                    LOGEX(logger, ILogger::Verbose, L"Failed to get temp file to move cache file to.  %u.", hr);
                }
                else
                {
                    // We'll ignore failures to remove files for now.
                    if (!::MoveFileExW(wzFile, wzTempPath, MOVEFILE_REPLACE_EXISTING))
                    {
                        hr = HRESULT_FROM_WIN32(GetLastError());
                        LOGEX(logger, ILogger::Verbose, L"Failed to move cache file to temp directory.  %u.", hr);
                    }
                }
            }
        }
        return hr;
    }

    //---------------------------------------------------------------------------------------------------------
    // DeleteTemporaryCacheDirectories
    // Deletes the temporary cache directories
    //---------------------------------------------------------------------------------------------------------
    virtual void DeleteTemporaryCacheDirectories(UnElevatedController* pUnElevatedController) const
    {
        CPath pathTempCachedDir;

        // PerMachine
        pathTempCachedDir.Append(m_pathCacheCommonRoot);
        pathTempCachedDir.Append(L"Apps\\Cache\\temp");
        pathTempCachedDir.Append(m_csBundleId);
        if (pathTempCachedDir.IsDirectory() && NULL != pUnElevatedController)
        {
            pUnElevatedController->DeleteTemporaryCacheDirectory();
        }

        // PerUser
        pathTempCachedDir = m_pathCacheLocalRoot;
        pathTempCachedDir.Append(L"Apps\\Cache\\temp");
        pathTempCachedDir.Append(m_csBundleId);
        if (pathTempCachedDir.IsDirectory())
        {
            pathTempCachedDir.AddBackslash();
            DirEnsureDelete(pathTempCachedDir, true, true);
        }
    }

    //---------------------------------------------------------------------------------------------------------
    // IsCached
    // Check to see if the item is cached on the machine already and is Valid.
    // If it is cached but not valid, then it will be deleted and it will need to be cached again
    // If it is cached and valid will change the state to Available
    //---------------------------------------------------------------------------------------------------------
    virtual bool IsCached(IItems& items, unsigned int nIndex, UnElevatedController* pUnElevatedController = NULL)
    {
        bool bPerMachine;
        bool bCacheItem;
        // Protect simultaneous calls from Download and Install threads
        ::EnterCriticalSection(&m_cs);
        bool bCacheFileExistsAndIsValid = false;
        // First see if file exists in the permanant cache
        CPath pathCacheFile;
        GetCachedPath(items, nIndex, pathCacheFile, bPerMachine, bCacheItem);
        if (pathCacheFile.FileExists())
        {
            // Check to see if the LocalPath is already set to the Cached File and if it is in the Available State(already verified)
            CString csSourceFile(items.GetItemName(nIndex));
            if ( 0 == csSourceFile.CompareNoCase(pathCacheFile) && items.IsItemAvailable(nIndex) )
            {
                LOG(m_logger, ILogger::Information, L" Cached file exists and has already been verified.");
                bCacheFileExistsAndIsValid = true;
            }
            else
            {
                if ( NULL != pUnElevatedController && bPerMachine )
                {
                    bCacheFileExistsAndIsValid = (S_OK == pUnElevatedController->IsCached(nIndex));
                }
                else
                {
                    // Store item path and set item path to cache so it can be verified
                    CPath pathOriginalItem = items.GetItemName(nIndex);
                    // update the LocalPath.  Do this before SetItemStateAsAvailable as UpdateItemPath resets ItemState to AvailableUnverified
                    items.UpdateItemPath(nIndex, pathCacheFile);
                    if (items.VerifyItem(nIndex, m_logger))
                    {
                        if (VerifyChildItems(items, nIndex))
                        {
                            // Changes state from to Available, since a valid cached file is avaliable
                            items.SetItemStateAsAvailable(nIndex); 
                            LOG(m_logger, ILogger::Information, L" Cached file already exists and has been verified.");
                            bCacheFileExistsAndIsValid = true;
                        }
                        else
                        {
                            bCacheFileExistsAndIsValid = false;
                            // reset path to original item location
                            items.UpdateItemPath(nIndex, pathOriginalItem);
                        }
                    }
                    else
                    {
                        bCacheFileExistsAndIsValid = false;
                        // Attempt to delete invalid cached file
                        // reset path to original item location
                        items.UpdateItemPath(nIndex, pathOriginalItem);
                        // Cached item is not valid, delete file so it can be re-cached
                        if ( SUCCEEDED(CacheManager::DeleteCachedPackage(pathCacheFile, bPerMachine, pUnElevatedController, m_logger)) )
                        {
                            LOGEX(m_logger, ILogger::Verbose, L"Failed to verify and authenticate the cached file -%s.  Deleting cached file so that it will be re-cached.", pathCacheFile);
                        }
                        else
                        {
                            LOGEX(m_logger, ILogger::Verbose, L"Failed to delete invalid cached file %s.  Will try to recache it later", pathCacheFile);
                        }
                    }
                }
            }
        }
        else
        {
            // File does not exist
            bCacheFileExistsAndIsValid = false;
        }

        ::LeaveCriticalSection(&m_cs);
        return bCacheFileExistsAndIsValid;
    }

    //---------------------------------------------------------------------------------------------------------
    // VerifyChildItems
    // Verify child payload item
    //---------------------------------------------------------------------------------------------------------
    bool VerifyChildItems(IItems& items, unsigned int nParentIndex) const
    {
        // Get the directory the parent item is in
        CPath pathParentDir(items.GetItemName(nParentIndex));
        pathParentDir.RemoveFileSpec();

        for ( unsigned int nChildIndex = 0; nChildIndex < items.GetChildItemCount(nParentIndex); ++nChildIndex)
        {
            const ItemBase* pItemBase = items.GetChildItem(nParentIndex, nChildIndex);
            const LocalPath* localPath = dynamic_cast<const LocalPath*>(pItemBase);
            if (!localPath)
            {
                return false;
            }
            // Get path to child item in parent directory
            CPath pathChildItem(pathParentDir);
            pathChildItem.Append(localPath->GetOriginalName());
            if ( !VerifyItem(pItemBase, pathChildItem, true))
            {
                return false;
            }
        }
        return true;
    }

    //---------------------------------------------------------------------------------------------------------
    // VerifiyAndCacheItem
    // Verify item and then cache it
    //---------------------------------------------------------------------------------------------------------
    virtual HRESULT VerifyAndCachePackage(IInstallItems& items, unsigned int nIndex, UnElevatedController* pUnElevatedController = NULL)
    {
        HRESULT hr = S_OK;
        bool bPerMachine;
        bool bCacheItem;
        bool bWasItemCached = false;

        CPath src(items.GetItemName(nIndex));
        if ( src.IsRelative() )
        {
            hr = E_FAIL;
        }
        else
        {
            if ( !src.FileExists() )
            {
                hr = E_FAIL;
            }
            else
            {
                // Check if file is already cached and validated
                if ( !IsCached(items, nIndex, pUnElevatedController) )
                {
                    LOG(m_logger, ILogger::Verbose, L"Caching : " + src);

                    // Item not cached, cache it
                    CPath pathCacheFile;
                    GetCachedPath(items, nIndex, pathCacheFile, bPerMachine, bCacheItem);

                    // If bDoesItemNeedToBeCached is false, but the file is in the temp directory
                    // then the file should be temporarily cached
                    CString tempPath;
                    ::GetTempPath(_MAX_DIR, tempPath.GetBuffer(_MAX_DIR));
                    tempPath._ReleaseBuffer();
                    CString csSourceFile(src);
                    csSourceFile.MakeLower();
                    if (pUnElevatedController != NULL && bPerMachine)
                    {
                        hr = pUnElevatedController->VerifyAndCachePackage(nIndex, src);
                    }
                    else
                    {
                        if (!items.VerifyItem(nIndex, m_logger))
                        {
                            hr = TRUST_E_FAIL;
                            LOGEX(m_logger, ILogger::Error, L"Failed to verify and authenticate the file -%s", src);
                            LOGEX(m_logger, ILogger::Error, L"Please delete the file, %s and run the package again", src);
                        }
                        else
                        {
                            // Verify and Cache child payload items
                            bCacheItem = bCacheItem || (-1 != csSourceFile.Find(tempPath.MakeLower()));
                            hr = VerifyAndCacheChildItems(items, nIndex, pathCacheFile, bCacheItem);
                            if ( SUCCEEDED(hr) && bCacheItem )
                            {
                                // Cache the verified item
                                hr = CacheManager::MoveFileToCached(src, pathCacheFile, m_logger);
                                bWasItemCached = SUCCEEDED(hr);

                            }
                        }
                        if (SUCCEEDED(hr))
                        {
                            if (bWasItemCached)
                            {
                                // update the LocalPath.  Do this before SetItemStateAsAvailable as UpdateItemPath resets ItemState to AvailableUnverified
                                items.UpdateItemPath(nIndex, pathCacheFile);
                            }
                            items.SetItemStateAsAvailable(nIndex); 
                        }
                    }
                }
            }
        }
        return hr;
    }

    //---------------------------------------------------------------------------------------------------------
    // VerifyAndCacheChildItems
    // Verify child payload item and then cache it
    //---------------------------------------------------------------------------------------------------------
    HRESULT VerifyAndCacheChildItems(IInstallItems& items, unsigned int nParentIndex, const CPath& pathParentDest, bool bCacheItem = true) const
    {
        // Get the directory the parent item is in
        CPath pathParentSource(items.GetItemName(nParentIndex));
        pathParentSource.RemoveFileSpec();
        HRESULT hr = S_OK;
        for ( unsigned int nChildIndex = 0; nChildIndex < items.GetChildItemCount(nParentIndex); ++nChildIndex)
        {
            const ItemBase* pItemBase = items.GetChildItem(nParentIndex, nChildIndex);
            const LocalPath* localPath = dynamic_cast<const LocalPath*>(pItemBase);
            if (!localPath)
            {
                LOG(m_logger, ILogger::Error, L"Failed to cast child payload ItemBase to LocalPath");
                return E_FAIL;
            }
            CPath childItemName(localPath->GetName());
            childItemName.StripPath();
            LOG(m_logger, ILogger::Verbose, L"Caching : " + childItemName);
            CPath pathChildItemSrc(pathParentSource);
            pathChildItemSrc.Append(childItemName);
            if (pathChildItemSrc.FileExists())
            {
                // Verify source item
                hr = VerifyItem(pItemBase, pathChildItemSrc);
                if ( SUCCEEDED(hr) )
                {
                    if (!bCacheItem)
                    {
                        // don't cache child item, just continue to verify child items
                        continue;
                    }
                    // Get destination path to child item in parent directory
                    CPath pathChildItemDest(pathParentDest);
                    pathChildItemDest.RemoveFileSpec();
                    pathChildItemDest.Append(localPath->GetOriginalName());

                    // Copy Payload to Cache Directory
                    // Need to copy and not move in case this is used by multiple packages
                    hr = FileEnsureCopy(pathChildItemSrc, pathChildItemDest, TRUE);
                    if ( !SUCCEEDED(hr) )
                    {
                        LOGEX(m_logger, ILogger::Error, L"Failed to copy file %s to %s", pathChildItemSrc, pathChildItemDest);
                        return hr;
                    }
                }
            }
            else
            {
                LOGEX(m_logger, ILogger::Verbose, L"File not found: %s", pathChildItemSrc);
                return E_FAIL;
            }
        }
        return hr;
    }

private:
    //---------------------------------------------------------------------------------------------------------
    // VerifyItem
    // Verify item in a specific location
    //---------------------------------------------------------------------------------------------------------
    HRESULT VerifyItem(const ItemBase* pItemBase, const CPath& pathFile, bool bDeleteBadFile = false) const
    {
        HRESULT hr = S_OK;
        if ( pathFile.FileExists() )
        {
            const LocalPath* localPath = dynamic_cast<const LocalPath*>(pItemBase);
            if (!localPath)
            {
                return false;
            }
            CString strFriendlyName(localPath->GetOriginalName());

            const DownloadPath* downloadPath = dynamic_cast<const DownloadPath*>(pItemBase);
            if (downloadPath) // might be media only, in which case we never downloaded anything.
            {
                FileAuthenticity fileAuthenticity(strFriendlyName, pathFile, downloadPath->GetHashValue(), downloadPath->GetItemSize(), m_logger);
                hr = fileAuthenticity.Verify();
                if (SUCCEEDED(hr))
                {
                    // File verified
                    return hr;
                }
                else
                {
                    if (bDeleteBadFile)
                    {
                        // Cached item is not valid, delete file so it can be re-cached
                        if ( SUCCEEDED(CacheManager::DeleteCachedFile(pathFile, m_logger)) )
                        {
                            LOGEX(m_logger, ILogger::Verbose, L"Failed to verify and authenticate the file -%s.  Deleting cached file so that it will be re-cached.", pathFile);
                        }
                        else
                        {
                            LOGEX(m_logger, ILogger::Verbose, L"Failed to delete invalid file %s.  Will try to recache it later", pathFile);
                        }
                    }
                    else
                    {
                        LOGEX(m_logger, ILogger::Result, L"File %s (%s), failed authentication. (Error = %d). It is recommended that you delete this file and retry setup again.", strFriendlyName, pathFile, hr);
                    }
                    return hr;
                }
            }
            else
            {
                LOGEX(m_logger, ILogger::Verbose, L"File not found: %s", strFriendlyName);
                return hr;
            }
        }
        else
        {
            LOGEX(m_logger, ILogger::Verbose, L"File not found: %s", pathFile);
            return hr;
        }
    }

protected: // instead of private, for unit testing

    // Example Paths
    // "C:\ProgramData\Applications\Cache\Sha1HashValue\Sha1HashValue.exe"
    // "C:\ProgramData\Applications\Cache\{BundleId}\Sha1HashValue\Sha1HashValue.exe"
    // "C:\Users\<user>\AppData\Local\Applications\Cache\Sha1HashValue\Sha1HashValue.exe"
    // "C:\Users\<user>\AppData\Local\Applications\Cache\{BundleId}\Sha1HashValue\Sha1HashValue.exe"
    //---------------------------------------------------------------------------------------------------------
    // GetCachedPath
    // Gets the Cached Path.
    // if bDoesItemNeedToBeCached is returned false, then the package will only be cached temporarily
    //---------------------------------------------------------------------------------------------------------
    void GetCachedPath(IItems& items, unsigned int nIndex, CPath& pathCachePath, bool& bPerMachine, bool& bDoesItemNeedToBeCached) const
    {
        CPath pathCachedFileName;
        bDoesItemNeedToBeCached = items.DoesItemNeedToBeCached(nIndex, pathCachedFileName, bPerMachine);

        if ( bPerMachine )
        {
            pathCachePath.Append(m_pathCacheCommonRoot);
        }
        else
        {
            pathCachePath.Append(m_pathCacheLocalRoot);
        }

        pathCachePath.Append(L"Apps\\Cache");

        if ( !bDoesItemNeedToBeCached )
        {
            // Temporary Cache
            pathCachePath.Append(L"temp");
            pathCachePath.Append(m_csBundleId);
        }

        pathCachePath.Append(pathCachedFileName);
    }
};
}