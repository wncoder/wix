//-------------------------------------------------------------------------------------------------
// <copyright file="FindFile.h" company="Microsoft">
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

namespace IronMan
{

// sub-set of CFindFile from WTL 8.0 Internal (atlmisc.h)
class CFindFile
{
public:
// Data members
    WIN32_FIND_DATA m_fd;
    mutable TCHAR m_lpszRoot[MAX_PATH];
    TCHAR m_chDirSeparator;
    HANDLE m_hFind;
    BOOL m_bFound;

// Constructor/destructor
    CFindFile() : m_hFind(NULL), m_chDirSeparator(_T('\\')), m_bFound(FALSE)
    {}
    virtual ~CFindFile()
    {
        Close();
    }

// Attributes
    CString GetFileName() const
    {
        IMASSERT(m_hFind != NULL);

        if(m_bFound)
            return CString(m_fd.cFileName, MAX_PATH);
        return CString();
    }

    CString GetFilePath() const
    {
        IMASSERT(m_hFind != NULL);

        m_lpszRoot[MAX_PATH-1] = 0;
        CString strResult = m_lpszRoot;
        int nLen = strResult.GetLength();
        IMASSERT(nLen > 0);
        if(nLen == 0)
            return strResult;

        if((strResult[nLen - 1] != _T('\\')) && (strResult[nLen - 1] != _T('/')))
            strResult += m_chDirSeparator;
        strResult += GetFileName();
        return strResult;
    }

    CString GetRoot() const
    {
        IMASSERT(m_hFind != NULL);
        m_lpszRoot[MAX_PATH-1] = 0;
        return m_lpszRoot;
    }

    BOOL GetLastWriteTime(FILETIME* pTimeStamp) const
    {
        IMASSERT(m_hFind != NULL);
        IMASSERT(pTimeStamp != NULL);

        if(m_bFound && pTimeStamp != NULL)
        {
            *pTimeStamp = m_fd.ftLastWriteTime;
            return TRUE;
        }

        return FALSE;
    }

#ifdef NOT_USED_HERE // but handy, so keep these around
    ULONGLONG GetFileSize() const
    {
        IMASSERT(m_hFind != NULL);

        ULARGE_INTEGER nFileSize = { 0 };

        if(m_bFound)
        {
            nFileSize.LowPart = m_fd.nFileSizeLow;
            nFileSize.HighPart = m_fd.nFileSizeHigh;
        }
        else
        {
            nFileSize.QuadPart = 0;
        }

        return nFileSize.QuadPart;
    }

    BOOL GetLastAccessTime(FILETIME* pTimeStamp) const
    {
        IMASSERT(m_hFind != NULL);
        IMASSERT(pTimeStamp != NULL);

        if(m_bFound && pTimeStamp != NULL)
        {
            *pTimeStamp = m_fd.ftLastAccessTime;
            return TRUE;
        }

        return FALSE;
    }

    BOOL GetCreationTime(FILETIME* pTimeStamp) const
    {
        IMASSERT(m_hFind != NULL);

        if(m_bFound && pTimeStamp != NULL)
        {
            *pTimeStamp = m_fd.ftCreationTime;
            return TRUE;
        }

        return FALSE;
    }
#endif

    BOOL MatchesMask(DWORD dwMask) const
    {
        IMASSERT(m_hFind != NULL);

        if(m_bFound)
            return ((m_fd.dwFileAttributes & dwMask) != 0);

        return FALSE;
    }

    BOOL IsDots() const
    {
        IMASSERT(m_hFind != NULL);

        // return TRUE if the file name is "." or ".." and
        // the file is a directory

        BOOL bResult = FALSE;
        if(m_bFound && IsDirectory())
        {
            if(m_fd.cFileName[0] == _T('.') && (m_fd.cFileName[1] == _T('\0') || (m_fd.cFileName[1] == _T('.') && m_fd.cFileName[2] == _T('\0'))))
                bResult = TRUE;
        }

        return bResult;
    }

    BOOL IsReadOnly()   const { return MatchesMask(FILE_ATTRIBUTE_READONLY);  }
    BOOL IsDirectory()  const { return MatchesMask(FILE_ATTRIBUTE_DIRECTORY); }
    BOOL IsCompressed() const { return MatchesMask(FILE_ATTRIBUTE_COMPRESSED);}
    BOOL IsSystem()     const { return MatchesMask(FILE_ATTRIBUTE_SYSTEM);    }
    BOOL IsHidden()     const { return MatchesMask(FILE_ATTRIBUTE_HIDDEN);    }
    BOOL IsTemporary()  const { return MatchesMask(FILE_ATTRIBUTE_TEMPORARY); }
    BOOL IsNormal()     const { return MatchesMask(FILE_ATTRIBUTE_NORMAL);    }
    BOOL IsArchived()   const { return MatchesMask(FILE_ATTRIBUTE_ARCHIVE);   }

// Operations
    BOOL FindFile(LPCTSTR pstrName = NULL)
    {
        Close();

        if(pstrName == NULL)
        {
            pstrName = _T("*.*");
        }
        else if(_tcslen(pstrName) >= MAX_PATH)
        {
            IMASSERT(FALSE);
            return FALSE;
        }

        _tcscpy_s(m_fd.cFileName, _countof(m_fd.cFileName), pstrName);

        m_hFind = ::FindFirstFile(pstrName, &m_fd);

        if(m_hFind == INVALID_HANDLE_VALUE)
            return FALSE;

        bool bFullPath = (::GetFullPathName(pstrName, MAX_PATH, m_lpszRoot, NULL) != 0);

        // passed name isn't a valid path but was found by the API
        IMASSERT(bFullPath);
        if(!bFullPath)
        {
            Close();
            ::SetLastError(ERROR_INVALID_NAME);
            return FALSE;
        }
        else
        {
            // find the last forward or backward whack
            LPTSTR pstrBack  = (LPTSTR)_cstrrchr(m_lpszRoot, _T('\\'));
            LPTSTR pstrFront = (LPTSTR)_cstrrchr(m_lpszRoot, _T('/'));

            if(pstrFront != NULL || pstrBack != NULL)
            {
                if(pstrFront == NULL)
                    pstrFront = m_lpszRoot;
                if(pstrBack == NULL)
                    pstrBack = m_lpszRoot;

                // from the start to the last whack is the root

                if(pstrFront >= pstrBack)
                    *pstrFront = _T('\0');
                else
                    *pstrBack = _T('\0');
            }
        }

        m_bFound = TRUE;

        return TRUE;
    }

    BOOL FindNextFile()
    {
        IMASSERT(m_hFind != NULL);

        if(m_hFind == NULL)
            return FALSE;

        if(!m_bFound)
            return FALSE;

        m_bFound = ::FindNextFile(m_hFind, &m_fd);

        return m_bFound;
    }

    void Close()
    {
        m_bFound = FALSE;

        if(m_hFind != NULL && m_hFind != INVALID_HANDLE_VALUE)
        {
            ::FindClose(m_hFind);
            m_hFind = NULL;
        }
    }

// Helper
    static const TCHAR* _cstrrchr(const TCHAR* p, TCHAR ch)
    {
#ifdef _ATL_MIN_CRT
        const TCHAR* lpsz = NULL;
        while (*p != 0)
        {
            if (*p == ch)
                lpsz = p;
            p = ::CharNext(p);
        }
        return lpsz;
#else // !_ATL_MIN_CRT
        return _tcsrchr(p, ch);
#endif // !_ATL_MIN_CRT
    }
};
}
