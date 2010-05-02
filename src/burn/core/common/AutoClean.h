//-------------------------------------------------------------------------------------------------
// <copyright file="AutoClean.h" company="Microsoft">
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


/*
    void* pMem = ::LocalAlloc(0, 100);
    AutoLocalFree autoLocalFreeMem(pMem);

*/
class AutoLocalFree
{
    void* m_pMem;
public:
    explicit AutoLocalFree(void* pMem) : m_pMem(pMem) {}
    ~AutoLocalFree()
    {
        ::LocalFree(m_pMem);
    }
};


/*
    HGLOBAL pMem = ::GlobalAlloc(0, 100);
    AutoGlobalFree autoGlobalFreeMem(pMem);

*/
template <typename T>
class AutoGlobalFree
{
    T& m_pMem;
public:
    explicit AutoGlobalFree(T& pMem) : m_pMem(pMem) {}
    ~AutoGlobalFree()
    {
        if (m_pMem)
        {
            ::GlobalFree(reinterpret_cast<HGLOBAL>(m_pMem));
            m_pMem = NULL;
        }
    }
};



/*
    FILE* pFile = new FILE();
    AutoDelete<FILE> delFile(pFile);
*/
template<typename T>
class AutoDelete
{
    T* m_pT;
public:
    AutoDelete(T* pT) : m_pT(pT) {}
    ~AutoDelete() { delete m_pT; }
};


/*
    char* szTest = new char[20] = "Test";
    AutoDeleteArray<char> delSzTest(szTest);
*/
template<typename T>
class AutoDeleteArray
{
    T* m_pT;
public:
    AutoDeleteArray(T* pT) : m_pT(pT) {}
    ~AutoDeleteArray() { delete [] m_pT; }
};



/*
    void* pBits;
    HBITMAP h = ::CreateBitmap(20, 90, 2, 222,pBits);
    AutoCloseHandleT<HBITMAP> autoCloseHandle(h);  
    or 
    AutoCloseHandle autoCloseHandle(h);
*/
template<typename T>
struct CloseHandleFunctor
{
    void operator() (T handle)
    {
        ::CloseHandle(handle);
    }
};

template<typename T, template<typename T> class functor > 
struct AutoClose
{
    T m_handle;
public:
    explicit AutoClose(T handle) : m_handle(handle) {}
    ~AutoClose()
    {
        functor<T>(m_handle);
    }
};

template<typename T> struct AutoCloseHandleT : public AutoClose<T, CloseHandleFunctor> 
{
    AutoCloseHandleT(T t) : AutoClose<T, CloseHandleFunctor>(t) {}
};

typedef  AutoClose<HBITMAP, CloseHandleFunctor>  AutoCloseBitmap;
typedef  AutoClose<HANDLE, CloseHandleFunctor>  AutoCloseHandle;


} // namespace IronMan
