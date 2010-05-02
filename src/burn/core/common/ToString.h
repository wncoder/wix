//-------------------------------------------------------------------------------------------------
// <copyright file="ToString.h" company="Microsoft">
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
    // helpers
    inline CString SignedIntToString(int i)
    {
        CString cs;
        cs.Format(L"%d", i);
        return cs;
    }
    inline CString UnsignedIntToString(unsigned int i)
    {
        CString cs;
        cs.Format(L"%u", i);
        return cs;
    }
    inline CString ULONGLONGToString(ULONGLONG i)
    {
        CString cs;
        cs.Format(L"%I64u", i);
        return cs;
    }

    template <typename T> CString ToString(const T& t) { return t.MessageForToStringUser_YouMustWriteASpecializationOf_ToString_ForYourType(); }
    // the line above will always generate a compiler error.
    // Implement free functions similar to these for your user-defined types
    
    template <> inline CString ToString<bool>			(const bool & t)			{ return t ? L"true" : L"false"; }

    template <> inline CString ToString<int>			(const int & t)				{ return SignedIntToString(t); }
    template <> inline CString ToString<long>			(const long & t)			{ return SignedIntToString(t); }
    template <> inline CString ToString<short>			(const short & t)			{ return SignedIntToString(t); }
    template <> inline CString ToString<char>			(const char & t)			{ return SignedIntToString(t); }
    template <> inline CString ToString<signed char>	(const signed char & t)		{ return SignedIntToString(t); }

    template <> inline CString ToString<unsigned int>	(const unsigned int & t)	{ return UnsignedIntToString(t); }
    template <> inline CString ToString<unsigned long>	(const unsigned long & t)	{ return UnsignedIntToString(t); }
    template <> inline CString ToString<unsigned short>	(const unsigned short & t)	{ return UnsignedIntToString(t); }
    template <> inline CString ToString<unsigned char>	(const unsigned char & t)	{ return UnsignedIntToString(t); }
    
    template <> inline CString ToString<ULONGLONG>		(const ULONGLONG & t)		{ return ULONGLONGToString(t); }
}
