//-------------------------------------------------------------------------------------------------
// <copyright file="" company="Microsoft">
//    Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// 
// <summary>
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once

namespace IronMan
{

class SourceLocation
{
    CRITICAL_SECTION m_cs;
    CString m_sourceLocation;

    SourceLocation() : m_sourceLocation()
    {
        ::InitializeCriticalSection(&m_cs);
    }

    ~SourceLocation()
    {
        DeleteCriticalSection(&m_cs);
    }

    CString GetPathPrivate(void)
    {
        CString str;

        ::EnterCriticalSection(&m_cs);
        str = m_sourceLocation;
        ::LeaveCriticalSection(&m_cs);

        return str;
    }

    void SetPathPrivate(CString str)
    {
        ::EnterCriticalSection(&m_cs);
        m_sourceLocation = str;
        ::LeaveCriticalSection(&m_cs);
    }

    static SourceLocation& GetSingleton(void)
    {
        static SourceLocation s_sourceLocation;
        return s_sourceLocation;
    }

public:
    static CString GetPath(void)
    {
        return GetSingleton().GetPathPrivate();
    }
    static void SetPath(CString str)
    {
        GetSingleton().SetPathPrivate(str);
    }
};

}