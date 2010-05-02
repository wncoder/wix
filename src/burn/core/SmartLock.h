//-------------------------------------------------------------------------------------------------
// <copyright file="SmartLock.h" company="Microsoft">
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

#include "schema\enginedata.h"
#include "Interfaces\ILogger.h"
#include "CheckTrust.h"

namespace IronMan
{
class SmartLock
{
    const ItemBase* m_pItem;
    bool m_bFileLocked;
    bool m_bDontUnlock;

public:
    SmartLock(const ItemBase* pItem) : m_pItem(pItem), m_bFileLocked(false), m_bDontUnlock(false)
    {
    }
    virtual ~SmartLock()
    {
        if (!m_bDontUnlock)
            Unlock();
    }

    // Verifies and Locks
    bool VerifyAndLock(bool bVerify, bool bLock, ActionTable::Actions currentAction, ILogger& logger)
    {
        if (!m_pItem)
            return false;
        const LocalPath* localPath = dynamic_cast<const LocalPath*>(m_pItem);
        if (!localPath)
        {
            // For items like ServiceControl, CleanupBlock, there is no path to verify.
            return true;
        }

        const ActionTable* actionTable = dynamic_cast<const ActionTable*>(m_pItem);
        if (actionTable)
        {
            if (!actionTable->IsPayloadRequired(currentAction))
                return true;
        }

        if (localPath->GetName().FileExists())
        {
            const DownloadPath* downloadPath = dynamic_cast<const DownloadPath*>(m_pItem);
            if (downloadPath) // might be media only, in which case we never downloaded anything.
            {
                CPath file = localPath->GetName(); // must make a copy, otherwise the copy goes out of scope at the ; at the end of the following line.

                if (bVerify)
                {
                    FileAuthenticity fileAuthenticity(localPath->GetOriginalName(),file, downloadPath->GetHashValue(), downloadPath->GetItemSize(), logger);
                    HRESULT hr = fileAuthenticity.Verify();
                    if (FAILED(hr))
                    {
                        LOGEX(logger, ILogger::Result, L"File %s, failed authentication. (Error = %d). It is recommended that you delete this file and retry setup again.", file, hr);
                        return false;
                    }
                    else
                    {
                        LOGEX(logger, ILogger::Verbose, L"File %s, is verified successfully. ", file);
                    }
                }
                // Lock file
                if (bLock)
                {
                    HANDLE hFile = LockFile(localPath->GetName(), logger);
                    if (INVALID_HANDLE_VALUE == hFile)
                    {
                        return false;
                    }
                    else
                    {
                        const_cast<ItemBase*>(m_pItem)->SetHandle(hFile);
                        m_bFileLocked = true;
                    }
                } // if Lock
            } // valid download path
        } // file exists
        else
        {
            LOGEX(logger, ILogger::Verbose, L"File does not exist to lock: %s", localPath->GetName());
            return false;
        } // file doesn't exist
        // file exists, and verifies. 
        return true;
    } // VerifyAndLock

    void DontUnlock()
    {
        m_bDontUnlock = true;
    }

    void Unlock()
    {
        if (m_bFileLocked)
        {
            const_cast<ItemBase*>(m_pItem)->CloseHandle();
            m_bFileLocked = false;
        }
    }

    static HANDLE LockFile(const CPath& pthFileName, ILogger& logger)
    {
        // Lock file
        HANDLE hFile = ::CreateFile(pthFileName
                                    , GENERIC_READ
                                    , FILE_SHARE_READ
                                    , NULL
                                    , OPEN_EXISTING
                                    , FILE_ATTRIBUTE_NORMAL
                                    , NULL);

        if (INVALID_HANDLE_VALUE == hFile)
        {
            LOGEX(logger, ILogger::Verbose, L"Failed to lock file %s.", pthFileName);
        }
        else
        {
            LOGEX(logger, ILogger::Verbose, L"File %s, locked for install. ", pthFileName);
        }
        return hFile;
    }

}; // SmartLock
} // namespace