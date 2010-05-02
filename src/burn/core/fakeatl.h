//-------------------------------------------------------------------------------------------------
// <copyright file="fakeatl.h" company="Microsoft">
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

#if 0

#define ATL_URL_MAX_URL_LENGTH  (32 \ + sizeof("://") \ + 2048)


class CString
{
public:
    CString()
    {
    }

    CString(LPCWSTR wz)
    {
    }

    CString(WCHAR c)
    {
    }

    CString(LPCSTR wz)
    {
    }

    CString(LPCWSTR wz, DWORD n)
    {
    }

    CString(const CString& wz)
    {
    }

    operator LPCWSTR() const
    {
        return NULL;
    }

    CString operator+(LPCWSTR wz)
    {
        return CString();
    }

    CString operator+(const CString& wz)
    {
        return CString();
    }

    CString operator+=(LPCWSTR wz)
    {
        return CString();
    }

    CString operator+=(WCHAR wc)
    {
        return CString();
    }

    bool operator==(const CString& s)
    {
        return false;
    }

    bool operator==(LPCWSTR s)
    {
        return false;
    }

    //CString operator+=(const CString& wz)
    //{
    //    return CString();
    //}

    void FormatV(LPCWSTR wzFormat, va_list args)
    {
    }

    //void __cdecl Format(UINT nFormatID, ...)
    //{
    //}

    void __cdecl Format(LPCWSTR wzFormat, ...)
    {
    }

    CString MakeUpper()
    {
        return CString();
    }

    int Find(LPCWSTR wzSub, int iStart = 0) const throw()
    {
        return 0;
    }

    int Find(WCHAR ch, int iStart = 0) const throw()
    {
        return 0;
    }

    int ReverseFind(WCHAR ch) const throw()
    {
        return 0;
    }

    CString Left(int nCount) const
    {
        return CString();
    }

    CString Right(int nCount) const
    {
        return CString();
    }

    CString Trim()
    {
        return CString();
    }

    CString& Trim(WCHAR chTarget)
    {
        return CString();
    }

    CString Mid(int iFirst) const
    {
        return CString();
    }

    CString Mid(int iFirst, int nCount) const
    {
        return CString();
    }

    int GetLength() const
    {
        return 0;
    }

    int Replace(LPCWSTR wzOld, LPCWSTR wzNew)
    {
        return 0;
    }

    int FindOneOf(LPCWSTR wzCharSet) const throw()
    {
        return 0;
    }

    void Append(LPCWSTR wz)
    {
    }

    CString Tokenize(LPCWSTR wzTokens, int& iStart) const
    {
        return CString();
    }

    int CompareNoCase(LPCWSTR wz) const throw()
    {
        return 0;
    }

    BOOL IsEmpty() const
    {
        return FALSE;
    }

    WCHAR GetAt(int i) const
    {
        return 0;
    }

    LPWSTR GetBuffer(int nMinBufferLength)
    {
        return NULL;
    }

    LPWSTR GetBufferSetLength(int nLength)
    {
        return NULL;
    }

    void _ReleaseBuffer(int nNewLength = -1)
    {
    }

    void __cdecl AppendFormat(LPCWSTR wzFormat, ...)
    {
    }

    int Compare(LPCWSTR wz) const
    {
        return 0;
    }

    void Empty()
    {
    }

    LPWSTR GetBuffer()
    {
        return NULL;
    }

    void SetAt(int iChar, WCHAR ch)
    {
    }

    CString& TrimRight()
    {
        return *this;
    }

    CString& TrimRight(WCHAR chTarget)
    {
        return *this;
    }

    int GetAllocLength() const throw()
    {
        return 0;
    }

    LPCWSTR GetString() const throw()
    {
        return NULL;
    }

    CString& MakeLower()
    {
        return *this;
    }

    int LoadString(HMODULE hInst, UINT nID) throw ()
    {
        return 0;
    }

    void Truncate(int nNewLength)
    {
    }

    void Preallocate(int nLength)
    {
    }

    static int StringLength(PCWSTR psz) throw()
    {
        return 0;
    }

    static void CopyChars(WCHAR* pchDest, DWORD cchDest, const WCHAR* pchSrc, int nChars) throw()
    {
    }
};

class CStringA
{
public:
    CStringA()
    {
    }

    operator LPCSTR() const
    {
        return NULL;
    }

    LPSTR GetBuffer(int nLength)
    {
        return NULL;
    }

    void _ReleaseBuffer(int nNewLength = -1)
    {
    }

    bool operator==(const CStringA& s)
    {
        return false;
    }
};

CString operator+(LPCWSTR wz1, const CString& wz2)
{
    return CString();
}

class CPath
{
public:
    CString m_strPath;

    CPath()
    {
    }

    CPath(const CString& wz)
    {
    }

    CPath(LPCWSTR wz)
    {
    }

    BOOL IsRelative( ) const
    {
        return FALSE;
    }

    BOOL RemoveFileSpec()
    {
        return FALSE;
    }

    operator LPCWSTR() const
    {
        return NULL;
    }

    operator CString&() const
    {
        return CString();
    }

    int FindFileName() const
    {
        return 0;
    }

    int FindExtension() const
    {
        return 0;
    }

    BOOL IsDirectory() const
    {
        return FALSE;
    }

    void StripPath()
    {
    }

    void Combine(LPCWSTR wzDir, LPCWSTR wzFile)
    {
    }

    BOOL Append(PCWSTR wzMore)
    {
        return FALSE;
    }

    BOOL FileExists( ) const
    {
        return FALSE;
    }

    void QuoteSpaces()
    {
    }

    BOOL RelativePathTo(PCWSTR pszFrom, DWORD dwAttrFrom, PCWSTR pszTo, DWORD dwAttrTo)
    {
        return FALSE;
    }

    CString GetExtension()
    {
        return CString();
    }

    BOOL StripToRoot()
    {
        return FALSE;
    }

    int GetDriveNumber( ) const
    {
        return 0;
    }

    void RemoveExtension()
    {
    }

    BOOL CompactPathEx(UINT nMaxChars, DWORD dwFlags = 0)
    {
        return FALSE;
    }
};

class CComBSTR
{
public:
    CComBSTR()
    {
    }

    CComBSTR(const CComBSTR& s)
    {
    }

    CComBSTR(const CString& s)
    {
    }

    CComBSTR(DWORD n)
    {
    }

    CComBSTR operator=(const CComBSTR& s)
    {
        return CComBSTR();
    }

    CComBSTR operator=(const CString& s)
    {
        return CComBSTR();
    }

    CComBSTR operator=(LPCWSTR wz)
    {
        return CComBSTR();
    }

    operator BSTR() const
    {
        return NULL;
    }

    BSTR* operator&()
    {
        return NULL;
    }

    unsigned int Length()
    {
        return 0;
    }
};

template <class T>
class CSimpleArrayEqualHelper
{
public:
    static bool IsEqual(const T& t1, const T& t2)
    {
        return (t1 == t2);
    }
};

template <class T, class TEqual = CSimpleArrayEqualHelper<T> >
class CSimpleArray
{
public:
    BOOL Add(const T& t)
    {
        return FALSE;
    }

    int GetSize() const
    {
        return 0;
    }

    T& operator[](int i) const
    {
        T* p = NULL;
        return *p;
    }

    int Find(const T& t) const
    {
        return 0;
    }

    BOOL Remove(const T& t)
    {
        return FALSE;
    }

    void RemoveAll()
    {
    }

    BOOL RemoveAt(int nIndex)
    {
        return FALSE;
    }

    T* GetData( ) const
    {
        return NULL;
    }

    bool SetCount(size_t nNewSize, int nGrowBy = - 1)
    {
        return false;
    }
};

#define CAtlArray CSimpleArray

class CTimeSpan
{
public:
    CTimeSpan()
    {
    }

    CTimeSpan(const CTimeSpan& t)
    {
    }

    CString __cdecl Format(LPCWSTR wzFormat, ...)
    {
        return CString();
    }
};

class CTime
{
public:
    CTime()
    {
    }

    CTime(const CTime& t)
    {
    }

    __time64_t GetTime( ) const throw()
    {
        return 0;
    }

    static CTime GetCurrentTime()
    {
        return CTime();
    }

    CTimeSpan operator-(const CTime& t)
    {
        return CTimeSpan();
    }
};

class CRegKey
{
public:
    CRegKey()
    {
    }

    CRegKey(const CRegKey& k)
    {
    }

    LONG Open(HKEY hKeyParent, LPCWSTR lpszKeyName, REGSAM samDesired = KEY_READ | KEY_WRITE) throw()
    {
        return 0;
    }

    LONG QueryDWORDValue(LPCWSTR pszValueName, DWORD& dwValue) throw()
    {
        return 0;
    }

    LONG QueryValue(LPCWSTR pszValueName, DWORD* pdwType, void* pData, ULONG* pnBytes) throw()
    {
        return 0;
    }

    LONG QueryStringValue(LPCWSTR pszValueName, LPWSTR pszValue, ULONG* pnChars) throw()
    {
        return 0;
    }

};

CString AtlGetErrorDescription(HRESULT hr, DWORD lcid = 0)
{
    return CString();
}

template <class T>
class CComPtr
{
public:
    CComPtr()
    {
    }

    CComPtr(T* p)
    {
    }

    //CComPtr(CComPtr<T> p)
    //{
    //}

    operator T*() const
    {
        return NULL;
    }
    T** operator&() throw()
    {
        return NULL;
    }

    T* operator->() const throw()
    {
        return NULL;
    }

    bool operator!() const
    {
        return false;
    }

    bool operator==(T* p) const throw()
    {
        return false;
    }

    bool operator==(const CComPtr<T>& p) const throw()
    {
        return false;
    }

    void Release()
    {
    }

    HRESULT CoCreateInstance(
        REFCLSID rclsid,
        LPUNKNOWN pUnkOuter = NULL,
        DWORD dwClsContext = CLSCTX_ALL 
    ) throw( )
    {
        return S_OK;
    }
};

template <class T, const IID* piid = &__uuidof(T)>
class CComQIPtr : public CComPtr<T>
{
public:
    CComQIPtr()
    {
    }

    CComQIPtr(T* p)
    {
    }

    CComQIPtr(IUnknown* p) throw()
    {
    }
};

class CComVariant : public tagVARIANT
{
public:
    CComVariant()
    {
    }

    CComVariant(VARIANT v)
    {
    }
};

void AtlStrToNum(DWORD64* pqw, BSTR , wchar_t** x, DWORD base)
{
}

class CAtlFile
{
public:
    HANDLE m_h;

    CAtlFile()
    {
    }

    CAtlFile(HANDLE h)
    {
    }

    HRESULT Create(
        LPCWSTR wzFilename,
        DWORD dwDesiredAccess,
        DWORD dwShareMode,
        DWORD dwCreationDisposition,
        DWORD dwFlagsAndAttributes = FILE_ATTRIBUTE_NORMAL,
        LPSECURITY_ATTRIBUTES lpsa = NULL,
        HANDLE hTemplateFile = NULL 
    ) throw()
    {
        return S_OK;
    }
 
    HRESULT Seek(LONGLONG nOffset, DWORD dwFrom = FILE_CURRENT) throw()
    {
        return S_OK;
    }

    HRESULT Read(LPVOID pBuffer, DWORD nBufSize, DWORD& nBytesRead) throw()
    {
        return S_OK;
    }

    operator HANDLE() const
    {
        return m_h;
    }

    HRESULT GetSize(ULONGLONG& nLen) const throw()
    {
        return S_OK;
    }

    HRESULT Flush() throw()
    {
        return S_OK;
    }

    HRESULT Write(LPCVOID pBuffer, DWORD nBufSize, DWORD* pnBytesWritten = NULL) throw()
    {
        return S_OK;
    }

    void Close()
    {
    }

    void Attach(HANDLE h) throw()
    {
    }

    HRESULT SetSize(ULONGLONG nNewLen) throw()
    {
        return S_OK;
    }

    HANDLE Detach() throw()
    {
        return S_OK;
    }

};

class CAtlTemporaryFile : public CAtlFile
{
public:
    CAtlTemporaryFile()
    {
    }

    HRESULT Create() throw()
    {
        return S_OK;
    }

    void HandsOff()
    {
    }

    LPCWSTR TempFileName()
    {
        return NULL;
    }
};

class CUrl
{
public:
    CUrl()
    {
    }

    DWORD GetUrlLength() const
    {
        return 0;
    }

    BOOL CreateUrl(LPWSTR lpszUrl, DWORD* pdwMaxLength, DWORD dwFlags = 0) const throw()
    {
        return FALSE;
    }

    BOOL CrackUrl(LPCWSTR lpszUrl, DWORD dwFlags = 0) throw()
    {
        return FALSE;
    }

    LPCWSTR GetSchemeName() const throw()
    {
        return NULL;
    }

    BOOL SetSchemeName(LPCWSTR lpszSchm) throw()
    {
        return FALSE;
    }

    BOOL SetHostName(LPCTSTR lpszHost) throw()
    {
        return FALSE;
    }

    LPCWSTR GetHostName( ) const throw()
    {
        return NULL;
    }

    LPCWSTR GetUrlPath( ) const throw()
    {
        return NULL;
    }

    LPCWSTR GetExtraInfo( ) const throw()
    {
        return NULL;
    }
};

template <class TKey, class TVal>
class CSimpleMapEqualHelper
{
public:
    static bool IsEqualKey(const TKey& k1, const TKey& k2)
    {
        return CSimpleArrayEqualHelper<TKey>::IsEqual(k1, k2);
    }

    static bool IsEqualValue(const TVal& v1, const TVal& v2)
    {
        return CSimpleArrayEqualHelper<TVal>::IsEqual(v1, v2);
    }
};

template <class TKey, class TVal, class TEqual = CSimpleMapEqualHelper<TKey, TVal> >
class CSimpleMap
{
public:
    DWORD m_nSize;

    CSimpleMap()
    {
    }

    int GetSize() const
    {
        return 0;
    }

    TKey& GetKeyAt(int nIndex) const
    {
        TKey* p = NULL;
        return *p;
    }

    TVal& GetValueAt(int nIndex) const
    {
        TVal* p = NULL;
        return *p;
    }

    BOOL Add(const TKey& key, const TVal& val)
    {
        return FALSE;
    }

    int FindKey(const TKey& key) const
    {
        return 0;
    }

    BOOL SetAt(const TKey& key, const TVal& val)
    {
        return FALSE;
    }

    BOOL SetAtIndex(int nIndex, const TKey& key, const TVal& val)
    {
        return FALSE;
    }

    void RemoveAll()
    {
    }

    BOOL Remove(const TKey& key)
    {
        return FALSE;
    }

    BOOL RemoveAt(int nIndex)
    {
        return FALSE;
    }
};

#endif

class CStatic
{
public:
    HWND m_hWnd;

    CStatic()
    {
    }

    BOOL ShowWindow(int nCmdShow) throw()
    {
        return FALSE;
    }

    BOOL UpdateWindow() throw()
    {
        return FALSE;
    }

    BOOL Create(HWND hWndParent, RECT* rect, LPCWSTR szWindowName = NULL, DWORD dwStyle = 0)
    {
        return FALSE;
    }

    BOOL CenterWindow(HWND hWndCenter = NULL) throw()
    {
        return FALSE;
    }

    LONG GetWindowLong(int nIndex) const throw()
    {
        return 0;
    }

    LONG SetWindowLong(int nIndex, LONG dwNewLong) throw()
    {
        return 0;
    }

    HBITMAP SetBitmap(HBITMAP hBitmap)
    {
        return 0;
    }

    BOOL PostMessage(UINT message, WPARAM wParam = 0, LPARAM lParam = 0) throw()
    {
        return FALSE;
    }

};
